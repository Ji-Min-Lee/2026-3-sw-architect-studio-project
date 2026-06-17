#pragma once
#include <QVector>
#include <cstdint>

// Per-event data for one A(T1) or C(T3) acoustic event in a processed block
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

// Single Measurement emitted by MeasurementEngine after each tg_process call.
// All tabs receive the same instance — AP-4 single-source consistency guarantee.
struct Measurement {
    // Processed (envelope-filtered) PCM block for ScopePlot
    QVector<double> pcm;
    QVector<double> threshold;

    // HPF output (bipolar, delay-aligned with pcm) — F1 / "raw" scope view
    QVector<float>  hpfPcm;

    uint64_t        graphTickStart = 0;
    uint64_t        graphTickEnd   = 0;

    // Raw PCM (before DSP) — needed by SoundPrintTab for SoundImageRenderer
    QVector<float>  rawPcm;

    // Events in this block
    QVector<AcousticEvent> events;

    // Sync / BPH (from tg_result_t)
    bool    synced      = false;
    int     detectedBph = 0;

    // Rolling-average values for the Results label
    bool    rateValid      = false;
    double  rateErrorSpd   = 0.0; // s/day (RLS average)

    bool    beatErrorValid = false;
    double  beatErrorMs    = 0.0; // ms (rolling average)

    bool    amplitudeValid = false;
    double  amplitudeDeg   = 0.0; // degrees (rolling average)

    // QAS-4: true when no A-event received for >= 3 s (set by MeasurementEngine)
    bool    noSignal = false;

    // Handling-noise (tap) events rejected in this block (telemetry / EXP-04)
    int     handlingNoiseRejected = 0;

    int     samplesPerSecond = 48000;
};
