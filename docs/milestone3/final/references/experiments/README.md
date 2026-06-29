# Experiments — TimeChecker

All 8 experiments completed. Each experiment file contains its objective, run history, results, and architecture decisions.

---

## Experiment Index

| ID | Experiment | QA | ADR | Status | Date |
|----|------------|----|-----|:------:|------|
| [EXP-01](exp-01-realtime-dropped-block.md) | RPi Real-Time Performance — Dropped Block Measurement | QAS-1 | [ADR-003](../adr/ADR-003-sample-rate-selection.md) | ✅ Done | 2026-06-15 |
| [EXP-02](exp-02-latency-e2e.md) | End-to-End Latency — 2-Segment Timestamp Measurement | QAS-2 | [ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md), [ADR-002](../adr/ADR-002-r1-lazy-rendering.md) | ✅ Done | 2026-06-11~16 |
| [EXP-03](exp-03-extensibility-observer-pattern.md) | Observer Pattern Compliance — Tab Extension Cost Measurement | QAS-3 | [ADR-006](../adr/ADR-006-basegraphtab-observer-pattern.md), [ADR-005](../adr/ADR-005-p1-iaudiosource-dependency-inversion.md) | ✅ Done | 2026-06-21 |
| [EXP-04](exp-04-correctness-detector-optimization.md) | Detector Parameter Optimization Under Noise | QAS-4 | — | ✅ Done | 2026-06-16~17 |
| [EXP-05](exp-05-noise-threshold-popup.md) | Signal Quality Warning — Ambient Noise Threshold Validation | QAS-4 + Usability | — | ✅ Done | 2026-06-23 |
| [EXP-06](exp-06-accuracy-witschi-comparison.md) | Witschi Accuracy Comparison — TimeChecker vs Witschi No.1000 | QAS-5 | — | ✅ Done | 2026-06-25 |
| [EXP-07](exp-07-longterm-aging.md) | Long-Term Aging Test — Bucket Downsampling Efficiency | QAS-6 | [ADR-007](../adr/ADR-007-longtermtab-downsampling.md) | ✅ Done | 2026-06-25 |
| [EXP-08](exp-08-tab-expansion-file-change-cost.md) | Tab Expansion File-Change Cost | QAS-3 | [ADR-006](../adr/ADR-006-basegraphtab-observer-pattern.md) | ✅ Done | 2026-06-21 |

> **Dependency order**: EXP-01 → EXP-02 → EXP-04 → EXP-05 → EXP-06. EXP-03 / EXP-08 are independent. EXP-07 is independent (analytical).

---

## QA Traceability

| QA Scenario | Experiments |
|-------------|-------------|
| QAS-1 — Real-Time Audio Capture | [EXP-01](exp-01-realtime-dropped-block.md) |
| QAS-2 — End-to-End Latency | [EXP-02](exp-02-latency-e2e.md) |
| QAS-3 — Modifiability (Tab Extension) | [EXP-03](exp-03-extensibility-observer-pattern.md), [EXP-08](exp-08-tab-expansion-file-change-cost.md) |
| QAS-4 — Correctness Under Noise | [EXP-04](exp-04-correctness-detector-optimization.md), [EXP-05](exp-05-noise-threshold-popup.md) |
| QAS-5 — Measurement Accuracy | [EXP-06](exp-06-accuracy-witschi-comparison.md) |
| QAS-6 — Long-Term Operation | [EXP-07](exp-07-longterm-aging.md) |
