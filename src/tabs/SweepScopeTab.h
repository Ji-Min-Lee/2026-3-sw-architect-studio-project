#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QComboBox>

// Graph 12 (NEW): Scope Mode with Synchronized Sweep Display
// (project plan Figure 18).
//
// Oscilloscope-style fixed sweep window whose length is a configurable
// multiple (1 / 2 / 4) of the watch's tick interval, fed with the processed
// signal. Stability is shown visually: ghost trace of the prior sweep,
// beat-period dividers, and a corner traffic-light dot with a short callout label.
class SweepScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SweepScopeTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;
private:
    void resizeSweep(int bph);
    void redraw();
    void onSweepWrapped();
    int  beatSamples() const;
    int  findPeakInSegment(const QVector<double> &buf, int start, int len) const;
    void updateBeatDividers();
    void updateStabilityIndicator();

    QComboBox      *mMultipleCombo;
    QCustomPlot    *mPlot;
    QCPItemEllipse *mStabilityDot = nullptr;
    QCPItemText    *mStabilityNote = nullptr;
    QCPItemLine    *mStabilityLeader = nullptr;

    QVector<double>  mSweep;      // |processed| samples, fixed length
    QVector<double>  mGhostSweep; // previous sweep — alignment shows drift
    QList<QCPItemLine *> mBeatDividers;

    int    mWriteIdx  = 0;
    int    mSps       = 48000;
    int    mBph       = 28800;  // fallback until detected
    int    mMultiple  = 2;
    double mDriftMs   = 0.0;    // mean peak shift vs prior sweep
};
