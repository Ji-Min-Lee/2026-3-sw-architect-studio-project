# Planned Experiments — TimeChecker (M2)

**Team**: Blue Sky (Team 3) | **Milestone**: M3 | **Updated**: 2026-06-28

---

## Overview

| ID | Experiment | QA | Risk | Status | Date |
|----|------------|----|------|:------:|------|
| **EXP-01** | [RPi Real-Time Performance — Dropped Block Measurement](exp-01-realtime-dropped-block.md) | QAS-1 | TR-01 | ✅ Done | 2026-06-15 |
| **EXP-02** | [End-to-End Latency — 2-Segment Timestamp Measurement](exp-02-latency-e2e.md) | QAS-2 | TR-02, TR-03, TR-04 | ✅ Done | 2026-06-11~16 |
| **EXP-03** | [Observer Pattern Compliance — Tab Extension Cost Measurement](exp-03-extensibility-observer-pattern.md) | QAS-3 | — | ✅ Done | 2026-06-21 |
| **EXP-04** | [Detector Parameter Optimization Under Noise](exp-04-correctness-detector-optimization.md) | QAS-4 | TR-05 | ✅ Done | 2026-06-16~17 |
| **EXP-05** | [Signal Quality Warning — Ambient Noise Threshold Validation](exp-05-noise-threshold-popup.md) | QAS-4 + Usability | — | ✅ Done | 2026-06-23 |
| **EXP-06** | [Witschi Accuracy Comparison — TimeChecker vs Witschi No.1000](exp-06-accuracy-witschi-comparison.md) | QAS-5 | — | ✅ Done | 2026-06-25 |
| **EXP-07** | [Long-Term Aging Test — Bucket Downsampling Efficiency](exp-07-longterm-aging.md) | QAS-6 | — | ✅ Done | 2026-06-25 |
| **EXP-08** | [Tab Expansion File-Change Cost](exp-08-tab-expansion-file-change-cost.md) | QAS-3 | TR-08 | ✅ Done | 2026-06-21 |

> **Dependency order**: EXP-01 → EXP-02 → EXP-04 → EXP-05 → EXP-06. EXP-03 / EXP-08 are independent. EXP-07 is independent (analytical).

---

## EXP-06: Witschi Accuracy Comparison — TimeChecker vs Witschi No.1000

### Status

✅ **Done** — 2026-06-25

### Objective

Verify that TimeChecker (RPi 5, 96 kHz, real microphone) produces Rate, Amplitude, and Beat Error values within tolerance of Witschi No.1000, confirming QAS-5 (Measurement Accuracy).

### Result

**Outcome: ✅ Pass** — all three metrics within tolerance across 2 rounds.

| Round | Date | Δ Rate (s/d) | Δ Amplitude (°) | Δ Beat Error (ms) | Result |
|:-----:|------|:------------:|:---------------:|:-----------------:|:------:|
| R1 | 2026-06-25 | **0.4** | **15** | **0.1** | ✅ Pass |
| R2 | 2026-06-25 | **0.2** | **~25** | **0** | ✅ Pass |

| Metric | Tolerance | Max Observed Δ | Pass? |
|--------|:---------:|:--------------:|:-----:|
| Rate | < ±2 s/d | 0.4 s/d | ✅ |
| Amplitude | ± 30° | ~25° | ✅ |
| Beat Error | ± 0.3 ms | 0.1 ms | ✅ |

The ~15–25° amplitude offset is systematic: C-event detection delay extends measured T1, and amplitude is inversely proportional to T1. This is deterministic and consistent — not random error.

### Prerequisites (all completed)

EXP-01 (96k sps confirmed), EXP-02 (E2E < 100 ms), EXP-04 (onset=0.08 confirmed).

### Resources Used

| Resource | Detail |
|----------|--------|
| Hardware | RPi 5, real microphone, Witschi No.1000 |
| Watch | 21,600 BPH |
| Full write-up | [exp-06-accuracy-witschi-comparison.md](exp-06-accuracy-witschi-comparison.md) |

---

## EXP-01: RPi Real-Time Performance — Dropped Block Measurement

### Status

✅ **Done** — 2026-06-15

### Objective

Verify that RPi 5 sustains zero dropped audio blocks at 96k sps under continuous operation, confirming QAS-1 is achievable without real-time scheduling extensions.

### Result

**Dropped Block = 0** across all 9 runs (3 sps × 3 scheduling policies). QAS-1 Pass.

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
| Log directory | `src/logs/EXP-01/` |

---

## EXP-02: End-to-End Latency — 2-Segment Timestamp Measurement

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
| Final (E3-07) | **2.2 / 4.8 ms** | ✅ QAS-2 Pass |

### Architecture Decisions

- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — wait_ms 420ms → 0.013ms (×32,000)
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — replot_count 8.22 → 1.20/beat (↓85%)

### Resources Used

| Resource | Detail |
|----------|--------|
| Hardware | RPi 5 (rpi1, rpi2), macOS dev PC |
| Log directory | `src/logs/EXP-02/` |
| Analysis tool | `src/tools/analyze_log.py` |

---

## EXP-03: Observer Pattern Compliance — Tab Extension Cost Measurement

### Status

✅ **Done** — 2026-06-21

### Objective

Verify that the BaseGraphTab observer pattern allows adding a new graph tab with ≤ 3 file changes and zero references from Presentation to Signal Processing or Acquisition layers.

### Result

All 14 tabs implemented under the ≤ 3-file constraint. Zero layer violations. QAS-3 Pass.

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
| Evidence | [Graph Tab Module Uses View](../views/view-decomposition-graph-tab.md) |
| Code | `src/tabs/BaseGraphTab.h`, `src/tests/` |

---

## EXP-04: Detector Parameter Optimization Under Noise

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
| Log directory | `src/logs/EXP-04/` |
| Analysis tools | `src/tools/analyze_exp04_scatter.py` |

---

## EXP-05: Signal Quality Warning — Ambient Noise Threshold Validation

### Status

✅ **Done** — 2026-06-23

### Objective

Verify that the 55 dB `noiseDb` threshold triggers the signal quality warning popup at the correct SNR boundary: fires at SNR ≤ 0 dB and produces zero false alarms at SNR ≥ 10 dB.

### Result

**Outcome: ✅ Pass** — popup fires at SNR ≤ 0 dB; 0 false alarms at SNR ≥ 10 dB.

| SNR (dB) | noiseDb max | Popup triggers? | Rate Error (s/d) |
|:--------:|:-----------:|:---------------:|:----------------:|
| 10 | 54.9 | No | 1.76 |
| **0** | **56.9** | **Yes** | **−4,968** |
| **−10** | **63.3** | **Yes** | INVALID |

Implemented in `feature/noise` (commits `c0a882a`, `2cac301`).

### Resources Used

| Resource | Detail |
|----------|--------|
| WAV source | `28800BPH_3235_Starbucks_snr{M10..60}db.wav` (8 files, 96kHz) |
| Log | `src/logs/EXP-05/` |
| Full write-up | [exp-05-noise-threshold-popup.md](exp-05-noise-threshold-popup.md) |

---

## EXP-07: Long-Term Aging Test — Bucket Downsampling Efficiency

### Status

✅ **Done** — 2026-06-25 (analytical verification)

### Objective

Verify that the `mBucketSize` time-based downsampling strategy in `LongTermTab` keeps total plotted points ≤ 3,000 and `QCustomPlot::replot()` ≤ 16 ms after 7 days of continuous operation.

### Result

**Conclusion: ✅ Pass** — 2,520 total plotted points at 7 days (≤ 3,000 budget). QCP render time well under 16 ms.

| Session duration | `mBucketSize` | Points per series | Total points (×3) |
|:----------------:|:-------------:|:-----------------:|:-----------------:|
| 7 days | 60 | ~840 | **~2,520** |

Completed as analytical verification from the implemented `mBucketSize` policy and the fixed 12 s averaging period. No live aging run required.

### Resources Used

| Resource | Detail |
|----------|--------|
| Code | `src/tabs/LongTermTab.cpp` — `onMeasurement()`, `addPoint()` |
| ADR | [ADR-007: LongTermTab Downsampling](../adr/ADR-007-longtermtab-downsampling.md) |
| Full write-up | [exp-07-longterm-aging.md](exp-07-longterm-aging.md) |

---

## EXP-08: Tab Expansion File-Change Cost

### Status

✅ **Done** — 2026-06-21

### Objective

Measure how many files outside the new tab itself must be changed when a new graph tab is added to the Presentation layer. Directly tests the QAS-3 modifiability goal (≤ 3 file changes, zero lower-layer references).

### Result

All 14 tabs added within the ≤ 3-file budget. No Domain / Signal Processing / Acquisition file modified in any batch. **QAS-3 Pass. TR-08 Resolved.**

| Batch | Tabs added | Files changed outside new tab |
|-------|-----------|:----:|
| W2 S1 | 11 (baseline) | **2** |
| W2 S2 | +2 → 13 | **2 each** |
| W3 S1 | +1 → **14** | **3** ¹ |

¹ RadarChartTab — SequenceTab modified within Presentation layer only; no lower-layer file touched.

### Resources Used

| Resource | Detail |
|----------|--------|
| Evidence | Git history (W2 S1 → W3 S1), DSM, `TestAddedTabs`, `TestGraphTabs` |
| ADR | [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) |
| Full write-up | [exp-08-tab-expansion-file-change-cost.md](exp-08-tab-expansion-file-change-cost.md) |

---

## Deferred

| Item | Reason | Target |
|------|--------|--------|
| BPH Escalation (36k / 43k BPH) | Blocked until all 28,800 BPH QA targets confirmed | Post-M3 |
