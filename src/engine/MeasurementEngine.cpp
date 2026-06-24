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

// Grid-outlier (handling-noise tap) rejection. A real beat's instantaneous grid
// error changes only slowly between same-parity beats (sub-ms to ~1-2 ms jitter);
// a tap inserted between beats lands tens of ms off the grid. Reject A-events
// whose per-parity grid error jumps by more than this. kMaxConsecRejects then
// re-baselines so a genuine grid shift (missed beat / re-lock) is not stuck.
static constexpr double kGridOutlierMs    = 4.5;
static constexpr int    kMaxConsecRejects = 4;
// A running watch's beat error is at most a few ms; a value beyond this is a tap
// that displaced a beat by less than the grid threshold — drop it from the average.
static constexpr double kBeatErrorMaxMs   = 5.0;

MeasurementEngine::MeasurementEngine(QObject *parent)
    : QObject(parent)
{
    mRate.rlsTic      = new RollingLeastSquares(RLS_WINDOW_INIT);
    mRate.rlsToc      = new RollingLeastSquares(RLS_WINDOW_INIT);
    mBeat.roll        = new RollingAverage(10);
    mAmp.roll         = new RollingAverage(10);
    mAsym.roll        = new RollingAverage(10);
    mJitter.roll      = new RollingAverage(20);
    mEsc.roll         = new RollingAverage(20);
    mPeriod.shortRoll = new RollingAverage(32); // default 4 s @ 8 Hz; resized on first sync
}

MeasurementEngine::~MeasurementEngine()
{
    destroy();
    delete mRate.rlsTic;
    delete mRate.rlsToc;
    delete mBeat.roll;
    delete mAmp.roll;
    delete mAsym.roll;
    delete mJitter.roll;
    delete mEsc.roll;
    delete mPeriod.shortRoll;
}

void MeasurementEngine::init(const MovementSpec &movement, const AcquisitionConfig &config)
{
    destroy();
    mSamplesPerSecond = config.sampleRate;
    mLiftAngle        = movement.liftAngle;
    mAveragingPeriod  = config.averagingPeriod;

    tg_config_default(&mCfg);
    mCfg.sample_rate              = config.sampleRate;
    mCfg.suppress_pre_sync_events = true;
    mCfg.hpf_cutoff_hz            = config.hpfCutoff;
    if (movement.bph == 0)
        mCfg.bph_mode = TG_BPH_MODE_AUTO;
    else {
        mCfg.bph_mode   = TG_BPH_MODE_MANUAL;
        mCfg.manual_bph = movement.bph;
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
    mRate.haveLastInstErr[0] = mRate.haveLastInstErr[1] = false;
    mRate.consecRejects[0] = mRate.consecRejects[1] = 0;

    mBeat.idx = 0;
    mBeat.roll->Reset();

    mAmp.haveA    = false;
    mAmp.ticValid = false;
    mAmp.roll->Reset();

    mAsym.roll->Reset();
    mJitter.roll->Reset();
    mEsc.haveLast = false;
    mEsc.roll->Reset();

    mHandling.recentPeaks.clear();
    mHandling.lastAcceptedPos = -1.0;

    mRate.havePrevInstError = false;
    mRate.prevInstErrorSec  = 0.0;
    mPeriod.shortRoll->Reset();
    mPeriod.sumAll   = 0.0;
    mPeriod.countAll = 0;
    mHaveDiffTicTac  = false;

    mNoSignalTimerStarted = false;
    mLastKnownMetrics     = {};
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
    measurement.signal.samplesPerSecond = mSamplesPerSecond;
    measurement.signal.tickStart        = mGraphTicks;
    measurement.synced                  = (tgResult.sync_status == TG_SYNC_SYNCED);
    measurement.detectedBph             = tgResult.detected_bph;

    // Raw PCM for SoundPrintTab (copy before any DSP transformation)
    measurement.signal.rawPcm.reserve(numSamples);
    for (int i = 0; i < numSamples; i++) measurement.signal.rawPcm.append(pcm[i]);

    // Ambient-noise level = the detector's adaptive noise floor (onset_threshold),
    // i.e. the baseline BETWEEN beats — not the beat peaks — so it reflects the
    // environment, not how loud the watch is. Zero extra cost (already computed).
    // dB on an SPL-like scale: dBFS + reference offset (tunable; EXP-04 calibrates).
    measurement.noiseDb = 20.0 * std::log10(tgResult.onset_threshold + 1e-9) + kNoiseDbRefOffset;

    // Processed PCM + threshold for ScopePlot
    measurement.signal.pcm.reserve(tgResult.processed_pcm_len);
    measurement.signal.threshold.reserve(tgResult.processed_pcm_len);
    measurement.signal.hpfPcm.reserve(tgResult.filtered_pcm_len);
    for (int i = 0; i < tgResult.processed_pcm_len; i++) {
        measurement.signal.pcm.append(tgResult.processed_pcm[i]);
        measurement.signal.threshold.append(tgResult.onset_threshold);
    }
    for (size_t i = 0; i < tgResult.filtered_pcm_len; i++) {
        measurement.signal.hpfPcm.append(tgResult.filtered_pcm[i]);
    }
    mGraphTicks                  += tgResult.processed_pcm_len;
    measurement.signal.tickEnd    = mGraphTicks;

    // Process each A/C event
    for (int i = 0; i < tgResult.num_events; i++) {
        const tg_event_t &ev = tgResult.events[i];

        // Handling-noise rejection (tap on watch/sensor). A tap usually does not
        // ADD an event — it pulls one A-detection off the beat grid (right count,
        // wrong time). Dropping such an A-event would lose its beat slot and shift
        // the grid, so A-events are NOT removed here; the grid gate in
        // computeRateError suppresses the tap's rate/beat output while keeping the
        // count. The amplitude/refractory gate still drops genuinely extra non-A
        // impulses (e.g. ring-down) before they pollute anything.
        const double evPos = ev.sample_index + ev.sub_sample_offset;
        const bool   handlingNoise = isHandlingNoise(evPos, ev.peak_value);
        if (handlingNoise) measurement.handlingNoiseRejected++;
        if (handlingNoise && ev.type != TG_EVENT_A)
            continue;   // extra non-A impulse: drop it
        // Update the rolling amplitude baseline + refractory ref from clean events
        // only (a flagged A-tap must not inflate the median or move the ref).
        if (!handlingNoise) {
            mHandling.lastAcceptedPos = evPos;
            mHandling.recentPeaks.append(ev.peak_value);
            if (mHandling.recentPeaks.size() > HandlingNoiseState::kWindow)
                mHandling.recentPeaks.removeFirst();
        }

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

            // Grid-outlier (tap) rejection: if the A-event does not fit the beat
            // grid it is handling noise that slipped past the amplitude gate —
            // skip the beat-error update too so it cannot corrupt that graph.
            bool tapOutlier = computeRateError(acousticEvent.samplePos, measurement.synced, tgResult.detected_bph, acousticEvent);
            if (tapOutlier)
                mBeat.idx = 0;   // corrupted slot: restart the beat-error triplet (no gap/split spike)
            else
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
                if (mEsc.haveLast)
                    mEsc.roll->Add(std::fabs(acousticEvent.escapementMs - mEsc.lastMs));
                mEsc.lastMs   = acousticEvent.escapementMs;
                mEsc.haveLast = true;
            }

            computeAmplitude(acousticEvent.samplePos, measurement.synced, tgResult.detected_bph, measurement.metrics, acousticEvent);
        } else {
            qWarning() << "MeasurementEngine: unknown event type";
            continue;
        }
        measurement.events.append(acousticEvent);
    }

    if (mRate.rateValid)
        measurement.metrics.rate = mRate.rateSpd;
    if (mBeat.roll->CurrentSize() > 0)
        measurement.metrics.beatError = mBeat.roll->GetAverage();
    if (mAsym.roll->CurrentSize() > 0)
        measurement.metrics.ticTocAsymmetryDeg = mAsym.roll->GetAverage();
    if (mJitter.roll->CurrentSize() > 0)
        measurement.metrics.rateJitterMs = mJitter.roll->GetAverage();
    if (mEsc.roll->CurrentSize() > 0)
        measurement.metrics.escapementDeltaMs = mEsc.roll->GetAverage();
    if (mHaveDiffTicTac)
        measurement.metrics.diffTicTac = mLastDiffTicTac;
    if (mPeriod.shortRoll->CurrentSize() > 0)
        measurement.metrics.diffPeriod = mPeriod.shortRoll->GetAverage();
    if (mPeriod.countAll > 0)
        measurement.metrics.avgPeriod = mPeriod.sumAll / mPeriod.countAll;

    measurement.noSignal = mNoSignalTimerStarted && (mNoSignalTimer.elapsed() > kNoSignalThresholdMs);

    // Retain last valid values — metrics are freshly built each frame so
    // amplitude (set only on C-event frames) would otherwise flicker to nullopt.
    if (!measurement.metrics.rate)               measurement.metrics.rate               = mLastKnownMetrics.rate;
    if (!measurement.metrics.amplitude)          measurement.metrics.amplitude          = mLastKnownMetrics.amplitude;
    if (!measurement.metrics.beatError)          measurement.metrics.beatError          = mLastKnownMetrics.beatError;
    if (!measurement.metrics.ticTocAsymmetryDeg) measurement.metrics.ticTocAsymmetryDeg = mLastKnownMetrics.ticTocAsymmetryDeg;
    if (!measurement.metrics.rateJitterMs)       measurement.metrics.rateJitterMs       = mLastKnownMetrics.rateJitterMs;
    if (!measurement.metrics.escapementDeltaMs)  measurement.metrics.escapementDeltaMs  = mLastKnownMetrics.escapementDeltaMs;
    if (!measurement.metrics.diffTicTac)         measurement.metrics.diffTicTac         = mLastKnownMetrics.diffTicTac;
    if (!measurement.metrics.diffPeriod)         measurement.metrics.diffPeriod         = mLastKnownMetrics.diffPeriod;
    if (!measurement.metrics.avgPeriod)          measurement.metrics.avgPeriod          = mLastKnownMetrics.avgPeriod;
    mLastKnownMetrics = measurement.metrics;

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

bool MeasurementEngine::computeRateError(double evTime, bool synced, int bph, AcousticEvent &ae)
{
    if (!synced && mRate.haveStart) {
        mRate.haveStart = false;
        mRate.bphValid  = false;
        return false;
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
        mRate.haveLastInstErr[0] = mRate.haveLastInstErr[1] = false;
        mRate.consecRejects[0] = mRate.consecRejects[1] = 0;
        int window = mAveragingPeriod * mRate.watchHz;
        mRate.rlsTic->Resize(window);
        mRate.rlsToc->Resize(window);
        mRate.rlsTic->Reset();
        mRate.rlsToc->Reset();
        mBeat.roll->Reset();
        mAmp.roll->Reset();
        // Size DiffPeriod window to ~4 s of beats
        int periodWindow = std::max(4, mRate.watchHz * 4);
        mPeriod.shortRoll->Resize(periodWindow);
        mPeriod.shortRoll->Reset();
        mPeriod.sumAll   = 0.0;
        mPeriod.countAll = 0;
        mRate.havePrevInstError = false;
    }
    if (!synced || !mRate.haveStart) return false;

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

    // Grid-outlier (tap) rejection — compared against this parity's own baseline
    // so the genuine tic/toc asymmetry is not flagged. A light tap does not add
    // an event; it pulls one detection off the grid (right count, wrong time).
    // So drop this beat's rate + beat-error contribution but KEEP the beat count
    // in sync with the detector (do not touch beatNumber) and keep the last good
    // parity baseline, so the next real beat re-aligns immediately.
    if (mRate.haveLastInstErr[ticOrToc]
        && std::fabs(instErrorMs - mRate.lastInstErrMs[ticOrToc]) > kGridOutlierMs
        && mRate.consecRejects[ticOrToc] < kMaxConsecRejects) {
        mRate.consecRejects[ticOrToc]++;
        ae.hasRatePoint = false;
        return true;                      // reject: caller skips beat error too
    }
    mRate.consecRejects[ticOrToc]   = 0;
    mRate.lastInstErrMs[ticOrToc]   = instErrorMs;
    mRate.haveLastInstErr[ticOrToc] = true;

    double wrapped = wrapInRange(instErrorMs, -ERROR_RATE_Y_SCALE, ERROR_RATE_Y_SCALE);
    ae.hasRatePoint     = true;
    ae.wrappedRateError = wrapped;
    ae.isTic            = (ticOrToc == TIC);

    mJitter.roll->Add(std::fabs(wrapped));  // scatter magnitude for AI prompt

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
    // DiffPeriod / AvgPeriod: per-beat deviation from expected interval in ms.
    // delta = -(instError[n] - instError[n-1]) * 1000:
    //   positive → beat arrived later than expected (watch slow)
    //   negative → beat arrived earlier than expected (watch fast)
    if (mRate.havePrevInstError) {
        double beatDevMs = -(instError - mRate.prevInstErrorSec) * 1000.0;
        mPeriod.shortRoll->Add(beatDevMs);
        mPeriod.sumAll += beatDevMs;
        mPeriod.countAll++;
    }
    mRate.prevInstErrorSec  = instError;
    mRate.havePrevInstError = true;

    return false;   // accepted as a real beat
}

void MeasurementEngine::computeBeatError(double evTime, bool, int)
{
    mBeat.times[mBeat.idx] = evTime;
    mBeat.idx++;
    if (mBeat.idx == 3) {
        double t1 = (mBeat.times[1] - mBeat.times[0]) / mSamplesPerSecond;
        double t2 = (mBeat.times[2] - mBeat.times[1]) / mSamplesPerSecond;
        double beMs = qAbs(((t1 - t2) / 2.0) * 1000.0);
        if (beMs <= kBeatErrorMaxMs)         // drop non-physical (tap) samples
            mBeat.roll->Add(beMs);
        mLastDiffTicTac = (t1 - t2) * 1000.0; // signed: positive when tic > toc
        mHaveDiffTicTac = true;
        mBeat.times[0] = mBeat.times[2];
        mBeat.idx       = 1;
    }
}

void MeasurementEngine::computeAmplitude(double cTime, bool synced, int bph, WatchMetrics &metrics, AcousticEvent &ae)
{
    if (!mAmp.haveA || !mRate.bphValid) return;
    double T1  = (cTime - mAmp.lastA) / mSamplesPerSecond;
    double amp = (3600.0 * mLiftAngle) / (M_PI * bph * T1);
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
            mAsym.roll->Add(std::fabs(mAmp.ticAmp - amp));  // asymmetry for AI prompt
            mAmp.ticValid = false;
        }
    }
    if (mAmp.roll->CurrentSize() > 0)
        metrics.amplitude = mAmp.roll->GetAverage();
}
