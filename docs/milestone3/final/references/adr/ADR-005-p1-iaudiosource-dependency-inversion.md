# ADR-005: IAudioSource Dependency Inversion (P1 Refactor)

The original design wired `MainWindow` directly to three concrete audio worker classes
(`TAudioWorker`, `TPlaybackWorker`, `TSimWorker`). Each mode required a separate concrete
pointer and a duplicated `connect()` block in `MainWindow`. Adding a fourth audio source
(e.g., network stream) required modifying `MainWindow` and repeating the wiring pattern —
a change unrelated to the new source's logic.

Additionally, `MainWindow` mixed UI lifecycle management with audio thread lifecycle
management, making both concerns harder to reason about independently.

## Decision

We introduce `IAudioSource` as an abstract Qt interface (`QObject` subclass) with three
signals that define the contract for all audio sources:

```cpp
class IAudioSource : public QObject {
    Q_OBJECT
signals:
    void dataReady(int64_t emitTimestampUs); // PCM block written to ring buffer
    void finished();                          // worker event loop should stop
    void sourceComplete();                   // source has no more data (EOF/end)
};
```

All concrete workers implement this interface:

| Class | Mode | `sourceComplete()` |
|-------|------|--------------------|
| `TAudioWorker` | Live mic (ALSA) | Never — live mic has no EOF |
| `TPlaybackWorker` | WAV file playback | Emitted at EOF |
| `TSimWorker` | Synthetic signal | Emitted at synthesis end |

Thread lifecycle and DSP wiring are extracted into `SessionController`, which holds
`IAudioSource *mActiveSource` — a single interface pointer — and wires the DSP thread
once via `startSourceThread()`:

```cpp
// SessionController::startSourceThread() — single connect block for all modes
connect(source, &IAudioSource::dataReady,
        mDspWorker, &DSPWorker::onDataReady, Qt::QueuedConnection);
```

Mode-specific setup (e.g., `StartAudioRecording`, `StartPlayback`) is dispatched via
`QMetaObject::invokeMethod` after the thread starts, keeping the shared wiring path
mode-agnostic.

## Rationale

**AS-IS (before P1)**: `MainWindow` held three concrete worker pointers and duplicated
the `connect()` block for each mode. Adding a `NetworkWorker` required:
1. Adding a new concrete pointer in `MainWindow`
2. Adding a fourth `connect()` block
3. Modifying session start logic in `MainWindow`

**TO-BE (after P1 + i1)**: `SessionController` holds `IAudioSource *mActiveSource`.
Adding a `NetworkWorker` requires:
1. Create `NetworkWorker : public IAudioSource` (1–2 files)
2. Add `startNetwork()` factory method in `SessionController` (≤ 10 lines)
3. Zero changes to `MainWindow`, `DSPWorker`, or `MeasurementEngine`

This satisfies QAS-3 (Extensibility / Modifiability): adding a new audio input source
affects only the Acquisition layer and does not propagate upward.

Rejected alternative — keep concrete pointers, use polymorphic dispatch via `qobject_cast`:
adds runtime type checks throughout the codebase; does not reduce the number of `connect()`
blocks; and ties `SessionController` to all concrete types anyway.

## Status

Accepted

Implemented as part of the P1 refactor series (i1: `SessionController` extraction,
i6: `AudioManager` dead-code removal).

## Consequences

**Positive**:
- `SessionController::startSourceThread()` written once; all three modes share identical
  thread wiring via the `IAudioSource` interface
- `MainWindow` reduced from ~949 to ~750 lines (i1 extraction)
- Adding a new audio source = implement `IAudioSource` + add one factory method;
  zero changes to the signal processing or presentation layers
- `sourceComplete()` contract is explicit: live mic never emits it; Playback/Sim emit at
  EOF. `SessionController::onSourceComplete()` handles both uniformly

**Negative**:
- `sourceComplete()` contract differs between source types — new implementations must
  follow the contract or session teardown will malfunction (live mic: omit; finite sources:
  emit at natural end)
- Live-only controls (`StopAudioRecording`, `SetAudioInputVolume`) still require direct
  access to the concrete `TAudioWorker` type; these are wired per-session in `startLive()`
  before the abstract pointer is stored, which is a deliberate exception to the interface rule

## Related views

- [Module View: IAudioSource Dependency Inversion](../views/view-iaudiosource.md) — AS-IS
  vs TO-BE structural comparison
- [Layered and Module Decomposition View](../views/view-layered-4layer.md) — `IAudioSource`
  and workers in the Acquisition layer context
- [C&C View: DSP Pipeline Thread Model](../views/view-cc-dsp-pipeline.md) — runtime view;
  `SessionController` owns the T1 thread lifecycle shown here
