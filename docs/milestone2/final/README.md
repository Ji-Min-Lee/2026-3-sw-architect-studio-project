# TimeGrapher — Milestone 2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

---

## Presentation

| # | Section | Document | Time |
|---|---------|----------|------|
| 1 | Wrap-up M1 & Intro | [slide-wrapup-intro.md](slide-wrapup-intro.md) | ~3 min |
| 2 | Architecture Views | [slide-architecture-view.md](slide-architecture-view.md) | ~12 min |
| 3 | Remaining Schedule | [slide-schedule.md](slide-schedule.md) | ~5 min |

---

## Traceability: QA → Risk → Experiment → Architecture Decision

> Includes items not covered in the presentation. Full design rationale at a glance.
>
> **★ = Covered in today's presentation (06/22)**

### QAS-1 — Measurement Accuracy *(Governing Goal)*

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| | [TR-05](references/risks.md) | LP/HP filter defaults block beat signal or pass ambient noise at edge BPH values | [EXP-03](references/experiments/exp-03-filter-tuning-noise-accuracy.md) | ⏳ 06/25 | — (ADR pending EXP-03 results) |
| ★ | [NTR-07](references/risks.md) | Team lacks domain knowledge of watch measurement formulas (Rate / Amplitude / Beat Error) | — | AI-generated unit tests for structural verification (119 tests / 5 binaries) | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) Observer — consistent Measurement delivery to all tabs |

### QAS-2 — Real-Time Performance

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| ★ | [TR-02](references/risks.md) | Single-threaded pipeline saturates cpu2 (91%); 43% deadline miss on RPi | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) | wait_ms 420ms → **0.013ms** (×32,000) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread ✅ |
| ★ | [TR-03](references/risks.md) | Qt QueuedConnection accumulates 420ms backlog; frame queue grows unbounded | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) | Backlog 0% (macOS + RPi E2-5/6) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread ✅ |
| | [TR-04](references/risks.md) | `replot()` consumes 79% of exec budget (16ms / 21ms); scales with number of tabs | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) / [EXP-05](references/experiments/exp-05-rendering-realtime-impact.md) | replot/beat 8.22 → **1.20** (↓85%) macOS | [ADR-002](references/adr/ADR-002-r1-lazy-rendering.md) R1 Lazy Rendering ✅ · [ADR-004](references/adr/ADR-004-r2-timer-decoupled-rendering.md) R2 conditional ⏳ |
| ★ | [TR-10](references/risks.md) 🔴 | Qt FG event loop picks up frameLogged signal avg 60.1ms late (84% > 21ms deadline) | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) E2-7 | 🔴 Unresolved | E2-8: SCHED_RR / QTimer polling — scheduled 06/22 |

### QAS-3 — Low Latency

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| | [TR-01](references/risks.md) | RPi 5 cannot sustain 96kHz audio capture without block drops while Qt GUI runs | [EXP-01](references/experiments/exp-01-high-res-sampling-beat-error.md) | Dropped=0 at 48k/96k/192k · SCHED_RR not required | [ADR-003](references/adr/ADR-003-sample-rate-selection.md) 96kHz Accepted ✅ |
| ★ | TR-02/03 | *(shared with QAS-2)* Single-threaded capture-to-process latency | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) | E2E avg **2.05ms** on RPi | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 + AudioRingBuffer ✅ |

### QAS-4 — Extensibility / Modifiability

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| ★ | [TR-06](references/risks.md) | Layer refactoring introduces regression in existing DSP behavior | — | 116 unit tests (7 binaries) all passing ✅ | 4-Layer Allowed-to-Use structure enforced |
| ★ | [TR-07](references/risks.md) | Residual cross-layer coupling survives refactoring | — | Compiler catches upward dependency ✅ | Allowed-to-use rule + per-layer include restriction |
| ★ | [TR-08](references/risks.md) | New graph tab requires data not in current `MeasurementEngine` output | — | All 11-tab data requirements covered by current Domain output ✅ | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) BaseGraphTab Observer |
| ★ | — | `MainWindow` directly coupled to concrete audio workers; adding a source requires 3+ file changes | — | Adding `NetworkWorker` reduced to ≤ 2 files | [ADR-005](references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) IAudioSource Dependency Inversion ✅ |

### QAS-5 — Correctness

| | Risk | Description | Experiment | Result | Architecture Decision |
|--|------|-------------|-----------|--------|-----------------------|
| ★ | [NTR-07](references/risks.md) | Formula knowledge gap makes structural correctness of tab implementations hard to verify | — | AI-generated unit tests — structural verification without domain expertise | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) — all tabs receive identical `Measurement` |

---

## Architecture Views

| View | Type | Primary QA | Document |
|------|------|------------|----------|
| C&C: DSP Pipeline Thread Model | C&C / Runtime | QAS-2, QAS-3 | [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) |
| Layered: 4-Layer Allowed-to-Use | Module — Layered | QAS-4 | [view-layered-4layer.md](references/views/view-layered-4layer.md) |
| Decomposition: Graph Tab | Module — Decomposition | QAS-4 | [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md) |
| Module: IAudioSource Dependency Inversion | Module — Decomposition | QAS-4 (Extensibility) | [view-iaudiosource.md](references/views/view-iaudiosource.md) |
| Deployment: Build-Deploy Pipeline | Deployment | Deployability | [view-deployment-build-pipeline.md](references/views/view-deployment-build-pipeline.md) |

---

## Document Structure

```
docs/milestone2/final/
├── README.md                         ← this file — full traceability map
├── slide-wrapup-intro.md             ← Section 1: M1 feedback + ARS overview
├── slide-architecture-view.md        ← Section 2: Latency / Correctness / Extensibility / Risk
├── slide-schedule.md                 ← Section 3: sprint recap + M2→Final schedule
├── assets/                           ← view diagrams (.drawio + .png)
└── references/
    ├── qa/                           ← QA scenario files (qas-1 ~ qas-5)
    ├── risks.md                      ← full risk register
    ├── views/                        ← Merson 7-section template view files
    ├── experiments/                  ← experiment files (EXP-01 ~ EXP-05)
    └── adr/                          ← ADR files (ADR-001 ~ ADR-006)
```
