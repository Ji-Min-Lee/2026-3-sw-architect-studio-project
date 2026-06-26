# DSP pipeline thread model view

This view shows the runtime component-and-connector structure of the audio processing pipeline. It captures which components exist at runtime, which threads they run on, and how data flows between them via connectors (ring buffer, Qt QueuedConnection). It is the primary view for reasoning about real-time performance and latency.

![DSP Pipeline Thread Model](../../assets/view3-thread-model-simple.png)

## Element Catalog

#### Audio Source Thread
- Dedicated Qt thread running `AudioWorker` (or `PlaybackWorker` / `SimWorker`).
- Produces one PCM block (~21ms at 96kHz) per callback and writes it to the Audio Buffer.
- Real-time constraint: must complete the write within the callback period or the block is dropped.

#### Audio Buffer (Audio Source Thread → DSP Thread boundary)
- SPSC ring buffer; the only data path between Audio Source Thread and DSP Thread.
- Uses a mutex for index sync only — data copy is always lock-free.

#### DSP Thread [ADR-001]
- Dedicated Qt thread introduced to offload signal processing from the Qt main thread.
- Reads PCM from the Audio Buffer; runs `FilterChain` → `BeatDetector` → `MeasurementEngine`; emits `measurementReady`.
- Delivers measurement result to Qt Main Thread via `Qt::QueuedConnection`.
- Result: wait_ms 77.4ms → 0.03ms (×2,600); deadline miss 43% → 0%.

#### Qt Main Thread
- Receives measurement result via `QueuedConnection`; renders graph tabs.
- No further thread crossings downstream.

## Behavior

**Normal beat processing path:**

```
IAudioSource / AudioWorker (Audio Source Thread, ~21ms period)
    │  write PCM block
    ▼
Audio Buffer (SPSC ring buffer)
    │  read PCM block
    ▼
DSPWorker (DSP Thread, separate core) [ADR-001]
    │  tg_process() → FilterChain → BeatDetector → MeasurementEngine
    │  Qt::QueuedConnection (cross-thread)
    ▼
MeasurementEngine (DSP Thread) → emit measurementReady(Measurement)
    │
    ▼  [Qt Main Thread]
GraphTabManager → BaseGraphTab::onMeasurement(m)
    │  isVisible() guard (R1) [ADR-002]
    └─▶ update() → paintEvent() → replot()
```

Measured results on RPi: → [EXP-02: End-to-End Latency](../experiments/exp-02-latency-e2e.md)

## Related ADRs

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — introduces `DSPWorker` thread and Audio Buffer ring buffer
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard removing `replot()` from exec path
- [ADR-003: Audio Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md) — determines block period (96kHz → ~21ms) and Beat Error resolution

## Related views

- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — module structure that this runtime view instantiates
- [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) — detail of the Presentation layer components at the right end of this pipeline
- [Deployment View: Build-Deploy Pipeline](view-deployment-build-pipeline.md) — shows which hardware nodes run the threads depicted here
