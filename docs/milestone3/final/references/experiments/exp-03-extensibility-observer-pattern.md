# EXP-03: Observer Pattern Compliance — Tab Extension Cost Measurement

**QA**: QAS-3 | **Status**: ✅ Done (2026-06-21)

---

## Objective

Verify that the BaseGraphTab observer pattern allows adding a new graph tab with **≤ 3 file changes** and **zero references** from Presentation to Signal Processing or Acquisition layers.

## Result

All 14 tabs implemented under the ≤ 3-file constraint. Zero layer violations. QAS-3 Pass.

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

**IAudioSource dependency inversion** (ADR-005): `AudioWorker` / `PlaybackWorker` / `SimWorker` unified under single interface. `SessionController` holds 1 `connect()` site, ensuring all three input modes traverse an identical DSP wiring path.

## Links

- Full run history: [experiment-results.md](experiment-results.md)
- Analysis tool: `src/tools/analyze_log.py`
