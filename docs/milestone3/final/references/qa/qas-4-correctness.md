# QAS-4: Correctness — Priority 4

> M1 name: "Correctness (QA-C2)". M2 refactor: split into signal quality (this) and accuracy goal (QAS-1).
> M1 status: Provisional (pending EXP-04). M2 status: ✅ Verified 06/17 — onset=0.08, min_peak=0.10 confirmed.

Correctness is not a standard Bass/CMK Quality Attribute. Its three sub-requirements each map to a different QA lens:

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

**Tactics applied (Bass/CMK Ch.12 §12.2):**

| Tactic | How it appears in this system |
|--------|-------------------------------|
| **Limit Structural Complexity** | ADR-008 isolates `WatchMath` as a no-dependency pure-math module — no Qt, no ALSA, no hardware coupling. Low coupling → small state space → faults surface immediately during test. |
| **Abstract Data Sources** | `WatchMath` functions accept numeric primitives (`double`, `int`) directly; tests inject any value without mocking hardware or the audio pipeline. |
| **Specialized Interfaces** | Each formula (`beatErrorMs`, `amplitudeDeg`, `rateSpdFromPhase`, `instErrorSec`) is exposed as an individual public method, allowing per-formula test cases rather than end-to-end integration tests. |
| **Limit Nondeterminism** | Pure math functions — no threads, no timers, no I/O — guarantee deterministic output for any given input, making fault reproduction exact. |

## Related

[QA Priority Summary](README.md)

| Architecture | Rationale | Experiment | View |
|---|---|---|---|
| `WatchMath` Pure Calculation Module (Limit Structural Complexity + Abstract Data Sources) | [ADR-008: WatchMath Module Isolation](../adr/ADR-008-watchmath-module-isolation.md) | [EXP-02: Calculation Accuracy Unit Tests](../experiments/exp-03-calculation-accuracy.md) | [Allocation View: Implementation Style](../views/view-allocation-implementation.md) |

---

## Sub-Requirement 2: Internal Consistency — Reliability

Displayed values and graphs shall remain consistent across all GUI tabs — rate, amplitude, and beat error shown in the summary bar, Rate/Scope tab, Trace tab, Vario tab, and Sequence tab must all derive from the same underlying data.

| Field | Detail |
|-------|--------|
| **Source** | All graph tabs (11 tabs) reading measurement data |
| **Stimulus** | MeasurementEngine produces a new `Measurement` struct per beat |
| **Artifact** | `AudioRingBuffer` (single source) → `MeasurementEngine` → all tabs |
| **Environment** | Live mode; multiple tabs open simultaneously |
| **Response** | Every tab receives the identical `Measurement` object via Observer signal; no tab queries a separate data source |
| **Measure** | Deviation between the same metric shown in any two tabs = 0 at all times |

## Related

[QA Priority Summary](README.md)

| Architecture | Rationale | Experiment | View |
|---|---|---|---|
| Observer Pattern (BaseGraphTab) | [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) | [EXP-03: Observer Pattern Compliance](../experiments/exp-03-extensibility-observer-pattern.md) | [Decomposition View: Graph Tab](../views/view-decomposition-graph-tab.md) |
| 4-Layer Allowed-to-Use (single ownership of measurement state) | [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) | [EXP-03: Observer Pattern Compliance](../experiments/exp-03-extensibility-observer-pattern.md) | [Layered View: 4-Layer Allowed-to-Use](../views/view-layered-4layer.md) |

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

## Related

[QA Priority Summary](README.md)

| Architecture | Rationale | Experiment | View |
|---|---|---|---|
| 96kHz Sample Rate | [ADR-003: Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md) | [EXP-04: Detector Parameter Optimization](../experiments/exp-04-correctness-detector-optimization.md) | [Allocation View](../views/view-allocation.md) |
| LP/HP FilterChain | [ADR-009: FilterChain Design](../adr/ADR-009-filterchain-design.md) | [EXP-04: Detector Parameter Optimization](../experiments/exp-04-correctness-detector-optimization.md) | [C&C View: Signal Processing Pipeline](../views/view-cnc-signal-pipeline.md) |

---

## Warning Conditions — Usability

| Condition | Detection Method | GUI Message |
|-----------|-----------------|-------------|
| No signal | No beat event for N seconds | `⚠ No signal` — auto-cleared on recovery |
| Noisy signal | Beat event inter-arrival variance exceeds threshold | `⚠ Noisy signal` — auto-cleared on stabilization |
