# EXP-01: WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000

**QA**: QAS-1 | **Status**: ⏸ Planned — W5 S1 (2026-06-29 ~ 2026-06-30)

---

## Objective

Verify that TimeChecker (RPi 5, 96 kHz, real microphone) produces Rate, Amplitude, and Beat Error values that match WeiShi No.1000 within measurement tolerance.

## Pass Condition

| Metric | Tolerance |
|--------|:---------:|
| Δ Rate | < 0.3 s/d |
| Δ Amplitude | < 0.01° |
| Δ Beat Error | 0 ms |

## Experiment Plan

> **Note**: Only one watch available — measurements are taken sequentially, not simultaneously. Each round is kept to ≤ 5 minutes to minimize drift between devices.

1. Place the watch on WeiShi No.1000 → run 5 min → record Rate (s/d), Amplitude (°), Beat Error (ms)
2. Immediately transfer to TimeChecker (RPi 5, 96kHz, real mic) → run 5 min → record same metrics
3. Repeat steps 1–2 for ≥ 3 rounds to average out short-term rate drift
4. Compute |TimeChecker avg − WeiShi avg| for each metric across all rounds
5. Pass if all three deltas are within tolerance

## Prerequisites

EXP-02 (96k sps confirmed), EXP-03 (E2E latency < 100 ms), EXP-05 (detector params onset=0.08) must be complete before running.

## Links

- Full experiment tracker: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-01-weishi-accuracy-comparison----timechecker-vs-weishi-no1000)
