# EXP-01: WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000

**QA**: QAS-5 | **Status**: ⏸ Planned — W5 S1 (2026-06-29 ~ 2026-06-30)

---

## Objective

Verify that TimeChecker (RPi 5, 96 kHz, real microphone) produces Rate, Amplitude, and Beat Error values that match WeiShi No.1000 within measurement tolerance.

## Pass Condition

| Metric | Tolerance |
|--------|:---------:|
| Δ Rate | < 0.3 s/d |
| Δ Amplitude | < 0.01° |
| Δ Beat Error | 0 ms |

## Experiment Design Rationale

This experiment uses a **dual-system comparison** approach: the same watch is measured on both systems under equivalent conditions. If the two systems produce similar values, both are likely measuring the same physical phenomenon correctly. If they differ, that difference is the starting point for diagnosis — not a failure to ignore.

This is a scientific validation: consistency across repeated measurements on a consistently-wound watch strengthens the accuracy argument independently of absolute ground truth.

## Experiment Plan

> **Note**: Only one watch available — measurements are taken sequentially, not simultaneously. Each round is kept to ≤ 5 minutes to minimize drift between devices.

1. Wind the watch fully; allow to stabilize for 2 minutes before starting
2. Place the watch on WeiShi No.1000 → run 5 min → record Rate (s/d), Amplitude (°), Beat Error (ms)
3. Immediately transfer to TimeChecker (RPi 5, 96kHz, real mic) → run 5 min → record same metrics
4. Repeat steps 2–3 for ≥ 3 rounds to average out short-term rate drift
5. Compute |TimeChecker avg − WeiShi avg| for each metric across all rounds
6. Pass if all three deltas are within tolerance

## If Results Differ

A difference between the two systems is not automatically a failure — it is data. Investigate the most likely causes in this order:

| Likely Cause | Diagnostic Step |
|---|---|
| Microphone placement / coupling | Re-seat the watch; re-run one round and compare |
| FilterChain parameters (LP/HP cutoff) | Check if cutoff values are clipping the T1 or T3 event envelope |
| Gain / clipping | Inspect the Sound Print tab for signal saturation |
| Short-term rate drift between sequential measurements | Increase rounds to 5; check if delta shrinks with averaging |
| WeiShi calibration offset | Compare WeiShi reading against a known-rate reference signal if available |

Document the observed delta and the most plausible explanation regardless of pass/fail outcome.

## Prerequisites

EXP-02 (96k sps confirmed), EXP-03 (E2E latency < 100 ms), EXP-05 (detector params onset=0.08) must be complete before running.

## Links

- Full experiment tracker: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-01-weishi-accuracy-comparison----timechecker-vs-weishi-no1000)
