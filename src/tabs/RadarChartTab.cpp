#include "RadarChartTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

namespace {
// Acceptable tolerance bands (match the reference values used elsewhere):
//   Rate  −5..+15 s/d   Amplitude 270..300°   Beat error 0..0.6 ms
constexpr double kRateLo = -5.0,  kRateHi = 15.0;
constexpr double kAmpLo  = 270.0, kAmpHi  = 300.0;
constexpr double kBeatHi = 0.6;

// Radial axis span per metric (center = lower bound, edge = upper bound).
constexpr double kRateAxisLo = -20.0, kRateAxisHi = 20.0;
constexpr double kAmpAxisLo  = 180.0, kAmpAxisHi  = 330.0;
constexpr double kBeatAxisLo = 0.0,   kBeatAxisHi = 2.0;
} // namespace

RadarChartTab::RadarChartTab(SequenceTab *sequence, QWidget *parent)
    : BaseGraphTab(parent), mSeq(sequence)
{
    auto *lay = new QVBoxLayout(this);

    auto *top = new QHBoxLayout;
    top->addWidget(new QLabel("Metric:", this));
    mMetricCombo = new QComboBox(this);
    mMetricCombo->addItems({"Rate (s/d)", "Amplitude (°)", "Beat error (ms)"});
    mMetricCombo->setCurrentIndex(Amplitude);   // amplitude reads most naturally on a radar
    top->addWidget(mMetricCombo);
    top->addStretch(1);
    lay->addLayout(top);

    mPlot = new QCustomPlot(this);
    lay->addWidget(mPlot, 1);

    mVerdictLabel = new QLabel(this);
    mVerdictLabel->setWordWrap(true);
    mVerdictLabel->setAlignment(Qt::AlignHCenter);
    lay->addWidget(mVerdictLabel);

    // Polar layout: replace the default cartesian axis rect with an angular axis
    mPlot->plotLayout()->clear();
    mAngular = new QCPPolarAxisAngular(mPlot);
    mPlot->plotLayout()->addElement(0, 0, mAngular);
    mAngular->setRangeDrag(false);
    mAngular->setRange(0, 360);
    mRadial = mAngular->radialAxis();

    // Spoke labels = watch positions, evenly spaced around the circle
    const QStringList pos = SequenceTab::positions();
    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    for (int i = 0; i < pos.size(); ++i)
        ticker->addTick(360.0 * i / pos.size(), pos[i]);
    mAngular->setTicker(ticker);

    // Reference tolerance rings (drawn first, under the data)
    auto makeRing = [&](const QColor &c) {
        auto *g = new QCPPolarGraph(mAngular, mRadial);
        g->setPen(QPen(c, 1, Qt::DashLine));
        g->setLineStyle(QCPPolarGraph::lsLine);
        return g;
    };
    mRefLo = makeRing(QColor(60, 160, 60));
    mRefHi = makeRing(QColor(60, 160, 60));

    // Measured polygon
    mData = new QCPPolarGraph(mAngular, mRadial);
    mData->setPen(QPen(QColor(0, 110, 200), 2));
    mData->setBrush(QBrush(QColor(0, 110, 200, 50)));
    mData->setLineStyle(QCPPolarGraph::lsLine);
    mData->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QColor(0, 110, 200), 6));

    // Out-of-tolerance markers (red, on top)
    mBad = new QCPPolarGraph(mAngular, mRadial);
    mBad->setLineStyle(QCPPolarGraph::lsNone);
    mBad->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QColor(210, 30, 30), 9));

    connect(mMetricCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { rebuild(); });

    configureRadialForMetric(currentMetric());
    reset();
}

RadarChartTab::Metric RadarChartTab::currentMetric() const
{
    return static_cast<Metric>(qBound(0, mMetricCombo->currentIndex(), 2));
}

double RadarChartTab::metricValue(Metric m, const SequenceTab::PositionReading &r) const
{
    switch (m) {
        case Rate:      return r.rate;
        case Amplitude: return r.amp;
        case BeatError: return r.beat;
    }
    return 0.0;
}

void RadarChartTab::toleranceBounds(Metric m, double &lo, double &hi,
                                    bool &hasLo, bool &hasHi) const
{
    switch (m) {
        case Rate:      lo = kRateLo; hi = kRateHi; hasLo = true;  hasHi = true;  break;
        case Amplitude: lo = kAmpLo;  hi = kAmpHi;  hasLo = true;  hasHi = true;  break;
        case BeatError: lo = 0.0;     hi = kBeatHi; hasLo = false; hasHi = true;  break;
    }
}

bool RadarChartTab::isOutOfTolerance(Metric m, double v) const
{
    double lo, hi; bool hasLo, hasHi;
    toleranceBounds(m, lo, hi, hasLo, hasHi);
    return (hasLo && v < lo) || (hasHi && v > hi);
}

void RadarChartTab::configureRadialForMetric(Metric m)
{
    switch (m) {
        case Rate:      mRadial->setRange(kRateAxisLo, kRateAxisHi); break;
        case Amplitude: mRadial->setRange(kAmpAxisLo,  kAmpAxisHi);  break;
        case BeatError: mRadial->setRange(kBeatAxisLo, kBeatAxisHi); break;
    }
}

void RadarChartTab::showEvent(QShowEvent *e)
{
    BaseGraphTab::showEvent(e);
    if (mDirty) rebuild();   // R1 lazy: render only once actually shown
}

void RadarChartTab::rebuild()
{
    if (!isVisible()) { mDirty = true; return; }   // defer until shown
    mDirty = false;

    const Metric m = currentMetric();
    configureRadialForMetric(m);

    const QStringList pos = SequenceTab::positions();
    const QVector<SequenceTab::PositionReading> readings =
        mSeq ? mSeq->capturedReadings() : QVector<SequenceTab::PositionReading>();

    // Collect captured positions: angle + value, and out-of-tolerance subset
    QVector<double> angK, valV, badK, badV;
    int captured = 0, outCount = 0, worstIdx = -1;
    double worstDev = -1.0;
    for (int i = 0; i < readings.size() && i < pos.size(); ++i) {
        if (!readings[i].valid) continue;
        captured++;
        const double angle = 360.0 * i / pos.size();
        const double v = metricValue(m, readings[i]);
        angK.append(angle);
        valV.append(v);
        if (isOutOfTolerance(m, v)) {
            outCount++;
            badK.append(angle);
            badV.append(v);
            double lo, hi; bool hasLo, hasHi; toleranceBounds(m, lo, hi, hasLo, hasHi);
            double dev = qMax(hasLo ? lo - v : 0.0, hasHi ? v - hi : 0.0);
            if (dev > worstDev) { worstDev = dev; worstIdx = i; }
        }
    }

    // Close the polygon by repeating the first vertex
    if (angK.size() >= 2) { angK.append(angK.first() + 360.0); valV.append(valV.first()); }

    mData->setData(angK, valV);
    mBad->setData(badK, badV);

    // Reference tolerance rings (constant radius around the full circle)
    double lo, hi; bool hasLo, hasHi;
    toleranceBounds(m, lo, hi, hasLo, hasHi);
    QVector<double> ringK; QVector<double> ringLoV, ringHiV;
    for (int a = 0; a <= 360; a += 15) { ringK.append(a); ringLoV.append(lo); ringHiV.append(hi); }
    mRefLo->setData(ringK, ringLoV);
    mRefLo->setVisible(hasLo);
    mRefHi->setData(ringK, ringHiV);
    mRefHi->setVisible(hasHi);

    // Verdict
    const QString unit = (m == Rate) ? "s/d" : (m == Amplitude) ? "°" : "ms";
    if (captured < 3) {
        mVerdict = QString("Capture at least 3 positions to plot a radar "
                           "(captured %1).").arg(captured);
    } else if (outCount == 0) {
        mVerdict = QString("✓ Balanced — all %1 positions within tolerance for %2.")
                       .arg(captured)
                       .arg(mMetricCombo->currentText());
    } else {
        const QString worst = (worstIdx >= 0 && worstIdx < pos.size()) ? pos[worstIdx] : "?";
        const double worstVal = (worstIdx >= 0) ? metricValue(m, readings[worstIdx]) : 0.0;
        mVerdict = QString("⚠ %1 of %2 positions out of tolerance — worst at %3 "
                           "(%4 %5). Positional variance suggests poising / balance "
                           "review.")
                       .arg(outCount).arg(captured).arg(worst)
                       .arg(worstVal, 0, 'f', (m == Amplitude) ? 0 : 1).arg(unit);
    }
    mVerdictLabel->setText(mVerdict);
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

QString RadarChartTab::verdictText() const { return mVerdict; }

void RadarChartTab::reset()
{
    mData->data()->clear();
    mBad->data()->clear();
    mRefLo->data()->clear();
    mRefHi->data()->clear();
    mVerdict.clear();
    mVerdictLabel->setText("Capture watch readings across positions in the "
                           "Sequence tab to build the radar.");
    mDirty = true;
    mPlot->replot();
}
