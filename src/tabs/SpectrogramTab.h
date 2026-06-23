#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include "kiss_fft.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <vector>

// Graph 10: Time-Frequency Spectrogram Display (project plan Figure 16).
//
// Ported from commit 6535adf: rolling HPF PCM buffer (F1 / filtered_pcm),
// FFT 1024 / hop 512, column-scrolling QCPColorMap heatmap.
//
// Lazy Pull — FFT only when tab is visible (QAS-1/QAS-2).
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
    enum class ViewMode { Seconds, LastBeat };

    // FFT 1024 / hop 512; column count is derived from the visible time window
    // so each column spans one hop (~10.7 ms @ 48 kHz) on the X axis.
    static constexpr int    kFftSize        = 1024;
    static constexpr int    kHopSize        = 512;
    static constexpr int    kMaxTimeColumns = 512;
    static constexpr double kMaxDisplayHz   = 20000.0;
    // Figure 16 reference: viridis colormap, fixed colorbar −70…−10 dB
    static constexpr double kDbMin        = -70.0;
    static constexpr double kDbMax        = -10.0;
    static constexpr double kMaxBufferSec = 12.0;

    void prepareFft();
    void ensureMapSize();
    int  freqBinCount() const;
    double hopMs() const;
    double windowMs() const;
    int  timeColumnCount() const;
    double binMagnitudeToDb(double magnitude, int bin) const;
    std::vector<double> computeMagnitudes(const float *pcm) const;
    void shiftColumnsAndAppend(const std::vector<double> &magnitudes);
    void processPendingColumns();
    void rebuildAxisRanges();
    void rebuildLastBeatView();
    void updateBeatMarkers(const Measurement &m);
    double truePeakDbfs(const QVector<float> &pcm) const;
    void updateColorRange();
    static QCPColorGradient spectrogramGradient();

    QCustomPlot      *mPlot = nullptr;
    QCPColorMap      *mMap = nullptr;
    QCPColorScale    *mScale = nullptr;
    QLabel           *mTitleLabel = nullptr;
    QCheckBox        *mPeakMeterCheck = nullptr;
    QLabel           *mPeakDbLabel = nullptr;
    QProgressBar     *mPeakBar = nullptr;
    QButtonGroup     *mModeGroup = nullptr;
    QDoubleSpinBox   *mWindowSpin = nullptr;
    QCheckBox        *mAutoScaleCheck = nullptr;

    ViewMode mViewMode = ViewMode::Seconds;

    std::vector<float> mPcmBuffer;
    uint64_t           mBufferStartTick = 0;

    int      mSampleRate = 48000;
    int      mFreqBins   = 171;
    double   mWindowSec  = 1.0;
    bool     mHaveLastBeat = false;
    uint64_t mLastBeatTick = 0;
    double   mBeatPeriodMs = 1000.0;
    double   mLastPeakDbfs = -100.0;

    kiss_fft_cfg     mCfg = nullptr;
    std::vector<float> mHannWindow;
};
