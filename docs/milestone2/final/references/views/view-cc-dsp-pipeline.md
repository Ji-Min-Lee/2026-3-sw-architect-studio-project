# C&C View: DSP Pipeline Thread Model

This view shows the runtime component-and-connector structure of the audio processing pipeline. It captures which components exist at runtime, which threads they run on, and how data flows between them via connectors (ring buffer, Qt QueuedConnection). It is the primary view for reasoning about real-time performance and latency.

![DSP Pipeline Thread Model](../../assets/view3-thread-model.png)

## Element Catalog

#### SessionController (lifecycle owner — cross-cutting)
- Extracted from `MainWindow` (i1 refactor); owns the T1 and T2 thread lifecycle.
- Holds `IAudioSource*`, `QThread*` (source thread + DSP thread), `DSPWorker*`, and `AudioRingBuffer`.
- Exposes `startLive()`, `startPlayback()`, `startSim()`, `stop()` to `MainWindow`.
- Depends on `IAudioSource` interface, not concrete worker classes — enables adding a new audio source without touching `SessionController`.

#### IAudioSource / AudioWorker (Audio Source Thread — T1)
- `IAudioSource` is the abstract interface (`dataReady(int64_t emitTimestampUs)`, `finished()`, `sourceComplete()` signals).
- `AudioWorker` (ALSA live mic), `PlaybackWorker` (WAV file), `SimWorker` (synthetic signal) each `«realize»` `IAudioSource`.
- Fires `dataReady` every ~10ms at 96kHz (2048-sample block); writes raw PCM into `AudioRingBuffer`.
- Must complete within the audio callback period or the block is dropped.

#### AudioRingBuffer (Ring Buffer Connector)
- Lock-free single-producer / single-consumer PCM ring buffer (i3 refactor replacing mutex-based buffer).
- Decouples the audio source thread (T1) from the DSP worker thread (T2).
- Eliminates mutex lock contention between T1 and T2.

#### DSPWorker (DSP Thread — T2) [ADR-001]
- Dedicated Qt thread introduced by ADR-001.
- Reads PCM blocks from `AudioRingBuffer`; calls `tg_process()` which runs `FilterChain` → `BeatDetector`.
- Emits `BeatEvent` (T1/T3 timestamps) via Qt `QueuedConnection` to `MeasurementEngine`.
- Runs on a separate CPU core from the Qt GUI thread; eliminates single-core saturation.

#### MeasurementEngine (Qt Main Thread)
- Receives `BeatEvent` via Qt `QueuedConnection` (cross-thread signal delivery).
- Computes Rate (s/d), Amplitude (°), Beat Error (ms), BPH.
- Emits `Measurement` struct (SignalFrame + WatchMetrics + AcousticEvent list) to all registered tabs.

#### GraphTabManager / BaseGraphTab (Qt Main Thread)
- Receives `Measurement`; calls `updateData()` on each of the 14 `BaseGraphTab` instances.
- `isVisible()` guard (ADR-002 R1) skips `replot()` for non-visible tabs.

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
