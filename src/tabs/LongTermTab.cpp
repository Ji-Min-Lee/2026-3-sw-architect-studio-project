#include "LongTermTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>

LongTermTab::LongTermTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    mSummaryLabel = new QLabel("—", this);
    mSummaryLabel->setAlignment(Qt::AlignHCenter);
    lay->addWidget(mSummaryLabel);
    lay->addWidget(mPlot, 1);
    setLayout(lay);

    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mPlot->legend->setVisible(false);

    mRate = makeSeries(0, "Rate (s/d)",      QColor(170, 30, 90), true);
    mAmp  = makeSeries(1, "Amplitude (°)",   QColor(40, 70, 200), false);
    mBeat = makeSeries(2, "Beat Error (ms)", QColor(30, 140, 60), false);
    mBeat.rect->axis(QCPAxis::atBottom)->setLabel("Time (s)");

    // Synchronise all three X-axes so every subplot always shows the same time window.
    QCPAxis *xRate = mRate.rect->axis(QCPAxis::atBottom);
    QCPAxis *xAmp  = mAmp.rect->axis(QCPAxis::atBottom);
    QCPAxis *xBeat = mBeat.rect->axis(QCPAxis::atBottom);
    connect(xRate, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
            xAmp,  QOverload<const QCPRange&>::of(&QCPAxis::setRange));
    connect(xRate, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
            xBeat, QOverload<const QCPRange&>::of(&QCPAxis::setRange));

    // Tolerance / acceptance reference lines (project plan Figure 14).
    addTolLine(mRate,  5.0);   // Rate upper limit  +5 s/d
    addTolLine(mRate, -5.0);   // Rate lower limit  -5 s/d
    addTolLine(mAmp,  310.0);  // Amplitude upper   310°
    addTolLine(mAmp,  270.0);  // Amplitude lower   270°
    addTolLine(mBeat,   0.6);  // Beat Error limit  0.6 ms
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

void LongTermTab::addTolLine(Series &s, double yVal)
{
    QCPAxis *x = s.rect->axis(QCPAxis::atBottom);
    QCPAxis *y = s.rect->axis(QCPAxis::atLeft);
    auto *line = new QCPItemLine(mPlot);
    line->setClipAxisRect(s.rect);
    line->start->setAxes(x, y);
    line->end->setAxes(x, y);
    line->start->setTypeX(QCPItemPosition::ptAxisRectRatio);
    line->end->setTypeX(QCPItemPosition::ptAxisRectRatio);
    line->start->setCoords(0.0, yVal);
    line->end->setCoords(1.0, yVal);
    line->setPen(QPen(QColor(160, 160, 160), 1, Qt::DotLine));
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
    updateOverlay(s);  // update mean/σ only when a bucket is complete
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
    mSummaryLabel->setText("—");
    { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}

void LongTermTab::onMeasurement(const Measurement &m)
{
    if (!m.rateValid && !m.amplitudeValid && !m.beatErrorValid) return;
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;

    if (m.rateValid)      addPoint(mRate, mTimeElapsed, m.rateErrorSpd);
    if (m.amplitudeValid) addPoint(mAmp,  mTimeElapsed, m.amplitudeDeg);
    if (m.beatErrorValid) addPoint(mBeat, mTimeElapsed, m.beatErrorMs);

    // Reduce update frequency as elapsed time grows (time-based thresholds).
    int newBucket = 1;
    if      (mTimeElapsed > 7200) newBucket = 60;  // > 2 hours
    else if (mTimeElapsed > 1800) newBucket = 30;  // > 30 min
    else if (mTimeElapsed > 300)  newBucket = 10;  // > 5 min
    mBucketSize = newBucket;

    // Update summary header
    {
        auto fmt = [](const Series &s, int dec) {
            return s.n ? QString::number(s.mean(), 'f', dec) : QString("—");
        };
        QString granularity = (mBucketSize == 1)
            ? "live"
            : QString("%1 meas/pt").arg(mBucketSize);
        mSummaryLabel->setText(
            QString("Rate: %1 s/d   Amp: %2°   Beat: %3 ms   |   Granularity: %4")
                .arg(fmt(mRate, 1)).arg(fmt(mAmp, 0)).arg(fmt(mBeat, 2)).arg(granularity));
    }

    if (mPaused || !isVisible()) return;
    // Rescale X only on the primary axis — linked axes follow automatically.
    mRate.rect->axis(QCPAxis::atBottom)->rescale();
    for (Series *s : {&mRate, &mAmp, &mBeat}) {
        s->rect->axis(QCPAxis::atLeft)->rescale();
        s->rect->axis(QCPAxis::atLeft)->scaleRange(1.15);  // 15% padding so lines aren't clipped
    }
    g_replotCount++;
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void LongTermTab::replotAll() { { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); }; }
