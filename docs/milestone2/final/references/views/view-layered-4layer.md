# Layered View: 4-Layer Allowed-to-Use

This view shows the four-layer module structure of the TimeGrapher system and the allowed dependency directions between layers. It is the primary tool for enforcing modifiability: any new graph tab must be added to Presentation only, with zero changes to lower layers. A new audio source requires implementing `IAudioSource` only, with zero changes to `SessionController` or above.

![4-Layer Allowed-to-Use View](../../assets/view1-layered-module.png)

## Element Catalog

#### UI Coordinator (cross-cutting — not a numbered layer)
- Contains `MainWindow` (Qt top-level window), `SessionController` (owns T1+T2 thread lifecycle), and `DiagnosisDialog`.
- `SessionController` was extracted from `MainWindow` (i1 refactor) to reduce MainWindow from ~949 to ~750 lines.
- `SessionController` depends on `IAudioSource` (Acquisition layer) via dependency inversion; it has no knowledge of concrete worker types.

#### Acquisition Layer
- Contains `IAudioSource` (abstract Qt interface), `AudioWorker` (live mic, ALSA), `PlaybackWorker` (WAV file), `SimWorker` (synthetic signal), and `AudioRingBuffer` (lock-free SPSC ring buffer).
- Each concrete worker `«realize»` `IAudioSource`; `SessionController` depends on the interface only.
- `AudioRingBuffer` decouples the audio source thread (T1) from the DSP thread (T2) without mutex lock contention.
- Adding a new audio source = implement `IAudioSource` only; zero changes to `SessionController` or Signal Processing.

#### Signal Processing Layer
- Contains `DSPWorker` [ADR-001], `FilterChain` (LP/HP bandpass), `BeatDetector`, and `tg_process` (external library in `src/external/`).
- `DSPWorker` runs on a dedicated thread (T2); reads PCM blocks from `AudioRingBuffer` via lock-free read.
- Emits `BeatEvent` (T1/T3 timestamps) to the Domain layer via Qt `QueuedConnection`.

#### Domain / Engine Layer
- Contains `MeasurementEngine`, `WatchDiagnostics` (rule-based diagnosis), `WatchExplainer` (Ollama LLM, on-device), and value objects `SignalFrame`, `WatchMetrics`, `AcousticEvent`.
- `MeasurementEngine` consumes `BeatEvent` objects and computes Rate (s/d), Amplitude (°), Beat Error (ms).
- Emits a single `Measurement` struct (contains SignalFrame + WatchMetrics + AcousticEvent list) via Qt Signal-Slot to Presentation.
- No display logic; no direct reference to any graph widget.

#### Presentation Layer
- Contains `GraphTabManager` and all 14 `BaseGraphTab` implementations.
- **14 tabs**: TraceTab, RateScopeTab, SweepScopeTab, FilterScopeTab, BeatNoiseScopeTab, SoundPrintTab, VarioTab, BeatErrorTab, EscapementTab, LongTermTab, SequenceTab, SpectrogramTab, WaveformCompTab, RadarChartTab.
- Each tab subscribes to `MeasurementEngine`'s signal and calls `updateData(Measurement)`.
- `isVisible()` guard (ADR-002 R1) skips `replot()` for non-visible tabs.
- Allowed to reference Domain only; references to Signal Processing or Acquisition are forbidden.

## Behavior

**Dependency rule**: each layer may only reference the layer immediately below it. No upward or skip-layer dependencies are permitted.

```
Acquisition
    ↑ forbidden
Signal Processing  →  Acquisition (reads ring buffer)
    ↑ forbidden
Domain             →  Signal Processing (receives BeatEvent)
    ↑ forbidden
Presentation       →  Domain (receives Measurement struct)
```

**Adding a new graph tab** (≤ 3 file change rule):

| Step | File | Count |
|------|------|:-----:|
| Implement `BaseGraphTab` subclass | `FooTab.cpp` + `FooTab.h` | 1–2 |
| Register in tab manager | `GraphTabManager.cpp` | 1 |
| **Total** | | **≤ 3** |

Zero changes to Domain, Signal Processing, or Acquisition layers required.

**Adding a new audio source** (IAudioSource rule):

| Step | File | Count |
|------|------|:-----:|
| Implement `IAudioSource` subclass | `NetworkWorker.cpp` + `NetworkWorker.h` | 1–2 |
| **Total** | | **≤ 2** |

Zero changes to `SessionController`, DSPWorker, or any other component required.

## Related ADRs

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — introduces `DSPWorker` as a separate thread between Acquisition and Signal Processing
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard placed in Presentation layer `updateData()` implementations

## Related views

- [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) — refinement of the Presentation layer; shows internal structure of a single `BaseGraphTab`
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view of the Acquisition ↔ Signal Processing boundary (ring buffer + thread)
- [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) — AS-IS vs TO-BE comparison of the audio source extension point (QAS-3)
