# TimeGrapher — Milestone 2 Presentation

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Agenda

| # | Section |
|---|---------|
| 1 | M1 Feedback & Improvements |
| 2 | Architectural Approach Overview |
| 3 | Risk → Experiment → Architecture Decision |
| 4 | Remaining Issues |
| 5 | Schedule |

---

## 1. M1 Feedback & Improvements

### What We Fixed

| Area | M1 Issue | M2 Fix |
|------|----------|--------|
| Project Plan | No owner/date. Experiments not tracked. No Kanban link | Owner + date on every task. Experiments as GitHub issues → [Board](references/github-project-status.md) |
| Architecture Diagrams | Source unlabeled. Too detailed | One diagram per view, legend added, source labeled |
| Architectural Drivers | Tactics mixed into QA doc. Provisional numbers. Solution language | QAs describe problem only. Tactics moved to Approaches doc. Numbers confirmed by EXP-02 |
| Experiments | 1 of 9 risks linked. None started | EXP-02 complete. Every experiment maps to Risk ID(s) |
| Navigation | No README. No cross-links | README written. All docs cross-linked |

---

### QA Priority — Accuracy as the Governing Goal

> Accuracy is not one QA among equals. It is the criterion the architecture is evaluated against.  
> Modifiability is elevated to Priority 1 — 11 graphs must exist before experiments can run, and architecture decisions must apply fast enough to stay on schedule.

```
Goal: Measurement Accuracy
├── Enabler 1:      Modifiability          → 11 graphs must exist before experiments can run;
│                                            architecture changes must apply fast to meet schedule
├── Prerequisite 2: Real-Time Performance  → missed deadline = dropped beat = wrong Rate/BPH
├── Prerequisite 3: Low Latency            → late timestamp = wrong Beat Error / Amplitude
└── Prerequisite 4: Reliability → false trigger = wrong everything
```

→ Detail: [references/qa.md](references/qa.md)

---

## 2. Architectural Approach Overview

### Two Structural Pillars

The architecture rests on two decisions, both forced by EXP-02 evidence (43% deadline miss on RPi):

| Pillar | What | Why |
|--------|------|-----|
| **4-Layer Allowed-to-Use** | Acquisition → Signal Processing → Domain → Presentation; downward-only dependencies | Modifiability: new tab = 1 class, 0 Domain changes |
| **Explicit Concurrency (T2 + R1)** | DSP on dedicated thread; render skipped for non-visible tabs | Real-Time Performance: eliminates single-core saturation and exec path coupling |

### View Selection Rationale

Views are documented only when needed and useful to a specific reader (Merson principle).

| View | Type | Question Answered | Primary Reader |
|------|------|-------------------|----------------|
| 4-Layer Allowed-to-Use | Module — Layered | Which direction do dependencies flow? Which layer changes when a new tab is added? | Developers (11 tabs in parallel) |
| Graph Tab Decomposition | Module — Decomposition | What is the internal structure of Presentation? How is a new tab added? | Developers (per-tab ownership) |
| DSP Pipeline Thread Model | Runtime / C&C | How do the two threads cooperate? Where do T2 and R1 operate at runtime? | Developers (performance / concurrency) |
| RPi 5 Hardware Deployment | Deployment / Allocation | Which hardware node runs which software? What is the deploy path? | Operations, hardware owners |

---

### View 1 — 4-Layer Allowed-to-Use (Module View)

> Which direction do dependencies flow? Which layer changes when a new tab is added?

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

**Rule**: Presentation → Domain only. New graph tab = one class in Presentation. Zero changes below.

---

### View 2 — Graph Tab Decomposition (Module — Zoom)

> What is the internal structure of Presentation? How is a new tab added?

![Graph Tab Decomposition View](assets/view2-decomposition.png)

**Rule**: Implement `IGraphTab` + register in `GraphTabManager`. No other files touched.

---

### View 3 — DSP Pipeline Thread Model (Runtime / C&C View)

> How do the two threads cooperate? Where do T2 and R1 operate at runtime?

![DSP Pipeline Thread Model](assets/view3-thread-model.png)

**T2** (ADR-001): AudioCapture → ring buffer → DSPWorker (separate thread) → QueuedConnection → UI  
**R1** (ADR-002): `isVisible()` guard in `updateData()` — non-visible tabs skip `replot()`

---

### View 4 — Raspberry Pi 5 Deployment View

> Which hardware runs which software? What is the deploy path?

![RPi 5 Deployment View](assets/view4-deployment.png)

**Constraint**: AGC must be disabled on every RPi boot (`alsamixer`) — AGC on → Amplitude / Beat Error unreliable.

---

## 3. Risk → Experiment → Architecture Decision

---

### Case 1 — Single-Core Saturation

| | |
|-|-|
| **Risk** | TR-02/03: RPi cpu2 at 91%; Qt event loop coupling causes 420ms backlog; 43% deadline miss |
| **Experiment** | [EXP-02](references/experiments/exp-02-pipeline-latency.md): wait_ms 420ms → **0.013ms** (×32,000); backlog 47% → **0%** after T2 |
| **Decision** | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md): Introduce `DSPWorker` thread — AudioCapture writes to ring buffer; DSP runs on separate core |
| **Status** | ✅ Accepted — macOS validated. RPi R5: 06/23 |

---

### Case 2 — Rendering Bottleneck in Exec Path

| | |
|-|-|
| **Risk** | TR-04: `replot()` in exec path = 16ms (79% of 21ms budget); scales with N active tabs |
| **Experiment** | [EXP-02](references/experiments/exp-02-pipeline-latency.md): replot_count 8.22 → **2.08** (↓75%) / **1.20** (↓85%) after R1 guard |
| **Decision** | [ADR-002](references/adr/ADR-002-r1-lazy-rendering.md): `isVisible()` guard in each tab's `updateData()`; `showEvent()` catch-up frame |
| **Status** | ✅ Accepted — macOS validated. RPi impact: ~14ms plot_ms saving expected |

---

### Case 3 — RPi Sample Rate Ceiling (Pending)

| | |
|-|-|
| **Risk** | TR-01: RPi 5 may not sustain 96kHz block-drop-free while Qt GUI runs |
| **Experiment** | [EXP-01](references/experiments/exp-01-sample-rate.md): macOS ✅ 96kHz, 0 dropped. RPi ⏳ target 06/23 |
| **Decision** | ADR-003: pending EXP-01 RPi result. Fallback: 48kHz (Beat Error resolution: 10.4µs → 20.8µs) |
| **Status** | ⏳ Decision deferred |

---

## 4. Remaining Issues

### Open Experiments

| ID | Experiment | Target | Blocks |
|----|------------|:------:|--------|
| [EXP-01](references/experiments/exp-01-sample-rate.md) | RPi sample rate (96kHz, 0 dropped blocks) | 06/23 | ADR-003 |
| EXP-02 R5 | T2+R1 on RPi — confirm deadline miss resolved | 06/23 | Phase A go/no-go |
| EXP-02 R6 | T1 SCHED_RR on RPi — thermal throttle mitigation | 06/24 | ADR-003 supplement |
| [EXP-03](references/experiments/exp-03-filter-sweep.md) | LP/HP filter parameter sweep | 06/25 | Phase A task A-02 |
| [EXP-05](references/experiments/exp-05-rendering-fps.md) | Qt 11-tab FPS on RPi | 06/26 | ADR-002 confirmation |

### Unresolved Critical Concerns

| Concern | Plan |
|---------|------|
| T2+R1 unconfirmed on RPi | EXP-02 R5 — 06/23 |
| Thermal throttle (85°C) not mitigated | SCHED_RR affinity EXP-02 R6 — 06/24 |
| Filter cutoffs undetermined | EXP-03 — 06/25 |

→ Full risk register: [references/risks.md](references/risks.md)

---

## 5. Schedule

```
06/22 (Mon) — M2 submission  [All 11 graph tabs already implemented ✅]
06/23 (Tue) — M2 feedback + EXP-01 RPi (96kHz ceiling) + EXP-02 R5 (T2+R1 on RPi)
06/24 (Wed) — M2 feedback addressed + EXP-02 R6 (SCHED_RR) + ADR-003 finalized
06/25 (Thu) — EXP-03 filter sweep → ADR-003 supplement (filter cutoffs confirmed)
06/26 (Fri) — EXP-05 (11-tab FPS on RPi) → ADR-004 decision (R1 sufficient or R2 adopted)
06/29 (Mon) — RPi integration + WeiShi accuracy validation (Phase A goal)
06/30 (Tue) — Demo rehearsal + E2E latency documented
07/01 (Wed) — M3 FINAL DEMO
```

### Phase Priority

| Phase | Scope | Status | Must? |
|-------|-------|:------:|:-----:|
| Graphs | All 11 graph tabs (Trace / Vario / BeatError / Sequence / BeatNoiseScope / LongTerm / Escapement / Spectrogram / WaveformComp / SweepScope / FilterScope) | ✅ Complete | ✅ |
| A | Core pipeline: capture → DSP → WeiShi accuracy validation on RPi | ⏳ 06/29 | ✅ |
| Experiments | EXP-01/02/03/05 on RPi — confirm ADR-003 sample rate, ADR-002/004 rendering | ⏳ 06/23–06/26 | ✅ |
| Demo | E2E latency documented + rehearsal | ⏳ 06/30 | ✅ |

**All graph tabs are implemented. Remaining critical path: RPi experiments → WeiShi validation → demo.**

---

## M2 Deliverable Status

| Deliverable | Status |
|-------------|:------:|
| Updated Project Plan | ✅ |
| Experiment Results (EXP-02 complete) | ✅ |
| Architecture — Module View (×2) | ✅ |
| Architecture — Runtime / C&C View | ✅ |
| Architecture — Deployment View | ✅ |
| Construction Plan | ✅ |
