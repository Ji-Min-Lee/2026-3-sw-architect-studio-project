# Layered View: 4-Layer Allowed-to-Use

This view shows the four-layer module structure of the TimeGrapher system and the allowed dependency directions between layers. It is the primary tool for enforcing modifiability: any new graph tab must be added to Presentation only, with zero changes to lower layers.

![4-Layer Allowed-to-Use View](../../assets/view1-layered-module.png)

## Element Catalog

#### Acquisition Layer
- Contains `AudioCapture` — the ALSA callback handler that reads PCM blocks from the microphone.
- Writes raw PCM samples into the `SignalBuffer` ring buffer.
- Has no knowledge of signal processing algorithms or domain concepts.

#### Signal Processing Layer
- Contains `FilterChain` (LP/HP bandpass), `BeatDetector`, and `DSPWorker`.
- `DSPWorker` runs on a dedicated thread (ADR-001 T2); reads from `SignalBuffer` via lock-free ring buffer.
- Emits `BeatEvent` (T1/T3 timestamps) to the Domain layer via Qt `QueuedConnection`.

#### Domain Layer
- Contains `MeasurementEngine`, which consumes `BeatEvent` objects and computes Rate (s/d), Amplitude (°), and Beat Error (ms).
- Emits a single `Measurement` struct via Qt Signal-Slot to all subscribers in Presentation.
- No display logic; no direct reference to any graph widget.

#### Presentation Layer
- Contains `GraphTabManager` and all 11 `IGraphTab` implementations (TraceDisplay, VarioDisplay, BeatErrorDisplay, …).
- Each tab subscribes to `MeasurementEngine`'s signal and calls `updateData(Measurement)`.
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
| Implement `IGraphTab` subclass | `GraphFoo.cpp` + `GraphFoo.h` | 1–2 |
| Register in tab manager | `GraphTabManager.cpp` | 1 |
| **Total** | | **≤ 3** |

Zero changes to Domain, Signal Processing, or Acquisition layers required.

## Related ADRs

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — introduces `DSPWorker` as a separate thread between Acquisition and Signal Processing
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard placed in Presentation layer `updateData()` implementations

## Related views

- [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) — refinement of the Presentation layer; shows internal structure of a single `IGraphTab`
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view of the Acquisition ↔ Signal Processing boundary (ring buffer + thread)
