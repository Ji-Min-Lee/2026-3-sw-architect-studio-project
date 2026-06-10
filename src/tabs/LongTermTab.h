#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 8: Long-term rate error trace (wider x-axis window than TraceTab).
class LongTermTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit LongTermTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    double       mTimeElapsed = 0.0;
};
