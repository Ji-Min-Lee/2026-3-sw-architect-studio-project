# IAudioSource Dependency Inversion View

This view shows the AS-IS vs. TO-BE structure of the audio source abstraction. It answers: "How does the architecture guarantee that all three input modes (live mic, WAV playback, synthetic signal) traverse the identical DSP path?" It is the primary evidence for **QAS-4 Sub-2 (Internal Consistency)**.

In the AS-IS design, each input mode had its own concrete pointer and its own `connect()` block in `MainWindow`. Mode-specific wiring meant the DSP chain could silently diverge between modes — a correctness risk. In the TO-BE design (P1 + i1 refactors), `SessionController` holds a single `IAudioSource*` and connects once; all three modes share the identical DSP entry path, enforced at compile time.

![IAudioSource Dependency Inversion](../../assets/view5-iaudiosource.png)

## Element Catalog

#### AS-IS (before P1 refactor)

| Element | Role |
|---------|------|
| `MainWindow` | Qt top-level window; held three concrete worker pointers; wired each mode separately |
| `AudioManager` | Dead coordinator; held 3 concrete worker pointers; removed in i6 refactor |
| `TAudioWorker` | Concrete ALSA live mic worker (T-prefixed legacy name) |
| `TPlaybackWorker` | Concrete WAV file playback worker |
| `TSimWorker` | Concrete synthetic signal worker |
| `DSPWorker` | DSP thread; had 3 separate `connect()` blocks (one per worker type) |

Each mode required its own `connect()` block — a divergence point where per-mode DSP wiring differences could arise undetected.

#### TO-BE (after P1 + i1 refactors)

| Element | Role |
|---------|------|
| `IAudioSource` | Abstract Qt interface; signals: `dataReady(int64_t)`, `finished()`, `sourceComplete()` |
| `AudioWorker` | Implements `IAudioSource` — live mic (ALSA) |
| `PlaybackWorker` | Implements `IAudioSource` — WAV file playback |
| `SimWorker` | Implements `IAudioSource` — synthetic signal generator |
| `SessionController` | Owns `IAudioSource*` + thread lifecycle; depends on interface only; 1 `connect()` block shared by all modes |

All three modes enter the DSP chain through the same single `connect()` site. The compiler enforces this: `SessionController` references only `IAudioSource`, making per-mode branching in the wiring path impossible.

## Behavior

**Consistency guarantee (TO-BE)**:

```
SessionController::startSourceThread()
    → connect(source, &IAudioSource::dataReady,
              mDspWorker, &DSPWorker::onDataReady, Qt::QueuedConnection)

AudioWorker   ─┐
PlaybackWorker ─┼─ same connect() site ─→ DSPWorker ─→ MeasurementEngine ─→ all 14 tabs
SimWorker     ─┘
```

All three input modes emit `dataReady(int64_t emitTimestampUs)` — the timestamp is used downstream for latency measurement (QAS-2). Because all modes share this single wiring, any measurement derived from audio data is produced by the same DSP computation regardless of which input mode is active.

**`sourceComplete()` contract**: live mic never emits it (no EOF); `PlaybackWorker` and `SimWorker` emit at end of data. `SessionController::onSourceComplete()` handles both cases uniformly — new implementations must follow this contract or session teardown will malfunction.

**Secondary benefit — future extensibility**: because `SessionController` depends only on `IAudioSource`, a future input mode (e.g., network stream) requires only implementing `IAudioSource` and adding one factory method in `SessionController`. No changes to `MainWindow`, `DSPWorker`, or `MeasurementEngine` are needed. This is a structural benefit, not a current requirement.

## Related QA

- **Primary**: [QAS-4 Sub-2 (Internal Consistency)](../qa/qas-4-correctness.md) — single `connect()` site ensures all input modes produce measurements via the identical DSP path; no mode-specific computation divergence possible
- **Secondary**: [QAS-3 (Extensibility, Modifiability)](../qa/qas-3-extensibility-modifiability.md) — interface boundary reduces future audio source addition cost to 1–2 files; not a current requirement

## Related ADRs

- [ADR-005: IAudioSource Dependency Inversion](../adr/ADR-005-p1-iaudiosource-dependency-inversion.md) — decision record for introducing `IAudioSource` and extracting `SessionController`
- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — `DSPWorker` connects to `IAudioSource::dataReady` via `AudioRingBuffer`; the single connect site makes the T2 wiring unambiguous across all modes

## Related Views

- [Layered and Module Decomposition View](view-layered-4layer.md) — shows `IAudioSource` and workers in the Acquisition layer context
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view; `SessionController` owns the T1 thread lifecycle shown here
