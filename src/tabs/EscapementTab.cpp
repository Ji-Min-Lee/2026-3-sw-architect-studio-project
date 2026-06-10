#include "EscapementTab.h"
#include <QVBoxLayout>

EscapementTab::EscapementTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    mPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    mPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(0)->setPen(QPen(Qt::darkGreen));
    mPlot->graph(0)->setName("Escapement T1→T3 (ms)");
    mPlot->xAxis->setLabel("Beat #");
    mPlot->yAxis->setLabel("Interval (ms)");
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void EscapementTab::reset()
{
    mIdx = 0;
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

void EscapementTab::onMeasurement(const Measurement &m)
{
    bool changed = false;
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA || !ev.hasEscapementMs) continue;
        mPlot->graph(0)->addData(mIdx++, ev.escapementMs);
        changed = true;
    }
    if (!changed) return;
    mPlot->xAxis->setRange(qMax(0, mIdx - 200), mIdx);
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
