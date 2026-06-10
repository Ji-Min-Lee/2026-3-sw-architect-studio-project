#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>
#include <QList>

// Graph 11: Waveform Comparison Display with Timing Markers
// (project plan Figure 17).
//
// Compares multiple beat waveforms in aligned lanes:
//   graph(0)/graph(1) — the latest Tic/Toc pair, overlaid
//   below them       — up to 4 previous beat pairs in vertically offset lanes
// with vertical millisecond guide markers and a numeric overlay showing
// rate / beat error / bph from the same Measurement stream.
//
// 문제: A(Tic)와 C(Toc) event가 서로 다른 블록에 있을 수 있음.
// 해결: A event 발생 시 rawPcm 윈도우를 mTicXs/mTicYs에 복사해서 보관.
class WaveformCompTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit WaveformCompTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QLabel      *mValuesLabel;
    QCustomPlot *mPlot;

    static constexpr int kWindowSamples = 512;
    static constexpr int kLanes        = 4;     // previous beat pairs shown
    static constexpr double kLaneSpacing = 1.2; // vertical offset per lane

    // A event 파형 복사본 — 블록 경계를 넘어도 유지됨
    QVector<double> mTicXs;
    QVector<double> mTicYs;
    bool            mHaveTic = false;

    struct BeatPair { QVector<double> ticXs, ticYs, tocXs, tocYs; };
    QList<BeatPair> mHistory;                   // newest first
    QList<QCPGraph *> mLaneGraphs;              // 2 per lane (tic, toc)
    QList<QCPItemLine *> mGuides;
    int mSps = 48000;

    void extractWindow(const QVector<float> &rawPcm,
                       double samplePos, uint64_t tickStart,
                       QVector<double> &outXs, QVector<double> &outYs) const;
    void redrawLanes();
    void updateGuides();
};
