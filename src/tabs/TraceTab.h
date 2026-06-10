#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Trace Display (Spec p.14, Figure 8)
//
// Composite view: rate (top) + amplitude (bottom) as two stacked plots.
// Rolling window policy: shows the most recent kWindowSecs seconds.
// Both axes share the same rolling x-range so the user can correlate
// rate and amplitude behaviour at the same point in time.
class TraceTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit TraceTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;

private:
    static constexpr double kWindowSecs = 600.0; // 10-minute rolling window

    QCustomPlot *mRatePlot = nullptr;  // top:    rate error (s/day)
    QCustomPlot *mAmpPlot  = nullptr;  // bottom: amplitude (°)

    double mTimeElapsed = 0.0;

    void applyRollingWindow();
};
