# P1: Introduction of IAudioSource Interface

## Overview

This change inverts the dependency structure in which `MainWindow` (and `AudioManager`) directly
depended on three concrete worker classes, by introducing the `IAudioSource` interface.

---

## AS-IS

![AS-IS Module Uses View](assets/module-view-p1-asis.png)

`MainWindow` directly `«use»`s three concrete types: `TAudioWorker`, `TPlaybackWorker`, and `TSimWorker`.
Each worker emits signals under different names (`AudioDataReady`, `PlaybackDataReady`, `SimDataReady`, etc.),
requiring `MainWindow`/`AudioManager` to duplicate a separate `connect()` block for each worker.

**Problems**

- Adding a new audio source (e.g., a network stream) requires modifying `MainWindow`/`AudioManager` code.
- Different signal names per worker cause three copies of the `connect()` logic.
- EOF/completion handlers (`HandlePlaybackDoneReadingFile`, `HandleSimDone`) are duplicated despite identical logic.

---

## TO-BE

![TO-BE Module Uses View](assets/module-view-p1-tobe.png)

The `IAudioSource` interface is introduced and all three workers realize it.
`MainWindow`/`AudioManager` now `«use»` only a single `IAudioSource*`,
and `DSPWorker` receives data via the unified `IAudioSource::dataReady` signal.

**Rationale**

| Principle | Application |
|-----------|-------------|
| Dependency Inversion | `MainWindow` depends on the abstraction (`IAudioSource`), not on concrete types |
| Open/Closed | A new audio source only needs to implement `IAudioSource`; `MainWindow` requires no modification |
| DRY | Signal names are unified to `dataReady` / `sourceComplete`, reducing `connect()` blocks to one |

**Key Code Changes**

- Added `IAudioSource.h` / `IAudioSource.cpp` — `Q_OBJECT`-based abstract interface
- Changed the constructor initializer of `TAudioWorker`, `TPlaybackWorker`, `TSimWorker` from `QObject(parent)` to `IAudioSource(parent)`
- Unified all `connect()` calls in `AudioManager.cpp` to use `IAudioSource::dataReady` and `IAudioSource::sourceComplete`
- Added `IAudioSource.cpp` to `CMakeLists.txt` to prevent Qt MOC link errors
