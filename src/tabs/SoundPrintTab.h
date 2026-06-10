#pragma once
#include "BaseGraphTab.h"
#include "SoundImageWidget.h"

// Graph 3: Sound print (spectrogram-style bitmap from raw PCM).
// SoundImageWidget is injected from MainWindow.ui.
class SoundPrintTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SoundPrintTab(SoundImageWidget *widget, QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    SoundImageWidget *mWidget;
};
