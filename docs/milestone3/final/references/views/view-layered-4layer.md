# Layered View: 4-Layer Allowed-to-Use

This view shows the four-layer module structure of the TimeGrapher system and the allowed dependency directions between layers. It is the primary tool for enforcing modifiability: any new graph tab must be added to Presentation only, with zero changes to lower layers.

![4-Layer Allowed-to-Use View](../../assets/view1-layered-module.png)

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

**Dependency Structure Matrix** (actual `#include` trace):

Row = used module · Column = using module · `1` = depends · `-` = self

| used ↓ \ using → | Presentation | UI Coord | Sig.Proc | Domain | Acquisition |
|---|:---:|:---:|:---:|:---:|:---:|
| **Presentation** (`*Tab`, `BaseGraphTab`) | - | 0 | 0 | 0 | 0 |
| **UI Coord** (`MainWindow`, `SessionCtrl`) | 0 | - | 0 | 0 | 0 |
| **Sig.Proc** (`DSPWorker`, `MeasurementEngine`) | 1 | 1 | - | 0 | 0 |
| **Domain** (`Measurement`, `WatchMath`, `WatchDiagnostics`) | 1 | 1 | 1 | - | 0 |
| **Acquisition** (`IAudioSource`, `AudioWorker`, `AudioRingBuffer`) | 0 | 1 | 1 | 0 | - |

All 1s are in the lower triangle → no layer violations ✅. Domain column is all `0` → no upward dependency ✅.

## Modifiability Tactics (Bass/CMK Ch.8)

The 4-layer structure directly implements the **Restrict Dependencies** tactic from
Bass, Clements & Kazman *Software Architecture in Practice* (4th ed., Ch.8 §8.2 p.124):

> *"The restrict dependencies tactic is seen in layered architectures, in which a layer
> is allowed to use only lower layers … restricting a module's visibility (when developers
> cannot see an interface, they cannot employ it)."*

Consequences for change cost (Ch.8 p.128 Layers Pattern):

> *"Because a layer is constrained to use only lower layers, software in lower layers can
> be changed (as long as the interface does not change) without affecting the upper layers."*

In this system: any change to `WatchMath` or `MeasurementEngine` (Domain / Signal Processing)
is invisible to Presentation — 14 tabs require zero modification when lower-layer internals change.

**Change cost table** (response measure for modifiability scenarios):

| Change type | Files outside the changed unit | Reason |
|---|:---:|---|
| New graph tab | ≤ 3 | Only `NewTab.h/cpp` + `MainWindow.cpp` (registerTab) |
| New audio source | 1–2 | Only new class implementing `IAudioSource` |
| New watch formula | 1 | Only `WatchMath.cpp` + test case |
| New DSP filter | 1–2 | Only FilterChain internals; no Presentation change |

## Testability Connection (Bass/CMK Ch.12 §12.2 p.191)

The same structural properties that achieve modifiability also enable testability.
Ch.12 p.191 states:

> *"Ensuring that the system has high cohesion, loose coupling, and separation of concerns —
> all modifiability tactics (see Chapter 8) — can also help with testability."*

And specifically for the layered pattern (Ch.12 p.191):

> *"In a layered pattern, you can test lower layers first, then test higher layers with
> confidence in the lower layers."*

This is exactly how our test suite is structured:

| Test binary | Layer under test | Dependencies needed | Tests |
|---|---|---|:---:|
| `TestWatchMath` | Domain | None (pure math) | 44 |
| `TestDetector` | Signal Processing | Domain only | ~30 |
| `TestFilterChain` | Signal Processing | Domain only | ~20 |
| `TestAddedTabs` | Presentation | Domain structs (mock) | 20 |
| … (6 more) | Various | Layer-appropriate | ~28 |
| **Total** | | | **142** |

**The 142 unit tests across 10 independent binaries are structural evidence**:
they are only possible because the 4-layer architecture achieves the low coupling
required by the *Limit Structural Complexity* testability tactic (Ch.12 §12.2).
If any layer boundary were violated, transitive hardware or Qt dependencies would
prevent isolated compilation of a test binary.

## Related ADRs

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — introduces `DSPWorker` as a separate thread between Acquisition and Signal Processing
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard placed in Presentation layer `updateData()` implementations

## Related views

- [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) — refinement of the Presentation layer; shows internal structure of a single `BaseGraphTab`
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view of the Acquisition ↔ Signal Processing boundary (ring buffer + thread)
- [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) — AS-IS vs TO-BE comparison of the audio source extension point (QAS-3)
