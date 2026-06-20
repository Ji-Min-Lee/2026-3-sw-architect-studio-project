# Section 2 — Architecture Views

← [Wrap-up & Intro](slide-m1-wrapup-intro.md) | [Presentation Index](README.md) | Next: [Schedule →](slide-schedule.md)

> **Time**: ~12 min | Goal: Latency → Correctness → Extensibility — each decision driven by experiment evidence

All views follow the **Merson 7-section template**. Each view is written for a specific reader and a specific QA.  
→ Full view documents: [references/views/](references/views/)

---

## 2-A. Latency: Thread Separation

> 📢 **PRESENT** (~4 min) · Evidence: [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) · Decision: [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md)

**Problem**: GUI replot blocks DSP processing on a single thread → 43% deadline miss on RPi

![DSP Pipeline Thread Model](assets/view3-thread-model.png)

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

**Module structure** (static relationships):

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
    }

    MeasurementEngine ..> BaseGraphTab : «notify»
    MeasurementEngine ..> Measurement : «creates»
    Measurement *-- WatchMetrics
    Measurement *-- SignalFrame
    Measurement *-- AcousticEvent
    BaseGraphTab <|-- TraceTab
    BaseGraphTab <|-- VarioTab
    BaseGraphTab <|-- BeatErrorTab
    BaseGraphTab <|-- OtherTabs
    MainWindow o-- BaseGraphTab : mAllTabs[*]
```

> `SessionController` wires the Qt signal-slot connection at session start — it is a runtime connector, not shown here (see 2-A C&C view).

**Effects**:
- `MeasurementEngine` has zero knowledge of concrete tabs (depends only on abstract `BaseGraphTab`) → Subject and Observer fully decoupled
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

Acquisition → Signal Processing → Domain → Presentation. Dependencies flow downward only.  
Adding a new tab = ≤ 3 files in Presentation. Zero changes to Domain or below. ✅ 14 tabs implemented.

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

**Response**: AI-generated unit tests — structural correctness of `BaseGraphTab::updateData()` verifiable without domain expertise

- Validates interface compliance and layer boundary enforcement → runnable on macOS immediately
- All 14 tabs completed within W2 Sprint 1 without a domain expert

---

*Reference only — not presented:*

- [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) — RPi deploy workflow (Deployability)
- [ADR-002: R1 Lazy Rendering](references/adr/ADR-002-r1-lazy-rendering.md) — isVisible() guard, replot/beat 8.22 → 1.20 (↓85%)
- [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) — ≤3-file extension recipe in detail
