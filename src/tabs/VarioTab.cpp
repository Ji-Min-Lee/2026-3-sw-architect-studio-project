#include "VarioTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>

// Acceptable (green) ranges fixed to the Witschi reference figure (Fig 9):
// rate −5..+15 s/d (asymmetric — slight gain preferred over loss),
// amplitude 195..300°. Witschi stores these per watch program; a per-watch
// tolerance UI can replace these constants later.
static constexpr double kRateLo = -20,  kRateHi = 20;
static constexpr double kRateBandLo = -5,  kRateBandHi = 15;
static constexpr double kAmpLo  = 180, kAmpHi  = 330;
static constexpr double kAmpBandLo = 195, kAmpBandHi = 300;

VarioTab::VarioTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(8, 8, 8, 8);

    mElapsedLabel = new QLabel("0:00", this);
    mElapsedLabel->setAlignment(Qt::AlignHCenter);
    QFont f = mElapsedLabel->font(); f.setPointSize(16); f.setBold(true);
    mElapsedLabel->setFont(f);
    lay->addWidget(mElapsedLabel);

    mRateScale = makeScale(kRateLo, kRateHi, kRateBandLo, kRateBandHi);
    lay->addWidget(mRateScale.label);
    lay->addWidget(mRateScale.plot, 1);

    mAmpScale = makeScale(kAmpLo, kAmpHi, kAmpBandLo, kAmpBandHi);
    lay->addWidget(mAmpScale.label);
    lay->addWidget(mAmpScale.plot, 1);

    updateScale(mRateScale, mRate, "Rate", "s/d", 1, false, 0);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, false, 0);
}

VarioTab::Scale VarioTab::makeScale(double lo, double hi, double bandLo, double bandHi)
{
    Scale s;
    s.nomLo  = lo;
    s.nomHi  = hi;
    s.bandLo = bandLo;
    s.bandHi = bandHi;
    s.label = new QLabel(this);
    s.label->setTextFormat(Qt::RichText);
    s.label->setAlignment(Qt::AlignHCenter);

    s.plot = new QCustomPlot(this);
    s.plot->xAxis->setRange(lo, hi);
    s.plot->yAxis->setRange(0, 1);
    s.plot->yAxis->setVisible(false);
    s.plot->setMinimumHeight(70);

    // Green acceptable band
    auto *band = new QCPItemRect(s.plot);
    band->topLeft->setCoords(bandLo, 0.95);
    band->bottomRight->setCoords(bandHi, 0.05);
    band->setBrush(QBrush(QColor(120, 220, 120, 110)));
    band->setPen(Qt::NoPen);

    // Yellow X̄ highlight stripe (drawn over the band, under the arrows)
    s.meanStripe = new QCPItemRect(s.plot);
    s.meanStripe->setBrush(QBrush(QColor(255, 215, 60, 170)));
    s.meanStripe->setPen(Qt::NoPen);
    s.meanStripe->setVisible(false);

    s.minArrow  = makeArrow(s.plot, QColor(30, 60, 220), 2);
    s.maxArrow  = makeArrow(s.plot, QColor(30, 60, 220), 2);
    s.meanArrow = makeArrow(s.plot, QColor(200, 30, 30), 3);
    return s;
}

QCPItemLine *VarioTab::makeArrow(QCustomPlot *p, const QColor &c, double width)
{
    auto *a = new QCPItemLine(p);
    a->start->setCoords(0, 0.9);
    a->end->setCoords(0, 0.15);
    a->setHead(QCPLineEnding(QCPLineEnding::esSpikeArrow, 9, 14));
    a->setPen(QPen(c, width));
    a->setVisible(false);
    return a;
}

void VarioTab::updateScale(Scale &s, const Stats &st, const QString &name,
                           const QString &unit, int decimals,
                           bool haveNow, double now)
{
    auto num = [&](double v) { return QString::number(v, 'f', decimals); };
    if (st.n == 0) {
        s.label->setText(QString("<b>%1</b> (%2) — waiting for data · green band = "
                                 "acceptable %3..%4 %2")
                             .arg(name, unit, num(s.bandLo), num(s.bandHi)));
        return;
    }
    // Witschi-style pass/fail against the acceptable (green) range
    bool pass = (st.mean() >= s.bandLo && st.mean() <= s.bandHi);
    QString verdict = pass
        ? "<span style='color:#1c8a1c'><b>✓</b></span>"
        : QString("<span style='color:#c01e1e'><b>✗ outside acceptable "
                  "%1..%2 %3</b></span>").arg(num(s.bandLo), num(s.bandHi), unit);
    QString nowStr = haveNow ? QString("   now <b>%1</b>").arg(num(now)) : QString();
    s.label->setText(QString("<b>%1</b>  Min <b>%2</b>   "
                             "<span style='background-color:#ffd73c;color:#000'>"
                             "&nbsp;X̄ <b>%3</b>&nbsp;</span>   "
                             "<span style='color:#b8860b'>σ <b>%4</b></span>   "
                             "Max <b>%5</b>   Δ <b>%6</b>%7  %8   %9")
                         .arg(name, num(st.min), num(st.mean()), num(st.sigma()),
                              num(st.max), num(st.max - st.min), nowStr, unit, verdict));

    // Witschi-style adaptive axis: keep the nominal span but widen so the
    // min/max arrows never sit on (or past) the plot edge
    double margin = qMax((s.nomHi - s.nomLo) * 0.05, st.sigma());
    s.plot->xAxis->setRange(qMin(s.nomLo, st.min - margin),
                            qMax(s.nomHi, st.max + margin));

    // Yellow X̄ stripe: half a sigma each side (minimum visible width)
    double half = qMax(st.sigma() / 2.0, (s.plot->xAxis->range().size()) * 0.006);
    s.meanStripe->topLeft->setCoords(st.mean() - half, 0.95);
    s.meanStripe->bottomRight->setCoords(st.mean() + half, 0.05);
    s.meanStripe->setVisible(true);

    s.minArrow->start->setCoords(st.min, 0.9);
    s.minArrow->end->setCoords(st.min, 0.15);
    s.maxArrow->start->setCoords(st.max, 0.9);
    s.maxArrow->end->setCoords(st.max, 0.15);
    s.meanArrow->start->setCoords(st.mean(), 0.9);
    s.meanArrow->end->setCoords(st.mean(), 0.15);
    s.minArrow->setVisible(true);
    s.maxArrow->setVisible(true);
    s.meanArrow->setVisible(true);
}

void VarioTab::reset()
{
    mRate = Stats{};
    mAmp  = Stats{};
    mElapsedSec = 0.0;
    mHaveRateNow = false;
    mHaveAmpNow  = false;
    mElapsedLabel->setText("0:00");
    for (Scale *s : {&mRateScale, &mAmpScale}) {
        for (QCPItemLine *a : {s->minArrow, s->maxArrow, s->meanArrow})
            a->setVisible(false);
        s->meanStripe->setVisible(false);
        s->plot->xAxis->setRange(s->nomLo, s->nomHi);
        s->plot->replot();
    }
    updateScale(mRateScale, mRate, "Rate", "s/d", 1, false, 0);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, false, 0);
}

void VarioTab::onMeasurement(const Measurement &m)
{
    mElapsedSec += (double)m.pcm.size() / m.samplesPerSecond;

    bool changed = false;
    if (m.rateValid) {
        mRate.add(m.rateErrorSpd);
        mRateNow = m.rateErrorSpd; mHaveRateNow = true;
        changed = true;
    }
    if (m.amplitudeValid) {
        mAmp.add(m.amplitudeDeg);
        mAmpNow = m.amplitudeDeg; mHaveAmpNow = true;
        changed = true;
    }
    if (!changed || mPaused) return;

    int sec = (int)mElapsedSec;
    mElapsedLabel->setText(QString("%1:%2").arg(sec / 60)
                               .arg(sec % 60, 2, 10, QChar('0')));
    updateScale(mRateScale, mRate, "Rate", "s/d", 1, mHaveRateNow, mRateNow);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, mHaveAmpNow, mAmpNow);
    g_replotCount++;
    mRateScale.plot->replot(QCustomPlot::rpQueuedReplot);
    g_replotCount++;
    mAmpScale.plot->replot(QCustomPlot::rpQueuedReplot);
}
