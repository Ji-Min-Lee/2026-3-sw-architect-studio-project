# Performance Logging Facility

A reusable, compile-time-gated logging facility for measuring the TimeGrapher
audio pipeline (latency, per-section timing, throughput). Designed so that
**release builds carry zero overhead** while measurement builds produce both a
live console summary and a CSV file for offline analysis.

---

## Goals

- Measure real performance on the **optimized (Release)** build — Debug numbers
  are unrepresentative.
- Add **zero overhead** when logging is disabled (production/release default).
- Keep instrumentation **out of the hot code** — a dedicated `Logger` class,
  not scattered `qInfo` calls.
- Output both a **live console summary** and a **CSV** for offline plotting.

---

## Compile-time gate: `ENABLE_LOGGING`

Logging is controlled by the CMake option `ENABLE_LOGGING` (default `OFF`),
which is independent of `CMAKE_BUILD_TYPE`. This separates two orthogonal axes:

| Axis | Values | Controls |
|------|--------|----------|
| `CMAKE_BUILD_TYPE` | Debug / Release | optimization |
| `ENABLE_LOGGING`   | OFF / ON        | instrumentation |

Three meaningful configurations:

| Configuration | Optimized | Logging | Use |
|---------------|-----------|---------|-----|
| Release, `ENABLE_LOGGING=OFF` | yes | no  | production / shipping (no overhead) |
| Release, `ENABLE_LOGGING=ON`  | yes | yes | **performance measurement** |
| Debug                         | no  | (n/a) | code debugging only |

> Measuring on a Debug build gives misleading timings (no `-O2`). Always
> measure with `Release + ENABLE_LOGGING=ON`.

### Zero-overhead mechanism

The probe macro folds away when logging is off:

```cpp
#ifdef ENABLE_LOGGING
  #define TG_NOW() nowUs()        // real monotonic clock read
#else
  #define TG_NOW() ((int64_t)0)   // compile-time constant
#endif
```

With `ENABLE_LOGGING` off:
- `ts = TG_NOW();` becomes `ts = 0;`
- `bd.copy_us += TG_NOW() - ts;` becomes `bd.copy_us += 0 - 0;`

Both fold away under optimization. The `Logger` object is never constructed
(guarded by `#ifdef`), and workers emit `0` as the timestamp instead of reading
the clock. Net runtime cost: **zero**.

When `ENABLE_LOGGING` is on, CMake also switches `WIN32_EXECUTABLE` to `FALSE`
so the console subsystem is available to show `qInfo` output; otherwise the
normal GUI subsystem is used (no console window).

---

## Components

```
src/
├── Logger.h / Logger.cpp     Logging facility
│     - nowUs()               monotonic us clock (shared by workers + main)
│     - TG_NOW()              probe macro (clock when on, 0 when off)
│     - class Logger          accumulate -> average -> console + CSV
│     - Logger::Frame         one frame's measurements
│
├── SharedAudio.h             keeps <cstdint> for int64_t signal signatures
├── AudioWorker / Playback / Sim
│     - signal XxxDataReady(int64_t emitTimestampUs)
│     - emit XxxDataReady(TG_NOW())     T0 = emit time (0 when off)
│
└── MainWindow
      - HandleInputData(ptr, emitTimestampUs)
          wait = TG_NOW() - emitTimestampUs        (queue + scheduling)
          ProcessSamples(ptr, frame)               (fills section timings)
          exec = TG_NOW() - fgStart                (processing time)
          #ifdef ENABLE_LOGGING  mLogger->record(frame)  #endif
      - ProcessSamples(ptr, frame)
          times copy / sound / tg / ui / plot via TG_NOW()
```

---

## Data flow

```
BG worker        emit XxxDataReady(TG_NOW())   ── T0
   │
Qt event queue
   │
FG HandleInputData(ptr, T0)
   ├─ wait = TG_NOW() - T0
   ├─ ProcessSamples(ptr, frame)   fills copy/sound/tg/ui/plot
   ├─ exec = TG_NOW() - fgStart
   ├─ frame += bg/fg fps,sps,spf
   └─ mLogger->record(frame)
                     │  every 100 frames
                     ▼
            average window -> console (ms) + CSV (ms)
```

---

## Measured metrics (per frame)

| Field | Meaning |
|-------|---------|
| `samples` | samples processed this frame (backlog indicator; 480 = 1 frame @ 48 kHz) |
| `wait` | BG emit → FG handler start (queue wait + OS scheduling) |
| `exec` | FG handler start → end (processing time) |
| `copy` | ring buffer → input block memcpy |
| `sound` | SoundImageRenderer.processSamples() |
| `tg` | tg_process() beat detection |
| `ui` | ScopePlot addData + A/C event markers |
| `plot` | PurgeHistory + replot + DrawImage |
| `bg_fps/sps/spf` | background (worker) throughput |
| `fg_fps/sps/spf` | foreground (handler) throughput |

`total = wait + exec`.

---

## Units

- **Measured** in microseconds (us) internally for precision.
- **Reported** in milliseconds (ms) in both the console summary and the CSV.

---

## Output

### Console (every 100 valid frames)

```
[000100] avg_samples=492.3  BG: fps=100.0 sps=48000.0 spf=480.0  FG: fps=99.8 sps=47900.0 spf=480.0
[000100] total=2.15ms [wait=0.18 + exec=1.97]  exec=[copy=0.004 sound=0.001 tg=0.390 ui=0.013 plot=1.450] ms
```

### CSV: `src/logs/log_YYYYMMDD_HHmmss.csv`

Written to `src/logs/` deterministically — the path is resolved from the
executable location (`applicationDirPath()/../logs`), so it is independent of
the current working directory. The folder is created automatically if missing.

One row per 100-frame window. Columns (durations in ms):

```
frame, avg_samples,
avg_total_ms, avg_wait_ms, avg_exec_ms,
avg_copy_ms, avg_sound_ms, avg_tg_ms, avg_ui_ms, avg_plot_ms,
bg_fps, bg_sps, bg_spf, fg_fps, fg_sps, fg_spf
```

CSV logs are committed to git for experiment history (this is a short,
4-week project, so the volume stays small).

---

## Build & run

```powershell
# Windows (Release, default unified build dir)
.\src\tools\run_timegrapher.ps1 build            # ENABLE_LOGGING=OFF -> src/build
.\src\tools\run_timegrapher.ps1 build -Logging   # ENABLE_LOGGING=ON  -> src/build-log
```

Logging and non-logging binaries use separate build directories (`build` vs
`build-log`) so their CMake caches never thrash.

---

## Notes

- CSV save location is deterministic: `src/logs/` (resolved from the executable
  path, independent of cwd).
- Logging window size is fixed at 100 frames (≈ once per second at 100 fps).
