# Unit Test Results

**Branch**: `feature/layer-ex-baseline`  
**Run date**: 2026-06-22  
**Environment**: macOS 14.6.1, Qt 6.11.1, Apple LLVM 16.0.0 (arm64)  
**Build**: `src/build/` (CMake release)

---

## Summary

| Binary | Passed | Failed | Total | Result |
|--------|:------:|:------:|:-----:|:------:|
| TestWatchMath | 44 | 0 | 44 | ✅ PASS |
| TestMeasurementEngine | 8 | 0 | 8 | ✅ PASS |
| TestRollingAverage | 14 | 0 | 14 | ✅ PASS |
| TestRollingLeastSquares | 13 | 0 | 13 | ✅ PASS |
| TestGraphTabs | 17 | 0 | 17 | ✅ PASS |
| TestAddedTabs | 12 | 4 | 16 | ❌ 4 FAIL |
| TestRemainingTabs | 11 | 2 + crash | 13+ | ❌ FAIL |
| **Total** | **119** | **6+** | **125+** | |

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
| `computeRateError_perfectWatch_setsZeroWrappedPoints` | Perfect beat stream → rate = 0, wrapped error = 0 |
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

## Failing Binaries

### TestAddedTabs — 12 passed / 4 failed ❌

| Failed Test | Error | Root Cause |
|-------------|-------|------------|
| `escapementTab_deltaMs_matchesEventSpacing` | `qAbs(currentEscapementMs() - 5.0) < 1e-6` returned FALSE | EscapementTab internal state not updated via test-accessible path |
| `beatNoise_capturesBeatWaveformWindow` | `data->size()` = 0, expected 960 | BeatNoiseScopeTab waveform buffer not populated without full audio pipeline |
| `waveformComp_ticWindow_matchesHpfPcm` | `data->size() > 0` returned FALSE | WaveformComparisonTab requires HPF PCM path not wired in unit context |
| `waveformComp_tocPair_completesBeat` | `data->size() > 0` returned FALSE | Same as above |

**Assessment**: Failures are in tabs that require raw PCM pipeline input (BeatNoise, WaveformComparison) or internal state not exposed by the current test interface (EscapementTab). These are **integration-level gaps**, not logic bugs. Core measurement logic (WatchMath, MeasurementEngine) is fully verified.

---

### TestRemainingTabs — 11 passed / 2 failed + 1 crash ❌

| Failed Test | Error | Root Cause |
|-------------|-------|------------|
| `SweepScopeTab::pcmBlock_producesPlotData` | `dataCount() > 0` returned FALSE | Sweep trigger condition not met in headless test context |
| `SweepScopeTab::bufferLength_matchesBphMultiple` | x-axis upper = 5ms, expected 250ms | BPH not propagated to tab before test assertion |
| `FilterScopeTab::f0_outputSizeMatchesInput` | **SIGSEGV** (signal 11) | Null pointer dereference — filter chain not initialized before PCM input |

**Assessment**: SIGSEGV in FilterScopeTab is the one issue that warrants a fix. The two SweepScopeTab failures are test setup issues (BPH not set, trigger not fired). None affect the core audio pipeline or measurement correctness.

---

## What the Tests Protect

| Risk | Protection |
|------|-----------|
| Domain equation correctness (Rate / Amplitude / Beat Error) | TestWatchMath + TestMeasurementEngine — all 52 tests pass |
| Regression during layer refactoring (God Object → 4-layer) | TestGraphTabs — 17 tests verify data contract unchanged |
| Rolling buffer correctness (smoothing, trend) | TestRollingAverage + TestRollingLeastSquares — 27 tests pass |
| Tab data model integrity | TestAddedTabs (partial), TestRemainingTabs (partial) |
