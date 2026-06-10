#include "TraceTab.h"
#include <QVBoxLayout>
#include <QSplitter>

TraceTab::TraceTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    // ── rate plot (top) ───────────────────────────────────────
    mRatePlot = new QCustomPlot;
    mRatePlot->addGraph();
    QPen rpen; rpen.setColor(QColor(0, 120, 215)); rpen.setWidth(1);
    mRatePlot->graph(0)->setPen(rpen);
    mRatePlot->graph(0)->setName("Rate (s/day)");
    mRatePlot->xAxis->setLabel("Time (s)");
    mRatePlot->yAxis->setLabel("Rate Error (s/day)");
    mRatePlot->yAxis->setRange(-20, 20);
    mRatePlot->legend->setVisible(true);
    mRatePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    splitter->addWidget(mRatePlot);

    // ── amplitude plot (bottom) ───────────────────────────────
    mAmpPlot = new QCustomPlot;
    mAmpPlot->addGraph();
    QPen apen; apen.setColor(QColor(180, 60, 0)); apen.setWidth(1);
    mAmpPlot->graph(0)->setPen(apen);
    mAmpPlot->graph(0)->setName("Amplitude (°)");
    mAmpPlot->xAxis->setLabel("Time (s)");
    mAmpPlot->yAxis->setLabel("Amplitude (°)");
    // Healthy range 270–310° highlighted
    mAmpPlot->yAxis->setRange(150, 360);
    mAmpPlot->legend->setVisible(true);
    mAmpPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    splitter->addWidget(mAmpPlot);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    layout->addWidget(splitter);
}

void TraceTab::reset()
{
    mTimeElapsed = 0.0;
    mRatePlot->graph(0)->data()->clear();
    mAmpPlot->graph(0)->data()->clear();
    mRatePlot->replot();
    mAmpPlot->replot();
}

// Template Method hook: clip both axes to the rolling window
void TraceTab::applyRollingWindow()
{
    double lo = qMax(0.0, mTimeElapsed - kWindowSecs);
    double hi = mTimeElapsed;
    mRatePlot->xAxis->setRange(lo, hi);
    mAmpPlot->xAxis->setRange(lo, hi);

    // Prune data older than the window to keep memory bounded
    if (mTimeElapsed > kWindowSecs) {
        mRatePlot->graph(0)->data()->removeBefore(lo);
        mAmpPlot->graph(0)->data()->removeBefore(lo);
    }
}

void TraceTab::onMeasurement(const Measurement &m)
{
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;

    if (m.rateValid)
        mRatePlot->graph(0)->addData(mTimeElapsed, m.rateErrorSpd);

    if (m.amplitudeValid)
        mAmpPlot->graph(0)->addData(mTimeElapsed, m.amplitudeDeg);

    applyRollingWindow();  // Template Method hook

    mRatePlot->yAxis->rescale();
    mAmpPlot->yAxis->rescale();
    mRatePlot->replot(QCustomPlot::rpQueuedReplot);
    mAmpPlot->replot(QCustomPlot::rpQueuedReplot);
}
