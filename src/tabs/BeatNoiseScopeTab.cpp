#include "BeatNoiseScopeTab.h"
#include <QVBoxLayout>

BeatNoiseScopeTab::BeatNoiseScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph(); // A-events (tic)
    mPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    mPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(0)->setPen(QPen(Qt::red));
    mPlot->graph(0)->setName("A peak");

    mPlot->addGraph(); // C-events (toc)
    mPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    mPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(1)->setPen(QPen(Qt::blue));
    mPlot->graph(1)->setName("C peak");

    mPlot->xAxis->setLabel("Beat #");
    mPlot->yAxis->setLabel("Peak amplitude");
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void BeatNoiseScopeTab::reset()
{
    mIdx = 0;
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    mPlot->replot();
}

void BeatNoiseScopeTab::onMeasurement(const Measurement &m)
{
    bool changed = false;
    for (const AcousticEvent &ev : m.events) {
        int g = ev.isA ? 0 : 1;
        mPlot->graph(g)->addData(mIdx++, ev.peakValue);
        changed = true;
    }
    if (!changed) return;
    mPlot->xAxis->setRange(qMax(0, mIdx - 200), mIdx);
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
