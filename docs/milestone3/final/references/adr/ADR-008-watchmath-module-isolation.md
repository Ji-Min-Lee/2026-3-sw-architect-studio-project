# ADR-008: WatchMath Pure Calculation Module Isolation

`MeasurementEngine` is the runtime subject in the Observer pattern: it owns the DSP
pipeline, audio state, and rolling accumulators. Before this change, the watch-math
formulas (beat error, amplitude, rate) were private methods inside `MeasurementEngine`,
entangled with Qt types, DSP state, and the rolling-window accumulators.

This made formula verification impossible without instantiating the full engine — which
requires an audio context, a Qt event loop, and live DSP state. A wrong formula silently
produced incorrect Rate, Amplitude, or Beat Error values across all 11 display tabs
with no automated check to catch it.

## Decision

We extract all watch-math formulas into a separate `namespace WatchMath` in
`src/engine/WatchMath.h` / `WatchMath.cpp` with the following constraints:

- **No Qt dependencies** — no `QObject`, no `QVector`, no signals
- **No DSP dependencies** — no `tg_context_t`, no ring buffer, no audio types
- **Pure functions only** — no mutable state; all inputs via parameters, output via return value
- **Directly linkable into a test binary** — `test_watch_math.cpp` links `WatchMath.cpp` alone

The extracted functions are:

| Function | Formula source |
|---|---|
| `beatErrorMs(t0, t1, t2, fs)` | TimeGrapher Equations §Beat Error |
| `amplitudeDeg(aPos, cPos, fs, liftAngle, bph)` | TimeGrapher Equations §Amplitude |
| `escapementMs(aPos, cPos, fs)` | TimeGrapher Equations §Escapement |
| `instErrorSec(timeMeasured, tStart, beatIndex, bph)` | TimeGrapher Equations §Rate Part I |
| `rateSpdFromPhase(tTicSec, tTacSec, bph)` | TimeGrapher Equations §Rate Part II |
| `halfBeatInterval(bph)` | Derived: 3600 / BPH |
| `applyZeroOffset(firstError, currentError)` | Zero-anchoring: beat 0 plots at 0 |
| `wrapInRange(value, lo, hi)` | Modulo arithmetic helper |

`MeasurementEngine` calls these functions instead of inlining the formulas.

## Rationale

**Why isolation is necessary for accuracy:**

The project's governing goal is measurement accuracy. A single incorrect formula
propagates silently to every display tab because all 11 tabs subscribe to
`MeasurementEngine::measurementReady()`. There is no runtime indicator that a computed
value is wrong — the display simply shows the incorrect number.

Isolating the formulas enables a pre-commit unit test gate (`TestWatchMath`) that
catches formula regressions before they reach the binary. Each test verifies a
specific worked example from the TimeGrapher Equations document with a strict tolerance
(< 1e-3 ms / < 0.01°), so any deviation from the documented formula fails the build.
This is a direct application of the testability tactics for limiting structural
complexity and nondeterminism [Bass21].

**Why a namespace, not a class:**

The functions have no shared state. A `namespace` with free functions expresses this
correctly and avoids the overhead of constructing an instance in test code.

**Why not keep formulas in MeasurementEngine as private methods:**

Private methods in a `QObject` subclass cannot be called without instantiating the class.
`MeasurementEngine` requires a Qt event loop, a `tg_context_t` DSP context, and audio
configuration to construct. This makes unit-testing the formulas in isolation impractical.
Extracting them removes the transitive dependency entirely.

**Modifiability vs. Accuracy trade-off:**

Extracting `WatchMath` as a separate namespace increases structural complexity: the
production binary now has an additional compilation unit, the test binary links
`WatchMath.cpp` separately, and `MeasurementEngine` delegates formula calls rather than
owning them inline. This is a real Modifiability cost — more moving parts. We accepted
it because keeping formulas inside `MeasurementEngine` means a formula bug propagates
silently to all 11 display tabs with no automated check to catch it before the binary
ships. The cost of a wrong formula (corrupt Rate, Amplitude, and Beat Error across every
view) outweighs the cost of maintaining a separate module and test binary.

**Rejected alternative — test via `MeasurementEngine` integration test:**

Integration tests feed synthetic audio through the full pipeline. This verifies
end-to-end behavior but is slow, non-deterministic (depends on DSP timing), and does not
pinpoint which formula is wrong when a test fails. Pure unit tests on `WatchMath`
are an order of magnitude faster and produce precise failure messages.

## Status

Accepted

Implemented in `src/engine/WatchMath.h` / `WatchMath.cpp`.
Verified by `src/tests/test_watch_math.cpp` — 44 test cases, all pass (see unit-test-results.md).

## Consequences

**Positive**:
- Formula regressions caught at pre-commit time; incorrect values cannot reach the binary
- `TestWatchMath` runs in < 10 ms with no audio hardware required
- `MeasurementEngine` delegates formula computation rather than owning it; the class
  responsibility narrows to state management and DSP pipeline control
- New formulas (e.g., DiffTicTac, DiffPeriod) are added in one place and immediately
  covered by the existing test harness

**Negative**:
- `MeasurementEngine` retains a `wrapInRange` private method for internal use —
  a duplication of `WatchMath::wrapInRange`. This is intentional (MeasurementEngine
  uses a slightly different call pattern for legacy rolling-state code) but should be
  consolidated in a future cleanup.

## Related

- [QAS-4: Correctness — Sub-1 Calculation Accuracy](../qa/qas-4-correctness.md)
- [ADR-006: BaseGraphTab Observer Pattern](ADR-006-basegraphtab-observer-pattern.md) —
  `MeasurementEngine` is the observable subject; `WatchMath` computes the values
  it publishes via `measurementReady()`

## References

- [Bass21] L. Bass, P. Clements, R. Kazman. *Software Architecture in Practice*, Fourth Edition. Addison-Wesley, 2021.
