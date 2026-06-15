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

Based on M1 feedback and EXP-02 results, we revised our QA priority:

| Priority | QA | Rationale |
|:--------:|----|-----------|
| **1** | **Modifiability** | Prerequisite for everything else. 11 graphs built in parallel requires a layer structure — without it, developers block each other. Also enables swapping tactics without touching other layers |
| **2** | **Real-Time + Low Latency** | EXP-02 confirmed 43% deadline miss on RPi. Largest structural change required. T2 + R1 validated on macOS — clear path to fix |
| **3** | **Usability** | Signal quality warnings, tab UX. Added after structure is solid. Optional for M2 |

**Trade-off accepted**: BPH coverage narrowed (focus on 28,800 BPH). Accuracy is 4th priority — we favor *finishing a working system* over maximizing precision within 5 weeks.

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
| NTR-05 | Single RPi device creates experiment bottleneck | EXP sequenced by dependency: EXP-02 macOS first → RPi R5 next. RPi-independent work (layer refactor, graph stubs, unit tests) ran on dev machine in parallel | ✅ Mitigated |
| NTR-07 | Domain knowledge gap — conceptual understanding of Rate/Beat Error/Amplitude is solid, but equation-level derivations and applied formulas remain difficult | Used AI assistance to interpret equations and derive test cases. 119 tests passing across 5 binaries (WatchMath, MeasurementEngine, Rolling*, GraphTabs) verify correctness independently of individual equation fluency. Layer isolation ensures DSP logic cannot be silently broken by architectural changes → [Unit Test Results](references/unit-test-results.md) | ✅ Mitigated — test suite is the safety net |
