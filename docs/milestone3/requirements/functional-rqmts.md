# Functional Requirements — TimeGrapher System

Requirements are derived from the project specification (`TimeGrapher_v10.5` baseline) and refined through M1/M2 experimentation.

---

## Actors

- **User** — operates the GUI to measure and diagnose a mechanical watch
- **Developer** — builds, deploys, and extends the application on Raspberry Pi 5
- **Mechanical Watch** — external signal source (acoustic beat noise)
- **WeiShi No.1000** — external reference device used for accuracy validation

---

## Signal Capture & Processing

| ID | Requirement | Priority | Status |
|----|-------------|:--------:|:------:|
| FR-01 | Detect T1(A) and T3(C) acoustic events from the watch escapement | HIGH | ✅ Implemented (`Detector.cpp`) |
| FR-02 | Compute Rate (s/d), Amplitude (°), Beat Error (ms), and BPH from detected events | HIGH | ✅ Implemented |
| FR-03 | Support three operating modes: Live (USB mic), Playback (PCM file), Simulation | HIGH | ✅ Implemented |
| FR-04 | Apply configurable Low-pass and High-pass filter chain to the audio signal | HIGH | ⚠️ Partial (HPF only) |

## GUI & Visualization

| ID | Requirement | Priority | Status |
|----|-------------|:--------:|:------:|
| FR-05 | Trace Display: real-time scrolling chart of Rate and Amplitude over time | MEDIUM | ❌ Not implemented |
| FR-06 | Vario Display: running Min / Max / Avg / σ for Rate and Amplitude | MEDIUM | ❌ Not implemented |
| FR-07 | Beat Error Display: numerical value + diagnostic trace line | MEDIUM | ❌ Not implemented |
| FR-08 | Multi-Position Sequence Display: record measurements per watch position (CH, 9H, 6H, etc.) | MEDIUM | ❌ Not implemented |
| FR-09 | Beat-Noise Scope 1: beat strip view with selectable time ranges (20 / 200 / 400 ms) | MEDIUM | ❌ Not implemented |
| FR-10 | Beat-Noise Scope 2: tic/tac dual-axis view with optional averaging (Σ) | MEDIUM | ❌ Not implemented |
| FR-11 | Long-Term Performance Graph: extended time-series of rate, amplitude, and beat error | MEDIUM | ❌ Not implemented |
| FR-12 | Escapement Analyzer: waveform display with A/C marker lines and ms labels | MEDIUM | ❌ Not implemented |
| FR-13 | Scope Mode: oscilloscope-style sweep with configurable sweep time | LOW | ❌ Not implemented |
| FR-14 | Scope Function: F0 / F1 / F2 / F3 filter-view display simultaneously | LOW | ❌ Not implemented |

## Controls & Navigation

| ID | Requirement | Priority | Status |
|----|-------------|:--------:|:------:|
| FR-15 | Pause and resume measurement; navigate the time axis in paused state | LOW | ❌ Not implemented |
| FR-16 | Configure sample rate, filter cutoffs, BPH setting, and lift angle from the control panel | HIGH | ✅ Implemented |

## Extensibility Contract

Adding a new graph tab must require changes **only** in the Presentation layer.  
No modifications to the Domain, Signal Processing, or Acquisition layers are permitted.  
This is enforced by the [Four-Layer Architecture](../ADRs/ADR003-layered-architecture.md).
