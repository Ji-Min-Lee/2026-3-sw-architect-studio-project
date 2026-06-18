#include "SweepScopeTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

namespace {
QColor stabilityColor(double driftMs)
{
    if (driftMs < 2.0)  return QColor(46, 204, 113);   // stable — on-rate
    if (driftMs < 8.0)  return QColor(243, 156, 18);   // drifting
    return QColor(231, 76, 60);                      // fast drift
}

QString stabilityLabel(double driftMs)
{
    if (driftMs < 2.0)  return QObject::tr("on-rate");
    if (driftMs < 8.0)  return QObject::tr("drifting");
    return QObject::tr("off-rate");
}
} // namespace

SweepScopeTab::SweepScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);

    auto *controls = new QHBoxLayout;
    controls->addWidget(new QLabel("Sweep length:", this));
    mMultipleCombo = new QComboBox(this);
    mMultipleCombo->addItems({"1 tick", "2 ticks", "4 ticks"});
    mMultipleCombo->setCurrentIndex(1);
    controls->addWidget(mMultipleCombo);
    controls->addStretch(1);
    lay->addLayout(controls);

    mPlot = new QCustomPlot(this);

    mPlot->addGraph();
    mPlot->graph(0)->setPen(QPen(QColor(186, 120, 170), 1.6));
    mPlot->graph(0)->setBrush(QBrush(QColor(200, 140, 190, 130)));

    mPlot->addGraph();
    mPlot->graph(1)->setPen(QPen(QColor(110, 85, 110, 100), 1.2));
    mPlot->graph(1)->setBrush(Qt::NoBrush);
    mPlot->graph(1)->setName(QString());

    mPlot->xAxis->setLabel(tr("Sweep time (ms)"));
    mPlot->yAxis->setLabel(tr("|Amplitude| (rel.)"));
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mPlot->legend->setVisible(false);

    mStabilityDot = new QCPItemEllipse(mPlot);
    mStabilityDot->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
    mStabilityDot->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
    mStabilityDot->topLeft->setCoords(0.965, 0.04);
    mStabilityDot->bottomRight->setCoords(0.995, 0.12);
    mStabilityDot->setPen(QPen(Qt::white, 1.5));
    mStabilityDot->setBrush(QBrush(stabilityColor(0.0)));

    mStabilityNote = new QCPItemText(mPlot);
    mStabilityNote->position->setType(QCPItemPosition::ptAxisRectRatio);
    mStabilityNote->position->setCoords(0.84, 0.08);
    mStabilityNote->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mStabilityNote->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::DemiBold));
    mStabilityNote->setPadding(QMargins(5, 3, 5, 3));
    mStabilityNote->setBrush(QBrush(QColor(14, 14, 20, 210)));
    mStabilityNote->setPen(QPen(QColor(70, 70, 82)));
    mStabilityNote->setColor(stabilityColor(0.0));

    mStabilityLeader = new QCPItemLine(mPlot);
    mStabilityLeader->start->setType(QCPItemPosition::ptAxisRectRatio);
    mStabilityLeader->end->setType(QCPItemPosition::ptAxisRectRatio);
    mStabilityLeader->setPen(QPen(QColor(120, 120, 135, 160), 1.0, Qt::DotLine));
    mStabilityLeader->start->setCoords(0.955, 0.08);
    mStabilityLeader->end->setCoords(0.962, 0.08);

    lay->addWidget(mPlot, 1);

    connect(mMultipleCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int i) {
                static const int mult[] = {1, 2, 4};
                mMultiple = mult[qBound(0, i, 2)];
                mSweep.clear();
                mGhostSweep.clear();
                mDriftMs = 0.0;
                updateStabilityIndicator();
            });

    updateStabilityIndicator();
}

int SweepScopeTab::beatSamples() const
{
    if (mSweep.isEmpty() || mMultiple <= 0) {
        return 0;
    }
    return mSweep.size() / mMultiple;
}

void SweepScopeTab::updateBeatDividers()
{
    for (QCPItemLine *line : mBeatDividers) {
        mPlot->removeItem(line);
    }
    mBeatDividers.clear();

    if (mSps <= 0 || mMultiple <= 1) {
        return;
    }

    const double beatMs = beatSamples() * 1000.0 / mSps;
    QPen pen(QColor(80, 160, 90, 140), 1, Qt::DotLine);
    for (int k = 1; k < mMultiple; ++k) {
        auto *line = new QCPItemLine(mPlot);
        line->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
        line->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
        line->setPen(pen);
        const double xMs = k * beatMs;
        line->start->setCoords(xMs, 0.0);
        line->end->setCoords(xMs, 1.0);
        mBeatDividers.append(line);
    }
}

void SweepScopeTab::updateStabilityIndicator()
{
    if (!mStabilityDot) {
        return;
    }
    const QColor col = stabilityColor(mDriftMs);
    mStabilityDot->setBrush(QBrush(col));

    if (mStabilityNote) {
        mStabilityNote->setColor(col);
        mStabilityNote->setText(
            tr("peak drift  %1").arg(stabilityLabel(mDriftMs)));
    }
    if (mStabilityLeader) {
        mStabilityLeader->setPen(QPen(col.lighter(130), 1.0, Qt::DotLine));
    }
}

int SweepScopeTab::findPeakInSegment(const QVector<double> &buf, int start, int len) const
{
    if (len <= 0 || start < 0 || start + len > buf.size()) {
        return 0;
    }
    int peakIdx = 0;
    double peakVal = -1.0;
    for (int i = 0; i < len; ++i) {
        if (buf[start + i] > peakVal) {
            peakVal = buf[start + i];
            peakIdx = i;
        }
    }
    return peakIdx;
}

void SweepScopeTab::onSweepWrapped()
{
    const int segLen = beatSamples();
    if (segLen <= 0) {
        return;
    }

    if (mGhostSweep.size() != mSweep.size()) {
        mGhostSweep = mSweep;
        mDriftMs = 0.0;
        updateStabilityIndicator();
        return;
    }

    double totalDriftSamples = 0.0;
    for (int tick = 0; tick < mMultiple; ++tick) {
        const int off = tick * segLen;
        const int currPeak = findPeakInSegment(mSweep, off, segLen);
        const int ghostPeak = findPeakInSegment(mGhostSweep, off, segLen);
        totalDriftSamples += std::abs(currPeak - ghostPeak);
    }
    mDriftMs = (totalDriftSamples / mMultiple) * 1000.0 / mSps;
    mGhostSweep = mSweep;
    updateStabilityIndicator();
}

void SweepScopeTab::resizeSweep(int bph)
{
    int beatSamplesCount = (int)std::lround((3600.0 / bph) * mSps);
    int len = qMax(64, beatSamplesCount * mMultiple);
    if (mSweep.size() == len) {
        return;
    }
    mSweep.fill(0.0, len);
    mGhostSweep.clear();
    mWriteIdx = 0;
    mDriftMs = 0.0;
    updateBeatDividers();
    updateStabilityIndicator();
}

void SweepScopeTab::onMeasurement(const Measurement &m)
{
    mSps = m.samplesPerSecond;
    if (m.synced && m.detectedBph > 0) {
        mBph = m.detectedBph;
    }
    resizeSweep(mBph);

    const int prevWriteIdx = mWriteIdx;
    for (double v : m.pcm) {
        mSweep[mWriteIdx] = std::abs(v);
        mWriteIdx = (mWriteIdx + 1) % mSweep.size();
    }

    if (!mSweep.isEmpty() && prevWriteIdx + m.pcm.size() >= mSweep.size()) {
        onSweepWrapped();
    }

    if (mPaused || !isVisible()) {
        return;
    }

    redraw();
}

void SweepScopeTab::redraw()
{
    const int sweepSize = mSweep.size();
    if (sweepSize == 0) {
        return;
    }

    auto buildCurve = [&](const QVector<double> &buf, QVector<double> &xs, QVector<double> &ys) {
        const int step = qMax(1, buf.size() / 2000);
        xs.clear();
        ys.clear();
        xs.reserve(buf.size() / step + 1);
        ys.reserve(buf.size() / step + 1);
        for (int i = 0; i < buf.size(); i += step) {
            double peak = 0;
            for (int k = i; k < qMin(buf.size(), i + step); ++k) {
                peak = qMax(peak, buf[k]);
            }
            xs.append(static_cast<double>(i) / mSps * 1000.0);
            ys.append(peak);
        }
    };

    QVector<double> xs, ys, ghostXs, ghostYs;
    buildCurve(mSweep, xs, ys);
    mPlot->graph(0)->setData(xs, ys, true);

    if (mGhostSweep.size() == sweepSize) {
        buildCurve(mGhostSweep, ghostXs, ghostYs);
        mPlot->graph(1)->setData(ghostXs, ghostYs, true);
        mPlot->graph(1)->setVisible(true);
    } else {
        mPlot->graph(1)->data()->clear();
        mPlot->graph(1)->setVisible(false);
    }

    const double windowMs = static_cast<double>(sweepSize) / mSps * 1000.0;
    mPlot->xAxis->setRange(0, windowMs);
    mPlot->yAxis->rescale();
    updateBeatDividers();
    g_replotCount++;
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void SweepScopeTab::replotAll() { redraw(); }

void SweepScopeTab::reset()
{
    mSweep.clear();
    mGhostSweep.clear();
    mWriteIdx = 0;
    mDriftMs = 0.0;
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    updateStabilityIndicator();
    { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}
