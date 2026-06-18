# Quality Attribute Requirements

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Governing Goal

**Measurement Accuracy, Error Detection, and Handling** is not one QA among equals — it is the criterion the entire architecture is evaluated against. Rate, Amplitude, and Beat Error values must match the WeiShi No.1000 reference device.

Every other QA is a structural prerequisite that makes accuracy achievable.

```
Goal: Measurement Accuracy, Error Detection, and Handling
├── Enabler:        Extensibility, Modifiability    → 11 graphs must exist before experiments can run;
│                                                    architecture changes must apply fast enough to meet schedule
├── Prerequisite:   Real Time Performance           → missed deadline = dropped beat = wrong Rate/BPH
├── Prerequisite:   Low Latency and Low Number      → late timestamp = wrong Beat Error / Amplitude
│                   of Missed Beats
└── Prerequisite:   Correctness                    → false trigger = wrong everything
```

---

## Priority Summary

| Priority | QA | Role | Rationale | M2 Status |
|:--------:|----|------|-----------|:---------:|
| **1** | **Measurement Accuracy, Error Detection, and Handling** | Why this system exists | Rate / Amplitude / Beat Error must match WeiShi reference. This is the criterion the entire architecture is evaluated against — all other QAs exist only to make this achievable | 🔶 RPi pending |
| **2** | **Real Time Performance** | Structural prerequisite | If the pipeline misses the 21ms deadline, beat events are dropped. Dropped events mean Rate and BPH cannot be calculated correctly. EXP-02 confirmed 43% miss — largest structural fix required | 🔶 macOS ✅, RPi R5 06/23 |
| **3** | **Low Latency and Low Number of Missed Beats** | Structural prerequisite | If capture→detect latency exceeds one beat period, T1/T3 event timestamps are wrong. Wrong timestamps corrupt Beat Error and Amplitude calculations | 🔶 macOS ✅, RPi R5 06/23 |
| **4** | **Extensibility, Modifiability** | Execution enabler | 11 graph tabs must exist before experiments can run on them. Architecture decisions derived from experiments must be applied quickly enough to complete the project on schedule. Without a clean layer structure, both conditions fail | ✅ Verified |
| **5** | **Correctness** | Signal-level prerequisite | LP/HP filtering removes ambient noise and false triggers. Without it, the detector fires on non-beat events — undermining accuracy regardless of pipeline performance | ⏳ EXP-03 06/25 |

**Change from M1**: M1 treated Accuracy as implicit and ranked Extensibility last. M2 promotes Measurement Accuracy, Error Detection, and Handling to explicit Priority 1 — it is the criterion graders evaluate the architecture against.

---

## QAS-0: Measurement Accuracy, Error Detection, and Handling — Priority 1 (Governing Goal)

> This QA did not exist explicitly in M1. Promoted from "Correctness (QAS-3 M1)" to Priority 1 / governing goal in M2 after recognizing that every other QA exists only to make this one achievable.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch under measurement |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH input to microphone |
| **Artifact** | Full pipeline — AudioCapture → DSP → MeasurementEngine → all graph displays |
| **Environment** | Raspberry Pi 5, Live mode, 96kHz sample rate, normal operating temperature |
| **Response** | Rate (s/d), Amplitude (°), Beat Error (ms), BPH computed and displayed in all views |
| **Measure** | Values match WeiShi No.1000 reference within tolerance; deviation = 0 across all graph tabs showing the same metric |

### Architecture Connection

| QA Threat | Root Cause | Architecture Decision |
|-----------|------------|----------------------|
| Dropped beat event | Single-core saturation (RPi baseline 43% miss) | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md): T2 DSP Offload Thread |
| Shifted timestamps | 420ms Qt event loop backlog | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md): QueuedConnection decoupling |
| False triggers | LP/HP filter cutoffs unconfirmed | ADR-003 pending [EXP-03](experiments/exp-03-filter-sweep.md) |
| Value inconsistency across tabs | — | Observer / Qt Signal-Slot — structurally guaranteed |

### Trade-off Accepted

BPH coverage narrowed to 28,800 BPH for M3. Full range (18,000–36,000 BPH) is an accuracy stretch goal, not a structural constraint.

---

## QAS-1: Real Time Performance — Priority 2

> M1 status: Provisional (pending EXP-01). M2 status: macOS confirmed; RPi R5 scheduled 06/23.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH; each 2048-sample audio block arrives from ALSA |
| **Artifact** | Audio pipeline — AudioCapture → DSPWorker → MeasurementEngine |
| **Environment** | Raspberry Pi 5, 96kHz sample rate, Qt GUI running concurrently |
| **Response** | Every audio block is captured, processed, and a beat measurement emitted without skipping any block |
| **Measure** | Deadline miss = 0%; dropped blocks = 0 over a 10-minute session at 96kHz |

### SPS ↔ Block Period Relationship

| SPS | Block Period | Beat Error Resolution | Target |
|:---:|:-----------:|:--------------------:|:------:|
| 48,000 | ~20 ms | 20.8 µs/sample | Fallback |
| **96,000** | **~10 ms** | **10.4 µs/sample** | **Primary** |
| 192,000 | ~5 ms | 5.2 µs/sample | Stretch |

### M2 Evidence

| Run | Platform | Deadline Miss | Decision Applied |
|:---:|----------|:-------------:|-----------------|
| RPi Baseline (EXP-02 R1) | Raspberry Pi 5 | **43%** | None |
| macOS + T2 (EXP-02 R2) | macOS | **0%** | ADR-001 T2 applied |
| RPi R5 (EXP-02) | Raspberry Pi 5 | ⏳ | T2 + R1 target 06/23 |

### Architecture Tactics

| Tactic | Decision | Rationale |
|--------|----------|-----------|
| Introduce Concurrency | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread | Separates AudioCapture from DSP; eliminates single-core saturation |
| Manage Work Requests (Lazy Rendering) | [ADR-002](adr/ADR-002-r1-lazy-rendering.md) R1 isVisible() guard | Removes replot() from exec path; reduces exec budget consumption 75–85% |
| Priority Scheduling (Supplementary) | ADR-003 supplement — T1 SCHED_RR (EXP-02 R6 06/24) | Absorbs OS scheduler jitter; mitigates thermal throttle (85°C on RPi) |

---

## QAS-2: Low Latency and Low Number of Missed Beats — Priority 3

> M1 status: Provisional (pending EXP-02). M2 status: macOS segment ① confirmed; RPi pending.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | A single beat event (tick) occurs at the microphone |
| **Artifact** | Full pipeline — from AudioCapture callback to GUI display update |
| **Environment** | Raspberry Pi 5, normal operating conditions, Qt GUI active |
| **Response** | Corresponding waveform, beat markers, and computed values appear in the GUI |
| **Measure** | ① capture→detect < 20.8ms (one beat period at 28,800 BPH); ③ E2E < 100ms |

### Latency Segment Breakdown

| Segment | Boundary | Bottleneck | Target |
|---------|----------|------------|:------:|
| ① capture→process | ALSA callback → T1/T3 timestamp | OS callback period (~20ms) + Qt backlog | < 20.8ms |
| ② process→display | T1/T3 timestamp → `paintEvent()` complete | Qt rendering, replot_count | < 30ms |
| ③ end-to-end | ALSA callback → screen update | ①+② combined | < 100ms |

### Latency Targets by BPH

| BPH | Beat Period | E2E Target (80% margin) | ① capture→process | ② process→display |
|:---:|:-----------:|:-----------------------:|:-----------------:|:-----------------:|
| 28,800 | 125 ms | **< 100 ms** (Primary) | < 70 ms | < 30 ms |
| 36,000 | 100 ms | < 80 ms | < 56 ms | < 24 ms |
| 43,200 | 83 ms | < 66 ms | < 46 ms | < 20 ms |

### M2 Evidence

| Metric | Before T2 (macOS R1) | After T2 (macOS R2) | Change |
|--------|:--------------------:|:-------------------:|:------:|
| wait_ms avg (segment ①) | **420 ms** | **0.013 ms** | ×32,000 reduction |
| backlog | **47%** | **0%** | fully eliminated |
| RPi E2E | ⏳ EXP-02 R5 06/23 | — | — |

### Architecture Tactics

| Tactic | Decision | Effect |
|--------|----------|--------|
| Introduce Concurrency | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) — ring buffer + DSPWorker | wait_ms ×32,000 reduction; segment ① backlog eliminated |
| Lock-Free Ring Buffer | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) — PCM ring buffer between AudioCapture and DSPWorker | Eliminates mutex lock contention between capture and DSP |
| Reduce Computational Overhead | [ADR-002](adr/ADR-002-r1-lazy-rendering.md) — isVisible() guard | Removes unnecessary replot() from exec path; segment ② load reduced |

---

## QAS-3: Extensibility, Modifiability — Priority 4 (Execution Enabler)

> M1 name: "Extensibility (QAS-5)". M2: uses exact Project Plan terminology "Extensibility, Modifiability".
> M1 status: Provisional (post-refactoring). M2 status: ✅ Verified by 4-layer structure + IGraphTab interface.

| Field | Detail |
|-------|--------|
| **Source** | Developer (adding a new graph display tab) |
| **Stimulus** | Request to implement a new graph type (e.g., Spectrogram, Long-Term Trend, Sequence Display) |
| **Artifact** | Source code — Presentation layer and its relationship to Domain and below |
| **Environment** | Development environment (Qt Creator, macOS/Windows), within the 5-week project timeline |
| **Response** | New graph widget implemented, registered, and tested independently; no existing preprocessing modules modified |
| **Measure** | Files changed = ≤ 3; direct references from the new tab to Signal Processing or Acquisition = 0 |

### File Change Breakdown (≤ 3 Rule)

| Change | File | Count |
|--------|------|:-----:|
| New graph widget class | `GraphFoo.cpp` + `GraphFoo.h` | 1 (or 2 if header separate) |
| Register tab in manager | `GraphTabManager.cpp` | 1 |
| (No data subscription change needed) | `IGraphTab` interface already wired | 0 |
| **Total** | | **≤ 3** |

### M2 Verification

The 4-layer allowed-to-use structure and `IGraphTab` interface enforce the ≤ 3 rule structurally:

```
Presentation → Domain (IGraphTab::updateData receives Measurement struct)
             ↛ Signal Processing  (forbidden by layer rule)
             ↛ Acquisition        (forbidden by layer rule)
```

Adding a new tab = implement `IGraphTab` + register in `GraphTabManager`. Zero changes to Domain or below. Verified against existing tabs (TraceDisplay, VarioDisplay, BeatErrorDisplay).

### Architecture Tactics

| Tactic | Rationale |
|--------|-----------|
| Split Module (4-layer) | Separates Presentation from DSP concerns; each layer has a single responsibility |
| Observer / Qt Signal-Slot | MeasurementEngine emits one `Measurement` struct; all tabs subscribe — new tab adds only a subscription |
| Restrict Dependencies (IGraphTab) | Presentation layer allowed to reference Domain only; compiler enforces the boundary |

---

## QAS-4: Correctness — Priority 5

> M1 name: "Correctness (QA-C2)". M2 refactor: split into signal quality (this) and accuracy goal (QAS-0).
> M1 status: Provisional (pending EXP-03). M2 status: Pending — EXP-03 scheduled 06/25.

| Field | Detail |
|-------|--------|
| **Source** | Microphone input signal |
| **Stimulus** | Non-beat acoustic signal (ambient noise, vibration) enters the microphone alongside watch beats |
| **Artifact** | Signal Processing layer — LP/HP FilterChain → BeatDetector |
| **Environment** | Live mode; ambient noise present (office, workshop, handling noise) |
| **Response** | LP/HP filter rejects the non-beat signal; BeatDetector fires only on genuine T1/T3 events |
| **Measure** | False trigger rate < 1% under standard ambient conditions; true T1 detection rate > 99% |

### Warning Conditions (Usability Integration)

| Condition | Detection Method | GUI Message |
|-----------|-----------------|-------------|
| No signal | No beat event for N seconds (Heartbeat tactic) | `⚠ No signal` — auto-cleared on recovery |
| Noisy signal | Beat event inter-arrival variance exceeds threshold | `⚠ Noisy signal` — auto-cleared on stabilization |

N and M values (display/clear delay) to be confirmed by experiment after EXP-03.

### Architecture Tactics

| Tactic | Rationale |
|--------|-----------|
| LP/HP FilterChain (Pipes and Filters) | Rejects ambient noise below HP cutoff and above LP cutoff before BeatDetector runs — removes false trigger source |
| Heartbeat (N-second window) | Reuses existing T1/T3 event stream as heartbeat; no signal for N seconds triggers warning without separate detection logic |
| Adaptive Threshold | BeatDetector threshold tunable via single parameter; accommodates microphone sensitivity differences across watch models |

### Pending

EXP-03 (06/25) determines final LP/HP cutoff constants. Until then, default values are conservative but unconfirmed for edge cases.

---

## QA → Architecture Traceability Matrix

| QA | Architecture Mechanism | Decision | Evidence |
|----|----------------------|----------|---------|
| QAS-0 Measurement Accuracy, Error Detection, and Handling | All QAs in combination | All ADRs | EXP-02 macOS validated; RPi R5 pending |
| QAS-1 Real Time Performance | T2 DSP Offload Thread + Lazy Rendering isVisible() guard | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md), [ADR-002](adr/ADR-002-r1-lazy-rendering.md) | wait_ms ×32,000 ↓, backlog 0%; replot_count ↓75–85% (EXP-02 R2/R3/R4) |
| QAS-2 Low Latency and Low Number of Missed Beats | QueuedConnection + ring buffer | [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) | wait_ms 420ms → 0.013ms (EXP-02 R2) |
| QAS-3 Extensibility, Modifiability | 4-layer structure + IGraphTab | Architecture structure | ≤ 3 file change rule verified on existing tabs |
| QAS-4 Correctness | LP/HP FilterChain + Adaptive Threshold | ADR-003 pending EXP-03 | EXP-03 scheduled 06/25 |

---

## Changes from M1

| Item | M1 | M2 |
|------|----|----|
| QA numbers | Provisional (`⚠️` flagged) | EXP-02 confirmed — `⚠️` removed for QAS-1/2 |
| Tactic location | Mixed into QA doc | Moved to [Architectural Approaches](approaches.md); QAs describe problem only |
| QA names | Team-invented names | Exact terms from Project Plan (Plakosh/Popowski/Beck 2026) |
| QAS-3 Correctness | Single QA covering both consistency and noise | Split: false-trigger correctness → QAS-4 (Correctness, Priority 5); value accuracy → QAS-0 (Priority 1) |
| QAS-5 Extensibility | Provisional (post-refactoring) | "Extensibility, Modifiability" per Project Plan; Priority 4 (execution enabler) |
| Accuracy | Implicit in Correctness | Promoted to explicit Priority 1 — "Measurement Accuracy, Error Detection, and Handling" per Project Plan |
