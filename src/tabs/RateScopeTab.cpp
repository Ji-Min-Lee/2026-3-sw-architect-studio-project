#include "RateScopeTab.h"
#include "ReplotCounter.h"
#include <QDebug>
#include <cmath>

#define ERROR_RATE_Y_SCALE    10
#define GRAPH_HISTORY_SECONDS 10

static inline double inwardMarkerLen(int sps)
{
    return 500.0 * (sps / 48000.0);
}

RateScopeTab::RateScopeTab(QCustomPlot *ratePlot, QCustomPlot *scopePlot, QWidget *parent)
    : BaseGraphTab(parent), mRatePlot(ratePlot), mScopePlot(scopePlot)
{
    mXTic.reserve(mMaxPoints); mXToc.reserve(mMaxPoints);
    mYTic.reserve(mMaxPoints); mYToc.reserve(mMaxPoints);
    setupPlots();
}

void RateScopeTab::setupPlots()
{
    QFont legendFont; legendFont.setPointSize(10);

    mScopePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mScopePlot->legend->setVisible(true);
    mScopePlot->legend->setFont(legendFont);
    mScopePlot->yAxis->setLabel("Amplitude");
    mScopePlot->xAxis->setLabel("");
    mScopePlot->yAxis->setRange(0, 0.1);
    mScopeTicker = QSharedPointer<ScopeTimeTicker>(new ScopeTimeTicker());
    mScopePlot->xAxis->setTicker(mScopeTicker);
    mScopePlot->xAxis->setTickLabels(true);
    mScopePlot->clearGraphs();
    mScopePlot->addGraph();
    QPen pen; pen.setWidth(1); pen.setColor(Qt::blue);
    mScopePlot->graph(0)->setPen(pen);
    mScopePlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    mScopePlot->graph(0)->setName("Rectified");
    mScopePlot->addGraph();
    pen.setColor(Qt::red);
    mScopePlot->graph(1)->setPen(pen);
    mScopePlot->graph(1)->setName("Trigger");
    mScopePlot->legend->setVisible(true);

    mRatePlot->legend->setVisible(true);
    mRatePlot->legend->setFont(legendFont);
    mRatePlot->yAxis->setLabel("Timing offset (ms)");
    mRatePlot->xAxis->setLabel("Beat count");
    mRatePlot->yAxis->setRange(-ERROR_RATE_Y_SCALE, ERROR_RATE_Y_SCALE);
    mRatePlot->xAxis->setRange(0, mMaxPoints);
    mRatePlot->xAxis->setTickLabels(true);
    mRatePlot->clearGraphs();
    // graph(0): Tic scatter
    mRatePlot->addGraph();
    mRatePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    mRatePlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mRatePlot->graph(0)->setPen(QPen(QColor(255, 80, 80)));
    mRatePlot->graph(0)->setName("Tic Rate");
    // graph(1): Toc scatter
    mRatePlot->addGraph();
    mRatePlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    mRatePlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    mRatePlot->graph(1)->setPen(QPen(QColor(80, 80, 255)));
    mRatePlot->graph(1)->setName("Toc Rate");
    // RS-1: graph(2): rolling average trend line (orange, 2px)
    mRatePlot->addGraph();
    QPen trendPen(QColor(0, 180, 120));  // teal-green, distinct from red/blue scatter
    trendPen.setWidth(2);
    mRatePlot->graph(2)->setPen(trendPen);
    mRatePlot->graph(2)->setLineStyle(QCPGraph::lsLine);
    mRatePlot->graph(2)->setScatterStyle(QCPScatterStyle::ssNone);
    mRatePlot->graph(2)->setName(QString("Trend (%1-beat avg)").arg(kTrendWindow));
    mRatePlot->legend->setVisible(true);
    mRatePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Click-to-sync: rate plot click → pan scope to that beat's sample position
    connect(mRatePlot, &QCustomPlot::mousePress,
            this, &RateScopeTab::onRatePlotClicked);
    // Click-to-sync: scope plot click → highlight corresponding beat on rate plot
    connect(mScopePlot, &QCustomPlot::mousePress,
            this, &RateScopeTab::onScopePlotClicked);

    // RS-2: statistics text label pinned to top-left of rate plot axes
    mStatsLabel = new QCPItemText(mRatePlot);
    mStatsLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
    mStatsLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    mStatsLabel->position->setCoords(0.01, 0.01);
    QFont statsFont("Monospace", 9);
    mStatsLabel->setFont(statsFont);
    mStatsLabel->setColor(Qt::black);
    mStatsLabel->setPen(QPen(QColor(180, 180, 180)));
    mStatsLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
    mStatsLabel->setPadding(QMargins(5, 3, 5, 3));
    mStatsLabel->setText("mean: —   σ: —   n: 0");
}

void RateScopeTab::reset()
{
    mXTicIdx = mXTocIdx = 0;
    mBeatX = 0;
    mXTic.clear(); mYTic.clear(); mXToc.clear(); mYToc.clear();
    mSampleTic.clear(); mSampleToc.clear();
    mRateCrosshair = nullptr;
    mHaveLastA = false;
    mStatCount = 0; mStatMean = 0.0; mStatM2 = 0.0;
    mRatePlot->yAxis->setRange(-ERROR_RATE_Y_SCALE, ERROR_RATE_Y_SCALE);
    mRatePlot->xAxis->setRange(0, mMaxPoints);
    for (int i = 0; i < mRatePlot->graphCount(); i++) mRatePlot->graph(i)->data()->clear();
    mRatePlot->clearItems();
    // Re-create stats label after clearItems() destroyed it
    mStatsLabel = new QCPItemText(mRatePlot);
    mStatsLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
    mStatsLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    mStatsLabel->position->setCoords(0.01, 0.01);
    mStatsLabel->setFont(QFont("Monospace", 9));
    mStatsLabel->setColor(Qt::black);
    mStatsLabel->setPen(QPen(QColor(180, 180, 180)));
    mStatsLabel->setBrush(QBrush(QColor(255, 255, 255, 200)));
    mStatsLabel->setPadding(QMargins(5, 3, 5, 3));
    mStatsLabel->setText("mean: —   σ: —   n: 0");
    { int64_t _pt=TG_NOW(); mRatePlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
    for (int i = 0; i < mScopePlot->graphCount(); i++) mScopePlot->graph(i)->data()->clear();
    mScopePlot->clearItems(); { int64_t _pt=TG_NOW(); mScopePlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}

// RS-1: recompute the rolling average trend line from all scatter points combined.
// Uses a sliding window of kTrendWindow samples over the merged tic+toc series
// sorted by x (beat index). Points outside the window are smoothed with the
// same window to keep the line continuous as the circular buffer wraps.
void RateScopeTab::updateTrendLine()
{
    // Merge tic and toc into one series sorted by x
    int total = mXTic.size() + mXToc.size();
    if (total < 2) {
        mRatePlot->graph(2)->data()->clear();
        return;
    }

    QVector<QPair<double,double>> pts;
    pts.reserve(total);
    for (int i = 0; i < mXTic.size(); ++i) pts.append({mXTic[i], mYTic[i]});
    for (int i = 0; i < mXToc.size(); ++i) pts.append({mXToc[i], mYToc[i]});
    std::sort(pts.begin(), pts.end(), [](const QPair<double,double> &a, const QPair<double,double> &b){
        return a.first < b.first;
    });

    QVector<double> xTrend, yTrend;
    xTrend.reserve(pts.size());
    yTrend.reserve(pts.size());

    for (int i = 0; i < pts.size(); ++i) {
        int lo = qMax(0, i - kTrendWindow / 2);
        int hi = qMin(pts.size() - 1, i + kTrendWindow / 2);
        double sum = 0.0;
        for (int j = lo; j <= hi; ++j) sum += pts[j].second;
        xTrend.append(pts[i].first);
        yTrend.append(sum / (hi - lo + 1));
    }

    mRatePlot->graph(2)->setData(xTrend, yTrend);
}

// RS-2: update mean ± σ label using Welford online algorithm accumulated over
// all rate points seen since last reset (both tic and toc combined).
void RateScopeTab::updateStatsOverlay()
{
    if (!mStatsLabel) return;
    if (mStatCount < 2) {
        mStatsLabel->setText(QString("mean: —   σ: —   n: %1").arg(mStatCount));
        return;
    }
    double sigma = std::sqrt(mStatM2 / (mStatCount - 1));
    mStatsLabel->setText(
        QString("mean: %1 ms   σ: %2 ms   n: %3")
            .arg(mStatMean, 0, 'f', 3)
            .arg(sigma,     0, 'f', 3)
            .arg(mStatCount)
    );
}

void RateScopeTab::onMeasurement(const Measurement &m)
{
    if (mPaused || !isVisible()) return;

    for (int i = 0; i < m.signal.pcm.size(); i++) {
        uint64_t tick = m.signal.tickStart + i;
        mScopePlot->graph(0)->addData((double)tick, m.signal.pcm[i]);
        mScopePlot->graph(1)->addData((double)tick, m.signal.threshold[i]);
    }

    bool rateUpdated = false;
    double markerLen = inwardMarkerLen(m.signal.samplesPerSecond);
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            addVerticalMarker(ev.samplePos, ev.peakValue, Qt::green);
            if (mHaveLastA) {
                double delta = ev.samplePos - mLastA;
                addHorizontalMarkerOutward(mLastA, ev.samplePos, ev.peakValue / 2.0, Qt::black);
                addText(mLastA + delta / 2.0, ev.peakValue / 2.0,
                        QString(" %1 ms ").arg(delta * 1000.0 / m.signal.samplesPerSecond, 0, 'f', 2),
                        Qt::black, Qt::AlignHCenter | Qt::AlignTop);
            }
            mLastA = ev.samplePos; mHaveLastA = true;

            if (ev.hasRatePoint) {
                QVector<double> &xv  = ev.isTic ? mXTic : mXToc;
                QVector<double> &yv  = ev.isTic ? mYTic : mYToc;
                QVector<double> &sv  = ev.isTic ? mSampleTic : mSampleToc;
                // Shared monotonic beat count as the x-coordinate so Tic and Toc
                // stay aligned even when ambient noise rejects one series more than
                // the other (separate per-series indices would otherwise drift, so
                // one colour ends up several beats ahead of the other).
                xv.append((double)mBeatX);
                yv.append(ev.wrappedRateError);
                sv.append((double)ev.samplePos);
                mBeatX++;
                // Sliding window: keep only the last mMaxPoints beats in both series.
                const double minX = (double)mBeatX - mMaxPoints;
                auto trim = [minX](QVector<double> &X, QVector<double> &Y, QVector<double> &S) {
                    while (!X.isEmpty() && X.first() < minX) { X.removeFirst(); Y.removeFirst(); S.removeFirst(); }
                };
                trim(mXTic, mYTic, mSampleTic);
                trim(mXToc, mYToc, mSampleToc);
                mRatePlot->graph(0)->setData(mXTic, mYTic);
                mRatePlot->graph(1)->setData(mXToc, mYToc);
                const int hi = qMax(mBeatX, mMaxPoints);
                mRatePlot->xAxis->setRange(hi - mMaxPoints, hi);

                // RS-2: Welford online update
                mStatCount++;
                double delta = ev.wrappedRateError - mStatMean;
                mStatMean += delta / mStatCount;
                mStatM2   += delta * (ev.wrappedRateError - mStatMean);

                rateUpdated = true;
            }
        } else {
            double delta = ev.samplePos - mLastA;
            QString txt;
            if (m.synced && m.metrics.amplitude && qRound(*m.metrics.amplitude) < 360)
                txt = QString(" %1 ms\n%2°")
                      .arg(delta * 1000.0 / m.signal.samplesPerSecond, 0, 'f', 1)
                      .arg(qRound(*m.metrics.amplitude));
            else
                txt = QString(" %1 ms ").arg(delta * 1000.0 / m.signal.samplesPerSecond, 0, 'f', 1);
            addVerticalMarker(ev.samplePos, ev.peakValue, Qt::red);
            addHorizontalMarkerInward(mLastA, ev.samplePos, markerLen, ev.peakValue, Qt::black);
            addText(ev.samplePos + markerLen, ev.peakValue, txt, Qt::black, Qt::AlignLeft | Qt::AlignTop);
        }
    }

    if (rateUpdated) {
        updateTrendLine();      // RS-1
        updateStatsOverlay();   // RS-2
        g_replotCount++;
        mRatePlot->replot(QCustomPlot::rpQueuedReplot);
    }

    purgeScopeHistory(m.signal.samplesPerSecond);
    double divisor = (mScopeScale > 0) ? mScopeScale : 4;
    if (m.signal.samplesPerSecond != mSamplesPerSecond) {
        mSamplesPerSecond = m.signal.samplesPerSecond;
        mScopeTicker->setSampleRate(m.signal.samplesPerSecond);
    }
    mLastTickEnd = (double)m.signal.tickEnd;
    mScopePlot->xAxis->setRange(mLastTickEnd, m.signal.samplesPerSecond / divisor, Qt::AlignRight);
    mScopePlot->yAxis->rescale();
    g_replotCount++;
    mScopePlot->replot(QCustomPlot::rpQueuedReplot);
}

void RateScopeTab::setScopeScale(int scale)
{
    mScopeScale = scale;
    if (mLastTickEnd > 0.0) {
        double divisor = (scale > 0) ? scale : 4;
        mScopePlot->xAxis->setRange(mLastTickEnd, mSamplesPerSecond / divisor, Qt::AlignRight);
        mScopePlot->replot();
    }
}

void RateScopeTab::replotAll()
{
    { int64_t _pt=TG_NOW(); mRatePlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); }; { int64_t _pt=TG_NOW(); mScopePlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}

void RateScopeTab::purgeScopeHistory(int sps)
{
    for (int i = 0; i < mScopePlot->graphCount(); i++) {
        auto data = mScopePlot->graph(i)->data();
        if (data->size() > GRAPH_HISTORY_SECONDS * sps) {
            bool found;
            QCPRange r = mScopePlot->graph(i)->getKeyRange(found, QCP::sdBoth);
            if (!found) continue;
            double removeEnd = r.lower + (r.upper - r.lower) - GRAPH_HISTORY_SECONDS * sps / 2.0;
            removeMarkersAndText(r.lower, removeEnd);
            data->remove(r.lower, removeEnd);
        }
    }
}

void RateScopeTab::addVerticalMarker(double x, double height, const QColor &color)
{
    QCPItemLine *line = new QCPItemLine(mScopePlot);
    line->start->setCoords(x, 0.0); line->end->setCoords(x, height);
    QPen pen; pen.setColor(color); pen.setWidth(2); pen.setStyle(Qt::DashLine);
    line->setPen(pen);
}
void RateScopeTab::addText(double x, double height, const QString &text,
                             const QColor &color, Qt::Alignment align)
{
    QCPItemText *textItem = new QCPItemText(mScopePlot);
    textItem->setColor(color);
    textItem->setPositionAlignment(align);
    textItem->position->setType(QCPItemPosition::ptPlotCoords);
    textItem->position->setCoords(x, height);
    textItem->setText(text);
    textItem->setPen(QPen(color));
}
void RateScopeTab::addHorizontalMarkerOutward(double xL, double xR, double h, const QColor &color)
{
    QCPItemLine *line = new QCPItemLine(mScopePlot);
    line->start->setCoords(xL, h); line->end->setCoords(xR, h);
    QPen pen; pen.setColor(color); pen.setWidth(1); pen.setStyle(Qt::SolidLine);
    line->setHead(QCPLineEnding::esSpikeArrow);
    line->setTail(QCPLineEnding::esSpikeArrow);
    line->setPen(pen);
}
void RateScopeTab::addHorizontalMarkerInward(double xL, double xR, double len,
                                              double h, const QColor &color)
{
    QPen pen; pen.setColor(color); pen.setWidth(1); pen.setStyle(Qt::SolidLine);
    QCPItemLine *left = new QCPItemLine(mScopePlot);
    left->start->setCoords(xL - len, h); left->end->setCoords(xL, h);
    left->setHead(QCPLineEnding::esSpikeArrow); left->setPen(pen);
    QCPItemLine *right = new QCPItemLine(mScopePlot);
    right->start->setCoords(xR, h); right->end->setCoords(xR + len, h);
    right->setTail(QCPLineEnding::esSpikeArrow); right->setPen(pen);
}
// Click-to-sync: rate plot clicked → pan scope to the sample at the nearest beat
void RateScopeTab::onRatePlotClicked(QMouseEvent *event)
{
    double beatX = mRatePlot->xAxis->pixelToCoord(event->pos().x());
    syncScopeToRateBeat(beatX);
}

// Click-to-sync: scope plot clicked → highlight nearest beat on rate plot
void RateScopeTab::onScopePlotClicked(QMouseEvent *event)
{
    double sampleX = mScopePlot->xAxis->pixelToCoord(event->pos().x());
    syncRateToBeatNearSample(sampleX);
}

// Pan the scope to center on the beat whose x-index (in rate plot) is nearest to beatX.
void RateScopeTab::syncScopeToRateBeat(double beatX)
{
    // Find the closest stored beat index across tic and toc
    double bestDist = std::numeric_limits<double>::max();
    double bestSample = -1.0;

    auto search = [&](const QVector<double> &xv, const QVector<double> &sv) {
        for (int i = 0; i < xv.size(); ++i) {
            double d = std::abs(xv[i] - beatX);
            if (d < bestDist) { bestDist = d; bestSample = sv[i]; }
        }
    };
    search(mXTic, mSampleTic);
    search(mXToc, mSampleToc);

    if (bestSample < 0.0) return;

    double half = (mSamplesPerSecond / (mScopeScale > 0 ? mScopeScale : 4)) / 2.0;
    mScopePlot->xAxis->setRange(bestSample - half, bestSample + half);
    mScopePlot->replot();
}

// Draw a vertical crosshair on the rate plot at the beat closest to sampleX.
void RateScopeTab::syncRateToBeatNearSample(double sampleX)
{
    double bestDist = std::numeric_limits<double>::max();
    double bestBeatX = -1.0;

    auto search = [&](const QVector<double> &xv, const QVector<double> &sv) {
        for (int i = 0; i < sv.size(); ++i) {
            double d = std::abs(sv[i] - sampleX);
            if (d < bestDist) { bestDist = d; bestBeatX = xv[i]; }
        }
    };
    search(mXTic, mSampleTic);
    search(mXToc, mSampleToc);

    if (bestBeatX < 0.0) return;

    // Remove old crosshair and draw a new one
    if (mRateCrosshair) {
        mRatePlot->removeItem(mRateCrosshair);
        mRateCrosshair = nullptr;
    }
    mRateCrosshair = new QCPItemLine(mRatePlot);
    QPen pen(QColor(255, 140, 0));
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    mRateCrosshair->setPen(pen);
    mRateCrosshair->start->setType(QCPItemPosition::ptPlotCoords);
    mRateCrosshair->start->setCoords(bestBeatX, -ERROR_RATE_Y_SCALE);
    mRateCrosshair->end->setType(QCPItemPosition::ptPlotCoords);
    mRateCrosshair->end->setCoords(bestBeatX, ERROR_RATE_Y_SCALE);
    mRatePlot->replot();
}

void RateScopeTab::removeMarkersAndText(double lo, double hi)
{
    for (int i = mScopePlot->itemCount() - 1; i >= 0; --i) {
        auto *item = mScopePlot->item(i);
        if (auto *line = qobject_cast<QCPItemLine *>(item)) {
            double startKey = line->start->coords().x(), endKey = line->end->coords().x();
            if ((startKey >= lo && startKey <= hi) || (endKey >= lo && endKey <= hi)) mScopePlot->removeItem(line);
        } else if (auto *textItem = qobject_cast<QCPItemText *>(item)) {
            double textX = textItem->position->coords().x();
            if (textX >= lo && textX <= hi) mScopePlot->removeItem(textItem);
        }
    }
}
