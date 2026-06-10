#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 5: Amplitude (degrees) over time — Tic and Toc separately.
class VarioTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit VarioTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    double       mTimeElapsed = 0.0;
};
