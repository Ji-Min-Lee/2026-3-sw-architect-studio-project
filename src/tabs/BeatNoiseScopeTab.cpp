#include "BeatNoiseScopeTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
constexpr double kBufSeconds   = 1.0;   // rolling buffer length
constexpr double kPreEventMs   = 2.0;   // window starts this much before A
constexpr double kScope2Ms     = 20.0;  // fixed Scope 2 range
} // namespace

BeatNoiseScopeTab::BeatNoiseScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);

    auto *controls = new QHBoxLayout;
    mViewCombo  = new QComboBox(this);
    mViewCombo->addItems({"Scope 1", "Scope 2"});
    mRangeCombo = new QComboBox(this);
    mRangeCombo->addItems({"20 ms", "200 ms", "400 ms"});
    mBeatCombo  = new QComboBox(this);
    mBeatCombo->addItem("Latest beat");
    mAvgCheck   = new QCheckBox("Σ averaging (50 tic / 50 tac)", this);
    mInfoLabel  = new QLabel(this);
    controls->addWidget(mViewCombo);
    controls->addWidget(mRangeCombo);
    controls->addWidget(mBeatCombo);
    controls->addWidget(mAvgCheck);
    controls->addWidget(mInfoLabel, 1);
    lay->addLayout(controls);

    mStack = new QStackedWidget(this);

    mPlot1 = new QCustomPlot(this);
    mPlot1->addGraph();
    mPlot1->graph(0)->setPen(QPen(QColor(180, 140, 0)));
    mPlot1->graph(0)->setBrush(QBrush(QColor(240, 200, 60, 120)));
    mPlot1->xAxis->setLabel("Time (ms)");
    mPlot1->yAxis->setLabel("|Amplitude|");
    mPlot1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mAMarker = new QCPItemLine(mPlot1);
    mCMarker = new QCPItemLine(mPlot1);
    for (auto *mk : {mAMarker, mCMarker}) {
        mk->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
        mk->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
        mk->setVisible(false);
    }
    mAMarker->setPen(QPen(Qt::darkGreen, 1.5, Qt::DashLine));
    mCMarker->setPen(QPen(Qt::red, 1.5, Qt::DashLine));

    mStripBar = new BeatStripBar(this);

    auto *scope1Container = new QWidget(this);
    auto *scope1Lay = new QVBoxLayout(scope1Container);
    scope1Lay->setContentsMargins(0, 0, 0, 0);
    scope1Lay->setSpacing(2);
    scope1Lay->addWidget(mPlot1, 1);
    scope1Lay->addWidget(mStripBar);
    mStack->addWidget(scope1Container);

    connect(mStripBar, &BeatStripBar::beatSelected, this, [this](int i) {
        mBeatCombo->setCurrentIndex(i);
    });

    mPlot2 = new QCustomPlot(this);
    mPlot2->plotLayout()->clear();
    auto *ticRect = new QCPAxisRect(mPlot2);
    auto *tocRect = new QCPAxisRect(mPlot2);
    mPlot2->plotLayout()->addElement(0, 0, ticRect);
    mPlot2->plotLayout()->addElement(1, 0, tocRect);
    mTicGraph2 = mPlot2->addGraph(ticRect->axis(QCPAxis::atBottom), ticRect->axis(QCPAxis::atLeft));
    mTocGraph2 = mPlot2->addGraph(tocRect->axis(QCPAxis::atBottom), tocRect->axis(QCPAxis::atLeft));
    mTicGraph2->setPen(QPen(QColor(180, 140, 0)));
    mTicGraph2->setBrush(QBrush(QColor(240, 200, 60, 120)));
    mTocGraph2->setPen(QPen(QColor(180, 140, 0)));
    mTocGraph2->setBrush(QBrush(QColor(240, 200, 60, 120)));
    ticRect->axis(QCPAxis::atLeft)->setLabel("|Amplitude|");
    tocRect->axis(QCPAxis::atLeft)->setLabel("|Amplitude|");
    tocRect->axis(QCPAxis::atBottom)->setLabel("Time (ms)");
    mStack->addWidget(mPlot2);

    lay->addWidget(mStack, 1);

    connect(mViewCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
        mStack->setCurrentIndex(i);
        mRangeCombo->setEnabled(i == 0);
        mBeatCombo->setEnabled(i == 0);
        mAvgCheck->setEnabled(i == 1);
        if (i == 0) redrawScope1(); else redrawScope2();
    });
    connect(mRangeCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { redrawScope1(); });
    connect(mBeatCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { redrawScope1(); });
    connect(mAvgCheck, &QCheckBox::toggled, this, [this](bool) { redrawScope2(); });

    mRangeCombo->setEnabled(true);
    mAvgCheck->setEnabled(false);
    setLiftAngle(mLiftAngle);
}

void BeatNoiseScopeTab::setLiftAngle(double deg)
{
    mLiftAngle = deg;
    mInfoLabel->setText(QString("Lift angle %1°").arg(mLiftAngle, 0, 'f', 1));
}

int BeatNoiseScopeTab::rangeSamples() const
{
    static const double ms[] = {20.0, 200.0, 400.0};
    int idx = qBound(0, mRangeCombo->currentIndex(), 2);
    return (int)(ms[idx] / 1000.0 * mSps);
}

void BeatNoiseScopeTab::appendSamples(const Measurement &m)
{
    mSps = m.samplesPerSecond;
    if (mBuf.isEmpty()) mBufStartAbs = (double)m.graphTickStart;
    for (double v : m.pcm) mBuf.append(std::abs(v));
    int maxLen = (int)(kBufSeconds * mSps);
    if (mBuf.size() > maxLen) {
        int drop = mBuf.size() - maxLen;
        mBuf.remove(0, drop);
        mBufStartAbs += drop;
    }
}

void BeatNoiseScopeTab::onMeasurement(const Measurement &m)
{
    appendSamples(m);

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            Beat b;
            b.aPos  = ev.samplePos;
            b.isTic = ev.hasRatePoint ? ev.isTic : (mParity == 0);
            mParity ^= 1;
            mPending.append(b);
            if (mPending.size() > 4) mPending.removeFirst();
        } else if (!mPending.isEmpty() && mPending.last().cPos < 0) {
            mPending.last().cPos = ev.samplePos;
        }
    }
    fulfillPending();

    if (mPaused || !isVisible()) return;
    if (mStack->currentIndex() == 0) redrawScope1();
    else                             redrawScope2();
}

void BeatNoiseScopeTab::fulfillPending()
{
    const int pre  = (int)(kPreEventMs / 1000.0 * mSps);
    const int len  = rangeSamples();
    const int len2 = (int)(kScope2Ms / 1000.0 * mSps);
    const double bufEndAbs = mBufStartAbs + mBuf.size();

    for (int i = mPending.size() - 1; i >= 0; i--) {
        Beat &b = mPending[i];
        double start = b.aPos - pre;
        int    need  = qMax(len, len2);
        if (start < mBufStartAbs) { mPending.removeAt(i); continue; }
        if (start + need > bufEndAbs) continue;        // wait for more samples

        b.startAbs = start;
        int s0 = (int)(start - mBufStartAbs);
        b.ys.resize(need);
        for (int k = 0; k < need; k++) b.ys[k] = mBuf[s0 + k];

        // Scope 2 accumulation (first 20 ms of the window)
        QVector<double> *sum = b.isTic ? &mTicSum : &mTocSum;
        int *cnt = b.isTic ? &mTicCount : &mTocCount;
        if (*cnt < kAvgCycle) {
            if (sum->size() != len2) { sum->fill(0.0, len2); *cnt = 0; }
            for (int k = 0; k < len2; k++) (*sum)[k] += b.ys[k];
            (*cnt)++;
        }

        mBeats.prepend(b);
        if (mBeats.size() > 10) mBeats.removeLast();
        mPending.removeAt(i);
    }

    // Beat selector: latest + up to 9 prior strips
    while (mBeatCombo->count() < qMin(mBeats.size(), 10))
        mBeatCombo->addItem(QString("Beat −%1").arg(mBeatCombo->count()));
}

void BeatNoiseScopeTab::redrawScope1()
{
    if (mBeats.isEmpty()) return;
    int idx = qBound(0, mBeatCombo->currentIndex(), mBeats.size() - 1);
    const Beat &b = mBeats[idx];

    const int len = qMin(rangeSamples(), (int)b.ys.size());
    if (len <= 0) return;
    QVector<double> xs(len), ys(len);
    for (int i = 0; i < len; i++) {
        xs[i] = (double)i / mSps * 1000.0;
        ys[i] = b.ys[i];
    }
    mPlot1->graph(0)->setData(xs, ys, true);
    mPlot1->xAxis->setRange(0, xs.last());
    mPlot1->yAxis->rescale();

    auto place = [&](QCPItemLine *mk, double absPos) {
        if (absPos < 0) { mk->setVisible(false); return; }
        double xMs = (absPos - b.startAbs) / mSps * 1000.0;
        mk->start->setCoords(xMs, 0.0);
        mk->end->setCoords(xMs, 1.0);
        mk->setVisible(xMs >= 0 && xMs <= xs.last());
    };
    place(mAMarker, b.aPos);
    place(mCMarker, b.cPos);

    mInfoLabel->setText(QString("Lift angle %1°   A=green C=red markers   beat %2 (%3)")
                            .arg(mLiftAngle, 0, 'f', 1)
                            .arg(idx == 0 ? "latest" : QString("−%1").arg(idx))
                            .arg(b.isTic ? "tic" : "tac"));
    QList<QVector<double>> strips;
    for (const Beat &b : mBeats)
        strips.append(b.ys.mid(0, qMin(rangeSamples(), (int)b.ys.size())));
    mStripBar->setStrips(strips, idx);

    g_replotCount++;
    mPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void BeatNoiseScopeTab::redrawScope2()
{
    const int len2 = (int)(kScope2Ms / 1000.0 * mSps);
    auto trace = [&](bool tic) -> QVector<double> {
        if (mAvgCheck->isChecked()) {
            const QVector<double> &sum = tic ? mTicSum : mTocSum;
            int cnt = tic ? mTicCount : mTocCount;
            QVector<double> out(sum.size());
            for (int i = 0; i < sum.size(); i++) out[i] = cnt ? sum[i] / cnt : 0.0;
            return out;
        }
        for (const Beat &b : mBeats)
            if (b.isTic == tic)
                return b.ys.mid(0, qMin(len2, (int)b.ys.size()));
        return {};
    };

    double avgAmp[2] = {0, 0};
    QCPGraph *graphs[2] = {mTicGraph2, mTocGraph2};
    for (int t = 0; t < 2; t++) {
        QVector<double> ys = trace(t == 0);
        QVector<double> xs(ys.size());
        double peak = 0;
        for (int i = 0; i < ys.size(); i++) {
            xs[i] = (double)i / mSps * 1000.0;
            peak  = qMax(peak, ys[i]);
        }
        avgAmp[t] = peak;
        graphs[t]->setData(xs, ys, true);
        graphs[t]->keyAxis()->setRange(0, kScope2Ms);
        graphs[t]->valueAxis()->rescale();
    }

    QString cycle = (mTicCount >= kAvgCycle && mTocCount >= kAvgCycle)
                        ? "cycle complete"
                        : QString("averaging %1/%2 + %3/%4")
                              .arg(mTicCount).arg(kAvgCycle).arg(mTocCount).arg(kAvgCycle);
    mInfoLabel->setText(QString("Lift angle %1°   trace A avg ➜ %2   trace B avg ➜ %3   (%4)")
                            .arg(mLiftAngle, 0, 'f', 1)
                            .arg(avgAmp[0], 0, 'f', 3).arg(avgAmp[1], 0, 'f', 3)
                            .arg(mAvgCheck->isChecked() ? cycle : "Σ off"));
    g_replotCount++;
    mPlot2->replot(QCustomPlot::rpQueuedReplot);
}

void BeatNoiseScopeTab::replotAll()
{
    if (mStack->currentIndex() == 0) mPlot1->replot();
    else                             mPlot2->replot();
}

void BeatNoiseScopeTab::reset()
{
    mBuf.clear();
    mPending.clear();
    mBeats.clear();
    mParity = 0;
    mTicSum.clear(); mTocSum.clear();
    mTicCount = mTocCount = 0;
    mBeatCombo->blockSignals(true);
    mBeatCombo->clear();
    mBeatCombo->addItem("Latest beat");
    mBeatCombo->blockSignals(false);
    mPlot1->graph(0)->data()->clear();
    mAMarker->setVisible(false);
    mCMarker->setVisible(false);
    mTicGraph2->data()->clear();
    mTocGraph2->data()->clear();
    mStripBar->setStrips({}, 0);
    mPlot1->replot();
    mPlot2->replot();
    setLiftAngle(mLiftAngle);
}
