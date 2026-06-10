#include "VarioTab.h"
#include <QVBoxLayout>

VarioTab::VarioTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    // Graph 0: Tic amplitude (red)
    mPlot->addGraph();
    mPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    mPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(0)->setPen(QPen(Qt::red));
    mPlot->graph(0)->setName("Tic (°)");

    // Graph 1: Toc amplitude (blue)
    mPlot->addGraph();
    mPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    mPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(1)->setPen(QPen(Qt::blue));
    mPlot->graph(1)->setName("Toc (°)");

    mPlot->xAxis->setLabel("Beat #");
    mPlot->yAxis->setLabel("Amplitude (°)");
    mPlot->yAxis->setRange(150, 350);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void VarioTab::reset()
{
    mBeatIdx = 0;
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    mPlot->replot();
}

void VarioTab::onMeasurement(const Measurement &m)
{
    bool changed = false;
    for (const AcousticEvent &ev : m.events) {
        if (!ev.hasAmpSplit) continue;
        mPlot->graph(0)->addData(mBeatIdx, ev.ticAmpDeg);
        mPlot->graph(1)->addData(mBeatIdx, ev.tocAmpDeg);
        mBeatIdx++;
        changed = true;
    }
    if (!changed) return;
    mPlot->xAxis->setRange(qMax(0, mBeatIdx - 200), mBeatIdx);
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
