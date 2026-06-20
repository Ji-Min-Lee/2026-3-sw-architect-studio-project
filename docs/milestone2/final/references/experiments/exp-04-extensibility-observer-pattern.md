# EXP-04: Observer Pattern Compliance — Tab Extension Cost Measurement

**QA**: QAS-4 | **Status**: ✅ Done (2026-06-21)

---

## Objective

Verify that the BaseGraphTab observer pattern allows adding a new graph tab with **≤ 3 file changes** and **zero references** from Presentation to Signal Processing or Acquisition layers.

## Result

All 14 tabs implemented under the ≤ 3-file constraint. Zero layer violations. QAS-4 Pass.

## Evidence

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 (header + source + registration) |
| Signal Processing / Acquisition refs from Presentation | 0 | ✅ 0 — DSM verified |
| Observer contract compliance (all 14 tabs) | 100% | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

## Architecture Decisions

**Observer pattern (BaseGraphTab + Qt Signal-Slot)**: `MeasurementEngine` publishes `Measurement` via Qt signal; all 14 tabs subscribe via `QueuedConnection`. `MeasurementEngine` has zero knowledge of tabs.

**IAudioSource dependency inversion**: `AudioWorker` / `PlaybackWorker` / `SimWorker` unified under single interface. `SessionController` holds 1 `connect()` site.

## Links

- Full experiment tracker: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-04-observer-pattern-compliance----tab-extension-cost-measurement)
- Test evidence: [unit-test-results.md](../unit-test-results.md)
