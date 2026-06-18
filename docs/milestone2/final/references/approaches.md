# Architectural Approaches

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

Based on EXP-02 results (RPi real-time FAIL — 43% deadline miss), this document defines and compares architectural alternatives to meet [QAS-1 (Real Time Performance)](qa.md#qas-1-real-time-performance--priority-2) and [QAS-2 (Low Latency and Low Number of Missed Beats)](qa.md#qas-2-low-latency-and-low-number-of-missed-beats--priority-3). Two independent decisions are analyzed, each with three options. The chosen combination is reflected in the ADRs and Construction Plan.

---

## Background

### Confirmed Problem (EXP-02 E2-2 — RPi Baseline, rpi1)

| Metric | Value | Implication |
|--------|:-----:|-------------|
| exec avg | 20 ms | 95% of 21 ms deadline consumed |
| `plot` share | 16 ms | **79% of exec** — rendering is the bottleneck |
| deadline miss | 43% | 441 / 1015 frames failed |
| cpu2 utilization | 91% | audio pipeline pinned to one core |
| temperature | 85 °C | sustained thermal throttling → clock reduction |
| other cores | idle | multi-core unused |

### Root Cause Structure

```
Root Cause 1 — Rendering coupled to audio path
  All tabs' plot() executed inside AudioThread exec
  → plot 16ms × N_tabs = deadline overrun

Root Cause 2 — Single-core saturation
  AudioCapture + DSP + Qt Event Loop → cpu2 91%
  → other 3 cores idle

Root Cause 3 — Thermal throttling
  85°C → clock reduction → further performance degradation
```

The bottleneck is **structural**, not algorithmic. These three causes interact: single-core saturation makes the thermal problem worse, and thermal throttling amplifies the exec overrun. Algorithmic tuning alone cannot fix any of them.

---

## Decision 1: Rendering Strategy

> Addresses: `plot` overrun of exec deadline (Root Cause 1)

### Option Comparison

| Item | R1: Lazy Rendering | R2: Timer-Decoupled Rendering | R3: Double-Buffer Async Rendering |
|------|--------------------|-------------------------------|-----------------------------------|
| Core tactic | Repaint visible tab only | Fixed-FPS timer drives repaint | Render to QPixmap off-screen; UI blits finished pixmap |
| Implementation complexity | **Low** | Medium | **High** |
| exec reduction | High (single tab) | Medium (bounded FPS) | **Very high** (full audio-path isolation) |
| 11 tabs all open | Only 1 renders | N_active × (1/FPS) | Async — no audio-path impact |
| Data consistency risk | None | None | Requires lock on QPixmap sharing |
| M2 feasibility | ✅ Immediate | ✅ Feasible | ⚠️ Design change too large |

---

### R1: Lazy Rendering (Active Tab Only)

Only the currently visible tab repaints each beat. Inactive tabs update their data model but skip rendering via an `isVisible()` guard.

```
BeatEvent → GraphTabManager
  ├→ ActiveTab.updateData(m) → update() → paintEvent()   ← renders
  └→ InactiveTabs.updateData(m)                           ← data only, no render
```

Implementation change per tab (one line):

```cpp
void TraceDisplay::updateData(const Measurement& m) {
    store(m);
    if (!isVisible()) return;  // isVisible() guard
    update();
}
```

A `showEvent()` override renders a catch-up frame when the user switches to a previously inactive tab (< 21ms at 28,800 BPH — imperceptible).

| Pros | Cons |
|------|------|
| Minimal change scope (one guard per tab) | If all 11 tabs are simultaneously visible, guard provides no benefit |
| Fully removes plot from exec path for inactive tabs | Stale frame on tab switch — catch-up frame required |
| No structural change below Presentation layer | |

**QA impact**: QAS-1 exec 16ms → ~1ms (10 inactive tabs); QAS-3 tab ≤ 3 file rule preserved.

---

### R2: Timer-Decoupled Rendering

Decouple rendering from beat events entirely. A `QTimer` (20 FPS, 50ms interval) drives `update()` on visible tabs; beat events only update the data model.

```
BeatEvent → IGraphTab::updateData(m) → store in data model (no update() call)
QTimer(50ms) → all visible tabs → update() → paintEvent() → plot()
```

| Pros | Cons |
|------|------|
| Rendering fully decoupled from audio deadline path | Timer lifecycle management required (start/stop with pipeline) |
| FPS cap explicit and controllable | Over-renders at low BPH (e.g. 18,000 BPH = 5 beats/sec) |
| All tabs update uniformly | Wider code change scope than R1 (TabManager + all tabs) |

---

### R3: Double-Buffer Async Rendering

Each tab renders into a `QPixmap` asynchronously on a render worker thread; the UI thread only blits the finished pixmap.

```
Audio Thread: BeatEvent → DataModel (lock-free write)
Render Worker: DataModel (read) → QPixmap (off-screen)
UI Thread: QPixmap blit → paintEvent() (microseconds)
```

| Pros | Cons |
|------|------|
| Complete audio ↔ render isolation | Qt restricts QPixmap creation to the UI thread — design requires significant rework |
| Maximum exec reduction | 3-thread synchronization, data race risk |
| High-BPH renders are no concern | Too high risk for M2 deadline |

**Status**: Rejected for M2. Deferred to post-M3 review (see [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md)).

---

### Decision 1 Outcome

**R1 (Lazy Rendering) — Accepted.** See [ADR-002](adr/ADR-002-r1-lazy-rendering.md).

Rationale: Lowest implementation risk for M2. EXP-02 confirmed replot_count 8.22 → 1.20 (↓85%) on macOS. R2 is documented as a conditional replacement ([ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md)) if EXP-05 shows R1 insufficient under 11-tab full load on RPi.

---

## Decision 2: Threading Strategy

> Addresses: single-core saturation and idle multi-core (Root Causes 2 & 3)

### Current Structure

```
cpu2 (Audio Thread):
  AudioCapture → SignalBuffer → FilterChain → BeatDetector → MeasurementEngine
                                                                   ↓ Qt queued signal
Qt Main Thread (any core):
  GraphTabManager → 11 Graph Tabs → paintEvent()
```

### Option Comparison

| Item | T1: SCHED_RR + CPU Affinity | T2: DSP Offload Thread | T3: Full Pipeline Split |
|------|-----------------------------|------------------------|-------------------------|
| Core tactic | OS real-time scheduling + core pinning | Separate AudioCapture from DSP onto two threads | One thread per pipeline stage |
| Core distribution | Partial (capture core isolated) | Medium (capture + DSP on separate cores) | **High** (stage-per-core) |
| Implementation complexity | **Low** (OS API only) | Medium | **High** |
| Direct exec reduction | None (scheduling improvement only) | Medium | High |
| Module structure change | None | SignalBuffer role added | Full pipeline redesign |
| M2 feasibility | ✅ Immediate | ✅ Feasible | ⚠️ High risk |

---

### T1: SCHED_RR + CPU Affinity

Apply SCHED_RR to the audio thread and pin it to a dedicated core using `pthread_setaffinity_np`. Other cores handle the Qt event loop and rendering.

```cpp
struct sched_param param { .sched_priority = 80 };
pthread_setschedparam(pthread_self(), SCHED_RR, &param);

cpu_set_t cpuset;
CPU_ZERO(&cpuset); CPU_SET(0, &cpuset);
pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
```

| Pros | Cons |
|------|------|
| Minimal code change | Does not reduce exec — plot bottleneck remains |
| Can improve QAS-1 Dropped Block | Does not solve root cause (single-core saturation) |
| Immediately testable via EXP-01 | Does not address thermal throttling |

**EXP-01 result**: SCHED_RR showed no improvement in Dropped Block count on RPi. T1 applied to the audio capture thread is **not required** for QAS-1. See [EXP-01](experiments/exp-01-sample-rate.md).

---

### T2: DSP Offload Thread (AudioCapture ↔ DSP Separation)

Separate `AudioCapture` (audio callback) from the DSP pipeline into two threads connected by a lock-free PCM ring buffer.

```
cpu_N (Capture Thread, SCHED_RR):
  AudioCapture → [PCM Ring Buffer — lock-free write]

cpu_M (DSP Thread):
  [PCM Ring Buffer — lock-free read] → FilterChain → BeatDetector → MeasurementEngine
                                                                          ↓ Qt QueuedConnection

Qt Main Thread:
  GraphTabManager → Tabs
```

| Pros | Cons |
|------|------|
| Capture thread stays lightweight (write only) | Ring buffer added — small design change |
| DSP spread across cores — cpu2 saturation resolved | Thread synchronization design required |
| SCHED_RR scope narrowed to capture thread only | PCM ring buffer and existing SignalBuffer roles must be clearly separated |

**QA impact**: QAS-1 Dropped Block risk reduced (lightweight capture). QAS-2 wait_ms 420ms → 0.013ms (×32,000) — Qt event loop backlog eliminated.

---

### T3: Full Pipeline Thread Split

Each pipeline stage (Capture / Filter / Detect / Measure) on a separate thread pinned to a dedicated core.

```
cpu0: AudioCapture (SCHED_RR)
cpu1: FilterChain
cpu2: BeatDetector + MeasurementEngine
cpu3: Qt Event Loop + Rendering
```

| Pros | Cons |
|------|------|
| Maximum core utilization | Inter-stage queues × 3 → potential latency increase |
| Thermal load distributed | Complexity greatly increased |
| | Very high implementation risk for M2 |

**Status**: Rejected for M2. T2 covers the required benefit with far less complexity.

---

### Decision 2 Outcome

**T2 (DSP Offload Thread) — Accepted.** See [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md).

Rationale: EXP-02 confirmed wait_ms 420ms → 0.013ms (×32,000) and backlog 47% → 0% on macOS. RPi runs (E2-5/E2-6) confirmed E2E avg 2.05ms, 0 deadline miss, 0 backlog. T1 found unnecessary by EXP-01.

---

## Combined Trade-off Matrix

| Combo | exec reduction | Core distribution | Complexity | M2 risk | Recommendation |
|:-----:|:--------------:|:-----------------:|:----------:|:-------:|----------------|
| **R1 + T2** | ★★★★☆ | ★★★★☆ | ★★★☆☆ | Medium | **Selected — balanced** |
| R1 + T1 | ★★★★☆ | ★★☆☆☆ | ★☆☆☆☆ | Low | Fast MVP, weaker threading |
| R2 + T2 | ★★★★★ | ★★★★☆ | ★★★★☆ | Medium | Structural completeness priority |
| R2 + T1 | ★★★☆☆ | ★★☆☆☆ | ★★☆☆☆ | Low | Render timing control priority |
| R3 + T3 | ★★★★★ | ★★★★★ | ★★★★★ | **High** | Post-M3 only |

**Selected combination: R1 + T2.** Confirmed by EXP-02 on both macOS and RPi (E2-5/E2-6).

---

## Experiment Outcomes

| Experiment | Decision Impact | Outcome |
|-----------|-----------------|---------|
| [EXP-01](experiments/exp-01-sample-rate.md) | T1 (SCHED_RR) necessity for audio thread | **Not required** — Dropped=0 at 96kHz across all scheduling policies |
| [EXP-02 macOS](experiments/exp-02-pipeline-latency.md) | R1 + T2 quantification | R1: replot ↓85%. T2: wait_ms ×32,000, backlog 0% |
| [EXP-02 RPi E2-5/6](experiments/exp-02-pipeline-latency.md) | R1 + T2 RPi confirmation | E2E avg 2.05ms, 0 deadline miss, 0 backlog — **fully confirmed** |
| [EXP-02 RPi E2-7](experiments/exp-02-pipeline-latency.md) | New bottleneck identification | fg_wait avg 60.1ms, 84% > deadline — **FG Qt scheduler is the next concern** |
| [EXP-03](experiments/exp-03-filter-sweep.md) | Filter cutoff constants | ⏳ In Progress (target: 06/25) |
| [EXP-05](experiments/exp-05-rendering-fps.md) | R1 sufficiency under 11-tab load | ⏳ In Progress (target: 06/26). If fail → R2 via [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) |

### Open Concern: FG Scheduling Bottleneck

EXP-02 E2-7 revealed that the Qt FG event loop on RPi takes avg **60.1ms** to wake the FG thread after `frameLogged` is emitted — 84% of frames exceed the 21.33ms deadline. This is not CPU-bounded (cpu0/2/3 near-idle); it is an OS scheduler priority issue.

Current options under consideration:
- T1 applied to the **FG thread** (not the audio thread): SCHED_RR + CPU affinity for `MainWindow::onFrameLogged`
- `QTimer`-based periodic FG polling instead of signal-driven delivery

This concern does not affect DSP accuracy (DSP E2E avg 2.2ms, 0 deadline miss) but will affect GUI responsiveness and display latency measurement completeness.

---

## Architecture Decision Records

| Decision | Option Chosen | ADR |
|----------|:-------------:|-----|
| Rendering strategy | **R1** Lazy Rendering | [ADR-002](adr/ADR-002-r1-lazy-rendering.md) |
| Threading strategy | **T2** DSP Offload Thread | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) |
| Sample rate | **96kHz** (Option A) | [ADR-003](adr/ADR-003-sample-rate-selection.md) |
| Rendering fallback (conditional) | R2 Timer-Decoupled | [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) |
