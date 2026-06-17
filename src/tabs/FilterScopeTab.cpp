#include "ReplotCounter.h"
#include "FilterScopeTab.h"
#include <QVBoxLayout>
#include <cmath>

namespace {
constexpr int    kMovingAvgWin = 32;
constexpr double kFallDecay    = 0.85;

const char *kPanelTitles[] = {
    "Raw",
    "Smoothed",
    "Envelope",
    "Upper Envelope",
};
} // namespace

FilterScopeTab::FilterScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    setStyleSheet(QStringLiteral("FilterScopeTab { background-color: #ffffff; }"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 4, 6, 4);
    mainLayout->setSpacing(4);

    mBlockLabel = new QLabel(this);
    mBlockLabel->setAlignment(Qt::AlignHCenter);
    mBlockLabel->setStyleSheet(QStringLiteral("color: #000000; font-weight: bold;"));
    mainLayout->addWidget(mBlockLabel);

    for (int i = 0; i < kFilterPanels; ++i) {
        FilterPanel panel;

        panel.title = new QLabel(kPanelTitles[i], this);
        panel.title->setStyleSheet(QStringLiteral(
            "font-weight: bold; padding-left: 4px; color: #000000;"));
        mainLayout->addWidget(panel.title);

        panel.plot = new QCustomPlot(this);
        panel.plot->setMinimumHeight(90);
        mainLayout->addWidget(panel.plot, 1);

        panel.posGraph = panel.plot->addGraph();
        panel.posGraph->setPen(QPen(QColor(190, 170, 30), 1.3));
        panel.posGraph->setBrush(QBrush(QColor(230, 210, 70, 110)));

        panel.negGraph = panel.plot->addGraph();
        panel.negGraph->setPen(QPen(QColor(190, 170, 30), 1.3));
        panel.negGraph->setBrush(QBrush(QColor(230, 210, 70, 110)));

        panel.plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        stylePanel(panel, i == kFilterPanels - 1);

        mPanels.append(panel);
    }

    reset();
}

void FilterScopeTab::stylePanel(FilterPanel &panel, bool showXLabel)
{
    QCustomPlot *plot = panel.plot;
    plot->setBackground(QBrush(QColor(0x0a, 0x0a, 0x0c)));
    plot->axisRect()->setBackground(QBrush(Qt::black));

    const QPen axisPen(QColor(0x88, 0x88, 0x99));
    plot->xAxis->setBasePen(axisPen);
    plot->yAxis->setBasePen(axisPen);
    plot->xAxis->setTickPen(axisPen);
    plot->yAxis->setTickPen(axisPen);
    plot->xAxis->setSubTickPen(axisPen);
    plot->yAxis->setSubTickPen(axisPen);
    plot->xAxis->setTickLabelColor(QColor(0xc8, 0xc8, 0xd0));
    plot->yAxis->setTickLabelColor(QColor(0xc8, 0xc8, 0xd0));
    plot->xAxis->setLabelColor(QColor(0xc8, 0xc8, 0xd0));
    plot->yAxis->setLabelColor(QColor(0xc8, 0xc8, 0xd0));
    plot->yAxis->setLabel(tr("Amplitude (rel.)"));
    plot->xAxis->setLabel(showXLabel ? tr("Time (ms)") : QString());
}

FilterScopeTab::FilterStages FilterScopeTab::computeFilterStages(const QVector<float> &pcm)
{
    const int sampleCount = pcm.size();
    FilterStages stages;
    if (sampleCount == 0) {
        return stages;
    }

    stages.f0.resize(sampleCount);
    stages.f1.resize(sampleCount);
    stages.f2.resize(sampleCount);
    stages.f3.resize(sampleCount);

    double mean = 0.0;
    for (float sample : pcm) {
        mean += sample;
    }
    mean /= sampleCount;

    for (int i = 0; i < sampleCount; ++i) {
        stages.f0[i] = static_cast<double>(pcm[i]) - mean;
    }

    double movingAvgSum = 0.0;
    for (int i = 0; i < sampleCount; ++i) {
        movingAvgSum += std::abs(stages.f0[i]);
        if (i >= kMovingAvgWin) {
            movingAvgSum -= std::abs(stages.f0[i - kMovingAvgWin]);
        }
        stages.f1[i] = movingAvgSum / qMin(i + 1, kMovingAvgWin);
    }

    double prev = 0.0;
    for (int i = 0; i < sampleCount; ++i) {
        stages.f2[i] = (stages.f1[i] >= prev) ? stages.f1[i] : prev * kFallDecay;
        prev = stages.f2[i];
    }

    prev = 0.0;
    for (int i = 0; i < sampleCount; ++i) {
        const double upper = qMax(0.0, stages.f0[i]);
        stages.f3[i] = (upper >= prev) ? upper : prev * kFallDecay;
        prev = stages.f3[i];
    }

    return stages;
}

void FilterScopeTab::drawPanel(FilterPanel &panel, int mode,
                               const QVector<double> &xs, const QVector<double> &ys,
                               const Measurement &m)
{
    const int sampleCount = ys.size();
    const bool mirrored = (mode == 0); // only F0 "raw mirrored" is a bipolar signal

    if (mirrored) {
        QVector<double> pos(sampleCount);
        QVector<double> neg(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            pos[i] = std::abs(ys[i]);
            neg[i] = -std::abs(ys[i]);
        }
        panel.posGraph->setData(xs, pos, true);
        panel.negGraph->setData(xs, neg, true);
    } else {
        panel.posGraph->setData(xs, ys, true);
        panel.negGraph->data()->clear();
    }

    for (QCPItemLine *marker : panel.markers) {
        marker->setVisible(false);
    }

    int used = 0;
    const double xMax = xs.isEmpty() ? 0.0 : xs.last();
    for (const AcousticEvent &ev : m.events) {
        const double xMs = (ev.samplePos - static_cast<double>(m.graphTickStart))
                           / m.samplesPerSecond * 1000.0;
        if (xMs < 0.0 || xMs > xMax) {
            continue;
        }
        if (used >= panel.markers.size()) {
            auto *marker = new QCPItemLine(panel.plot);
            marker->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
            marker->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
            panel.markers.append(marker);
        }
        QCPItemLine *marker = panel.markers[used++];
        marker->setPen(QPen(ev.isA ? Qt::darkGreen : Qt::red, 1.5, Qt::DashLine));
        marker->start->setCoords(xMs, 0.0);
        marker->end->setCoords(xMs, 1.0);
        marker->setVisible(true);
    }

    panel.plot->xAxis->setRange(0.0, xMax);
    panel.plot->yAxis->rescale();
    if (mirrored) {
        panel.plot->yAxis->setRange(-panel.plot->yAxis->range().upper,
                                     panel.plot->yAxis->range().upper);
    }
    panel.plot->replot(QCustomPlot::rpQueuedReplot);
}

void FilterScopeTab::redraw()
{
    if (!mHaveData) {
        return;
    }

    const Measurement &m = mLatest;
    const QVector<float> &pcm = !m.hpfPcm.isEmpty() ? m.hpfPcm : m.rawPcm;
    const int pcmSampleCount = pcm.size();
    if (pcmSampleCount == 0) {
        return;
    }

    const FilterStages stages = computeFilterStages(pcm);

    QVector<double> xs(pcmSampleCount);
    for (int i = 0; i < pcmSampleCount; ++i) {
        xs[i] = static_cast<double>(i) / m.samplesPerSecond * 1000.0;
    }

    const double blockMs = xs.last();
    mBlockLabel->setText(
        tr("Filter scope — block %1 ms  (%2 samples @ %3 Hz)")
            .arg(blockMs, 0, 'f', 1)
            .arg(pcmSampleCount)
            .arg(m.samplesPerSecond));

    const QVector<double> *stageYs[] = {&stages.f0, &stages.f1, &stages.f2, &stages.f3};
    for (int i = 0; i < kFilterPanels; ++i) {
        drawPanel(mPanels[i], i, xs, *stageYs[i], m);
    }
}

void FilterScopeTab::onMeasurement(const Measurement &m)
{
    if (m.hpfPcm.isEmpty() && m.rawPcm.isEmpty()) {
        return;
    }
    mLatest   = m;
    mHaveData = true;
    if (mPaused || !isVisible()) {
        return;
    }
    redraw();
}

void FilterScopeTab::reset()
{
    mHaveData = false;
    mBlockLabel->setText(tr("Filter scope — waiting for data"));
    for (FilterPanel &panel : mPanels) {
        panel.posGraph->data()->clear();
        panel.negGraph->data()->clear();
        for (QCPItemLine *mk : panel.markers) {
            mk->setVisible(false);
        }
        { int64_t _pt=TG_NOW(); panel.plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
    }
}
