# QAS-4: Correctness — Priority 4

> M1 name: "Correctness (QA-C2)". M2 refactor: split into signal quality (this) and accuracy goal (QAS-1).
> M1 status: Provisional (pending EXP-05). M2 status: ✅ Verified 06/17 — onset=0.08, min_peak=0.10 confirmed.

## Classification

Correctness is not a standard Bass/CMK Quality Attribute. It is better understood as a **Functional Requirement + Reliability** property: the system must compute watch-performance measures correctly, display them consistently across all views, and remain reliable in the presence of acoustic noise.

Architecture does not "implement" correctness directly. Instead, correctness is **achieved** by applying tactics from Testability, Modifiability, and Reliability — each targeting one of the three sub-requirements below.

---

## Sub-Requirement 1: Calculation Accuracy

The system shall compute rate (s/d), amplitude (°), and beat error (ms) in exact accordance with the formulas in the TimeGrapher Equations document.

| Field | Detail |
|-------|--------|
| **Source** | Developer running pre-commit hook |
| **Stimulus** | Commit containing changes to measurement calculation code |
| **Artifact** | `WatchMath` module (`src/engine/WatchMath.h/.cpp`) |
| **Environment** | Development environment (pre-commit CI gate) |
| **Response** | All unit tests in `test_watch_math.cpp` pass before commit is accepted |
| **Measure** | 30+ test cases covering beatErrorMs, amplitudeDeg, rateSpdFromPhase, instErrorSec all pass; zero tolerance on formula deviation from Equations doc worked examples |

### Architecture: Increase Semantic Coherence (aka SRP) + Restrict Dependencies

Two Modifiability tactics from Merson Lecture 14 are applied together:

**Increase semantic coherence (aka SRP):** All calculation functions — `beatErrorMs`, `amplitudeDeg`, `rateSpdFromPhase`, `instErrorSec` — are co-located in the `WatchMath` module. One module, one responsibility: watch math. No other module duplicates these formulas.

**Restrict dependencies:** `WatchMath` is a pure C++ namespace with **no Qt, no DSP, no audio dependencies**. Coupling is cut to the minimum necessary — `MeasurementEngine` calls `WatchMath`; nothing calls back. This is the loose coupling the Restrict Dependencies tactic targets.

```
[WatchMath]          ← pure C++ namespace; no Qt, no DSP
     ↑  (uses)
[MeasurementEngine]  ← calls WatchMath functions; owns DSP pipeline
     ↑  (Observer signal)
[Graph Tabs / GUI]   ← receives Measurement; never touches WatchMath directly
```

The structural consequence is Testability: because dependencies are restricted, `test_watch_math.cpp` can exercise every formula in a standard C++ test runner with no Qt application and no audio hardware. A pre-commit hook enforces that all tests pass before a commit is accepted — correctness becomes a **structural gate**, not a manual check.

**Rationale:** Len Bass and Rick Kazman note that calculation accuracy tactics are "not architectural" in the AI/ML sense. For formula-based correctness (our case), the architectural answer is to make the math module independently testable — achieved here by increasing semantic coherence and restricting dependencies so the math can be verified in isolation against the Equations document worked examples.

| Architecture | Tactic (Merson L14) | View |
|---|---|---|
| `WatchMath` Pure Calculation Module | Increase Semantic Coherence (SRP) + Restrict Dependencies | Module view |

---

## Sub-Requirement 2: Internal Consistency

Displayed values and graphs shall remain consistent across all GUI tabs — rate, amplitude, and beat error shown in the summary bar, Rate/Scope tab, Trace tab, Vario tab, and Sequence tab must all derive from the same underlying data.

| Field | Detail |
|-------|--------|
| **Source** | All graph tabs (11 tabs) reading measurement data |
| **Stimulus** | MeasurementEngine produces a new `Measurement` struct per beat |
| **Artifact** | `AudioRingBuffer` (single source) → `MeasurementEngine` → all tabs |
| **Environment** | Live mode; multiple tabs open simultaneously |
| **Response** | Every tab receives the identical `Measurement` object via Observer signal; no tab queries a separate data source |
| **Measure** | Deviation between the same metric shown in any two tabs = 0 at all times |

### Architecture: Observer Pattern + Redistribute Responsibilities

The 4-Layer Allowed-to-Use architecture (QAS-3) already enforces that no graph tab has a direct dependency on `AudioRingBuffer` or `MeasurementEngine` internals — that is the Restrict Dependencies tactic applied at the system level. Internal consistency builds on top of that structural constraint with two additional decisions:

**Observer Pattern:** `MeasurementEngine` emits `measurementReady(const Measurement &)` once per beat. All 11 tabs subscribe to this single signal. The layer boundary (QAS-3) prevents bypass; the Observer pattern provides the single publication point within that boundary.

**Tactic: Redistribute Responsibilities (Modifiability)**
All measurement state — rate, amplitude, beat error — is owned exclusively by `MeasurementEngine`. No tab holds a private copy or runs its own calculation. If calculation logic were duplicated across tabs, a formula fix would require touching every tab. Redistributing all responsibility into `MeasurementEngine` means there is exactly one place to correct.

```
[AudioRingBuffer]
      ↓ (feeds)
[MeasurementEngine] ──── measurementReady(Measurement) ────→ [Tab 1]
       (owns all state)                                   ──→ [Tab 2..11]
                                                              (read-only consumers)
```

**Rationale:** Internal consistency is not a runtime invariant to maintain — it is a structural consequence of two decisions: the 4-Layer boundary (no tab reads audio directly) and single-ownership of measurement state (no tab computes independently). If either were violated, drift between views would be unavoidable.

| Architecture | Tactic (Merson L14) | View |
|---|---|---|
| Observer Pattern (on top of 4-Layer boundary from QAS-3) | Redistribute Responsibilities | Module view |

---

## Sub-Requirement 3: Noise Resilience

The system shall remain usable and produce reliable measurements in the presence of ambient acoustic noise (speech, handling vibration, environmental hum), while preserving the T1 (A) and T3 (C) events needed for correct beat detection.

| Field | Detail |
|-------|--------|
| **Source** | External environment (ambient office/workshop noise) |
| **Stimulus** | Non-beat acoustic signal enters the microphone alongside watch beats |
| **Artifact** | Signal Processing layer — LP/HP `FilterChain` → `BeatDetector` |
| **Environment** | Live mode; ambient noise present |
| **Response** | LP/HP filter rejects the non-beat signal; `BeatDetector` fires only on genuine T1/T3 events |
| **Measure** | False trigger rate < 1% under standard ambient conditions; true T1 detection rate > 99% (verified in EXP-05) |

### Architecture: 96kHz Sampling + LP/HP FilterChain (Restrict Dependencies in Signal Pipeline)

Two design decisions work together:

**Accuracy tactic — Increase sampling rate (Merson L14):** ADR-003 selects 96kHz over 48kHz. A higher sampling rate gives sub-sample beat timing resolution (~10µs at 96kHz vs ~21µs at 48kHz), directly reducing quantization error in T1/T3 event timestamps. This is the lecture's "Increase sampling rate" accuracy tactic: read the sensor more frequently to improve fidelity.

**Modifiability tactic — Restrict dependencies (Merson L14):** A two-stage LP/HP FilterChain sits between raw audio capture and `BeatDetector`. The `BeatDetector` depends only on the filtered signal output — it has no dependency on raw PCM or noise characteristics. This restricts what the detector couples to: a clean, band-limited signal. The `FilterChain` module is separately configurable (onset=0.08, min_peak=0.10 tuned in EXP-05) without touching `BeatDetector` logic.

```
[AudioCapture] → [LP Filter] → [HP Filter] → [BeatDetector]
                  └─── FilterChain ────┘
                       (restricted interface: BeatDetector never sees raw PCM)
```

**Rationale:** Beat detection accuracy is directly determined by signal quality at the detector input. Increasing sampling rate reduces timing quantization error; restricting the detector's dependency to filtered-only input means noise conditions never propagate into the measurement logic. Both tactics address correctness from different angles: one improves raw data fidelity, the other prevents noise from corrupting downstream calculation inputs.

| Architecture | Tactic (Merson L14) | View |
|---|---|---|
| 96kHz Sample Rate (ADR-003) | Increase Sampling Rate (Accuracy) | Allocation view |
| LP/HP FilterChain | Restrict Dependencies (Modifiability) | C&C view |

---

## Warning Conditions (Usability Integration)

| Condition | Detection Method | GUI Message |
|-----------|-----------------|-------------|
| No signal | No beat event for N seconds | `⚠ No signal` — auto-cleared on recovery |
| Noisy signal | Beat event inter-arrival variance exceeds threshold | `⚠ Noisy signal` — auto-cleared on stabilization |

---

## Related

- [QA Priority Summary](README.md)
- [ADR-003: Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md)
- [EXP-05: Detector Parameter Optimization](../experiments/exp-05-correctness-detector-optimization.md)
- [Unit Test Results](../unit-test-results.md)
