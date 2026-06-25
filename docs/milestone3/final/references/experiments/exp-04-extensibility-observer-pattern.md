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

## Tab Addition History

Observed file-change counts across three sprint rounds (excludes build system and test files):

| Batch | Tabs added | Trigger | Files changed |
|-------|-----------|---------|:-------------:|
| W2 S1 | 11 (baseline) | Core requirements | **2** (NewTab + MainWindow) |
| W2 S2 | +2 → 13 (FilterScopeTab, SweepScopeTab) | Project-plan screen requirements (Fig 7-19) | **2 each** |
| W3 S1 | +1 → **14** (RadarChartTab, bonus) | Radar/Polar chart (bonus) | **3** ¹ |

¹ RadarChartTab reads per-position data from SequenceTab directly (not via `measurementReady`) → SequenceTab modified to expose `capturedReadings()` + `sequenceUpdated()`.

✅ All 14 tabs added within ≤ 3-file rule — Domain layer untouched each time.

## Architecture Decisions

**Observer pattern (BaseGraphTab + Qt Signal-Slot)**: `MeasurementEngine` publishes `Measurement` via Qt signal; all 14 tabs subscribe via `QueuedConnection`. `MeasurementEngine` has zero knowledge of tabs.

**IAudioSource dependency inversion**: `AudioWorker` / `PlaybackWorker` / `SimWorker` unified under single interface. `SessionController` holds 1 `connect()` site.

## Links

- Full experiment tracker: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-04-observer-pattern-compliance----tab-extension-cost-measurement)
- Test evidence: [Allocation View: Implementation Style](../views/view-allocation-implementation.md)
