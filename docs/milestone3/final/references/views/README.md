# Architecture Views — TimeGrapher

Six views covering Latency, Correctness, Extensibility, and Deployability.  
Each view follows the **Merson 7-section template** and targets a specific QA.

---

## [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md)

> QA: Real-time Performance · Low Latency (QAS-2, QAS-3)

[![DSP Pipeline Thread Model](../../assets/view3-thread-model-simple.png)](view-cc-dsp-pipeline.md)

[![E2E Latency Comparison](../../assets/thread-latency-chart.png)](view-cc-dsp-pipeline.md)

---

## [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md)

> QA: Extensibility / Modifiability (QAS-4)

[![4-Layer Allowed-to-Use](../../assets/view1-layered-module.png)](view-layered-4layer.md)

---

## [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md)

> QA: Extensibility / Modifiability (QAS-4)

[![IAudioSource Dependency Inversion](../../assets/view5-iaudiosource.png)](view-iaudiosource.md)

---

## [Module View: Domain Entity / Value Object](view-domain-entity-vo.md)

> QA: Correctness · Accuracy (QAS-1, QAS-5)

[![Domain Entity / Value Object](../../assets/view6-domain-entity-vo.png)](view-domain-entity-vo.md)

---

## [Decomposition View: Graph Tab](view-decomposition-graph-tab.md)

> QA: Correctness · Extensibility (QAS-4, QAS-5)

[![Graph Tab Decomposition](../../assets/view2-decomposition.png)](view-decomposition-graph-tab.md)

[![Observer Module](../../assets/view2b-observer-module.png)](view-decomposition-graph-tab.md)

---

## [Deployment View: Build-Deploy Pipeline](view-deployment-build-pipeline.md)

> QA: Deployability

[![Build-Deploy Pipeline](../../assets/view4-deployment.png)](view-deployment-build-pipeline.md)

---

## [Allocation View: Implementation Style — Test Binaries & Build Artifacts](view-allocation-implementation.md)

> QA: Correctness (QAS-4, QAS-5) · Style: 구현 스타일

Software modules → Build artifacts (`src/build-mac/`). Shows which source module is realized as which test binary and what verification scope each binary covers. 142 tests / 10 binaries, all PASS.

---

## [Allocation View: Work Assignment Style — Sprint & Team Structure](view-allocation-work-assignment.md)

> QA: All QAS · Style: 작업할당 스타일

Architecture elements → Organizational units (team1, team2, milestones, sprints). Shows who owns what, when it lands, and how scope is gated between Milestone 2 (06-22) and Milestone 3 Demo (07-01).
