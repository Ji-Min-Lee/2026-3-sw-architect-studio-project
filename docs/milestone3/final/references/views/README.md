# Architecture Views — TimeGrapher

Eight views covering Latency, Correctness, Extensibility, and Deployability.  
Each view follows the **Merson 7-section template** and targets a specific QA.

---

## [DSP Pipeline Thread Model View](view-cc-dsp-pipeline.md)

> QA: Real-time Performance · Low Latency (QAS-1, QAS-2)

[![DSP Pipeline Thread Model View](../../assets/cc-dsp-pipeline.png)](view-cc-dsp-pipeline.md)

[![E2E Latency Comparison](../../assets/thread-latency-chart.png)](view-cc-dsp-pipeline.md)

---

## [Layered and Module Decomposition View](view-layered-4layer.md)

> QA: Extensibility / Modifiability (QAS-3)

[![Layered and Module Decomposition](../../assets/layered-view.png)](view-layered-4layer.md)

---

## [IAudioSource Dependency Inversion View](view-iaudiosource.md)

> QA: Correctness · Internal Consistency (QAS-4 Sub-2)

[![IAudioSource Dependency Inversion View](../../assets/module-iaudiosource.png)](view-iaudiosource.md)

---

## [Graph Tab Module Uses View](view-decomposition-graph-tab.md)

> QA: Correctness · Extensibility (QAS-4, QAS-3)

[![Graph Tab Module Uses View](../../assets/module-uses-graph-tab-decomposition.png)](view-decomposition-graph-tab.md)

[![Observer Module](../../assets/module-uses-graph-tab-observer-module.png)](view-decomposition-graph-tab.md)

---

## [LongTermTab Downsampling Decomposition View](view-longtermtab-downsampling.md)

> QA: Long-Term Performance · Bounded Plot Growth (QAS-6)

[![LongTermTab Downsampling Decomposition View](../../assets/module-decomposition-longtermtab.png)](view-longtermtab-downsampling.md)

---

## [Raspberry Pi Deployment View](view-deployment-build-pipeline.md)

> QA: Deployability

Shows how validated code moves from the development machine to the Raspberry Pi target and where target-dependent validation happens. Deployment and commit-time correctness enforcement are treated as separate concerns.

---

## [Pre-commit Correctness Gate View](view-allocation-implementation.md)

> QA: Correctness (QAS-4 Sub-1) · Commit-time enforcement view

Shows how formula and calculation regressions are blocked before commit acceptance through the local `pre-commit` gate. Focuses on `TestWatchMath` and `TestMeasurementEngine` as the structural enforcement path for QAS-4 Sub-1.

---

## [Work Assignment View](view-allocation-work-assignment.md)

> QA: All QAS · Style: Work Assignment

[![Work Assignment View](../../assets/allocation-work-assignment.png)](view-allocation-work-assignment.md)

Architecture elements → Organizational units (team1, team2, milestones, sprints). Shows who owns what, when it lands, and how scope is gated between Milestone 2 (06-22) and Milestone 3 Demo (07-01).
