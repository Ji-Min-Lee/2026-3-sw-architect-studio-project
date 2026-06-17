#include "LongTermTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>

LongTermTab::LongTermTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mSummaryLabel = new QLabel("—", this);
    mSummaryLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(mSummaryLabel);
    mainLayout->addWidget(mPlot, 1);
    setLayout(mainLayout);

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
                                            const QColor &color, bool firstUsesDefaultRect)
{
    Series series;
    if (firstUsesDefaultRect) {
        series.rect  = mPlot->axisRect(0);
        series.graph = mPlot->addGraph();           // graph(0) on the default axes
    } else {
        series.rect = new QCPAxisRect(mPlot);
        mPlot->plotLayout()->addElement(row, 0, series.rect);
        series.graph = mPlot->addGraph(series.rect->axis(QCPAxis::atBottom),
                                  series.rect->axis(QCPAxis::atLeft));
    }
    QPen pen(color); pen.setWidth(1);
    series.graph->setPen(pen);
    series.graph->setName(name);
    series.rect->axis(QCPAxis::atLeft)->setLabel(name);

    QCPAxis *xAxis = series.rect->axis(QCPAxis::atBottom);
    QCPAxis *yAxis = series.rect->axis(QCPAxis::atLeft);

    series.band = new QCPItemRect(mPlot);
    series.band->setClipAxisRect(series.rect);
    series.band->topLeft->setAxes(xAxis, yAxis);
    series.band->bottomRight->setAxes(xAxis, yAxis);
    series.band->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
    series.band->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
    series.band->setBrush(QBrush(QColor(color.red(), color.green(), color.blue(), 36)));
    series.band->setPen(Qt::NoPen);
    series.band->setVisible(false);

    series.meanLine = new QCPItemLine(mPlot);
    series.meanLine->setClipAxisRect(series.rect);
    series.meanLine->start->setAxes(xAxis, yAxis);
    series.meanLine->end->setAxes(xAxis, yAxis);
    series.meanLine->start->setTypeX(QCPItemPosition::ptAxisRectRatio);
    series.meanLine->end->setTypeX(QCPItemPosition::ptAxisRectRatio);
    series.meanLine->setPen(QPen(color, 1, Qt::DashLine));
    series.meanLine->setVisible(false);
    return series;
}

void LongTermTab::addTolLine(Series &series, double yVal)
{
    QCPAxis *xAxis = series.rect->axis(QCPAxis::atBottom);
    QCPAxis *yAxis = series.rect->axis(QCPAxis::atLeft);
    auto *line = new QCPItemLine(mPlot);
    line->setClipAxisRect(series.rect);
    line->start->setAxes(xAxis, yAxis);
    line->end->setAxes(xAxis, yAxis);
    line->start->setTypeX(QCPItemPosition::ptAxisRectRatio);
    line->end->setTypeX(QCPItemPosition::ptAxisRectRatio);
    line->start->setCoords(0.0, yVal);
    line->end->setCoords(1.0, yVal);
    line->setPen(QPen(QColor(160, 160, 160), 1, Qt::DotLine));
}

void LongTermTab::updateOverlay(Series &series)
{
    if (series.n < 2) return;
    double mean = series.mean(), sd = series.sigma();
    series.meanLine->start->setCoords(0.0, mean);
    series.meanLine->end->setCoords(1.0, mean);
    series.meanLine->setVisible(true);
    series.band->topLeft->setCoords(0.0, mean + sd);
    series.band->bottomRight->setCoords(1.0, mean - sd);
    series.band->setVisible(true);
}

void LongTermTab::addPoint(Series &series, double timeSec, double value)
{
    series.addRunning(value);

    // Aggregate into buckets so long sessions stay readable and cheap
    series.bucketSum += value;
    series.bucketN++;
    if (series.bucketN < mBucketSize) return;
    series.graph->addData(timeSec, series.bucketSum / series.bucketN);
    series.bucketSum = 0;
    series.bucketN   = 0;
    updateOverlay(series);  // update mean/σ only when a bucket is complete
}

void LongTermTab::reset()
{
    for (Series *series : {&mRate, &mAmp, &mBeat}) {
        series->graph->data()->clear();
        series->meanLine->setVisible(false);
        series->band->setVisible(false);
        series->sum = series->sumSq = 0; series->n = 0;
        series->bucketSum = 0; series->bucketN = 0;
    }
    mTimeElapsed = 0.0;
    mBucketSize  = 1;
    mSummaryLabel->setText("—");
    { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}

void LongTermTab::onMeasurement(const Measurement &m)
{
    if (!m.metrics.rate.has_value() && !m.metrics.amplitude.has_value() && !m.metrics.beatError.has_value()) return;
    mTimeElapsed += (double)m.signal.pcm.size() / m.signal.samplesPerSecond;

    if (m.metrics.rate.has_value())      addPoint(mRate, mTimeElapsed, *m.metrics.rate);
    if (m.metrics.amplitude.has_value()) addPoint(mAmp,  mTimeElapsed, *m.metrics.amplitude);
    if (m.metrics.beatError.has_value()) addPoint(mBeat, mTimeElapsed, *m.metrics.beatError);

    // Reduce update frequency as elapsed time grows (time-based thresholds).
    int newBucket = 1;
    if      (mTimeElapsed > 7200) newBucket = 60;  // > 2 hours
    else if (mTimeElapsed > 1800) newBucket = 30;  // > 30 min
    else if (mTimeElapsed > 300)  newBucket = 10;  // > 5 min
    mBucketSize = newBucket;

    // Update summary header
    {
        auto fmt = [](const Series &series, int dec) {
            return series.n ? QString::number(series.mean(), 'f', dec) : QString("—");
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
    for (Series *series : {&mRate, &mAmp, &mBeat}) {
        series->rect->axis(QCPAxis::atLeft)->rescale();
        series->rect->axis(QCPAxis::atLeft)->scaleRange(1.15);  // 15% padding so lines aren't clipped
    }
    g_replotCount++;
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void LongTermTab::replotAll() { { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); }; }
