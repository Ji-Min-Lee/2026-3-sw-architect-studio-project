# Architectural Drivers

**Milestone**: M1 | **Due**: 2026-06-09 | **Status**: [ ] Draft / [ ] Final

---

## 1. Business Goals & Project Context

| Goal | Description |
|------|-------------|
| Real-time watch diagnostics | GUI must process and display watch acoustic signals in real time on Raspberry Pi 5 |
| Modular extensibility | New graphs/analyses must be addable without major changes to existing code |
| Accuracy | Measurements must match WeiShi No.1000 reference device |
| Educational outcome | Architecture must demonstrate software architecture design principles (CMU MSE) |

---

## 2. Quality Attribute Requirements (QAR)

> Each QAR must be **actionable** — measurable and verifiable by design.

### QAR-01: Real-Time Performance

| Attribute | Real-Time Performance |
|-----------|----------------------|
| **Stimulus** | Continuous acoustic signal from mechanical watch microphone |
| **Source** | USB audio capture at selected sample rate |
| **Environment** | Raspberry Pi 5, 8GB RAM, running Qt GUI |
| **Response** | System acquires, processes, and displays data without frame drops |
| **Response Measure** | Sustained processing at **96,000 sps** (objective); min 48,000 sps; stretch 192,000 sps. GUI FPS ≥ target refresh rate |
| **Priority** | HIGH |

### QAR-02: Low Latency

| Attribute | Low Latency |
|-----------|-------------|
| **Stimulus** | Audio sample block captured from microphone |
| **Source** | Live mode signal acquisition |
| **Environment** | Running on Raspberry Pi 5 with Qt GUI |
| **Response** | Waveform, markers, and computed values appear in GUI |
| **Response Measure** | Measure and report: (1) capture→process latency, (2) process→display latency, (3) end-to-end latency (ms). Report avg and worst-case |
| **Priority** | HIGH |

### QAR-03: Measurement Accuracy (Correctness)

| Attribute | Correctness |
|-----------|-------------|
| **Stimulus** | Same mechanical watch measured simultaneously |
| **Source** | Live microphone input |
| **Environment** | Normal operating conditions (quiet environment) |
| **Response** | Rate, Amplitude, Beat Error values displayed |
| **Response Measure** | Displayed values match WeiShi No.1000 reference within ±2 s/d (Rate), ±5° (Amplitude), ±0.1 ms (Beat Error) |
| **Priority** | HIGH |

### QAR-04: Extensibility

| Attribute | Extensibility / Modifiability |
|-----------|-------------------------------|
| **Stimulus** | Request to add a new graph tab or analysis mode |
| **Source** | Developer (team member) |
| **Environment** | Existing codebase |
| **Response** | New graph implemented and integrated |
| **Response Measure** | Adding a new graph tab requires changes to ≤ N files (define N); zero changes to signal acquisition/processing pipeline |
| **Priority** | HIGH |

### QAR-05: Measurement Stability (Consistency)

| Attribute | Consistency |
|-----------|-------------|
| **Stimulus** | Same watch, same position, measured repeatedly |
| **Source** | Live mode |
| **Environment** | Stable environment, same configuration |
| **Response** | Consistent measurement values across runs |
| **Response Measure** | Rate std deviation ≤ 1.0 s/d, Beat Error std deviation ≤ 0.05 ms over 30s window |
| **Priority** | MEDIUM |

### QAR-06: Noise Robustness

| Attribute | Correctness under noise |
|-----------|------------------------|
| **Stimulus** | Ambient noise present during measurement (speech, handling) |
| **Source** | Environment |
| **Environment** | Typical office/workshop conditions |
| **Response** | System filters noise; displayed values remain within acceptable accuracy |
| **Response Measure** | Measurement values remain within QAR-03 thresholds with SNR ≥ X dB ambient noise |
| **Priority** | MEDIUM |

---

## 3. Functional Requirements Summary

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-01 | Detect T1 (A) and T3 (C) acoustic events per beat | HIGH |
| FR-02 | Calculate Rate (s/d), Amplitude (°), Beat Error (ms), BPH | HIGH |
| FR-03 | Support Live / Playback / Sim operating modes | HIGH |
| FR-04 | Display real-time Trace Display (rate + amplitude over time) | HIGH |
| FR-05 | Display Vario (rate & amplitude stability stats) | HIGH |
| FR-06 | Display Multi-Position Sequence Display (up to 10 positions) | HIGH |
| FR-07 | Display Beat-Noise Scope (Scope 1 & Scope 2) | MEDIUM |
| FR-08 | Display Beat Error Display & Diagnostic Trace | HIGH |
| FR-09 | Display Long-Term Performance Graph | MEDIUM |
| FR-10 | Display Escapement Analyzer & Marker-Line Display | MEDIUM |
| FR-11 | Display Time-Frequency Spectrogram | MEDIUM |
| FR-12 | Display Waveform Comparison Display with Timing Markers | MEDIUM |
| FR-13 | Display Scope Mode with Synchronized Sweep | MEDIUM |
| FR-14 | Display Scope Function with F0/F1/F2/F3 Filter Views | MEDIUM |
| FR-15 | Support Watch-Position testing (CH, CB, 9H, 6H, 3H, 12H, DU, DD) | MEDIUM |
| FR-16 | Pause/inspect prior data without losing live recording | HIGH |
| FR-17 | Apply low-pass / high-pass filtering | HIGH |
| FR-18 | Report latency metrics (capture→process→display) | HIGH |
| FR-19 | AI-based signal quality classification (optional) | LOW |

---

## 4. Requirements Priority Matrix

| Priority | QARs | Functional Requirements |
|----------|------|------------------------|
| HIGH | QAR-01, QAR-02, QAR-03, QAR-04 | FR-01~04, FR-08, FR-16~18 |
| MEDIUM | QAR-05, QAR-06 | FR-05~07, FR-09~15 |
| LOW | — | FR-19 |

---

## 5. Constraints

| Constraint | Description |
|------------|-------------|
| Platform | Must run on Raspberry Pi 5 (ARM64, 8GB RAM) |
| Language | C++ / Qt framework |
| Baseline code | Must extend TimeGrapher_v10.5, not replace |
| AGC | Must be disabled in AlsaMixer |
| Timeline | ~5 weeks total |

---

## 6. Review Checklist

- [ ] QA requirements expressed in measurable/verifiable form (stimulus-response)
- [ ] Drivers linked to overall project goals
- [ ] Functional requirements sufficiently understood
- [ ] Requirements prioritized (H/M/L)
