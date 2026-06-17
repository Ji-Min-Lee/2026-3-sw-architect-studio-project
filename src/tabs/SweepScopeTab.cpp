#include "SweepScopeTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

SweepScopeTab::SweepScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);

    auto *controls = new QHBoxLayout;
    controls->addWidget(new QLabel("Sweep length:", this));
    mMultipleCombo = new QComboBox(this);
    mMultipleCombo->addItems({"1 tick", "2 ticks", "4 ticks"});
    mMultipleCombo->setCurrentIndex(1);
    controls->addWidget(mMultipleCombo);
    mRefLabel = new QLabel(this);
    controls->addWidget(mRefLabel, 1);
    lay->addLayout(controls);

    mPlot = new QCustomPlot(this);
    mPlot->addGraph();
    mPlot->graph(0)->setPen(QPen(QColor(186, 120, 170)));
    mPlot->graph(0)->setBrush(QBrush(QColor(200, 140, 190, 130)));
    mPlot->xAxis->setLabel("Sweep time (ms) — stable pattern = on-rate, drifting = fast/slow");
    mPlot->yAxis->setLabel("|Amplitude| (rel.)");
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    lay->addWidget(mPlot, 1);

    connect(mMultipleCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int i) {
                static const int mult[] = {1, 2, 4};
                mMultiple = mult[qBound(0, i, 2)];
                mSweep.clear();          // force re-size on next block
            });

    mRefLabel->setText("Waiting for signal…");
}

void SweepScopeTab::resizeSweep(int bph)
{
    // One tick = one beat: beat period (s) = 3600 / bph
    int beatSamples = (int)std::lround((3600.0 / bph) * mSps);
    int len = qMax(64, beatSamples * mMultiple);
    if (mSweep.size() == len) return;
    mSweep.fill(0.0, len);
    mWriteIdx = 0;
}

void SweepScopeTab::onMeasurement(const Measurement &m)
{
    mSps = m.samplesPerSecond;
    if (m.synced && m.detectedBph > 0) mBph = m.detectedBph;
    resizeSweep(mBph);

    for (double v : m.pcm) {
        mSweep[mWriteIdx] = std::abs(v);
        mWriteIdx = (mWriteIdx + 1) % mSweep.size();
    }

    if (mPaused || !isVisible()) return;

    mRefLabel->setText(QString("<b>DAILY RATE %1 s/d   AMPLITUDE %2°   BEAT ERROR %3 ms   %4 bph</b>")
                           .arg(m.rateValid ? QString::number(m.rateErrorSpd, 'f', 1) : "---",
                                m.amplitudeValid ? QString::number(m.amplitudeDeg, 'f', 0) : "---",
                                m.beatErrorValid ? QString::number(m.beatErrorMs, 'f', 2) : "---",
                                m.synced ? QString::number(mBph) : "-----"));
    redraw();
}

void SweepScopeTab::redraw()
{
    const int sweepSize = mSweep.size();
    if (sweepSize == 0) return;

    // Decimate to ~2000 points for cheap replots (QAS-1)
    const int step = qMax(1, sweepSize / 2000);
    QVector<double> xs, ys;
    xs.reserve(sweepSize / step + 1);
    ys.reserve(sweepSize / step + 1);
    for (int i = 0; i < sweepSize; i += step) {
        double peak = 0;
        for (int k = i; k < qMin(sweepSize, i + step); k++) peak = qMax(peak, mSweep[k]);
        xs.append((double)i / mSps * 1000.0);
        ys.append(peak);
    }
    mPlot->graph(0)->setData(xs, ys, true);
    mPlot->xAxis->setRange(0, (double)sweepSize / mSps * 1000.0);
    mPlot->yAxis->rescale();
    g_replotCount++;
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void SweepScopeTab::replotAll() { redraw(); }

void SweepScopeTab::reset()
{
    mSweep.clear();
    mWriteIdx = 0;
    mPlot->graph(0)->data()->clear();
    mRefLabel->setText("Waiting for signal…");
    { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}
