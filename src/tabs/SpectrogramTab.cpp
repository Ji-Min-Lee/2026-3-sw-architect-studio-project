#include "SpectrogramTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <cmath>
#include <vector>

SpectrogramTab::SpectrogramTab(QWidget *parent) : BaseGraphTab(parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *controls = new QHBoxLayout;
    controls->setContentsMargins(4, 4, 4, 0);
    controls->addWidget(new QLabel("Window:", this));
    mWindowCombo = new QComboBox(this);
    mWindowCombo->addItems({"Last 2 s", "Last 6 s", "Last 12 s"});
    mWindowCombo->setCurrentIndex(1);
    controls->addWidget(mWindowCombo);
    controls->addStretch(1);
    lay->addLayout(controls);

    mPlot = new QCustomPlot(this);
    lay->addWidget(mPlot, 1);

    mMap = new QCPColorMap(mPlot->xAxis, mPlot->yAxis);
    mMap->data()->setSize(kCols, kRows);
    mMap->data()->setRange(QCPRange(-12.0, 0.0), QCPRange(0, mSps / 2.0));

    mScale = new QCPColorScale(mPlot);
    mPlot->plotLayout()->addElement(0, 1, mScale);
    mScale->setType(QCPAxis::atRight);
    mScale->axis()->setLabel("Signal strength (dB)");
    mMap->setColorScale(mScale);
    mMap->setGradient(QCPColorGradient::gpSpectrum);
    mMap->setDataRange(QCPRange(-70, -10));

    mPlot->xAxis->setLabel("Time (s)");
    mPlot->yAxis->setLabel("Frequency (Hz)");
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    connect(mWindowCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { rebuildMap(); });
}

SpectrogramTab::~SpectrogramTab()
{
    if (mCfg) { kiss_fft_free(mCfg); mCfg = nullptr; }
}

void SpectrogramTab::reset()
{
    mColumns.clear();
    mMap->data()->fill(-70);
    mPlot->replot();
}

// Kiss FFT 플랜 생성 — nfft가 바뀔 때만 재생성 (비용 절감)
void SpectrogramTab::prepareFft(int nfft)
{
    if (mFftLen == nfft) return;
    if (mCfg) { kiss_fft_free(mCfg); mCfg = nullptr; }
    mCfg    = kiss_fft_alloc(nfft, 0, nullptr, nullptr);
    mFftLen = nfft;
}

// One spectrogram column: Hann window → FFT → kRows aggregated dB bins
QVector<double> SpectrogramTab::computeColumn(const QVector<float> &rawPcm)
{
    int n = 1;
    while (n * 2 <= rawPcm.size()) n <<= 1;
    prepareFft(n);
    if (!mCfg) return {};

    std::vector<kiss_fft_cpx> in(n), out(n);
    for (int i = 0; i < n; i++) {
        double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (n - 1)));
        in[i].r = rawPcm[i] * (float)window;
        in[i].i = 0.0f;
    }
    kiss_fft(mCfg, in.data(), out.data());

    const int half = n / 2;
    QVector<double> col(kRows, -70.0);
    const int binsPerRow = qMax(1, half / kRows);
    for (int row = 0; row < kRows; row++) {
        double maxMag = 0.0;
        int b0 = row * binsPerRow;
        int b1 = qMin(half, b0 + binsPerRow);
        for (int b = b0; b < b1; b++) {
            double mag = std::sqrt((double)out[b].r * out[b].r +
                                   (double)out[b].i * out[b].i) / n;
            maxMag = qMax(maxMag, mag);
        }
        col[row] = 20.0 * std::log10(maxMag + 1e-10);
    }
    return col;
}

void SpectrogramTab::rebuildMap()
{
    static const double windows[] = {2.0, 6.0, 12.0};
    double windowSec = windows[qBound(0, mWindowCombo->currentIndex(), 2)];

    mMap->data()->setSize(kCols, kRows);
    mMap->data()->setRange(QCPRange(-kCols * mBlockSec, 0.0),
                           QCPRange(0, mSps / 2.0));
    for (int c = 0; c < kCols; c++) {
        int srcIdx = mColumns.size() - kCols + c;
        for (int r = 0; r < kRows; r++)
            mMap->data()->setCell(c, r,
                srcIdx >= 0 ? mColumns[srcIdx][r] : -70.0);
    }
    mPlot->xAxis->setRange(-windowSec, 0.0);
    mPlot->yAxis->setRange(0, mSps / 2.0);
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void SpectrogramTab::onMeasurement(const Measurement &m)
{
    // Lazy Pull: 탭이 안 보이면 FFT 스킵 → real-time path 보호 (QAS-1/QAS-2)
    if (!isVisible() || m.rawPcm.isEmpty()) return;

    mSps      = m.samplesPerSecond;
    mBlockSec = (double)m.rawPcm.size() / m.samplesPerSecond;

    QVector<double> col = computeColumn(m.rawPcm);
    if (col.isEmpty()) return;
    mColumns.append(col);
    while (mColumns.size() > kCols) mColumns.removeFirst();

    if (mPaused) return;
    rebuildMap();
}
