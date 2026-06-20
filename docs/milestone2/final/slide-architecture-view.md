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
| Deadline miss | 43% | **0%** (macOS) · RPi: E2-8 scheduled |
| Backlog | Present | **None** |

> 🔴 **Unresolved**: FG scheduling — fg_wait avg 60.1ms, 84% > deadline → E2-8 (6/22)

→ Full view: [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) · ADR: [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md)

---

## 2-B. Correctness: Observer Pattern

> 📢 **PRESENT** (~3 min) · Evidence: QAS-1 Measurement Accuracy (governing goal)

**Problem**: Measurement results must be delivered consistently to all tabs. Direct per-tab calls increase coupling and risk missed updates.

**Decision**: Qt Signal-Slot as Observer — `MeasurementEngine` publishes a `Measurement` struct; all tabs subscribe via `BaseGraphTab::onMeasurement()`

**Observer pattern — static structure** (Subject, Observer, wiring coordinator; compile-time contracts):

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

> No compile-time link `MeasurementEngine → BaseGraphTab`. `SessionController` is the wiring coordinator — it stores `mObserverTabs` and applies `connect()` at session start (sequence below). Per-beat delivery remains Qt signal-slot only.

**Runtime wiring** — [TimeGrapher Observer Runtime Sequence](assets/view2b-observer-runtime.puml):

![TimeGrapher Observer Runtime Sequence](assets/view2b-observer-runtime.png)

Three phases (autonumbered UML sequence; lifelines left → right: **User** · Qt Main Thread · **MeasurementEngine** · **Mic**):

1. **Register observers (once)** — `MainWindow` → `SessionController.connectObservers()`; stores `mObserverTabs` only (no `connect` yet)
2. **Wire signal-slot (per session)** — **User** `Start()` → each session start registers Qt `connect()`: `MeasurementEngine::measurementReady` → `BaseGraphTab::onMeasurement` (×14) and → `MainWindow::onMeasurementReady()`; `QueuedConnection` (DSP Thread → Main Thread)
3. **Deliver measurement (per DSP block)** — **Mic** `PCM samples` → DSP Thread `processBlock()` → Main Thread `onMeasurement()` ×14 + `onMeasurementReady()` (`{duration}` `tg_us`, `wait_ms` — EXP-02)

> Source: [`assets/view2b-observer-runtime.puml`](assets/view2b-observer-runtime.puml) · Cross-thread via `QueuedConnection` (see 2-A)

**Effects**:
- `MeasurementEngine` has **zero compile-time knowledge of tabs** — emits signal only; `SessionController` wires the abstract `onMeasurement` slot → Subject and Observer decoupled at build time
- All 14 tabs inherit `BaseGraphTab` → same `onMeasurement()` contract, same `isVisible()` lazy-render guard, same `showEvent()` catch-up frame
- Adding a new tab = 1 new subclass + 3 lines in `MainWindow` → zero changes to `MeasurementEngine` or any other tab (ADR-006)
- `Measurement` is composed of immutable VOs — tabs receive a read-only snapshot → correctness guaranteed

→ Full view: [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md) · ADR: [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md)

---

## 2-C. Extensibility: Layer + Interface + Entity/VO

> 📢 **PRESENT** (~4 min) · Evidence: [QAS-4 Extensibility/Modifiability](references/qa/qas-4-extensibility-modifiability.md)

Three design decisions together enforce extensibility.

### Layer — 4-Layer Allowed-to-Use

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

> **What the diagram shows**: Five horizontal bands (colored by layer), each containing its key modules. Dashed arrows indicate allowed-to-use direction — downward only. UI Coordinator (`MainWindow`, `SessionController`) sits above Presentation as a wiring layer; it is not counted as a domain layer.

| Layer | Responsibility | Key Modules |
|-------|---------------|-------------|
| **Presentation** | Render measurement data in graphs and charts | `BaseGraphTab`, 14 concrete tabs (`TraceTab`, `VarioTab`, `BeatErrorTab`, …) |
| **Signal Processing** | DSP computation and beat detection on T2 thread | `DSPWorker`, `MeasurementEngine`, `FilterChain`, `BeatDetector` |
| **Domain** | Watch physics math and AI diagnosis | `WatchMath`, `WatchDiagnostics`, `WatchExplainer`, `Measurement` VOs |
| **Acquisition** | Audio input abstraction and ring buffer | `IAudioSource`, `AudioWorker`, `PlaybackWorker`, `SimWorker`, `AudioRingBuffer` |

Acquisition → Signal Processing → Domain → Presentation. Dependencies flow downward only.  
Adding a new tab = ≤ 3 files in Presentation. Zero changes to Domain or below.

| Batch | Tabs | Trigger | Files changed (excl. build/test) |
|---|---|---|---|
| W2 S1 | 11 (baseline) | Core requirements | NewTab + MainWindow = **2** |
| W2 S2 | +2 → 13 | Project-plan screen requirements (Fig 7-19) | FilterScopeTab + SweepScopeTab + MainWindow = **2 each** |
| W3 S1 | +1 → **14** | Radar/Polar chart (bonus) | RadarChartTab + SequenceTab + MainWindow = **3** ¹ |

¹ RadarChartTab reads per-position data from SequenceTab (not via `measurementReady`) → SequenceTab modified to expose `capturedReadings()` + `sequenceUpdated()`.

✅ All 14 added within ≤ 3-component rule — Domain layer untouched each time.

**Dependency Structure Matrix (actual code — `#include` trace)**

Row = **used module** (depended upon) · Column = **using module** (depends on) · `1` = depends · `-` = self · `🔴` = layer violation

| used ↓ \ using → | Presentation | UI Coord | Sig.Proc | Domain | Acquisition |
|---|:---:|:---:|:---:|:---:|:---:|
| **Presentation** (`*Tab`, `BaseGraphTab`) | - | 0 | 0 | 0 | 0 |
| **UI Coord** (`MainWindow`, `SessionCtrl`) | 0 | - | 0 | 0 | 0 |
| **Sig.Proc** (`DSPWorker`, `MeasurementEngine`) | 1 | 1 | - | 0 | 0 |
| **Domain** (`Measurement`, `WatchMath`, `WatchDiagnostics`) | 1 | 1 | 1 | - | 0 |
| **Acquisition** (`IAudioSource`, `AudioWorker`, `AudioRingBuffer`) | 0 | 1 | 1 | 0 | - |

> **Layer boundary note**: `MeasurementEngine` is classified in the Sig.Proc layer because it is directly owned and driven by `DSPWorker` within the T2 thread pipeline (`DSPWorker.h:37`, `DSPWorker.cpp:12`). It does not belong to Domain — it calls `processBlock()` as part of the DSP loop, not as a stand-alone domain service. Pure domain objects (`Measurement` VO, `WatchMath`, `WatchDiagnostics`) remain in the Domain layer and have no upward dependencies.

**Key observations:**
- All 1s are in the lower triangle → no layer violations ✅
- Domain column is all `0` → pure computation, no upward dependency ✅
- Presentation is never used by lower layers ✅

→ Full view: [view-layered-4layer.md](references/views/view-layered-4layer.md)

### Interface — IAudioSource Dependency Inversion

![IAudioSource Dependency Inversion](assets/view5-iaudiosource.png)

> **What the diagram shows**: AS-IS (left) — 3 separate `connect()` blocks, one per concrete worker. TO-BE (right) — a single `connect()` block in `SessionController` through the `IAudioSource` interface. Arrows show the direction of dependency inversion.

3 concrete sources exist: `AudioWorker` (mic) · `PlaybackWorker` (file) · `SimWorker` (simulation)

| | AS-IS | TO-BE |
|---|-------|-------|
| `connect()` blocks | 3 (one per concrete worker, scattered) | **1** (unified in `SessionController`) |
| Changing audio wiring | Touch multiple files | **1 place** |
| Readability | Duplicated logic per source | Single interface contract |

→ Full view: [view-iaudiosource.md](references/views/view-iaudiosource.md)

### Entity / Value Object — Domain Layer

The `Measurement` struct published by `MeasurementEngine` is composed of three VOs:

![Domain Entity / Value Object Module View](assets/view6-domain-entity-vo.png)

> **What the diagram shows**: `Measurement` as a composition root (solid border) with three immutable Value Objects (dashed borders). Color coding matches the Domain layer in view1. No arrows point upward — the domain has zero dependency on Presentation or Signal Processing.

| Value Object | Contents | Immutability |
|---|---|---|
| `WatchMetrics` | Rate (s/d), Amplitude (°), Beat Error (ms), BPH | Immutable after computation |
| `SignalFrame` | PCM sample block + timestamp | Immutable after capture |
| `AcousticEvent` | Individual beat event (T1/T3 timestamps) | Immutable after detection |

Tabs receive `Measurement` as read-only → cannot mutate measurement results → correctness guaranteed.  
VOs stay in the Domain layer → replacing or adding Presentation components has zero impact on domain logic.

→ Full view: [view-domain-entity-vo.md](references/views/view-domain-entity-vo.md)

---

## 2-D. Risk: AI-Assisted Unit Test

> 📢 **PRESENT** (~1 min) · Evidence: [NTR-07](references/risks.md) — domain knowledge gap

**Risk**: Most team members lack watch measurement domain expertise (Rate, Amplitude, Beat Error) → hard to verify correctness of tab implementations

**Response**: AI-generated unit tests — structural correctness of `BaseGraphTab::onMeasurement()` verifiable without domain expertise

- Validates interface compliance and layer boundary enforcement → runnable on macOS immediately
- 11 baseline tabs completed in W2 S1; 3 more added in W2 S2 + W3 S1 (project-plan screens + bonus) — all without a domain expert

---

*Reference only — not presented:*

- [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) — RPi deploy workflow (Deployability)
- [ADR-002: R1 Lazy Rendering](references/adr/ADR-002-r1-lazy-rendering.md) — isVisible() guard, replot/beat 8.22 → 1.20 (↓85%)
- [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) — ≤3-file extension recipe in detail
