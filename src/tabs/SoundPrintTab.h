#pragma once
#include "BaseGraphTab.h"
#include "SoundImageWidget.h"
#include "SoundImageRenderer.h"

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
private:
    SoundImageWidget   *mWidget;
    SoundImageRenderer  mRenderer;   // 탭이 직접 소유
    int                 mSampleRate = 48000;
};
