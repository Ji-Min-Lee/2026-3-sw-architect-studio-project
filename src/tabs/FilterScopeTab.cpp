#include "ReplotCounter.h"
#include "FilterScopeTab.h"
#include <QVBoxLayout>
#include <cmath>

namespace {
constexpr double kDefaultHpfCutoffHz = 200.0;

const char *kPanelTitles[] = {
    "F0",
    "F1",
    "F2",
    "F3",
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

int FilterScopeTab::beatSamples() const
{
    if (mSps <= 0 || mBph <= 0) {
        return 0;
    }
    return static_cast<int>(std::lround((3600.0 / static_cast<double>(mBph)) * mSps));
}

void FilterScopeTab::appendBuffers(const Measurement &m)
{
    mSps = m.samplesPerSecond;

    if (!m.rawPcm.isEmpty()) {
        mRawBuf.reserve(mRawBuf.size() + m.rawPcm.size());
        for (float v : m.rawPcm) {
            mRawBuf.append(v);
        }
    }

    if (!m.hpfPcm.isEmpty()) {
        if (mHpfBuf.isEmpty()) {
            mOutStartAbs = static_cast<double>(m.graphTickStart);
        }
        mHpfBuf.reserve(mHpfBuf.size() + m.hpfPcm.size());
        for (float v : m.hpfPcm) {
            mHpfBuf.append(v);
        }
    }

    if (!m.pcm.isEmpty()) {
        if (mHpfBuf.isEmpty() && mPcmBuf.isEmpty()) {
            mOutStartAbs = static_cast<double>(m.graphTickStart);
        }
        mPcmBuf.reserve(mPcmBuf.size() + m.pcm.size());
        mThrBuf.reserve(mThrBuf.size() + m.pcm.size());
        for (int i = 0; i < m.pcm.size(); ++i) {
            mPcmBuf.append(m.pcm[i]);
            mThrBuf.append(i < m.threshold.size() ? m.threshold[i] : 0.0);
        }
    }

    const int maxLen = static_cast<int>(kBufSeconds * mSps);
    if (mRawBuf.size() > maxLen) {
        const int drop = mRawBuf.size() - maxLen;
        mRawBuf.remove(0, drop);
        mRawStartAbs += drop;
    }
    if (mHpfBuf.size() > maxLen) {
        const int drop = mHpfBuf.size() - maxLen;
        mHpfBuf.remove(0, drop);
        mPcmBuf.remove(0, drop);
        mThrBuf.remove(0, drop);
        mOutStartAbs += drop;
    }
}

void FilterScopeTab::tryCaptureBeat(double aPos)
{
    const int winLen = beatSamples();
    if (winLen <= 0) {
        return;
    }

    const double winEnd = aPos + winLen;
    const double rawEnd = mRawStartAbs + mRawBuf.size();
    const double outEnd = mOutStartAbs + mHpfBuf.size();

    if (aPos < mRawStartAbs || winEnd > rawEnd) {
        return;
    }
    if (aPos < mOutStartAbs || winEnd > outEnd || mHpfBuf.isEmpty()) {
        return;
    }

    const int rawOff = static_cast<int>(aPos - mRawStartAbs);
    const int outOff = static_cast<int>(aPos - mOutStartAbs);

    QVector<float> rawSlice(winLen);
    QVector<float> hpfSlice(winLen);
    QVector<double> pcmSlice(winLen);
    QVector<double> thrSlice(winLen);

    for (int i = 0; i < winLen; ++i) {
        rawSlice[i] = mRawBuf[rawOff + i];
        hpfSlice[i] = mHpfBuf[outOff + i];
        pcmSlice[i] = (outOff + i < mPcmBuf.size()) ? mPcmBuf[outOff + i] : 0.0;
        thrSlice[i] = (outOff + i < mThrBuf.size()) ? mThrBuf[outOff + i] : 0.0;
    }

    mBeatStages = buildStagesFromSlice(rawSlice, hpfSlice, pcmSlice, thrSlice, mSps);
    mBeatAnchorAbs = aPos;
    mBeatEvents.clear();
    mHaveBeatWindow = true;
}

void FilterScopeTab::refreshBeatEvents(const Measurement &m)
{
    if (!mHaveBeatWindow) {
        return;
    }
    const int winLen = beatSamples();
    if (winLen <= 0) {
        return;
    }
    const double winEnd = mBeatAnchorAbs + winLen;
    for (const AcousticEvent &e : m.events) {
        if (e.samplePos < mBeatAnchorAbs || e.samplePos >= winEnd) {
            continue;
        }
        bool exists = false;
        for (const AcousticEvent &stored : mBeatEvents) {
            if (stored.isA == e.isA
                && std::abs(stored.samplePos - e.samplePos) < 0.5) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            mBeatEvents.append(e);
        }
    }
}

void FilterScopeTab::ensureFallbackDsp(int sampleRate)
{
    if (mFallbackReady && mFallbackSps == sampleRate) {
        return;
    }
    tg_hpf_init(&mFallbackHpf, sampleRate, kDefaultHpfCutoffHz);
    tg_envelope_init(&mFallbackEnv, sampleRate, 0.15);
    mFallbackSps   = sampleRate;
    mFallbackReady = true;
}

FilterScopeTab::FilterStages FilterScopeTab::buildStagesFromSlice(
    const QVector<float> &raw,
    const QVector<float> &hpf,
    const QVector<double> &pcm,
    const QVector<double> &threshold,
    int sampleRate) const
{
    FilterStages stages;
    const int refN = !pcm.isEmpty() ? pcm.size()
                   : !hpf.isEmpty()  ? hpf.size()
                   : raw.size();
    if (refN <= 0) {
        return stages;
    }

    stages.f0.resize(refN);
    for (int i = 0; i < refN; ++i) {
        stages.f0[i] = (i < raw.size()) ? static_cast<double>(raw[i]) : 0.0;
    }

    QVector<float> hpfBlock(refN, 0.0f);
    if (!hpf.isEmpty()) {
        for (int i = 0; i < refN; ++i) {
            hpfBlock[i] = hpf[i];
        }
    } else if (!raw.isEmpty()) {
        tg_hpf_t hpfState{};
        tg_hpf_init(&hpfState, sampleRate, kDefaultHpfCutoffHz);
        tg_hpf_process(&hpfState, raw.constData(), hpfBlock.data(), refN);
    }

    stages.f1.resize(refN);
    for (int i = 0; i < refN; ++i) {
        stages.f1[i] = static_cast<double>(hpfBlock[i]);
    }

    if (!pcm.isEmpty()) {
        stages.f2.resize(refN);
        stages.f3.resize(refN);
        for (int i = 0; i < refN; ++i) {
            stages.f2[i] = pcm[i];
            stages.f3[i] = pcm[i];
        }
        if (!threshold.isEmpty()) {
            stages.f3Threshold.resize(refN);
            for (int i = 0; i < refN; ++i) {
                stages.f3Threshold[i] = threshold[i];
            }
        }
    } else {
        tg_envelope_t envState{};
        tg_envelope_init(&envState, sampleRate, 0.15);
        QVector<float> envOut(refN);
        tg_envelope_process(&envState, hpfBlock.constData(), envOut.data(), refN);
        stages.f2.resize(refN);
        stages.f3.resize(refN);
        for (int i = 0; i < refN; ++i) {
            stages.f2[i] = envOut[i];
            stages.f3[i] = envOut[i];
        }
    }

    return stages;
}

FilterScopeTab::FilterStages FilterScopeTab::buildFilterStages(const Measurement &m)
{
    const int refN = !m.pcm.isEmpty()      ? m.pcm.size()
                   : !m.hpfPcm.isEmpty()   ? m.hpfPcm.size()
                   : m.rawPcm.size();
    if (refN <= 0) {
        return {};
    }

    const int rawOff = m.rawPcm.isEmpty() ? 0 : qMax(0, m.rawPcm.size() - refN);
    QVector<float> rawSlice(refN);
    for (int i = 0; i < refN; ++i) {
        rawSlice[i] = (rawOff + i < m.rawPcm.size()) ? m.rawPcm[rawOff + i] : 0.0f;
    }

    const int hpfOff = m.hpfPcm.isEmpty() ? 0 : qMax(0, m.hpfPcm.size() - refN);
    QVector<float> hpfSlice(refN);
    for (int i = 0; i < refN; ++i) {
        hpfSlice[i] = (hpfOff + i < m.hpfPcm.size()) ? m.hpfPcm[hpfOff + i] : 0.0f;
    }

    const int pcmOff = m.pcm.isEmpty() ? 0 : qMax(0, m.pcm.size() - refN);
    QVector<double> pcmSlice(refN);
    QVector<double> thrSlice;
    if (!m.pcm.isEmpty()) {
        thrSlice.resize(refN);
        for (int i = 0; i < refN; ++i) {
            pcmSlice[i] = m.pcm[pcmOff + i];
            thrSlice[i] = (pcmOff + i < m.threshold.size()) ? m.threshold[pcmOff + i] : 0.0;
        }
    }

    if (hpfSlice.isEmpty() && !rawSlice.isEmpty()) {
        ensureFallbackDsp(m.samplesPerSecond);
        hpfSlice.resize(refN);
        tg_hpf_process(&mFallbackHpf, rawSlice.constData(), hpfSlice.data(), refN);
    }

    if (pcmSlice.isEmpty() && !hpfSlice.isEmpty()) {
        ensureFallbackDsp(m.samplesPerSecond);
        pcmSlice.resize(refN);
        QVector<float> envOut(refN);
        tg_envelope_process(&mFallbackEnv, hpfSlice.constData(), envOut.data(), refN);
        for (int i = 0; i < refN; ++i) {
            pcmSlice[i] = envOut[i];
        }
    }

    return buildStagesFromSlice(rawSlice, hpfSlice, pcmSlice, thrSlice, m.samplesPerSecond);
}

void FilterScopeTab::drawPanel(FilterPanel &panel, int mode,
                               const QVector<double> &xs, const QVector<double> &ys,
                               const QVector<double> *threshold,
                               const QVector<AcousticEvent> &events,
                               double anchorAbs, int sampleRate, bool showEventMarkers)
{
    const int sampleCount = ys.size();
    const bool mirrored = (mode == 0);

    if (mirrored) {
        QVector<double> pos(sampleCount);
        QVector<double> neg(sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            pos[i] = std::abs(ys[i]);
            neg[i] = -std::abs(ys[i]);
        }
        panel.posGraph->setData(xs, pos, true);
        panel.negGraph->setData(xs, neg, true);
        panel.negGraph->setPen(QPen(QColor(190, 170, 30), 1.3));
    } else {
        panel.posGraph->setData(xs, ys, true);
        if (threshold && !threshold->isEmpty()) {
            panel.negGraph->setPen(QPen(QColor(220, 80, 80), 1.2, Qt::DashLine));
            panel.negGraph->setData(xs, *threshold, true);
        } else {
            panel.negGraph->data()->clear();
        }
    }

    for (QCPItemLine *marker : panel.markers) {
        marker->setVisible(false);
    }

    int used = 0;
    const double xMax = xs.isEmpty() ? 0.0 : xs.last();
    if (showEventMarkers) {
        for (const AcousticEvent &ev : events) {
            const double xMs = (ev.samplePos - anchorAbs) / sampleRate * 1000.0;
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

    const FilterStages *stages = nullptr;
    QVector<AcousticEvent> events;
    double anchorAbs = 0.0;
    int sampleRate = mSps > 0 ? mSps : mLatest.samplesPerSecond;
    int sampleCount = 0;
    double windowMs = 0.0;
    QString modeLabel;

    if (mHaveBeatWindow) {
        stages     = &mBeatStages;
        events     = mBeatEvents;
        anchorAbs  = mBeatAnchorAbs;
        sampleCount = mBeatStages.f0.size();
        windowMs   = (mBph > 0) ? (3600000.0 / static_cast<double>(mBph)) : 0.0;
        modeLabel  = tr("beat %1 ms @ %2 BPH")
                         .arg(windowMs, 0, 'f', 1)
                         .arg(mBph);
    } else {
        static FilterStages blockStages;
        blockStages = buildFilterStages(mLatest);
        stages      = &blockStages;
        anchorAbs   = static_cast<double>(mLatest.graphTickStart);
        events      = mLatest.events;
        sampleCount = blockStages.f0.size();
        windowMs    = sampleCount > 0
                          ? static_cast<double>(sampleCount - 1) / sampleRate * 1000.0
                          : 0.0;
        modeLabel   = tr("block %1 ms (waiting for beat)")
                          .arg(windowMs, 0, 'f', 1);
    }

    if (sampleCount <= 0) {
        return;
    }

    QVector<double> xs(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        xs[i] = static_cast<double>(i) / sampleRate * 1000.0;
    }

    mBlockLabel->setText(
        tr("Filter scope — %1  (%2 samples @ %3 Hz)")
            .arg(modeLabel)
            .arg(sampleCount)
            .arg(sampleRate));

    const QVector<double> *stageYs[] = {&stages->f0, &stages->f1, &stages->f2, &stages->f3};
    const int modes[] = {0, 0, 0, 2};
    for (int i = 0; i < kFilterPanels; ++i) {
        const QVector<double> *thr = (i == 3 && !stages->f3Threshold.isEmpty())
                                       ? &stages->f3Threshold
                                       : nullptr;
        drawPanel(mPanels[i], modes[i], xs, *stageYs[i], thr, events,
                  anchorAbs, sampleRate, i == 3);
    }
}

void FilterScopeTab::onMeasurement(const Measurement &m)
{
    if (m.hpfPcm.isEmpty() && m.rawPcm.isEmpty()) {
        return;
    }

    mLatest   = m;
    mHaveData = true;

    if (m.synced && m.detectedBph > 0) {
        mBph = m.detectedBph;
    }

    appendBuffers(m);

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            mLastAPos = ev.samplePos;
        }
    }

    if (mLastAPos >= 0.0) {
        const bool hadBeat = mHaveBeatWindow;
        const double prevAnchor = mBeatAnchorAbs;
        tryCaptureBeat(mLastAPos);
        if (mHaveBeatWindow && (!hadBeat || mBeatAnchorAbs != prevAnchor)) {
            mBeatEvents.clear();
        }
    }
    refreshBeatEvents(m);

    if (mPaused || !isVisible()) {
        return;
    }
    redraw();
}

void FilterScopeTab::replotAll()
{
    if (mHaveData && !mPaused) {
        redraw();
    }
}

void FilterScopeTab::reset()
{
    mHaveData      = false;
    mHaveBeatWindow = false;
    mFallbackReady = false;
    mRawBuf.clear();
    mHpfBuf.clear();
    mPcmBuf.clear();
    mThrBuf.clear();
    mRawStartAbs   = 0.0;
    mOutStartAbs   = 0.0;
    mBeatEvents.clear();
    mBeatAnchorAbs = 0.0;
    mLastAPos      = -1.0;
    mBph           = kDefaultBph;
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
