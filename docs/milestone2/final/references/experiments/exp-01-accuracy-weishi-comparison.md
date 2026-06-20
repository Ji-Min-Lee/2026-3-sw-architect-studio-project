# EXP-01: WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000

**QA**: QAS-1 | **Status**: ⏸ Planned — W5 S1 (2026-06-29 ~ 2026-06-30)

---

## Objective

Verify that TimeChecker (RPi 5, 96 kHz, real microphone) produces Rate, Amplitude, and Beat Error values that match WeiShi No.1000 within measurement tolerance.

## Pass Condition

| Metric | Tolerance |
|--------|:---------:|
| Δ Rate | < 0.5 s/d |
| Δ Amplitude | < 5° |
| Δ Beat Error | < 0.1 ms |

## Experiment Plan

1. Place the same mechanical watch (28,800 BPH) on WeiShi No.1000 and TimeChecker simultaneously
2. Run both for ≥ 5 minutes
3. Record Rate (s/d), Amplitude (°), Beat Error (ms) from both devices
4. Compute |TimeChecker − WeiShi| for each metric
5. Pass if all three deltas are within tolerance

## Prerequisites

EXP-02 (96k sps confirmed), EXP-03 (E2E latency < 100 ms), EXP-05 (detector params onset=0.08) must be complete before running.

## Links

- Full experiment tracker: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-01-weishi-accuracy-comparison----timechecker-vs-weishi-no1000)
