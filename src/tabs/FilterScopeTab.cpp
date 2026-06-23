#include "ReplotCounter.h"
#include "FilterScopeTab.h"
#include <QVBoxLayout>
#include <algorithm>
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
    plot->yAxis->setLabel(tr("Amplitude"));
    plot->xAxis->setLabel(showXLabel ? tr("Time (ms)") : QString());
}

int FilterScopeTab::beatPeriodSamples() const
{
    const int bph = qMax(1, mBph);
    const double periodSec = 3600.0 / static_cast<double>(bph);
    return qMax(64, static_cast<int>(std::ceil(periodSec * mSampleRate)));
}

double FilterScopeTab::beatPeriodMs() const
{
    return beatPeriodSamples() * 1000.0 / static_cast<double>(mSampleRate);
}

void FilterScopeTab::appendToRing(const Measurement &m)
{
    const QVector<float> &pcm = !m.signal.hpfPcm.isEmpty() ? m.signal.hpfPcm : m.signal.rawPcm;
    if (pcm.isEmpty()) {
        return;
    }

    if (mHpfRing.empty()) {
        mRingStartTick = m.signal.tickStart;
    } else {
        const uint64_t expected = mRingStartTick + static_cast<uint64_t>(mHpfRing.size());
        if (m.signal.tickStart > expected) {
            mHpfRing.clear();
            mRingStartTick = m.signal.tickStart;
        }
    }

    mHpfRing.insert(mHpfRing.end(), pcm.constBegin(), pcm.constEnd());

    const int maxKeep = beatPeriodSamples() * 3;
    if (static_cast<int>(mHpfRing.size()) > maxKeep) {
        const int drop = static_cast<int>(mHpfRing.size()) - maxKeep;
        mHpfRing.erase(mHpfRing.begin(), mHpfRing.begin() + drop);
        mRingStartTick += static_cast<uint64_t>(drop);
    }
}

QVector<float> FilterScopeTab::extractBeatCyclePcm()
{
    const int need = beatPeriodSamples();
    QVector<float> out;
    if (mHpfRing.empty()) {
        return out;
    }

    size_t startIdx = 0;
    if (mHaveLastA && mLastASamplePos >= mRingStartTick) {
        startIdx = static_cast<size_t>(mLastASamplePos - mRingStartTick);
    } else if (mHpfRing.size() > static_cast<size_t>(need)) {
        startIdx = mHpfRing.size() - static_cast<size_t>(need);
    }

    const size_t avail = (startIdx < mHpfRing.size()) ? mHpfRing.size() - startIdx : 0;
    const size_t take  = std::min(static_cast<size_t>(need), avail);
    out.reserve(static_cast<int>(take));
    for (size_t i = 0; i < take; ++i) {
        out.append(mHpfRing[startIdx + i]);
    }
    mCycleStartTick = mRingStartTick + static_cast<uint64_t>(startIdx);
    return out;
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
                               const Measurement &m, double cycleMs)
{
    const int sampleCount = ys.size();
    const bool mirrored = (mode != 3); // F0–F2: bipolar scope; F3 upper envelope stays positive-only

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
    for (const AcousticEvent &ev : m.events) {
        const double xMs = (ev.samplePos - static_cast<double>(mCycleStartTick))
                           / m.signal.samplesPerSecond * 1000.0;
        if (xMs < 0.0 || xMs > cycleMs) {
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

    panel.plot->xAxis->setRange(0.0, cycleMs);
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
    const QVector<float> pcm = extractBeatCyclePcm();
    const int pcmSampleCount = pcm.size();
    if (pcmSampleCount == 0) {
        return;
    }

    const FilterStages stages = computeFilterStages(pcm);

    QVector<double> xs(pcmSampleCount);
    for (int i = 0; i < pcmSampleCount; ++i) {
        xs[i] = static_cast<double>(i) / m.signal.samplesPerSecond * 1000.0;
    }

    const double spanMs = beatPeriodMs();
    mBlockLabel->setText(
        tr("Filter scope — 1 beat cycle (%1 ms, %2 samples @ %3 Hz, %4 BPH)")
            .arg(spanMs, 0, 'f', 1)
            .arg(pcmSampleCount)
            .arg(m.signal.samplesPerSecond)
            .arg(mBph));

    const QVector<double> *stageYs[] = {&stages.f0, &stages.f1, &stages.f2, &stages.f3};
    for (int i = 0; i < kFilterPanels; ++i) {
        drawPanel(mPanels[i], i, xs, *stageYs[i], m, spanMs);
    }
}

void FilterScopeTab::onMeasurement(const Measurement &m)
{
    if (m.signal.hpfPcm.isEmpty() && m.signal.rawPcm.isEmpty()) {
        return;
    }

    if (m.detectedBph > 0) {
        mBph = m.detectedBph;
    }
    if (m.signal.samplesPerSecond > 0) {
        mSampleRate = m.signal.samplesPerSecond;
    }

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            mLastASamplePos = static_cast<uint64_t>(ev.samplePos);
            mHaveLastA      = true;
        }
    }

    appendToRing(m);
    mLatest   = m;
    mHaveData = true;
    if (mPaused || !isVisible()) {
        return;
    }
    redraw();
}

void FilterScopeTab::reset()
{
    mHaveData       = false;
    mHpfRing.clear();
    mRingStartTick  = 0;
    mHaveLastA      = false;
    mLastASamplePos = 0;
    mCycleStartTick = 0;
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
