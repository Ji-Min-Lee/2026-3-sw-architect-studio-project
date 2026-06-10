#include "SpectrogramTab.h"
#include <QVBoxLayout>

SpectrogramTab::SpectrogramTab(QWidget *parent) : BaseGraphTab(parent)
{
    mLabel = new QLabel("Spectrogram — FFT not yet implemented", this);
    mLabel->setAlignment(Qt::AlignCenter);
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(mLabel);
    setLayout(lay);
}

void SpectrogramTab::reset() {}

void SpectrogramTab::onMeasurement(const Measurement &) {}
