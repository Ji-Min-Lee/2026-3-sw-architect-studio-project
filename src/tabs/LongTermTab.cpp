#include "LongTermTab.h"
#include <QVBoxLayout>

LongTermTab::LongTermTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mPlot->legend->setVisible(true);

    mRate = makeSeries(0, "Rate (s/d)",      QColor(170, 30, 90), true);
    mAmp  = makeSeries(1, "Amplitude (°)",   QColor(40, 70, 200), false);
    mBeat = makeSeries(2, "Beat Error (ms)", QColor(30, 140, 60), false);
    mBeat.rect->axis(QCPAxis::atBottom)->setLabel("Time (s)");
}

LongTermTab::Series LongTermTab::makeSeries(int row, const QString &name,
                                            const QColor &c, bool firstUsesDefaultRect)
{
    Series s;
    if (firstUsesDefaultRect) {
        s.rect  = mPlot->axisRect(0);
        s.graph = mPlot->addGraph();           // graph(0) on the default axes
    } else {
        s.rect = new QCPAxisRect(mPlot);
        mPlot->plotLayout()->addElement(row, 0, s.rect);
        s.graph = mPlot->addGraph(s.rect->axis(QCPAxis::atBottom),
                                  s.rect->axis(QCPAxis::atLeft));
    }
    QPen pen(c); pen.setWidth(1);
    s.graph->setPen(pen);
    s.graph->setName(name);
    s.rect->axis(QCPAxis::atLeft)->setLabel(name);

    QCPAxis *x = s.rect->axis(QCPAxis::atBottom);
    QCPAxis *y = s.rect->axis(QCPAxis::atLeft);

    s.band = new QCPItemRect(mPlot);
    s.band->setClipAxisRect(s.rect);
    s.band->topLeft->setAxes(x, y);
    s.band->bottomRight->setAxes(x, y);
    s.band->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
    s.band->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
    s.band->setBrush(QBrush(QColor(c.red(), c.green(), c.blue(), 36)));
    s.band->setPen(Qt::NoPen);
    s.band->setVisible(false);

    s.meanLine = new QCPItemLine(mPlot);
    s.meanLine->setClipAxisRect(s.rect);
    s.meanLine->start->setAxes(x, y);
    s.meanLine->end->setAxes(x, y);
    s.meanLine->start->setTypeX(QCPItemPosition::ptAxisRectRatio);
    s.meanLine->end->setTypeX(QCPItemPosition::ptAxisRectRatio);
    s.meanLine->setPen(QPen(c, 1, Qt::DashLine));
    s.meanLine->setVisible(false);
    return s;
}

void LongTermTab::updateOverlay(Series &s)
{
    if (s.n < 2) return;
    double m = s.mean(), sd = s.sigma();
    s.meanLine->start->setCoords(0.0, m);
    s.meanLine->end->setCoords(1.0, m);
    s.meanLine->setVisible(true);
    s.band->topLeft->setCoords(0.0, m + sd);
    s.band->bottomRight->setCoords(1.0, m - sd);
    s.band->setVisible(true);
}

void LongTermTab::addPoint(Series &s, double t, double v)
{
    s.addRunning(v);

    // Aggregate into buckets so long sessions stay readable and cheap
    s.bucketSum += v;
    s.bucketN++;
    if (s.bucketN < mBucketSize) return;
    s.graph->addData(t, s.bucketSum / s.bucketN);
    s.bucketSum = 0;
    s.bucketN   = 0;
    mTotalPoints++;
}

void LongTermTab::reset()
{
    for (Series *s : {&mRate, &mAmp, &mBeat}) {
        s->graph->data()->clear();
        s->meanLine->setVisible(false);
        s->band->setVisible(false);
        s->sum = s->sumSq = 0; s->n = 0;
        s->bucketSum = 0; s->bucketN = 0;
    }
    mTimeElapsed = 0.0;
    mBucketSize  = 1;
    mTotalPoints = 0;
    mPlot->replot();
}

void LongTermTab::onMeasurement(const Measurement &m)
{
    if (!m.rateValid && !m.amplitudeValid && !m.beatErrorValid) return;
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;

    if (m.rateValid)      addPoint(mRate, mTimeElapsed, m.rateErrorSpd);
    if (m.amplitudeValid) addPoint(mAmp,  mTimeElapsed, m.amplitudeDeg);
    if (m.beatErrorValid) addPoint(mBeat, mTimeElapsed, m.beatErrorMs);

    // Reduce update frequency as elapsed time grows
    if (mTotalPoints > 3 * kMaxPointsBeforeCoarsen) {
        mBucketSize *= 2;
        mTotalPoints = 0;
    }

    if (mPaused) return;
    for (Series *s : {&mRate, &mAmp, &mBeat}) {
        updateOverlay(*s);
        s->rect->axis(QCPAxis::atBottom)->rescale();
        s->rect->axis(QCPAxis::atLeft)->rescale();
    }
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
