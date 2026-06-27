# Architecture Views — TimeGrapher

Five views covering Latency, Correctness, Extensibility, and Deployability.  
Each view follows the **Merson 7-section template** and targets a specific QA.

---

## [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md)

> QA: Real-time Performance · Low Latency (QAS-1, QAS-2)

[![DSP Pipeline Thread Model](../../assets/view3-thread-model-simple.png)](view-cc-dsp-pipeline.md)

[![E2E Latency Comparison](../../assets/thread-latency-chart.png)](view-cc-dsp-pipeline.md)

---

## [Layered and Module Decomposition View](view-layered-4layer.md)

> QA: Extensibility / Modifiability (QAS-3)

[![Layered and Module Decomposition](../../assets/view1-layered-module.png)](view-layered-4layer.md)

---

## [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md)

> QA: Extensibility / Modifiability (QAS-3)

[![IAudioSource Dependency Inversion](../../assets/view5-iaudiosource.png)](view-iaudiosource.md)

---

## [Graph Tab Module Uses View](view-decomposition-graph-tab.md)

> QA: Correctness · Extensibility (QAS-4, QAS-3)

[![Graph Tab Module Uses View](../../assets/view2-decomposition.png)](view-decomposition-graph-tab.md)

[![Observer Module](../../assets/view2b-observer-module.png)](view-decomposition-graph-tab.md)

---

## [Deployment View: Build-Deploy Pipeline](view-deployment-build-pipeline.md)

> QA: Deployability

[![Build-Deploy Pipeline](../../assets/view4-deployment.png)](view-deployment-build-pipeline.md)

---

## [Deployment View: Local Pre-commit Unit Test Gate and Manual Deploy Path](view-allocation-implementation.md)

> QA: Correctness (QAS-4) · Deployability · Current-state gate view

Current repository facts only: shared local `pre-commit` gate runs `TestWatchMath` and `TestMeasurementEngine` before commit, then push/deploy remains manual. No GitHub workflow is described because `.github/workflows/` is absent in the current tree.

---

## [Allocation View: Work Assignment Style — Sprint & Team Structure](view-allocation-work-assignment.md)

> QA: All QAS · Style: 작업할당 스타일

Architecture elements → Organizational units (team1, team2, milestones, sprints). Shows who owns what, when it lands, and how scope is gated between Milestone 2 (06-22) and Milestone 3 Demo (07-01).
