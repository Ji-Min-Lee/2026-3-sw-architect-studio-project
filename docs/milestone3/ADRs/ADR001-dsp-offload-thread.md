# ADR 001: Introduce Concurrency — Offload DSP to a Dedicated Thread

EXP-02 macOS baseline revealed that the DSP pipeline (FilterChain + BeatDetector + MeasurementEngine) was executing inside the Audio Thread, which forced it to wait on the Qt main event loop before dispatching `Measurement` objects to the UI. This wait (`wait_ms` avg 420 ms) caused a 47% audio block backlog and would have caused complete pipeline breakdown on Raspberry Pi 5.

## Decision

Introduce a dedicated **DSP Thread** (`DSPWorker`) that owns `FilterChain`, `BeatDetector`, and `MeasurementEngine`. The Audio Thread writes PCM blocks into a `SignalBuffer` (ring buffer) and returns immediately; the DSP Thread reads from the buffer independently.

Cross-thread communication from DSP Thread to the UI Thread is handled exclusively by Qt's `Qt::QueuedConnection`, which automatically posts `Measurement` objects to the UI thread's event queue without any manual locking.

## Rationale

The root cause of the backlog was **structural**: the Audio Thread was blocked waiting for the Qt event loop to consume a signal before it could process the next PCM block. Moving DSP to its own thread eliminates the dependency on the Qt event loop entirely.

| Factor | Evidence |
|--------|---------|
| `wait_ms` avg 420 ms — audio thread idle waiting for Qt dispatch | EXP-02 macOS R0 |
| Backlog 47% before DSP offload | EXP-02 macOS R0 |
| Backlog 0%, `wait_ms` 0.013 ms after DSP offload | EXP-02 macOS T2 |
| RPi thermal throttle (85°C) compounds single-threaded load | EXP-02 RPi baseline |

## Rejected Alternatives

- **T1 — SCHED_RR + CPU Affinity**: Linux real-time scheduling with CPU pinning. Valid on RPi but cannot be validated on macOS (SCHED_RR is not available); does not address the Qt event-loop blocking root cause. Retained as a complementary tactic for RPi tuning.
- **T3 — Full Pipeline Split**: Split every stage (FilterChain, BeatDetector, MeasurementEngine) into separate threads connected by queues. Excessive complexity and synchronization overhead; beyond MVP scope.

## Status

Implemented. Validated by EXP-02 macOS T2 (backlog 0%). RPi validation pending EXP RPi R5.

## Consequences

- **Positive**: Audio Thread is never blocked by Qt event-loop delays; backlog drops to 0%; `wait_ms` reduced by ×32,000.
- **Positive**: DSP Thread can be pinned to a dedicated CPU core on RPi (T1 complementary tactic) for further latency reduction.
- **Trade-off**: Data shared between DSP Thread and UI Thread (`MeasurementStore`) requires a read-write lock (`std::shared_mutex`). Mitigated by Qt's `QueuedConnection` for live measurement dispatch (no manual locking needed for the hot path).
- **Trade-off**: Debugging multi-threaded race conditions is harder; mitigated by keeping the thread boundary clean (only `SignalBuffer` and `MeasurementQueue` cross it).

## Related ADRs
- [ADR 005 — Ring Buffer as Thread Boundary Connector](ADR005-ring-buffer-connector.md)
- [ADR 004 — Qt as Application Framework](ADR004-qt-framework.md)
