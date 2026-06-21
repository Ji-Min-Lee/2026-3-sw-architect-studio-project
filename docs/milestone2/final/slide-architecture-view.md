# Section 2 — Architecture Views

← [Wrap-up & Intro](slide-m1-wrapup-intro.md) | [Presentation Index](README.md) | Next: [Schedule →](slide-schedule.md)

> **Time**: ~12 min | Goal: Latency → Correctness → Extensibility — each decision driven by experiment evidence

All views follow the **Merson 7-section template**. Each view is written for a specific reader and a specific QA.  
→ Full view documents: [references/views/](references/views/)

---

## 2-A. Latency: Thread Separation

> 📢 **PRESENT** (~4 min) · Evidence: [EXP-02](references/experiments/exp-03-latency-e2e.md) · Decision: [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md)

**Problem**: GUI replot blocks DSP processing on a single thread → 43% deadline miss on RPi

![DSP Pipeline Thread Model](assets/view3-thread-model-simple.png)

![E2E Latency Comparison](assets/thread-latency-chart.png)

**Decision**: Separate into three threads — T1 (audio capture) · T2 (DSP) · Qt main (rendering), connected by a lock-free ring buffer

| Metric | Before (single thread) | After (T2 offload) |
|--------|:----------------------:|:------------------:|
| wait_ms avg | 420 ms | **0.013 ms** (×32,000) |
| Deadline miss | 43% | **0%** (macOS + RPi) |
| Backlog | Present | **None** |

→ Full view: [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) · ADR: [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md)

---

## 2-B. Correctness: Observer Pattern

```mermaid
classDiagram
    class MeasurementEngine {
        <<Subject>>
        +measurementReady(m Measurement)
    }
    class BaseGraphTab {
        <<abstract, Observer>>
        +onMeasurement(m Measurement)*
        +replotAll()*
        #mPaused bool
    }
    class TraceTab
    class VarioTab
    class BeatErrorTab
    class OtherTabs["... + 11 more tabs"]
    class Measurement {
        <<Value Object>>
    }
    class WatchMetrics {
        rate_spd, amplitude_deg
        beatError_ms, bph
    }
    class SignalFrame {
        samples: PCMBlock
        timestamp: uint64
    }
    class AcousticEvent {
        t1, t3: uint64
    }
    class MainWindow {
        +mAllTabs List~BaseGraphTab~
        +registerTab(tab, label)
        +onMeasurementReady(m Measurement)
        -mSession SessionController
    }
    class SessionController {
        <<wiring coordinator>>
        +connectObservers(tabs, receiver, slot)
        -mObserverTabs List~BaseGraphTab~
    }

    MeasurementEngine ..> Measurement : «creates»
    Measurement *-- WatchMetrics
    Measurement *-- SignalFrame
    Measurement *-- AcousticEvent
    BaseGraphTab <|-- TraceTab
    BaseGraphTab <|-- VarioTab
    BaseGraphTab <|-- BeatErrorTab
    BaseGraphTab <|-- OtherTabs
    MainWindow o-- BaseGraphTab : mAllTabs[*]
    MainWindow *-- SessionController : mSession
    MainWindow ..> SessionController : connectObservers(mAllTabs)
    SessionController ..> BaseGraphTab : «uses» mObserverTabs
    SessionController ..> MeasurementEngine : «uses» at connect
```

→ [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md) · [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md)

---

## 2-C. Extensibility: Layer + Interface + Entity/VO

### Layer — 4-Layer Allowed-to-Use

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

| Sprint | Tabs | Why |
|---|:---:|---|
| W2 S1 | 11 | Core requirements — baseline graph set |
| W2 S2 | +2 → 13 | Project-plan screens added (Fig 7-19): FilterScope + SweepScope |
| W3 S1 | +1 → **14** | Bonus: Radar/Polar chart for multi-position comparison |

→ [view-layered-4layer.md](references/views/view-layered-4layer.md)

### Interface — IAudioSource Dependency Inversion

![IAudioSource Dependency Inversion](assets/view5-iaudiosource.png)

→ [view-iaudiosource.md](references/views/view-iaudiosource.md) · [ADR-005](references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md)

### Entity / Value Object — Domain Layer

![Domain Entity / Value Object Module View](assets/view6-domain-entity-vo.png)

→ [view-domain-entity-vo.md](references/views/view-domain-entity-vo.md)

---

## 2-D. Risk: AI-Assisted Unit Test

→ [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md) · [risks.md — NTR-07](references/risks.md)

---

*Reference only — not presented:*

- [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) — RPi deploy workflow (Deployability)
- [ADR-002: R1 Lazy Rendering](references/adr/ADR-002-r1-lazy-rendering.md) — isVisible() guard, replot/beat 8.22 → 1.20 (↓85%)
- [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) — ≤3-file extension recipe in detail
