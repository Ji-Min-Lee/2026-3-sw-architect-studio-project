#include "BeatNoiseScopeTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <cmath>

BeatNoiseScopeTab::BeatNoiseScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(4, 4, 4, 4);
    root->setSpacing(4);

    // ── control bar ──────────────────────────────────────────
    auto *bar = new QHBoxLayout;
    bar->addWidget(new QLabel("Scope 1 range:"));
    mRangeCombo = new QComboBox;
    mRangeCombo->addItem("20 ms");
    mRangeCombo->addItem("200 ms");
    mRangeCombo->addItem("400 ms");
    bar->addWidget(mRangeCombo);
    bar->addSpacing(16);
    mAvgCheck = new QCheckBox("Σ Avg");
    mAvgCheck->setChecked(true);
    bar->addWidget(mAvgCheck);
    bar->addStretch();
    root->addLayout(bar);

    // ── plots in a vertical splitter ─────────────────────────
    auto *splitter = new QSplitter(Qt::Vertical, this);

    mScope1Plot = new QCustomPlot;
    setupScope1();
    splitter->addWidget(mScope1Plot);

    mScope2Plot = new QCustomPlot;
    setupScope2();
    splitter->addWidget(mScope2Plot);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    root->addWidget(splitter, 1);
}

void BeatNoiseScopeTab::setupScope1()
{
    mScope1Plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mScope1Plot->addGraph();
    QPen wpen; wpen.setColor(QColor(80, 80, 80)); wpen.setWidth(1);
    mScope1Plot->graph(0)->setPen(wpen);
    mScope1Plot->graph(0)->setName("Waveform");
    mScope1Plot->xAxis->setLabel("Time (ms)");
    mScope1Plot->yAxis->setLabel("Amplitude");
    mScope1Plot->legend->setVisible(false);

    // A-event marker (green dashed, at x=0 initially)
    mAMarker = new QCPItemLine(mScope1Plot);
    QPen ap; ap.setColor(Qt::darkGreen); ap.setStyle(Qt::DashLine); ap.setWidth(2);
    mAMarker->setPen(ap);
    mAMarker->start->setType(QCPItemPosition::ptPlotCoords);
    mAMarker->end->setType(QCPItemPosition::ptPlotCoords);
    mAMarker->start->setCoords(0, -1); mAMarker->end->setCoords(0, 1);

    // C-event marker (red dashed)
    mCMarker = new QCPItemLine(mScope1Plot);
    QPen cp; cp.setColor(Qt::red); cp.setStyle(Qt::DashLine); cp.setWidth(2);
    mCMarker->setPen(cp);
    mCMarker->start->setType(QCPItemPosition::ptPlotCoords);
    mCMarker->end->setType(QCPItemPosition::ptPlotCoords);
    mCMarker->start->setCoords(0, -1); mCMarker->end->setCoords(0, 1);
    mCMarker->setVisible(false);
}

void BeatNoiseScopeTab::setupScope2()
{
    mScope2Plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // graph 0: tic (red), offset to upper half
    mScope2Plot->addGraph();
    mScope2Plot->graph(0)->setPen(QPen(Qt::red));
    mScope2Plot->graph(0)->setName("Tic");

    // graph 1: tac (blue), offset to lower half
    mScope2Plot->addGraph();
    mScope2Plot->graph(1)->setPen(QPen(Qt::blue));
    mScope2Plot->graph(1)->setName("Tac");

    // separator line at y = 0
    QCPItemLine *sep = new QCPItemLine(mScope2Plot);
    QPen sp; sp.setColor(QColor(180, 180, 180)); sp.setStyle(Qt::DotLine);
    sep->setPen(sp);
    sep->start->setType(QCPItemPosition::ptAxisRectRatio);
    sep->end->setType(QCPItemPosition::ptAxisRectRatio);
    sep->start->setCoords(0, 0.5); sep->end->setCoords(1, 0.5);

    mScope2Plot->xAxis->setLabel("Time (ms)");
    mScope2Plot->yAxis->setLabel("Tic / Tac");
    mScope2Plot->yAxis->setTickLabels(false);
    mScope2Plot->yAxis->setRange(-2, 2);
    mScope2Plot->xAxis->setRange(0, kScope2Ms);
    mScope2Plot->legend->setVisible(true);
}

// ── reset ────────────────────────────────────────────────────

void BeatNoiseScopeTab::reset()
{
    mHavePendingA  = false;
    mPendingRawPcm.clear();
    mTicCount = mTacCount = 0;
    mScopeWin2 = 0;
    mTicAccum.clear(); mTacAccum.clear();
    mTicAvg.clear();   mTacAvg.clear();

    mScope1Plot->graph(0)->data()->clear();
    mCMarker->setVisible(false);
    mScope1Plot->replot();

    mScope2Plot->graph(0)->data()->clear();
    mScope2Plot->graph(1)->data()->clear();
    mScope2Plot->replot();
}

// ── helpers ──────────────────────────────────────────────────

int BeatNoiseScopeTab::scope1HalfSamples(int sps) const
{
    int msVal = 20;
    if (mRangeCombo) {
        int idx = mRangeCombo->currentIndex();
        if (idx == 1) msVal = 200;
        else if (idx == 2) msVal = 400;
    }
    return sps * msVal / 1000;
}

void BeatNoiseScopeTab::ensureScope2Buffers(int sps)
{
    int win = sps * kScope2Ms / 1000;
    if (mScopeWin2 == win) return;
    mScopeWin2 = win;
    mTicAccum.assign(win, 0.0); mTacAccum.assign(win, 0.0);
    mTicAvg.assign(win, 0.0);   mTacAvg.assign(win, 0.0);
    mTicCount = mTacCount = 0;
}

QVector<double> BeatNoiseScopeTab::extractWindow(const QVector<float> &rawPcm,
                                                  uint64_t tickStart,
                                                  double eventPosAbs,
                                                  int halfSamples, int sps) const
{
    int center = (int)(eventPosAbs - (double)tickStart);
    QVector<double> out(halfSamples, 0.0);
    for (int i = 0; i < halfSamples; i++) {
        int idx = center + i;
        if (idx >= 0 && idx < rawPcm.size())
            out[i] = rawPcm[idx];
    }
    return out;
}

// ── Scope 1 update ───────────────────────────────────────────

void BeatNoiseScopeTab::updateScope1(const QVector<float> &rawPcm,
                                      uint64_t tickStart,
                                      double aPosAbs, double cPosAbs, int sps)
{
    int half = scope1HalfSamples(sps);
    double msPerSample = 1000.0 / sps;

    // Align window: A event sits at t = 0; show [0, range_ms)
    int aIdx = (int)(aPosAbs - (double)tickStart);
    QVector<double> xs, ys;
    xs.reserve(half); ys.reserve(half);
    for (int i = 0; i < half; i++) {
        int idx = aIdx + i;
        xs.append(i * msPerSample);
        ys.append((idx >= 0 && idx < rawPcm.size()) ? (double)rawPcm[idx] : 0.0);
    }
    mScope1Plot->graph(0)->setData(xs, ys);

    // A marker always at x = 0
    double yRange = 1.0;
    mAMarker->start->setCoords(0, -yRange);
    mAMarker->end->setCoords(0,  yRange);

    // C marker at offset from A
    double cOffsetMs = (cPosAbs - aPosAbs) * msPerSample;
    mCMarker->start->setCoords(cOffsetMs, -yRange);
    mCMarker->end->setCoords(cOffsetMs,  yRange);
    mCMarker->setVisible(true);

    mScope1Plot->xAxis->setRange(0, half * msPerSample);
    mScope1Plot->yAxis->rescale();
    mScope1Plot->replot(QCustomPlot::rpQueuedReplot);
}

// ── Scope 2 accumulation ─────────────────────────────────────

void BeatNoiseScopeTab::accumulateScope2(const QVector<float> &rawPcm,
                                          uint64_t tickStart,
                                          double eventPosAbs, int sps,
                                          QVector<double> &accum, int &count,
                                          QVector<double> &avg, int graphIdx)
{
    if (!mAvgCheck || !mAvgCheck->isChecked()) {
        // No averaging: just show the raw window
        QVector<double> win = extractWindow(rawPcm, tickStart, eventPosAbs,
                                            mScopeWin2, sps);
        double msPerSample = 1000.0 / sps;
        double yOff = (graphIdx == 0) ? 1.0 : -1.0;
        QVector<double> xs(mScopeWin2), ys(mScopeWin2);
        for (int i = 0; i < mScopeWin2; i++) {
            xs[i] = i * msPerSample;
            ys[i] = win[i] + yOff;
        }
        mScope2Plot->graph(graphIdx)->setData(xs, ys);
        mScope2Plot->replot(QCustomPlot::rpQueuedReplot);
        return;
    }

    // Accumulate
    QVector<double> win = extractWindow(rawPcm, tickStart, eventPosAbs,
                                        mScopeWin2, sps);
    for (int i = 0; i < mScopeWin2; i++)
        accum[i] += win[i];
    count++;

    // Show running average
    double msPerSample = 1000.0 / sps;
    double yOff = (graphIdx == 0) ? 1.0 : -1.0;
    QVector<double> xs(mScopeWin2), ys(mScopeWin2);
    for (int i = 0; i < mScopeWin2; i++) {
        xs[i] = i * msPerSample;
        ys[i] = (accum[i] / count) + yOff;
        avg[i] = accum[i] / count;
    }
    mScope2Plot->graph(graphIdx)->setData(xs, ys);
    mScope2Plot->replot(QCustomPlot::rpQueuedReplot);

    // Cycle complete: reset accumulation for next cycle
    if (count >= kAvgCycle) {
        accum.fill(0.0);
        count = 0;
    }
}

// ── onMeasurement ────────────────────────────────────────────

void BeatNoiseScopeTab::onMeasurement(const Measurement &m)
{
    if (m.rawPcm.isEmpty()) return;

    ensureScope2Buffers(m.samplesPerSecond);

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            // Save this A event; wait for the paired C
            mHavePendingA    = true;
            mPendingAPos     = ev.samplePos;
            mPendingRawPcm   = m.rawPcm;
            mPendingTickStart = m.graphTickStart;
        } else if (mHavePendingA) {
            // Paired C event arrived
            double aPosAbs = mPendingAPos;
            double cPosAbs = ev.samplePos;

            // Scope 1: use the block that contains the A event
            updateScope1(mPendingRawPcm, mPendingTickStart,
                         aPosAbs, cPosAbs, m.samplesPerSecond);

            // Scope 2: accumulate tic (channel 0) aligned on A, tac (channel 1) aligned on C
            accumulateScope2(mPendingRawPcm, mPendingTickStart,
                             aPosAbs, m.samplesPerSecond,
                             mTicAccum, mTicCount, mTicAvg, 0);
            accumulateScope2(m.rawPcm, m.graphTickStart,
                             cPosAbs, m.samplesPerSecond,
                             mTacAccum, mTacCount, mTacAvg, 1);

            mHavePendingA = false;
        }
    }
}
