# Architectural Drivers — TimeGrapher

---

This document answers the five mentor review questions in order:

1. Are QA requirements "actionable"? (Expressed so the team can verify a design supports them?)
2. Do the drivers relate to the overall objectives of the project?
3. Are the measures clearly derived from the overall goals?
4. Are the functional requirements understood?
5. Are requirements prioritized?

---

## 1. Project Objectives

TimeGrapher analyzes acoustic signals (beat noise) from the escapement vibration of a mechanical watch to diagnose watch performance in real time. It captures tick-tock sounds via microphone, detects A(T1)·C(T3) events, and computes Rate, Amplitude, and Beat Error for GUI display.

**Team's core objectives (in priority order):**

| Rank | Objective | Description |
|:---:|-----------|-------------|
| **1st** | **Accurate Measurement** | Priority is to measure Rate / Amplitude / Beat Error accurately for at least some BPH range. Sacrificing accuracy to cover more BPH is not an acceptable trade-off |
| **2nd** | **Wider BPH Coverage** | Extend accurate support to higher BPH (faster watches) while preserving accuracy |
| **3rd** | **Extensible Architecture** | Enable parallel development of 11 graphs within 5 weeks |
| **4th** | **Architecture Principles** | Apply CMU MSE software architecture design principles |

---

## 2. Functional Requirements

| ID | Functional Requirement | Tier | Priority | Status |
|----|----------------------|:----:|:-------:|:------:|
| FR-01 | The system shall detect T1(A) and T3(C) acoustic events | — | HIGH | ✅ Implemented (`Detector.cpp`) |
| FR-02 | The system shall compute Rate (s/d), Amplitude (°), Beat Error (ms), and BPH | — | HIGH | ✅ Implemented |
| FR-03 | The system shall support Live / Playback / Sim operating modes | — | HIGH | ✅ Implemented |
| FR-04 | The system shall apply signal filtering (HPF + Envelope) | — | HIGH | ⚠️ Partial (HPF only) |
| FR-05 | The system shall provide Trace Display (real-time Rate + Amplitude recording) | **Core** | HIGH | ❌ Not implemented |
| FR-06 | The system shall provide Rate & Amplitude Stability / Vario (Min/Max/Avg/σ) | **Core** | HIGH | ❌ Not implemented |
| FR-07 | The system shall provide Beat Error Display & Diagnostic Trace | **Core** | HIGH | ❌ Not implemented |
| FR-08 | The system shall display signal quality warnings (No signal / Noisy signal) | — | MEDIUM | ❌ Not implemented |
| FR-09 | The system shall support Pause + timeline navigation — the user shall be able to freeze the live display and move backward/forward through captured beat data using a cursor; background data collection shall continue while paused | — | LOW | ❌ Not implemented |
| FR-10 | The system shall measure Rate deviation across multiple watch positions (Dial-Up, Crown-Left, etc.) | **Required** | MEDIUM | ❌ Not implemented |
| FR-11 | The system shall display the beat waveform in oscilloscope style (Scope 1: raw, Scope 2: filtered) | **Required** | MEDIUM | ❌ Not implemented |
| FR-12 | The system shall sequentially compare and display measurement results across multiple positions | **Required** | MEDIUM | ❌ Not implemented |
| FR-13 | The system shall record and display Rate/Amplitude trends over hours to days | **Stretch** | LOW | ❌ Not implemented |
| FR-14 | The system shall display escapement action analysis with marker-line overlay | **Stretch** | LOW | ❌ Not implemented |
| FR-15 | The system shall display a time-frequency spectrogram of the beat signal | **Stretch** | LOW | ❌ Not implemented |
| FR-16 | The system shall overlay reference and current waveforms with timing markers for comparison | **Stretch** | LOW | ❌ Not implemented |
| FR-17 | The system shall display a trigger-synchronized sweep mode scope | **Stretch** | LOW | ❌ Not implemented |
| FR-18 | The system shall provide a scope view for simultaneously comparing multiple filter combinations | **Stretch** | LOW | ❌ Not implemented |

---

## 3. Quality Attribute Scenarios

### 3.1 Prioritization Criteria

Priority is determined by two axes aligned with the team goal ("accurate data first") and architectural constraint impact.

---

### 3.2 Priority Summary

| Rank | QA | Key Requirement | Business Importance | Technical Difficulty / Risk | One-Line Rationale |
|:----:|----|----------------| :------------------:| :-------------------------:|-------------------|
| **1** | Real-Time Performance | The system shall process each audio block within the block period to prevent Ring Buffer overflow | H | H | Prerequisite for all other QAs — Dropped Block destroys T1/T3 timestamps, making measurement impossible |
| **2** | Low Latency | The system shall maintain end-to-end latency from audio capture to GUI display within 100ms (based on 28,800 BPH) | H | H | Hard threshold defined by beat period; exceeding it causes functional failure; unverified on RPi + Qt |
| **3** | Correctness | The system shall display identical values across all GUI views derived from the same beat data (inter-view deviation = 0) | H | M | Directly tied to team's primary goal (accurate data); QA-C1 structurally guaranteed by Observer pattern; only QA-C2 (noise-condition parameters) remains open |
| **4** | Usability | The system shall distinguish and display ⚠ No signal and ⚠ Noisy signal states separately | M | M | Indirectly supports accurate data collection by alerting users to unreliable measurement conditions; threshold is environment-dependent |
| **5** | Extensibility | The system shall allow new display features to be implemented with minimal file changes and without modifying existing preprocessing logic | M | M | Developer productivity; directly controls schedule risk for 11-graph parallel implementation |

---

### QAS-1: Real-Time Performance — Priority 1 (Business: H / Risk: H)

> ⚠️ **Provisional**: Target SPS and Dropped Block feasibility cannot be confirmed before **EXP-01** measurement.

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (acoustic signal source) |
| **Stimulus** | Continuous audio sample stream via USB microphone |
| **Artifact** | Audio capture pipeline — ALSA driver → Audio Thread → Ring Buffer |
| **Environment** | Raspberry Pi 5 (8GB RAM), Ubuntu 24.04, Live mode, 28,800 BPH mechanical watch, continuous ≥ 10 min, Qt GUI + DSP running concurrently<br>• **Minimum**: 48,000 sps (below this = project failure)<br>• **Objective**: 96,000 sps *(provisional, confirmed by EXP-01)*<br>• **Stretch**: 192,000 sps |
| **Response** | Process each audio block within the block period to prevent Ring Buffer overflow |
| **Response Measure** | Dropped audio block: **0** (no Ring Buffer overflow) |

**Related architecture tactics:**

| Tactic | Rationale |
|--------|-----------|
| Lock-Free Ring Buffer | Eliminates mutex between Audio Thread ↔ DSP Thread → prevents DSP delay from lock contention |
| Priority Scheduling | Elevates audio thread priority → absorbs Linux scheduler jitter |
| Graceful Degradation | Auto-fallback to 48k sps if 96k is unachievable → guarantees Dropped Block = 0 |

---

### QAS-2: Low Latency — Priority 2 (Business: H / Risk: H)

> ⚠️ **Provisional**: Values below cannot be confirmed before **EXP-02** measurement.

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (beat event source) |
| **Stimulus** | Audio sample block capture begins from microphone |
| **Artifact** | Full TimeGrapher application — audio capture → DSP pipeline → Qt GUI rendering |
| **Environment** | Raspberry Pi 5 (8GB RAM), normal operation — Qt GUI active, live capture running |
| **Response** | Waveform, beat markers, and computed Rate·Amplitude·Beat Error displayed in GUI |
| **Response Measure** | See BPH-based latency targets below |

**BPH-based latency targets:**

> Core constraint: end-to-end latency > beat period causes real-time display failure. 80% safety margin applied to absorb OS scheduler jitter and Qt rendering variation.

| BPH | Beat Period | End-to-End Target (80%) | ① capture→process (70%) | ② process→display (30%) |
|-----|:-------:|:-------------------:|:----------------------:|:----------------------:|
| **28,800** | **125 ms** | **< 100 ms** *(Primary target)* | **< 70 ms** | **< 30 ms** |
| 36,000 | 100 ms | 80 ms | 56 ms | 24 ms |
| 43,200 | 83 ms | 66 ms | 46 ms | 20 ms |

**Related architecture tactics:**

| ID | Tactic | Rationale |
|----|--------|-----------|
| LT-01 | Introduce Concurrency (Audio/DSP/GUI thread separation) | Prevents capture callback from being blocked by DSP/GUI |
| LT-02 | Lock-Free Ring Buffer | Eliminates mutex between threads → prevents segment ① latency spikes |
| LT-03 | Reduce Computational Overhead | Eliminate unnecessary operations on the real-time path |
| LT-04 | Lazy Rendering (active tab only) | Reduces Qt main thread rendering load → decreases segment ② latency |

---

### QAS-3: Correctness — Priority 3 (Business: H / Risk: M)

| Item | Detail |
|------|--------|
| **Source** | User (same watch, same conditions) / Noisy environment |
| **Stimulus** | Multiple GUI views displayed simultaneously / beat detection under ambient noise |
| **Artifact** | Rate·Amplitude·Beat Error computation logic and all GUI views / DSP pipeline |
| **Environment** | Normal operating state / environment with ambient noise |
| **Response** | All GUI views display consistent values from same data + beat detection quality maintained under noise |
| **Response Measure** | • **QA-C1**: Value deviation across all views derived from same beat data: **0** — structurally guaranteed by Observer pattern<br>• **QA-C2**: Optimal Detector parameters (`onset_fraction`, `min_peak_fraction`) minimizing Δ across 3 noise conditions — **finalized after EXP-03** |

**Related architecture tactics:**

| Tactic | Linked QA | Rationale |
|--------|:---------:|-----------|
| Observer / Qt Signal-Slot | QA-C1 | MeasurementEngine publishes single `Measurement` struct → all tabs subscribe to the same signal |
| Adaptive threshold (noise floor-based) | QA-C2 | noise_floor = 75th percentile of last 256 ms, reference_peak = median of last 16 beats |
| Detector parameter tuning (EXP-03) | QA-C2 | Optimal onset_fraction / min_peak_fraction confirmed by experiment |

---

### QAS-4: Usability — Priority 4 (Business: M / Risk: M)

> ⚠️ **Provisional**: N·M values and thresholds require experimental confirmation.

| Item | Detail |
|------|--------|
| **Source** | Microphone input signal |
| **Stimulus** | Microphone input outside normal range — no signal or excessive noise |
| **Artifact** | GUI status display area |
| **Environment** | Live mode, normal and abnormal operating states |
| **Response** | Immediate signal quality warning in GUI; auto-cleared on signal recovery |
| **Response Measure** | • Warning displayed within **≤ N seconds** *(confirmed by experiment)*<br>• Warning cleared within **≤ M seconds** *(confirmed by experiment)*<br>• Conditions: `⚠ No signal` / `⚠ Noisy signal` |

**Related architecture tactic:**

| Tactic | Rationale |
|--------|-----------|
| Heartbeat | Reuses existing A/C events as heartbeat → no signal for N seconds triggers `⚠ No signal`; no additional detection logic needed |

---

### QAS-5: Extensibility — Priority 5 (Business: M / Risk: M)

| Item | Detail |
|------|--------|
| **Source** | Developer (adding new graph or display feature) |
| **Stimulus** | Request to add new graph tab (e.g., Spectrogram, Long-Term, Sequence) |
| **Artifact** | Source code (full GUI module structure) |
| **Environment** | Development environment (Qt Creator, Windows PC / RPi), within 5-week project timeline |
| **Response** | New graph widget can be created, registered, and tested independently without modifying preprocessing modules |
| **Response Measure** | • Files changed when adding 1 new graph: **≤ 3** *(confirmed after Observer pattern refactoring)*<br>• Dependencies follow Presentation → Domain direction (direct Signal Processing / Acquisition references = **0**) |

**≤ 3-file rationale:**

| Change type | Target | File count |
|------------|--------|:----------:|
| New graph widget file creation | `GraphFoo.cpp` / `GraphFoo.h` | 1 |
| Register new tab in tab container | `TabPanel.cpp` | 1 |
| Wire up data subscription | `DataBroker.cpp` | 1 |
| **Total** | | **3** |

**Related architecture tactics:**

| Tactic | Rationale |
|--------|-----------|
| Split Module | God Object → separate modules by concern (preprocessing / display / acquisition) |
| Observer / Signal-Slot | Adding new graph only requires adding a subscription — no modification of existing logic |
| Restrict Dependencies (Layered Architecture) | Presentation Layer references Domain Layer only → display layer independently replaceable |

---

## 4. QA–Architecture Tactic Mapping

| QA | Priority | Core Tactic(s) | Open Experiment |
|----|:-------:|---------------|----------------|
| Real-Time Performance | 1 | Lock-Free Ring Buffer, Priority Scheduling, Graceful Degradation | EXP-01 (Dropped Block per SPS) |
| Low Latency | 2 | Thread separation (Concurrency), Lock-Free Ring Buffer, Lazy Rendering | EXP-02 (3-segment timestamp measurement) |
| Correctness | 3 | Observer/Signal-Slot, Adaptive threshold, Detector parameter tuning | EXP-03 (noise × parameter Δ) |
| Usability | 4 | Heartbeat pattern | Threshold experiment (N·M values) |
| Extensibility | 5 | Split Module, Observer/Signal-Slot, Layered Architecture | ≤ 3-file verification experiment |

---

## 5. Design Constraints

| Constraint | Detail | Rationale |
|------------|--------|-----------|
| Target Hardware | Raspberry Pi 5 (ARM64, 16GB RAM) + USB audio sensor + 8" touchscreen | Hardware provided by program |
| Development Environment | Qt Creator (C++/Qt6) | Based on existing codebase (`TimeGrapher_v10.5`) |

---

## 6. Open Issues Summary

| ID | Open Question | Resolution | Affected QA |
|----|--------------|-----------|------------|
| OI-P1 | Can RPi 5 achieve Dropped Block = 0 at 96k sps? | EXP-01 | QAS-1 |
| OI-L1 | What is the actual QAudioSource live callback period? | EXP-02 measurement | QAS-2 |
| OI-L2 | Is ② process→display < 30ms with 11 tabs rendering? | EXP-02 measurement | QAS-2 |
| OI-L3 | After achieving 28,800 BPH target, can the target be raised to 36,000 / 43,200 BPH? | EXP-02 result → team decision | QAS-2 |
| OI-C1 | What Detector parameter values minimize Δ across 3 noise conditions? | EXP-03 | QAS-3 |
| OI-U1 | What are the N and M second values for warning onset/clear? | Watch removal/restore experiment | QAS-4 |
| OI-U2 | What are the thresholds for No signal / Noisy signal? | Multi-environment threshold search | QAS-4 |
