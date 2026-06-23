#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>
#include <QComboBox>

// Graph 4: Beat Error Display and Diagnostic Trace (project plan Figures 12/13).
//
// Top: beat error (ms) rolling average over time with the 0.6 ms
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
    void replotAll() override;
private:
    void updateAlerts(const Measurement &m);
    void applyBeatErrorYScale();

    double windowSec() const;       // visible rolling window of the ms trace
    double yScaleMaxMs() const;     // fixed Y-axis upper bound (ms)

    QLabel      *mAlertLabel;
    QComboBox   *mZoomCombo;
    QComboBox   *mYScaleCombo;
    QCustomPlot *mPlot;
    QCPGraph    *mTicGraph = nullptr;   // diagnostic trace (bottom rect)
    QCPGraph    *mTocGraph = nullptr;
    QCPAxisRect *mTraceRect = nullptr;

    double mTimeElapsed = 0.0;
    int    mBeatIdx     = 0;
    // 150 beats ≈ 25 s @21600 bph — scrolling becomes visible within the
    // ~45 s test files (axis width is fixed from the start, fill then scroll)
    static constexpr int kTracePoints = 150;
};
