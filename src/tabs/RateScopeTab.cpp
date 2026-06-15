#include "RateScopeTab.h"
#include "ReplotCounter.h"
#include <QDebug>

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
    mScopePlot->xAxis->setLabel("Time");
    mScopePlot->yAxis->setRange(0, 0.1);
    mScopePlot->xAxis->setTickLabels(false);
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
    mRatePlot->yAxis->setLabel("Rate Error (milliseconds)");
    mRatePlot->xAxis->setLabel("Time");
    mRatePlot->yAxis->setRange(-ERROR_RATE_Y_SCALE, ERROR_RATE_Y_SCALE);
    mRatePlot->xAxis->setRange(0, mMaxPoints);
    mRatePlot->xAxis->setTickLabels(false);
    mRatePlot->clearGraphs();
    mRatePlot->addGraph();
    mRatePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    mRatePlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    mRatePlot->graph(0)->setPen(QPen(Qt::red));
    mRatePlot->graph(0)->setName("Tic Rate");
    mRatePlot->addGraph();
    mRatePlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    mRatePlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    mRatePlot->graph(1)->setPen(QPen(Qt::blue));
    mRatePlot->graph(1)->setName("Toc Rate");
    mRatePlot->legend->setVisible(true);
}

void RateScopeTab::reset()
{
    mXTicIdx = mXTocIdx = 0;
    mXTic.clear(); mYTic.clear(); mXToc.clear(); mYToc.clear();
    mHaveLastA = false;
    mRatePlot->yAxis->setRange(-ERROR_RATE_Y_SCALE, ERROR_RATE_Y_SCALE);
    mRatePlot->xAxis->setRange(0, mMaxPoints);
    for (int i = 0; i < mRatePlot->graphCount(); i++) mRatePlot->graph(i)->data()->clear();
    mRatePlot->clearItems(); mRatePlot->replot();
    for (int i = 0; i < mScopePlot->graphCount(); i++) mScopePlot->graph(i)->data()->clear();
    mScopePlot->clearItems(); mScopePlot->replot();
}

void RateScopeTab::onMeasurement(const Measurement &m)
{
    if (!isVisible()) return;

    for (int i = 0; i < m.pcm.size(); i++) {
        uint64_t tick = m.graphTickStart + i;
        mScopePlot->graph(0)->addData((double)tick, m.pcm[i]);
        mScopePlot->graph(1)->addData((double)tick, m.threshold[i]);
    }

    double markerLen = inwardMarkerLen(m.samplesPerSecond);
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            addVerticalMarker(ev.samplePos, ev.peakValue, Qt::green);
            if (mHaveLastA) {
                double delta = ev.samplePos - mLastA;
                addHorizontalMarkerOutward(mLastA, ev.samplePos, ev.peakValue / 2.0, Qt::black);
                addText(mLastA + delta / 2.0, ev.peakValue / 2.0,
                        QString(" %1 ms ").arg(delta * 1000.0 / m.samplesPerSecond, 0, 'f', 2),
                        Qt::black, Qt::AlignHCenter | Qt::AlignTop);
            }
            mLastA = ev.samplePos; mHaveLastA = true;

            if (ev.hasRatePoint) {
                int g = ev.isTic ? 0 : 1;
                QVector<double> &xv = ev.isTic ? mXTic : mXToc;
                QVector<double> &yv = ev.isTic ? mYTic : mYToc;
                int             &idx = ev.isTic ? mXTicIdx : mXTocIdx;
                if (yv.size() < mMaxPoints) { yv.append(ev.wrappedRateError); xv.append(idx); }
                else { yv[idx] = ev.wrappedRateError; }
                idx = (idx + 1) % mMaxPoints;
                mRatePlot->graph(g)->setData(xv, yv);
                g_replotCount++;
                mRatePlot->replot(QCustomPlot::rpQueuedReplot);
            }
        } else {
            double delta = ev.samplePos - mLastA;
            QString txt;
            if (m.synced && m.amplitudeValid && qRound(m.amplitudeDeg) < 360)
                txt = QString(" %1 ms\n%2°")
                      .arg(delta * 1000.0 / m.samplesPerSecond, 0, 'f', 1)
                      .arg(qRound(m.amplitudeDeg));
            else
                txt = QString(" %1 ms ").arg(delta * 1000.0 / m.samplesPerSecond, 0, 'f', 1);
            addVerticalMarker(ev.samplePos, ev.peakValue, Qt::red);
            addHorizontalMarkerInward(mLastA, ev.samplePos, markerLen, ev.peakValue, Qt::black);
            addText(ev.samplePos + markerLen, ev.peakValue, txt, Qt::black, Qt::AlignLeft | Qt::AlignTop);
        }
    }

    purgeScopeHistory(m.samplesPerSecond);
    double divisor = (mScopeScale > 0) ? mScopeScale : 4;
    mScopePlot->xAxis->setRange((double)m.graphTickEnd, m.samplesPerSecond / divisor, Qt::AlignRight);
    mScopePlot->yAxis->rescale();
    g_replotCount++;
    mScopePlot->replot(QCustomPlot::rpQueuedReplot);
}

void RateScopeTab::replotAll()
{
    mRatePlot->replot(); mScopePlot->replot();
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
    QCPItemLine *l = new QCPItemLine(mScopePlot);
    l->start->setCoords(x, 0.0); l->end->setCoords(x, height);
    QPen pen; pen.setColor(color); pen.setWidth(2); pen.setStyle(Qt::DashLine);
    l->setPen(pen);
}
void RateScopeTab::addText(double x, double height, const QString &text,
                             const QColor &color, Qt::Alignment align)
{
    QCPItemText *t = new QCPItemText(mScopePlot);
    t->setColor(color);
    t->setPositionAlignment(align);
    t->position->setType(QCPItemPosition::ptPlotCoords);
    t->position->setCoords(x, height);
    t->setText(text);
    t->setPen(QPen(color));
}
void RateScopeTab::addHorizontalMarkerOutward(double xL, double xR, double h, const QColor &c)
{
    QCPItemLine *l = new QCPItemLine(mScopePlot);
    l->start->setCoords(xL, h); l->end->setCoords(xR, h);
    QPen pen; pen.setColor(c); pen.setWidth(1); pen.setStyle(Qt::SolidLine);
    l->setHead(QCPLineEnding::esSpikeArrow);
    l->setTail(QCPLineEnding::esSpikeArrow);
    l->setPen(pen);
}
void RateScopeTab::addHorizontalMarkerInward(double xL, double xR, double len,
                                              double h, const QColor &c)
{
    QPen pen; pen.setColor(c); pen.setWidth(1); pen.setStyle(Qt::SolidLine);
    QCPItemLine *left = new QCPItemLine(mScopePlot);
    left->start->setCoords(xL - len, h); left->end->setCoords(xL, h);
    left->setHead(QCPLineEnding::esSpikeArrow); left->setPen(pen);
    QCPItemLine *right = new QCPItemLine(mScopePlot);
    right->start->setCoords(xR, h); right->end->setCoords(xR + len, h);
    right->setTail(QCPLineEnding::esSpikeArrow); right->setPen(pen);
}
void RateScopeTab::removeMarkersAndText(double lo, double hi)
{
    for (int i = mScopePlot->itemCount() - 1; i >= 0; --i) {
        auto *item = mScopePlot->item(i);
        if (auto *l = qobject_cast<QCPItemLine *>(item)) {
            double sk = l->start->coords().x(), ek = l->end->coords().x();
            if ((sk >= lo && sk <= hi) || (ek >= lo && ek <= hi)) mScopePlot->removeItem(l);
        } else if (auto *t = qobject_cast<QCPItemText *>(item)) {
            double k = t->position->coords().x();
            if (k >= lo && k <= hi) mScopePlot->removeItem(t);
        }
    }
}
