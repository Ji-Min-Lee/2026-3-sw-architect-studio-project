# IAudioSource dependency inversion view

This view shows the AS-IS vs. TO-BE structure of the audio source extension point. It answers: "What must a developer do to add a new audio source (e.g., network stream, USB device)?" It is the primary evidence for QAS-3 (Extensibility / Modifiability).

In the AS-IS design, adding a new audio input source requires modifying `MainWindow` and the now-removed `AudioManager` coordinator — two components unrelated to the new source. In the TO-BE design (P1 + i1 refactors), the developer implements `IAudioSource` only; no other file changes are needed.

![IAudioSource Dependency Inversion](../../assets/view5-iaudiosource.png)

## Element Catalog

#### AS-IS (before P1 refactor)

| Element | Role |
|---------|------|
| `MainWindow` | Qt top-level window; directly wired to all concrete worker types |
| `AudioManager` | Dead coordinator; held 3 concrete worker pointers; removed in i6 refactor |
| `TAudioWorker` | Concrete ALSA live mic worker (T-prefixed legacy name) |
| `TPlaybackWorker` | Concrete WAV file playback worker |
| `TSimWorker` | Concrete synthetic signal worker |
| `DSPWorker` | DSP thread; had 3 separate `connect()` blocks (one per worker type) |

Adding a `NetworkWorker` required: (1) modifying `MainWindow`, (2) modifying `AudioManager`, (3) adding a 4th `connect()` block in `DSPWorker`.

#### TO-BE (after P1 + i1 refactors)

| Element | Role |
|---------|------|
| `IAudioSource` | Abstract Qt interface; signals: `dataReady(int64_t)`, `finished()`, `sourceComplete()` |
| `AudioWorker` | Implements `IAudioSource` — live mic (ALSA) |
| `PlaybackWorker` | Implements `IAudioSource` — WAV file playback |
| `SimWorker` | Implements `IAudioSource` — synthetic signal generator |
| `SessionController` | Owns `IAudioSource*` + thread lifecycle; depends on interface only; 1 `connect()` block |

Adding a `NetworkWorker` requires: implement `IAudioSource` in 1–2 files. Zero other changes.

## Behavior

**Extension procedure (TO-BE)**:

```
1. Create NetworkWorker : public IAudioSource
2. Implement dataReady() emission loop
3. Register in SessionController::startNetwork() (optional new method)

SessionController «use» IAudioSource
    → single connect(source, &IAudioSource::dataReady, …)
    → works for all current and future workers
```

**Trade-off**: `sourceComplete()` signal contract differs between source types — live mic never emits it; Playback/Sim emit at EOF. New source implementations must follow this contract; `SessionController` handles both cases uniformly.

## Related ADRs

- [ADR-005: IAudioSource Dependency Inversion](../adr/ADR-005-p1-iaudiosource-dependency-inversion.md) — introduces `IAudioSource` interface; eliminates `AudioManager`
- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — `DSPWorker` connects to `IAudioSource::dataReady` via `AudioRingBuffer`

## Related views

- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — shows `IAudioSource` and workers in the Acquisition layer context
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — runtime view; `SessionController` owns the T1 thread lifecycle shown here
