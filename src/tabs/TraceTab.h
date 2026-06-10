#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 2 (NEW): Rate Error s/day trace over time.
// Demonstrates AP-3 ≤3-file rule.
class TraceTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit TraceTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    double       mTimeElapsed = 0.0;
};
