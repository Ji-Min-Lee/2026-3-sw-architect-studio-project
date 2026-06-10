#include "BeatErrorTab.h"
#include <QVBoxLayout>

namespace {
constexpr double kGoodBeatErrorMs = 0.6;  // "values under 0.6 ms generally good"
} // namespace

BeatErrorTab::BeatErrorTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    mHeaderLabel = new QLabel(this);
    mHeaderLabel->setAlignment(Qt::AlignHCenter);
    lay->addWidget(mHeaderLabel);

    mAlertLabel = new QLabel(this);
    mAlertLabel->setAlignment(Qt::AlignHCenter);
    lay->addWidget(mAlertLabel);

    mPlot = new QCustomPlot(this);
    lay->addWidget(mPlot, 1);

    // Top rect (default axes): beat error rolling average — graph(0)
    mPlot->addGraph();
    QPen pen; pen.setColor(Qt::darkMagenta); pen.setWidth(2);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Beat Error (ms)");
    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Beat Error (ms)");
    mPlot->yAxis->setRange(0, 5);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Good-range band (0 .. 0.6 ms)
    auto *band = new QCPItemRect(mPlot);
    band->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
    band->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
    band->topLeft->setCoords(0.0, kGoodBeatErrorMs);
    band->bottomRight->setCoords(1.0, 0.0);
    band->setBrush(QBrush(QColor(120, 220, 120, 80)));
    band->setPen(Qt::NoPen);

    // Bottom rect: diagnostic tic/toc dotted trace
    mTraceRect = new QCPAxisRect(mPlot);
    mPlot->plotLayout()->addElement(1, 0, mTraceRect);
    mTicGraph = mPlot->addGraph(mTraceRect->axis(QCPAxis::atBottom),
                                mTraceRect->axis(QCPAxis::atLeft));
    mTocGraph = mPlot->addGraph(mTraceRect->axis(QCPAxis::atBottom),
                                mTraceRect->axis(QCPAxis::atLeft));
    mTicGraph->setLineStyle(QCPGraph::lsNone);
    mTocGraph->setLineStyle(QCPGraph::lsNone);
    mTicGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2.5));
    mTocGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2.5));
    mTicGraph->setPen(QPen(Qt::red));
    mTocGraph->setPen(QPen(Qt::blue));
    mTraceRect->axis(QCPAxis::atBottom)->setLabel("Beat #");
    mTraceRect->axis(QCPAxis::atLeft)->setLabel("Timing offset (ms)");
    mTraceRect->axis(QCPAxis::atLeft)->setRange(-10, 10);

    updateHeader(Measurement{});
}

void BeatErrorTab::reset()
{
    mTimeElapsed = 0.0;
    mBeatIdx     = 0;
    mPlot->graph(0)->data()->clear();
    mTicGraph->data()->clear();
    mTocGraph->data()->clear();
    updateHeader(Measurement{});
    mPlot->replot();
}

void BeatErrorTab::updateHeader(const Measurement &m)
{
    auto fmt = [](bool valid, double v, int dec, const QString &unit) {
        return valid ? QString("%1 %2").arg(v, 0, 'f', dec).arg(unit) : QString("--- %1").arg(unit);
    };
    mHeaderLabel->setText(QString("<b>RATE %1   AMPLITUDE %2   BEAT ERROR %3   BEAT %4</b>")
                              .arg(fmt(m.rateValid, m.rateErrorSpd, 1, "s/d"),
                                   fmt(m.amplitudeValid, m.amplitudeDeg, 0, "°"),
                                   fmt(m.beatErrorValid, m.beatErrorMs, 2, "ms"),
                                   m.synced ? QString("%1 bph").arg(m.detectedBph) : "----- bph"));

    QStringList alerts;
    if (m.beatErrorValid && m.beatErrorMs > kGoodBeatErrorMs)
        alerts << QString("<span style='color:#c01e1e'>⚠ Line separation %1 ms exceeds "
                          "%2 ms</span>").arg(m.beatErrorMs, 0, 'f', 2).arg(kGoodBeatErrorMs);

    // Slope check: linear fit of the recent diagnostic trace, converted to
    // on-screen angle using the current axis ranges (>45° = major fault)
    auto data = mTicGraph->data();
    int n = data->size();
    if (n >= 10) {
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        int from = n - qMin(n, 50);
        int cnt = 0;
        for (int i = from; i < n; i++) {
            double x = data->at(i)->key, y = data->at(i)->value;
            sx += x; sy += y; sxx += x * x; sxy += x * y; cnt++;
        }
        double denom = cnt * sxx - sx * sx;
        if (std::abs(denom) > 1e-12) {
            double slope = (cnt * sxy - sx * sy) / denom;   // ms per beat
            QCPAxis *xa = mTraceRect->axis(QCPAxis::atBottom);
            QCPAxis *ya = mTraceRect->axis(QCPAxis::atLeft);
            double pxPerBeat = mTraceRect->width()  / qMax(1.0, xa->range().size());
            double pxPerMs   = mTraceRect->height() / qMax(1.0, ya->range().size());
            double screenSlope = std::abs(slope) * pxPerMs / qMax(1.0, pxPerBeat);
            if (screenSlope > 1.0)  // tan(45°)
                alerts << "<span style='color:#c01e1e'><b>✖ MAJOR FAULT — trace slope "
                          "exceeds 45°</b></span>";
        }
    }
    mAlertLabel->setText(alerts.isEmpty()
                             ? "Trace separation = beat error · slope = rate deviation"
                             : alerts.join("   "));
}

void BeatErrorTab::onMeasurement(const Measurement &m)
{
    // Diagnostic trace points (every synced A-event)
    for (const AcousticEvent &ev : m.events) {
        if (!ev.isA || !ev.hasRatePoint) continue;
        if (ev.isTic) mTicGraph->addData(mBeatIdx, ev.wrappedRateError);
        else          mTocGraph->addData(mBeatIdx, ev.wrappedRateError);
        mBeatIdx++;
    }

    if (m.beatErrorValid) {
        mTimeElapsed += (double)m.pcm.size() / m.samplesPerSecond;
        mPlot->graph(0)->addData(mTimeElapsed, m.beatErrorMs);
    }

    if (mPaused) return;
    updateHeader(m);
    if (m.beatErrorValid) {
        mPlot->xAxis->rescale();
        mPlot->yAxis->rescale();
        if (mPlot->yAxis->range().lower > 0) mPlot->yAxis->setRangeLower(0);
    }
    mTraceRect->axis(QCPAxis::atBottom)->setRange(qMax(0, mBeatIdx - kTracePoints), mBeatIdx);
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
