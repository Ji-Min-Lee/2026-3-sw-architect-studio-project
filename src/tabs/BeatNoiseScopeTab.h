#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QVector>
#include <QComboBox>
#include <QCheckBox>

// Beat-Noise Scope Display (Spec p.17, Figure 11)
//
// Scope 1 (top): live waveform of the most recent beat, centred on the A event.
//   - Selectable time window: 20 / 200 / 400 ms
//   - Green dashed line = A event position (x = 0)
//   - Red  dashed line  = C event position
//
// Scope 2 (bottom): averaged tic / tac waveforms over a 50-beat cycle.
//   - Fixed 20 ms window, centred on the triggering event
//   - When averaging is ON, waveforms accumulate until 50 tic + 50 tac,
//     then the cycle resets and starts again.
//   - Tic trace (red) plotted at y-offset +1, Tac trace (blue) at y-offset -1.
//   - The system does not assign tic/tac to a fixed channel; it collects
//     alternating A/C pairs as-is (per spec: "no fixed assignment").
class BeatNoiseScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit BeatNoiseScopeTab(QWidget *parent = nullptr);
    void reset() override;

public slots:
    void onMeasurement(const Measurement &m) override;

private:
    // ── controls ──────────────────────────────────────────────
    QComboBox   *mRangeCombo  = nullptr;  // 20 / 200 / 400 ms
    QCheckBox   *mAvgCheck    = nullptr;  // Σ averaging toggle

    // ── Scope 1 ───────────────────────────────────────────────
    QCustomPlot *mScope1Plot  = nullptr;
    QCPItemLine *mAMarker     = nullptr;  // green vertical at x=0
    QCPItemLine *mCMarker     = nullptr;  // red   vertical at C offset

    // ── Scope 2 ───────────────────────────────────────────────
    QCustomPlot *mScope2Plot  = nullptr;

    // ── internal state ────────────────────────────────────────
    static constexpr int kAvgCycle   = 50;   // beats per channel per cycle
    static constexpr int kScope2Ms   = 20;   // fixed 20 ms window for Scope 2

    bool   mHavePendingA      = false;
    double mPendingAPos       = 0.0;
    QVector<float> mPendingRawPcm;
    uint64_t mPendingTickStart = 0;

    // Scope 2 accumulation buffers (allocated on first use)
    int     mScopeWin2        = 0;    // kScope2Ms * sps / 1000
    QVector<double> mTicAccum;
    QVector<double> mTacAccum;
    int     mTicCount         = 0;
    int     mTacCount         = 0;
    QVector<double> mTicAvg;
    QVector<double> mTacAvg;

    // helpers
    void setupScope1();
    void setupScope2();
    void updateScope1(const QVector<float> &rawPcm, uint64_t tickStart,
                      double aPosAbs, double cPosAbs, int sps);
    void accumulateScope2(const QVector<float> &rawPcm, uint64_t tickStart,
                          double eventPosAbs, int sps,
                          QVector<double> &accum, int &count,
                          QVector<double> &avg, int graphIdx);
    QVector<double> extractWindow(const QVector<float> &rawPcm,
                                  uint64_t tickStart,
                                  double eventPosAbs,
                                  int halfSamples, int sps) const;
    void ensureScope2Buffers(int sps);
    int  scope1HalfSamples(int sps) const;
};
