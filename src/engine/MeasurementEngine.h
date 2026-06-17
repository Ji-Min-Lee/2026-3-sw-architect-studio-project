#pragma once
#include <QObject>
#include <QElapsedTimer>
#include "Timegrapher.h"
#include "RollingLeastSquares.h"
#include "RollingAverage.h"
#include "SharedAudio.h"
#include "Measurement.h"

// MVC: Model (Subject).
// Owns the tg_context DSP pipeline and all measurement state.
// Emits measurementReady() — Observer notify() — after every tg_process call.
// MainWindow (Controller) connects tabs (Observer) via Qt Signal-Slot.
class MeasurementEngine : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementEngine(QObject *parent = nullptr);
    ~MeasurementEngine();

    // Called once before starting audio.  bph==0 means auto-BPH mode.
    void init(int sampleRate, int bph, double liftAngle, int averagingPeriod, double hpfCutoff);
    void destroy();
    void reset();

    void setUseOnset(bool on) { mUseOnset = on; }

public slots:
    // Feed one block of raw PCM (from ring buffer).  Emits measurementReady when done.
    void processBlock(const float *pcm, int numSamples);

signals:
    // AP-4: single publication source.  All 11 tabs subscribe to this signal.
    void measurementReady(const Measurement &m);

private:
    double wrapInRange(double value, double lo, double hi) const;
    void   addOrOverwrite(QVector<double> &xv, QVector<double> &yv,
                          double val, int maxSize, int &idx);
    void   computeRateError(double evTime, bool synced, int bph, AcousticEvent &ae);
    void   computeBeatError(double evTime, bool synced, int bph);
    void   computeAmplitude(double cTime, bool synced, int bph, Measurement &m, AcousticEvent &ae);

    // Handling-noise rejection: discard impulsive events caused by tapping the
    // watch or sensor, while preserving the real A/C beat events. Event-level
    // (O(1) per event, no per-sample cost) — real-time safe on the RPi.
    bool   isHandlingNoise(double samplePos, float peak) const;

    // DSP pipeline
    tg_config_t   mCfg{};
    tg_context_t *mCtx      = nullptr;
    bool          mUseOnset = false;

    int      mSamplesPerSecond = 48000;
    double   mLiftAngle        = 52.0;
    int      mAveragingPeriod  = 20;
    uint64_t mGraphTicks       = 0;

    // Rate state  (extracted from TRateErrorEvents)
    struct RateState {
        uint64_t beatNumber     = 0;
        bool     haveStart      = false;
        double   startTime      = 0.0;
        bool     haveZeroOffset = false;
        double   zeroOffset     = 0.0;
        bool     bphValid       = false;
        int      bph            = 0;
        int      watchHz        = 0;
        int      maxPoints      = 250;
        int      xTicIdx        = 0;
        int      xTocIdx        = 0;
        QVector<double> xTic, yTic, xToc, yToc;
        RollingLeastSquares *rlsTic = nullptr;
        RollingLeastSquares *rlsToc = nullptr;
        bool   rateValid = false;
        double rateSpd   = 0.0; // s/day
    } mRate;

    // Beat-error state  (extracted from TBeatErrorEvents)
    struct BeatState {
        double times[3] = {};
        int    idx      = 0;
        RollingAverage *roll = nullptr;
    } mBeat;

    // Amplitude state  (extracted from TAmplitudeErrorEvents)
    struct AmpState {
        bool   haveA    = false;
        double lastA    = 0.0;
        double ticAmp   = 0.0;
        bool   ticValid = false;
        RollingAverage *roll = nullptr;
    } mAmp;

    double mLastA     = 0.0;
    bool   mHaveLastA = false;

    // Handling-noise (tap) rejection state. Thresholds are relative (median×K)
    // so faint watches' real A/C are preserved; tune via EXP-03/04.
    struct HandlingNoiseState {
        QVector<double> recentPeaks;       // last accepted beat peaks (rolling)
        double lastAcceptedPos = -1.0;     // sample index of last accepted event
        static constexpr int    kWindow       = 24;   // peaks kept for the median
        static constexpr int    kMinForOutlier = 6;   // need a baseline before rejecting
        static constexpr double kAmplitudeK   = 5.0;  // reject if peak > median × K
        static constexpr double kRefractoryMs = 3.0;  // reject events closer than this
    } mHandling;

    // QAS-4: no-signal detection (owned by Model, not Controller)
    QElapsedTimer mNoSignalTimer;
    bool          mNoSignalTimerStarted = false;
    static constexpr qint64 kNoSignalThresholdMs = 3000;
};
