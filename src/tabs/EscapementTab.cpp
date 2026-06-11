#include "EscapementTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <numeric>
#include <cmath>

namespace {
constexpr double kBufSeconds = 1.0;
constexpr double kPreMs      = 3.0;   // window margin before A
constexpr double kPostMs     = 5.0;   // window margin after C
} // namespace

EscapementTab::EscapementTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);

    auto *controls = new QHBoxLayout;
    controls->addWidget(new QLabel("C reference:", this));
    mRefCombo = new QComboBox(this);
    mRefCombo->addItems({"Peak", "Onset"});
    controls->addWidget(mRefCombo);
    mDeltaLabel = new QLabel(this);
    controls->addWidget(mDeltaLabel, 1);
    lay->addLayout(controls);

    mStabilityLabel = new QLabel(this);
    mStabilityLabel->setStyleSheet("color: gray; font-size: 11px;");
    lay->addWidget(mStabilityLabel);

    mPlot = new QCustomPlot(this);
    mPlot->addGraph();
    mPlot->graph(0)->setPen(QPen(QColor(90, 90, 90)));
    mPlot->xAxis->setLabel("Time from A (ms)");
    mPlot->yAxis->setLabel("Amplitude");
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    lay->addWidget(mPlot, 1);

    auto makeMarker = [&](const QColor &c) {
        auto *mk = new QCPItemLine(mPlot);
        mk->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
        mk->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
        mk->setPen(QPen(c, 2));
        mk->setVisible(false);
        return mk;
    };
    mAMarker = makeMarker(Qt::darkGreen);
    mCMarker = makeMarker(Qt::red);

    mACRect = new QCPItemRect(mPlot);
    mACRect->setBrush(QBrush(QColor(255, 165, 0, 40)));
    mACRect->setPen(Qt::NoPen);
    mACRect->setVisible(false);

    auto makeText = [&](const QColor &c) {
        auto *t = new QCPItemText(mPlot);
        t->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
        t->setColor(c);
        t->setVisible(false);
        return t;
    };
    mALabel    = makeText(Qt::darkGreen);
    mCLabel    = makeText(Qt::red);
    mDeltaText = makeText(Qt::black);
    QFont f = mDeltaText->font(); f.setBold(true); mDeltaText->setFont(f);

    connect(mRefCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { redraw(); });

    mDeltaLabel->setText("Waiting for a complete beat (A → C)…");
}

void EscapementTab::onMeasurement(const Measurement &m)
{
    mSps = m.samplesPerSecond;

    // Rolling raw buffer
    if (mBuf.isEmpty()) mBufStartAbs = (double)m.graphTickStart;
    for (float v : m.rawPcm) mBuf.append(v);
    int maxLen = (int)(kBufSeconds * mSps);
    if (mBuf.size() > maxLen) {
        int drop = mBuf.size() - maxLen;
        mBuf.remove(0, drop);
        mBufStartAbs += drop;
    }

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            mPendingBeat = Beat{};
            mPendingBeat.aPos = ev.samplePos;
            mHavePending = true;
        } else if (mHavePending && mPendingBeat.cPeakPos < 0) {
            mPendingBeat.cPeakPos    = ev.samplePos;
            mPendingBeat.cOnsetValid = ev.cOnsetValid;
            mPendingBeat.cOnsetPos   = ev.cOnsetPos;
        }
    }

    // Snapshot the window once the buffer covers A-pre .. C+post
    if (mHavePending && mPendingBeat.cPeakPos >= 0) {
        double start = mPendingBeat.aPos - kPreMs / 1000.0 * mSps;
        double end   = mPendingBeat.cPeakPos + kPostMs / 1000.0 * mSps;
        double bufEnd = mBufStartAbs + mBuf.size();
        if (start >= mBufStartAbs && end <= bufEnd) {
            mPendingBeat.startAbs = start;
            int s0 = (int)(start - mBufStartAbs);
            int n  = (int)(end - start);
            mPendingBeat.ys.resize(n);
            for (int i = 0; i < n; i++) mPendingBeat.ys[i] = mBuf[s0 + i];
            mBeat = mPendingBeat;
            mHaveBeat   = true;
            mHavePending = false;

            double peakMs = (mBeat.cPeakPos - mBeat.aPos) / mSps * 1000.0;
            mPeakHistory.append(peakMs);
            if (mPeakHistory.size() > kHistoryN) mPeakHistory.removeFirst();

            if (mBeat.cOnsetValid) {
                double onsetMs = (mBeat.cOnsetPos - mBeat.aPos) / mSps * 1000.0;
                mOnsetHistory.append(onsetMs);
                if (mOnsetHistory.size() > kHistoryN) mOnsetHistory.removeFirst();
            }

            if (!mPaused) redraw();
        } else if (start < mBufStartAbs) {
            mHavePending = false;  // too old, drop
        }
    }
}

void EscapementTab::redraw()
{
    if (!mHaveBeat) return;
    const Beat &b = mBeat;

    const int n = b.ys.size();
    QVector<double> xs(n);
    double aMs = (b.aPos - b.startAbs) / mSps * 1000.0;
    for (int i = 0; i < n; i++)
        xs[i] = (double)i / mSps * 1000.0 - aMs;        // x = 0 at A event
    mPlot->graph(0)->setData(xs, b.ys, true);
    mPlot->xAxis->setRange(xs.first(), xs.last());
    mPlot->yAxis->rescale();
    double yTop = mPlot->yAxis->range().upper;
    double yBot = mPlot->yAxis->range().lower;

    bool useOnset = mRefCombo->currentIndex() == 1 && b.cOnsetValid;
    double cAbs   = useOnset ? b.cOnsetPos : b.cPeakPos;
    double cMs    = (cAbs - b.aPos) / mSps * 1000.0;
    mCurrentMs    = cMs;

    auto placeMarker = [&](QCPItemLine *mk, QCPItemText *label,
                           double xMs, const QString &text) {
        mk->start->setCoords(xMs, 0.02);
        mk->end->setCoords(xMs, 0.98);
        mk->setVisible(true);
        label->position->setCoords(xMs, 0.04);
        label->setText(text);
        label->setVisible(true);
    };
    placeMarker(mAMarker, mALabel, 0.0, "A (T1)");
    placeMarker(mCMarker, mCLabel, cMs, useOnset ? "C onset (T3)" : "C peak (T3)");

    mDeltaText->position->setCoords(cMs / 2.0, 0.12);
    mDeltaText->setText(QString("Δ %1 ms").arg(cMs, 0, 'f', 2));
    mDeltaText->setVisible(true);

    mACRect->topLeft->setCoords(0.0, yTop);
    mACRect->bottomRight->setCoords(cMs, yBot);
    mACRect->setVisible(true);

    mDeltaLabel->setText(QString("A → C interval: <b>%1 ms</b>  (C reference: %2%3)")
                             .arg(cMs, 0, 'f', 2)
                             .arg(useOnset ? "onset" : "peak")
                             .arg(mRefCombo->currentIndex() == 1 && !b.cOnsetValid
                                      ? " — onset unavailable, using peak" : ""));

    auto sigma = [](const QVector<double> &v) -> double {
        if (v.size() < 2) return -1.0;
        double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        double var  = 0.0;
        for (double x : v) var += (x - mean) * (x - mean);
        return std::sqrt(var / (v.size() - 1));
    };
    double sPeak  = sigma(mPeakHistory);
    double sOnset = sigma(mOnsetHistory);
    if (sPeak >= 0 && sOnset >= 0) {
        QString winner = (sPeak <= sOnset) ? "Peak more stable" : "Onset more stable";
        mStabilityLabel->setText(
            QString("σ peak: %1 ms  |  σ onset: %2 ms  (N=%3)  →  %4")
                .arg(sPeak,  0, 'f', 3)
                .arg(sOnset, 0, 'f', 3)
                .arg(mPeakHistory.size())
                .arg(winner));
    } else if (sPeak >= 0) {
        mStabilityLabel->setText(
            QString("σ peak: %1 ms  (N=%2)  |  onset: not enough data")
                .arg(sPeak, 0, 'f', 3)
                .arg(mPeakHistory.size()));
    } else {
        mStabilityLabel->clear();
    }

    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void EscapementTab::reset()
{
    mBuf.clear();
    mHavePending = false;
    mHaveBeat    = false;
    mCurrentMs   = 0.0;
    mPeakHistory.clear();
    mOnsetHistory.clear();
    mPlot->graph(0)->data()->clear();
    for (auto *item : {mALabel, mCLabel, mDeltaText}) item->setVisible(false);
    mAMarker->setVisible(false);
    mCMarker->setVisible(false);
    mACRect->setVisible(false);
    mDeltaLabel->setText("Waiting for a complete beat (A → C)…");
    mStabilityLabel->clear();
    mPlot->replot();
}
