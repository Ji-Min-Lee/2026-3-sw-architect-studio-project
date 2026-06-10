#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 4: Beat error (ms) rolling average over time.
class BeatErrorTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit BeatErrorTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    double       mTimeElapsed = 0.0;
};
