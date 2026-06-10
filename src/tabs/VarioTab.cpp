#include "VarioTab.h"
#include <QVBoxLayout>

// Acceptable ranges (project plan: amplitude 270–300° normal; rate ±10 s/d)
static constexpr double kRateLo = -20,  kRateHi = 20;
static constexpr double kRateBandLo = -10, kRateBandHi = 10;
static constexpr double kAmpLo  = 180, kAmpHi  = 330;
static constexpr double kAmpBandLo = 270, kAmpBandHi = 300;

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

    updateScale(mRateScale, mRate, "Rate", "s/d", 1);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0);
}

VarioTab::Scale VarioTab::makeScale(double lo, double hi, double bandLo, double bandHi)
{
    Scale s;
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
                           const QString &unit, int decimals)
{
    auto num = [&](double v) { return QString::number(v, 'f', decimals); };
    if (st.n == 0) {
        s.label->setText(QString("<b>%1</b> (%2) — waiting for data").arg(name, unit));
        return;
    }
    s.label->setText(QString("<b>%1</b>  Min <b>%2</b>   "
                             "<span style='color:#c01e1e'>X̄ <b>%3</b></span>   "
                             "<span style='color:#b8860b'>σ <b>%4</b></span>   "
                             "Max <b>%5</b>  %6")
                         .arg(name, num(st.min), num(st.mean()),
                              num(st.sigma()), num(st.max), unit));
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
    mElapsedLabel->setText("0:00");
    for (Scale *s : {&mRateScale, &mAmpScale}) {
        for (QCPItemLine *a : {s->minArrow, s->maxArrow, s->meanArrow})
            a->setVisible(false);
        s->plot->replot();
    }
    updateScale(mRateScale, mRate, "Rate", "s/d", 1);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0);
}

void VarioTab::onMeasurement(const Measurement &m)
{
    mElapsedSec += (double)m.pcm.size() / m.samplesPerSecond;

    bool changed = false;
    if (m.rateValid)      { mRate.add(m.rateErrorSpd); changed = true; }
    if (m.amplitudeValid) { mAmp.add(m.amplitudeDeg);  changed = true; }
    if (!changed || mPaused) return;

    int sec = (int)mElapsedSec;
    mElapsedLabel->setText(QString("%1:%2").arg(sec / 60)
                               .arg(sec % 60, 2, 10, QChar('0')));
    updateScale(mRateScale, mRate, "Rate", "s/d", 1);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0);
    mRateScale.plot->replot(QCustomPlot::rpQueuedReplot);
    mAmpScale.plot->replot(QCustomPlot::rpQueuedReplot);
}
