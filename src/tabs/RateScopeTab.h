#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Converts absolute sample-index positions to elapsed seconds from measurement
// start so the scope x-axis reads "0.0 s … 0.5 s" and increases left to right.
class ScopeTimeTicker : public QCPAxisTicker
{
public:
    void setSampleRate(int sps) { mSps = sps; }

protected:
    QString getTickLabel(double tick, const QLocale &, QChar, int) override
    {
        double s = tick / mSps;
        return QString("%1 s").arg(s, 0, 'f', 1);
    }

private:
    int mSps = 48000;
};

// Graph 1: Rate Error scatter plot (top) + Scope waveform (bottom).
// QCustomPlot widgets are injected from MainWindow.ui.
// RS-1: rolling average trend line (graph index 2) overlaid on scatter.
// RS-2: mean ± σ statistics text item updated each beat.
class RateScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit RateScopeTab(QCustomPlot *ratePlot, QCustomPlot *scopePlot,
                          QWidget *parent = nullptr);
    void reset() override;
    void setScopeScale(int scale);


public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;

private:
    void setupPlots();
    void purgeScopeHistory(int sps);
    void addVerticalMarker(double x, double height, const QColor &color);
    void addText(double x, double height, const QString &text,
                 const QColor &color, Qt::Alignment align);
    void addHorizontalMarkerOutward(double xLeft, double xRight,
                                     double height, const QColor &color);
    void addHorizontalMarkerInward(double xLeft, double xRight,
                                    double length, double height, const QColor &color);
    void removeMarkersAndText(double rangeMin, double rangeMax);

    // RS-1: rolling average helpers
    void updateTrendLine();
    // RS-2: statistics overlay helpers
    void updateStatsOverlay();

    // Click-to-sync helpers
    void syncScopeToRateBeat(double beatX);
    void syncRateToBeatNearSample(double sampleX);
    void onRatePlotClicked(QMouseEvent *event);
    void onScopePlotClicked(QMouseEvent *event);

    QCustomPlot *mRatePlot;
    QCustomPlot *mScopePlot;

    int     mScopeScale = 4;
    int     mXTicIdx    = 0;
    int     mXTocIdx    = 0;
    int     mMaxPoints  = 250;
    QVector<double> mXTic, mYTic, mXToc, mYToc;
    // Parallel sample-position vectors for click-to-sync
    QVector<double> mSampleTic, mSampleToc;

    double  mLastA     = 0.0;
    bool    mHaveLastA = false;

    QSharedPointer<ScopeTimeTicker> mScopeTicker;
    int    mSamplesPerSecond = 48000;
    double mLastTickEnd      = 0.0;

    // RS-1: rolling window size for trend line
    static constexpr int kTrendWindow = 20;

    // RS-2: running stats (Welford online algorithm)
    int    mStatCount = 0;
    double mStatMean  = 0.0;
    double mStatM2    = 0.0;   // sum of squared deviations

    QCPItemText *mStatsLabel    = nullptr;
    QCPItemLine *mRateCrosshair = nullptr;  // vertical beat marker on rate plot
};
