#include "SoundPrintTab.h"
#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QPushButton>
#include <QDialogButtonBox>
#include <cmath>

// SP-2: Map normalized event amplitude to confidence color.
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
    if (mWidget)
        connect(mWidget, &SoundImageWidget::clicked, this, &SoundPrintTab::onColumnClicked);
}

void SoundPrintTab::setSampleRate(int sampleRate) { mSampleRate = sampleRate; }
void SoundPrintTab::setBph(int bph)               { mRenderer.setBph(bph); }

quint64 SoundPrintTab::maxBufferedSamples() const
{
    // Cover the full visible image; fall back to 120s if BPH not yet known
    if (mRenderer.bphValid() && mRenderer.samplesPerColumnExact() > 0.0)
        return static_cast<quint64>(mRenderer.imageWidth()
                                    * mRenderer.samplesPerColumnExact()) + 4096;
    return static_cast<quint64>(mSampleRate) * 120;
}

void SoundPrintTab::reset()
{
    if (!mWidget) return;

    mWidget->CreateImage();

    SoundImageRenderer::Config cfg;
    cfg.bph                          = 0.0;
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

    mPcmBuffer.clear();
    mPcmBufferedSamples = 0;

    mWidget->update();
}

void SoundPrintTab::onMeasurement(const Measurement &m)
{
    if (mPaused) return;
    if (!mWidget) return;

    if (m.synced && mRenderer.currentBph() <= 0.0)
        mRenderer.setBph(m.detectedBph);

    mRenderer.processSamples(m.signal.rawPcm.data(), m.signal.rawPcm.size());

    // PCM ring buffer
    if (!m.signal.rawPcm.isEmpty()) {
        PcmChunk chunk;
        chunk.startSample = mRenderer.nextInputAbsoluteSampleIndex()
                            - static_cast<quint64>(m.signal.rawPcm.size());
        chunk.pcm = m.signal.rawPcm;
        mPcmBuffer.push_back(std::move(chunk));
        mPcmBufferedSamples += m.signal.rawPcm.size();

        while (mPcmBufferedSamples > maxBufferedSamples() && !mPcmBuffer.empty()) {
            mPcmBufferedSamples -= mPcmBuffer.front().pcm.size();
            mPcmBuffer.pop_front();
        }
    }

    // A/C event markers (SP-2)
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

    mWidget->DrawImage();
}

void SoundPrintTab::onColumnClicked(int x)
{
    quint64 start = 0, end = 0;
    if (!mRenderer.sampleRangeForColumn(x, start, end)) return;
    showWaveformPopup(x);
}

// ── Waveform rendering ────────────────────────────────────────

QPixmap SoundPrintTab::renderWaveformPixmap(const QVector<float> &wave, int numSamples,
                                             qint64 beatIndex, double ms, bool hasData)
{
    const int W = 600, H = 200;
    QPixmap pix(W, H);
    pix.fill(QColor(20, 20, 20));
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (hasData && numSamples > 0) {
        float peak = 1e-9f;
        int peakIdx = 0;
        for (int i = 0; i < numSamples; ++i) {
            float a = std::abs(wave[i]);
            if (a > peak) { peak = a; peakIdx = i; }
        }

        int half = numSamples / 2;
        int viewStart = qMax(0, peakIdx - half / 2);
        int viewEnd   = qMin(numSamples, viewStart + half);
        viewStart     = qMax(0, viewEnd - half);
        int viewCount = viewEnd - viewStart;

        const float cy    = H / 2.0f;
        const float scale = (H / 2.0f - 4) / peak;

        p.setPen(QPen(QColor(80, 200, 120), 1));
        for (int i = viewStart + 1; i < viewEnd; ++i) {
            float x0 = static_cast<float>(i - 1 - viewStart) / viewCount * W;
            float x1 = static_cast<float>(i     - viewStart) / viewCount * W;
            p.drawLine(QPointF(x0, cy - wave[i-1]*scale),
                       QPointF(x1, cy - wave[i  ]*scale));
        }

        float peakX = static_cast<float>(peakIdx - viewStart) / viewCount * W;
        p.setPen(QPen(QColor(255, 100, 100, 150), 1, Qt::DashLine));
        p.drawLine(QPointF(peakX, 0), QPointF(peakX, H));
    } else {
        p.setPen(Qt::gray);
        p.drawText(QRect(0,0,W,H), Qt::AlignCenter, "No PCM data in buffer");
    }

    p.setPen(QColor(60, 60, 60));
    p.drawLine(0, H/2, W, H/2);
    p.setPen(Qt::white);
    p.drawText(4, 14, QString("Beat #%1   %2 ms   %3 samples")
               .arg(beatIndex).arg(ms, 0, 'f', 1).arg(numSamples));
    p.end();
    return pix;
}

// ── Popup with Prev / Next navigation ────────────────────────

void SoundPrintTab::showWaveformPopup(int columnX)
{
    const int imgW = mRenderer.imageWidth();

    auto getWave = [&](int col, QVector<float> &wave, qint64 &beatIdx,
                        double &ms, bool &hasData) -> bool {
        quint64 start = 0, end = 0;
        if (!mRenderer.sampleRangeForColumn(col, start, end)) return false;
        int numSamples = static_cast<int>(end - start);
        wave.assign(numSamples, 0.0f);
        hasData = false;
        for (const PcmChunk &chunk : mPcmBuffer) {
            quint64 cEnd = chunk.startSample + chunk.pcm.size();
            if (cEnd <= start || chunk.startSample >= end) continue;
            quint64 oStart = qMax(start, chunk.startSample);
            quint64 oEnd   = qMin(end,   cEnd);
            int dstOff = static_cast<int>(oStart - start);
            int srcOff = static_cast<int>(oStart - chunk.startSample);
            std::copy(chunk.pcm.constData() + srcOff,
                      chunk.pcm.constData() + srcOff + static_cast<int>(oEnd - oStart),
                      wave.data() + dstOff);
            hasData = true;
        }
        beatIdx = mRenderer.currentBeatIndex() - (mRenderer.currentColumn() - col);
        ms      = static_cast<double>(numSamples) / mSampleRate * 1000.0;
        return true;
    };

    // ── Build dialog ──────────────────────────────────────────
    QDialog dlg(mWidget ? mWidget->window() : nullptr);
    dlg.setWindowTitle("Waveform");
    dlg.setFixedSize(640, 120 + 200);

    auto *imgLabel = new QLabel(&dlg);
    imgLabel->setFixedSize(600, 200);
    imgLabel->setAlignment(Qt::AlignCenter);

    auto *titleLabel = new QLabel(&dlg);
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *prevBtn = new QPushButton("◀ Prev", &dlg);
    auto *nextBtn = new QPushButton("Next ▶", &dlg);
    auto *closeBtn = new QPushButton("Close", &dlg);

    auto *navLayout = new QHBoxLayout;
    navLayout->addWidget(prevBtn);
    navLayout->addStretch();
    navLayout->addWidget(titleLabel);
    navLayout->addStretch();
    navLayout->addWidget(nextBtn);

    auto *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->addWidget(imgLabel, 0, Qt::AlignHCenter);
    layout->addLayout(navLayout);
    layout->addWidget(closeBtn, 0, Qt::AlignHCenter);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    // ── Update function ───────────────────────────────────────
    int curCol = columnX;
    auto update = [&]() {
        QVector<float> wave;
        qint64 beatIdx = 0;
        double ms = 0.0;
        bool hasData = false;

        if (!getWave(curCol, wave, beatIdx, ms, hasData)) return;

        imgLabel->setPixmap(renderWaveformPixmap(wave, wave.size(), beatIdx, ms, hasData));
        titleLabel->setText(QString("Beat #%1").arg(beatIdx));
        dlg.setWindowTitle(QString("Waveform — Beat #%1").arg(beatIdx));

        // Disable at boundaries (check adjacent columns for valid data)
        quint64 dummy1, dummy2;
        prevBtn->setEnabled(curCol > 0 && mRenderer.sampleRangeForColumn(curCol - 1, dummy1, dummy2));
        nextBtn->setEnabled(curCol < imgW - 1 && mRenderer.sampleRangeForColumn(curCol + 1, dummy1, dummy2));
    };

    connect(prevBtn, &QPushButton::clicked, [&]() { --curCol; update(); });
    connect(nextBtn, &QPushButton::clicked, [&]() { ++curCol; update(); });

    update();
    dlg.exec();
}
