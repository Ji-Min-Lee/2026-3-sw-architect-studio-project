#include "SpectrogramTab.h"
#include <QVBoxLayout>
#include <cmath>
#include <vector>

SpectrogramTab::SpectrogramTab(QWidget *parent) : BaseGraphTab(parent)
{
    mPlot = new QCustomPlot(this);
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mPlot);
    setLayout(lay);

    mPlot->addGraph();
    QPen pen; pen.setColor(Qt::darkCyan); pen.setWidth(1);
    mPlot->graph(0)->setPen(pen);
    mPlot->graph(0)->setName("Magnitude");
    mPlot->graph(0)->setBrush(QBrush(QColor(0, 180, 180, 40)));

    mPlot->xAxis->setLabel("Frequency (Hz)");
    mPlot->yAxis->setLabel("Magnitude");
    mPlot->yAxis->setRange(0, 1);
    mPlot->legend->setVisible(true);
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

SpectrogramTab::~SpectrogramTab()
{
    if (mCfg) { kiss_fft_free(mCfg); mCfg = nullptr; }
}

void SpectrogramTab::reset()
{
    mPlot->graph(0)->data()->clear();
    mPlot->replot();
}

// Kiss FFT 플랜 생성 — nfft가 바뀔 때만 재생성 (비용 절감)
void SpectrogramTab::prepareFft(int nfft)
{
    if (mFftLen == nfft) return;
    if (mCfg) { kiss_fft_free(mCfg); mCfg = nullptr; }
    // inverse=0 → forward FFT
    mCfg    = kiss_fft_alloc(nfft, 0, nullptr, nullptr);
    mFftLen = nfft;
}

QVector<double> SpectrogramTab::computeSpectrum(const QVector<float> &rawPcm,
                                                  int sampleRate,
                                                  double &outFreqStep)
{
    if (rawPcm.isEmpty()) return {};

    // Kiss FFT는 임의 크기를 지원하지만 2의 거듭제곱이 가장 빠름
    int n = 1;
    while (n * 2 <= rawPcm.size()) n <<= 1;

    prepareFft(n);
    if (!mCfg) return {};

    // Hann window 적용 후 입력 구성
    std::vector<kiss_fft_cpx> in(n), out(n);
    for (int i = 0; i < n; i++) {
        double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (n - 1)));
        in[i].r = rawPcm[i] * (float)window;
        in[i].i = 0.0f;
    }

    kiss_fft(mCfg, in.data(), out.data());

    // 절반만 사용 (실수 입력의 대칭성), magnitude 정규화
    int half = n / 2;
    outFreqStep = (double)sampleRate / n;

    QVector<double> mag(half);
    double maxMag = 1e-10;
    for (int i = 0; i < half; i++) {
        mag[i] = std::sqrt((double)out[i].r * out[i].r +
                           (double)out[i].i * out[i].i) / n;
        if (mag[i] > maxMag) maxMag = mag[i];
    }
    for (int i = 0; i < half; i++) mag[i] /= maxMag;

    return mag;
}

void SpectrogramTab::onMeasurement(const Measurement &m)
{
    // Lazy Pull: 탭이 안 보이면 FFT 스킵 → real-time path 보호 (QAS-1/QAS-2)
    if (!isVisible() || m.rawPcm.isEmpty()) return;

    double freqStep = 0.0;
    QVector<double> mag = computeSpectrum(m.rawPcm, m.samplesPerSecond, freqStep);
    if (mag.isEmpty()) return;

    QVector<double> freqs(mag.size());
    for (int i = 0; i < mag.size(); i++)
        freqs[i] = i * freqStep;

    mPlot->graph(0)->setData(freqs, mag);
    mPlot->xAxis->setRange(0, freqs.last());
    mPlot->yAxis->setRange(0, 1);
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
