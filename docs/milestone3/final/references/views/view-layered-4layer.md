# TimeGrapher Layered View

This view answers one question: **why can you add a new graph tab by touching ≤ 3 files?** The four-layer structure enforces that Presentation may only depend on Domain — so a new tab implements `BaseGraphTab`, reads the `Measurement` VO, and registers in `MainWindow`. Nothing below Domain is touched. It is the primary evidence for [QAS-3 (Extensibility / Modifiability)](../qa/qas-3-extensibility-modifiability.md).

![TimeGrapher Layered View](../../assets/view1-layered-4layer.png)

## Element Catalog

#### Acquisition Layer
- Contains `IAudioSource` (abstract Qt interface), `TAudioWorker` (live mic, ALSA), `TPlaybackWorker` (WAV file), `TSimWorker` (synthetic signal), and `AudioRingBuffer` (lock-free SPSC ring buffer).
- `IAudioSource` unifies all three input modes under a single `connect()` site in `SessionController`, ensuring identical DSP entry path for live mic, WAV playback, and synthetic signal: see [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) (QAS-4 Sub-2).

#### Signal Processing Layer
- Contains `DSPWorker` [ADR-001], `MeasurementEngine`, and `«external» Timegrapher` (C library in `src/external/`, wraps `tg_process`/`tg_context`).
- `DSPWorker` runs on a dedicated thread (T2); reads PCM blocks from `AudioRingBuffer` via lock-free read.
- `DSPWorker` directly owns `MeasurementEngine` as a member (`DSPWorker.h:37`) and calls `mEngine->processBlock()` within the T2 DSP loop — they form a single T2 pipeline unit.
- `MeasurementEngine` emits a `Measurement` struct via Qt Signal-Slot to Presentation (cross-thread, `QueuedConnection`).
- **Layer boundary note**: `MeasurementEngine` is classified here (not Domain) because it is owned and driven by `DSPWorker` within the T2 DSP loop. Pure domain objects (`Measurement` VO, `WatchMath`, `WatchDiagnostics`) remain in the Domain layer and have no upward dependencies.

#### Domain Layer
- Contains pure computation and value objects only: `WatchMath`, `WatchDiagnostics` (rule-based diagnosis), `WatchExplainer` (Ollama LLM, on-device), and VOs `Measurement`, `SignalFrame`, `WatchMetrics`, `AcousticEvent`, `MovementSpec`.
- No thread ownership, no Qt signal emission, no display logic.
- `Measurement` is the key VO — the only type a new graph tab needs to read.

#### Presentation Layer
- Contains `MainWindow` (tab host, owns `registerTab<T>()`) and all 14 `BaseGraphTab` implementations.
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
Presentation    →  Domain (reads Measurement VO via Qt signal from MeasurementEngine)
    ↑ forbidden (Presentation cannot reference Signal Processing or Acquisition)
Domain          (no external layer dependencies — pure computation and VOs)
    ↑ forbidden
Signal Processing  →  Acquisition (reads AudioRingBuffer)
    ↑ forbidden
Acquisition     (bottom — no dependencies on layers above)
```

This rule matters because it constrains change impact. The clearest example is graph-tab addition.

**Adding a new graph tab** (≤ 3 file change rule, codified in [ADR-006](../adr/ADR-006-basegraphtab-observer-pattern.md)):

| Step | File | Count |
|------|------|:-----:|
| Implement `BaseGraphTab` subclass | `FooTab.h` + `FooTab.cpp` | 1–2 |
| Register in `MainWindow` | `MainWindow.cpp` | 1 |
| **Total** | | **≤ 3** |

Zero changes to Domain, Signal Processing, or Acquisition layers are required. This is verified by [EXP-03](../experiments/exp-03-extensibility-observer-pattern.md): all 14 existing tabs were implemented ≤ 3 files each, with zero Presentation→Signal Processing or Presentation→Acquisition references.

The layer rule is enforced at compile time — a boundary violation causes a compiler error due to per-layer include restrictions. It is also verified by DSM (Dependency Structure Matrix): all 14 tabs show 0 layer violations.

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
- [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) — defines the `BaseGraphTab` observer contract and `MainWindow::registerTab<T>()` as the single registration point; codifies the ≤ 3-file change rule ([EXP-03](../experiments/exp-03-extensibility-observer-pattern.md) verifies compliance for all 14 tabs)

## Related views

- [Graph Tab Module Uses View](view-decomposition-graph-tab.md) — refinement of the Presentation layer; shows internal structure of a single `BaseGraphTab`
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view of the Acquisition ↔ Signal Processing boundary (ring buffer + thread)
- [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) — AS-IS vs TO-BE comparison showing how all three input modes share an identical DSP wiring path (QAS-4 Sub-2)
