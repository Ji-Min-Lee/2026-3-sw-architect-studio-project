#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include "kiss_fft.h"
#include <QComboBox>

// Graph 10: Time-Frequency Spectrogram Display (project plan Figure 16).
//
// 2D heatmap: x = time (rolling window), y = frequency (Hz),
// color = signal strength in dB, with a color scale legend.
// A window selector lets the user inspect the most recent 2 / 6 / 12 s.
//
// 방식: Lazy Pull — isVisible() 일 때만 FFT 계산 (QAS-1/QAS-2).
// FFT: Kiss FFT — FFTW 대비 경량, 임의 크기 지원, 외부 라이브러리.
class SpectrogramTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SpectrogramTab(QWidget *parent = nullptr);
    ~SpectrogramTab() override;
    void reset() override;

    QCPColorMap *colorMap() const { return mMap; }

public slots:
    void onMeasurement(const Measurement &m) override;

private:
    static constexpr int kRows = 128;   // frequency bins displayed
    static constexpr int kCols = 144;   // time columns kept (~12 s at 85 ms/block)

    void prepareFft(int nfft);
    QVector<double> computeColumn(const QVector<float> &rawPcm); // kRows dB values
    void rebuildMap();

    QCustomPlot   *mPlot;
    QCPColorMap   *mMap;
    QCPColorScale *mScale;
    QComboBox     *mWindowCombo;

    QList<QVector<double>> mColumns;    // newest last
    double         mBlockSec = 0.085;
    int            mSps      = 48000;

    kiss_fft_cfg   mCfg    = nullptr;
    int            mFftLen = 0;
};
