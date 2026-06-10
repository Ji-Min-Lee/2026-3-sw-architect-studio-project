#include "TraceTab.h"
#include <QVBoxLayout>

TraceTab::TraceTab(QWidget *parent)
    : BaseGraphTab(parent)
    , mPlot(new QCustomPlot(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mPlot);

    mPlot->addGraph();
    QPen pen; pen.setColor(QColor(0, 120, 215)); pen.setWidth(2);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Rate Error (s/day)");

    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Rate Error (s/day)");
    mPlot->yAxis->setRange(-20, 20);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void TraceTab::reset()
{
    mTimeElapsed = 0.0;
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

void TraceTab::onMeasurement(const Measurement &m)
{
    if (!m.rateValid) return;

    // Advance time by the number of PCM samples in this block
    mTimeElapsed += (double)m.rawPcm.size() / m.samplesPerSecond;

    mPlot->graph(0)->addData(mTimeElapsed, m.rateErrorSpd);
    mPlot->xAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
