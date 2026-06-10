#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QComboBox>
#include <QLabel>

// Graph 12 (NEW): Scope Mode with Synchronized Sweep Display
// (project plan Figure 18).
//
// Oscilloscope-style fixed sweep window whose length is a configurable
// multiple (1 / 2 / 4) of the watch's tick interval, fed with the processed
// signal. When the watch runs at its nominal rate the beat pattern stays
// visually stable; a fast or slow watch makes the pattern drift across the
// display. Reference values (daily rate, amplitude, beat error, bph) from
// the most recent measurement are shown above the sweep.
class SweepScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SweepScopeTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    void resizeSweep(int bph);
    void redraw();

    QComboBox   *mMultipleCombo;
    QLabel      *mRefLabel;
    QCustomPlot *mPlot;

    QVector<double> mSweep;     // |processed| samples, fixed length
    int    mWriteIdx  = 0;
    int    mSps       = 48000;
    int    mBph       = 28800;  // fallback until detected
    int    mMultiple  = 2;
};
