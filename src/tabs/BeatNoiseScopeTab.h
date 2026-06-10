#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>

// Graph 7: Beat-Noise Scope Display — Scope 1 & Scope 2
// (Witschi Chronoscope X1 G3 manual p.19, project plan Figure 11).
//
// Scope 1: waveform of the alternating tick/tock beat noises with a
//   selectable time range (20 / 200 / 400 ms), |signal| display, A (green)
//   and C (red) markers, lift angle readout, and a selector to bring one of
//   the last 10 captured beats back for enlarged inspection.
//
// Scope 2: tic and tac noises on two horizontal axes over a fixed 20 ms
//   range, with a Σ control that averages up to 50 tic and 50 tac noises to
//   reduce random noise; the average amplitude of each trace is reported.
class BeatNoiseScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit BeatNoiseScopeTab(QWidget *parent = nullptr);
    void reset() override;
    void setLiftAngle(double deg);

    int capturedBeats() const { return mBeats.size(); }
    QCustomPlot *scope1Plot() const { return mPlot1; }

public slots:
    void onMeasurement(const Measurement &m) override;

private:
    struct Beat {
        QVector<double> ys;       // |processed pcm| window
        double startAbs = 0;      // absolute sample index of ys[0]
        double aPos = -1, cPos = -1;
        bool   isTic = true;
    };

    void appendSamples(const Measurement &m);
    void fulfillPending();
    void redrawScope1();
    void redrawScope2();
    int  rangeSamples() const;

    // Controls
    QComboBox *mViewCombo;
    QComboBox *mRangeCombo;
    QComboBox *mBeatCombo;
    QCheckBox *mAvgCheck;
    QLabel    *mInfoLabel;

    // Scope 1 plot (single rect) / Scope 2 plot (two stacked rects)
    QStackedWidget *mStack;
    QCustomPlot    *mPlot1;
    QCustomPlot    *mPlot2;
    QCPGraph       *mTicGraph2, *mTocGraph2;
    QCPItemLine    *mAMarker, *mCMarker;

    // Rolling |processed| buffer (~1 s)
    QVector<double> mBuf;
    double          mBufStartAbs = 0;
    int             mSps = 48000;

    QList<Beat> mPending;       // waiting for enough samples
    QList<Beat> mBeats;         // fulfilled, newest first (max 10)
    int         mParity = 0;

    // Scope 2 averaging state (20 ms windows)
    QVector<double> mTicSum, mTocSum;
    int mTicCount = 0, mTocCount = 0;
    static constexpr int kAvgCycle = 50;

    double mLiftAngle = 52.0;
};
