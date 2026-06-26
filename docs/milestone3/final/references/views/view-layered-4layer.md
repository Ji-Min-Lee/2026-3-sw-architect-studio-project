# TimeGrapher Layered View

This view shows the four-layer module structure of the TimeGrapher system and the allowed dependency directions between layers. Its purpose is not just to name the layers, but to show the architectural rule that keeps change localized: new UI features stay in Presentation, new audio sources stay in Acquisition, and lower-layer changes do not ripple upward. It is the primary evidence for QAS-3 (Extensibility / Modifiability).

![4-Layer Allowed-to-Use View](../../assets/view1-layered-module.png)

## When to Show This View

Show this view first when the discussion starts with any of these questions:

- "How is the system modularized?"
- "Why is the architecture easy to extend?"
- "Why can you add a new graph tab without touching DSP or audio capture?"

This view works best as the entry point because it establishes the global dependency rule before zooming into any one extension point.

Recommended follow-up views:

| If the audience asks... | Show this view next | Why |
|---|---|---|
| "What exactly must be implemented to add a new graph tab?" | [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) | It refines the Presentation layer and shows the `BaseGraphTab` observer contract and tab registration steps. |
| "What exactly must be implemented to add a new audio source?" | [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) | It refines the Acquisition layer and shows why a new source is isolated behind `IAudioSource`. |
| "How does data actually move across threads at runtime?" | [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) | It complements this static module view with the runtime thread and connector path. |

In a presentation, the recommended sequence is:

`Layered View` → `Graph Tab` for UI extensibility questions  
`Layered View` → `IAudioSource` for source extensibility questions  
`Layered View` → `C&C Pipeline` for runtime and latency questions

## Element Catalog

#### UI Coordinator (cross-cutting — not a numbered layer)
- Contains `MainWindow` (Qt top-level window), `SessionController` (owns T1+T2 thread lifecycle), and `DiagnosisDialog`.
- `SessionController` was extracted from `MainWindow` (i1 refactor) to reduce MainWindow from ~949 to ~750 lines.

#### Acquisition Layer
- Contains `IAudioSource` (abstract Qt interface), `AudioWorker` (live mic, ALSA), `PlaybackWorker` (WAV file), `SimWorker` (synthetic signal), and `AudioRingBuffer` (lock-free SPSC ring buffer).
- Extension point for new audio sources: see [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md).

#### Signal Processing Layer
- Contains `DSPWorker` [ADR-001], `MeasurementEngine`, `FilterChain` (LP/HP bandpass), `BeatDetector`, and `tg_process` (external library in `src/external/`).
- `DSPWorker` runs on a dedicated thread (T2); reads PCM blocks from `AudioRingBuffer` via lock-free read.
- `DSPWorker` directly owns `MeasurementEngine` as a member (`DSPWorker.h:37`) and calls `mEngine->processBlock()` within the T2 DSP loop — they form a single T2 pipeline unit.
- `MeasurementEngine` emits a `Measurement` struct via Qt Signal-Slot to Presentation (cross-thread, `QueuedConnection`).
- **Layer boundary note**: `MeasurementEngine` is classified here (not Domain) because it is owned and driven by `DSPWorker` within the T2 DSP loop — it is not a stand-alone domain service. Pure domain objects (`Measurement` VO, `WatchMath`, `WatchDiagnostics`) remain in the Domain layer and have no upward dependencies.

#### Domain Layer
- Contains pure computation and value objects only: `WatchMath`, `WatchDiagnostics` (rule-based diagnosis), `WatchExplainer` (Ollama LLM, on-device), and VOs `Measurement`, `SignalFrame`, `WatchMetrics`, `AcousticEvent`, `MovementSpec`.
- No thread ownership, no Qt signal emission, no display logic.
- `WatchDiagnostics` reads the `Measurement` VO produced by Sig.Proc; `WatchExplainer` reads the diagnosis result.

#### Presentation Layer
- Contains `GraphTabManager` and all 14 `BaseGraphTab` implementations.
- **14 tabs**: TraceTab, RateScopeTab, SweepScopeTab, FilterScopeTab, BeatNoiseScopeTab, SoundPrintTab, VarioTab, BeatErrorTab, EscapementTab, LongTermTab, SequenceTab, SpectrogramTab, WaveformCompTab, RadarChartTab.
- Each tab subscribes to `MeasurementEngine`'s signal and calls `updateData(Measurement)`.
- `isVisible()` guard (ADR-002 R1) skips `replot()` for non-visible tabs.
- Allowed to reference Domain only; references to Signal Processing or Acquisition are forbidden.

## Behavior

The main message of this view is the **allowed-to-use rule**:

- Each layer may reference only the layer immediately below it.
- No upward dependency is allowed.
- No skip-layer dependency is allowed.
- Extension work must remain inside the owning layer whenever possible.

```
Acquisition
    ↑ forbidden
Signal Processing  →  Acquisition (reads ring buffer)
    ↑ forbidden
Domain             →  Signal Processing (receives BeatEvent)
    ↑ forbidden
Presentation       →  Domain (receives Measurement struct)
```

This rule matters because it constrains change impact. The clearest example is graph-tab addition.

**Adding a new graph tab** (≤ 3 file change rule):

| Step | File | Count |
|------|------|:-----:|
| Implement `BaseGraphTab` subclass | `FooTab.cpp` + `FooTab.h` | 1–2 |
| Register in tab manager | `GraphTabManager.cpp` | 1 |
| **Total** | | **≤ 3** |

Zero changes to Domain, Signal Processing, or Acquisition layers are required. That is the practical meaning of "modifiability" in this system: the architecture lets a developer make the requested change without reopening unrelated modules.

The layer rule is verified by an actual `#include` trace — all dependencies fall in the lower triangle with no upward dependency. A boundary violation causes a compiler error due to per-layer include restrictions.

**Change cost** (response measure for modifiability scenarios):

| Change type | Files outside the changed unit |
|---|:---:|
| New graph tab | ≤ 3 |
| New audio source | 1–2 |
| New watch formula | 1 |
| New DSP filter | 1–2 |

The 4-layer structure implements the *Restrict Dependencies* tactic (Bass, Clements & Kazman, Ch.8): lower-layer changes do not ripple upward, and per-layer compilation isolation enables 142 independent unit tests across 10 binaries.

## Related ADRs

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — introduces `DSPWorker` as a separate thread between Acquisition and Signal Processing
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard placed in Presentation layer `updateData()` implementations

## Related views

- [Graph Tab Module Uses View](view-decomposition-graph-tab.md) — refinement of the Presentation layer; shows internal structure of a single `BaseGraphTab`
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view of the Acquisition ↔ Signal Processing boundary (ring buffer + thread)
- [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) — AS-IS vs TO-BE comparison of the audio source extension point (QAS-3)
