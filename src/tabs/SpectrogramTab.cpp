#include "ReplotCounter.h"
#include "SpectrogramTab.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <vector>

SpectrogramTab::SpectrogramTab(QWidget *parent) : BaseGraphTab(parent)
{
    setStyleSheet(QStringLiteral(
        "SpectrogramTab { background-color: #121218; color: #c8c8d0; }"
        "QLabel { color: #c8c8d0; }"
        "QCheckBox { color: #c8c8d0; }"
        "QPushButton { background-color: #2a2a36; color: #c8c8d0; border: 1px solid #444; padding: 4px 10px; }"
        "QPushButton:checked { background-color: #3d5a80; border-color: #6a9fd8; }"
        "QDoubleSpinBox { background-color: #2a2a36; color: #c8c8d0; border: 1px solid #444; }"
        "QProgressBar { background-color: #1a1a22; border: 1px solid #444; height: 14px; }"
        "QProgressBar::chunk { background-color: #3ecf6e; }"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    mTitleLabel = new QLabel(tr("Time and Frequency Reassigned Spectrogram"), this);
    QFont titleFont = mTitleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    mTitleLabel->setFont(titleFont);
    mainLayout->addWidget(mTitleLabel);

    auto *meterRow = new QHBoxLayout;
    mPeakMeterCheck = new QCheckBox(tr("True Peak Programme Meter"), this);
    mPeakMeterCheck->setChecked(true);
    meterRow->addWidget(mPeakMeterCheck);
    mPeakDbLabel = new QLabel(QStringLiteral("-∞ dBFS"), this);
    meterRow->addWidget(mPeakDbLabel);
    mPeakBar = new QProgressBar(this);
    mPeakBar->setRange(0, 100);
    mPeakBar->setValue(0);
    mPeakBar->setTextVisible(false);
    mPeakBar->setFixedWidth(180);
    meterRow->addWidget(mPeakBar);
    meterRow->addStretch(1);
    mainLayout->addLayout(meterRow);

    auto *controls = new QHBoxLayout;
    controls->addWidget(new QLabel(tr("Spectrograms"), this));

    auto *lastBeatBtn = new QPushButton(tr("Last Beat"), this);
    lastBeatBtn->setCheckable(true);
    auto *secondsBtn = new QPushButton(tr("Seconds"), this);
    secondsBtn->setCheckable(true);
    secondsBtn->setChecked(true);
    mModeGroup = new QButtonGroup(this);
    mModeGroup->addButton(lastBeatBtn, static_cast<int>(ViewMode::LastBeat));
    mModeGroup->addButton(secondsBtn, static_cast<int>(ViewMode::Seconds));
    controls->addWidget(lastBeatBtn);
    controls->addWidget(secondsBtn);

    mWindowSpin = new QDoubleSpinBox(this);
    mWindowSpin->setRange(0.2, 12.0);
    mWindowSpin->setSingleStep(0.1);
    mWindowSpin->setDecimals(1);
    mWindowSpin->setValue(1.0);
    mWindowSpin->setSuffix(QStringLiteral(" s"));
    mWindowSpin->setFixedWidth(90);
    controls->addWidget(mWindowSpin);
    mAutoScaleCheck = new QCheckBox(tr("Auto-scale dB"), this);
    controls->addWidget(mAutoScaleCheck);
    controls->addStretch(1);
    mainLayout->addLayout(controls);

    mPlot = new QCustomPlot(this);
    mainLayout->addWidget(mPlot, 1);

    mPlot->setBackground(QBrush(QColor(0x12, 0x12, 0x18)));
    mPlot->axisRect()->setBackground(QBrush(QColor(0x3a, 0x3a, 0x3e)));
    const QPen axisPen(QColor(0x88, 0x88, 0x99));
    mPlot->xAxis->setBasePen(axisPen);
    mPlot->yAxis->setBasePen(axisPen);
    mPlot->xAxis->setTickPen(axisPen);
    mPlot->yAxis->setTickPen(axisPen);
    mPlot->xAxis->setSubTickPen(axisPen);
    mPlot->yAxis->setSubTickPen(axisPen);
    mPlot->xAxis->setTickLabelColor(QColor(0xc8, 0xc8, 0xd0));
    mPlot->yAxis->setTickLabelColor(QColor(0xc8, 0xc8, 0xd0));
    mPlot->xAxis->setLabelColor(QColor(0xc8, 0xc8, 0xd0));
    mPlot->yAxis->setLabelColor(QColor(0xc8, 0xc8, 0xd0));

    mMap = new QCPColorMap(mPlot->xAxis, mPlot->yAxis);
    mScale = new QCPColorScale(mPlot);
    mPlot->plotLayout()->addElement(0, 1, mScale);
    mScale->setType(QCPAxis::atRight);
    mScale->axis()->setLabel(tr("dB"));
    mScale->axis()->setTickLabelColor(QColor(0xc8, 0xc8, 0xd0));
    mScale->axis()->setLabelColor(QColor(0xc8, 0xc8, 0xd0));
    mMap->setColorScale(mScale);
    mMap->setGradient(spectrogramGradient());
    mMap->setDataRange(QCPRange(kDbMin, kDbMax));
    mMap->setInterpolate(true);
    mMap->setTightBoundary(false);

    mPlot->xAxis->setLabel(tr("Time (ms)"));
    mPlot->yAxis->setLabel(tr("Frequency (Hz)"));
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    prepareFft();
    reset();

    connect(mModeGroup, &QButtonGroup::idClicked, this, [this](int id) {
        mViewMode = static_cast<ViewMode>(id);
        mWindowSpin->setEnabled(mViewMode == ViewMode::Seconds);
        if (mViewMode == ViewMode::LastBeat) {
            rebuildLastBeatView();
        } else {
            rebuildAxisRanges();
            mPlot->replot(QCustomPlot::rpQueuedReplot);
        }
    });
    connect(mWindowSpin, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, [this](double windowSec) {
        mWindowSec = windowSec;
        rebuildAxisRanges();
        if (!mPaused) {
            mPlot->replot(QCustomPlot::rpQueuedReplot);
        }
    });
    connect(mPeakMeterCheck, &QCheckBox::toggled, this, [this](bool on) {
        mPeakDbLabel->setVisible(on);
        mPeakBar->setVisible(on);
    });
    connect(mAutoScaleCheck, &QCheckBox::toggled, this, [this](bool) {
        updateColorRange();
        if (!mPaused) mPlot->replot(QCustomPlot::rpQueuedReplot);
    });
}

QCPColorGradient SpectrogramTab::spectrogramGradient()
{
    // Matplotlib viridis — matches project plan Figure 16 colorbar
    QCPColorGradient grad;
    grad.clearColorStops();
    grad.setColorStopAt(0.00, QColor(0x44, 0x01, 0x54));
    grad.setColorStopAt(0.25, QColor(0x20, 0x88, 0x94));
    grad.setColorStopAt(0.50, QColor(0x35, 0xb7, 0x79));
    grad.setColorStopAt(0.75, QColor(0xbc, 0xbd, 0x22));
    grad.setColorStopAt(1.00, QColor(0xfd, 0xe7, 0x25));
    grad.setColorInterpolation(QCPColorGradient::ciRGB);
    grad.setLevelCount(256);
    return grad;
}

void SpectrogramTab::updateColorRange()
{
    if (mAutoScaleCheck && mAutoScaleCheck->isChecked()) {
        mMap->rescaleDataRange(true);
        mScale->setDataRange(mMap->dataRange());
    } else {
        const QCPRange range(kDbMin, kDbMax);
        mMap->setDataRange(range);
        mScale->setDataRange(range);
    }
}

SpectrogramTab::~SpectrogramTab()
{
    if (mCfg) {
        kiss_fft_free(mCfg);
        mCfg = nullptr;
    }
}

void SpectrogramTab::prepareFft()
{
    if (mCfg) {
        return;
    }
    mCfg = kiss_fft_alloc(kFftSize, 0, nullptr, nullptr);
    mHannWindow.resize(kFftSize);
    for (int i = 0; i < kFftSize; ++i) {
        mHannWindow[i] = static_cast<float>(
            0.5 * (1.0 - std::cos(2.0 * M_PI * i / (kFftSize - 1))));
    }
}

void SpectrogramTab::ensureMapSize()
{
    QCPColorMapData *data = mMap->data();
    if (data->keySize() != kTimeColumns || data->valueSize() != mFreqBins) {
        data->setSize(kTimeColumns, mFreqBins);
        // QCPColorMapData::setSize() zero-fills; 0 lies above −10 dB → false yellow.
        data->fill(kDbMin);
    }
}

int SpectrogramTab::freqBinCount() const
{
    int bins = static_cast<int>(kMaxDisplayHz * kFftSize / mSampleRate) + 1;
    bins = std::min(bins, kFftSize / 2);
    return qMax(1, bins);
}

void SpectrogramTab::reset()
{
    mPcmBuffer.clear();
    mBufferStartTick = 0;
    mHaveLastBeat = false;
    mLastBeatTick = 0;
    mBeatPeriodMs = 1000.0;
    mLastPeakDbfs = -100.0;
    mPeakDbLabel->setText(QStringLiteral("-∞ dBFS"));
    mPeakBar->setValue(0);

    mFreqBins = freqBinCount();
    ensureMapSize();
    mMap->data()->setRange(QCPRange(0.0, mWindowSec * 1000.0),
                           QCPRange(0.0, kMaxDisplayHz));
    rebuildAxisRanges();
    const QCPRange fullRange(kDbMin, kDbMax);
    mMap->setDataRange(fullRange);
    mScale->setDataRange(fullRange);
    { int64_t _pt=TG_NOW(); mPlot->replot(); g_plotUs.fetch_add(TG_NOW()-_pt,std::memory_order_relaxed); };
}

std::vector<double> SpectrogramTab::computeMagnitudes(const float *pcm) const
{
    std::vector<kiss_fft_cpx> in(kFftSize);
    std::vector<kiss_fft_cpx> out(kFftSize);
    for (int i = 0; i < kFftSize; ++i) {
        in[i].r = pcm[i] * mHannWindow[i];
        in[i].i = 0.0f;
    }
    kiss_fft(mCfg, in.data(), out.data());

    std::vector<double> magnitudes(kFftSize / 2);
    for (int i = 0; i < kFftSize / 2; ++i) {
        magnitudes[i] = std::hypot(static_cast<double>(out[i].r),
                                   static_cast<double>(out[i].i));
    }
    return magnitudes;
}

double SpectrogramTab::binMagnitudeToDb(double magnitude, int bin) const
{
    const double norm = (bin > 0 && bin < kFftSize / 2 - 1) ? 2.0 : 1.0;
    const double amplitude = norm * magnitude / (static_cast<double>(kFftSize) * 0.5);
    const double db = 20.0 * std::log10(amplitude + 1e-12);
    return qBound(kDbMin, db, kDbMax);
}

void SpectrogramTab::shiftColumnsAndAppend(const std::vector<double> &magnitudes)
{
    QCPColorMapData *data = mMap->data();
    const int keySize = data->keySize();
    const int valSize = data->valueSize();

    for (int k = 0; k < keySize - 1; ++k) {
        for (int binRow = 0; binRow < valSize; ++binRow) {
            data->setCell(k, binRow, data->cell(k + 1, binRow));
        }
    }

    const int newCol = keySize - 1;
    const int binsToStore = std::min(valSize, static_cast<int>(magnitudes.size()));
    for (int binRow = 0; binRow < binsToStore; ++binRow) {
        data->setCell(newCol, binRow,
                      binMagnitudeToDb(magnitudes[static_cast<size_t>(binRow)], binRow));
    }
    for (int binRow = binsToStore; binRow < valSize; ++binRow) {
        data->setCell(newCol, binRow, kDbMin);
    }
}

void SpectrogramTab::processPendingColumns()
{
    while (mPcmBuffer.size() >= static_cast<size_t>(kFftSize)) {
        const std::vector<double> magnitudes = computeMagnitudes(mPcmBuffer.data());
        if (!magnitudes.empty()) {
            shiftColumnsAndAppend(magnitudes);
        }
        mPcmBuffer.erase(mPcmBuffer.begin(),
                         mPcmBuffer.begin() + kHopSize);
        if (!mPcmBuffer.empty()) {
            mBufferStartTick += static_cast<uint64_t>(kHopSize);
        }
    }
}

void SpectrogramTab::rebuildAxisRanges()
{
    const double windowMs = (mViewMode == ViewMode::Seconds)
        ? mWindowSec * 1000.0
        : qMax(200.0, mBeatPeriodMs);

    mMap->data()->setKeyRange(QCPRange(0.0, windowMs));
    mMap->data()->setValueRange(QCPRange(0.0, kMaxDisplayHz));
    mPlot->xAxis->setRange(0, windowMs);
    mPlot->yAxis->setRange(0, kMaxDisplayHz);
}

void SpectrogramTab::rebuildLastBeatView()
{
    mMap->data()->fill(kDbMin);
    if (!mHaveLastBeat || mPcmBuffer.empty()) {
        rebuildAxisRanges();
        mPlot->replot(QCustomPlot::rpQueuedReplot);
        return;
    }

    const double windowMs = qMax(200.0, mBeatPeriodMs);
    const uint64_t windowEndTick = mLastBeatTick
        + static_cast<uint64_t>(windowMs / 1000.0 * mSampleRate);

    uint64_t startTick = mLastBeatTick;
    if (startTick < mBufferStartTick) {
        startTick = mBufferStartTick;
    }
    uint64_t endTick = windowEndTick;
    const uint64_t bufferEndTick = mBufferStartTick + mPcmBuffer.size();
    if (endTick > bufferEndTick) {
        endTick = bufferEndTick;
    }

    const int offset = static_cast<int>(startTick - mBufferStartTick);
    const int count = static_cast<int>(endTick - startTick);
    if (count < kFftSize) {
        rebuildAxisRanges();
        mPlot->replot(QCustomPlot::rpQueuedReplot);
        return;
    }

    ensureMapSize();

    int col = 0;
    for (int start = 0; start + kFftSize <= count && col < kTimeColumns; start += kHopSize) {
        const std::vector<double> magnitudes =
            computeMagnitudes(mPcmBuffer.data() + offset + start);
        const int binsToStore = std::min(mFreqBins, static_cast<int>(magnitudes.size()));
        for (int binRow = 0; binRow < binsToStore; ++binRow) {
            mMap->data()->setCell(col, binRow,
                                  binMagnitudeToDb(magnitudes[static_cast<size_t>(binRow)], binRow));
        }
        ++col;
    }

    rebuildAxisRanges();
    updateColorRange();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}

void SpectrogramTab::updateBeatMarkers(const Measurement &m)
{
    if (m.synced && m.detectedBph > 0) {
        mBeatPeriodMs = 3600000.0 / static_cast<double>(m.detectedBph);
    }
    for (const AcousticEvent &ev : m.events) {
        if (ev.isA) {
            mHaveLastBeat = true;
            mLastBeatTick = static_cast<uint64_t>(ev.samplePos);
        }
    }
}

double SpectrogramTab::truePeakDbfs(const QVector<float> &pcm) const
{
    if (pcm.isEmpty()) {
        return -100.0;
    }
    float peak = 0.0f;
    for (float s : pcm) {
        peak = qMax(peak, std::abs(s));
    }
    if (peak <= 1e-10f) {
        return -100.0;
    }
    return 20.0 * std::log10(static_cast<double>(peak));
}

void SpectrogramTab::onMeasurement(const Measurement &m)
{
    if (m.hpfPcm.isEmpty()) {
        return;
    }

    mSampleRate = m.samplesPerSecond;
    mFreqBins = freqBinCount();

    if (mPcmBuffer.empty()) {
        mBufferStartTick = m.graphTickStart;
    }

    mPcmBuffer.insert(mPcmBuffer.end(), m.hpfPcm.constBegin(), m.hpfPcm.constEnd());

    const uint64_t maxSamples = static_cast<uint64_t>(kMaxBufferSec * mSampleRate);
    if (mPcmBuffer.size() > static_cast<int>(maxSamples)) {
        const int excess = mPcmBuffer.size() - static_cast<int>(maxSamples);
        mPcmBuffer.erase(mPcmBuffer.begin(), mPcmBuffer.begin() + excess);
        mBufferStartTick += static_cast<uint64_t>(excess);
    }

    updateBeatMarkers(m);

    const double peakDb = truePeakDbfs(m.hpfPcm);
    if (peakDb > mLastPeakDbfs - 0.5) {
        mLastPeakDbfs = peakDb;
    }
    if (mPeakMeterCheck->isChecked()) {
        mPeakDbLabel->setText(QStringLiteral("%1 dBFS").arg(mLastPeakDbfs, 0, 'f', 1));
        const int bar = qBound(0, static_cast<int>((mLastPeakDbfs + 60.0) / 60.0 * 100.0), 100);
        mPeakBar->setValue(bar);
    }

    // Lazy pull: skip FFT when an embedded tab page is hidden (QAS-1/QAS-2).
    // Standalone widgets (parentWidget()==nullptr, e.g. unit tests) always process.
    if (mPaused || (parentWidget() != nullptr && !isVisible())) {
        return;
    }

    if (mViewMode == ViewMode::LastBeat) {
        rebuildLastBeatView();
        return;
    }

    ensureMapSize();

    processPendingColumns();
    rebuildAxisRanges();
    updateColorRange();
    mPlot->replot(QCustomPlot::rpQueuedReplot);
}
