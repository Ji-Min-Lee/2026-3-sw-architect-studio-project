#pragma once
#include <QVector>
#include <QMetaType>
#include <optional>
#include <cstdint>

// Per-event data for one A(T1) or C(T3) acoustic event in a processed block.
// Value Object — immutable once produced by MeasurementEngine.
struct AcousticEvent {
    double samplePos;        // absolute sample index (floating-point sub-sample precision)
    bool   isA;              // true = A(T1), false = C(T3)
    float  peakValue;

    bool   cOnsetValid;      // only meaningful when isA==false
    double cOnsetPos;        // onset sample position (when useOnset flag is set)

    // Rate scatter point (valid only for A-events when synced)
    bool   hasRatePoint;
    double wrappedRateError; // ms, for RatePlot Y-axis scatter
    bool   isTic;            // true=Tic (even beat), false=Toc (odd beat)

    // Escapement: A→C interval in ms (filled on C-events)
    bool   hasEscapementMs;
    double escapementMs;     // T1→T3 interval

    // Per-beat amplitude for Tic/Toc split display (filled on C-events)
    bool   hasAmpSplit;
    double ticAmpDeg;
    double tocAmpDeg;
};

// Signal Processing Context — raw and processed PCM for one tg_process block.
// Value Object — immutable snapshot of one audio block.
struct SignalFrame {
    QVector<double> pcm;           // envelope-filtered, for ScopePlot
    QVector<double> threshold;
    QVector<float>  hpfPcm;        // bipolar HPF output, delay-aligned with pcm
    QVector<float>  rawPcm;        // pre-DSP, for SoundPrintTab
    uint64_t        tickStart = 0;
    uint64_t        tickEnd   = 0;
    int             samplesPerSecond = 48000; // signal interpretation unit
};

// Watch Analysis Context — rolling-average measurement outcomes.
// Value Object — immutable publish snapshot; absent means not yet valid.
struct WatchMetrics {
    std::optional<double> rate;       // s/day  (RLS average)
    std::optional<double> amplitude;  // degrees (rolling average)
    std::optional<double> beatError;  // ms      (rolling average)
};

// Single Measurement emitted by MeasurementEngine after each tg_process call.
// All tabs receive the same instance — AP-4 single-source consistency guarantee.
struct Measurement {
    SignalFrame            signal;
    QVector<AcousticEvent> events;

    // Sync / BPH (from tg_result_t)
    bool    synced      = false;
    int     detectedBph = 0;

    WatchMetrics metrics;

    // QAS-4: true when no A-event received for >= 3 s (set by MeasurementEngine)
    bool    noSignal = false;

    // Handling-noise (tap) events rejected in this block (telemetry / EXP-04)
    int     handlingNoiseRejected = 0;

    // Ambient noise level of this block (dB estimate). MainWindow raises the
    // "too noisy to measure" popup when this stays above the threshold.
    double  noiseDb = 0.0;
};
Q_DECLARE_METATYPE(Measurement)
