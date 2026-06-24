# Quality Attribute Requirements

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Governing Goal

**Real Time Performance** is the top structural priority — a missed 21ms deadline drops a beat event, which directly corrupts Rate, BPH, and downstream calculations. All other QAs either enable or depend on reliable real-time throughput.

```
Goal: Real Time Performance
├── Prerequisite:   Low Latency and Low Number      → late timestamp = wrong Beat Error / Amplitude
│                   of Missed Beats
├── Enabler:        Extensibility, Modifiability    → 11 graphs must exist before experiments can run;
│                                                    architecture changes must apply fast enough to meet schedule
├── Prerequisite:   Correctness                    → false trigger = wrong everything
└── Usability:      Measurement Accuracy,           → user-facing accuracy display; values must match
                    Error Detection, and Handling    WeiShi No.1000 reference within tolerance
```

---

## Priority Summary

| Priority | QA | Role | Rationale | M2 Status |
|:--------:|----|------|-----------|:---------:|
| **1** | [**Real Time Performance**](qas-1-real-time-performance.md) | Structural prerequisite | If the pipeline misses the 21ms deadline, beat events are dropped. Dropped events mean Rate and BPH cannot be calculated correctly. EXP-02 confirmed 43% miss — largest structural fix required | 🔶 macOS ✅, RPi R5 06/23 |
| **2** | [**Low Latency and Low Number of Missed Beats**](qas-2-low-latency-and-low-number-of-missed-beats.md) | Structural prerequisite | If capture→detect latency exceeds one beat period, T1/T3 event timestamps are wrong. Wrong timestamps corrupt Beat Error and Amplitude calculations | 🔶 macOS ✅, RPi R5 06/23 |
| **3** | [**Extensibility, Modifiability**](qas-3-extensibility-modifiability.md) | Execution enabler | 11 graph tabs must exist before experiments can run on them. Architecture decisions derived from experiments must be applied quickly enough to complete the project on schedule. Without a clean layer structure, both conditions fail | ✅ Verified |
| **4** | [**Correctness**](qas-4-correctness.md) | Signal-level prerequisite | LP/HP filtering removes ambient noise and false triggers. Without it, the detector fires on non-beat events — undermining accuracy regardless of pipeline performance | ⏳ EXP-03 06/25 |
| **5** | [**Measurement Accuracy, Error Detection, and Handling**](qas-5-measurement-accuracy-error-detection-handling.md) | Usability | Rate / Amplitude / Beat Error must match WeiShi reference and be legible to the user. All structural QAs (1–4) are prerequisites that make this user-facing outcome achievable | 🔶 RPi pending |

**Change from M2**: M2 treated Measurement Accuracy as Priority 1 / governing goal. M3 reclassifies it as a **usability** concern (Priority 5) — the criterion users observe — and promotes Real Time Performance to Priority 1 as the structural prerequisite that everything else depends on.
