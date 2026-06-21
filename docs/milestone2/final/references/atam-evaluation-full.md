# Architecture Tradeoff Analysis Method (ATAM) — TimeGrapher

**Project**: TimeGrapher — Real-Time Acoustic Watch Analysis  
**Evaluation Date**: 2026-06-22  
**Evaluation Team**: TimeGrapher Architecture Team (LG SW Architect Training Program × CMU MSE)  
**Method Reference**: ATAM (SEI) — Paulo Merson, CMU MSE LG Architecture Training Program

---

## Executive Summary

This document records the results of an ATAM-style architecture evaluation conducted on the TimeGrapher system at Milestone 2. The evaluation was triggered by EXP-02 results showing a 43% deadline-miss rate on Raspberry Pi 5, confirming that the current architecture cannot satisfy QAS-1 (Real-Time Performance) and QAS-2 (Low Latency) as-is.

The evaluation identified **3 risks**, **3 non-risks**, **4 sensitivity points**, and **3 tradeoff points**. The most critical risk theme is **rendering-audio coupling**: the fact that `plot` rendering inside the audio thread consumes 79% of the exec budget is a structural defect that no amount of algorithmic tuning can overcome. The chosen architectural approach (R1 + T1 for MVP, with a path to R2 + T2 for M3) addresses this theme directly.

---

## 1. The ATAM

The Architecture Tradeoff Analysis Method (ATAM) was developed by the Software Engineering Institute (SEI). ATAM is a **scenario-based architecture evaluation method** that provides qualitative assessments of risks and tradeoffs. Its purpose is **not** precise measurement, but to discover risks created by architectural decisions and to find correlations between architectural decisions and predictions of system properties.

**ATAM outputs produced by this evaluation:**

| Output | Status |
|--------|--------|
| Architectural approaches | Section 3 |
| Quality attribute utility tree | Section 4 |
| Scenarios | Section 4 |
| Risks | Section 5 |
| Non-risks | Section 5 |
| Sensitivity points | Section 5 |
| Tradeoff points | Section 5 |
| Risk themes | Section 6 |

---

## 2. Business Goals and Architecture

### 2.1 Business Context

The TimeGrapher project extends the baseline `TimeGrapher_v10.5` Qt/C++ application to run on **Raspberry Pi 5** (quad-core ARM, 8 GB RAM) with an 8" touchscreen. The system captures acoustic beat noise from a mechanical watch via a USB contact microphone and delivers real-time diagnostic measurements (Rate, Amplitude, Beat Error, BPH) with multiple visualization modes.

**Business drivers:**

| Driver | Description |
|--------|-------------|
| **Real-time correctness** | Beat events must be detected with sub-millisecond precision; missed or delayed beats produce wrong measurements |
| **Raspberry Pi deployment** | The system must run on constrained hardware (4 cores, thermal throttling at 85°C) |
| **Extensibility** | New graphs and analyses must be addable without modifying existing pipeline code |
| **Training program context** | Architecture decisions must be documented and justified for the CMU MSE mentoring review |

### 2.2 Architecture Overview

The system is organized in four layers:

```
┌─────────────────────────────────────────────────────┐
│  Presentation Layer                                  │
│  MainWindow → GraphTabManager → 11 Graph Tabs        │
│  (TraceDisplay, VarioDisplay, SequenceDisplay,       │
│   ScopeDisplay, BeatErrorTrace, LongTermPerf,        │
│   EscapementAnalyzer, Spectrogram, WaveformComparison│
│   ScopeMode, ScopeFunctionF0–F3)                     │
├─────────────────────────────────────────────────────┤
│  Domain Layer                                        │
│  MeasurementEngine → BeatDetector → MeasurementStore │
├─────────────────────────────────────────────────────┤
│  Signal Processing Layer                             │
│  FilterChain (LP/HP) → SignalBuffer (ring buffer)    │
├─────────────────────────────────────────────────────┤
│  Acquisition Layer                                   │
│  AudioCapture → [LiveCapture | PlaybackCapture       │
│                  | SimCapture]                       │
└─────────────────────────────────────────────────────┘
```

**Technical constraints:**
- Runtime: Raspberry Pi OS, Raspberry Pi 5 (BCM2712, quad-core Cortex-A76, 2.4 GHz)
- Framework: Qt (C++), single Qt event loop on main thread
- Audio: ALSA/PulseAudio, AGC disabled
- Beat interval at 28,800 BPH: 125 ms (8 beats/s); exec deadline = 21 ms

**Current threading structure:**

```
cpu2 (Audio Thread):
  AudioCapture → SignalBuffer → FilterChain
                → BeatDetector → MeasurementEngine
                → [11 Graph Tabs: updateData() + paintEvent()]  ← bottleneck

Qt Main Thread (any core):
  Qt Event Loop + UI interactions
```

### 2.3 EXP-02 Baseline — Confirmed Problem

| Metric | Value | Implication |
|--------|-------|-------------|
| exec mean | 20 ms | 95% of 21 ms deadline consumed |
| `plot` share of exec | 16 ms (79%) | Rendering is the bottleneck |
| Deadline miss rate | 43% | 441 of 1,015 frames late |
| cpu2 utilization | 91% | Entire pipeline pinned to one core |
| Temperature | 85°C | Sustained thermal throttling reduces clock speed |
| Remaining 3 cores | idle | Multi-core not utilized |

---

## 3. Architectural Approaches

An architectural approach is a tactic, design pattern, architectural style, or design strategy used to achieve quality attribute goals. The following are the predominant approaches identified for TimeGrapher.

### Decision 1: Rendering Strategy

Addresses the `plot` overrun of the exec deadline (root cause: rendering coupled to audio thread).

| ID | Approach | Core Tactic |
|----|----------|-------------|
| **R1** | Lazy Rendering (Active Tab Only) | `isVisible()` guard — only the visible tab calls `update()` per beat |
| **R2** | Timer-Decoupled Rendering | Qt timer (20 FPS) drives `update()`; beat events update data model only |
| **R3** | Double-Buffer Async Rendering | Each tab renders into a QPixmap off-screen; UI thread blits the finished pixmap |

### Decision 2: Threading Strategy

Addresses single-core (cpu2) saturation and idle multi-core (root cause: all pipeline stages on one thread).

| ID | Approach | Core Tactic |
|----|----------|-------------|
| **T1** | SCHED_RR + CPU Affinity | RT scheduling + pin audio thread to cpu0; other cores free for Qt/UI |
| **T2** | DSP Offload Thread | Separate AudioCapture (SCHED_RR, cpu0) from DSP pipeline (cpu1) via PCM ring buffer |
| **T3** | Full Pipeline Thread Split | Each pipeline stage (Capture/Filter/Detect/Measure) on separate core |

### Additional Approaches (Non-Decision)

| ID | Approach | Quality Attribute |
|----|----------|------------------|
| **A1** | Observer pattern (Qt signals/slots) | Extensibility — new tabs subscribe without modifying pipeline |
| **A2** | Strategy pattern (AudioCapture subclasses) | Extensibility, Testability — Live/Playback/Sim swap without pipeline change |
| **A3** | Ring buffer (SignalBuffer) | Performance — lock-free PCM handoff between capture callback and DSP |

---

## 4. Quality Attribute Utility Tree

The utility tree is a top-down vehicle for describing and prioritizing quality attribute requirements. Scenarios are rated on two dimensions: **(Technical Difficulty or Risk, Business Importance)** using H (High), M (Medium), L (Low).

```
Utility
├── Performance
│   ├── Real-Time Throughput
│   │   └── (H,H) QAS-1: At 28,800 BPH on RPi 5, the audio processing pipeline
│   │               completes each beat cycle within the 21 ms exec deadline
│   │               with a miss rate < 5% (currently 43% — FAIL)
│   └── End-to-End Latency
│       └── (H,H) QAS-2: From audio capture to GUI display update,
│                   end-to-end latency < 200 ms under normal operating conditions
│
├── Correctness
│   ├── Beat detection accuracy
│   │   └── (H,H) QAS-3: Detected Rate (s/d) matches WeiShi 1000 reference
│   │               within ±2 s/d for a 28,800 BPH watch in Live mode
│   └── Measurement consistency
│       └── (M,H) QAS-4: Under identical conditions (same watch, same position),
│                   Beat Error reported by TimeGrapher stays within ±0.1 ms
│                   across 10 consecutive 60-second measurement runs
│
└── Extensibility
    └── New graph addition
        └── (L,M) QAS-5: A developer can add a new graph tab (new Qt widget
                    subscribing to MeasurementEngine output) by modifying
                    ≤ 3 existing files, with no changes to the audio pipeline
```

### Utility Tree — Tabular Form

| QA | Sub-Attribute | Priority (Risk, Business) | Scenario |
|----|--------------|--------------------------|----------|
| Performance | Real-Time Throughput | **(H, H)** | QAS-1: exec deadline miss < 5% at 28,800 BPH on RPi 5 |
| Performance | End-to-End Latency | **(H, H)** | QAS-2: capture→display < 200 ms under normal load |
| Correctness | Beat Detection Accuracy | **(H, H)** | QAS-3: Rate deviation within ±2 s/d vs WeiShi 1000 |
| Correctness | Measurement Consistency | **(M, H)** | QAS-4: Beat Error within ±0.1 ms over 10 runs |
| Extensibility | New Graph Addition | **(L, M)** | QAS-5: Add new tab touching ≤ 3 files, zero pipeline changes |

---

## 5. Analysis of Architectural Approaches

### 5.1 Approach Analysis per Scenario

#### QAS-1 / QAS-2 — Performance

**Question probed**: How does the current rendering-in-audio-thread structure affect the exec deadline? What is the sensitivity of deadline-miss rate to rendering approach?

| Approach | Analysis |
|----------|----------|
| **R1 (Lazy Rendering)** | Removes `plot` cost for 10 of 11 tabs when only 1 is visible. exec drops from ~20 ms to ~4 ms in typical use. Risk: all 11 tabs open simultaneously recovers the bottleneck — `isVisible()` guard returns true for 11 tabs. |
| **R2 (Timer-Decoupled)** | Completely removes rendering from the audio thread budget. exec becomes audio-only (≈ 4 ms measured DSP). FPS controlled independently at 20 Hz. Requires QTimer lifecycle management alongside the pipeline. |
| **R3 (Double-Buffer Async)** | Maximum separation, but QPixmap must be created on the UI thread — the design requires careful mutex handling. Implementation complexity is M3-level, not MVP. |
| **T1 (SCHED_RR + Affinity)** | Does not reduce exec time. Reduces OS-induced jitter, improving Dropped Block metric. Must be combined with R1 or R2 to address the bottleneck. |
| **T2 (DSP Offload Thread)** | Capture callback becomes write-only (< 1 ms). DSP on cpu1 runs without competing with capture SCHED_RR thread. Synergizes with R1/R2. Adds PCM ring buffer between capture and DSP. |

**Chosen combination for M2/M3**: **R1 + T1** (MVP, lowest risk); **R2 + T2** (target for M3, best structural foundation).

#### QAS-3 / QAS-4 — Correctness

**Question probed**: Does rendering decoupling (R1/R2) introduce timing errors into beat event detection?

| Approach | Analysis |
|----------|----------|
| **R1/R2** | Beat detection (BeatDetector) and measurement (MeasurementEngine) remain on the audio thread. Rendering is separated from the measurement path. No impact on T1/T3 event timestamps. |
| **T2** | The PCM ring buffer introduces a bounded additional latency (buffer depth × sample period). At 48,000 sps and a 1024-sample buffer, this is ~21 ms — within the beat interval. The buffer must be sized to avoid phase drift in T1 onset detection. |
| **A3 (SignalBuffer)** | Lock-free ring buffer already in place. T2 adds a second ring buffer upstream (PCM). The two buffers must be sized and coordinated to avoid cumulative latency exceeding the beat interval. |

#### QAS-5 — Extensibility

**Question probed**: Does adding the DSP Offload Thread (T2) or the new rendering guard (R1) break the "add tab by touching ≤ 3 files" contract?

| Approach | Analysis |
|----------|----------|
| **A1 (Observer / Qt signals)** | MeasurementEngine emits `beatResult(Measurement)` signal. New tabs connect without modifying the pipeline. The QAS-5 contract holds for any rendering approach. |
| **R1** | Adding the `isVisible()` guard to `updateData()` must be applied consistently to each new tab. If the guard is part of a shared base class (`GraphTab`), new subclasses inherit it automatically — QAS-5 contract holds. If applied per-file, each new tab touches 1 file, still within contract. |
| **T2** | DSP Offload Thread is an internal pipeline change. New tabs subscribe to MeasurementEngine signal — unaffected by threading below the domain layer. QAS-5 contract holds. |

---

### 5.2 Risks, Non-Risks, Sensitivity Points, and Tradeoff Points

#### Risks

A **risk** is a potentially problematic architectural decision.

| ID | Risk | Related QA | Architectural Decision |
|----|------|-----------|----------------------|
| **R-1** | `plot` rendering is called inside the audio thread's beat callback. At 11 active tabs, `plot` consumes 79% of the exec budget (16 ms / 21 ms). If the team deploys without a rendering strategy change, the 43% deadline-miss rate will persist, breaking QAS-1. | QAS-1, QAS-2 | Current architecture: no rendering separation |
| **R-2** | DSP Offload Thread (T2) introduces a PCM ring buffer between AudioCapture and FilterChain. If the buffer depth is sized incorrectly (too shallow: Dropped Blocks; too deep: added latency beyond the beat interval), beat event detection accuracy may degrade, breaking QAS-3. This is a sensitivity point without a closed-form solution — the depth must be set conservatively and made configurable. | QAS-2, QAS-3 | T2 — PCM ring buffer depth |
| **R-3** | Thermal throttling at 85°C reduces the effective CPU clock below the nominal 2.4 GHz. Even with R1+T1 applied, exec headroom may erode under sustained operation. No cooling solution is currently specified. | QAS-1 | Hardware/deployment constraint — no active cooling |

#### Non-Risks

A **non-risk** is an architectural decision that, upon analysis, is deemed safe.

| ID | Non-Risk | Rationale |
|----|----------|-----------|
| **NR-1** | Qt signals/slots mechanism (Observer pattern) for delivering `Measurement` results to graph tabs does not introduce timing risk. Qt's queued signal connection (cross-thread) is FIFO and bounded in latency by the Qt event loop, which is well under 1 ms at 28,800 BPH intervals of 125 ms. | QAS-2, QAS-5 |
| **NR-2** | Adding R1's `isVisible()` guard to each tab's `updateData()` does not break the extensibility contract (QAS-5). The guard is a one-line addition per tab file; it does not modify the pipeline or any shared interface. If placed in a shared `GraphTab` base class, it is zero-overhead for new tabs. | QAS-5 |
| **NR-3** | AudioCapture subclasses (LiveCapture / PlaybackCapture / SimCapture) exchanged via Strategy pattern do not affect beat detection correctness. All three produce the same PCM sample stream at the same sample rate. The domain layer (BeatDetector, MeasurementEngine) is fully decoupled from the capture source. | QAS-3, QAS-4 |

#### Sensitivity Points

A **sensitivity point** is a property of one or more components that is critical for achieving a particular quality attribute response.

| ID | Sensitivity Point | Components | Quality Attribute |
|----|-----------------|------------|------------------|
| **SP-1** | The `isVisible()` check in `TraceDisplay::updateData()` (and equivalent in all tab `updateData()` methods) is the key leverage point for exec deadline compliance. If this guard is absent from any active tab, that tab's `paintEvent()` fires inside the audio thread, restoring the bottleneck. | GraphTab subclasses (11 tabs) | QAS-1 |
| **SP-2** | The PCM ring buffer depth between AudioCapture and FilterChain (T2 approach) directly determines the tradeoff between Dropped Block rate and additional end-to-end latency. This is the most sensitive parameter for simultaneously satisfying QAS-1 and QAS-2. | SignalBuffer / PCM Ring Buffer | QAS-1, QAS-2 |
| **SP-3** | SCHED_RR priority value assigned to the audio thread determines its preemption behavior over system daemons (PulseAudio, kernel tasks). Priority too high: system instability; too low: OS jitter not absorbed. | AudioCapture thread configuration | QAS-1 |
| **SP-4** | The 20 FPS timer period in Option R2 determines rendering overhead under all tab-open conditions. At 28,800 BPH (8 beats/s), a 50 ms timer fires less often than beats arrive — a safe budget. At 36,000 BPH (10 beats/s), the ratio tightens; the timer period should be tunable. | QTimer in GraphTabManager (R2) | QAS-1, QAS-2 |

#### Tradeoff Points

A **tradeoff point** is a property that affects two or more quality attributes, where one improves while the other degrades.

| ID | Tradeoff | Improves | Degrades | Decision |
|----|----------|----------|----------|---------|
| **TP-1** | **Rendering decoupling (R2 vs R1)**: Timer-decoupled rendering (R2) completely removes rendering from the audio path (best for QAS-1/QAS-2) but all tabs update at a fixed FPS regardless of beat rate, increasing CPU load compared to R1's event-driven approach. | QAS-1 (deadline miss ↓), QAS-2 (latency stability ↑) | Resource usage (CPU for timer wake-ups even at low BPH) | R1 for MVP; R2 for M3 |
| **TP-2** | **DSP Offload Thread (T2)**: Separating AudioCapture from DSP (T2) distributes CPU load across two cores, improving QAS-1 and reducing Dropped Blocks, but adds a PCM ring buffer that introduces a bounded propagation delay, slightly increasing end-to-end latency (QAS-2). | QAS-1 (Dropped Block ↓, exec headroom ↑) | QAS-2 (additional ~21 ms ring buffer propagation) | Accept the tradeoff; 21 ms is well within the 200 ms QAS-2 target |
| **TP-3** | **Tab count and exec budget (QAS-1 vs QAS-5)**: Extensibility (QAS-5) encourages adding more graph tabs. Each additional active-visible tab adds proportional rendering cost inside the audio thread under the current architecture. With R1 applied, only 1 tab renders per beat — the per-tab extensibility cost collapses to near-zero. Without R1/R2, QAS-1 and QAS-5 are in direct conflict. | QAS-5 (tabs addable freely with R1/R2) | QAS-1 (each added active tab increases exec cost without rendering guard) | R1/R2 resolves the conflict — both quality attributes can be met simultaneously |

---

## 6. Risk Themes

Risk themes are cross-cutting patterns distilled from individual risks.

### Theme 1: Rendering-Audio Coupling (Critical)

Risks: R-1, Sensitivity Points: SP-1, SP-4, Tradeoffs: TP-1, TP-3

The most pervasive risk theme is that rendering and real-time audio processing share the same thread of execution. This single structural decision is responsible for the 43% deadline-miss rate observed in EXP-02. It also creates a hidden tension between Extensibility (QAS-5) and Real-Time Performance (QAS-1): every new graph tab added makes the system harder to keep real-time unless the rendering guard is enforced.

**Mitigation**: Apply R1 (Lazy Rendering) immediately as MVP. Plan R2 (Timer-Decoupled) for M3 to fully resolve the coupling.

### Theme 2: Single-Core Concentration (High)

Risks: R-1 (contributing), R-3, Sensitivity Points: SP-2, SP-3

The entire signal processing pipeline (AudioCapture → FilterChain → BeatDetector → MeasurementEngine → Rendering) runs on a single core (cpu2 at 91% utilization), while three cores idle. Combined with thermal throttling, this concentrates failure risk on a single execution context. A spike in any one component propagates directly to the beat deadline.

**Mitigation**: Apply T1 (SCHED_RR + CPU Affinity) for immediate isolation of the audio thread. Apply T2 (DSP Offload) for M3 to distribute load across two cores.

### Theme 3: Threading Parameter Sensitivity (Medium)

Risks: R-2, Sensitivity Points: SP-2, SP-3

The proposed threading enhancements (T1: SCHED_RR priority, T2: PCM ring buffer depth) contain parameters whose values are critical but cannot be derived analytically. Incorrect SCHED_RR priority values can cause system instability; incorrect ring buffer depths can either cause Dropped Blocks or add excessive latency. These parameters are identified as sensitivity points (SP-2, SP-3) and must be set conservatively and made configurable.

**Mitigation**: SCHED_RR priority should be set at 80 (below kernel real-time tasks) and the PCM ring buffer depth parameterized so it can be adjusted without recompilation. The EXP-02 results inform initial values.

---

## 7. Suggested Next Steps

| Priority | Action | Addresses |
|----------|--------|-----------|
| **Immediate** | Apply R1 (`isVisible()` guard to all 11 tab `updateData()` methods) | Risk R-1, Theme 1 |
| **Immediate** | Apply T1 (SCHED_RR priority 80, pin audio thread to cpu0) | Theme 2, SP-3 |
| **Before M3** | Implement R2 (Timer-Decoupled Rendering) as structural improvement over R1 | Theme 1, TP-1 |
| **Before M3** | Implement T2 (DSP Offload Thread) with parameterized PCM ring buffer depth | Theme 2, SP-2 |
| **Before M3** | Add active cooling (heatsink + fan) to address thermal throttling | Risk R-3 |
| **M3 Demo** | Demonstrate exec deadline miss < 5% on RPi 5 with all graphs active | QAS-1 |
| **M3 Demo** | Measure and report end-to-end latency (capture→display) | QAS-2 |

---

## Appendix A: Scenario Traceability

| Scenario | Architectural Approach | Risk/Non-Risk/SP/TP |
|----------|----------------------|---------------------|
| QAS-1 (exec deadline < 5% miss) | R1 or R2 + T1 or T2 | R-1, SP-1, SP-2, SP-3, TP-1, TP-3 |
| QAS-2 (latency < 200 ms) | R1/R2 (remove render from audio path) + T2 (bounded ring buffer) | SP-2, TP-2 |
| QAS-3 (Rate ±2 s/d accuracy) | A3 (SignalBuffer) + T2 ring buffer sizing | R-2, NR-3 |
| QAS-4 (Beat Error ±0.1 ms consistency) | BeatDetector on stable thread (T1/T2) | NR-3 |
| QAS-5 (Add tab ≤ 3 files) | A1 (Observer/Qt signals) + R1 guard in base class | NR-1, NR-2, TP-3 |

## Appendix B: ATAM Steps Mapping

| ATAM Step | TimeGrapher Evaluation Activity | Section |
|-----------|--------------------------------|---------|
| Step 1: Present the ATAM | Method and outputs described | Section 1 |
| Step 2: Present business goals | LG Training context, RPi deployment, extensibility | Section 2.1 |
| Step 3: Present architecture | 4-layer structure, threading model, EXP-02 baseline | Section 2.2–2.3 |
| Step 4: Identify architectural approaches | R1/R2/R3 (rendering), T1/T2/T3 (threading), A1/A2/A3 | Section 3 |
| Step 5: Generate quality attribute utility tree | QAS-1 through QAS-5 with (Risk, Business) priority | Section 4 |
| Step 6: Analyze architectural approaches | QAS-specific analysis, risks/non-risks/SP/TP | Section 5 |
| Steps 7–8: Brainstorm/prioritize scenarios + analyze | Consolidated in utility tree and Section 5 analysis | Sections 4–5 |
| Step 9: Present results | Summary in this document | All sections |
