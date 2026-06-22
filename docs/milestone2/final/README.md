# TimeGrapher — Milestone 2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

> **★ = Covered in today's presentation (06/22)**

---

## Presentation

| # | Section | Document | Time |
|---|---------|----------|------|
| 1 | Milestone 1 Wrap-up & Intro | [1-slide-m1-wrapup-intro.md](1-slide-m1-wrapup-intro.md) | ~3 min |
| 2 | Architecture Views | [2-slide-architecture-view.md](2-slide-architecture-view.md) | ~12 min |
| 3 | Remaining Schedule | [3-slide-risk-and-schedule.md](3-slide-risk-and-schedule.md) | ~5 min |

---

## Traceability: QA → Risk → Experiment → Architecture Decision

> Includes items not covered in the presentation. Full design rationale at a glance.

### QAS-1 — Measurement Accuracy *(Governing Goal)*

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| | [TR-05](references/risks.md) | Filter configuration may degrade beat detection accuracy at edge BPH values | [EXP-05](references/experiments/exp-05-correctness-detector-optimization.md) | ⏳ 06/25 | — (ADR pending EXP-05 results) |
| ★ | [NTR-07](references/risks.md) | Domain knowledge gap prevents structural verification of measurement tab correctness | — | AI-generated unit tests for structural verification (119 tests / 5 binaries) | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) Observer — consistent Measurement delivery to all tabs |

### QAS-2 — Real-Time Performance

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| ★ | [TR-02](references/risks.md) | Single-threaded design misses real-time beat processing deadline on RPi | [EXP-03](references/experiments/exp-03-latency-e2e.md) | wait_ms 420ms → **0.013ms** (×32,000) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread ✅ |
| ★ | [TR-03](references/risks.md) | Signal backlog accumulates unbounded under single-threaded load | [EXP-03](references/experiments/exp-03-latency-e2e.md) | Backlog 0% (macOS + RPi E2-5/6) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread ✅ |
| | [TR-04](references/risks.md) | Rendering cost per beat consumes most of the real-time budget | [EXP-03](references/experiments/exp-03-latency-e2e.md) | replot/beat 8.22 → **1.20** (↓85%) macOS | [ADR-002](references/adr/ADR-002-r1-lazy-rendering.md) R1 Lazy Rendering ✅ · [ADR-004](references/adr/ADR-004-r2-timer-decoupled-rendering.md) R2 conditional ⏳ |
### QAS-3 — Low Latency

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| | [TR-01](references/risks.md) | RPi may not sustain high-resolution audio capture alongside Qt GUI | [EXP-02](references/experiments/exp-02-realtime-dropped-block.md) | Dropped=0 at 48k/96k/192k · SCHED_RR not required | [ADR-003](references/adr/ADR-003-sample-rate-selection.md) 96kHz Accepted ✅ |
| ★ | TR-02/03 | *(shared with QAS-2)* Single-threaded capture-to-process latency | [EXP-03](references/experiments/exp-03-latency-e2e.md) | E2E avg **2.05ms** on RPi | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 + AudioRingBuffer ✅ |

### QAS-4 — Extensibility / Modifiability

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| ★ | [TR-06](references/risks.md) | Layer refactoring may introduce regression in existing DSP behavior | — | 116 unit tests (7 binaries) all passing ✅ | 4-Layer Allowed-to-Use structure enforced |
| ★ | [TR-07](references/risks.md) | Residual coupling may survive refactoring and reintroduce layer violations | — | Compiler catches upward dependency ✅ | Allowed-to-use rule + per-layer include restriction |
| ★ | [TR-08](references/risks.md) | New tabs may require measurement data not available in current domain output | — | All 11-tab data requirements covered by current Domain output ✅ | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) BaseGraphTab Observer |
| ★ | — | Audio source extension requires changes across multiple unrelated components | — | Adding `NetworkWorker` reduced to ≤ 2 files | [ADR-005](references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) IAudioSource Dependency Inversion ✅ |

### QAS-5 — Correctness

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| ★ | [NTR-07](references/risks.md) | Domain knowledge gap makes correctness of tab implementations hard to verify | — | AI-generated unit tests — structural verification without domain expertise | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) — all tabs receive identical `Measurement` |

---

## Architecture Views

| | View | Type | Primary QA | Document |
|--|------|------|------------|----------|
| ★ | C&C: DSP Pipeline Thread Model | C&C / Runtime | QAS-2, QAS-3 | [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) |
| ★ | Layered: 4-Layer Allowed-to-Use | Module — Layered | QAS-4 | [view-layered-4layer.md](references/views/view-layered-4layer.md) |
| | Decomposition: Graph Tab | Module — Decomposition | QAS-4 | [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md) |
| ★ | Module: IAudioSource Dependency Inversion | Module — Decomposition | QAS-4 (Extensibility) | [view-iaudiosource.md](references/views/view-iaudiosource.md) |
| | Deployment: Build-Deploy Pipeline | Deployment | Deployability | [view-deployment-build-pipeline.md](references/views/view-deployment-build-pipeline.md) |

---

## Architecture Evaluation

The M2 architecture was evaluated with **ATAM** (Architecture Tradeoff Analysis Method) — see [atam-evaluation-v3.md](references/atam-evaluation-v3.md).

---

## Document Structure

```
docs/milestone2/final/
├── README.md                         ← this file — full traceability map
├── 1-slide-m1-wrapup-intro.md             ← Section 1: M1 feedback + ARS overview
├── 2-slide-architecture-view.md        ← Section 2: Latency / Correctness / Extensibility / Risk
├── 3-slide-risk-and-schedule.md                 ← Section 3: sprint recap + M2→Final schedule
├── assets/                           ← view diagrams (.drawio + .png)
└── references/
    ├── qa/                           ← QA scenario files (qas-1 ~ qas-5)
    ├── risks.md                      ← full risk register
    ├── views/                        ← Merson 7-section template view files
    ├── experiments/                  ← experiment files (EXP-01 ~ EXP-05)
    └── adr/                          ← ADR files (ADR-001 ~ ADR-006)
```
