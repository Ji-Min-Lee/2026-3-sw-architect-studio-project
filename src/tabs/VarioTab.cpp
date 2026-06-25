#include "VarioTab.h"
#include "ReplotCounter.h"
#include <QVBoxLayout>

namespace {
// Acceptable (green) ranges — graph-analysis.md / TraceTab strong zone:
//   Rate −5..+15 s/d (Witschi Fig 9, asymmetric),
//   Amplitude 270..310°.
constexpr double kRateLo = -20,  kRateHi = 20;
constexpr double kRateBandLo = -5,  kRateBandHi = 15;
constexpr double kAmpLo  = 180, kAmpHi  = 330;
constexpr double kAmpBandLo = 270, kAmpBandHi = 310;

constexpr QColor kMinMaxColor(30, 60, 220);
constexpr QColor kNowColor(200, 30, 30);
const char *kMinMaxHtml = "#1e3cdc";
const char *kNowHtml    = "#c81e1e";
} // namespace

VarioTab::VarioTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    mElapsedLabel = new QLabel("0:00", this);
    mElapsedLabel->setAlignment(Qt::AlignHCenter);
    QFont elapsedFont = mElapsedLabel->font(); elapsedFont.setPointSize(16); elapsedFont.setBold(true);
    mElapsedLabel->setFont(elapsedFont);
    mainLayout->addWidget(mElapsedLabel);

    mRateScale = makeScale(kRateLo, kRateHi, kRateBandLo, kRateBandHi, "s/d");
    mainLayout->addWidget(mRateScale.label);
    mainLayout->addWidget(mRateScale.plot, 1);

    mAmpScale = makeScale(kAmpLo, kAmpHi, kAmpBandLo, kAmpBandHi, "°");
    mainLayout->addWidget(mAmpScale.label);
    mainLayout->addWidget(mAmpScale.plot, 1);

    mTimingLabel = new QLabel(this);
    mTimingLabel->setTextFormat(Qt::RichText);
    mTimingLabel->setAlignment(Qt::AlignHCenter);
    mTimingLabel->setStyleSheet("QLabel { border-top: 1px solid #ccc; padding-top: 4px; }");
    mainLayout->addWidget(mTimingLabel);

    updateScale(mRateScale, mRate, "Rate", "s/d", 1, false, 0);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, false, 0);
    updateTimingLabel();
}

VarioTab::Scale VarioTab::makeScale(double lo, double hi, double bandLo, double bandHi,
                                    const QString &axisUnit)
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
    scale.plot->xAxis->setLabel(axisUnit);
    scale.plot->yAxis->setRange(0, 1);
    scale.plot->yAxis->setVisible(false);
    scale.plot->setMinimumHeight(70);

    auto *band = new QCPItemRect(scale.plot);
    band->topLeft->setCoords(bandLo, 0.95);
    band->bottomRight->setCoords(bandHi, 0.05);
    band->setBrush(QBrush(QColor(120, 220, 120, 110)));
    band->setPen(Qt::NoPen);

    scale.minArrow = makeArrow(scale.plot, kMinMaxColor, 2);
    scale.maxArrow = makeArrow(scale.plot, kMinMaxColor, 2);
    scale.nowArrow = makeArrow(scale.plot, kNowColor, 3);
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
        s.label->setText(QString("<b>%1</b> — waiting for data").arg(name));
        s.minArrow->setVisible(false);
        s.maxArrow->setVisible(false);
        s.nowArrow->setVisible(false);
        return;
    }

    QString nowPart;
    if (haveNow) {
        nowPart = QString("   <span style='color:%1'>Now <b>%2</b> %3</span>")
                      .arg(kNowHtml, formatNum(now), unit);
    }

    QString sigmaPart;
    if (stats.n >= 2)
        sigmaPart = QString("   σ <b>%1</b> %2").arg(formatNum(stats.sigma()), unit);

    s.label->setText(QString("<b>%1</b>   "
                             "<span style='color:%2'>Min <b>%3</b> %4</span>   "
                             "<span style='color:%2'>Max <b>%5</b> %4</span>%6%7")
                         .arg(name, kMinMaxHtml,
                              formatNum(stats.min), unit,
                              formatNum(stats.max), nowPart, sigmaPart));

    if (haveNow) {
        const bool pass = (now >= s.bandLo && now <= s.bandHi);
        const QString verdict = pass
            ? QString("   <span style='color:#1c8a1c'><b>✓</b></span>")
            : QString("   <span style='color:#c01e1e'><b>✗ outside acceptable "
                      "%1..%2 %3</b></span>")
                  .arg(formatNum(s.bandLo), formatNum(s.bandHi), unit);
        s.label->setText(s.label->text() + verdict);
    }

    const double margin = (s.nomHi - s.nomLo) * 0.05;
    double axisLo = s.nomLo;
    double axisHi = s.nomHi;
    axisLo = qMin(axisLo, stats.min - margin);
    axisHi = qMax(axisHi, stats.max + margin);
    if (haveNow) {
        axisLo = qMin(axisLo, now - margin);
        axisHi = qMax(axisHi, now + margin);
    }
    s.plot->xAxis->setRange(axisLo, axisHi);

    s.minArrow->start->setCoords(stats.min, 0.9);
    s.minArrow->end->setCoords(stats.min, 0.15);
    s.maxArrow->start->setCoords(stats.max, 0.9);
    s.maxArrow->end->setCoords(stats.max, 0.15);
    s.minArrow->setVisible(true);
    s.maxArrow->setVisible(true);

    if (haveNow) {
        s.nowArrow->start->setCoords(now, 0.9);
        s.nowArrow->end->setCoords(now, 0.15);
        s.nowArrow->setVisible(true);
    } else {
        s.nowArrow->setVisible(false);
    }
}

void VarioTab::updateTimingLabel()
{
    auto fmt = [](std::optional<double> v, int dec) -> QString {
        if (!v) return "<b>—</b>";
        return QString("<b>%1</b>").arg(QString::number(*v, 'f', dec));
    };
    mTimingLabel->setText(
        QString("<b>Timing Deviation</b> &nbsp;&nbsp; "
                "Tic-Toc&nbsp;%1&nbsp;ms &nbsp;&nbsp; "
                "DiffPeriod&nbsp;%2&nbsp;ms &nbsp;&nbsp; "
                "AvgPeriod&nbsp;%3&nbsp;ms")
            .arg(fmt(mDiffTicTac, 2), fmt(mDiffPeriod, 2), fmt(mAvgPeriod, 2)));
}

void VarioTab::reset()
{
    mRate = Stats{};
    mAmp  = Stats{};
    mElapsedSec = 0.0;
    mHaveRateNow = false;
    mHaveAmpNow  = false;
    mDiffTicTac.reset();
    mDiffPeriod.reset();
    mAvgPeriod.reset();
    mElapsedLabel->setText("0:00");
    for (Scale *sc : {&mRateScale, &mAmpScale}) {
        for (QCPItemLine *a : {sc->minArrow, sc->maxArrow, sc->nowArrow})
            a->setVisible(false);
        sc->plot->xAxis->setRange(sc->nomLo, sc->nomHi);
        { int64_t _pt=TG_NOW(); sc->plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
    }
    updateScale(mRateScale, mRate, "Rate", "s/d", 1, false, 0);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, false, 0);
    updateTimingLabel();
}

void VarioTab::onMeasurement(const Measurement &m)
{
    mElapsedSec += (double)m.signal.pcm.size() / m.signal.samplesPerSecond;

    bool changed = false;
    if (m.metrics.rate.has_value()) {
        mRate.add(*m.metrics.rate);
        mRateNow = *m.metrics.rate; mHaveRateNow = true;
        changed = true;
    }
    if (m.metrics.amplitude.has_value()) {
        mAmp.add(*m.metrics.amplitude);
        mAmpNow = *m.metrics.amplitude; mHaveAmpNow = true;
        changed = true;
    }
    if (m.metrics.diffTicTac.has_value()) mDiffTicTac = m.metrics.diffTicTac;
    if (m.metrics.diffPeriod.has_value())  mDiffPeriod  = m.metrics.diffPeriod;
    if (m.metrics.avgPeriod.has_value())   mAvgPeriod   = m.metrics.avgPeriod;
    if (!changed || mPaused || !isVisible()) return;

    int sec = (int)mElapsedSec;
    mElapsedLabel->setText(QString("%1:%2").arg(sec / 60)
                               .arg(sec % 60, 2, 10, QChar('0')));
    updateScale(mRateScale, mRate, "Rate", "s/d", 1, mHaveRateNow, mRateNow);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, mHaveAmpNow, mAmpNow);
    updateTimingLabel();
    g_replotCount++;
    mRateScale.plot->replot(QCustomPlot::rpQueuedReplot);
    g_replotCount++;
    mAmpScale.plot->replot(QCustomPlot::rpQueuedReplot);
}

void VarioTab::replotAll()
{
    updateScale(mRateScale, mRate, "Rate", "s/d", 1, mHaveRateNow, mRateNow);
    updateScale(mAmpScale,  mAmp,  "Amplitude", "°", 0, mHaveAmpNow, mAmpNow);
    updateTimingLabel();
    { int64_t _pt=TG_NOW(); mRateScale.plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); }; { int64_t _pt=TG_NOW(); mAmpScale.plot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}
