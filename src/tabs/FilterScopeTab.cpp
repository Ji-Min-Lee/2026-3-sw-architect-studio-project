#include "FilterScopeTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

namespace {
constexpr int    kMovingAvgWin = 32;
constexpr double kFallDecay    = 0.85;  // attenuation applied to falling slopes

const char *kHints[] = {
    "F0 — closest representation of the captured watch signal",
    "F1 — smoothed envelope; low-amplitude components may become less visible",
    "F2 — rising slopes emphasized: T3 (and often T2) stand out",
    "F3 — upper portion only with rising-edge emphasis: useful for T1 / T3",
};
} // namespace

FilterScopeTab::FilterScopeTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);

    auto *controls = new QHBoxLayout;
    controls->addWidget(new QLabel("Filter view:", this));
    mFilterCombo = new QComboBox(this);
    mFilterCombo->addItems({"F0 — raw mirrored", "F1 — moving average",
                            "F2 — rising emphasis", "F3 — upper + rising"});
    controls->addWidget(mFilterCombo);
    mHintLabel = new QLabel(kHints[0], this);
    controls->addWidget(mHintLabel, 1);
    lay->addLayout(controls);

    mPlot = new QCustomPlot(this);
    mPlot->addGraph();                       // signal
    mPlot->graph(0)->setPen(QPen(QColor(190, 170, 30)));
    mPlot->graph(0)->setBrush(QBrush(QColor(230, 210, 70, 110)));
    mPlot->addGraph();                       // mirror (F0 only)
    mPlot->graph(1)->setPen(QPen(QColor(190, 170, 30)));
    mPlot->graph(1)->setBrush(QBrush(QColor(230, 210, 70, 110)));
    mPlot->xAxis->setLabel("Time (ms)");
    mPlot->yAxis->setLabel("Amplitude");
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    lay->addWidget(mPlot, 1);

    connect(mFilterCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int i) {
                mHintLabel->setText(kHints[qBound(0, i, 3)]);
                redraw();
            });
}

QVector<double> FilterScopeTab::applyFilter(const QVector<float> &raw) const
{
    const int n = raw.size();
    const int mode = mFilterCombo->currentIndex();

    double mean = 0;
    for (float v : raw) mean += v;
    mean /= qMax(1, n);

    QVector<double> out(n);

    if (mode == 0) {                               // F0: centered raw
        for (int i = 0; i < n; i++) out[i] = raw[i] - mean;
        return out;
    }

    // F1 base: moving average of |signal - mean|
    QVector<double> f1(n);
    double acc = 0;
    for (int i = 0; i < n; i++) {
        acc += std::abs(raw[i] - mean);
        if (i >= kMovingAvgWin) acc -= std::abs(raw[i - kMovingAvgWin] - mean);
        f1[i] = acc / qMin(i + 1, kMovingAvgWin);
    }
    if (mode == 1) return f1;

    if (mode == 2) {                               // F2: rising emphasis on F1
        double prev = 0;
        for (int i = 0; i < n; i++) {
            out[i] = (f1[i] >= prev) ? f1[i] : prev * kFallDecay;
            prev = out[i];
        }
        return out;
    }

    // F3: upper portion relative to average, rising-edge emphasis
    QVector<double> upper(n);
    for (int i = 0; i < n; i++) upper[i] = qMax(0.0, (double)raw[i] - mean);
    double prev = 0;
    for (int i = 0; i < n; i++) {
        out[i] = (upper[i] >= prev) ? upper[i] : prev * kFallDecay;
        prev = out[i];
    }
    return out;
}

void FilterScopeTab::redraw()
{
    if (!mHaveData) return;
    const Measurement &m = mLatest;
    const int n = m.rawPcm.size();
    if (n == 0) return;

    QVector<double> ys = applyFilter(m.rawPcm);
    QVector<double> xs(n);
    for (int i = 0; i < n; i++) xs[i] = (double)i / m.samplesPerSecond * 1000.0;

    const bool mirrored = mFilterCombo->currentIndex() == 0;
    mPlot->graph(0)->setData(xs, ys, true);
    if (mirrored) {
        QVector<double> neg(n);
        for (int i = 0; i < n; i++) neg[i] = -std::abs(ys[i]);
        QVector<double> pos(n);
        for (int i = 0; i < n; i++) pos[i] = std::abs(ys[i]);
        mPlot->graph(0)->setData(xs, pos, true);
        mPlot->graph(1)->setData(xs, neg, true);
    } else {
        mPlot->graph(1)->data()->clear();
    }

    // A/C markers for this block
    for (auto *mk : mMarkers) mk->setVisible(false);
    int used = 0;
    for (const AcousticEvent &ev : m.events) {
        double xMs = (ev.samplePos - (double)m.graphTickStart)
                     / m.samplesPerSecond * 1000.0;
        if (xMs < 0 || xMs > xs.last()) continue;
        if (used >= mMarkers.size()) {
            auto *mk = new QCPItemLine(mPlot);
            mk->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
            mk->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
            mMarkers.append(mk);
        }
        QCPItemLine *mk = mMarkers[used++];
        mk->setPen(QPen(ev.isA ? Qt::darkGreen : Qt::red, 1.5, Qt::DashLine));
        mk->start->setCoords(xMs, 0.0);
        mk->end->setCoords(xMs, 1.0);
        mk->setVisible(true);
    }

    mPlot->xAxis->setRange(0, xs.last());
    mPlot->yAxis->rescale();
    if (mirrored) mPlot->yAxis->setRange(-mPlot->yAxis->range().upper,
                                          mPlot->yAxis->range().upper);
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void FilterScopeTab::onMeasurement(const Measurement &m)
{
    if (m.rawPcm.isEmpty()) return;
    mLatest   = m;
    mHaveData = true;
    if (mPaused || !isVisible()) return;   // lazy pull — filtering is per-block work
    redraw();
}

void FilterScopeTab::reset()
{
    mHaveData = false;
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    for (auto *mk : mMarkers) mk->setVisible(false);
    mPlot->replot();
}
