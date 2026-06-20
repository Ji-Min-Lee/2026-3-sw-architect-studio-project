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

**Decision**: Qt Signal-Slot as Observer — `MeasurementEngine` publishes a `Measurement` struct; all tabs subscribe

```
MeasurementEngine  ── Qt Signal: measurement(Measurement) ──▶  GraphTabManager
                                                                     ├── TraceTab::updateData(m)
                                                                     ├── VarioTab::updateData(m)
                                                                     └── … (14 tabs, same Measurement received)
```

**Effects**:
- `MeasurementEngine` has no knowledge of tabs → measurement logic and display logic fully decoupled
- Adding a new tab requires zero changes to `MeasurementEngine` → correctness preserved
- Qt `QueuedConnection` — safe cross-thread delivery from T2 (DSP) to Qt main thread

→ Full view: [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) · [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md)

---

## 2-C. Extensibility: Layer + Interface + Entity/VO

> 📢 **PRESENT** (~4 min) · Evidence: [QAS-4 Extensibility/Modifiability](references/qa/qas-4-extensibility-modifiability.md)

Three design decisions together enforce extensibility.

### Layer — 4-Layer Allowed-to-Use

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

Acquisition → Signal Processing → Domain → Presentation. Dependencies flow downward only.  
Adding a new tab = ≤ 3 files in Presentation. Zero changes to Domain or below. ✅ 14 tabs implemented.

→ Full view: [view-layered-4layer.md](references/views/view-layered-4layer.md)

### Interface — IAudioSource Dependency Inversion

![IAudioSource Dependency Inversion](assets/view5-iaudiosource.png)

| | AS-IS | TO-BE |
|---|-------|-------|
| Extension point | Modify `MainWindow` + `AudioManager` + `DSPWorker` | Implement `IAudioSource` only |
| `connect()` blocks | 3 (one per concrete worker) | 1 (via interface) |
| Adding `NetworkWorker` | 3+ file changes | **≤ 2 files** |

→ Full view: [view-iaudiosource.md](references/views/view-iaudiosource.md)

### Entity / Value Object — Domain Layer

The `Measurement` struct published by `MeasurementEngine` is composed of three VOs:

| Value Object | Contents | Immutability |
|---|---|---|
| `WatchMetrics` | Rate (s/d), Amplitude (°), Beat Error (ms), BPH | Immutable after computation |
| `SignalFrame` | PCM sample block + timestamp | Immutable after capture |
| `AcousticEvent` | Individual beat event (T1/T3 timestamps) | Immutable after detection |

Tabs receive `Measurement` as read-only → cannot mutate measurement results → correctness guaranteed.  
VOs stay in the Domain layer → replacing or adding Presentation components has zero impact on domain logic.

---

## 2-D. Risk: AI-Assisted Unit Test

> 📢 **PRESENT** (~1 min) · Evidence: [NTR-07](references/risks.md) — domain knowledge gap

**Risk**: Most team members lack watch measurement domain expertise (Rate, Amplitude, Beat Error) → hard to verify correctness of tab implementations

**Response**: AI-generated unit tests — structural correctness of `BaseGraphTab::updateData()` verifiable without domain expertise

- Validates interface compliance and layer boundary enforcement → runnable on macOS immediately
- All 11 tabs completed within W2 Sprint 1 without a domain expert

---

*Reference only — not presented:*

- [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) — RPi deploy workflow (Deployability)
- [ADR-002: R1 Lazy Rendering](references/adr/ADR-002-r1-lazy-rendering.md) — isVisible() guard, replot/beat 8.22 → 1.20 (↓85%)
- [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) — ≤3-file extension recipe in detail
