# Architectural Approaches

**Milestone**: M1 | **Due**: 2026-06-09 | **Status**: [ ] Draft / [ ] Final

---

## 1. Architecture Overview

> Provide a high-level description of the overall system structure. What are the major subsystems and how do they interact?

```
[Watch + Microphone]
        ↓ USB audio (PCM)
[Signal Acquisition]  ←→  AlsaMixer (AGC disabled)
        ↓ raw PCM blocks
[Signal Processing]   ←  LP/HP Filter chain
        ↓ filtered signal
[Beat Event Detection]  →  T1 (A), T3 (C) timestamps
        ↓ beat events
[Measurement Calculation]  →  Rate, Amplitude, Beat Error, BPH
        ↓ measurement data
[Visualization Layer]  →  Multiple graph tabs (Qt Widgets)
        ↓
[Qt GUI / Main Window]  →  Touchscreen display (1280×800)
```

Operating Modes: **Live** | **Playback** | **Sim**

---

## 2. Key Architectural Approaches

### 2.1 Pattern: Pipeline / Pipe-and-Filter

| | |
|---|---|
| **Pattern** | Pipe-and-Filter (signal processing pipeline) |
| **Applied To** | Signal Acquisition → Processing → Detection → Calculation |
| **Rationale** | Each stage has a single responsibility; stages can be replaced or reordered independently |
| **Trade-off** | Latency from stage buffering; compensated by keeping buffer sizes minimal |
| **Linked Driver** | QAR-04 (Extensibility), QAR-02 (Low Latency) |

### 2.2 Pattern: Observer / Event-Driven (Measurement Updates)

| | |
|---|---|
| **Pattern** | Observer / Qt Signal-Slot |
| **Applied To** | Measurement Calculation → Visualization Layer |
| **Rationale** | Decouples calculation from display; new graph tabs subscribe without modifying calculation code |
| **Trade-off** | Slightly more indirection; acceptable for this use case |
| **Linked Driver** | QAR-04 (Extensibility) |

### 2.3 Tactic: Concurrency — Background Processing Thread

| | |
|---|---|
| **Tactic** | Separate UI thread from signal processing thread |
| **Applied To** | Signal acquisition and beat detection run on background thread(s); Qt GUI on main thread |
| **Rationale** | Prevent UI jank from blocking signal processing; meet real-time performance target |
| **Trade-off** | Thread synchronization complexity; use Qt thread-safe queues |
| **Linked Driver** | QAR-01 (Real-Time Performance), QAR-02 (Low Latency) |

### 2.4 Tactic: Layered Architecture for Extensibility

| | |
|---|---|
| **Tactic** | Strict layer separation: Acquisition / Processing / Domain / Presentation |
| **Applied To** | Overall module structure |
| **Rationale** | New graphs (Presentation layer) added without touching Acquisition or Processing layers |
| **Trade-off** | Requires up-front discipline in module design |
| **Linked Driver** | QAR-04 (Extensibility), QAR-03 (Correctness — same data source for all views) |

### 2.5 Tactic: Graceful Degradation for Performance

| | |
|---|---|
| **Tactic** | Configurable sample rate with fallback levels (192k → 96k → 48k sps) |
| **Applied To** | Signal acquisition and processing pipeline |
| **Rationale** | RPi may not sustain highest sample rate; system should degrade predictably |
| **Trade-off** | Lower sample rate reduces timing resolution |
| **Linked Driver** | QAR-01 (Real-Time Performance) |

---

## 3. Architecture ↔ Driver Mapping

| Driver | Architectural Approach |
|--------|----------------------|
| QAR-01 Real-Time Performance | Background processing thread (2.3), Graceful degradation (2.5) |
| QAR-02 Low Latency | Pipe-and-Filter with minimal buffering (2.1), Background thread (2.3) |
| QAR-03 Measurement Accuracy | Layered design — all views from same calculation layer (2.4) |
| QAR-04 Extensibility | Observer/Signal-Slot (2.2), Layered architecture (2.4) |
| QAR-05 Consistency | Shared averaging state in Measurement Calculation layer (2.4) |

---

## 4. Candidate Design Decisions (Open for Experiment Validation)

| Decision | Options | Linked Experiment | Status |
|----------|---------|-------------------|--------|
| T1 detection reference point | Onset vs Peak | EX-02 | Open |
| Threading model | Single thread vs separate audio/UI threads | EX-01, EX-05 | Open |
| Cross-compile vs native build | macOS cross-compile vs RPi native | EX-04 | Open |
| Filter cutoff defaults | LP=8000Hz HP=200Hz (tentative) | EX-03 | Open |
| Sample rate target | 96k sps objective / 48k fallback | EX-01 | Open |

---

## 5. What the Architecture Does NOT Include (Scope Boundaries)

- No cloud connectivity or remote data logging (on-device only)
- No custom DSP hardware (uses USB audio input as-is)
- No exact copy of Witschi Chronoscope UI (functional inspiration only)
- AI feature (EX-19) is optional; architecture must not depend on it

---

## 6. Review Checklist

- [ ] Architecture overview-level description provided
- [ ] Key architectural approaches (tactics, patterns, design strategies) defined
- [ ] Each approach linked to specific architectural drivers
- [ ] Open design decisions listed with linked experiments
- [ ] Design is sufficient to guide construction at M2
