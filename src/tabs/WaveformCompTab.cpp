#include "WaveformCompTab.h"
#include <QVBoxLayout>

WaveformCompTab::WaveformCompTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    mPlot->graph(0)->setPen(QPen(Qt::red));
    mPlot->graph(0)->setName("Tic");

    mPlot->addGraph();
    mPlot->graph(1)->setPen(QPen(Qt::blue));
    mPlot->graph(1)->setName("Toc");

    mPlot->xAxis->setLabel("Sample offset");
    mPlot->yAxis->setLabel("Amplitude");
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void WaveformCompTab::reset()
{
    mHaveTic = false;
    mTicXs.clear();
    mTicYs.clear();
    mPlot->graph(0)->data()->clear();
    mPlot->graph(1)->data()->clear();
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

void WaveformCompTab::onMeasurement(const Measurement &m)
{
    if (m.rawPcm.isEmpty()) return;

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
            // mTicXs/mTicYs는 이전 블록에서 복사해둔 Tic 파형.
            QVector<double> tocXs, tocYs;
            extractWindow(m.rawPcm, ev.samplePos, m.graphTickStart,
                          tocXs, tocYs);

            mPlot->graph(0)->setData(mTicXs, mTicYs);
            mPlot->graph(1)->setData(tocXs, tocYs);
            changed = true;
        }
    }

    if (!changed) return;
    mPlot->yAxis->rescale();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
