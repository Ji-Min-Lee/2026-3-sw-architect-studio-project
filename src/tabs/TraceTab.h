#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>
#include <QComboBox>

// Graph 2: Trace Display (Witschi Chronoscope X1 G3 manual §5.2, Figure 8).
//
// Two stacked graphs over a fixed rolling time window (Witschi: 10 min,
// zoom 1/2/4/8×):
//   top    — rate deviation (s/d); the plotted value is already smoothed by
//            the engine's Averaging Period rolling window (spec's required
//            smoothing function — user-adjustable via the control panel)
//   bottom — amplitude (°) with three colour zones:
//            green 270–310° (strong), amber 220–270° (acceptable/wear),
//            red <220° (needs service)
//
// Time always advances with the audio stream, so signal dropouts appear as
// visible gaps in the trace. All data is retained: with Pause the user can
// drag/zoom back through the full session (project plan p.10).
// Alerts when the rate indicates the watch is running late or amplitude
// leaves the normal band; shows session average + 60 s rolling average.
class TraceTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit TraceTab(QWidget *parent = nullptr);
    void reset() override;
public:
    QCustomPlot *plot() const { return mPlot; }
public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;
private:
    void updateAlerts();
    void updateRanges();
    double windowSec() const;        // visible rolling window (zoom-dependent)
    double rollingAvg(const QVector<QPair<double, double>> &buf) const;

    QLabel          *mAlertLabel;
    QCPTextElement  *mRateSummary = nullptr;  // note below rate graph
    QCPTextElement  *mAmpSummary  = nullptr;  // note below amplitude graph
    QComboBox   *mZoomCombo;
    QCustomPlot *mPlot;
    QCPGraph    *mAmpGraph = nullptr;   // amplitude (bottom rect)
    QCPAxisRect *mAmpRect  = nullptr;

    double mTimeElapsed = 0.0;

    // Y auto-expansion (never narrower than the nominal ranges)
    double mRateMin = 0, mRateMax = 0;
    double mAmpMin  = 0, mAmpMax  = 0;

    // Session averages + 60 s rolling averages
    double  mRateSum = 0; quint64 mRateN = 0;
    double  mAmpSum  = 0; quint64 mAmpN  = 0;
    QVector<QPair<double, double>> mRateRecent;  // (t, value), last 60 s
    QVector<QPair<double, double>> mAmpRecent;
    bool    mHaveRate = false;
    double  mLastRate = 0;
    bool    mHaveAmp  = false;
    double  mLastAmp  = 0;

    static constexpr double kBaseWindowSec = 600.0;  // Witschi: 10 min at zoom 1
    static constexpr double kRollingAvgSec = 60.0;
};
