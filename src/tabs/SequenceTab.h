#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 6: Consecutive A-event intervals — beat sequence scatter.
class SequenceTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SequenceTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;
    double       mLastA       = 0.0;
    bool         mHaveLastA   = false;
    int          mIdx         = 0;
    static constexpr int kMax = 200;
};
