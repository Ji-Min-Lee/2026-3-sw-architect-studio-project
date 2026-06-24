# Unit Test Results

**Branch**: `feature/m2-presentaion`  
**Run date**: 2026-06-21 (re-run after fixes)  
**Environment**: macOS 14.6.1, Qt 6.11.1, Apple LLVM 16.0.0 (arm64)  
**Build**: `src/build-mac/` (CMake release)

---

## Summary

| Binary | Passed | Failed | Total | Result |
|--------|:------:|:------:|:-----:|:------:|
| TestWatchMath | 44 | 0 | 44 | ✅ PASS |
| TestMeasurementEngine | 8 | 0 | 8 | ✅ PASS |
| TestRollingAverage | 14 | 0 | 14 | ✅ PASS |
| TestRollingLeastSquares | 13 | 0 | 13 | ✅ PASS |
| TestGraphTabs | 17 | 0 | 17 | ✅ PASS |
| TestAddedTabs | 20 | 0 | 20 | ✅ PASS |
| TestRateScopeTab | 7 | 0 | 7 | ✅ PASS |
| TestSweepScopeTab | 6 | 0 | 6 | ✅ PASS |
| TestFilterScopeTab | 7 | 0 | 7 | ✅ PASS |
| TestSoundPrintTab | 6 | 0 | 6 | ✅ PASS |
| **Total** | **142** | **0** | **142** | ✅ ALL PASS |

---

## Passing Binaries

### TestWatchMath — 44 / 44 ✅

Core watch math pure functions. All equation-level tests against *TimeGrapher Equations* document worked examples.

| Group | Tests | Coverage |
|-------|:-----:|---------|
| Beat Error | 6 | Perfect symmetry, worked example (0.8ms), asymmetry cases, multi-fs |
| Amplitude | 5 | Worked example (230°), invalid tAC, over-360 guard, approx vs exact convergence, 21600 BPH |
| Escapement | 6 | 9ms exact, fs scaling, non-zero aPos, zero interval, reverse order |
| wrapInRange | 7 | In-range, boundary, above/below, large values |
| applyZeroOffset | 4 | First beat = 0, relative values, fast/slow watch, negative anchor |
| halfBeatInterval | 3 | 28800 / 21600 / 36000 BPH |
| instErrorSec | 5 | beat 0, perfect/fast/slow watch, worked example p.2 |
| rateSpdFromPhase | 4 | Worked example p.6 (+8.640 s/day), perfect watch, fast watch, tic-tac asymmetry |
| Amplitude approx | 2 | Worked example, inverse proportionality to t_AC |

---

### TestMeasurementEngine — 8 / 8 ✅

End-to-end MeasurementEngine integration: Beat Event stream → Rate / Amplitude / Beat Error output.

| Test | What it verifies |
|------|-----------------|
| `beatError_workedExample_matchesEquation` | BE rolling average = 0.8ms matches Equations p.8 |
| `computeAmplitude_ticAndTocProduceSplitAndAverage` | Tic/toc split computed, average stored in rolling buffer |
| `computeRateError_perfectWatch_setsZeroWrappedPointsAndZeroRate` | Perfect beat stream → rate = 0, wrapped error = 0 |
| `computeRateError_knownDeviation_rateSpd_8p64` | T_tic=249.980ms, T_tac=249.970ms → Rate = +8.640 s/day |
| `computeRateError_multibeat_rateConverges` | 20-beat averaging → tolerance tightens to ±0.01 s/day |
| `computeRateError_slowWatch_rateSpd_negative` | T=250.010ms → Rate ≈ −3.456 s/day |

---

### TestRollingAverage — 14 / 14 ✅

Sliding window average used for Rate, Amplitude, Beat Error smoothing.

Covers: single value, arithmetic mean, constant stream, sliding window, capacity cap, reset, resize (shrink/grow), empty window, negative values, beat error scenario.

---

### TestRollingLeastSquares — 13 / 13 ✅

Least-squares slope estimator used for long-term rate trend.

Covers: perfect 2-point / 5-point lines, negative slope, rolling window eviction, capacity cap, degenerate cases (1 point, 0 points, singular X), reset, rate-to-seconds-per-day scenario.

---

### TestGraphTabs — 17 / 17 ✅

UI-layer graph tab data contract tests (no rendering — data model only).

| Tab | Tests |
|-----|:-----:|
| TraceDisplay | 6 (rate value, negative rate, multi-point accumulation, invalid rate guard, reset, x-axis advance) |
| BeatErrorTrace | 4 (value, multi-value ordering, invalid guard, reset) |
| VarioDisplay | 5 (min/mean/max tracking, σ = sample std dev, invalid guard, elapsed time, reset) |

---

### TestRateScopeTab — 7 / 7 ✅

| Test | What it verifies |
|------|-----------------|
| `pcmBlock_appearsInScopePlot` | PCM block data appears in scope plot |
| `ticEvent_appendsToTicSeries` | Tic event appended to tic series |
| `tocEvent_appendsToTocSeries` | Toc event appended to toc series |
| `wrappedValue_isPreserved` | Wrapped rate value preserved across update |
| `reset_clearsSeries` | Reset clears all series data |

---

### TestSweepScopeTab — 6 / 6 ✅

| Test | What it verifies |
|------|-----------------|
| `pcmBlock_producesPlotData` | PCM block triggers sweep plot population |
| `bufferLength_matchesBphMultiple` | X-axis upper = 250ms at 28800 BPH |
| `reset_clearsSweepAndPlot` | Reset clears sweep buffer and plot |
| `absoluteValue_storedInSweep` | Absolute PCM values stored in sweep buffer |

---

### TestFilterScopeTab — 7 / 7 ✅

| Test | What it verifies |
|------|-----------------|
| `f0_outputSizeMatchesInput` | Filter output size matches PCM input size |
| `f0_mirroredGraph1HasData` | Mirror graph has data after f0 processing |
| `f1_allValuesNonNegative` | f1 (envelope) output is non-negative |
| `f1_graph1IsEmpty` | Graph 1 is empty in f1 mode |
| `reset_clearsBothGraphs` | Reset clears both graphs |

---

### TestSoundPrintTab — 6 / 6 ✅

Null-safety smoke tests for SoundPrintTab.

Covers: construction with null widget, empty PCM input, reset with null widget, setBph / setSampleRate no-crash guards.

---

## Fixes Applied (2026-06-21)

All 8 previously failing tests resolved. Root causes and fixes:

| Failed Test | Root Cause | Fix Type |
|-------------|-----------|----------|
| `escapementTab_deltaMs_matchesEventSpacing` | `mCurrentMs` updated only inside `redraw()`, which is skipped when `!isVisible()` | **Code fix** — compute `mCurrentMs` in `onMeasurement()` before visibility check; TC also needed `tab.show()` for the plot assertion |
| `longTermTab_invalidRate_notDrawn` | `rate.reset()` followed immediately by `= 99.0` re-set the optional to a valid value | TC fix — removed `m.metrics.rate = 99.0` line |
| `beatNoise_capturesBeatWaveformWindow` | `redrawScope1()` blocked by `!isVisible()` guard; tab not shown | TC fix — added `tab.show() + processEvents()` |
| `waveformComp_ticWindow_matchesHpfPcm` | `redrawPlots()` blocked by `!isVisible()` guard | TC fix — added `tab.show() + processEvents()` |
| `waveformComp_tocPair_completesBeat` | Same as above | TC fix — added `tab.show() + processEvents()` |
| `sequence_capturedReadings_reflectValues` | `m.metrics.amplitude` never set (was left as a comment) | TC fix — added `m.metrics.amplitude = 285.0` |
| `radar_plotsCapturedPositions_closedPolygon` | Amplitude not passed into Measurement in lambda; `rebuild()` needed `processEvents()` after `show()` | TC fix — added `m.metrics.amplitude = amp` + `processEvents()` |
| `radar_flagsOutOfTolerance_inVerdict` | Same as above | TC fix — same fix |

---

## What the Tests Protect

| Risk | Protection |
|------|-----------|
| Domain equation correctness (Rate / Amplitude / Beat Error) | TestWatchMath + TestMeasurementEngine — all 52 tests pass |
| Regression during layer refactoring (God Object → 4-layer) | TestGraphTabs — 17 tests verify data contract unchanged |
| Rolling buffer correctness (smoothing, trend) | TestRollingAverage + TestRollingLeastSquares — 27 tests pass |
| Scope tab rendering pipeline | TestRateScopeTab + TestSweepScopeTab + TestFilterScopeTab + TestSoundPrintTab — 26 tests pass |
| Tab data model integrity | TestAddedTabs — all 20 tests pass |
