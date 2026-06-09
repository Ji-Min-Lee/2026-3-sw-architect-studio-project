# Architectural Drivers — TimeGrapher

**Team**: Blue Sky (Team 3) | **Milestone**: M1 | **Date**: 2026-06-07

---

## 1. Functional Requirements

| ID | Functional Requirement | Tier | Priority | Status |
|----|----------------------|:----:|:-------:|:------:|
| FR-01 | The system shall detect T1(A) and T3(C) acoustic events | — | HIGH | ✅ Implemented (`Detector.cpp`) |
| FR-02 | The system shall compute Rate (s/d), Amplitude (°), Beat Error (ms), and BPH | — | HIGH | ✅ Implemented |
| FR-03 | The system shall support Live / Playback / Sim operating modes | — | HIGH | ✅ Implemented |
| FR-04 | The system shall apply signal filtering (HPF + Envelope) | — | HIGH | ⚠️ Partial (HPF only) |
| FR-05 | The system shall provide Trace Display (real-time Rate + Amplitude recording) | **Core** | HIGH | ❌ Not implemented |
| FR-06 | The system shall provide Rate & Amplitude Stability / Vario (Min/Max/Avg/σ) | **Core** | HIGH | ❌ Not implemented |
| FR-07 | The system shall provide Beat Error Display & Diagnostic Trace | **Core** | HIGH | ❌ Not implemented |
| FR-08 | The system shall display signal quality warnings (No signal / Noisy signal) | **Core** | MEDIUM | ❌ Not implemented |
| FR-09 | The system shall support Pause + timeline navigation — the user shall be able to freeze the live display and move backward/forward through captured beat data using a cursor; background data collection shall continue while paused | **Core** | LOW | ❌ Not implemented |
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

## 2. Quality Attribute Scenarios

### 2.1 Prioritization Criteria

Priority is determined by two axes aligned with the team goal ("accurate data first") and architectural constraint impact.

| Axis | Description |
|:-------------|:-----------------|
| **Business Importance** | Direct relevance to team goal ("accurate data"). Does its failure eliminate the project's purpose entirely? |
| **Technical Difficulty / Risk** | Is achievement uncertain, verifiable only by experiment, and does failure cause total system functional collapse? |

---

### 2.2 Priority Summary
Each quality attribute is rated as High, Medium, or Low, and prioritized accordingly.
| Rank | QA | Key Requirement | Business Importance | Technical Difficulty / Risk | One-Line Rationale |
|:----:|----|----------------| :------------------:| :-------------------------:|-------------------|
| **1** | Real-Time Performance | The system shall detect and process every beat without interruption | H | H | Any missed beat breaks the measurement chain — this is the foundation all other QAs depend on |
| **2** | Low Latency | The system shall display beat data in real-time without perceptible delay | H | H | If display lags behind the beat, real-time feedback is lost and accurate data collection becomes impossible |
| **3** | Correctness | The system shall deliver consistent and accurate measurement values across all views and noise conditions | H | M | Accurate data requires both consistent values across views and reliable detection under noise |
| **4** | Usability | The system shall alert users to signal quality issues promptly and clearly | M | M | Timely warnings let users correct poor measurement conditions before data quality degrades |
| **5** | Extensibility | The system shall support independent addition of new graph features without modifying existing modules | M | M | A modular architecture is the only way to sustain parallel development of 11 graphs within the project timeline |

---

### QAS-1: Real-Time Performance — Priority 1 (Business: H / Risk: H)

> ⚠️ **Provisional**: Target SPS and Missed Beat = 0 achievement cannot be confirmed until **EXP-01** measurements are conducted.

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH |
| **Environment** | System running on Raspberry Pi configured at 96,000 sps |
| **Artifact** | The system (audio capture, processing, and display pipeline) |
| **Response** | The system continuously acquires, processes, and displays beat measurements without interruption |
| **Measure** | Missed beat count = 0 over a 10-minute session; no latency spikes |

> Higher SPS shortens the block period, reducing the time budget for DSP. SPS is the **cause** (environment condition) of Missed Beats; Ring Buffer overflow is the **measurement point**.

**SPS ↔ Block Period Relationship** (SPS is an environment condition):

| SPS (Environment) | Block Period | T1 Detection Resolution | Note |
|:-------------:|:-------:|:----------:|-----|
| 48,000 | ~20 ms | 20.8 µs/sample | Minimum — fallback if 96k is unachievable |
| **96,000** | **~10 ms** | **10.4 µs/sample** | **Objective** |
| 192,000 | ~5 ms | 5.2 µs/sample | Stretch — RPi performance unverified |

**Actionability Assessment:**

| Criterion | Result |
|------|------|
| Is it measurable? | ✅ Missed Beat count — directly measurable |
| Is the pass/fail criterion clear? | ✅ Missed Beat = 0 means PASS |
| Is it verifiable by experiment? | ✅ EXP-01: 48k/96k/192k sps × 10-minute continuous run |
| Does it drive architecture decisions? | ✅ Lock-Free Ring Buffer, Priority Scheduling, Graceful Degradation |

**Project Goal Connection:** Missed Beat → T1/T3 timestamp lost → Rate / Amplitude / Beat Error computation fails → team primary goal collapses. **Prerequisite for all other QAs.**

**Related Architectural Tactics:**

| Tactic | Rationale |
|--------|-----------|
| Lock-Free Ring Buffer | Eliminates mutex between Audio Thread ↔ DSP Thread → prevents DSP delays from lock contention |
| Increase Ring Buffer Size | Larger buffer capacity absorbs temporary DSP processing spikes → reduces Ring Buffer overflow risk without requiring lower sps |
| Priority Scheduling | Elevates audio thread scheduling priority → absorbs Linux scheduler jitter |
| Graceful Degradation | Auto-fallback to 48k sps if 96k is unachievable → guarantees Missed Beat = 0 |

---

### QAS-2: Low Latency — Priority 2 (Business: H / Risk: H)

> ⚠️ **Provisional**: Target latency values cannot be confirmed until **EXP-02** measurements are conducted.

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | A single beat event (tick) occurs at the microphone |
| **Environment** | System running on Raspberry Pi under normal operating conditions |
| **Artifact** | The system (audio capture, processing, and display pipeline) |
| **Response** | The corresponding waveform, markers, and computed values appear on the GUI |
| **Measure** | End-to-end latency from acoustic event to display update is < 100ms (worst-case); missed beat count = 0 |

**Latency Targets by BPH:**

> Core constraint: if end-to-end latency > beat period, real-time display function collapses. 80% safety margin applied (to absorb OS scheduler jitter and Qt rendering variability).

| BPH | Beat Period | end-to-end Target (80%) | ① capture→process (70%) | ② process→display (30%) |
|-----|:-------:|:-------------------:|:----------------------:|:----------------------:|
| **28,800** | **125 ms** | **< 100 ms** *(Primary target)* | **< 70 ms** | **< 30 ms** |
| 36,000 | 100 ms | 80 ms | 56 ms | 24 ms |
| 43,200 | 83 ms | 66 ms | 46 ms | 20 ms |

> **Primary target (Option A)**: 28,800 BPH baseline — end-to-end < 100 ms (based on 3 available watch models)
> **Stretch (Option B)**: Consider raising to 36,000 / 43,200 BPH after EXP-02 results

**Segment Separation Rationale:**

| Segment | Time Type | Measurement Boundary | Bottleneck Cause |
|------|---------|---------|---------|
| ① capture→process | **Wait** (OS callback period ~20ms) + **Execute** (DSP processing) | ALSA callback received → T1/T3 event timestamp | OS callback period (~20ms), Ring Buffer wait, DSP processing time |
| ② process→display | **Execute** (Qt rendering) | T1/T3 event timestamp → GUI `paintEvent()` complete | Qt rendering time, FPS, rendering thread contention |
| ③ end-to-end | Wait + Execute combined | ALSA callback received → GUI screen update complete | ①+② combined |

**Actionability Assessment:**

| Criterion | Result |
|------|------|
| Is it measurable? | ✅ Insert 3 timestamps (TS1/TS2/TS3) to measure each segment directly |
| Is the pass/fail criterion clear? | ✅ end-to-end < 100 ms (28,800 BPH baseline) |
| Is it verifiable by experiment? | ✅ EXP-02: 3 segments × 3 SPS levels × 2 tab configurations × 10 minutes |
| Does it drive architecture decisions? | ✅ Thread separation, Lock-Free Ring Buffer, Lazy Rendering |

**Project Goal Connection:** If end-to-end latency exceeds the beat period, real-time display is no longer possible → the real-time feedback loop required for the team's primary goal (accurate data) collapses. Latency accumulation also indirectly affects T1/T3 timestamp accuracy.

**Related Architectural Tactics:**

| Tactic | Rationale |
|--------|-----------|
| Introduce Concurrency (Audio/DSP/GUI thread separation) | Prevents capture callback from being blocked by DSP/GUI |
| Lock-Free Ring Buffer | Eliminates mutex between threads → prevents latency spikes in segment ① |
| Reduce Computational Overhead | Remove unnecessary computation from the real-time path |

---

### QAS-3: Correctness — Priority 3 (Business: H / Risk: M)

| Item | Detail |
|------|--------|
| **Source** | User (same watch, same conditions) / Noisy environment |
| **Stimulus** | Multiple GUI views displayed simultaneously / beat detection under ambient noise |
| **Artifact** | Rate·Amplitude·Beat Error computation logic and all GUI views / DSP pipeline |
| **Environment** | Normal operating state / environment with ambient noise |
| **Response** | All GUI views display consistent values from same data + beat detection quality maintained under noise |
| **Response Measure** | • **QA-C1**: Value deviation across all views derived from same beat data: **0** — structurally guaranteed by Observer pattern<br>• **QA-C2**: Optimal Detector parameters (`onset_fraction`, `min_peak_fraction`) minimizing Δ across 3 noise conditions — **confirmed by experiment** |

**Actionability Assessment:**

| Criterion | QA-C1 | QA-C2 |
|------|:-----:|:-----:|
| Is it measurable? | ✅ Compare values across views (deviation = 0) | ✅ Δ Rate / Δ Amplitude / Δ Beat Error |
| Is the pass/fail criterion clear? | ✅ Auto-PASS when Observer pattern is applied | ⚠️ Acceptable Δ values TBD — confirmed after EXP-03 |
| Is it verifiable by experiment? | ✅ Structurally guaranteed (no experiment needed) | ✅ EXP-03: 3 noise conditions × Detector parameter combinations |
| Does it drive architecture decisions? | ✅ Observer / Qt Signal-Slot | ✅ Adaptive threshold, parameter tuning |

**Project Goal Connection:** Supports the team's primary goal (accurate data) from two angles. QA-C1 guarantees "the same accurate value in every view." QA-C2 maintains beat detection quality under noise, ensuring accuracy structurally.

**Related Architectural Patterns:**

| Pattern | Linked QA | Rationale |
|---------|:--------:|-----------|
| Observer / Qt Signal-Slot (GoF) | QA-C1 | MeasurementEngine publishes a single Measurement struct → all tabs subscribe to the same signal |
| Pipes and Filters (POSA) | QA-C2 | Raw PCM → HPF → Envelope → Detector unidirectional pipeline maintains beat detection quality |

---

### QAS-4: Usability — Priority 4 (Business: M / Risk: M)

> ⚠️ **Provisional**: N and M values and thresholds require confirmation by experiment.

| Item | Detail |
|------|--------|
| **Source** | Microphone input signal |
| **Stimulus** | Microphone input outside normal range — no signal or excessive noise |
| **Artifact** | GUI status display area |
| **Environment** | Live mode, normal and abnormal operating states |
| **Response** | Immediate signal quality warning in GUI; auto-cleared on signal recovery |
| **Response Measure** | • Warning displayed within **≤ N seconds** *(confirmed by experiment)*<br>• Warning cleared within **≤ M seconds** *(confirmed by experiment)*<br>• Conditions: `⚠ No signal` / `⚠ Noisy signal` |

**Actionability Assessment:**

| Criterion | Result |
|------|------|
| Is it measurable? | ✅ Time elapsed until warning is displayed (in seconds) |
| Is the pass/fail criterion clear? | ⚠️ N and M values TBD — confirmed after experiment |
| Is it verifiable by experiment? | ✅ Watch removal/restore experiment to measure N·M; multi-environment threshold search |
| Does it drive architecture decisions? | ✅ Heartbeat pattern, warning state management logic |

**Project Goal Connection:** Users immediately recognize unreliable measurement conditions → take corrective action (repositioning watch, etc.) → accurate data collection enabled. Indirectly supports team primary goal.

**Related Architectural Tactic:**

| Tactic | Rationale |
|--------|-----------|
| Heartbeat | Reuses existing A/C events as heartbeat → no event for N seconds signals loss of signal; no separate detection logic needed |

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

**Rationale for "≤ 3 Files":**

| Change Type | Target | File Count |
|---------|------|:------:|
| Create new graph widget file | `GraphFoo.cpp` / `GraphFoo.h` | 1 |
| Register new tab in tab container | `TabPanel.cpp` | 1 |
| Connect data subscription | `DataBroker.cpp` | 1 |
| **Total** | | **3** |

**Actionability Assessment:**

| Criterion | Result |
|------|------|
| Is it measurable? | ✅ Directly measured via `git diff --stat` — file count |
| Is the pass/fail criterion clear? | ✅ ≤ 3 files = PASS; 4+ files = FAIL (insufficient modularization) |
| Is it verifiable by experiment? | ✅ Measured after Observer pattern refactoring by actually adding a new graph |
| Does it drive architecture decisions? | ✅ Split Module, Observer/Signal-Slot, Restrict Dependencies |

**Project Goal Connection:** Directly controls schedule risk of implementing 11 graphs in Weeks 3–4. Each developer can implement and test their assigned graph independently → team development velocity maintained.

**Related Architectural Tactics:**

| Tactic | Rationale |
|--------|-----------|
| Split Module | God Object → module separation by concern (preprocessing · display · acquisition) |
| Observer / Signal-Slot | Adding a new graph requires only a new subscription — no modification of existing logic |
| Restrict Dependencies (Layered Architecture) | Presentation Layer references only Domain Layer → display layer can be replaced independently |

---

## 3. Design Constraints

Design Constraints = already-decided design decisions. Not subject to architectural discussion; cannot be changed.

| Constraint | Detail | Rationale |
|------------|--------|-----------|
| Target Hardware | Raspberry Pi 5 (ARM64, 16GB RAM) + USB audio sensor + 8" touchscreen | Hardware provided by program |
| Development Environment | Qt Creator (C++/Qt6) | Based on existing codebase (`TimeGrapher_v10.5`) |
