# Architecture

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## View Selection Rationale

View selection follows the principle: document a view IFF it is needed, IFF it is useful to someone (Merson). The four views below each answer a distinct architectural question for this project.

| View Name | Type | Question Answered | Primary Reader |
|---|---|---|---|
| TimeGrapher 4-layer allowed-to-use view | Module — Layered | Which direction do dependencies flow? Which layer changes when a new tab is added? | Developers (11 tabs in parallel) |
| Graph tab decomposition view | Module — Decomposition | What is the internal structure of the Presentation layer? How is a new tab added? | Developers (per-tab ownership) |
| DSP pipeline thread model | Runtime / C&C | How do the two threads cooperate? Where do T2 and R1 decisions operate at runtime? | Developers (performance / concurrency) |
| Raspberry Pi 5 hardware deployment view | Deployment / Allocation | Which hardware node runs which software? What is the deploy path from dev to target? | Operations, hardware owners |

---

## View 1 — TimeGrapher 4-Layer Allowed-to-Use View

Layered module view. Shows the four layers (Acquisition / Signal Processing / Domain / Presentation) and the allowed-to-use relation (downward only). Presentation may not bypass Domain to reach Signal Processing or Acquisition directly.

![TimeGrapher 4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

**Key Design Principle**

- Presentation depends on Domain only. Direct access to Signal Processing or Acquisition is forbidden.
- Adding a new graph tab = one new class in Presentation. Zero changes to Domain or below. **(Modifiability QA)**
- ADD-2-02 (R1): `isVisible()` guard in each tab's `updateData()` — non-visible tabs skip `replot()`.

---

## View 2 — Graph Tab Decomposition View

Decomposition view zooming into the Presentation layer. Shows how `GraphTabManager` manages all tabs through the `IGraphTab` interface, enabling parallel development of 11 tabs without cross-tab blocking.

![Graph Tab Decomposition View](assets/view2-decomposition.png)

**Extension Rule**

To add a new graph tab: create one class implementing `IGraphTab`, register it in `GraphTabManager`. Zero changes to Domain or below.

---

## View 3 — DSP Pipeline Thread Model

Runtime C&C view. Shows the two threads (Audio Thread, UI Thread) and their thread-safe connectors (ring buffer, `Qt::QueuedConnection`). ADD-2-01 (T2) and ADD-2-02 (R1) are labeled directly in the diagram at the point where they operate.

![DSP Pipeline Thread Model](assets/view3-thread-model.png)

**Experiment Link**

| Decision | Evidence | Effect |
|---|---|---|
| ADD-2-01 (T2) — DSPWorker thread | EXP-02 R2: wait_ms 420ms → 0.013ms | ×32,000 reduction, backlog 0% |
| ADD-2-02 (R1) — isVisible() guard | EXP-02 R3/R4: baseline 8.22 replot/beat | 75–85% replot reduction |

---

## View 4 — Raspberry Pi 5 Hardware Deployment View

Deployment view. Shows which software components run on which hardware nodes, and the deploy path from the dev machine (macOS) to the runtime target (Raspberry Pi 5). The AGC operational constraint is annotated directly on the diagram.

![Raspberry Pi 5 Hardware Deployment View](assets/view4-deployment.png)

**Operational Constraint**

AGC (Auto Gain Control) must be disabled on every RPi boot (`alsamixer` → Auto Gain Control → OFF). If AGC is on, signal gain fluctuates and Amplitude / Beat Error measurements become unreliable.

---

## Quality Attribute Tradeoff Analysis

### Accuracy as the Governing Goal

Accuracy is not one QA among equals — it is the criterion this entire architecture is evaluated against. The other QAs are the structural conditions that make accuracy possible.

| QA | How It Connects to Accuracy |
|----|-----------------------------|
| **Real-Time Performance** | A missed 21ms deadline drops a beat event. One dropped event = one missing T1/T3 timestamp = Rate, BPH incorrect for that cycle |
| **Low Latency** | If capture→detect latency exceeds one beat period (~20.8ms at 28,800 BPH), timestamps shift forward. Shifted timestamps corrupt Beat Error (time between T1 and T3) and Amplitude (derived from timing) |
| **Signal Quality / Noise** | LP/HP filtering removes spurious triggers before the detector runs. A false positive creates a phantom beat event — the measurement engine computes valid-looking but wrong numbers |
| **Modifiability** | Enables parallel development of 11 graph displays. Without it, developers block each other and the system that *shows* the accuracy evidence can't be built in time |

### Key Tradeoff: Modifiability vs. Performance

The 4-layer structure (Presentation → Domain → Signal Processing → Acquisition) introduces a strict abstraction boundary. This means:

- **Benefit**: New graph tabs never touch DSP code. Filter parameters can be swapped at the Signal Processing layer without rippling into Presentation.
- **Cost**: One additional function call per domain query. Measured overhead: negligible (< 0.1ms, confirmed EXP-02 R2).
- **Decision**: Accepted. The abstraction cost is less than 0.5% of the exec budget. The modifiability gain is critical for the 10-day build schedule.

### Key Tradeoff: R1 (Lazy Rendering) vs. Display Freshness

- **Benefit**: 75–85% replot reduction → exec budget freed for DSP → fewer deadline misses → better accuracy.
- **Cost**: Non-visible tabs do not refresh. A user switching tabs sees a stale frame for ~1 frame.
- **Decision**: Accepted. `showEvent()` + `QTimer::singleShot(0)` delivers a catch-up frame on every tab switch. Stale display < 1 frame (< 21ms) is imperceptible.

### What Accuracy Limitations Remain

| Limitation | Root Cause | Mitigation |
|------------|------------|------------|
| BPH coverage: 28,800 only | Time constraint | Filter sweep (EXP-03) extends to other BPH in M3 |
| Beat Error resolution ±0.1ms | 96kHz sample rate = 10.4µs/sample | Sufficient for WeiShi comparison; 192kHz stretch goal |
| RPi accuracy not yet confirmed | T2+R1 unverified on target hardware | EXP-02 R5 on 06/23 |

---

## Architecture Evaluation

### Experiment → Architecture Decision Map

| Experiment Result | Architecture Change |
|---|---|
| RPi deadline miss 43% (EXP-02 R1) | Identified structural root cause — not solvable by algorithm tuning |
| wait_ms 420ms on macOS baseline (EXP-02 R1) | ADD-2-01: T2 DSP Offload Thread |
| replot_count 8.22/beat without guard (EXP-02 R2b) | ADD-2-02: R1 Lazy Rendering |
| backlog 0%, wait_ms ×32,000 after T2 (EXP-02 R2) | T2 confirmed; layered architecture validated |
| replot ↓75–85% after R1 (EXP-02 R3/R4) | R1 confirmed; expected ~14ms plot_ms saving on RPi |

### QA Achievement Status

| QA | Target | Current Evidence | Status |
|---|---|---|---|
| Modifiability | New tab ≤ 3 file changes, 0 Domain changes | 4-layer structure enforced; new tab = one Presentation class | ✅ Design ready |
| Real-Time Performance | 0% deadline miss at 96kHz on RPi | 0% on macOS (T2+R1). RPi R5 pending | 🔶 macOS ✅, RPi pending |
| Low Latency | capture→display < 100ms | wait_ms 0.013ms on macOS. RPi TBD | 🔶 macOS ✅, RPi pending |
| Usability | Tab switch < 200ms response | `showEvent()` + `singleShot(0)` catch-up confirmed | ✅ |

### Unresolved Critical Concerns

| Concern | Plan |
|---|---|
| T2+R1 effect on RPi not yet measured | EXP-02 RPi R5 — 06/23 |
| Thermal throttle mitigation | T1 (SCHED_RR) on RPi R6 — 06/24 |
| 11-tab rendering under full load on RPi | EXP-05 — 06/26 |
