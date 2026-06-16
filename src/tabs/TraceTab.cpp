#include "TraceTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace {
constexpr double kAmpBandLo  = 270.0;  // normal amplitude range (project plan)
constexpr double kAmpBandHi  = 300.0;
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

    mSummaryLabel = new QLabel(this);
    mSummaryLabel->setAlignment(Qt::AlignHCenter);
    mSummaryLabel->setWordWrap(true);
    mSummaryLabel->setToolTip("How to read: slope of the rate trace = gain/loss per day; "
                              "amplitude inside the green band 270–300° is healthy. "
                              "Smoothing of the s/d trace follows the Averaging Period setting.");
    layout->addWidget(mSummaryLabel);

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
    mPlot->plotLayout()->addElement(1, 0, mAmpRect);
    mAmpGraph = mPlot->addGraph(mAmpRect->axis(QCPAxis::atBottom),
                                mAmpRect->axis(QCPAxis::atLeft));
    mAmpGraph->setPen(QPen(QColor(20, 130, 60), 2));
    mAmpGraph->setName("Amplitude (°)");
    mAmpRect->axis(QCPAxis::atBottom)->setLabel("Time (s)");
    mAmpRect->axis(QCPAxis::atLeft)->setLabel("Amplitude (°)");
    mAmpRect->axis(QCPAxis::atLeft)->setRange(kAmpSpanLo, kAmpSpanHi);

    auto *band = new QCPItemRect(mPlot);
    band->setClipAxisRect(mAmpRect);
    band->topLeft->setAxes(mAmpRect->axis(QCPAxis::atBottom), mAmpRect->axis(QCPAxis::atLeft));
    band->bottomRight->setAxes(mAmpRect->axis(QCPAxis::atBottom), mAmpRect->axis(QCPAxis::atLeft));
    band->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
    band->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
    band->topLeft->setCoords(0.0, kAmpBandHi);
    band->bottomRight->setCoords(1.0, kAmpBandLo);
    band->setBrush(QBrush(QColor(120, 220, 120, 80)));
    band->setPen(Qt::NoPen);

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
    double s = 0;
    for (const auto &p : buf) s += p.second;
    return s / buf.size();
}

void TraceTab::updateAlerts()
{
    QStringList alerts;
    if (mHaveRate && mLastRate < kLateThresh)
        alerts << QString("<span style='color:#c01e1e'>⚠ Watch is running late "
                          "(%1 s/d)</span>").arg(mLastRate, 0, 'f', 1);
    if (mHaveAmp && (mLastAmp < kAmpBandLo || mLastAmp > kAmpBandHi))
        alerts << QString("<span style='color:#c01e1e'>⚠ Amplitude %1° outside "
                          "normal range %2–%3°</span>")
                      .arg(mLastAmp, 0, 'f', 0).arg(kAmpBandLo).arg(kAmpBandHi);
    if (alerts.isEmpty())
        mAlertLabel->setText("Rate & amplitude within normal limits");
    else
        mAlertLabel->setText(alerts.join("   "));

    if (mRateN > 0 || mAmpN > 0)
        mSummaryLabel->setText(QString("Session avg %1 s/d · %2°    |    "
                                       "60 s avg %3 s/d · %4°")
                                   .arg(mRateN ? mRateSum / mRateN : 0.0, 0, 'f', 1)
                                   .arg(mAmpN ? mAmpSum / mAmpN : 0.0, 0, 'f', 0)
                                   .arg(rollingAvg(mRateRecent), 0, 'f', 1)
                                   .arg(rollingAvg(mAmpRecent), 0, 'f', 0));
    else
        mSummaryLabel->setText("Slope = daily gain/loss · green band = healthy amplitude "
                               "(270–300°) · smoothing = Averaging Period");
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
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;

    if (m.rateValid) {
        mPlot->graph(0)->addData(mTimeElapsed, m.rateErrorSpd);
        if (!mRateN) { mRateMin = mRateMax = m.rateErrorSpd; }
        mRateMin = qMin(mRateMin, m.rateErrorSpd);
        mRateMax = qMax(mRateMax, m.rateErrorSpd);
        mRateSum += m.rateErrorSpd; mRateN++;
        mRateRecent.append({mTimeElapsed, m.rateErrorSpd});
        mLastRate = m.rateErrorSpd;
        mHaveRate = true;
    }
    if (m.amplitudeValid) {
        mAmpGraph->addData(mTimeElapsed, m.amplitudeDeg);
        if (!mAmpN) { mAmpMin = mAmpMax = m.amplitudeDeg; }
        mAmpMin = qMin(mAmpMin, m.amplitudeDeg);
        mAmpMax = qMax(mAmpMax, m.amplitudeDeg);
        mAmpSum += m.amplitudeDeg; mAmpN++;
        mAmpRecent.append({mTimeElapsed, m.amplitudeDeg});
        mLastAmp = m.amplitudeDeg;
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
