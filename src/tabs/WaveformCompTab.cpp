#include "WaveformCompTab.h"
#include <QVBoxLayout>

WaveformCompTab::WaveformCompTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    mValuesLabel = new QLabel(this);
    mValuesLabel->setAlignment(Qt::AlignHCenter);
    lay->addWidget(mValuesLabel);

    mPlot = new QCustomPlot(this);
    lay->addWidget(mPlot, 1);

    mPlot->addGraph();
    mPlot->graph(0)->setPen(QPen(Qt::red));
    mPlot->graph(0)->setName("Tic");

    mPlot->addGraph();
    mPlot->graph(1)->setPen(QPen(Qt::blue));
    mPlot->graph(1)->setName("Toc");

    // Lanes for previous beat pairs (lighter pens, no legend entries)
    for (int lane = 0; lane < kLanes; lane++) {
        QColor tic(255, 120, 120), toc(120, 120, 255);
        tic.setAlpha(200 - lane * 35);
        toc.setAlpha(200 - lane * 35);
        auto *g1 = mPlot->addGraph();
        g1->setPen(QPen(tic));
        g1->removeFromLegend();
        auto *g2 = mPlot->addGraph();
        g2->setPen(QPen(toc));
        g2->removeFromLegend();
        mLaneGraphs << g1 << g2;
    }

    mPlot->xAxis->setLabel("Sample offset");
    mPlot->yAxis->setLabel("Amplitude (lanes: previous beats below)");
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    mValuesLabel->setText("RATE --- s/d   BEAT ERROR --- ms   BEAT ----- bph");
}

void WaveformCompTab::reset()
{
    mHaveTic = false;
    mTicXs.clear();
    mTicYs.clear();
    mHistory.clear();
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
    for (QCPGraph *g : mLaneGraphs) g->data()->clear();
    mValuesLabel->setText("RATE --- s/d   BEAT ERROR --- ms   BEAT ----- bph");
    mPlot->replot();
}

// rawPcm에서 samplePos 주변 kWindowSamples개 샘플을 꺼냄.
// samplePos와 tickStart는 절대 샘플 인덱스이므로
// BPH/SPS에 관계없이 블록 내 올바른 위치를 찾을 수 있음.
void WaveformCompTab::extractWindow(const QVector<float> &rawPcm,
                                    double samplePos, uint64_t tickStart,
                                    QVector<double> &outXs, QVector<double> &outYs) const
{
    outXs.clear();
    outYs.clear();
    outXs.reserve(kWindowSamples);
    outYs.reserve(kWindowSamples);

    // samplePos를 블록 내 상대 인덱스로 변환
    int center = (int)(samplePos - (double)tickStart);
    int start  = center - kWindowSamples / 2;

    for (int i = 0; i < kWindowSamples; i++) {
        int idx = start + i;
        if (idx < 0 || idx >= rawPcm.size()) continue;
        outXs.append(i - kWindowSamples / 2);
        outYs.append(rawPcm[idx]);
    }
}

void WaveformCompTab::updateGuides()
{
    // Vertical guide markers every 1 ms across the comparison window
    int spacing = qMax(1, mSps / 1000);
    int needed  = kWindowSamples / spacing + 1;
    while (mGuides.size() < needed) {
        auto *g = new QCPItemLine(mPlot);
        g->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
        g->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
        g->setPen(QPen(QColor(0, 160, 0, 90), 1, Qt::DashLine));
        mGuides.append(g);
    }
    int half = kWindowSamples / 2;
    for (int i = 0; i < mGuides.size(); i++) {
        int x = -half + i * spacing;
        mGuides[i]->start->setCoords(x, 0.0);
        mGuides[i]->end->setCoords(x, 1.0);
        mGuides[i]->setVisible(x <= half);
    }
}

void WaveformCompTab::redrawLanes()
{
    for (int lane = 0; lane < kLanes; lane++) {
        QCPGraph *g1 = mLaneGraphs[lane * 2];
        QCPGraph *g2 = mLaneGraphs[lane * 2 + 1];
        if (lane >= mHistory.size()) {
            g1->data()->clear();
            g2->data()->clear();
            continue;
        }
        const BeatPair &bp = mHistory[lane];
        double off = -kLaneSpacing * (lane + 1);
        auto shifted = [&](const QVector<double> &ys) {
            QVector<double> out(ys.size());
            for (int i = 0; i < ys.size(); i++) out[i] = ys[i] + off;
            return out;
        };
        g1->setData(bp.ticXs, shifted(bp.ticYs));
        g2->setData(bp.tocXs, shifted(bp.tocYs));
    }
}

void WaveformCompTab::onMeasurement(const Measurement &m)
{
    if (m.rawPcm.isEmpty()) return;
    mSps = m.samplesPerSecond;

    bool changed = false;

    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            // A event: 파형 윈도우를 멤버에 복사해서 보관.
            // 다음 블록에서 C event가 와도 이 복사본을 사용할 수 있음.
            extractWindow(m.rawPcm, ev.samplePos, m.graphTickStart,
                          mTicXs, mTicYs);
            mHaveTic = true;

            // Tic 그래프 즉시 업데이트
            mPlot->graph(0)->setData(mTicXs, mTicYs);
            changed = true;

        } else if (mHaveTic) {
            // C event: 이번 블록 rawPcm에서 Toc 파형 꺼냄.
            QVector<double> tocXs, tocYs;
            extractWindow(m.rawPcm, ev.samplePos, m.graphTickStart,
                          tocXs, tocYs);

            mPlot->graph(0)->setData(mTicXs, mTicYs);
            mPlot->graph(1)->setData(tocXs, tocYs);

            // Completed pair → push into the lane history
            BeatPair bp{mTicXs, mTicYs, tocXs, tocYs};
            mHistory.prepend(bp);
            while (mHistory.size() > kLanes) mHistory.removeLast();
            changed = true;
        }
    }

    if (!changed || mPaused) return;

    mValuesLabel->setText(QString("<b>RATE %1 s/d   BEAT ERROR %2 ms   BEAT %3 bph</b>")
                              .arg(m.rateValid ? QString::number(m.rateErrorSpd, 'f', 1) : "---",
                                   m.beatErrorValid ? QString::number(m.beatErrorMs, 'f', 2) : "---",
                                   m.synced ? QString::number(m.detectedBph) : "-----"));
    redrawLanes();
    updateGuides();
    mPlot->yAxis->setRange(-kLaneSpacing * (kLanes + 0.5), kLaneSpacing);
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
