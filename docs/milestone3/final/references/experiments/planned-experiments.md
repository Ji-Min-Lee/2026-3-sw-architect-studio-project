# Planned Experiments — TimeChecker (M2)

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Updated**: 2026-06-20

---

## Overview

| ID | Experiment | QA | Risk | Status | Date |
|----|------------|----|------|:------:|------|
| **EXP-01** | [WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000](exp-01-accuracy-weishi-comparison.md) | QAS-1 | — | ⏸ Planned | W5 S1 (2026-06-29) |
| **EXP-02** | [RPi Real-Time Performance — Dropped Block Measurement](exp-02-realtime-dropped-block.md) | QAS-2 | TR-01 | ✅ Done | 2026-06-15 |
| **EXP-03** | [End-to-End Latency — 2-Segment Timestamp Measurement](exp-03-latency-e2e.md) | QAS-3 | TR-02, TR-03, TR-04 | ✅ Done | 2026-06-11~16 |
| **EXP-04** | [Observer Pattern Compliance — Tab Extension Cost Measurement](exp-04-extensibility-observer-pattern.md) | QAS-4 | — | ✅ Done | 2026-06-21 |
| **EXP-05** | [Detector Parameter Optimization Under Noise](exp-05-correctness-detector-optimization.md) | QAS-5 | TR-05 | ✅ Done | 2026-06-16~17 |

> **Dependency order**: EXP-02 → EXP-03 → EXP-05 → EXP-01. EXP-04 is independent.

---

## EXP-01: WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000

### Status

⏸ **Planned** — W5 S1 (2026-06-29 ~ 2026-06-30)

### Objective

Verify that TimeChecker (RPi 5, 96 kHz, real microphone) produces Rate, Amplitude, and Beat Error values within tolerance of WeiShi No.1000, confirming QAS-1 (Measurement Accuracy).

### Pass Condition

| Metric | Tolerance |
|--------|:---------:|
| Δ Rate | < 0.3 s/d |
| Δ Amplitude | < 0.01° |
| Δ Beat Error | 0 ms |

### Experiment Plan

> **Note**: Only one watch available — measurements are taken sequentially, not simultaneously.

1. Place the watch on WeiShi No.1000 → run 5 min → record Rate (s/d), Amplitude (°), Beat Error (ms)
2. Immediately transfer to TimeChecker (RPi 5, 96kHz, real mic) → run 5 min → record same metrics
3. Repeat steps 1–2 for ≥ 3 rounds to average out short-term rate drift
4. Compute |TimeChecker avg − WeiShi avg| for each metric across all rounds
5. Pass if all three deltas are within tolerance

### Prerequisites

EXP-02 (96k sps confirmed), EXP-03 (E2E < 100 ms), EXP-05 (onset=0.08 confirmed) must be complete.

### Resources Required

| Resource | Detail |
|----------|--------|
| Hardware | RPi 5, real microphone, WeiShi No.1000 |
| Software | TimeChecker (Live mode, 96kHz) |
| Mechanical watch | One 28,800 BPH watch |
| Effort | ~1 person-day |

---

## EXP-02: RPi Real-Time Performance — Dropped Block Measurement

### Status

✅ **Done** — 2026-06-15

### Objective

Verify that RPi 5 sustains zero dropped audio blocks at 96k sps under continuous operation, confirming QAS-2 is achievable without real-time scheduling extensions.

### Result

**Dropped Block = 0** across all 9 runs (3 sps × 3 scheduling policies). QAS-2 Pass.

**Confirmed operating point**: 96k sps, default scheduling — exec avg 9.6 ms (< 21.3 ms deadline). SCHED_RR / FIFO not required.

| Condition | sps | Scheduling | Dropped |
|-----------|:---:|-----------|:-------:|
| Target ★ | **96k** | default | **0** |
| Fallback | 48k | default | 0 |
| Stretch | 192k | default | 0 |
| RT tests | 96k | SCHED_RR / FIFO | 0 |

### Architecture Decision

→ 96k sps accepted. [ADR-003](../adr/ADR-003-sample-rate-selection.md)

### Resources Used

| Resource | Detail |
|----------|--------|
| Hardware | RPi 5 (host=lg1, device=rpi1) |
| Software | TimeChecker with `droppedBlockCount` counter, 30 s ring buffer |
| Log directory | `src/logs/EXP-01/` (legacy name) |

---

## EXP-03: End-to-End Latency — 2-Segment Timestamp Measurement

### Status

✅ **Done** — 2026-06-11 ~ 2026-06-16

### Objective

Measure end-to-end latency from audio block ready to beat result delivered, identify the bottleneck causing deadline misses, and validate the T2 and R1 tactic decisions ([ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md), [ADR-002](../adr/ADR-002-r1-lazy-rendering.md)).

### Measurement Segments

| Segment | Boundary |
|---------|----------|
| `wait_ms` | `emit PlaybackDataReady` → `MainWindow::HandleInputData()` entry |
| `exec_ms` | `HandleInputData()` entry → exit |

### Result

| Tactic | E2E avg/max | Improvement |
|--------|:-----------:|:-----------:|
| Baseline (rpi2) | 57.2 / 208.9 ms | — |
| +T2 DSP Offload | 2.1 / 11.1 ms | −97% |
| +R1 Lazy Rendering | 2.1 / 5.7 ms | tighter max |
| Final (E3-07) | **2.2 / 4.8 ms** | ✅ QAS-3 Pass |

### Architecture Decisions

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — wait_ms 420ms → 0.013ms (×32,000)
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — replot_count 8.22 → 1.20/beat (↓85%)

### Resources Used

| Resource | Detail |
|----------|--------|
| Hardware | RPi 5 (rpi1, rpi2), macOS dev PC |
| Log directory | `src/logs/EXP-02/` (legacy name) |
| Analysis tool | `src/tools/analyze_log.py` |

---

## EXP-04: Observer Pattern Compliance — Tab Extension Cost Measurement

### Status

✅ **Done** — 2026-06-21

### Objective

Verify that the BaseGraphTab observer pattern allows adding a new graph tab with ≤ 3 file changes and zero references from Presentation to Signal Processing or Acquisition layers.

### Result

All 14 tabs implemented under the ≤ 3-file constraint. Zero layer violations. QAS-4 Pass.

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 |
| SP / Acquisition refs from Presentation | 0 | ✅ 0 |
| Observer contract compliance | 100% | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

### Architecture Decisions

- [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md)
- [ADR-005: IAudioSource Dependency Inversion](../adr/ADR-005-p1-iaudiosource-dependency-inversion.md)

### Resources Used

| Resource | Detail |
|----------|--------|
| Evidence | [Allocation View: Implementation Style](../views/view-allocation-implementation.md) |
| Code | `src/tabs/BaseGraphTab.h`, `src/tests/` |

---

## EXP-05: Detector Parameter Optimization Under Noise

### Status

✅ **Done** — 2026-06-16 ~ 2026-06-17

### Objective

Identify `onset_fraction` and `min_peak_fraction` values that maintain accurate beat detection across 0–60 dB SNR noise conditions, confirming the default parameters for `Detector.cpp`.

### Result

**Recommended: `onset_fraction = 0.08`, `min_peak_fraction = 0.10`**

`onset=0.08` is the only value that maintains tracking at 60 dB SNR. `onset=0.02` and `0.05` fail catastrophically at high noise.

| Parameter | Tested range | Selected | Reason |
|-----------|-------------|:--------:|--------|
| `onset_fraction` | 0.02, 0.05, 0.08 | **0.08** | Only value stable at 60 dB SNR |
| `min_peak_fraction` | 0.10, 0.20, 0.30 | **0.10** | Lowest Beat Error within onset=0.08 group |

### Architecture Decision

`Detector.cpp` default parameters updated. [QAS-4: Correctness](../qa/qas-4-correctness.md)

### Resources Used

| Resource | Detail |
|----------|--------|
| Hardware | RPi 5 (host=lg1, device=rpi1) |
| WAV source | 28,800 BPH real recording + pink noise (96 kHz, float32) |
| Measurements | 274 (full grid: onset × min_peak × noise × 5 reps) |
| Log directory | `src/logs/EXP-03/` (legacy name) |
| Analysis tools | `src/tools/analyze_exp04_scatter.py` |

---

## Deferred

| Item | Reason | Target |
|------|--------|--------|
| Signal Quality Warning Threshold (`⚠ Noisy signal`) | Implementation prerequisite not met in M2 timeframe | M3 |
| BPH Escalation (36k / 43k BPH) | Blocked until all 28,800 BPH QA targets confirmed | M3+ |
