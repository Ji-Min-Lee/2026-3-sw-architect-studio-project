# QAS-4: Correctness — Priority 4

> M1 name: "Correctness (QA-C2)". M2 refactor: split into signal quality (this) and accuracy goal (QAS-1).
> M1 status: Provisional (pending EXP-04). M2 status: ✅ Verified 06/17 — onset=0.08, min_peak=0.10 confirmed.

Correctness is not a standard Bass/CMK Quality Attribute. Its three sub-requirements
each map to a different QA lens [Bass21]:

- **Sub-1** — **Testability** (Bass/CMK Ch.12): formula correctness is enforced through architectural isolation that makes the calculation unit independently and deterministically testable at every commit
- **Sub-2** — **Reliability**: single-source data flow ensures all tabs display consistent values at runtime
- **Sub-3** — **Usability**: the system remains usable (signal quality feedback, noise warnings) when the acoustic environment degrades

---

## Sub-Requirement 1: Calculation Accuracy — Testability

The architecture shall make formula correctness continuously verifiable: the `WatchMath` module must be structured so that any formula deviation from the TimeGrapher Equations document is revealed by a unit test before the commit is accepted.

| Field | Detail |
|-------|--------|
| **Source** | Automated unit tester (pre-commit CI gate) |
| **Stimulus** | A coding increment is completed — any change to `WatchMath.h/.cpp` triggers the test suite |
| **Artifact** | `WatchMath` module (`src/engine/WatchMath.h/.cpp`) + `TestWatchMath` test binary |
| **Environment** | Completion of a coding increment; development environment before commit is accepted |
| **Response** | Test suite executes, captures results, and blocks the commit if any case fails |
| **Measure** | 44 test cases covering `beatErrorMs`, `amplitudeDeg`, `rateSpdFromPhase`, `instErrorSec` all pass within 30 s; zero formula deviation from Equations doc worked examples tolerated |

**Tactics applied (Bass/CMK Ch.12 §12.2 [Bass21]):**

| Tactic | How it appears in this system |
|--------|-------------------------------|
| **Limit Structural Complexity** | ADR-008 isolates `WatchMath` as a no-dependency pure-math module — no Qt, no ALSA, no hardware coupling. Low coupling → small state space → faults surface immediately during test. |
| **Abstract Data Sources** | `WatchMath` functions accept numeric primitives (`double`, `int`) directly; tests inject any value without mocking hardware or the audio pipeline. |
| **Specialized Interfaces** | Each formula (`beatErrorMs`, `amplitudeDeg`, `rateSpdFromPhase`, `instErrorSec`) is exposed as an individual public method, allowing per-formula test cases rather than end-to-end integration tests. |
| **Limit Nondeterminism** | Pure math functions — no threads, no timers, no I/O — guarantee deterministic output for any given input, making fault reproduction exact. |

---

## Sub-Requirement 2: Internal Consistency — Reliability

Displayed values and graphs shall remain consistent across all GUI tabs — rate, amplitude, and beat error shown in the summary bar, Rate/Scope tab, Trace tab, Vario tab, and Sequence tab must all derive from the same underlying data. Furthermore, this data must be produced by the same DSP computation regardless of which input mode (live mic, WAV playback, or synthetic signal) is active.

| Field | Detail |
|-------|--------|
| **Source** | All graph tabs (14 tabs) reading measurement data; all three input modes (live mic, WAV playback, SimWorker) |
| **Stimulus** | MeasurementEngine produces a new `Measurement` struct per beat |
| **Artifact** | `IAudioSource` (3 input modes → single `connect()` site) → `AudioRingBuffer` → `MeasurementEngine` → all 14 tabs |
| **Environment** | Any input mode; multiple tabs open simultaneously |
| **Response** | Every tab receives the identical `Measurement` object via Observer signal; all input modes enter the DSP chain through the same `SessionController::startSourceThread()` wiring |
| **Measure** | Deviation between the same metric shown in any two tabs = 0 at all times; DSP wiring path is identical across all three input modes |

**Tactics applied:**

| Tactic | How it appears in this system |
|--------|-------------------------------|
| **Single data source (horizontal)** | `MeasurementEngine` broadcasts one `Measurement` VO via Qt signal; all 14 tabs subscribe to the same signal — no tab reads a private or duplicate data source (ADR-006) |
| **Single wiring path (vertical)** | `IAudioSource` (ADR-005) ensures all three input modes share the identical `connect()` block in `SessionController`; the compiler prevents mode-specific DSP entry point divergence |

---

## Sub-Requirement 3: Noise Resilience — Usability

The system shall remain usable and produce reliable measurements in the presence of ambient acoustic noise (speech, handling vibration, environmental hum), while preserving the T1 (A) and T3 (C) events needed for correct beat detection.

| Field | Detail |
|-------|--------|
| **Source** | External environment (ambient office/workshop noise) |
| **Stimulus** | Non-beat acoustic signal enters the microphone alongside watch beats |
| **Artifact** | Signal Processing layer — LP/HP `FilterChain` → `BeatDetector` |
| **Environment** | Live mode; ambient noise present |
| **Response** | LP/HP filter rejects the non-beat signal; `BeatDetector` fires only on genuine T1/T3 events |
| **Measure** | False trigger rate < 1% under standard ambient conditions; true T1 detection rate > 99% (verified in EXP-04) |

---

## Warning Conditions — Usability

| Condition | Detection Method | GUI Message |
|-----------|-----------------|-------------|
| No signal | No beat event for N seconds | `⚠ No signal` — auto-cleared on recovery |
| Noisy signal | Beat event inter-arrival variance exceeds threshold | `⚠ Noisy signal` — auto-cleared on stabilization |

## References

- [Bass21] L. Bass, P. Clements, R. Kazman. *Software Architecture in Practice*, Fourth Edition. Addison-Wesley, 2021.
