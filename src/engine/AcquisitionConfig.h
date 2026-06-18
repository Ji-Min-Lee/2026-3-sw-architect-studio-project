#pragma once

// Value Object — signal acquisition and analysis parameters for one session.
// All fields are fixed at session start; no identity distinguishes two configs
// with identical values.
// Immutable once passed to MeasurementEngine::init().
struct AcquisitionConfig {
    int    sampleRate      = 48000; // PCM sample rate in Hz
    double hpfCutoff       = 0.0;   // high-pass filter cutoff in Hz (0 = disabled)
    int    averagingPeriod = 20;    // rolling-average window in seconds
};
