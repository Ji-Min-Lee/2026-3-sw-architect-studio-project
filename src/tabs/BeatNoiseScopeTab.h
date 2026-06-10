#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 7: Scope view showing A-event peak amplitude over time.
class BeatNoiseScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit BeatNoiseScopeTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    int          mIdx = 0;
};
