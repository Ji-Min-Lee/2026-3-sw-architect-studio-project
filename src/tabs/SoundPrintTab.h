#pragma once
#include "BaseGraphTab.h"
#include "SoundImageWidget.h"
#include "SoundImageRenderer.h"
#include <deque>
#include <QVector>

// Graph 3: Sound print (spectrogram-style bitmap from raw PCM).
// Owns SoundImageRenderer — fully self-contained rendering (Ownership Transfer).
// SoundImageWidget is injected from MainWindow.ui (display target only).
class SoundPrintTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SoundPrintTab(SoundImageWidget *widget, int sampleRate,
                           QWidget *parent = nullptr);
    void reset() override;
    void setSampleRate(int sampleRate);
    void setBph(int bph);
public slots:
    void onMeasurement(const Measurement &m) override;
    void onColumnClicked(int x);
private:
    QRgb confidenceColorA(float norm) const;
    QRgb confidenceColorC(float norm) const;
    void showWaveformPopup(int columnX);
    QPixmap renderWaveformPixmap(const QVector<float> &wave, int numSamples,
                                 qint64 beatIndex, double ms, bool hasData);

    struct PcmChunk {
        quint64       startSample = 0;
        QVector<float> pcm;
    };

    SoundImageWidget   *mWidget;
    SoundImageRenderer  mRenderer;
    int                 mSampleRate = 48000;
    float               mPeakAmplitude = 0.0f;

    // PCM ring buffer — covers full visible image (imageWidth × samplesPerColumn)
    quint64 maxBufferedSamples() const;
    std::deque<PcmChunk> mPcmBuffer;
    quint64               mPcmBufferedSamples = 0;
};
