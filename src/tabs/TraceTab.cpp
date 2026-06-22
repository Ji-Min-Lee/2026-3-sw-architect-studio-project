#include "TraceTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
constexpr double kAmpBandLo  = 270.0;  // strong zone — WatchDiagnostics Excellent threshold
constexpr double kAmpBandHi  = 310.0;  // strong zone upper bound
constexpr double kAmpWarnLo  = 220.0;  // acceptable zone — WatchDiagnostics Good threshold
constexpr double kLateThresh = -3.0;   // engine-smoothed s/d below this → "running late"
constexpr double kRateSpanLo = -20.0;  // minimum visible rate range
constexpr double kRateSpanHi = +20.0;
constexpr double kAmpSpanLo  = 180.0;  // minimum visible amplitude range
constexpr double kAmpSpanHi  = 330.0;
} // namespace

TraceTab::TraceTab(QWidget *parent)
    : BaseGraphTab(parent)
    , mPlot(new QCustomPlot(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *top = new QHBoxLayout;
    top->setContentsMargins(4, 2, 4, 0);
    top->addWidget(new QLabel("Window:", this));
    mZoomCombo = new QComboBox(this);
    mZoomCombo->addItems({"10 min (1×)", "5 min (2×)", "2.5 min (4×)", "75 s (8×)", "30 s"});
    mZoomCombo->setCurrentIndex(4);  // short window default: scrolling visible with ~45 s test files
    top->addWidget(mZoomCombo);
    mAlertLabel = new QLabel(this);
    mAlertLabel->setAlignment(Qt::AlignHCenter);
    top->addWidget(mAlertLabel, 1);
    layout->addLayout(top);

    layout->addWidget(mPlot, 1);


    // Top rect (default axes): rate — graph(0).
    // The value is the engine's Averaging-Period rolling average, i.e. the
    // smoothing function required by the spec (user-adjustable in the panel).
    mPlot->addGraph();
    QPen pen; pen.setColor(QColor(0, 120, 215)); pen.setWidth(2);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Rate Error (s/day, smoothed by Averaging Period)");

    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Rate Error (s/day)");
    mPlot->yAxis->setRange(kRateSpanLo, kRateSpanHi);
    mPlot->legend->setVisible(false);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Bottom rect: amplitude with normal band
    mAmpRect = new QCPAxisRect(mPlot);
    mPlot->plotLayout()->addElement(2, 0, mAmpRect);
    mAmpGraph = mPlot->addGraph(mAmpRect->axis(QCPAxis::atBottom),
                                mAmpRect->axis(QCPAxis::atLeft));
    mAmpGraph->setPen(QPen(QColor(20, 130, 60), 2));
    mAmpGraph->setName("Amplitude (°)");
    mAmpRect->axis(QCPAxis::atBottom)->setLabel("Time (s)");
    mAmpRect->axis(QCPAxis::atLeft)->setLabel("Amplitude (°)");
    mAmpRect->axis(QCPAxis::atLeft)->setRange(kAmpSpanLo, kAmpSpanHi);

    // Three amplitude zone bands — rendered on "grid" layer so they sit behind
    // the data line.  Labels float at the left edge of each band.
    auto addAmpBand = [&](double yLo, double yHi, QColor color) {
        auto *b = new QCPItemRect(mPlot);
        b->setLayer("grid");
        b->setClipAxisRect(mAmpRect);
        b->topLeft->setAxes(mAmpRect->axis(QCPAxis::atBottom), mAmpRect->axis(QCPAxis::atLeft));
        b->bottomRight->setAxes(mAmpRect->axis(QCPAxis::atBottom), mAmpRect->axis(QCPAxis::atLeft));
        b->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
        b->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
        b->topLeft->setCoords(0.0, yHi);
        b->bottomRight->setCoords(1.0, yLo);
        b->setBrush(QBrush(color));
        b->setPen(Qt::NoPen);
    };
    // red: 180–220° (needs service)
    addAmpBand(kAmpSpanLo, kAmpWarnLo, QColor(210,  70,  70,  55));
    // amber: 220–270° (acceptable, may suggest wear)
    addAmpBand(kAmpWarnLo, kAmpBandLo, QColor(225, 160,   0,  60));
    // green: 270–310° (strong)
    addAmpBand(kAmpBandLo, kAmpBandHi, QColor(120, 215, 120,  80));

    // Inset legend — zone color swatches replace the floating text labels
    auto *ampLegend = new QCPLegend;
    mAmpRect->insetLayout()->addElement(ampLegend, Qt::AlignRight | Qt::AlignTop);
    ampLegend->setLayer("axes");
    { QFont lf; lf.setPointSize(8); ampLegend->setFont(lf); }
    ampLegend->setBorderPen(QPen(QColor(180, 180, 180)));
    ampLegend->setBrush(QBrush(QColor(255, 255, 255, 210)));
    ampLegend->setIconSize(QSize(14, 10));
    ampLegend->setIconTextPadding(4);
    ampLegend->setMargins(QMargins(4, 3, 4, 3));
    auto addZoneLegendItem = [&](const QString &name, QColor fill, QColor outline) {
        auto *g = mPlot->addGraph(mAmpRect->axis(QCPAxis::atBottom),
                                  mAmpRect->axis(QCPAxis::atLeft));
        g->setName(name);
        g->setPen(QPen(outline, 1));
        g->setBrush(QBrush(fill));
        ampLegend->addItem(new QCPPlottableLegendItem(ampLegend, g));
    };
    addZoneLegendItem("270–310°  Strong",       QColor(120, 215, 120, 180), QColor( 20, 110,  20));
    addZoneLegendItem("220–270°  Acceptable",   QColor(225, 160,   0, 180), QColor(140, 100,   0));
    addZoneLegendItem("< 220°  Needs service",  QColor(210,  70,  70, 180), QColor(160,  40,  40));

    // Summary text elements inside the plot, right below each respective graph
    mRateSummary = new QCPTextElement(mPlot);
    { QFont f; f.setPointSize(8); mRateSummary->setFont(f); }
    mRateSummary->setMargins(QMargins(4, 1, 4, 2));
    mRateSummary->setTextFlags(Qt::AlignHCenter | Qt::AlignVCenter);
    mPlot->plotLayout()->addElement(1, 0, mRateSummary);

    mAmpSummary = new QCPTextElement(mPlot);
    { QFont f; f.setPointSize(8); mAmpSummary->setFont(f); }
    mAmpSummary->setMargins(QMargins(4, 1, 4, 2));
    mAmpSummary->setTextFlags(Qt::AlignHCenter | Qt::AlignVCenter);
    mPlot->plotLayout()->addElement(3, 0, mAmpSummary);

    // Row 0=rate graph, 1=rate note, 2=amp graph, 3=amp note
    mPlot->plotLayout()->setRowStretchFactor(0, 4);
    mPlot->plotLayout()->setRowStretchFactor(1, 0);
    mPlot->plotLayout()->setRowStretchFactor(2, 3);
    mPlot->plotLayout()->setRowStretchFactor(3, 0);

    connect(mZoomCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { if (!mPaused) { updateRanges(); { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); }; } });

    updateAlerts();
}

double TraceTab::windowSec() const
{
    static const double zoom[] = {1, 2, 4, 8, 20};  // 20× = 30 s (demo/test files)
    return kBaseWindowSec / zoom[qBound(0, mZoomCombo->currentIndex(), 4)];
}

void TraceTab::reset()
{
    mTimeElapsed = 0.0;
    mRateSum = 0; mRateN = 0;
    mAmpSum  = 0; mAmpN  = 0;
    mRateRecent.clear();
    mAmpRecent.clear();
    mHaveRate = false;
    mHaveAmp  = false;
    mRateMin = mRateMax = 0;
    mAmpMin  = mAmpMax  = 0;
    mPlot->graph(0)->data()->clear();
    mAmpGraph->data()->clear();
    updateAlerts();
    updateRanges();
    { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}

double TraceTab::rollingAvg(const QVector<QPair<double, double>> &buf) const
{
    if (buf.isEmpty()) return 0.0;
    double runningSum = 0;
    for (const auto &entry : buf) runningSum += entry.second;
    return runningSum / buf.size();
}

void TraceTab::updateAlerts()
{
    QStringList alerts;
    if (mHaveRate && mLastRate < kLateThresh)
        alerts << QString("<span style='color:#c01e1e'>⚠ Watch is running late "
                          "(%1 s/d)</span>").arg(mLastRate, 0, 'f', 1);
    if (mHaveAmp && mLastAmp < kAmpWarnLo)
        alerts << QString("<span style='color:#c01e1e'>⚠ Amplitude %1° — likely "
                          "needs service (below %2°)</span>")
                      .arg(mLastAmp, 0, 'f', 0).arg(kAmpWarnLo);
    else if (mHaveAmp && mLastAmp < kAmpBandLo)
        alerts << QString("<span style='color:#a07000'>⚠ Amplitude %1° — acceptable "
                          "but may suggest wear (%2°–%3°)</span>")
                      .arg(mLastAmp, 0, 'f', 0).arg(kAmpWarnLo).arg(kAmpBandLo);
    // amplitude > kAmpBandHi (310°) is not flagged — high is generally fine

    if (alerts.isEmpty())
        mAlertLabel->setText("Rate & amplitude within normal limits");
    else
        mAlertLabel->setText(alerts.join("   "));

    if (mRateN > 0 || mAmpN > 0) {
        mRateSummary->setText(QString("Session avg %1 s/d  ·  60 s avg %2 s/d")
                                  .arg(mRateN ? mRateSum / mRateN : 0.0, 0, 'f', 1)
                                  .arg(rollingAvg(mRateRecent), 0, 'f', 1));
        mAmpSummary->setText(QString("Session avg %1°  ·  60 s avg %2°")
                                 .arg(mAmpN ? mAmpSum / mAmpN : 0.0, 0, 'f', 0)
                                 .arg(rollingAvg(mAmpRecent), 0, 'f', 0));
    } else {
        mRateSummary->setText("slope = daily gain/loss  ·  smoothing = Averaging Period");
        mAmpSummary->setText("");
    }
}

void TraceTab::updateRanges()
{
    // Fixed rolling time window (Witschi §5.2): the trace moves, never compresses
    double hi = qMax(mTimeElapsed, windowSec());
    mPlot->xAxis->setRange(hi - windowSec(), hi);
    mAmpRect->axis(QCPAxis::atBottom)->setRange(hi - windowSec(), hi);

    // Y: at least the nominal span, expanded when data exceeds it (no clipping)
    mPlot->yAxis->setRange(qMin(kRateSpanLo, mRateMin - 2.0),
                           qMax(kRateSpanHi, mRateMax + 2.0));
    mAmpRect->axis(QCPAxis::atLeft)->setRange(qMin(kAmpSpanLo, mAmpMin - 10.0),
                                              qMax(kAmpSpanHi, mAmpMax + 10.0));
}

void TraceTab::onMeasurement(const Measurement &m)
{
    // Time always advances with the audio stream — invalid stretches leave a
    // visible gap in the trace (usability: measurement interruptions stand out)
    mTimeElapsed += (double)m.signal.pcm.size() / m.signal.samplesPerSecond;

    if (m.metrics.rate.has_value()) {
        mPlot->graph(0)->addData(mTimeElapsed, *m.metrics.rate);
        if (!mRateN) { mRateMin = mRateMax = *m.metrics.rate; }
        mRateMin = qMin(mRateMin, *m.metrics.rate);
        mRateMax = qMax(mRateMax, *m.metrics.rate);
        mRateSum += *m.metrics.rate; mRateN++;
        mRateRecent.append({mTimeElapsed, *m.metrics.rate});
        mLastRate = *m.metrics.rate;
        mHaveRate = true;
    }
    if (m.metrics.amplitude.has_value()) {
        mAmpGraph->addData(mTimeElapsed, *m.metrics.amplitude);
        if (!mAmpN) { mAmpMin = mAmpMax = *m.metrics.amplitude; }
        mAmpMin = qMin(mAmpMin, *m.metrics.amplitude);
        mAmpMax = qMax(mAmpMax, *m.metrics.amplitude);
        mAmpSum += *m.metrics.amplitude; mAmpN++;
        mAmpRecent.append({mTimeElapsed, *m.metrics.amplitude});
        mLastAmp = *m.metrics.amplitude;
        mHaveAmp = true;
    }
    while (!mRateRecent.isEmpty() && mRateRecent.first().first < mTimeElapsed - kRollingAvgSec)
        mRateRecent.removeFirst();
    while (!mAmpRecent.isEmpty() && mAmpRecent.first().first < mTimeElapsed - kRollingAvgSec)
        mAmpRecent.removeFirst();

    if (mPaused || !isVisible()) return;
    updateAlerts();
    updateRanges();
    g_replotCount++;
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void TraceTab::replotAll()
{
    updateAlerts(); updateRanges(); { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}
