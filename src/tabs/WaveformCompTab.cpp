#include "WaveformCompTab.h"
#include <QVBoxLayout>

WaveformCompTab::WaveformCompTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph(); // Tic (A-event)
    mPlot->graph(0)->setPen(QPen(Qt::red));
    mPlot->graph(0)->setName("Tic");
    mPlot->addGraph(); // Toc (C-event)
    mPlot->graph(1)->setPen(QPen(Qt::blue));
    mPlot->graph(1)->setName("Toc");

    mPlot->xAxis->setLabel("Sample offset");
    mPlot->yAxis->setLabel("Amplitude");
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void WaveformCompTab::reset()
{
    mHaveLastA = false;
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    mPlot->replot();
}

void WaveformCompTab::onMeasurement(const Measurement &m)
{
    if (m.rawPcm.isEmpty()) return;

    auto extractWindow = [&](double samplePos, int graphIdx) {
        int center = (int)(samplePos - m.graphTickStart);
        int start  = center - kWindowSamples / 2;
        mPlot->graph(graphIdx)->data()->clear();
        QVector<double> xs, ys;
        xs.reserve(kWindowSamples);
        ys.reserve(kWindowSamples);
        for (int i = 0; i < kWindowSamples; i++) {
            int idx = start + i;
            if (idx < 0 || idx >= m.rawPcm.size()) continue;
            xs.append(i - kWindowSamples / 2);
            ys.append(m.rawPcm[idx]);
        }
        mPlot->graph(graphIdx)->setData(xs, ys);
    };

    bool changed = false;
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            extractWindow(ev.samplePos, 0);
            mLastA = ev.samplePos; mHaveLastA = true;
            changed = true;
        } else if (mHaveLastA) {
            extractWindow(ev.samplePos, 1);
            changed = true;
        }
    }

    if (!changed) return;
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
