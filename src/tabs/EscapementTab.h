#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 9: T1→T3 escapement interval per beat (ms).
// Uses AcousticEvent::escapementMs populated by MeasurementEngine.
class EscapementTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit EscapementTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    int          mIdx = 0;
};
