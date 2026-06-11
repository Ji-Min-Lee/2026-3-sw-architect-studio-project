#include "WaveformCompTab.h"
#include <QVBoxLayout>
#include <QtMath>
#include <cmath>

namespace {

void rescaleBipolarY(QCPAxis *yAxis, QCPGraph *graph)
{
    if (graph->data()->isEmpty()) {
        yAxis->setRange(-1.0, 1.0);
        return;
    }
    bool found = false;
    const QCPRange vr = graph->getValueRange(found, QCP::sdBoth);
    if (!found) {
        yAxis->setRange(-1.0, 1.0);
        return;
    }
    double absMax = qMax(qAbs(vr.lower), qAbs(vr.upper));
    if (absMax < 1e-9) {
        absMax = 1e-9;
    }
    const double pad = qMax(absMax * 0.10, 1e-9);
    yAxis->setRange(-absMax - pad, absMax + pad);
}

} // namespace

WaveformCompTab::WaveformCompTab(QWidget *parent) : BaseGraphTab(parent)
{
    setStyleSheet(QStringLiteral(
        "WaveformCompTab { background-color: #ffffff; }"));

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(6, 4, 6, 4);
    lay->setSpacing(4);

    mValuesLabel = new QLabel(this);
    mValuesLabel->setAlignment(Qt::AlignHCenter);
    mValuesLabel->setStyleSheet(QStringLiteral("color: #000000; font-weight: bold;"));
    lay->addWidget(mValuesLabel);

    mTacLabel = new QLabel(this);
    mTacLabel->setAlignment(Qt::AlignHCenter);
    mTacLabel->setStyleSheet(QStringLiteral("color: #000000;"));
    lay->addWidget(mTacLabel);

    for (int i = 0; i < kBeatPlots; ++i) {
        BeatWindow win;

        win.title = new QLabel(this);
        win.title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        win.title->setStyleSheet(QStringLiteral(
            "font-weight: bold; padding-left: 4px; color: #000000;"));
        lay->addWidget(win.title);

        win.plot = new QCustomPlot(this);
        win.plot->setMinimumHeight(120);
        lay->addWidget(win.plot, 1);

        win.hpfGraph = win.plot->addGraph();
        win.hpfGraph->setPen(QPen(QColor(180, 140, 0), 1.4));
        win.hpfGraph->setBrush(QBrush(QColor(240, 200, 60, 120)));
        win.hpfGraph->removeFromLegend();

        win.plot->legend->setVisible(false);
        win.plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

        stylePlot(win);

        auto makeVLine = [&win](const QColor &c, int width = 1, Qt::PenStyle style = Qt::SolidLine) {
            auto *line = new QCPItemLine(win.plot);
            line->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
            line->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
            line->setPen(QPen(c, width, style));
            return line;
        };

        win.zeroLine = new QCPItemLine(win.plot);
        win.zeroLine->setPen(QPen(QColor(0x55, 0x55, 0x66), 1, Qt::DashLine));

        win.aMarker = makeVLine(QColor(0x44, 0x99, 0xff), 2);
        win.cMarker = makeVLine(QColor(0xff, 0x44, 0x44), 2);
        win.cMarker->setVisible(false);

        win.tAcLabel = new QCPItemText(win.plot);
        win.tAcLabel->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
        win.tAcLabel->setColor(QColor(0xff, 0xaa, 0xaa));
        win.tAcLabel->setVisible(false);

        for (double ms = 0.0; ms <= kWindowMs + 0.001; ms += 5.0) {
            Q_UNUSED(ms);
            auto *g = makeVLine(QColor(0, 120, 0, 80), 1, Qt::DotLine);
            win.msGrid.append(g);
        }

        mWindows.append(win);
    }

    reset();
}

void WaveformCompTab::setLiftAngle(double degrees)
{
    mLiftAngle = degrees;
}

void WaveformCompTab::stylePlot(BeatWindow &win)
{
    QCustomPlot *plot = win.plot;
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
    plot->yAxis->setLabel(tr("HPF amplitude"));
    plot->xAxis->setLabel(tr("Time (ms)"));

    win.degAxis = plot->axisRect()->addAxis(QCPAxis::atTop);
    win.degAxis->setTickLabelColor(QColor(0x66, 0xcc, 0x88));
    win.degAxis->setLabelColor(QColor(0x66, 0xcc, 0x88));
    win.degAxis->setLabel(tr("Wheel from A (deg)"));
}

void WaveformCompTab::updateDegreeAxis(BeatWindow &win)
{
    if (!win.degAxis) {
        return;
    }
    if (mBph <= 0) {
        win.degAxis->setRange(0.0, kWindowMs);
        return;
    }
    const double beatMs = 3600000.0 / static_cast<double>(mBph);
    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    for (double ms = 0.0; ms <= kWindowMs + 0.01; ms += 5.0) {
        const double deg = (ms - kPreMs) / beatMs * 360.0;
        ticker->addTick(ms, QString::number(qRound(deg)) + QChar(0x00B0));
    }
    win.degAxis->setTicker(ticker);
    win.degAxis->setRange(0.0, kWindowMs);
}

void WaveformCompTab::reset()
{
    mBeats.clear();
    mPending.clear();
    mHpfBuf.clear();
    mBufStartAbs = 0.0;

    for (int i = 0; i < mWindows.size(); ++i) {
        BeatWindow &w = mWindows[i];
        w.hpfGraph->data()->clear();
        w.cMarker->setVisible(false);
        w.tAcLabel->setVisible(false);
        w.title->setText(tr("Beat %1 — (waiting)").arg(i + 1));
        w.plot->xAxis->setRange(0.0, kWindowMs);
        w.plot->yAxis->setRange(-1.0, 1.0);
        w.zeroLine->start->setCoords(0.0, 0.0);
        w.zeroLine->end->setCoords(kWindowMs, 0.0);
        updateDegreeAxis(w);
        w.plot->replot();
    }

    mValuesLabel->setText(tr("RATE --- s/d   BEAT ERROR --- ms   BEAT ----- bph"));
    mTacLabel->setText(tr("t_AC  min: --- ms   max: --- ms   σ: --- ms"));
}

void WaveformCompTab::appendBuffer(const Measurement &m)
{
    if (mHpfBuf.isEmpty()) {
        mBufStartAbs = static_cast<double>(m.graphTickStart);
    }

    mHpfBuf.reserve(mHpfBuf.size() + m.hpfPcm.size());
    for (float v : m.hpfPcm) {
        mHpfBuf.append(v);
    }

    const int maxLen = static_cast<int>(kBufSeconds * mSps);
    if (mHpfBuf.size() > maxLen) {
        const int drop = mHpfBuf.size() - maxLen;
        mHpfBuf.remove(0, drop);
        mBufStartAbs += drop;
    }
}

bool WaveformCompTab::extractBeatWindow(double aPos, BeatWave &out) const
{
    const int preSamples  = static_cast<int>(kPreMs  * mSps / 1000.0);
    const int postSamples = static_cast<int>(kPostMs * mSps / 1000.0);
    const int winLen = preSamples + postSamples + 1;
    const double winStart = aPos - preSamples;
    const double bufEnd = mBufStartAbs + mHpfBuf.size();

    if (winStart < mBufStartAbs || winStart + winLen > bufEnd) {
        return false;
    }

    const int s0 = static_cast<int>(winStart - mBufStartAbs);
    out.xs.resize(winLen);
    out.hpfYs.resize(winLen);
    out.aSamplePos = aPos;

    for (int k = 0; k < winLen; ++k) {
        out.xs[k] = k * 1000.0 / static_cast<double>(mSps);
        out.hpfYs[k] = static_cast<double>(mHpfBuf[s0 + k]);
    }

    return true;
}

void WaveformCompTab::fulfillPending()
{
    const int preSamples  = static_cast<int>(kPreMs  * mSps / 1000.0);
    const int postSamples = static_cast<int>(kPostMs * mSps / 1000.0);
    const int winLen = preSamples + postSamples + 1;
    const double bufEnd = mBufStartAbs + mHpfBuf.size();

    for (int i = mPending.size() - 1; i >= 0; --i) {
        const double aPos = mPending[i].aPos;
        const double winStart = aPos - preSamples;
        if (winStart < mBufStartAbs) {
            mPending.removeAt(i);
            continue;
        }
        if (winStart + winLen > bufEnd) {
            continue;
        }

        BeatWave bw;
        if (!extractBeatWindow(aPos, bw)) {
            continue;
        }

        if (!mBeats.isEmpty() && !mBeats.front().hasC) {
            mBeats.removeFirst();
        }
        pushBeat(bw);
        mPending.removeAt(i);
    }
}

void WaveformCompTab::pushBeat(const BeatWave &beat)
{
    mBeats.prepend(beat);
    while (mBeats.size() > kMaxBeats) {
        mBeats.removeLast();
    }
}

void WaveformCompTab::updateTacStats()
{
    QVector<double> tac;
    tac.reserve(mBeats.size());
    for (const BeatWave &b : mBeats) {
        if (b.hasC) {
            tac.append(b.tAcMs);
        }
    }

    if (tac.isEmpty()) {
        mTacLabel->setText(tr("t_AC  min: --- ms   max: --- ms   σ: --- ms"));
        return;
    }

    double minV = tac.front();
    double maxV = tac.front();
    double sum = 0.0;
    for (double v : tac) {
        minV = qMin(minV, v);
        maxV = qMax(maxV, v);
        sum += v;
    }
    const double mean = sum / tac.size();
    double var = 0.0;
    for (double v : tac) {
        const double d = v - mean;
        var += d * d;
    }
    const double sigma = std::sqrt(var / tac.size());

    mTacLabel->setText(
        tr("t_AC  min: %1 ms   max: %2 ms   σ: %3 ms   (lift %4°)")
            .arg(minV, 0, 'f', 2)
            .arg(maxV, 0, 'f', 2)
            .arg(sigma, 0, 'f', 2)
            .arg(mLiftAngle, 0, 'f', 1));
}

void WaveformCompTab::updatePlotGuides(BeatWindow &w, const BeatWave *beat,
                                       double yMin, double yMax)
{
    w.zeroLine->start->setCoords(0.0, 0.0);
    w.zeroLine->end->setCoords(kWindowMs, 0.0);

    int gi = 0;
    for (double ms = 0.0; ms <= kWindowMs + 0.001 && gi < w.msGrid.size(); ms += 5.0) {
        w.msGrid[gi]->start->setCoords(ms, yMin);
        w.msGrid[gi]->end->setCoords(ms, yMax);
        w.msGrid[gi]->setVisible(beat != nullptr);
        ++gi;
    }

    if (!beat) {
        w.aMarker->setVisible(false);
        w.cMarker->setVisible(false);
        w.tAcLabel->setVisible(false);
        return;
    }

    w.aMarker->start->setCoords(kPreMs, 0.0);
    w.aMarker->end->setCoords(kPreMs, 1.0);
    w.aMarker->setVisible(true);

    if (beat->hasC && beat->cMs <= kWindowMs + 0.001) {
        w.cMarker->start->setCoords(beat->cMs, 0.0);
        w.cMarker->end->setCoords(beat->cMs, 1.0);
        w.cMarker->setVisible(true);
        w.tAcLabel->setText(QStringLiteral("t_AC %1 ms").arg(beat->tAcMs, 0, 'f', 2));
        w.tAcLabel->position->setCoords(beat->cMs, 0.95);
        w.tAcLabel->setVisible(true);
    } else {
        w.cMarker->setVisible(false);
        w.tAcLabel->setVisible(false);
    }
}

void WaveformCompTab::redrawPlots()
{
    for (int i = 0; i < kBeatPlots; ++i) {
        BeatWindow &w = mWindows[i];
        const BeatWave *beat = (i < mBeats.size()) ? &mBeats[i] : nullptr;

        if (beat) {
            w.hpfGraph->setData(beat->xs, beat->hpfYs, true);

            if (beat->hasC) {
                w.title->setText(tr("Beat %1 — t_AC = %2 ms")
                                     .arg(i + 1)
                                     .arg(beat->tAcMs, 0, 'f', 2));
            } else {
                w.title->setText(tr("Beat %1 — (waiting for C)").arg(i + 1));
            }
        } else {
            w.hpfGraph->data()->clear();
            w.title->setText(tr("Beat %1 — (waiting)").arg(i + 1));
        }

        w.plot->xAxis->setRange(0.0, kWindowMs);
        rescaleBipolarY(w.plot->yAxis, w.hpfGraph);
        const QCPRange yr = w.plot->yAxis->range();
        updatePlotGuides(w, beat, yr.lower, yr.upper);
        updateDegreeAxis(w);
        w.plot->replot(QCustomPlot::rpQueuedReplot);
    }
}

void WaveformCompTab::onMeasurement(const Measurement &m)
{
    if (m.hpfPcm.isEmpty()) {
        return;
    }

    mSps = m.samplesPerSecond;
    if (m.synced && m.detectedBph > 0) {
        mBph = m.detectedBph;
    }

    appendBuffer(m);

    bool changed = false;

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            PendingBeat pb;
            pb.aPos = ev.samplePos;
            mPending.append(pb);
            changed = true;
        } else if (!mBeats.isEmpty() && !mBeats.front().hasC) {
            BeatWave &bw = mBeats.front();
            bw.tAcMs = (ev.samplePos - bw.aSamplePos) * 1000.0 / static_cast<double>(mSps);
            bw.cMs = kPreMs + bw.tAcMs;
            bw.hasC = true;
            changed = true;
        }
    }

    const int beatsBefore = mBeats.size();
    fulfillPending();
    if (mBeats.size() != beatsBefore) {
        changed = true;
    }

    mValuesLabel->setText(
        QString("RATE %1 s/d   BEAT ERROR %2 ms   BEAT %3 bph")
            .arg(m.rateValid ? QString::number(m.rateErrorSpd, 'f', 1) : "---",
                 m.beatErrorValid ? QString::number(m.beatErrorMs, 'f', 2) : "---",
                 m.synced ? QString::number(m.detectedBph) : "-----"));

    if (!changed || mPaused) {
        return;
    }

    redrawPlots();
    updateTacStats();
}
