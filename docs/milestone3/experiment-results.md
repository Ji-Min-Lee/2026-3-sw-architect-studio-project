# Experiment Results

This document records the five experiments conducted during Milestones 1–2 and their relationship to the architectural quality attribute scenarios (QAS). Each experiment was designed to answer a specific architectural question; the results either validated an assumption or triggered an architecture change.

---

## Summary

| ID | QA | Question | Runs | Key Result | Status |
|----|----|----------|:----:|------------|:------:|
| EXP-01 | QAS-2 Measurement Accuracy | Do computed Rate/Amplitude/Beat Error match the WeiShi No.1000 reference? | 0 | Scheduled for Week 5 (2026-06-29) | ⏳ Planned |
| EXP-02 | QAS-1 Real-Time Performance | Can RPi 5 sustain 96 kHz with zero dropped blocks? | 9 | Dropped blocks = **0** across all sps and scheduling policies | ✅ Done |
| EXP-03 | QAS-3 Low Latency | What is the end-to-end latency? Does T2 + R1 keep it within budget? | 7 | DSP E2E avg **2.2 ms** / max **4.8 ms** after T2+R1. FG scheduling lag avg 60 ms identified as next bottleneck | ✅ Done |
| EXP-04 | QAS-5 Extensibility | Can a new graph tab be added with ≤ 3 file changes and zero layer violations? | 1 | ≤ 3 files per tab · 0 Signal Processing refs from Presentation · 14 tabs all pass | ✅ Done |
| EXP-05 | QAS-2 Measurement Accuracy | Which detector parameters yield stable measurement under noise? | 274 | `onset=0.08, min_peak=0.10` — stable across 0–50 dB; only setting to survive 60 dB | ✅ Done |

---

## EXP-01: WeiShi Accuracy Comparison

**QA**: QAS-2 Measurement Accuracy | **Status**: ⏳ Planned (Week 5, 2026-06-29)

**Question**: Do Rate (s/d), Amplitude (°), and Beat Error (ms) computed by TimeGrapher match WeiShi No.1000 within the target tolerance?

**Plan**: Place watch on WeiShi → run 5 min → record values. Transfer to TimeGrapher (RPi 5, 96 kHz, USB microphone) → run 5 min → record same metrics. Repeat ≥ 3 rounds to average out short-term drift.

**Pass criteria**:

| Metric | Target |
|--------|--------|
| Rate error vs WeiShi | < 5 s/d |
| Amplitude error vs WeiShi | < 5° |
| Beat Error deviation | < 0.1 ms |

**Architecture risk**: This is the governing goal (QAS-2). EXP-05 confirmed the detector parameters are correct; EXP-01 validates the full measurement pipeline against physical hardware.

---

## EXP-02: RPi Real-Time Performance — Dropped Block Measurement

**QA**: QAS-1 Real-Time Performance | **Date**: 2026-06-15 | **Status**: ✅ Done

**Question**: Can RPi 5 process audio at 96,000 sps with zero dropped blocks while running Qt GUI and DSP concurrently?

**Answer**: Yes. **Dropped blocks = 0 across all sample rates (48k / 96k / 192k) and all scheduling policies (default / SCHED_RR / SCHED_FIFO).** QAS-1 Pass.

**Key results (96 kHz target — 9 runs)**:

| Scheduling | exec avg | exec max | exec > deadline (21.33 ms) | Dropped |
|-----------|:--------:|:--------:|:--------------------------:|:-------:|
| Default | 9.6 ms | 39.2 ms | 8.1 % | **0** |
| SCHED_RR p50 | 9.8 ms | 39.9 ms | 8.4 % | **0** |
| SCHED_FIFO p50 | 9.9 ms | 41.4 ms | 8.6 % | **0** |

**Conclusions**:
- 96 kHz is the confirmed target sample rate — sufficient headroom with 0 drops
- SCHED_RR / SCHED_FIFO do not improve dropped block count; no real-time OS priority needed
- Thermal throttling (≥ 85°C) was observed in all runs; does not affect dropped block count but indicates a cooling requirement for sustained operation

---

## EXP-03: End-to-End Latency — 2-Segment Timestamp Measurement

**QA**: QAS-3 Low Latency | **Date**: 2026-06-11 ~ 2026-06-16 | **Status**: ✅ Done

**Question**: What is the end-to-end latency from audio capture to screen render? Do ADR-001 (DSP Thread) and ADR-002 (Lazy Rendering) reduce it within the 100 ms budget?

**Answer**: Yes. T2 + R1 reduced E2E from 80 ms to 2.2 ms avg (−97%). FG Qt scheduling lag (avg 60 ms) is identified as the next bottleneck.

**Run progression**:

| Run | Configuration | E2E avg | E2E max | Result |
|:---:|--------------|:-------:|:-------:|:------:|
| E3-02 | RPi unoptimized baseline | 255.4 ms | 900.9 ms | ❌ FAIL |
| E3-03 | RPi baseline (plot removed from exec path) | 57.2 ms | 208.9 ms | ❌ FAIL |
| E3-04 | + multi-tab rendering | 80.1 ms | 258.7 ms | ❌ FAIL |
| E3-05 | + T2 DSP Offload Thread (ADR-001) | 2.1 ms | 11.1 ms | ✅ PASS |
| E3-06 | + R1 Lazy Rendering (ADR-002) | 2.1 ms | 5.7 ms | ✅ PASS |
| E3-07 | + FG wait measurement | 2.2 ms | 4.8 ms | ✅ PASS |

**Before / after ADR-001 (DSP Thread)**:

| Metric | Before | After | Change |
|--------|:------:|:-----:|:------:|
| wait_ms avg | 77.4 ms | 0.03 ms | ×2,600 improvement |
| deadline miss | 43% | 0% | eliminated |
| replot/beat after R1 | 8.22 | 1.20 | ↓85% |

**New finding (E3-07)**: FG (foreground Qt event-loop pickup) avg 60 ms — 84% of frames exceed the 21.33 ms deadline. This is 7× slower than macOS and is the next engineering target.

---

## EXP-04: Observer Pattern Compliance — Tab Extension Cost

**QA**: QAS-5 Extensibility | **Date**: 2026-06-21 | **Status**: ✅ Done

**Question**: Can a new graph tab be added with ≤ 3 file changes and zero direct dependencies on Signal Processing or Acquisition layers?

**Answer**: Yes. All 14 tabs were implemented under this constraint. Verified by automated unit tests and Dependency Structure Matrix (DSM).

**Evidence**:

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 (header + source + registration) |
| Signal Processing / Acquisition refs from Presentation | 0 | ✅ 0 — DSM verified |
| Observer contract compliance (all 14 tabs) | 100% | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 (37 test cases) |

**Tab addition history** (observed across three sprint rounds):

| Batch | Tabs added | Files changed per tab |
|-------|:----------:|:---------------------:|
| W2 Sprint 1 | 11 (baseline) | 2 |
| W2 Sprint 2 | +2 → 13 | 2 |
| W3 Sprint 1 | +1 → 14 | 3 |

**Conclusion**: The [BaseGraphTab Observer Pattern (ADR-006)](ADRs/ADR006-observer-pattern.md) and four-layer architecture ([ADR-003](ADRs/ADR003-layered-architecture.md)) structurally enforce the ≤ 3-file rule. `MeasurementEngine` has zero compile-time knowledge of any tab.

---

## EXP-05: Detector Parameter Optimization Under Noise

**QA**: QAS-2 Measurement Accuracy | **Date**: 2026-06-16 ~ 2026-06-17 | **Status**: ✅ Done

**Question**: Which combination of `onset_fraction` and `min_peak_fraction` yields the most stable measurement rate under varying noise conditions?

**Answer**: `onset_fraction = 0.08, min_peak_fraction = 0.10`. Rate stable at approximately +4.0 s/d across 0–50 dB SNR. Only this setting survives 60 dB noise (+7.5 s/d); onset=0.02 and 0.05 fail catastrophically at 60 dB.

**Parameter grid**: 274 measurement runs across onset {0.02, 0.05, 0.08} × min_peak {0.10, 0.20, 0.30} × noise {0–60 dB, 7 levels} × 5 reps.

**Rate stability by onset (min_peak=0.10, best setting)**:

| Noise (dB) | onset=0.02 | onset=0.05 | onset=0.08 ✅ |
|:----------:|:-----------:|:-----------:|:-------------:|
| 0 | +12.11 s/d | +4.24 s/d | +4.06 s/d |
| 30 | +8.11 s/d | +4.15 s/d | +4.02 s/d |
| 50 | +14.65 s/d | +4.02 s/d | +3.86 s/d |
| **60** | **−4,264** ❌ | **−393** ❌ | **+7.51** ✅ |

**Conclusion**: `onset=0.08, min_peak=0.10` is the recommended detector configuration. `min_peak` has negligible effect within the onset=0.08 group; 0.10 selected for slightly lower beat error. This configuration is set as the system default for the M3 demo.
