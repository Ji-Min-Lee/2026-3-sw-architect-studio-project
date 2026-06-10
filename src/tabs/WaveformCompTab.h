#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 11: Tic vs Toc waveform overlay.
//
// 문제: A(Tic)와 C(Toc) event가 서로 다른 블록에 있을 수 있음.
//   - 블록 크기 = 4096 샘플 (고정)
//   - beat 주기 = sampleRate * 3600 / bph
//   - 예) 14400bph + 48000Hz → 250ms > 85ms(블록) → 항상 다른 블록
//
// 해결: A event 발생 시 rawPcm 윈도우를 mTicWindow에 복사해서 보관.
//       BPH/SPS에 무관하게 블록 경계를 넘어도 Tic 파형을 꺼낼 수 있음.
class WaveformCompTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit WaveformCompTab(QWidget *parent = nullptr);
    void reset() override;
    QCustomPlot *plot() const { return mPlot; }
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;

    static constexpr int kWindowSamples = 512;

    // A event 파형 복사본 — 블록 경계를 넘어도 유지됨
    QVector<double> mTicXs;
    QVector<double> mTicYs;
    bool            mHaveTic = false;

    // rawPcm에서 event 주변 kWindowSamples개 샘플을 꺼냄
    // samplePos: 절대 샘플 인덱스, tickStart: 이 블록의 시작 절대 인덱스
    void extractWindow(const QVector<float> &rawPcm,
                       double samplePos, uint64_t tickStart,
                       QVector<double> &outXs, QVector<double> &outYs) const;
};
