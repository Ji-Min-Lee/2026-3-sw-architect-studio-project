#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"

// Graph 1: Rate Error scatter plot (top) + Scope waveform (bottom).
// QCustomPlot widgets are injected from MainWindow.ui.
class RateScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit RateScopeTab(QCustomPlot *ratePlot, QCustomPlot *scopePlot,
                          QWidget *parent = nullptr);
    void reset() override;
    void setScopeScale(int scale) { mScopeScale = scale; }

public slots:
    void onMeasurement(const Measurement &m) override;

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

    QCustomPlot *mRatePlot;
    QCustomPlot *mScopePlot;

    int     mScopeScale = 4;
    int     mXTicIdx    = 0;
    int     mXTocIdx    = 0;
    int     mMaxPoints  = 250;
    QVector<double> mXTic, mYTic, mXToc, mYToc;

    double  mLastA     = 0.0;
    bool    mHaveLastA = false;
};
