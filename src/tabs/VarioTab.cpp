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
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    mElapsedLabel = new QLabel("0:00", this);
    mElapsedLabel->setAlignment(Qt::AlignHCenter);
    QFont elapsedFont = mElapsedLabel->font(); elapsedFont.setPointSize(16); elapsedFont.setBold(true);
    mElapsedLabel->setFont(elapsedFont);
    mainLayout->addWidget(mElapsedLabel);

    mRateScale = makeScale(kRateLo, kRateHi, kRateBandLo, kRateBandHi);
    mainLayout->addWidget(mRateScale.label);
    mainLayout->addWidget(mRateScale.plot, 1);

    mAmpScale = makeScale(kAmpLo, kAmpHi, kAmpBandLo, kAmpBandHi);
    mainLayout->addWidget(mAmpScale.label);
    mainLayout->addWidget(mAmpScale.plot, 1);

    updateScale(mRateScale, mRate, "Rate", "s/d", 1, false, 0);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, false, 0);
}

VarioTab::Scale VarioTab::makeScale(double lo, double hi, double bandLo, double bandHi)
{
    Scale scale;
    scale.nomLo  = lo;
    scale.nomHi  = hi;
    scale.bandLo = bandLo;
    scale.bandHi = bandHi;
    scale.label = new QLabel(this);
    scale.label->setTextFormat(Qt::RichText);
    scale.label->setAlignment(Qt::AlignHCenter);

    scale.plot = new QCustomPlot(this);
    scale.plot->xAxis->setRange(lo, hi);
    scale.plot->yAxis->setRange(0, 1);
    scale.plot->yAxis->setVisible(false);
    scale.plot->setMinimumHeight(70);

    // Green acceptable band
    auto *band = new QCPItemRect(scale.plot);
    band->topLeft->setCoords(bandLo, 0.95);
    band->bottomRight->setCoords(bandHi, 0.05);
    band->setBrush(QBrush(QColor(120, 220, 120, 110)));
    band->setPen(Qt::NoPen);

    // Yellow X̄ highlight stripe (drawn over the band, under the arrows)
    scale.meanStripe = new QCPItemRect(scale.plot);
    scale.meanStripe->setBrush(QBrush(QColor(255, 215, 60, 170)));
    scale.meanStripe->setPen(Qt::NoPen);
    scale.meanStripe->setVisible(false);

    scale.minArrow  = makeArrow(scale.plot, QColor(30, 60, 220), 2);
    scale.maxArrow  = makeArrow(scale.plot, QColor(30, 60, 220), 2);
    scale.meanArrow = makeArrow(scale.plot, QColor(200, 30, 30), 3);
    return scale;
}

QCPItemLine *VarioTab::makeArrow(QCustomPlot *plot, const QColor &c, double width)
{
    auto *arrow = new QCPItemLine(plot);
    arrow->start->setCoords(0, 0.9);
    arrow->end->setCoords(0, 0.15);
    arrow->setHead(QCPLineEnding(QCPLineEnding::esSpikeArrow, 9, 14));
    arrow->setPen(QPen(c, width));
    arrow->setVisible(false);
    return arrow;
}

void VarioTab::updateScale(Scale &s, const Stats &stats, const QString &name,
                           const QString &unit, int decimals,
                           bool haveNow, double now)
{
    auto formatNum = [&](double value) { return QString::number(value, 'f', decimals); };
    if (stats.n == 0) {
        s.label->setText(QString("<b>%1</b> (%2) — waiting for data · green band = "
                                 "acceptable %3..%4 %2")
                             .arg(name, unit, formatNum(s.bandLo), formatNum(s.bandHi)));
        return;
    }
    // Witschi-style pass/fail against the acceptable (green) range
    bool pass = (stats.mean() >= s.bandLo && stats.mean() <= s.bandHi);
    QString verdict = pass
        ? "<span style='color:#1c8a1c'><b>✓</b></span>"
        : QString("<span style='color:#c01e1e'><b>✗ outside acceptable "
                  "%1..%2 %3</b></span>").arg(formatNum(s.bandLo), formatNum(s.bandHi), unit);
    QString nowStr = haveNow ? QString("   now <b>%1</b>").arg(formatNum(now)) : QString();
    s.label->setText(QString("<b>%1</b>  Min <b>%2</b>   "
                             "<span style='background-color:#ffd73c;color:#000'>"
                             "&nbsp;X̄ <b>%3</b>&nbsp;</span>   "
                             "<span style='color:#b8860b'>σ <b>%4</b></span>   "
                             "Max <b>%5</b>   Δ <b>%6</b>%7  %8   %9")
                         .arg(name, formatNum(stats.min), formatNum(stats.mean()), formatNum(stats.sigma()),
                              formatNum(stats.max), formatNum(stats.max - stats.min), nowStr, unit, verdict));

    // Witschi-style adaptive axis: keep the nominal span but widen so the
    // min/max arrows never sit on (or past) the plot edge
    double margin = qMax((s.nomHi - s.nomLo) * 0.05, stats.sigma());
    s.plot->xAxis->setRange(qMin(s.nomLo, stats.min - margin),
                            qMax(s.nomHi, stats.max + margin));

    // Yellow X̄ stripe: half a sigma each side (minimum visible width)
    double half = qMax(stats.sigma() / 2.0, (s.plot->xAxis->range().size()) * 0.006);
    s.meanStripe->topLeft->setCoords(stats.mean() - half, 0.95);
    s.meanStripe->bottomRight->setCoords(stats.mean() + half, 0.05);
    s.meanStripe->setVisible(true);

    s.minArrow->start->setCoords(stats.min, 0.9);
    s.minArrow->end->setCoords(stats.min, 0.15);
    s.maxArrow->start->setCoords(stats.max, 0.9);
    s.maxArrow->end->setCoords(stats.max, 0.15);
    s.meanArrow->start->setCoords(stats.mean(), 0.9);
    s.meanArrow->end->setCoords(stats.mean(), 0.15);
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
        { int64_t _pt=TG_NOW(); s->plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
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
    if (!changed || mPaused || !isVisible()) return;

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

void VarioTab::replotAll()
{
    updateScale(mRateScale, mRate, "Rate", "s/d", 1, mHaveRateNow, mRateNow);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, mHaveAmpNow, mAmpNow);
    { int64_t _pt=TG_NOW(); mRateScale.plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); }; { int64_t _pt=TG_NOW(); mAmpScale.plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}
