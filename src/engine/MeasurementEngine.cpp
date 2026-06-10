#include "MeasurementEngine.h"
#include "WatchMath.h"
#include <QtMath>
#include <QDebug>
#include <cmath>

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

    mNoSignalTimerStarted = false;
}

// ──────────────────────────────────────────────────────────────────────────────
// Public slot — receives one block of raw float PCM from the ring buffer
// ──────────────────────────────────────────────────────────────────────────────
void MeasurementEngine::processBlock(const float *pcm, int numSamples)
{
    if (!mCtx) return;

    tg_result_t r;
    if (tg_process(mCtx, pcm, numSamples, &r) != 0) {
        qWarning() << "MeasurementEngine: tg_process failed";
        return;
    }

    Measurement m;
    m.samplesPerSecond = mSamplesPerSecond;
    m.graphTickStart   = mGraphTicks;
    m.synced           = (r.sync_status == TG_SYNC_SYNCED);
    m.detectedBph      = r.detected_bph;

    // Raw PCM for SoundPrintTab (copy before any DSP transformation)
    m.rawPcm.reserve(numSamples);
    for (int i = 0; i < numSamples; i++) m.rawPcm.append(pcm[i]);

    // Processed PCM + threshold for ScopePlot
    m.pcm.reserve(r.processed_pcm_len);
    m.threshold.reserve(r.processed_pcm_len);
    for (int i = 0; i < r.processed_pcm_len; i++) {
        m.pcm.append(r.processed_pcm[i]);
        m.threshold.append(r.onset_threshold);
    }
    mGraphTicks    += r.processed_pcm_len;
    m.graphTickEnd  = mGraphTicks;

    // Process each A/C event
    for (int i = 0; i < r.num_events; i++) {
        const tg_event_t &ev = r.events[i];
        AcousticEvent ae;
        ae.cOnsetValid       = false;
        ae.hasRatePoint      = false;
        ae.wrappedRateError  = 0.0;
        ae.isTic             = false;
        ae.hasEscapementMs   = false;
        ae.escapementMs      = 0.0;
        ae.hasAmpSplit       = false;
        ae.ticAmpDeg         = 0.0;
        ae.tocAmpDeg         = 0.0;

        if (ev.type == TG_EVENT_A) {
            ae.isA       = true;
            ae.samplePos = ev.sample_index + ev.sub_sample_offset;
            ae.peakValue = ev.peak_value;

            // QAS-4: reset no-signal timer on each A-event
            mNoSignalTimer.restart();
            mNoSignalTimerStarted = true;

            computeRateError(ae.samplePos, m.synced, r.detected_bph, ae);
            computeBeatError(ae.samplePos, m.synced, r.detected_bph);

            mAmp.haveA = true;
            mAmp.lastA = ae.samplePos;
            mHaveLastA = true;
            mLastA     = ae.samplePos;
        } else if (ev.type == TG_EVENT_C) {
            ae.isA         = false;
            ae.peakValue   = ev.peak_value;
            ae.cOnsetValid = ev.onset_valid;
            ae.cOnsetPos   = ev.onset_sample_index + ev.onset_sub_sample_offset;

            if (mUseOnset && ev.onset_valid)
                ae.samplePos = ae.cOnsetPos;
            else
                ae.samplePos = ev.sample_index + ev.sub_sample_offset;

            // Escapement: T1→T3 interval
            if (mHaveLastA) {
                ae.hasEscapementMs = true;
                ae.escapementMs    = WatchMath::escapementMs(mLastA, ae.samplePos, mSamplesPerSecond);
            }

            computeAmplitude(ae.samplePos, m.synced, r.detected_bph, m, ae);
        } else {
            qWarning() << "MeasurementEngine: unknown event type";
            continue;
        }
        m.events.append(ae);
    }

    m.rateValid      = mRate.rateValid;
    m.rateErrorSpd   = mRate.rateSpd;
    m.beatErrorValid = (mBeat.roll->CurrentSize() > 0);
    m.beatErrorMs    = m.beatErrorValid ? mBeat.roll->GetAverage() : 0.0;
    m.amplitudeValid = (mAmp.roll->CurrentSize() > 0);
    m.amplitudeDeg   = m.amplitudeValid ? mAmp.roll->GetAverage() : 0.0;
    m.noSignal       = mNoSignalTimerStarted && (mNoSignalTimer.elapsed() > kNoSignalThresholdMs);

    emit measurementReady(m);
}

// ──────────────────────────────────────────────────────────────────────────────
// Private helpers  (extracted from MainWindow)
// ──────────────────────────────────────────────────────────────────────────────
double MeasurementEngine::wrapInRange(double n, double lo, double hi) const
{
    return WatchMath::wrapInRange(n, lo, hi);
}

void MeasurementEngine::addOrOverwrite(QVector<double> &xv, QVector<double> &yv,
                                        double val, int maxS, int &idx)
{
    if (yv.size() < maxS) { yv.append(val); xv.append(idx); }
    else { yv[idx] = val; }
    idx = (idx + 1) % maxS;
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
        mRate.zeroOffset     = instErrorMs;  // first error becomes the anchor
    }
    instErrorMs = WatchMath::applyZeroOffset(mRate.zeroOffset, instErrorMs);

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
        mBeat.roll->Add(WatchMath::beatErrorMs(
            mBeat.times[0], mBeat.times[1], mBeat.times[2], mSamplesPerSecond));
        mBeat.times[0] = mBeat.times[2];
        mBeat.idx       = 1;
    }
}

void MeasurementEngine::computeAmplitude(double cTime, bool synced, int bph, Measurement &m, AcousticEvent &ae)
{
    if (!mAmp.haveA || !mRate.bphValid) return;
    double amp = WatchMath::amplitudeDeg(mAmp.lastA, cTime, mSamplesPerSecond, mLiftAngle, bph);
    if (amp < 0) {
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
