#include "SequenceTab.h"
#include <QVBoxLayout>

SequenceTab::SequenceTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    mPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    mPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mPlot->graph(0)->setPen(QPen(Qt::darkCyan));
    mPlot->graph(0)->setName("Beat interval (ms)");
    mPlot->xAxis->setLabel("Beat #");
    mPlot->yAxis->setLabel("Interval (ms)");
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void SequenceTab::reset()
{
    mHaveLastA = false;
    mIdx = 0;
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

void SequenceTab::onMeasurement(const Measurement &m)
{
    bool changed = false;
    for (const AcousticEvent &ev : m.events) {
        if (!ev.isA) continue;
        if (mHaveLastA) {
            double intervalMs = (ev.samplePos - mLastA) / m.samplesPerSecond * 1000.0;
            mPlot->graph(0)->addData(mIdx++, intervalMs);
            changed = true;
        }
        mLastA = ev.samplePos;
        mHaveLastA = true;
    }
    if (!changed) return;
    mPlot->xAxis->setRange(qMax(0, mIdx - kMax), mIdx);
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
