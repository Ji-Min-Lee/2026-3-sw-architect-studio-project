#include "SoundPrintTab.h"
#include "SoundImageRenderer.h"
#include <QVBoxLayout>

SoundPrintTab::SoundPrintTab(SoundImageWidget *widget, QWidget *parent)
    : BaseGraphTab(parent), mWidget(widget)
{
    if (!mWidget) return;
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    // Widget is already placed by MainWindow.ui; just reparent if needed
    lay->addWidget(mWidget);
    setLayout(lay);
}

void SoundPrintTab::reset()
{
    if (mWidget) { mWidget->CreateImage(); mWidget->update(); }
}

void SoundPrintTab::onMeasurement(const Measurement &m)
{
    if (!mWidget || m.rawPcm.isEmpty()) return;
    // SoundImageRenderer is owned by MainWindow; this tab just triggers a repaint.
    // The actual rendering pipeline (raw PCM → SoundImageRenderer → SoundImageWidget)
    // remains in MainWindow to avoid duplicating the renderer state.
    Q_UNUSED(m)
    mWidget->DrawImage();
    mWidget->update();
}
