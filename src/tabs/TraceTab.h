#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>

// Graph 2: Trace Display (Witschi Chronoscope X1 G3 manual p.14, Figure 8).
//
// Two stacked graphs recorded continuously in real time:
//   top    — rate deviation (s/d): raw samples + smoothed line
//   bottom — amplitude (°) with the normal operating band 270–300° marked
// Alerts the user when the smoothed rate indicates the watch is running late
// or when amplitude falls outside the normal band, and shows long-term
// session averages for both measures.
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
private:
    void updateAlerts();

    QLabel      *mAlertLabel;
    QLabel      *mSummaryLabel;
    QCustomPlot *mPlot;
    QCPGraph    *mSmoothedGraph = nullptr;  // smoothed s/d (top rect)
    QCPGraph    *mAmpGraph      = nullptr;  // amplitude (bottom rect)
    QCPAxisRect *mAmpRect       = nullptr;

    double mTimeElapsed = 0.0;

    // Smoothing (rolling mean of recent rate samples)
    QVector<double> mRateWindow;
    static constexpr int kSmoothWindow = 15;

    // Long-term session averages
    double  mRateSum = 0; quint64 mRateN = 0;
    double  mAmpSum  = 0; quint64 mAmpN  = 0;
    double  mLastSmoothed = 0;
    bool    mHaveAmp = false;
    double  mLastAmp = 0;
};
