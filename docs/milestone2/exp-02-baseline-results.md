# EXP-02: E2E Audio Pipeline Latency Baseline Measurement

## 1. Results Summary

Measured the E2E latency baseline of the feature/layer codebase on macOS (Playback mode, 96kHz).
Average exec_ms of 0.57ms confirms very fast processing; deadline miss rate is 0%.
The dominant delay (avg wait_ms = 420ms) originates from Qt queue latency between the audio thread and the main thread.
Compared to the RPi baseline (EXP-02, baseline/experiments2), the QueuedConnection separation in feature/layer confirms that the 16ms plot burden has been removed from the exec path.

After applying T2 (DSP Offload Thread), wait_ms dropped dramatically from 420ms to 0.013ms, and backlog was fully eliminated (47% → 0%).
The DSP thread was confirmed to track the worker thread at identical FPS (95.6), validating real-time processing.

---

## 2. Objective

Measure the latency distribution of the E2E audio processing pipeline on the feature/layer codebase to answer the following questions.

- What is the actual processing time (exec_ms)?
- Is tab rendering separated from the exec path via Qt QueuedConnection?
- How does it differ from the RPi baseline (EXP-02, baseline/experiments2)?
- Establish a baseline reference point for subsequent tactic application: T1 (thread policy), T2 (DSP offload), R1/R2 (rendering strategy).

---

## 3. Status

Concluded (2026-06-14)

---

## 4. Environment

| Item | Value |
|------|-------|
| Platform | macOS (Apple Silicon) |
| Mode | Playback |
| WAV File | `28800BPH_3235_Starbucks.wav` |
| Sample Rate | 96,000 Hz |
| BPH | 28,800 |
| Branch | `feature/layer-ex-baseline` |
| Build Flag | `ENABLE_LOGGING=ON` |
| Total Frames | 2,900 |

---

## 5. Measurement Method

Implemented per-frame CSV logging infrastructure in `src/logging/Logger.h/cpp` (ported from `baseline/experiments2`).
Timestamp model:

- **TS1**: `emit PlaybackDataReady(TG_NOW())` inside `PlaybackWorker::StartPlayback()`
- **TS2**: `TG_NOW()` at entry of `MainWindow::HandleInputData()`
- `wait_us` = TS2 − TS1 (Qt queue wait + scheduling jitter)
- `exec_us` = HandleInputData exit − TS2 (actual processing time)
- `tg_us` = elapsed inside `MeasurementEngine::processBlock()`
- `copy_us` = ring-buffer copy elapsed
- `ui_us`, `plot_us` = 0 (separated asynchronously via QueuedConnection)

Log is flushed to CSV every 100 frames (data preserved even on force-quit).

---

## 6. Results

### 6.0 Run History

| Run | Date | Branch | Tactic | Platform | Frames | wait avg | exec avg | deadline miss | backlog | replot_count avg |
|:---:|------|--------|:------:|----------|:------:|:--------:|:--------:|:-------------:|:-------:|:----------------:|
| R1 | 2026-06-14 | feature/layer-ex-baseline | Baseline (main thread DSP) | macOS 96kHz Playback | 2,900 | 419.92 ms | 0.57 ms | 0% | 47% | — |
| R2 | 2026-06-14 | feature/layer-ex-baseline | **T2 DSP Offload Thread** | macOS 96kHz Playback | 4,500 | **0.013 ms** | **0.40 ms** | 0% | **0%** | — |
| R2b | 2026-06-15 | exp/r1-replot-baseline | T2 + **replot counter (no guard)** · Scenario A | macOS 96kHz Playback | 4,501 | 0.013 ms | ~0.40 ms | 0% | 0% | **8.22** |
| R3 | 2026-06-15 | feature/layer-ex-baseline | **T2 + R1 Lazy Rendering** · Scenario A (1 tab fixed) | macOS 96kHz Playback | 4,501 | 0.013 ms | 0.395 ms | 0% | 0% | **2.08** |
| R4 | 2026-06-15 | feature/layer-ex-baseline | **T2 + R1 Lazy Rendering** · Scenario B (tab switching) | macOS 96kHz Playback | 4,501 | 0.013 ms | 0.395 ms | 0% | 0% | **1.20** |
| R5 | 2026-06-16 | feature/measure_exe_time | T2 + R1 + **per-thread timing** (new fg_wait_ms) | macOS 96kHz **Sim** | 4,501 | 0.015 ms | 0.107 ms | 0% | 0% | **1.08** |

### 6.1 Latency Distribution (R1 Baseline)

| Metric | Avg | Max | Min |
|--------|-----|-----|-----|
| total_ms | 420.49 ms | 927.50 ms | 0.22 ms |
| wait_ms | 419.92 ms | 926.82 ms | 0.01 ms |
| exec_ms | 0.57 ms | 1.96 ms | 0.08 ms |

### 6.2 exec Breakdown (R1 Baseline)

| Metric | Avg | Max | Share |
|--------|-----|-----|-------|
| copy_ms | 0.008 ms | 0.025 ms | 1.4% |
| tg_ms (DSP) | 0.560 ms | 1.934 ms | 98.2% |
| ui_ms | 0.000 ms | 0.000 ms | 0% |
| plot_ms | 0.000 ms | 0.000 ms | 0% |

### 6.3 Throughput (R1 Baseline)

| Metric | Value |
|--------|-------|
| avg samples/frame | 1,468.8 |
| deadline (BG period) | 10.01 ms |
| deadline miss (exec > deadline) | **0 / 2,900 (0%)** |
| backlog (>1.5× SPF) | 1,374 / 2,900 (47%) |
| bg_fps avg | 95.5 |
| fg_fps avg | 62.3 |

---

### 6.4 T2 DSP Offload Thread Results (R2)

Measurement after applying T2 tactic (DSPWorker separate thread), macOS 96kHz Playback mode.
Log file: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_231224.csv`

| Metric | Avg | Max | Min |
|--------|:---:|:---:|:---:|
| total_ms | 0.413 ms | 6.030 ms | 0.099 ms |
| wait_ms | **0.013 ms** | 0.087 ms | 0.006 ms |
| exec_ms | **0.400 ms** | 5.993 ms | 0.084 ms |
| copy_ms | 0.006 ms | 0.025 ms | 0.005 ms |
| tg_ms (DSP) | **0.393 ms** | 5.982 ms | 0.079 ms |
| ui_ms | 0 ms | — | — |
| plot_ms | 0 ms | — | — |

| Throughput | Value |
|------------|-------|
| Total frames | 4,500 |
| bg_fps avg | 95.6 |
| fg_fps avg (DSP thread) | **95.6** |
| deadline miss | **0 / 4,500 (0%)** |
| backlog (>1.5× SPF) | **0 / 4,500 (0%)** |

Key observations:

1. **wait_ms 420ms → 0.013ms (×32,000 reduction)**: Main thread Qt event loop bottleneck fully eliminated by DSP thread separation.
2. **backlog 47% → 0%**: Frame accumulation completely eliminated — DSP thread tracks worker in real time.
3. **bg_fps ≈ fg_fps (95.6 ≈ 95.6)**: Worker and DSP thread processing speed confirmed identical.
4. **exec_ms 0.57 → 0.40ms**: Actual DSP processing time in dedicated thread (main thread contention removed).
5. **tg_ms max 5.98ms spike**: Single OS scheduling jitter in DSP thread. Avg 0.39ms is stable.

### 6.4.1 T2 Thread Architecture — Before / After

**T2 적용 전후 스레드 아키텍처 비교**

T2(DSP Offload Thread) 적용 전에는 AudioWorker가 emit한 시그널이 **Qt 메인 이벤트 루프 큐**에 적재되었다.
메인 스레드는 UI 이벤트·탭 렌더링 등과 CPU를 공유하므로 큐에 오디오 시그널이 쌓이다가 한꺼번에 처리되는 backlog가 발생했다 (wait avg 420 ms, backlog 47%).

T2 적용 후에는 DSPWorker가 **전용 스레드**에서 구동되므로 BG 시그널을 곧바로 수신한다 (wait avg 0.013 ms).
단, DSPWorker가 `frameLogged`를 emit하면 다시 Qt 메인 이벤트 루프를 통해 FG 스레드에 전달되므로, **fg_wait** 이라는 새로운 스케줄링 지연이 발생한다 (avg 8.9 ms, R5에서 측정).

**T2 Before / After Thread Architecture Comparison**

Before T2, the signal emitted by AudioWorker was posted to the **Qt main event-loop queue**.
Because the main thread shares CPU with UI events and tab rendering, audio signals accumulated and were processed in batches, causing backlog (wait avg 420 ms, backlog 47%).

After T2, DSPWorker runs in a **dedicated thread** and picks up BG signals almost immediately (wait avg 0.013 ms).
However, when DSPWorker emits `frameLogged`, the signal is delivered to the FG thread via the Qt main event loop again, introducing a new scheduling delay called **fg_wait** (avg 8.9 ms, measured in R5).

Interactive diagrams:
- [t2-thread-architecture-comparison.html](t2-thread-architecture-comparison.html) — Before / After thread architecture (component view)
- [t2-gantt-sequential-vs-parallel.html](t2-gantt-sequential-vs-parallel.html) — Sequential vs Parallel execution (Gantt timeline)

#### 구간별 변화 / Segment-by-Segment Changes

| 구간 / Segment | Before (R1 Baseline) | After (T2 R2) | 변화 원인 / Reason |
|---|---|---|---|
| BG → 처리 시작 `wait_ms` | avg **420 ms** 🔴 (Qt 메인 큐 경합) | avg **0.013 ms** ✅ | DSPWorker 전용 이벤트 루프 — UI 이벤트와 경합 없음 |
| DSP 처리 `exec_ms` | avg 0.57 ms (메인 스레드, UI 블로킹) | avg 0.40 ms (DSP 전용 스레드) | 메인 스레드 경합 제거, DSP 전용 CPU 시간 확보 |
| 탭 렌더링 `plot_ms` | ≈ 16 ms (RPi, 동기 호출) · exec path 포함 | 0 ms (exec path 제거) | Qt::QueuedConnection으로 FG 이벤트 루프에 위임 |
| DSP → FG 통보 `fg_wait_ms` | — (동일 스레드, 지연 없음) | avg **8.9 ms** · p99 20.5 ms 🟡 | T2 신규 비용 — Qt 스케줄러가 FG 스레드를 깨우는 지연 |
| 프레임 backlog | **47%** 🔴 | **0%** ✅ | DSP 스레드가 BG 시그널을 즉시 처리하여 적재 없음 |

---

### 6.5 R1 Lazy Rendering Results (R3 / R4)

Measurement after applying R1 tactic (isVisible() guard + showEvent catch-up), macOS 96kHz Playback mode.

- **Scenario A** (single tab fixed): stay on Trace tab, no tab switch
- **Scenario B** (tab switching): sequentially switch tabs during run

| Metric | R3 Scenario A | R4 Scenario B |
|--------|:-------------:|:-------------:|
| total_ms avg | 0.408 ms | 0.407 ms |
| exec_ms avg | 0.395 ms | 0.395 ms |
| tg_ms avg | 0.389 ms | 0.388 ms |
| plot_ms | 0 ms | 0 ms |
| deadline miss | 0 / 4,501 (0%) | 0 / 4,501 (0%) |
| backlog | 0 / 4,501 (0%) | 0 / 4,501 (0%) |
| **replot_count avg** | **2.08 / beat** | **1.20 / beat** |
| replot_count dist | [2, 3] | [1, 2, 3] |

replot_count interpretation:

- **R2b (no R1 guard, measured)**: avg **8.22** / beat, distribution [4, 5, 8, 9, 11, 13]
  - Mode 8: most beats trigger 1 replot per tab (BeatNoiseScope/RateScope have 2 plots each)
  - Max 13: RateScopeTab calls mRatePlot->replot() per rate-point event within a frame
  - Min 4: frames with no acoustic events skip BeatError/Sweep
- **R3 Scenario A (R1 applied, 1 tab)**: avg **2.08** → **75% reduction** vs R2b baseline
- **R4 Scenario B (R1 applied, tab switch)**: avg **1.20** → **85% reduction** vs R2b baseline

Key observations:

1. **exec/deadline metrics unchanged**: On macOS, QueuedConnection already makes plot_ms = 0; R1's replot reduction does not affect the exec path here.
2. **replot_count is the direct R1 validation metric**: Quantifies how many tabs actually call replot() per beat — 85% reduction confirmed in Scenario A.
3. **Scenario B catch-up works correctly**: `QTimer::singleShot(0)` in showEvent() ensures the tab refreshes to the latest data snapshot on switch with no UI blocking.
4. **Expected RPi impact**: Unlike macOS, if RPi's rendering runs in the exec path (no QueuedConnection), R1's 85% replot reduction would directly cut ~14ms from plot_ms per beat.

---

### 6.6 Per-Thread Timing Results (R5)

Added per-thread interval timing on the `feature/measure_exe_time` branch.
New metrics: `fg_wait_ms` (DSPWorker `frameLogged` emit → MainWindow `onFrameLogged` entry, FG scheduling latency) and `plot_ms` (accumulated synchronous `replot()` wall time).
Mode: macOS 96kHz **Sim** (synthetic watch signal), 960 samples/frame.
Log file: `src/build-log/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260616_102418.csv`

#### DSP Thread

| Metric | Avg | Max | Min |
|--------|:---:|:---:|:---:|
| wait_ms (BG emit → DSP pickup) | 0.015 ms | 0.049 ms | 0.005 ms |
| exec_ms (total DSP exec) | 0.107 ms | 0.222 ms | 0.030 ms |
| tg_ms (DSP processing) | 0.103 ms | 0.218 ms | — |
| copy_ms (ring-buffer copy) | 0.004 ms | 0.019 ms | — |

> exec_ms is lower than R2–R4 (0.40ms) because Sim mode generates a synthetic signal with lighter DSP load than Playback.

#### FG Thread (MainWindow)

| Metric | Value |
|--------|-------|
| fg_wait_ms avg | **8.936 ms** |
| fg_wait_ms median (p50) | 8.764 ms |
| fg_wait_ms p95 | 16.482 ms |
| fg_wait_ms p99 | **20.509 ms** |
| fg_wait_ms max | 27.145 ms |
| fg_wait > deadline (10.667ms) | **1,577 / 4,501 (35%)** |

> deadline = 1024 samples / 96,000 Hz = 10.667 ms (audio block period)

#### Rendering

| Metric | Value |
|--------|-------|
| plot_ms avg | 0.018 ms |
| plot_ms max | 82.903 ms |
| replot_count avg | 1.08 / beat |

> `plot_ms` accumulates only synchronous `replot()` calls from `replotAll()` (triggered on tab switch). Per-beat async rendering via `rpQueuedReplot` is dispatched through the FG event loop and cannot be captured with `TG_NOW()` wrapping — Qt paint-event level instrumentation would be required.
> `plot_ms max 82.9ms` is a transient spike from a `replotAll()` synchronous render immediately after a tab switch.

#### Thread Activity Timeline

Interactive visualization: [thread-timeline-r5.html](thread-timeline-r5.html)

The chart shows 30 consecutive frames (201–230) as a swimlane timeline:

- **BG lane** (cyan ticks): AudioWorker fires at a precise 10 ms interval
- **DSP lane** (green bars): DSPWorker executes immediately after each BG tick (~0.015 ms wait, ~0.1 ms exec — bars appear as thin lines at this scale)
- **FG lane** (orange bars): FG scheduling wait (translucent) followed by the onFrameLogged handle (solid). Bars that exceed the 10.667 ms deadline are highlighted red.

The FG lane makes the scheduling jitter immediately visible: some frames are handled within 1 ms of DSP completion, others accumulate up to 15 ms of wait before the Qt event loop wakes the main thread.

Key observations:

1. **FG scheduling latency is the key bottleneck**: DSP exec is negligible (0.107ms), but the Qt event loop takes avg 8.9ms to wake the FG thread. 35% of frames exceed the 10.7ms deadline.
2. **High FG wait variability**: p99=20.5ms is nearly 2× the deadline. Even on macOS, the absence of FG thread priority affects real-time data freshness.
3. **DSP thread is stable**: wait_ms ≈ 0.015ms, exec_ms ≤ 0.222ms. Zero deadline misses.
4. **Sim vs Playback exec difference**: Sim mode exec_ms is 0.107ms vs Playback 0.395ms. Use Playback mode for RPi comparisons.

---

## 7. Comparison with RPi Baseline

RPi baseline was measured on `baseline/experiments2` branch (single thread, synchronous tab rendering).

| Metric | macOS · Baseline (R1) | macOS · T2 (R2) | macOS · T2 no guard (R2b) | macOS · T2+R1 Scenario A (R3) | macOS · T2+R1+per-thread (R5, Sim) | RPi · baseline/experiments2 |
|--------|:---------------------:|:---------------:|:-------------------------:|:-----------------------------:|:-----------------------------------:|:---------------------------:|
| wait_ms avg | 420 ms | **0.013 ms** | 0.013 ms | 0.013 ms | 0.015 ms | N/A |
| exec_ms avg | 0.57 ms | **0.40 ms** | ~0.40 ms | 0.395 ms | 0.107 ms ¹ | 20 ms |
| tg_ms avg | 0.56 ms | 0.39 ms | ~0.39 ms | 0.389 ms | 0.103 ms | ~4 ms |
| plot_ms avg | 0 ms | 0 ms | 0 ms | 0 ms | 0.018 ms ² | 16 ms (79%) |
| fg_wait_ms avg | — | — | — | — | **8.936 ms** | — |
| fg_wait_ms p99 | — | — | — | — | **20.509 ms** | — |
| fg_wait > deadline | — | — | — | — | **35%** | — |
| deadline miss | 0% | **0%** | 0% | 0% | 0% | 43% |
| backlog | 47% | **0%** | 0% | 0% | 0% | — |
| replot_count avg | — | — | **8.22** (measured baseline) | **2.08** (↓75%) | 1.08 | — |

> ¹ Sim mode uses a synthetic signal with lower DSP load; Playback mode exec_ms is 0.40ms.
> ² `plot_ms` covers only synchronous `replotAll()` on tab switch. Per-beat async rendering is not measured.

Key observations:

1. **plot_ms = 0**: Tab rendering fully removed from exec path via QueuedConnection → design intent of feature/layer confirmed.
2. **exec_ms = 0.57ms**: Very fast processing on macOS hardware. Expected to be higher on RPi due to hardware difference.
3. **wait_ms = 420ms**: Qt queue wait dominates. On macOS with scheduling slack, signals accumulate and are processed in batches.
4. **backlog 47%**: Half of frames arrive in batch (accumulated). Expected to be worse on RPi under single-core saturation.
5. **deadline miss 0%**: exec processing capacity is sufficient on macOS. On RPi, slow exec caused 43% deadline miss.

---

## 8. Implications for Architecture Decisions

| Tactic | Implication |
|--------|-------------|
| R1 Lazy Render (active tab only) | ✅ **macOS validated** — 75–85% replot reduction. RPi measurement planned in R6 |
| R2 Timer-Decoupled Render | No spike observed on macOS → not needed now. Revisit if RPi R6 shows replot_count spikes or UI freeze |
| T1 SCHED_RR + CPU Affinity | Linux-only. Planned for RPi R7 on top of T2+R1 |
| T2 DSP Offload Thread | ✅ **macOS validated** — wait_ms ×32,000 reduction, backlog 0%. Same effect expected on RPi |

### R2 Trigger Criteria (evaluate after RPi R6)

Apply R2 (Timer-Decoupled 20FPS) after RPi R6 if any of the following is observed:

| Observation | Threshold | Note |
|-------------|-----------|------|
| replot_count distribution | instantaneous max > 20 (burst) | catch-up replots concentrated in one frame |
| exec_ms spike | frame after tab switch > deadline×2 | catch-up replot leaking into exec path |
| UI freeze | >200ms response lag on tab switch | singleShot(0) deferral insufficient |
| replot_count avg | R1 less effective on RPi than macOS | (e.g., avg > 5 → fixed FPS cap more beneficial) |

If none triggered, skip R2 and proceed to T1 (SCHED_RR) in R7.

---

## 9. Next Steps

| Step | Action | Status |
|:----:|--------|:------:|
| macOS R1 | Baseline (main thread DSP) measurement | ✅ Done |
| macOS R2 | T2 (DSP Offload Thread) measurement | ✅ Done |
| macOS R3 | T2+R1 Lazy Rendering · Scenario A (1 tab fixed) | ✅ Done |
| macOS R4 | T2+R1 Lazy Rendering · Scenario B (tab switching) | ✅ Done |
| macOS R5 | T2+R1 + per-thread timing (new fg_wait_ms) · Sim mode | ✅ Done |
| RPi R6 | Run T2+R1 branch on RPi → establish RPi exec_ms + replot_count baseline | ⏳ Next |
| RPi R6 decision | Check R2 trigger criteria (§8 table) → skip if no spike, apply R2 if triggered | Planned |
| RPi R7 | Add T1 (SCHED_RR + CPU Affinity) and re-measure on RPi | Planned |

---

## 10. Links and References

- `baseline/experiments2` branch: RPi baseline measurement source
- `docs/milestone2/architectural-approaches.md`: Tactic options and trade-off analysis
- Log file R1 (Baseline): `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_223711.csv`
- Log file R2 (T2): `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_231224.csv`
- Log file R2b (replot baseline, no guard): `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260615_050649.csv` (branch: `exp/r1-replot-baseline`)
- Log file R3 (T2+R1 Scenario A): `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260615_003136.csv`
- Log file R4 (T2+R1 Scenario B): `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260615_003429.csv`
- Log file R5 (per-thread timing, Sim): `src/build-log/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260616_102418.csv`
- Thread activity timeline (R5): [docs/milestone2/thread-timeline-r5.html](thread-timeline-r5.html)
- Analysis tool: `src/tools/analyze_log.py`
