# Section 2 — Architecture Views

← [Wrap-up & Intro](1-slide-m1-wrapup-intro.md) | [Presentation Index](README.md) | Next: [Schedule →](3-slide-risk-and-schedule.md)

> **Time**: ~9 min | Goal: Latency → Correctness → Extensibility — each decision driven by experiment evidence

All views follow the **Merson 7-section template**. Each view is written for a specific reader and a specific QA.  
→ Full view documents: [references/views/](references/views/)

---

## 2-A. Latency: Thread Separation

**Problem**: GUI replot blocks DSP processing on a single thread → 43% deadline miss on RPi

![DSP Pipeline Thread Model](assets/view3-thread-model-simple.png)

![E2E Latency Comparison](assets/thread-latency-chart.png)

**Decision**: Separate into three threads — T1 (audio capture) · T2 (DSP) · Qt main (rendering), connected by a ring buffer

| Metric | Before (single thread) | After (T2 offload) |
|--------|:----------------------:|:------------------:|
| wait_ms avg | 77.4 ms | **0.03 ms** (×2,600) |
| Deadline miss | 43% | **0%** |
| Backlog | Present | **None** |

| Category | Documents |
|----------|-----------|
| QA | [QAS-2 — Every audio block at 96kHz is processed without a single dropped block over a 10-minute session](references/qa/qas-1-real-time-performance.md) · [QAS-3 — From beat event at microphone to GUI update, end-to-end latency < 100ms](references/qa/qas-2-low-latency-and-low-number-of-missed-beats.md) |
| Risk | [TR-02 — Single-threaded pipeline: 43% deadline miss on RPi](references/risks.md) |
| Experiment | [EXP-02 — Dropped block measurement on RPi at 96kHz](references/experiments/exp-02-realtime-dropped-block.md) · [EXP-03 — E2E latency measurement using 2-segment timestamps](references/experiments/exp-03-latency-e2e.md) |
| ADR | [ADR-001 — Introduce a dedicated DSP offload thread (T2)](references/adr/ADR-001-t2-dsp-offload-thread.md) · [ADR-002 — Skip replot() for non-visible tabs (Lazy Rendering)](references/adr/ADR-002-r1-lazy-rendering.md) · [ADR-004 — Timer-decoupled rendering (R2)](references/adr/ADR-004-r2-timer-decoupled-rendering.md) |
| View | [C&C View: DSP Pipeline Thread Model](references/views/view-cc-dsp-pipeline.md) |
| Related References | [Deployment View — macOS to RPi 5 build and deploy pipeline](references/views/view-deployment-build-pipeline.md) |

---

## 2-B. Correctness: Observer Pattern

![Observer Module View](assets/view2b-observer-module.png)

| Category | Documents |
|----------|-----------|
| QA | [QAS-5 — Non-beat acoustic noise is rejected by the filter chain, keeping false trigger rate < 1% and T1 detection rate > 99%](references/qa/qas-4-correctness.md) |
| Risk | [Risk Register — NTR-07: Equation-level derivations difficult (Rate / Beat Error / Amplitude formulas)](references/risks.md) |
| Experiment | [EXP-04 — Observer pattern compliance: measuring tab extension cost in file count](references/experiments/exp-04-extensibility-observer-pattern.md) |
| ADR | [ADR-006 — BaseGraphTab Observer pattern and tab registration structure](references/adr/ADR-006-basegraphtab-observer-pattern.md) |
| ADR (contingency) | [ADR-004 — Timer-decoupled rendering: if all 14 tabs are visible simultaneously, rendering is driven by a fixed 20 FPS timer instead of each beat event — activated only if EXP-05 (06/26) shows deadline miss under full tab load](references/adr/ADR-004-r2-timer-decoupled-rendering.md) |
| View | [Decomposition View: Graph Tab — ≤3-file extension recipe](references/views/view-decomposition-graph-tab.md) |
| Test Results | [Unit Test Results — 142 tests across 10 binaries, all passing — domain math, engine integration, and Observer contract](references/unit-test-results.md) |

---

## 2-C. Extensibility: Layer + Interface + Entity/VO

### Layer — 4-Layer Allowed-to-Use

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

| Sprint | Tabs | Why |
|---|:---:|---|
| W2 S1 | 11 | Core requirements — baseline graph set |
| W2 S2 | +2 → 13 | Project-plan screens added (Fig 7-19): FilterScope + SweepScope |
| W3 S1 | +1 → **14** | Bonus: Radar/Polar chart for multi-position comparison |

| Category | Documents |
|----------|-----------|
| QA | [QAS-4 — Adding a new graph tab requires ≤ 3 files changed and zero references to layers below Presentation](references/qa/qas-3-extensibility-modifiability.md) |
| Risk | N/A |
| Experiment | N/A |
| ADR | N/A |
| View | [Layered View: 4-Layer Allowed-to-Use](references/views/view-layered-4layer.md) |
| Related References | [Decomposition View: Graph Tab — ≤3-file extension recipe in detail](references/views/view-decomposition-graph-tab.md) |

### Interface — IAudioSource Dependency Inversion

![IAudioSource Dependency Inversion](assets/view5-iaudiosource.png)

| Category | Documents |
|----------|-----------|
| QA | [QAS-4 — Adding a new graph tab requires ≤ 3 files changed and zero references to layers below Presentation](references/qa/qas-3-extensibility-modifiability.md) |
| Risk | N/A |
| Experiment | N/A |
| ADR | [ADR-005 — IAudioSource dependency inversion (P1 refactor)](references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) |
| View | [Module View: IAudioSource Dependency Inversion](references/views/view-iaudiosource.md) |
| Related References | [ADR-002 — Lazy Rendering: isVisible() guard reduces replot cost by 85%](references/adr/ADR-002-r1-lazy-rendering.md) |

### Entity / Value Object — Domain Layer

![Domain Entity / Value Object Module View](assets/view6-domain-entity-vo.png)

| Category | Documents |
|----------|-----------|
| QA | [QAS-1 — Rate, Amplitude, and Beat Error computed from a 28,800 BPH watch match WeiShi No.1000 reference within tolerance](references/qa/qas-5-measurement-accuracy-error-detection-handling.md) · [QAS-5 — Non-beat acoustic noise is rejected by the filter chain, keeping false trigger rate < 1% and T1 detection rate > 99%](references/qa/qas-4-correctness.md) |
| Risk | N/A |
| Experiment | [EXP-05 — Detector parameter optimization under noise conditions](references/experiments/exp-05-correctness-detector-optimization.md) |
| ADR | [ADR-003 — Audio sample rate selection for RPi 5](references/adr/ADR-003-sample-rate-selection.md) |
| View | [Module View: Domain Entity / Value Object](references/views/view-domain-entity-vo.md) |

