#include "BeatErrorTab.h"
#include <QVBoxLayout>

BeatErrorTab::BeatErrorTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    QPen pen; pen.setColor(Qt::darkMagenta); pen.setWidth(2);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Beat Error (ms)");
    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Beat Error (ms)");
    mPlot->yAxis->setRange(0, 5);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void BeatErrorTab::reset()
{
    mTimeElapsed = 0.0;
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

void BeatErrorTab::onMeasurement(const Measurement &m)
{
    if (!m.beatErrorValid) return;
    mTimeElapsed += (double)m.rawPcm.size() / m.samplesPerSecond;
    mPlot->graph(0)->addData(mTimeElapsed, m.beatErrorMs);
    mPlot->xAxis->rescale();
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
