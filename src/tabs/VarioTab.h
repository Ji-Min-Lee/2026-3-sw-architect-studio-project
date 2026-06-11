#pragma once
#include "BaseGraphTab.h"
#include "MeasurementSummaries.h"
#include "qcustomplot.h"

// Graph 5: Amplitude (°) over time, Tic (red) and Toc (blue) plotted separately.
// X-axis = beat index; Y-axis = amplitude in degrees.
// Uses AcousticEvent::hasAmpSplit populated by MeasurementEngine on each C-event.
class VarioTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit VarioTab(QWidget *parent = nullptr);
    void reset() override;
public:
    QCustomPlot *plot() const { return mPlot; }
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    int          mBeatIdx = 0;
    RunningStats mTicStats;
    RunningStats mTocStats;
};
