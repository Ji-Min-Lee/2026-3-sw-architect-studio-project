#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include "kiss_fft.h"

// Graph 10: Frequency spectrum (magnitude vs Hz).
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
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QCustomPlot   *mPlot;
    kiss_fft_cfg   mCfg    = nullptr;  // Kiss FFT 플랜 (캐시)
    int            mFftLen = 0;        // 현재 플랜 크기

    // Kiss FFT 플랜을 nfft 크기로 (재)생성
    void prepareFft(int nfft);

    // rawPcm → magnitude spectrum, freqStep(Hz/bin) 반환
    QVector<double> computeSpectrum(const QVector<float> &rawPcm,
                                    int sampleRate,
                                    double &outFreqStep);
};
