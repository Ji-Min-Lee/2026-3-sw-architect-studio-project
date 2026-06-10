#include "LongTermTab.h"
#include <QVBoxLayout>
#include <QSplitter>

static void setupPlot(QCustomPlot *plot, const QString &yLabel,
                      const QColor &color, const QString &seriesName)
{
    plot->addGraph();
    QPen pen; pen.setColor(color); pen.setWidth(1);
    plot->graph(0)->setPen(pen);
    plot->graph(0)->setName(seriesName);
    plot->xAxis->setLabel("Time (s)");
    plot->yAxis->setLabel(yLabel);
    plot->legend->setVisible(true);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

LongTermTab::LongTermTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    mRatePlot = new QCustomPlot;
    setupPlot(mRatePlot, "Rate Error (s/day)", QColor(0, 60, 160), "Rate (s/day)");
    splitter->addWidget(mRatePlot);

    mAmpPlot = new QCustomPlot;
    setupPlot(mAmpPlot, "Amplitude (°)", QColor(160, 60, 0), "Amplitude (°)");
    splitter->addWidget(mAmpPlot);

    mBeatPlot = new QCustomPlot;
    setupPlot(mBeatPlot, "Beat Error (ms)", QColor(0, 120, 60), "Beat Error (ms)");
    splitter->addWidget(mBeatPlot);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 1);
    layout->addWidget(splitter);
}

void LongTermTab::reset()
{
    mTimeElapsed = 0.0;
    mRateSeries.clear();
    mAmpSeries.clear();
    mBeatSeries.clear();
    mRatePlot->graph(0)->data()->clear();
    mAmpPlot->graph(0)->data()->clear();
    mBeatPlot->graph(0)->data()->clear();
    mRatePlot->replot();
    mAmpPlot->replot();
    mBeatPlot->replot();
}

void LongTermTab::onMeasurement(const Measurement &m)
{
    mTimeElapsed += (double)m.rawPcm.size() / m.samplesPerSecond;

    bool changed = false;

    if (m.rateValid && mRateSeries.feed(mTimeElapsed, m.rateErrorSpd)) {
        mRateSeries.applyTo(mRatePlot, 0);
        mRatePlot->xAxis->rescale();
        mRatePlot->yAxis->rescale();
        mRatePlot->replot(QCustomPlot::rpQueuedReplot);
        changed = true;
    }

    if (m.amplitudeValid && mAmpSeries.feed(mTimeElapsed, m.amplitudeDeg)) {
        mAmpSeries.applyTo(mAmpPlot, 0);
        mAmpPlot->xAxis->rescale();
        mAmpPlot->yAxis->rescale();
        mAmpPlot->replot(QCustomPlot::rpQueuedReplot);
        changed = true;
    }

    if (m.beatErrorValid && mBeatSeries.feed(mTimeElapsed, m.beatErrorMs)) {
        mBeatSeries.applyTo(mBeatPlot, 0);
        mBeatPlot->xAxis->rescale();
        mBeatPlot->yAxis->rescale();
        mBeatPlot->replot(QCustomPlot::rpQueuedReplot);
        changed = true;
    }

    (void)changed;
}
