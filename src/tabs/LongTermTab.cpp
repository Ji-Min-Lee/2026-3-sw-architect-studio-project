#include "LongTermTab.h"
#include <QVBoxLayout>

LongTermTab::LongTermTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    QPen pen; pen.setColor(Qt::darkBlue); pen.setWidth(1);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Long-term Rate (s/day)");
    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Rate Error (s/day)");
    mPlot->yAxis->setRange(-20, 20);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void LongTermTab::reset()
{
    mTimeElapsed = 0.0;
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

void LongTermTab::onMeasurement(const Measurement &m)
{
    if (!m.rateValid) return;
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;
    mPlot->graph(0)->addData(mTimeElapsed, m.rateErrorSpd);
    // Long-term: show all data, don't clip x-axis
    mPlot->xAxis->rescale();
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
