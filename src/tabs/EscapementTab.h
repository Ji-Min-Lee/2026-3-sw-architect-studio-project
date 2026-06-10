#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QComboBox>
#include <QLabel>

// Graph 9: Escapement Analyzer and Marker-Line Display (project plan Figure 15).
//
// Shows the raw acoustic waveform of one beat with vertical timing markers on
// the A (T1, green) and C (T3, red) events and a millisecond label for the
// elapsed A→C interval. A selector compares alternative C reference points
// (peak vs onset) so the user can judge which is the more stable timing
// reference.
class EscapementTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit EscapementTab(QWidget *parent = nullptr);
    void reset() override;

    double currentEscapementMs() const { return mCurrentMs; }

public slots:
    void onMeasurement(const Measurement &m) override;

private:
    struct Beat {
        QVector<double> ys;     // raw pcm window
        double startAbs = 0;
        double aPos = -1, cPeakPos = -1, cOnsetPos = -1;
        bool   cOnsetValid = false;
    };

    void redraw();

    QComboBox   *mRefCombo;
    QLabel      *mDeltaLabel;
    QCustomPlot *mPlot;
    QCPItemLine *mAMarker, *mCMarker;
    QCPItemText *mALabel, *mCLabel, *mDeltaText;

    QVector<double> mBuf;       // rolling raw pcm
    double          mBufStartAbs = 0;
    int             mSps = 48000;

    Beat   mPendingBeat;        // A seen, waiting for C / samples
    bool   mHavePending = false;
    Beat   mBeat;               // last complete beat
    bool   mHaveBeat = false;
    double mCurrentMs = 0.0;
};
