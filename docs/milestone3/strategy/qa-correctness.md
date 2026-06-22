# QA Strategy: Correctness

## Requirement

The system shall compute watch-performance measures correctly and consistently from the
captured acoustic signal. This covers three distinct sub-requirements:

| Sub-requirement | Description |
|----------------|-------------|
| **Calculation correctness** | Rate, Amplitude, Beat Error computed correctly per formula |
| **Internal consistency** | GUI, derived measurements, and long-term summaries always reflect the same underlying data |
| **Noise resilience** | Beat detection remains correct in the presence of ambient acoustic noise |

**Reference**: [QAS-5: Correctness](../../milestone2/final/references/qa/qas-5-correctness.md)

---

## Sub-requirement 1: Calculation Correctness

### Architectural Decision: Isolate Calculation Logic into a Dedicated Module

#### Rationale

Rate, Amplitude, and Beat Error are computed from precise timestamp differences between
T1 and T3 events. The formulas are defined in the TimeGrapher Equations document and have
no dependency on GUI state, threading, or signal capture. If calculation logic is
embedded inside GUI widgets or processing threads, it becomes difficult to test in
isolation and easy to break during refactoring.

Isolating calculation into a dedicated module (`MeasurementCalculator` or equivalent)
with pure functions (timestamps in → measurement values out) makes the formula
implementations directly testable via unit tests with known inputs and expected outputs.

#### Trade-offs

| | Embedded in processing thread | Isolated calculation module (chosen) |
|--|------------------------------|--------------------------------------|
| Testability | Requires full pipeline to test a formula | Unit-testable with timestamp inputs alone |
| Regression safety | Formula bug discovered during demo | Caught by pre-commit unit test |
| Separation of concerns | Low | High — calculation has no side effects |
| Complexity | Simpler initially | Requires defining module boundary |

#### Alternatives Considered

**Test calculations via integration tests only**
- Run the full pipeline with a known signal and verify the output.
- Not applied because: integration tests are slow and harder to pinpoint the source of a
  calculation error. A unit test with explicit timestamp inputs directly verifies the
  formula, which is simpler and faster to run on every commit.

#### Views

| View | What it shows | Status |
|------|--------------|--------|
| [Module View: Domain Entity / Value Object](../../milestone2/final/references/views/view-domain-entity-vo.md) | `Measurement` aggregate and Value Objects (Rate, Amplitude, BeatError) as dependency-free domain types | ✅ Exists |
| [Layered View: 4-Layer](../../milestone2/final/references/views/view-layered-4layer.md) | Calculation module sits in Domain layer; no upward dependency to GUI or threads | ✅ Exists |

#### Unit Tests

| Test Binary | Coverage | Status |
|-------------|----------|--------|
| TestWatchMath (44 tests) | Rate, Amplitude, Beat Error formula implementations | ✅ Pass (2026-06-21) |
| TestMeasurementEngine (8 tests) | End-to-end measurement computation | ✅ Pass (2026-06-21) |

**Reference**: [Unit Test Results](../../milestone2/final/references/unit-test-results.md)

> ⚠️ **TODO**: Unit tests currently run manually. Pre-commit CI hook to auto-run
> TestWatchMath and TestMeasurementEngine is not yet configured.

---

## Sub-requirement 2: Internal Consistency

### Architectural Decision: Observer Pattern — Single Measurement Source Notifies All Views

#### Rationale

The system includes multiple simultaneous views of the same underlying data: the
Measurement Summary Bar, Rate/Scope Tab, Trace Display, Vario Display, Beat Error
Display, and others. If each view independently re-derives or caches its own copy of
measurement data, a timing difference between re-computations can cause different views
to display different values for the same beat — a violation of internal consistency.

Using the Observer pattern, a single `MeasurementModel` holds the authoritative computed
values and notifies all registered views when new measurements are available. No view
computes measurements independently; they only render what the model provides. This makes
inconsistency structurally impossible: all views update from the same event with the same
data.

#### Trade-offs

| | Independent per-view computation | Observer / single source (chosen) |
|--|----------------------------------|-----------------------------------|
| Internal consistency | Cannot be guaranteed | Guaranteed by structure |
| Coupling | Views are independent | Views depend on MeasurementModel interface |
| Extensibility | Adding a view is self-contained | New view registers as observer — no model change |
| Complexity | Simple locally, complex globally | Requires model + observer interface definition |

#### Alternatives Considered

**Shared global state (singleton measurement struct)**
- All views read from a global struct updated by the processing thread.
- Not applied because: without notification, views poll at different rates and may read
  partially updated state. Thread safety requires locking, which reintroduces the
  consistency problem if locks are not held across a full measurement update.

#### ADRs

| ADR | Decision | Status |
|-----|----------|--------|
| [ADR-006: BaseGraphTab Observer Pattern](../../milestone2/final/references/adr/ADR-006-basegraphtab-observer-pattern.md) | All graph tabs implement BaseGraphTab and receive measurement updates via a single notification path | ✅ Applied |

#### Views

| View | What it shows | Status |
|------|--------------|--------|
| [Module View: Domain Entity / Value Object](../../milestone2/final/references/views/view-domain-entity-vo.md) | Single `Measurement` instance propagated to all tabs — structural consistency guarantee | ✅ Exists |
| [Decomposition View: Graph Tab](../../milestone2/final/references/views/view-decomposition-graph-tab.md) | `BaseGraphTab` interface; all tabs as observers of the same data source | ✅ Exists |

#### Experiments

| ID | Description | Status |
|----|-------------|--------|
| [EXP-04](../../milestone2/final/references/experiments/exp-04-extensibility-observer-pattern.md) | Observer Pattern Compliance — Tab Extension Cost Measurement | ✅ Done (2026-06-21) |

---

## Sub-requirement 3: Noise Resilience

### Architectural Decision: Reuse IAudioSource Interface for Noise-Mixed Wav Injection in CI

#### Rationale

Noise resilience is achieved through filter parameter tuning (HPF/LPF cutoffs, thresholds).
The architectural role is to ensure that tuning results do not silently regress when other
changes are made.

The `IAudioSource` interface defined for Measurement Accuracy is reused here: a
noise-mixed wav (watch signal + ambient noise) is injected via `WavSource` and the
system's beat detection output is compared against expected values. This test runs in CI
alongside the golden dataset test, catching filter regressions before they reach the demo.

The filter parameter values themselves are determined by EXP-05 and are not an
architectural decision.

#### Trade-offs

| | Manual noise testing only | CI noise-mixed wav test (chosen) |
|--|--------------------------|----------------------------------|
| Regression safety | Discovered at demo | Caught before commit |
| Setup cost | None | Requires noise-mixed wav and expected values |
| Reuse | No reuse | Reuses IAudioSource already built for Accuracy |

#### Alternatives Considered

**Hardware noise injection during demo**
- Play noise through a speaker during the live demo to show resilience in real time.
- Not applied as the sole verification method because: it is not reproducible across
  sessions and cannot serve as a regression gate.

#### Views

| View | What it shows | Status |
|------|--------------|--------|
| [Deployment View: Build-Deploy Pipeline](../../milestone2/final/references/views/view-deployment-build-pipeline.md) | CI pipeline where noise-mixed wav test cases run | ✅ Exists |

> ⚠️ **TODO**: The current deployment view does not yet include the noise-mixed wav CI test
> step. Update after EXP-05 parameter values are confirmed and test cases are created.

#### Experiments

| ID | Description | Status |
|----|-------------|--------|
| [EXP-05](../../milestone2/final/references/experiments/exp-05-correctness-detector-optimization.md) | Detector Parameter Optimization Under Noise | ✅ Done (2026-06-16 ~ 2026-06-17) |

---

## Verification

| Sub-requirement | Method | Status |
|----------------|--------|--------|
| Calculation correctness | Unit tests (TestWatchMath, TestMeasurementEngine) | ✅ Tests pass; ⚠️ pre-commit hook not yet configured |
| Internal consistency | ADR-006 Observer structure + EXP-04 tab extension test | ✅ Done |
| Noise resilience | EXP-05 parameter optimization + CI noise-mixed wav test | ✅ Params done; ⚠️ CI test not yet created |
