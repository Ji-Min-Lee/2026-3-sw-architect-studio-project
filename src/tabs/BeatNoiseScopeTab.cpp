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
    mBeatIdx      = 0;
    mHavePendingA = false;
    mPendingAPeak = 0.0;
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    mPlot->replot();
}

void BeatNoiseScopeTab::onMeasurement(const Measurement &m)
{
    bool changed = false;
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            if (mHavePendingA) {
                // 이전 A에 대응하는 C가 없었음 — orphan beat로 확정 (C는 gap)
                mPlot->graph(0)->addData(mBeatIdx, mPendingAPeak);
                mBeatIdx++;
                changed = true;
            }
            mPendingAPeak = ev.peakValue;
            mHavePendingA = true;
        } else if (mHavePendingA) {
            mPlot->graph(0)->addData(mBeatIdx, mPendingAPeak);
            mPlot->graph(1)->addData(mBeatIdx, ev.peakValue);
            mBeatIdx++;
            mHavePendingA = false;
            changed = true;
        }
    }
    if (!changed) return;
    mPlot->xAxis->setRange(qMax(0, mBeatIdx - 200), mBeatIdx);
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
