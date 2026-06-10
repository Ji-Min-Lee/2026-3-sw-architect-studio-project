#pragma once
#include "BaseGraphTab.h"
#include <QLabel>

// Graph 10: Spectrogram placeholder — FFT not yet implemented.
class SpectrogramTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit SpectrogramTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QLabel *mLabel;
};
