# QAS-5: Correctness — Priority 5

> M1 name: "Correctness (QA-C2)". M2 refactor: split into signal quality (this) and accuracy goal (QAS-1).
> M1 status: Provisional (pending EXP-05). M2 status: ✅ Verified 06/17 — onset=0.08, min_peak=0.10 confirmed.

| Field | Detail |
|-------|--------|
| **Source** | Microphone input signal |
| **Stimulus** | Non-beat acoustic signal (ambient noise, vibration) enters the microphone alongside watch beats |
| **Artifact** | Signal Processing layer — LP/HP FilterChain → BeatDetector |
| **Environment** | Live mode; ambient noise present (office, workshop, handling noise) |
| **Response** | LP/HP filter rejects the non-beat signal; BeatDetector fires only on genuine T1/T3 events |
| **Measure** | False trigger rate < 1% under standard ambient conditions; true T1 detection rate > 99% |

## Warning Conditions (Usability Integration)

| Condition | Detection Method | GUI Message |
|-----------|-----------------|-------------|
| No signal | No beat event for N seconds | `⚠ No signal` — auto-cleared on recovery |
| Noisy signal | Beat event inter-arrival variance exceeds threshold | `⚠ Noisy signal` — auto-cleared on stabilization |

N values to be confirmed after EXP-05 (completed 06/17 — onset=0.08, min_peak=0.10 confirmed stable across 0–60 dB SNR).

## Related

- [QA Priority Summary](README.md)
- [ADR-003: Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md) — sample rate and Beat Error resolution
- [EXP-05: Detector Parameter Optimization](../experiments/exp-05-correctness-detector-optimization.md)
