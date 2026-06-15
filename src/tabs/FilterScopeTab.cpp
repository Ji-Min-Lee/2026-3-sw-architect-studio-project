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

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(6, 4, 6, 4);
    lay->setSpacing(4);

    mBlockLabel = new QLabel(this);
    mBlockLabel->setAlignment(Qt::AlignHCenter);
    mBlockLabel->setStyleSheet(QStringLiteral("color: #000000; font-weight: bold;"));
    lay->addWidget(mBlockLabel);

    for (int i = 0; i < kFilterPanels; ++i) {
        FilterPanel panel;

        panel.title = new QLabel(kPanelTitles[i], this);
        panel.title->setStyleSheet(QStringLiteral(
            "font-weight: bold; padding-left: 4px; color: #000000;"));
        lay->addWidget(panel.title);

        panel.plot = new QCustomPlot(this);
        panel.plot->setMinimumHeight(90);
        lay->addWidget(panel.plot, 1);

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
    plot->yAxis->setLabel(tr("Amplitude"));
    plot->xAxis->setLabel(showXLabel ? tr("Time (ms)") : QString());
}

FilterScopeTab::FilterStages FilterScopeTab::computeFilterStages(const QVector<float> &pcm)
{
    const int n = pcm.size();
    FilterStages stages;
    if (n == 0) {
        return stages;
    }

    stages.f0.resize(n);
    stages.f1.resize(n);
    stages.f2.resize(n);
    stages.f3.resize(n);

    double mean = 0.0;
    for (float v : pcm) {
        mean += v;
    }
    mean /= n;

    for (int i = 0; i < n; ++i) {
        stages.f0[i] = static_cast<double>(pcm[i]) - mean;
    }

    double acc = 0.0;
    for (int i = 0; i < n; ++i) {
        acc += std::abs(stages.f0[i]);
        if (i >= kMovingAvgWin) {
            acc -= std::abs(stages.f0[i - kMovingAvgWin]);
        }
        stages.f1[i] = acc / qMin(i + 1, kMovingAvgWin);
    }

    double prev = 0.0;
    for (int i = 0; i < n; ++i) {
        stages.f2[i] = (stages.f1[i] >= prev) ? stages.f1[i] : prev * kFallDecay;
        prev = stages.f2[i];
    }

    prev = 0.0;
    for (int i = 0; i < n; ++i) {
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
    const int n = ys.size();
    const bool mirrored = (mode != 3);

    if (mirrored) {
        QVector<double> pos(n);
        QVector<double> neg(n);
        for (int i = 0; i < n; ++i) {
            pos[i] = std::abs(ys[i]);
            neg[i] = -std::abs(ys[i]);
        }
        panel.posGraph->setData(xs, pos, true);
        panel.negGraph->setData(xs, neg, true);
    } else {
        panel.posGraph->setData(xs, ys, true);
        panel.negGraph->data()->clear();
    }

    for (QCPItemLine *mk : panel.markers) {
        mk->setVisible(false);
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
            auto *mk = new QCPItemLine(panel.plot);
            mk->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
            mk->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
            panel.markers.append(mk);
        }
        QCPItemLine *mk = panel.markers[used++];
        mk->setPen(QPen(ev.isA ? Qt::darkGreen : Qt::red, 1.5, Qt::DashLine));
        mk->start->setCoords(xMs, 0.0);
        mk->end->setCoords(xMs, 1.0);
        mk->setVisible(true);
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
    const int n = pcm.size();
    if (n == 0) {
        return;
    }

    const FilterStages stages = computeFilterStages(pcm);

    QVector<double> xs(n);
    for (int i = 0; i < n; ++i) {
        xs[i] = static_cast<double>(i) / m.samplesPerSecond * 1000.0;
    }

    const double blockMs = xs.last();
    mBlockLabel->setText(
        tr("Filter scope — block %1 ms  (%2 samples @ %3 Hz)")
            .arg(blockMs, 0, 'f', 1)
            .arg(n)
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
        panel.plot->replot();
    }
}
