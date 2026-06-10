#include "VarioTab.h"
#include <QVBoxLayout>

VarioTab::VarioTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    mPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    mPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(0)->setPen(QPen(Qt::red));
    mPlot->graph(0)->setName("Amplitude (°)");
    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Amplitude (°)");
    mPlot->yAxis->setRange(200, 350);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void VarioTab::reset()
{
    mTimeElapsed = 0.0;
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

void VarioTab::onMeasurement(const Measurement &m)
{
    if (!m.amplitudeValid) return;
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;
    mPlot->graph(0)->addData(mTimeElapsed, m.amplitudeDeg);
    mPlot->xAxis->rescale();
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
