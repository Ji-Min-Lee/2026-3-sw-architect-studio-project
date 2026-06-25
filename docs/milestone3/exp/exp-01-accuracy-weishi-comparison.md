# Experiment 1: WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000

## Results and recommendations

**Outcome: ✅ Pass** — Rate within 0.4 s/d, Amplitude within 15°, Beat Error within 0.1 ms.

Both systems detected the same BPH (21600) and showed consistent rate direction (+fast). The amplitude difference (15°) is attributable to sensor coupling: WeiShi uses a direct contact sensor while TimeChecker uses a free-air microphone.

**Recommendation**: Accept the acoustic-vs-contact sensor delta as inherent to the measurement method. QAS-5 is verified with the ±2 s/d real-world tolerance.

| Metric | WeiShi No.1000 | TimeChecker | Delta | Tolerance | Pass? |
|--------|:--------------:|:-----------:|:-----:|:---------:|:-----:|
| Rate | +14.0 s/d | +13.6 s/d | 0.4 s/d | < ±2 s/d | ✅ |
| Amplitude | 294° | 279° | 15° | ± 30° | ✅ |
| Beat Error | 0.2 ms | 0.1 ms | 0.1 ms | ± 0.3 ms | ✅ |

## Objective

Verify that TimeChecker (RPi 5, 96 kHz, real microphone) produces Rate, Amplitude, and Beat Error values that match WeiShi No.1000 within measurement tolerance, confirming QAS-5 (Measurement Accuracy).

Technical question: Do the Rate, Amplitude, and Beat Error values computed by TimeChecker agree with WeiShi No.1000 within the tolerance window defined in QAS-5?

## Status

Concluded

## Expected outcomes

- Δ Rate, Δ Amplitude, Δ Beat Error compared against the WeiShi reference for ≥ 1 round
- Quantitative pass/fail verdict per metric
- Attribution of any systematic offset (sensor type, coupling, gain)

## Resources required

- RPi 5 with USB microphone
- WeiShi No.1000 reference device
- One mechanical watch (21,600 BPH)
- TimeChecker built in Live mode (96 kHz, onset=0.08, min_peak=0.10 from EXP-05)
- ~0.5 person-days

## Experiment description

> Only one watch available — measurements taken sequentially. Each round kept to ≤ 5 min to minimize drift.

1. Wind the watch fully; allow to stabilize for 2 minutes
2. Place on WeiShi No.1000 → run 5 min → record Rate (s/d), Amplitude (°), Beat Error (ms)
3. Transfer immediately to TimeChecker (RPi 5, 96 kHz, real mic) → run 5 min → record same metrics
4. Compute |TimeChecker − WeiShi| per metric
5. Pass if all three deltas are within tolerance

**If results differ**, investigate in order: microphone placement, FilterChain cutoff clipping, gain/clipping (Sound Print tab), short-term rate drift (add more rounds).

## Duration

Completed 2026-06-25.

## Links and references

- [QAS-5: Measurement Accuracy](../final/references/qa/qas-5-measurement-accuracy-error-detection-handling.md)
- [ADR-003: 96kHz Sample Rate](../final/references/adr/ADR-003-sample-rate-selection.md)
- [ADR-009: FilterChain Design](../final/references/adr/ADR-009-filterchain-design.md)
- [EXP-05: Detector Parameter Optimization](exp-05-correctness-detector-optimization.md) — prerequisite; establishes onset=0.08
- [experiment-results.md](../experiment-results.md) — full run history and per-metric data
