# EXP-02: E2E Audio Pipeline Latency Baseline Measurement

---

## Results and Recommendations

Measured the E2E latency baseline of the feature/layer codebase on macOS (96kHz Playback mode).
Average exec_ms of 0.57ms confirms very fast DSP processing with 0% deadline miss rate.
The dominant delay (avg wait_ms = 420ms) originates from Qt queue latency between the audio thread and the main thread.

Two tactics were applied to resolve the main performance issues:

- **T2 (DSP Offload Thread)**: wait_ms 420ms → 0.013ms (×32,000 reduction), backlog fully eliminated (47% → 0%)
- **R1 (Lazy Rendering)**: replot_count 8.22 → 2.08/beat (75% reduction, Scenario A), 1.20/beat (85% reduction, Scenario B)

**Recommendation**: Adopt T2+R1 as the baseline tactic for feature/layer. Re-measure on RPi under identical conditions (R5).

---

## Objective

Measure the E2E audio processing pipeline latency distribution on the feature/layer codebase to answer:

1. What is the actual DSP processing time (exec_ms)?
2. Is tab rendering separated from the exec path via Qt QueuedConnection?
3. How does it differ from the RPi baseline (baseline/experiments2)?
4. Can the effects of T2 (DSP thread offload) and R1 (Lazy Rendering) be quantified?

---

## Status

**Concluded** (2026-06-15)

---

## Expected Outcomes

- Per-frame latency distribution CSV logs (wait_ms, exec_ms, tg_ms, plot_ms, replot_count)
- Comparison table: macOS baseline vs T2 vs T2+R1
- Cross-comparison with RPi baseline (baseline/experiments2)
- ADD design decision records (T2: ADD-2-01, R1: ADD-2-02)

---

## Resources Required

| Item | Detail |
|------|--------|
| Platform | macOS (Apple Silicon) |
| Branch | `feature/layer-ex-baseline`, `exp/r1-replot-baseline` |
| Source | `src/logging/Logger.h/cpp` (per-frame CSV logging infrastructure) |
| Analysis tool | `src/tools/analyze_log.py` |
| WAV file | `28800BPH_3235_Starbucks.wav` (96kHz, BPH 28800) |
| Build flag | `ENABLE_LOGGING=ON` |

---

## Experiment Description

Implemented per-frame CSV logging infrastructure in `src/logging/Logger.h/cpp` (ported from `baseline/experiments2`). Timestamp model:

- **TS1**: `emit PlaybackDataReady(TG_NOW())` inside `PlaybackWorker::StartPlayback()`
- **TS2**: `TG_NOW()` at entry of `MainWindow::HandleInputData()`
- `wait_us` = TS2 − TS1 (Qt queue wait + scheduling jitter)
- `exec_us` = HandleInputData exit − TS2 (actual processing time)
- `tg_us` = elapsed inside `MeasurementEngine::processBlock()`
- `plot_us`, `ui_us` = 0 (separated asynchronously via QueuedConnection)

The experiment ran across 5 sequential runs:

| Run | Tactic | Scenario |
|:---:|:------:|:--------:|
| R1 | Baseline (main thread DSP) | baseline measurement |
| R2 | T2 DSP Offload Thread | separate DSPWorker thread |
| R2b | T2 + no replot guard | establish replot_count baseline |
| R3 | T2 + R1 Lazy Rendering · Scenario A | single tab fixed |
| R4 | T2 + R1 Lazy Rendering · Scenario B | repeated tab switching |

---

## Results

### Run History

| Run | Date | Tactic | Platform | Frames | wait avg | exec avg | deadline miss | backlog | replot_count avg |
|:---:|:----:|:------:|:--------:|:------:|:--------:|:--------:|:-------------:|:-------:|:----------------:|
| R1 | 2026-06-14 | Baseline | macOS 96kHz | 2,900 | 419.92 ms | 0.57 ms | 0% | 47% | — |
| R2 | 2026-06-14 | T2 DSP Offload | macOS 96kHz | 4,500 | **0.013 ms** | 0.40 ms | 0% | **0%** | — |
| R2b | 2026-06-15 | T2 + no guard | macOS 96kHz | 4,501 | 0.013 ms | ~0.40 ms | 0% | 0% | **8.22** |
| R3 | 2026-06-15 | T2 + R1 · Scenario A | macOS 96kHz | 4,501 | 0.013 ms | 0.395 ms | 0% | 0% | **2.08** |
| R4 | 2026-06-15 | T2 + R1 · Scenario B | macOS 96kHz | 4,501 | 0.013 ms | 0.395 ms | 0% | 0% | **1.20** |

### Latency Distribution — R1 Baseline

| Metric | Avg | Max | Min |
|--------|:---:|:---:|:---:|
| total_ms | 420.49 ms | 927.50 ms | 0.22 ms |
| wait_ms | 419.92 ms | 926.82 ms | 0.01 ms |
| exec_ms | 0.57 ms | 1.96 ms | 0.08 ms |
| copy_ms | 0.008 ms | 0.025 ms | — |
| tg_ms (DSP) | 0.560 ms | 1.934 ms | — |
| ui_ms / plot_ms | 0 ms | 0 ms | — |

### T2 DSP Offload Thread — R2

| Metric | Avg | Max |
|--------|:---:|:---:|
| wait_ms | **0.013 ms** | 0.087 ms |
| exec_ms | **0.400 ms** | 5.993 ms |
| tg_ms (DSP) | 0.393 ms | 5.982 ms |
| deadline miss | **0%** | — |
| backlog | **0%** | — |
| bg_fps ≈ fg_fps | **95.6 ≈ 95.6** | — |

### R1 Lazy Rendering — R3 / R4

| Metric | R3 Scenario A (1 tab) | R4 Scenario B (tab switch) |
|--------|:---------------------:|:--------------------------:|
| exec_ms avg | 0.395 ms | 0.395 ms |
| deadline miss | 0% | 0% |
| backlog | 0% | 0% |
| **replot_count avg** | **2.08 (↓75%)** | **1.20 (↓85%)** |

### Comparison with RPi Baseline

| Metric | macOS R1 (Baseline) | macOS R2 (T2) | macOS R3 (T2+R1) | RPi baseline/experiments2 |
|--------|:-------------------:|:-------------:|:-----------------:|:-------------------------:|
| wait_ms avg | 420 ms | **0.013 ms** | 0.013 ms | N/A |
| exec_ms avg | 0.57 ms | 0.40 ms | 0.395 ms | 20 ms |
| tg_ms avg | 0.56 ms | 0.39 ms | 0.389 ms | ~4 ms |
| plot_ms avg | 0 ms | 0 ms | 0 ms | 16 ms (79%) |
| deadline miss | 0% | **0%** | 0% | **43%** |
| backlog | 47% | **0%** | 0% | — |
| replot_count avg | — | — | **2.08** | — |

Key observations:

1. **plot_ms = 0**: Tab rendering fully removed from exec path via QueuedConnection — design intent of feature/layer confirmed.
2. **wait_ms dominates**: 420ms Qt queue wait on macOS is a scheduling artifact (batch processing). Expected to differ on RPi.
3. **T2 resolves backlog**: DSPWorker thread eliminates the event-loop bottleneck; wait_ms drops ×32,000.
4. **R1 reduces render load**: 75–85% replot reduction on macOS. On RPi (where plot_ms is in the exec path), this directly cuts ~14ms/beat.

---

## Architecture Decisions

| ID | Tactic | Decision | Outcome |
|:--:|:------:|----------|---------|
| ADD-2-01 | T2 DSP Offload Thread | Separate DSPWorker onto a dedicated thread (Introduce Concurrency) | wait_ms ×32,000 reduction, backlog 0% |
| ADD-2-02 | R1 Lazy Rendering | isVisible() guard + showEvent catch-up (Reduce Computational Overhead) | 75–85% replot reduction |

---

---

## RPi Runs (E2-3 through E2-7)

All runs on rpi2 (RPi 5 2nd unit, 16 GB, no thermal throttling at ~60 °C). Platform: debian, kernel=linux, host=lg1.

| Run | Date | Config | E2E avg/max (ms) | exec > DL | Backlog | Real-time |
|:---:|------|--------|:----------------:|:---------:|:-------:|:---------:|
| E2-3 | 2026-06-15 | rpi2 baseline (no T2/R1) | 57.2 / 208.9 | 4.4 % | 21 % | FAIL |
| E2-4 | 2026-06-15 | baseline + multi-graph | 80.1 / 258.7 | 0 % | 28 % | exec OK, wait high |
| E2-5 | 2026-06-15 | E2-4 + **T2** DSP Offload | **2.1 / 11.1** | **0 %** | **0 %** | ✅ ideal |
| E2-6 | 2026-06-15 | E2-5 + **R1** Lazy Rendering | **2.05 / 5.7** | **0 %** | **0 %** | ✅ ideal, tighter max |
| E2-7 | 2026-06-16 | E2-6 + fg_wait_ms instrumentation | DSP 2.2 / 4.8 | **0 %** | **0 %** | DSP ✅ / FG ⚠️ |

### E2-7 New Finding — FG Scheduling Bottleneck

E2-7 added `fg_wait_ms` measurement: time from DSPWorker `frameLogged` emit to MainWindow `onFrameLogged` entry (Qt FG-scheduler pickup latency).

| fg_wait metric | Value |
|----------------|------:|
| avg | **60.1 ms** 🔴 |
| p95 | 144.0 ms |
| p99 | 167.8 ms |
| max | 183.6 ms |
| > deadline (21.33 ms) | **1231 / 1458 (84 %)** 🔴 |

The DSP pipeline is healthy (E2E avg 2.2 ms, 0 deadline misses). The Qt FG event loop on RPi is ~7× slower to wake the FG thread than on macOS (macOS avg 8.9 ms, p99 20.5 ms). This is the next architecture concern.

**Root cause**: Qt event-loop scheduling priority on RPi, not CPU load (cpu0/2/3 are near-idle; only DSP core cpu1 at 95 %).

**Next action**: T1 (SCHED_RR + CPU affinity for FG thread) or `QTimer`-based periodic FG polling as an alternative to `frameLogged` signal delivery.

Full per-run detail: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-02)

---

## Duration

- Started: 2026-06-14
- macOS runs concluded: 2026-06-15 (R1–R4)
- RPi runs concluded: 2026-06-16 (E2-3 through E2-7)

---

## Links and References

- Full results: `docs/milestone2/exp-02-baseline-results.md`
- Tactic analysis (ADD design decisions): `docs/milestone2/tactic-analysis.md`
- Architecture tactic options: `docs/milestone2/architectural-approaches.md`
- Log R1 (Baseline): `logs/EXP-02/log_20260614_223711.csv`
- Log R2 (T2): `logs/EXP-02/log_20260614_231224.csv`
- Log R2b (replot baseline): `logs/EXP-02/log_20260615_050649.csv`
- Log R3 (T2+R1 Scenario A): `logs/EXP-02/log_20260615_003136.csv`
- Log R4 (T2+R1 Scenario B): `logs/EXP-02/log_20260615_003429.csv`
- Analysis tool: `src/tools/analyze_log.py`
- RPi baseline branch: `baseline/experiments2`
