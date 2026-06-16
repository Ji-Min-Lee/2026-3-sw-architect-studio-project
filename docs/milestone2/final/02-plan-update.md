# Project Plan Update

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## M1 Feedback: What We Fixed

| Area | M1 Feedback | M2 Response |
|------|-------------|-------------|
| **Project Plan** | No owner/date per task. Experiments not tracked as tasks. No Kanban link. ADD diagram source unlabeled | All tasks now have owner + target date. Experiments tracked on board. Sources labeled → [GitHub Project](references/github-project-status.md) |
| **Architectural Drivers** | Tactics mixed into QA doc. "Provisional" QAs. Solution language crept in | QAs describe problem only (no solution). Tactics moved to Approaches doc. Provisional removed — numbers confirmed by experiment |
| **Experiments** | Only 1 of 9 risks linked to experiments. No experiments started — red flag | EXP-02 completed. Each experiment explicitly maps to Risk ID(s) it resolves |
| **Navigation** | No README. No cross-document links | README.md written. Each doc has cross-links at top |
| **Architecture Diagrams** | Overly detailed in M1. Clarity needed | Diagrams simplified. Legend added. One diagram per view |

---

## Revised QA Priority

**The governing goal of this project is Measurement Accuracy** — Rate, Amplitude, and Beat Error values must match the WeiShi No.1000 reference device. Every other QA in this system is a structural prerequisite for achieving that goal.

```
Goal: Measurement Accuracy
├── Prerequisite 1: Real-Time Performance  → missed deadline = dropped beat event = wrong Rate/BPH
├── Prerequisite 2: Low Latency            → late timestamp = wrong Beat Error / Amplitude
├── Prerequisite 3: Signal Quality (Noise) → false detection = wrong everything
└── Enabler: Modifiability                 → 11 graphs in parallel without structural blocking
```

| Priority | QA | Role | Rationale |
|:--------:|----|------|-----------|
| **Goal** | **Measurement Accuracy** | Why this system exists | Rate / Amplitude / Beat Error must match WeiShi reference. This is the criterion the architecture is evaluated against |
| **1** | **Real-Time Performance** | Structural prerequisite | If the pipeline misses the 21ms deadline, beat events are dropped. Dropped events mean Rate and BPH cannot be calculated correctly. EXP-02 confirmed 43% miss — largest structural fix required |
| **2** | **Low Latency** | Structural prerequisite | If capture→detect latency exceeds one beat period, T1/T3 event timestamps are wrong. Wrong timestamps corrupt Beat Error and Amplitude calculations |
| **3** | **Signal Quality / Noise** | Signal-level prerequisite | LP/HP filtering removes ambient noise and false triggers. Without it, the detector fires on non-beat events — undermining accuracy regardless of pipeline performance |
| **4** | **Modifiability** | Execution enabler | Clean layer structure enables 11 graph tabs to be built in parallel by two teams, and allows filter/detection parameters to be swapped without touching other layers |

**Trade-off accepted**: BPH coverage narrowed to 28,800 BPH for M3. Full BPH range (18,000–36,000) is an accuracy stretch goal, not a structural constraint.

---

## Risk Resolution Status

### Technical Risks

| ID | Description | EXP | Resolution | Status |
|----|-------------|-----|------------|--------|
| TR-01 | RPi sample rate — Dropped Block = 0 at 96kHz | EXP-01 | EXP-01 running on RPi | ⏳ In progress |
| **TR-02/03** | **Real-time deadline miss + latency on RPi** | **EXP-02** | **T2 (DSP Offload) + R1 (Lazy Rendering) validated on macOS. backlog 47%→0%, wait_ms ×32,000 reduction** | **✅ macOS resolved. RPi R5 next** |
| TR-04 | 11-tab rendering pushes process→display > 30ms | EXP-05 | QueuedConnection separates render from exec path | ⏳ RPi measurement pending |
| TR-05 | Detector default params fail under ambient noise | EXP-03 | Adaptive threshold implemented; parameter sweep pending | ⏳ Pending |
| TR-06/07 | Residual coupling / regression after layer refactoring | — | 4-layer structure enforced. 116 unit tests (7 binaries) all passing — regression baseline secured | ✅ Resolved |
| TR-08 | New graph needs data not in MeasurementEngine | — | 11-graph data requirements reviewed; all covered by current Domain output | ✅ Resolved |
| TR-09 | Signal quality warning thresholds mismatched | — | Heartbeat pattern implemented; threshold tunable via single parameter | ✅ Resolved |

### Non-Technical Risks

| ID | Description | Resolution | Status |
|----|-------------|------------|--------|
| NTR-04 | English communication overhead | All deliverables written in English. Internal communication Korean; design decision summaries recorded bilingually | ✅ Ongoing — convention held |
| NTR-05 | Single RPi device creates experiment bottleneck | EXP sequenced by dependency: EXP-02 validated on each team member's laptop (Windows + macOS) first → RPi next. RPi-independent work (layer refactor, graph stubs, unit tests) ran on dev machines in parallel. Laptop results established confidence before consuming the single RPi slot | ✅ Mitigated |
| NTR-07 | Domain knowledge gap — conceptual understanding of Rate/Beat Error/Amplitude is solid, but equation-level derivations and applied formulas remain difficult | Used AI assistance to interpret equations and derive test cases. 119 tests passing across 5 binaries (WatchMath, MeasurementEngine, Rolling*, GraphTabs) verify correctness independently of individual equation fluency. Layer isolation ensures DSP logic cannot be silently broken by architectural changes → [Unit Test Results](references/unit-test-results.md) | ✅ Mitigated — test suite is the safety net |
