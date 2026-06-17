# P4: Measurement Struct Decomposition

## Summary

Decomposed the `Measurement` god-struct into two Value Objects (`SignalFrame`, `WatchMetrics`)
aligned with the two Bounded Contexts identified in the TimeGrapher domain model.

---

## AS-IS

![AS-IS Module View](assets/module-view-p4-asis.png)

`Measurement` was a single flat struct holding four unrelated concerns:

| Concern | Fields |
|---------|--------|
| Signal data | `pcm`, `hpfPcm`, `rawPcm`, `threshold`, `graphTickStart`, `graphTickEnd`, `samplesPerSecond` |
| Acoustic events | `QVector<AcousticEvent> events` |
| Sync state | `synced`, `detectedBph` |
| Watch metrics | `rateValid+rateErrorSpd`, `beatErrorValid+beatErrorMs`, `amplitudeValid+amplitudeDeg` |

All 13 tabs and `MainWindow` received the same `const Measurement &m` and accessed
whatever subset they needed — mixing fields from unrelated concerns in the same call site.

**Problems**

- Every consumer was implicitly coupled to all four concerns.
- `bool valid + double value` pairs (6 fields for 3 metrics) are a nullable anti-pattern —
  the compiler cannot enforce that `rateErrorSpd` is only read when `rateValid` is true.
- `samplesPerSecond` looked like a leaked `AcquisitionConfig` value, obscuring its real role
  as the signal interpretation unit.

---

## TO-BE

![TO-BE Module View](assets/module-view-p4-tobe.png)

`Measurement` is restructured as a thin container holding two named Value Objects plus
flat fields for sync state:

```cpp
struct SignalFrame {              // Signal Processing Context
    QVector<double> pcm, threshold;
    QVector<float>  hpfPcm, rawPcm;
    uint64_t        tickStart = 0, tickEnd = 0;
    int             samplesPerSecond = 48000;
};

struct WatchMetrics {             // Watch Analysis Context
    std::optional<double> rate;       // s/day
    std::optional<double> amplitude;  // degrees
    std::optional<double> beatError;  // ms
};

struct Measurement {
    SignalFrame              signal;
    QVector<AcousticEvent>   events;
    bool                     synced      = false;
    int                      detectedBph = 0;
    WatchMetrics             metrics;
    bool                     noSignal    = false;
};
```

---

## Rationale

### 1. Bounded Context alignment

The TimeGrapher domain has two distinct Bounded Contexts with different ubiquitous languages:

| Context | Language | Fields |
|---------|----------|--------|
| **Signal Processing** | sample, tick, onset, HPF, threshold | `signal.*` |
| **Watch Analysis** | rate (s/day), amplitude (°), beat error (ms) | `metrics.*` |

Mapping code structure to Bounded Context boundaries reduces cognitive load —
a tab author working on signal visualization does not need to reason about `metrics`,
and vice versa.

### 2. Value Object identity criterion (Larman OOAD)

Both `SignalFrame` and `WatchMetrics` satisfy the Value Object definition:

- **No OID needed** — two `SignalFrame`s with identical field values represent the same
  audio block; two `WatchMetrics` with the same rate/amplitude/beatError are the same result.
- **Immutable** — both are produced once by `MeasurementEngine` and published read-only
  to all subscribers; no consumer mutates them.

The flat fields (`synced`, `detectedBph`, `noSignal`) are kept at the top level of
`Measurement` because they are cross-cutting — both Bounded Contexts reference them.

### 3. `samplesPerSecond` belongs in `SignalFrame`, not `AcquisitionConfig`

Although `samplesPerSecond` originates from `AcquisitionConfig`, its role at the consumer
side is to interpret PCM time positions (convert sample indices to milliseconds, compute
Hz values for display). This is a Signal Processing Context responsibility, not a watch
analysis one. Placing it in `SignalFrame` makes that role explicit and removes the
appearance of a config value leaking into the published measurement.

### 4. `bool valid + double value` → `std::optional<double>`

The six nullable fields for three metrics (`rateValid`, `rateErrorSpd`, etc.) were
a C-style nullable anti-pattern. `std::optional<double>` expresses the same semantics
with compiler enforcement: a consumer cannot read `*m.metrics.rate` without first
testing `m.metrics.rate.has_value()` (or using `value_or()`).

### 5. Why decompose before other P2/P3 refactors

The `WatchMetrics` Value Object could not be properly extracted while it was embedded
in the god-struct — its form (6 fields) was dictated by the struct's layout, not by
domain semantics. Decomposing `Measurement` first establishes the correct domain shape,
after which `MovementSpec` (P2) and `AcousticEvent` optional fields (P3) can be addressed
independently.

---

## Tab Consumer Classification

| Consumer group | Bounded Context used | Fields accessed |
|----------------|---------------------|-----------------|
| BeatNoiseScopeTab, EscapementTab, FilterScopeTab, SoundPrintTab | Signal only | `signal`, `events` |
| SequenceTab, LongTermTab, TraceTab, VarioTab | Analysis only | `metrics` |
| RateScopeTab, SpectrogramTab, WaveformCompTab, SweepScopeTab, BeatErrorTab | Both | `signal` + `events` + `metrics` |
| MainWindow | Sync + Analysis | `noSignal`, `synced`, `detectedBph`, `metrics` |

Tabs spanning both contexts are expected — Witschi-style scope displays naturally overlay
rate/amplitude labels on waveform plots.

---

## Files Changed

| File | Change |
|------|--------|
| `src/engine/Measurement.h` | Add `SignalFrame`, `WatchMetrics`; restructure `Measurement` |
| `src/engine/MeasurementEngine.h` | Update `computeAmplitude` signature (`Measurement&` → `WatchMetrics&`) |
| `src/engine/MeasurementEngine.cpp` | Populate `signal.*` and `metrics.*`; replace bool pairs with `std::optional` assignment |
| `src/tabs/*.cpp` (13 files) | `m.pcm` → `m.signal.pcm`, `m.rateValid` → `m.metrics.rate.has_value()`, etc. |
| `src/ui/MainWindow.cpp` | Same field migration for Results label and DiagnosisInput |
