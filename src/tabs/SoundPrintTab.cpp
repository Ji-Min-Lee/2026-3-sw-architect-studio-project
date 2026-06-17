#include "SoundPrintTab.h"
#include <QVBoxLayout>

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
    cfg.live_preview_current_column  = true;
    cfg.beat_grid_enabled            = true;
    cfg.beat_grid_color              = qRgba(0, 0, 220, 100);   // blue  — bucket=0 renders where C events appear after centering
    cfg.beat_grid_half_color         = qRgba(0, 200, 0, 160);   // green — bucket=height/2 renders where A events appear after centering

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
    mRenderer.processSamples(m.rawPcm.data(), m.rawPcm.size());

    // ② A/C 이벤트 마커
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA)
            mRenderer.markAEventAbsoluteSampleIndex(
                (quint64)ev.samplePos, qRgba(0, 255, 0, 255), 3);
        else
            mRenderer.markCEventAbsoluteSampleIndex(
                (quint64)ev.samplePos, qRgba(0, 0, 255, 255), 3);
    }

    // ③ 화면 갱신
    mWidget->DrawImage();
}
