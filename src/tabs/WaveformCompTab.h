#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 11: Tic vs Toc waveform overlay — compares A-event and C-event PCM windows.
class WaveformCompTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit WaveformCompTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot *mPlot;

    static constexpr int kWindowSamples = 512;

    double mLastA = 0.0;
    bool   mHaveLastA = false;
};
