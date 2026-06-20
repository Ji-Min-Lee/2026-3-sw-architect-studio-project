# Architecture Views

← [Wrap-up & Intro](slide-wrapup-intro.md) | [Presentation Index](README.md) | Next: [Schedule →](slide-schedule.md)

---

## 3. Architectural Approach Overview

### Documentation Framework

Architecture views follow the **Merson 7-section template** (Primary Presentation, Element Catalog, Behavior, Context Diagram, Variability Guide, Design Rationale, Related Views). Views are documented only when needed and useful to a specific reader.

Each view file links to the QA scenario, experiment, and ADR that motivated it:

```
View document
  ├── Primary Presentation (diagram)
  ├── Element Catalog
  ├── Behavior
  ├── Design Rationale → ADR (decision + trade-off)
  └── Related Views → other view files
        ↑
        linked from QA scenario (references/qa.md)
        linked from Experiment (references/experiments/)
```

### View Selection Rationale

| View | Type | QA Addressed | Experiment | ADR |
|------|------|--------------|-----------|-----|
| [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) | Allocation | Deployability | — | — |
| [Layered: 4-Layer Allowed-to-Use](references/views/view-layered-4layer.md) | Module — Layered | QAS-3 Modifiability | — | ADR-001, ADR-002 |
| [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) | Module — Decomposition | QAS-3 Modifiability | — | ADR-002 |
| [C&C: DSP Pipeline Thread Model](references/views/view-cc-dsp-pipeline.md) | Runtime / C&C | QAS-0/1/2 Accuracy / Real-Time / Latency | [EXP-02](references/experiments/exp-02-pipeline-latency.md) | ADR-001, ADR-002 |
| [Module: IAudioSource Dependency Inversion](references/views/view-iaudiosource.md) | Module — Decomposition | QAS-3 Extensibility | — | ADR-P1 |

---

### 3-A: On-Schedule Delivery

#### View 1 — [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md)

![RPi 5 Deployment View](assets/view4-deployment.png)

Dev machine → build + test (macOS) → `git push` → RPi → `git pull` → build + experiment.  
Structural validation stays on macOS; RPi reserved for hardware experiments and demo.

#### View 2 — [Layered: 4-Layer Allowed-to-Use](references/views/view-layered-4layer.md)

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

Dependencies flow downward only: Acquisition → Signal Processing → Domain → Presentation.  
New graph tab = ≤ 3 files in Presentation only. Zero changes to Domain or below.

#### View 3 — [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md)

![Graph Tab Decomposition View](assets/view2-decomposition.png)

Implement `IGraphTab` + register in `GraphTabManager`. No other files touched.  
**Evidence**: All 11 graph tabs implemented ✅ by 06/22.

---

### 3-B: Accuracy

> Both decisions below were forced by [EXP-02](references/experiments/exp-02-pipeline-latency.md) evidence — 43% deadline miss on RPi under the original single-threaded design.

#### View 4 — [C&C: DSP Pipeline Thread Model](references/views/view-cc-dsp-pipeline.md)

![DSP Pipeline Thread Model](assets/view3-thread-model.png)

| Risk | Experiment | Decision | Key Result |
|------|-----------|----------|------------|
| TR-02/03: 420ms backlog, 43% deadline miss | [EXP-02](references/experiments/exp-02-pipeline-latency.md) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread | wait_ms: 420ms → **0.013ms** (×32,000). Backlog → **0%** |
| TR-04: `replot()` = 16ms (79% of budget) | [EXP-02](references/experiments/exp-02-pipeline-latency.md) | [ADR-002](references/adr/ADR-002-r1-lazy-rendering.md) R1 Lazy Rendering | replot/beat: 8.22 → **1.20** (↓85%) |
| TR-01: RPi 96kHz sustainability | [EXP-01](references/experiments/exp-01-sample-rate.md) | [ADR-003](references/adr/ADR-003-sample-rate-selection.md) Accepted | Dropped=0 at 48k/96k/192k. 96kHz selected. |

---

### 3-C: Extensibility (QAS-3)

#### View 5 — [Module: IAudioSource Dependency Inversion](references/views/view-iaudiosource.md)

![IAudioSource Dependency Inversion](assets/view5-iaudiosource.png)

| | AS-IS | TO-BE (P1 + i1 refactors) |
|---|-------|--------------------------|
| Extension point | Modify `MainWindow` + `AudioManager` + `DSPWorker` | Implement `IAudioSource` only |
| `connect()` blocks | 3 (one per concrete worker type) | 1 (via interface) |
| Adding `NetworkWorker` | 3+ file changes | **≤ 2 files** |

`SessionController` (i1 refactor) extracted from `MainWindow`; now depends on `IAudioSource` interface only. `AudioManager` removed (i6 — dead code).
