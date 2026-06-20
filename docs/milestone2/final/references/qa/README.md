# Quality Attribute Requirements

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Governing Goal

**Measurement Accuracy, Error Detection, and Handling** is not one QA among equals — it is the criterion the entire architecture is evaluated against. Rate, Amplitude, and Beat Error values must match the WeiShi No.1000 reference device.

Every other QA is a structural prerequisite that makes accuracy achievable.

```
Goal: Measurement Accuracy, Error Detection, and Handling
├── Enabler:        Extensibility, Modifiability    → 11 graphs must exist before experiments can run;
│                                                    architecture changes must apply fast enough to meet schedule
├── Prerequisite:   Real Time Performance           → missed deadline = dropped beat = wrong Rate/BPH
├── Prerequisite:   Low Latency and Low Number      → late timestamp = wrong Beat Error / Amplitude
│                   of Missed Beats
└── Prerequisite:   Correctness                    → false trigger = wrong everything
```

---

## Priority Summary

| Priority | QA | Role | Rationale | M2 Status |
|:--------:|----|------|-----------|:---------:|
| **1** | [**Measurement Accuracy, Error Detection, and Handling**](qas-1-measurement-accuracy-error-detection-handling.md) | Why this system exists | Rate / Amplitude / Beat Error must match WeiShi reference. This is the criterion the entire architecture is evaluated against — all other QAs exist only to make this achievable | 🔶 RPi pending |
| **2** | [**Real Time Performance**](qas-2-real-time-performance.md) | Structural prerequisite | If the pipeline misses the 21ms deadline, beat events are dropped. Dropped events mean Rate and BPH cannot be calculated correctly. EXP-02 confirmed 43% miss — largest structural fix required | 🔶 macOS ✅, RPi R5 06/23 |
| **3** | [**Low Latency and Low Number of Missed Beats**](qas-3-low-latency-and-low-number-of-missed-beats.md) | Structural prerequisite | If capture→detect latency exceeds one beat period, T1/T3 event timestamps are wrong. Wrong timestamps corrupt Beat Error and Amplitude calculations | 🔶 macOS ✅, RPi R5 06/23 |
| **4** | [**Extensibility, Modifiability**](qas-4-extensibility-modifiability.md) | Execution enabler | 11 graph tabs must exist before experiments can run on them. Architecture decisions derived from experiments must be applied quickly enough to complete the project on schedule. Without a clean layer structure, both conditions fail | ✅ Verified |
| **5** | [**Correctness**](qas-5-correctness.md) | Signal-level prerequisite | LP/HP filtering removes ambient noise and false triggers. Without it, the detector fires on non-beat events — undermining accuracy regardless of pipeline performance | ⏳ EXP-03 06/25 |

**Change from M1**: M1 treated Accuracy as implicit and ranked Extensibility last. M2 promotes Measurement Accuracy, Error Detection, and Handling to explicit Priority 1 — it is the criterion graders evaluate the architecture against.
