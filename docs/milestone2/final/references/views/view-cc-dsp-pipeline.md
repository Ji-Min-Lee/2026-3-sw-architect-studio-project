# C&C View: DSP Pipeline Thread Model

This view shows the runtime component-and-connector structure of the audio processing pipeline. It captures which components exist at runtime, which threads they run on, and how data flows between them via connectors (ring buffer, Qt QueuedConnection). It is the primary view for reasoning about real-time performance and latency.

![DSP Pipeline Thread Model](../../assets/view3-thread-model.png)

## Element Catalog

#### T1 — Audio Source Thread
- Dedicated Qt thread running `AudioWorker` (or `PlaybackWorker` / `SimWorker`).
- Produces one PCM block (~10ms at 96kHz) per callback and writes it to `AudioRingBuffer`.
- Real-time constraint: must complete the write within the callback period or the block is dropped.

#### AudioRingBuffer (T1 → T2 boundary)
- Lock-free SPSC ring buffer; the only data path crossing the T1/T2 thread boundary.
- No mutex — T1 writes and T2 reads without blocking each other.

#### T2 — DSP Thread [ADR-001]
- Dedicated Qt thread introduced to offload signal processing from the Qt main thread.
- Reads PCM from `AudioRingBuffer`; runs `FilterChain` → `BeatDetector`; emits `BeatEvent`.
- Delivers `BeatEvent` to the main thread via Qt `QueuedConnection` (T2 → main thread boundary).
- Result: wait_ms 420ms → 0.013ms; deadline miss 43% → 0%.

#### Qt Main Thread
- Receives `BeatEvent` via `QueuedConnection`; computes measurements; renders graph tabs.
- No further thread crossings downstream.

## Behavior

**Normal beat processing path:**

```
IAudioSource / AudioWorker (T1, ~10ms period)
    │  write PCM block
    ▼
AudioRingBuffer (lock-free SPSC ring buffer)
    │  read PCM block
    ▼
DSPWorker (T2, separate core) [ADR-001]
    │  tg_process() → FilterChain → BeatDetector → BeatEvent{T1_ts, T3_ts}
    │  Qt QueuedConnection (cross-thread)
    ▼
MeasurementEngine (Qt main thread)
    │  compute Rate / Amplitude / Beat Error
    │  Qt Signal: measurement(Measurement)
    ▼
GraphTabManager → BaseGraphTab::updateData(m)
    │  isVisible() guard (R1) [ADR-002]
    └─▶ update() → paintEvent() → replot()
```

**Key timing metrics (EXP-02 macOS results):**

| Metric | Before ADR-001 (R1 baseline) | After ADR-001 (T2) | Change |
|--------|:----------------------------:|:------------------:|:------:|
| wait_ms avg | 420 ms | 0.013 ms | ×32,000 |
| deadline miss | 43% | 0% (macOS) | eliminated |
| replot/beat (R1 guard) | 8.22 | 1.20 | ↓85% |

RPi confirmation: EXP-02 R5 scheduled 2026-06-23.

## Related ADRs

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — introduces `DSPWorker` thread and `SignalBuffer` ring buffer
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard removing `replot()` from exec path
- [ADR-003: Audio Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md) — determines block period (96kHz → ~10ms) and Beat Error resolution

## Related views

- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — module structure that this runtime view instantiates
- [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) — detail of the Presentation layer components at the right end of this pipeline
- [Deployment View: Build-Deploy Pipeline](view-deployment-build-pipeline.md) — shows which hardware nodes run the threads depicted here
