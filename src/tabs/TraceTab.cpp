#include "TraceTab.h"
#include <QVBoxLayout>

namespace {
constexpr double kAmpBandLo  = 270.0;  // normal amplitude range (project plan)
constexpr double kAmpBandHi  = 300.0;
constexpr double kLateThresh = -3.0;   // smoothed s/d below this → "running late"
} // namespace

TraceTab::TraceTab(QWidget *parent)
    : BaseGraphTab(parent)
    , mPlot(new QCustomPlot(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    mAlertLabel = new QLabel(this);
    mAlertLabel->setAlignment(Qt::AlignHCenter);
    layout->addWidget(mAlertLabel);

    layout->addWidget(mPlot, 1);

    mSummaryLabel = new QLabel(this);
    mSummaryLabel->setAlignment(Qt::AlignHCenter);
    mSummaryLabel->setToolTip("How to read: slope of the rate trace = gain/loss per day; "
                              "amplitude inside the green band 270–300° is healthy.");
    layout->addWidget(mSummaryLabel);

    // Top rect (default axes): rate — graph(0) raw points, graph(1) smoothed
    mPlot->addGraph();
    QPen pen; pen.setColor(QColor(0, 120, 215)); pen.setWidth(2);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Rate Error (s/day)");

    mSmoothedGraph = mPlot->addGraph();
    mSmoothedGraph->setPen(QPen(QColor(200, 60, 60), 2));
    mSmoothedGraph->setName("Smoothed (s/day)");

    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Rate Error (s/day)");
    mPlot->yAxis->setRange(-20, 20);
    mPlot->legend->setVisible(true);
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
    mAmpRect->axis(QCPAxis::atLeft)->setRange(180, 330);

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

    updateAlerts();
}

void TraceTab::reset()
{
    mTimeElapsed = 0.0;
    mRateWindow.clear();
    mRateSum = 0; mRateN = 0;
    mAmpSum  = 0; mAmpN  = 0;
    mHaveAmp = false;
    mPlot->graph(0)->data()->clear();
    mSmoothedGraph->data()->clear();
    mAmpGraph->data()->clear();
    updateAlerts();
    mPlot->replot();
}

void TraceTab::updateAlerts()
{
    QStringList alerts;
    if (mRateN > 0 && mLastSmoothed < kLateThresh)
        alerts << QString("<span style='color:#c01e1e'>⚠ Watch is running late "
                          "(%1 s/d)</span>").arg(mLastSmoothed, 0, 'f', 1);
    if (mHaveAmp && (mLastAmp < kAmpBandLo || mLastAmp > kAmpBandHi))
        alerts << QString("<span style='color:#c01e1e'>⚠ Amplitude %1° outside "
                          "normal range %2–%3°</span>")
                      .arg(mLastAmp, 0, 'f', 0).arg(kAmpBandLo).arg(kAmpBandHi);
    if (alerts.isEmpty())
        mAlertLabel->setText("Rate & amplitude within normal limits");
    else
        mAlertLabel->setText(alerts.join("   "));

    if (mRateN > 0 || mAmpN > 0)
        mSummaryLabel->setText(QString("Session average — rate %1 s/d, amplitude %2°   "
                                       "(slope = daily gain/loss · green band = healthy amplitude)")
                                   .arg(mRateN ? mRateSum / mRateN : 0.0, 0, 'f', 1)
                                   .arg(mAmpN ? mAmpSum / mAmpN : 0.0, 0, 'f', 0));
    else
        mSummaryLabel->setText("Slope of the rate trace = daily gain/loss · "
                               "green band = healthy amplitude (270–300°)");
}

void TraceTab::onMeasurement(const Measurement &m)
{
    if (!m.rateValid && !m.amplitudeValid) return;

    // Advance time by the number of PCM samples in this block
    mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;

    if (m.rateValid) {
        mPlot->graph(0)->addData(mTimeElapsed, m.rateErrorSpd);

        mRateWindow.append(m.rateErrorSpd);
        if (mRateWindow.size() > kSmoothWindow) mRateWindow.removeFirst();
        double s = 0;
        for (double v : mRateWindow) s += v;
        mLastSmoothed = s / mRateWindow.size();
        mSmoothedGraph->addData(mTimeElapsed, mLastSmoothed);

        mRateSum += m.rateErrorSpd; mRateN++;
    }
    if (m.amplitudeValid) {
        mAmpGraph->addData(mTimeElapsed, m.amplitudeDeg);
        mLastAmp = m.amplitudeDeg;
        mHaveAmp = true;
        mAmpSum += m.amplitudeDeg; mAmpN++;
    }

    if (mPaused) return;
    updateAlerts();
    mPlot->xAxis->rescale();
    mAmpRect->axis(QCPAxis::atBottom)->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
