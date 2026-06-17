#include "MeasurementEngine.h"
#include <QtMath>
#include <QDebug>
#include <cmath>
#include <algorithm>

// Reference offset mapping dBFS RMS to an SPL-like scale (uncalibrated; tune
// with a reference meter in EXP-04). With this, ~−45 dBFS RMS reads ~55 dB.
static constexpr double kNoiseDbRefOffset = 100.0;

#define TIC 0
#define TOC 1
#define ERROR_RATE_Y_SCALE  10
#define RLS_WINDOW_INIT    100

MeasurementEngine::MeasurementEngine(QObject *parent)
    : QObject(parent)
{
    mRate.rlsTic = new RollingLeastSquares(RLS_WINDOW_INIT);
    mRate.rlsToc = new RollingLeastSquares(RLS_WINDOW_INIT);
    mBeat.roll   = new RollingAverage(10);
    mAmp.roll    = new RollingAverage(10);
}

MeasurementEngine::~MeasurementEngine()
{
    destroy();
    delete mRate.rlsTic;
    delete mRate.rlsToc;
    delete mBeat.roll;
    delete mAmp.roll;
}

void MeasurementEngine::init(int sampleRate, int bph, double liftAngle,
                              int averagingPeriod, double hpfCutoff)
{
    destroy();
    mSamplesPerSecond = sampleRate;
    mLiftAngle        = liftAngle;
    mAveragingPeriod  = averagingPeriod;

    tg_config_default(&mCfg);
    mCfg.sample_rate              = sampleRate;
    mCfg.suppress_pre_sync_events = true;
    mCfg.hpf_cutoff_hz            = hpfCutoff;
    if (bph == 0)
        mCfg.bph_mode = TG_BPH_MODE_AUTO;
    else {
        mCfg.bph_mode   = TG_BPH_MODE_MANUAL;
        mCfg.manual_bph = bph;
    }

    mCtx = tg_init(&mCfg);
    if (!mCtx) qCritical() << "MeasurementEngine: tg_init failed";
}

void MeasurementEngine::destroy()
{
    if (mCtx) { tg_destroy(mCtx); mCtx = nullptr; }
}

void MeasurementEngine::reset()
{
    mGraphTicks = 0;
    mHaveLastA  = false;

    mRate.beatNumber     = 0;
    mRate.haveStart      = false;
    mRate.haveZeroOffset = false;
    mRate.bphValid       = false;
    mRate.xTicIdx        = 0;
    mRate.xTocIdx        = 0;
    mRate.xTic.clear(); mRate.yTic.clear();
    mRate.xToc.clear(); mRate.yToc.clear();
    mRate.rlsTic->Reset();
    mRate.rlsToc->Reset();
    mRate.rateValid = false;

    mBeat.idx = 0;
    mBeat.roll->Reset();

    mAmp.haveA    = false;
    mAmp.ticValid = false;
    mAmp.roll->Reset();

    mHandling.recentPeaks.clear();
    mHandling.lastAcceptedPos = -1.0;

    mNoSignalTimerStarted = false;
}

// Returns true when an event looks like handling noise (a tap on the watch or
// sensor) rather than a real A/C beat:
//   1) refractory — it arrives implausibly soon after the last accepted event
//      (tap ring-down), shorter than any real A→C interval; and
//   2) amplitude outlier — its peak is far above the watch's own beat amplitude.
// The amplitude test is relative (median × K), so faint watches' genuine A/C
// (which sit near their own median) are preserved, not rejected.
bool MeasurementEngine::isHandlingNoise(double samplePos, float peak) const
{
    if (mHandling.lastAcceptedPos >= 0.0) {
        double dtMs = (samplePos - mHandling.lastAcceptedPos) / mSamplesPerSecond * 1000.0;
        if (dtMs >= 0.0 && dtMs < HandlingNoiseState::kRefractoryMs)
            return true;
    }
    if (mHandling.recentPeaks.size() >= HandlingNoiseState::kMinForOutlier) {
        QVector<double> sorted = mHandling.recentPeaks;
        std::sort(sorted.begin(), sorted.end());
        double median = sorted[sorted.size() / 2];
        if (median > 1e-9 && peak > median * HandlingNoiseState::kAmplitudeK)
            return true;
    }
    return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// Public slot — receives one block of raw float PCM from the ring buffer
// ──────────────────────────────────────────────────────────────────────────────
void MeasurementEngine::processBlock(const float *pcm, int numSamples)
{
    if (!mCtx) return;

    tg_result_t tgResult;
    if (tg_process(mCtx, pcm, numSamples, &tgResult) != 0) {
        qWarning() << "MeasurementEngine: tg_process failed";
        return;
    }

    Measurement measurement;
    measurement.samplesPerSecond = mSamplesPerSecond;
    measurement.graphTickStart   = mGraphTicks;
    measurement.synced           = (tgResult.sync_status == TG_SYNC_SYNCED);
    measurement.detectedBph      = tgResult.detected_bph;

    // Raw PCM for SoundPrintTab (copy before any DSP transformation)
    measurement.rawPcm.reserve(numSamples);
    for (int i = 0; i < numSamples; i++) measurement.rawPcm.append(pcm[i]);

    // Ambient-noise level = the detector's adaptive noise floor (onset_threshold),
    // i.e. the baseline BETWEEN beats — not the beat peaks — so it reflects the
    // environment, not how loud the watch is. Zero extra cost (already computed).
    // dB on an SPL-like scale: dBFS + reference offset (tunable; EXP-04 calibrates).
    measurement.noiseDb = 20.0 * std::log10(tgResult.onset_threshold + 1e-9) + kNoiseDbRefOffset;

    // Processed PCM + threshold for ScopePlot
    measurement.pcm.reserve(tgResult.processed_pcm_len);
    measurement.threshold.reserve(tgResult.processed_pcm_len);
    measurement.hpfPcm.reserve(tgResult.filtered_pcm_len);
    for (int i = 0; i < tgResult.processed_pcm_len; i++) {
        measurement.pcm.append(tgResult.processed_pcm[i]);
        measurement.threshold.append(tgResult.onset_threshold);
    }
    for (size_t i = 0; i < tgResult.filtered_pcm_len; i++) {
        measurement.hpfPcm.append(tgResult.filtered_pcm[i]);
    }
    mGraphTicks              += tgResult.processed_pcm_len;
    measurement.graphTickEnd  = mGraphTicks;

    // Process each A/C event
    for (int i = 0; i < tgResult.num_events; i++) {
        const tg_event_t &ev = tgResult.events[i];

        // Handling-noise rejection (tap on watch/sensor): discard impulsive
        // outlier events before they corrupt any derived measurement, while
        // keeping real A/C. Event-level → no per-sample cost (QAS-1/QAS-2).
        const double evPos = ev.sample_index + ev.sub_sample_offset;
        if (isHandlingNoise(evPos, ev.peak_value)) {
            measurement.handlingNoiseRejected++;
            continue;   // not a beat: skip rate/beat/amplitude and the event list
        }
        // Accepted event → update the rolling amplitude baseline + refractory ref
        mHandling.lastAcceptedPos = evPos;
        mHandling.recentPeaks.append(ev.peak_value);
        if (mHandling.recentPeaks.size() > HandlingNoiseState::kWindow)
            mHandling.recentPeaks.removeFirst();

        AcousticEvent acousticEvent;
        acousticEvent.cOnsetValid       = false;
        acousticEvent.hasRatePoint      = false;
        acousticEvent.wrappedRateError  = 0.0;
        acousticEvent.isTic             = false;
        acousticEvent.hasEscapementMs   = false;
        acousticEvent.escapementMs      = 0.0;
        acousticEvent.hasAmpSplit       = false;
        acousticEvent.ticAmpDeg         = 0.0;
        acousticEvent.tocAmpDeg         = 0.0;

        if (ev.type == TG_EVENT_A) {
            acousticEvent.isA       = true;
            acousticEvent.samplePos = ev.sample_index + ev.sub_sample_offset;
            acousticEvent.peakValue = ev.peak_value;

            // QAS-4: reset no-signal timer on each A-event
            mNoSignalTimer.restart();
            mNoSignalTimerStarted = true;

            computeRateError(acousticEvent.samplePos, measurement.synced, tgResult.detected_bph, acousticEvent);
            computeBeatError(acousticEvent.samplePos, measurement.synced, tgResult.detected_bph);

            mAmp.haveA = true;
            mAmp.lastA = acousticEvent.samplePos;
            mHaveLastA = true;
            mLastA     = acousticEvent.samplePos;
        } else if (ev.type == TG_EVENT_C) {
            acousticEvent.isA         = false;
            acousticEvent.peakValue   = ev.peak_value;
            acousticEvent.cOnsetValid = ev.onset_valid;
            acousticEvent.cOnsetPos   = ev.onset_sample_index + ev.onset_sub_sample_offset;

            if (mUseOnset && ev.onset_valid)
                acousticEvent.samplePos = acousticEvent.cOnsetPos;
            else
                acousticEvent.samplePos = ev.sample_index + ev.sub_sample_offset;

            // Escapement: T1→T3 interval
            if (mHaveLastA) {
                acousticEvent.hasEscapementMs = true;
                acousticEvent.escapementMs    = (acousticEvent.samplePos - mLastA) / mSamplesPerSecond * 1000.0;
            }

            computeAmplitude(acousticEvent.samplePos, measurement.synced, tgResult.detected_bph, measurement, acousticEvent);
        } else {
            qWarning() << "MeasurementEngine: unknown event type";
            continue;
        }
        measurement.events.append(acousticEvent);
    }

    measurement.rateValid      = mRate.rateValid;
    measurement.rateErrorSpd   = mRate.rateSpd;
    measurement.beatErrorValid = (mBeat.roll->CurrentSize() > 0);
    measurement.beatErrorMs    = measurement.beatErrorValid ? mBeat.roll->GetAverage() : 0.0;
    measurement.amplitudeValid = (mAmp.roll->CurrentSize() > 0);
    measurement.amplitudeDeg   = measurement.amplitudeValid ? mAmp.roll->GetAverage() : 0.0;
    measurement.noSignal       = mNoSignalTimerStarted && (mNoSignalTimer.elapsed() > kNoSignalThresholdMs);

    emit measurementReady(measurement);
}

// ──────────────────────────────────────────────────────────────────────────────
// Private helpers  (extracted from MainWindow)
// ──────────────────────────────────────────────────────────────────────────────
double MeasurementEngine::wrapInRange(double value, double lo, double hi) const
{
    double range  = hi - lo;
    double shifted = fmod(value - lo, range);
    if (shifted < 0) shifted += range;
    return shifted + lo;
}

void MeasurementEngine::addOrOverwrite(QVector<double> &xv, QVector<double> &yv,
                                        double val, int maxSize, int &idx)
{
    if (yv.size() < maxSize) { yv.append(val); xv.append(idx); }
    else { yv[idx] = val; }
    idx = (idx + 1) % maxSize;
}

void MeasurementEngine::computeRateError(double evTime, bool synced, int bph, AcousticEvent &ae)
{
    if (!synced && mRate.haveStart) {
        mRate.haveStart = false;
        mRate.bphValid  = false;
        return;
    }
    if (synced && !mRate.haveStart) {
        mRate.haveStart       = true;
        mRate.beatNumber      = 0;
        mRate.bphValid        = true;
        mRate.bph             = bph;
        mRate.startTime       = evTime / mSamplesPerSecond;
        mRate.haveZeroOffset  = false;
        mRate.zeroOffset      = 0.0;
        mRate.rateValid       = false;
        mRate.watchHz         = bph / 3600;
        int window = mAveragingPeriod * mRate.watchHz;
        mRate.rlsTic->Resize(window);
        mRate.rlsToc->Resize(window);
        mRate.rlsTic->Reset();
        mRate.rlsToc->Reset();
        mBeat.roll->Reset();
        mAmp.roll->Reset();
    }
    if (!synced || !mRate.haveStart) return;

    double timeMeasured      = evTime / mSamplesPerSecond;
    double expectedInterval  = 3600.0 / bph;

    mRate.beatNumber++;
    int ticOrToc = (int)((mRate.beatNumber - 1) & 1);

    double instError   = (mRate.startTime + mRate.beatNumber * expectedInterval) - timeMeasured;
    double instErrorMs = instError * 1000.0;

    if (!mRate.haveZeroOffset) {
        mRate.haveZeroOffset = true;
        mRate.zeroOffset     = -instErrorMs;
    }
    instErrorMs += mRate.zeroOffset;

    double wrapped = wrapInRange(instErrorMs, -ERROR_RATE_Y_SCALE, ERROR_RATE_Y_SCALE);
    ae.hasRatePoint     = true;
    ae.wrappedRateError = wrapped;
    ae.isTic            = (ticOrToc == TIC);

    if (ticOrToc == TIC) {
        mRate.rlsTic->AddPoint(timeMeasured, instError);
        addOrOverwrite(mRate.xTic, mRate.yTic, wrapped, mRate.maxPoints, mRate.xTicIdx);
    } else {
        mRate.rlsToc->AddPoint(timeMeasured, instError);
        addOrOverwrite(mRate.xToc, mRate.yToc, wrapped, mRate.maxPoints, mRate.xTocIdx);
    }

    if (ticOrToc == TOC) {
        double slopeTic, slopeToc;
        if (mRate.rlsTic->GetRate(slopeTic) && mRate.rlsToc->GetRate(slopeToc)) {
            mRate.rateSpd   = ((slopeTic + slopeToc) / 2.0) * 86400.0;
            mRate.rateValid = true;
        } else {
            mRate.rateValid = false;
        }
    }
}

void MeasurementEngine::computeBeatError(double evTime, bool, int)
{
    mBeat.times[mBeat.idx] = evTime;
    mBeat.idx++;
    if (mBeat.idx == 3) {
        double t1 = (mBeat.times[1] - mBeat.times[0]) / mSamplesPerSecond;
        double t2 = (mBeat.times[2] - mBeat.times[1]) / mSamplesPerSecond;
        mBeat.roll->Add(qAbs(((t1 - t2) / 2.0) * 1000.0));
        mBeat.times[0] = mBeat.times[2];
        mBeat.idx       = 1;
    }
}

void MeasurementEngine::computeAmplitude(double cTime, bool synced, int bph, Measurement &m, AcousticEvent &ae)
{
    if (!mAmp.haveA || !mRate.bphValid) return;
    double T1     = (cTime - mAmp.lastA) / mSamplesPerSecond;
    double period = 7200.0 / bph;
    double amp    = mLiftAngle / sin((2.0 * M_PI * T1) / period);
    if (amp >= 360.0) {
        if ((int)((mRate.beatNumber - 1) & 1) == TIC) mAmp.ticValid = false;
        return;
    }
    int ticOrToc = (int)((mRate.beatNumber - 1) & 1);
    if (ticOrToc == TIC) {
        mAmp.ticValid = true;
        mAmp.ticAmp   = amp;
    } else {
        if (mAmp.ticValid) {
            mAmp.roll->Add((mAmp.ticAmp + amp) / 2.0);
            // Expose Tic/Toc amplitude on this C-event for VarioTab
            ae.hasAmpSplit = true;
            ae.ticAmpDeg   = mAmp.ticAmp;
            ae.tocAmpDeg   = amp;
            mAmp.ticValid = false;
        }
    }
    m.amplitudeValid = (mAmp.roll->CurrentSize() > 0);
    m.amplitudeDeg   = m.amplitudeValid ? mAmp.roll->GetAverage() : 0.0;
}
