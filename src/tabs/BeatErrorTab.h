#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>

// Graph 4: Beat Error Display and Diagnostic Trace (project plan Figures 12/13).
//
// Top: numeric readout (rate / amplitude / beat error / bph) plus alerts.
// Middle: beat error (ms) rolling average over time with the 0.6 ms
//   "generally considered good" band marked.
// Bottom: diagnostic tic/toc trace (classic two-dotted-line timegrapher view) —
//   line separation tracks beat error, slope tracks rate; the GUI flags a
//   major fault when the on-screen slope exceeds 45°.
class BeatErrorTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit BeatErrorTab(QWidget *parent = nullptr);
    void reset() override;
public:
    QCustomPlot *plot() const { return mPlot; }
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    void updateHeader(const Measurement &m);

    QLabel      *mHeaderLabel;
    QLabel      *mAlertLabel;
    QCustomPlot *mPlot;
    QCPGraph    *mTicGraph = nullptr;   // diagnostic trace (bottom rect)
    QCPGraph    *mTocGraph = nullptr;
    QCPAxisRect *mTraceRect = nullptr;

    double mTimeElapsed = 0.0;
    int    mBeatIdx     = 0;
    static constexpr int kTracePoints = 250;
};
