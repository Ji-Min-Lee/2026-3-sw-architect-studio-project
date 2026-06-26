#include "SoundPrintTab.h"
#include <QVBoxLayout>

// SP-2: Map normalized event amplitude to confidence color.
// Strong (≥0.7): pure A-green / C-blue
// Medium (0.4–0.7): yellow-green / cyan
// Weak (<0.4): yellow / light-cyan
QRgb SoundPrintTab::confidenceColorA(float norm) const
{
    if (norm >= 0.7f) return qRgba(0,   255, 0,   255);
    if (norm >= 0.4f) return qRgba(150, 255, 0,   255);
    return                   qRgba(255, 220, 0,   255);
}

QRgb SoundPrintTab::confidenceColorC(float norm) const
{
    if (norm >= 0.7f) return qRgba(0,   0,   255, 255);
    if (norm >= 0.4f) return qRgba(0,   150, 255, 255);
    return                   qRgba(0,   220, 255, 255);
}

SoundPrintTab::SoundPrintTab(SoundImageWidget *widget, int sampleRate, QWidget *parent)
    : BaseGraphTab(parent), mWidget(widget), mSampleRate(sampleRate)
{
}

void SoundPrintTab::setSampleRate(int sampleRate)
{
    mSampleRate = sampleRate;
}

void SoundPrintTab::setBph(int bph)
{
    mRenderer.setBph(bph);
}

void SoundPrintTab::reset()
{
    if (!mWidget) return;

    mWidget->CreateImage();

    SoundImageRenderer::Config cfg;
    cfg.bph                          = 0.0;   // BPH unknown until synced
    cfg.sample_rate_hz               = mSampleRate;
    cfg.sound_color                  = qRgba(255, 0, 0, 255);
    cfg.background_color             = qRgba(255, 255, 255, 255);
    cfg.vertical_time_direction      = SoundImageRenderer::TimeStartsAtTopMovesDown;
    cfg.warmup_columns               = 2;
    cfg.anchor_columns               = 12;
    cfg.gamma                        = 0.5f;
    cfg.per_column_normalize         = true;
    cfg.live_preview_current_column  = true;

    mPeakAmplitude = 0.0f;
    mRenderer.initialize(mWidget->GetImage(), cfg);
    mRenderer.reset();

    mWidget->update();
}

void SoundPrintTab::onMeasurement(const Measurement &m)
{
    if (mPaused) return;
    if (!mWidget) return;

    // BPH 확정 시 렌더러에 알림 (sync 직후 1회)
    if (m.synced && mRenderer.currentBph() <= 0.0)
        mRenderer.setBph(m.detectedBph);

    // ① 원본 PCM → 비트맵 업데이트
    mRenderer.processSamples(m.signal.rawPcm.data(), m.signal.rawPcm.size());

    // ② A/C event markers with confidence color (SP-2)
    for (const AcousticEvent &ev : m.events) {
        if (ev.peakValue > mPeakAmplitude)
            mPeakAmplitude = ev.peakValue;
        float norm = (mPeakAmplitude > 0.0f) ? ev.peakValue / mPeakAmplitude : 1.0f;
        if (ev.isA)
            mRenderer.markAEventAbsoluteSampleIndex(
                (quint64)ev.samplePos, confidenceColorA(norm), 3);
        else
            mRenderer.markCEventAbsoluteSampleIndex(
                (quint64)ev.samplePos, confidenceColorC(norm), 3);
    }

    // ③ 화면 갱신
    mWidget->DrawImage();
}
