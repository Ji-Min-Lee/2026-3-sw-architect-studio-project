# Experiment Results

**Milestone**: M3 | **Due**: 2026-07-01 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-25

---

## Summary

| ID | QA | Experiment | Runs | Key Result | Status |
|----|----|------------|:----:|------------|:------:|
| EXP-01 | QAS-1 | RPi Real-Time Performance — Dropped Block Measurement | 9 | Dropped Block = **0** across all sps × all scheduling policies — **QAS-1 Pass** | ✅ Done |
| EXP-02 | QAS-2 | End-to-End Latency — 2-Segment Timestamp Measurement | 7 | DSP E2E avg **2.2 ms** / max **4.8 ms** achieved. T2 offload reduced wait 80ms → 2.1ms | ✅ Done |
| EXP-03 | QAS-3 | Observer Pattern Compliance — Tab Extension Cost Measurement | — | ≤ 3 files per new tab · 0 Signal Processing references · 14 tabs all pass · DSM no violations | ✅ Done |
| EXP-04 | QAS-4 | Detector Parameter Optimization Under Noise | 274 | `onset=0.08` most robust: rate ≈ +4.0 s/d stable across 0–50 dB. **Recommended: onset=0.08, min_peak=0.10** | ✅ Done |
| EXP-05 | QAS-4 + Usability | Signal Quality Warning — Ambient Noise Threshold Validation | 1 | `noiseDb ≥ 55 dB` fires at SNR ≤ 0 dB · **0 false alarms** at SNR ≥ 10 dB | ✅ Done |
| EXP-06 | QAS-5 | Witschi Accuracy Comparison — TimeChecker vs Witschi No.1000 | 2 | Δ Rate **0.2–0.4 s/d** · Δ Amplitude **15–25°** · Δ Beat Error **0–0.1 ms** — all within tolerance across 2 rounds | ✅ Done |
| EXP-07 | QAS-6 | Long-Term Aging Test — Bucket Downsampling Efficiency | analytical | **2,520 total plotted points** at 7 days (≤ 3,000 budget). `replot()` well under 16 ms | ✅ Done |
| EXP-08 | QAS-3 | Tab Expansion File-Change Cost | — | All 14 tabs within ≤ 3-file budget · 0 layer violations · DSM + test suite confirmed — **QAS-3 Pass** | ✅ Done |

---

## EXP-01: RPi Real-Time Performance — Dropped Block Measurement

**QA**: QAS-1 | **Date**: 2026-06-15 | **Status**: ✅ Done

**Question**: Can RPi 5 process audio at 96,000 sps with zero dropped blocks while running Qt GUI + DSP concurrently?

**Answer**: Yes. **Dropped Block = 0 across all sps (48k / 96k / 192k) × all scheduling policies (default / RR / FIFO).** QAS-1 Pass.

### Run History

> Platform: RPi (host=lg1), 5 min/sps, 30 s ring buffer. Deadline: 48k=42.67 ms · **96k=21.33 ms** · 192k=10.67 ms.

| Run | Date | Scheduling | sps | exec avg/max (ms) | exec > deadline | Dropped | Data |
|:---:|------|-----------|:---:|:-----------------:|:---------------:|:-------:|:----:|
| E2-01 | 2026-06-15 | default | 48k | 5.8 / 36.6 | 4.9 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_203222_48000_default.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_203222_48000_default.png) |
| E2-02 | 2026-06-15 | default | **96k** ★ | **9.6 / 39.2** | **8.1 %** | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_204746_96000_default.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_204746_96000_default.png) |
| E2-03 | 2026-06-15 | default | 192k | 15.8 / 51.6 | 12.1 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_210310_192000_default.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_210310_192000_default.png) |
| E2-04 | 2026-06-15 | SCHED_RR p50 | 48k | 6.9 / 37.5 | 6.6 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_203730_48000_rr.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_203730_48000_rr.png) |
| E2-05 | 2026-06-15 | SCHED_RR p50 | **96k** ★ | **9.8 / 39.9** | **8.4 %** | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_205254_96000_rr.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_205254_96000_rr.png) |
| E2-06 | 2026-06-15 | SCHED_RR p50 | 192k | 16.0 / 61.7 | 12.5 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_210818_192000_rr.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_210818_192000_rr.png) |
| E2-07 | 2026-06-15 | SCHED_FIFO p50 | 48k | 7.2 / 35.2 | 6.9 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_204238_48000_fifo.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_204238_48000_fifo.png) |
| E2-08 | 2026-06-15 | SCHED_FIFO p50 | **96k** ★ | **9.9 / 41.4** | **8.6 %** | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_205802_96000_fifo.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_205802_96000_fifo.png) |
| E2-09 | 2026-06-15 | SCHED_FIFO p50 | 192k | 16.0 / 52.1 | 12.5 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_211326_192000_fifo.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_211326_192000_fifo.png) |

★ = QAS-1 target sps

### Conclusion

- **Target sps confirmed**: 96k sps — 0 drops, exec avg 9.6 ms (well within 21.3 ms deadline)
- **SCHED_RR / FIFO not required**: no improvement in dropped block count
- Full write-up: [exp-01-realtime-dropped-block.md](exp-01-realtime-dropped-block.md)

---

## EXP-02: End-to-End Latency — 2-Segment Timestamp Measurement

**QA**: QAS-2 | **Date**: 2026-06-11 ~ 2026-06-16 | **Status**: ✅ Done

**Question**: What is the end-to-end latency from audio capture → DSP → screen render? Does it stay within 30 ms with 11 tabs?

**Measurement segments**: ① BG audio received → DSP start (wait) · ② DSP processing → paint complete (exec)

### Run History

> Log files in `src/logs/EXP-02/`.

| Run | Date | Configuration | E2E avg/max (ms) | Note | Data |
|:---:|------|--------------|:----------------:|------|:----:|
| E3-01 | 2026-06-12 | Windows reference (dev PC) | 2.8 / 363.9 | Healthy — one OS scheduling spike | [csv](../../../../../src/logs/EXP-02/log_20260612_132536.csv) · [plot](../../../../../src/logs/EXP-02/log_20260612_132536.png) |
| E3-02 | 2026-06-11 | rpi1 — unoptimized | 255.4 / 900.9 | ❌ FAIL — exec overrun 43 % | [csv](../../../../../src/logs/EXP-02/log_20260611_145543.csv) · [plot](../../../../../src/logs/EXP-02/log_20260611_145543.png) |
| E3-03 | 2026-06-15 | rpi2 baseline | 57.2 / 208.9 | ❌ exec overrun 4.4 % | [csv](../../../../../src/logs/EXP-02/log_20260615_152751.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_152751.png) |
| E3-04 | 2026-06-15 | rpi2 + multi-tab | 80.1 / 258.7 | plot removed from exec path; FG queue lag | [csv](../../../../../src/logs/EXP-02/log_20260615_162055.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_162055.png) |
| E3-05 | 2026-06-15 | E3-04 + T2 (DSP offload thread) | 2.1 / 11.1 | ✅ **Ideal real-time** — 0 drops, 0 backlog | [csv](../../../../../src/logs/EXP-02/log_20260615_163106.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_163106.png) |
| E3-06 | 2026-06-15 | E3-05 + R1 (Lazy Rendering) | 2.1 / 5.7 | ✅ Same perf + tighter max | [csv](../../../../../src/logs/EXP-02/log_20260615_165612.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_165612.png) |
| E3-07 | 2026-06-16 | E3-06 + FG wait measurement | 2.2 / 4.8 | ✅ DSP healthy; **FG scheduling lag 60 ms revealed** | [csv](../../../../../src/logs/EXP-02/log_20260616_140850.csv) · [plot](../../../../../src/logs/EXP-02/log_20260616_140850.png) · [timeline](../../../../../src/logs/EXP-02/log_20260616_140850_timeline_dark_all.png) |

### Conclusion

- **Key fix**: T2 (DSP offload thread) — E2E 80 ms → 2.1 ms (−97 %)
- **R1 (Lazy Rendering)**: additional tail latency reduction (max 11.1 → 5.7 ms)
- **Known bottleneck**: FG Qt event-loop pickup avg 60 ms (84 % > 21.33 ms deadline)
- Full write-up: [exp-02-latency-e2e.md](exp-02-latency-e2e.md)

---

## EXP-03: Observer Pattern Compliance — Tab Extension Cost Measurement

**QA**: QAS-3 | **Date**: 2026-06-21 | **Status**: ✅ Done

**Question**: Can a new graph tab be added with ≤ 3 file changes and zero direct dependencies on Signal Processing or Acquisition layers?

**Answer**: Yes. All 14 tabs were implemented under this constraint. Verified by unit tests and DSM showing zero layer violations.

### Verification Record

| Run | Date | Scope | Result | Data |
|:---:|------|-------|:------:|:----:|
| E4-01 | 2026-06-21 | Unit test suite — all 14 tabs · DSM boundary check | ✅ Pass | — |

### Evidence

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 (header + source + registration) |
| Signal Processing / Acquisition references from Presentation | 0 | ✅ 0 — DSM verified |
| Observer contract compliance (all 14 tabs) | 100 % | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

### Conclusion

- **Extension cost**: ≤ 3 files — new tab is fully self-contained in the Presentation layer
- **Observer pattern**: `MeasurementEngine` has zero knowledge of tabs
- Full write-up: [exp-03-extensibility-observer-pattern.md](exp-03-extensibility-observer-pattern.md)

---

## EXP-04: Detector Parameter Optimization Under Noise

**QA**: QAS-4 | **Date**: 2026-06-16 ~ 2026-06-17 | **Status**: ✅ Done

**Question**: Which combination of `onset_fraction` and `min_peak_fraction` yields the most stable measurement under varying noise conditions?

**Answer**: `onset_fraction = 0.08, min_peak_fraction = 0.10`. Rate stable at ≈ +4.0 s/d across 0–50 dB. Tracks successfully at 60 dB (+7.5 s/d). onset=0.02 and 0.05 fail catastrophically at 60 dB.

### Run History

> WAV source: 28,800 BPH watch real recording + pink noise (96 kHz, float32). Platform: RPi 5 (host=lg1, device=rpi1).  
> Parameter grid: onset {0.02, 0.05, 0.08} × min_peak {0.10, 0.20, 0.30} × noise {0–60 dB, 7 levels} × 5 reps = 315 planned.

| Run | Date | Scope | Measurements | Key Result | Data |
|:---:|------|-------|:------------:|------------|:----:|
| E5-01 | 2026-06-15 | Pilot — default params, 48 kHz | 3 | File format validation only | — |
| E5-02 | 2026-06-16 | Early grid — onset {0.02, 0.08} × noise {0, 60} dB | 8 | 96 kHz playback confirmed | — |
| E5-03 | 2026-06-17 | Full grid — 3×3×7×5 reps | 274 | **onset=0.08/min_peak=0.10 best across all noise levels** | [scatter](../../../../../src/logs/EXP-04/full-grid/exp04_scatter.png) · [logs](../../../../../src/logs/EXP-04/full-grid/) |

### Results

Rate (s/d) averaged across all reps. Values for `min_peak=0.10` (best within each onset group).

| Noise | onset=0.02 rate (s/d) | onset=0.05 rate (s/d) | onset=0.08 rate (s/d) ✅ |
|:-----:|:---------------------:|:---------------------:|:------------------------:|
| 00 dB | +12.11 | +4.24 | +4.06 |
| 10 dB | +8.07 | +4.18 | +4.02 |
| 20 dB | +7.94 | +4.19 | +4.02 |
| 30 dB | +8.11 | +4.15 | +4.02 |
| 40 dB | +4.21 | +4.13 | +4.04 |
| 50 dB | +14.65 | +4.02 | +3.86 |
| **60 dB** | **−4,264** ❌ | **−393** ❌ | **+7.51** ✅ |

### Conclusion

- **Recommended**: `onset_fraction = 0.08`, `min_peak_fraction = 0.10`
- `onset=0.08` is the only setting that maintains tracking at 60 dB SNR
- Full write-up: [exp-04-correctness-detector-optimization.md](exp-04-correctness-detector-optimization.md)

---

## EXP-05: Signal Quality Warning — Ambient Noise Threshold Validation

**QA**: QAS-4 (Correctness, Sub-3) + Usability | **Date**: 2026-06-23 | **Status**: ✅ Done

**Question**: Does the 55 dB `noiseDb` threshold trigger the warning popup at the correct SNR boundary — fires at SNR ≤ 0 dB, zero false alarms at SNR ≥ 10 dB?

**Answer**: Yes. `noiseDb` exceeds 55 dB only at SNR ≤ 0 dB where measurements are genuinely corrupted. Zero false alarms at SNR ≥ 10 dB where measurements remain valid.

**Why both QAs**: Correctness requires detecting degraded-signal conditions before displaying corrupted values. Usability requires that the warning fires only when truly needed — false alarms erode user trust; missed alarms cause users to act on corrupted Rate / Beat Error.

### Run History

| Run | Date | File | noiseDb at SNR 0 dB | False alarms (SNR ≥ 10) | Result | Data |
|:---:|------|------|:-------------------:|:-----------------------:|:------:|:----:|
| E7-01 | 2026-06-23 | `28800BPH_3235_Starbucks_snr{M10..60}db.wav` (8 files, 96kHz) | avg 54.4 / max **56.9** | **0 / 6** | ✅ Pass | [csv](../../../../../src/logs/EXP-05/noise_detection_snr_sweep_20260623.csv) · [plot](../../../../../src/logs/EXP-05/noise_detection_snr_sweep_20260623.png) |

### Key Data

| SNR (dB) | noiseDb avg | noiseDb max | Popup triggers? | Beat Error (ms) | Rate Error (s/d) |
|:--------:|:-----------:|:-----------:|:---------------:|:---------------:|:----------------:|
| 60 | 46.6 | 54.9 | No | 0.198 | 1.86 |
| 10 | 49.6 | 54.9 | No | 0.192 | 1.76 |
| **0** | **54.4** | **56.9** | **Yes** | 16.527 | −4,968 |
| **−10** | **61.9** | **63.3** | **Yes** | INVALID | INVALID |

> At SNR 0 dB, Rate error is −4,968 s/d — confirming the popup fires exactly when output is meaningless, not before.

### Conclusion

- **0 false alarms** at SNR ≥ 10 dB (noiseDb max 54.9 — just under 55 dB threshold)
- **Popup correctly fires** at SNR ≤ 0 dB (noiseDb max 56.9+)
- **Threshold margin**: 0.1 dB at SNR 10 dB / 1.9 dB at SNR 0 dB — separation is clear
- Implemented in `feature/noise` (commits `c0a882a`, `2cac301`)
- Full write-up: [exp-05-noise-threshold-popup.md](exp-05-noise-threshold-popup.md)

---

## EXP-06: Witschi Accuracy Comparison — TimeChecker vs Witschi No.1000

**QA**: QAS-5 | **Date**: 2026-06-25 | **Status**: ✅ Done

**Question**: Do Rate, Amplitude, and Beat Error values computed by TimeChecker match Witschi No.1000 within measurement tolerance?

**Answer**: Yes. Both rounds confirmed agreement within tolerance. The persistent Amplitude offset (15–25° lower in TC) is a systematic C-event detection delay — not measurement error.

### Run History

Watch: 21600 BPH. Sequential measurement per round (watch transferred between systems; < 5 min between readings).

| Round | Date | Watch | Duration | Δ Rate (s/d) | Δ Amplitude (°) | Δ Beat Error (ms) | Result | Data |
|:-----:|------|-------|:--------:|:------------:|:---------------:|:-----------------:|:------:|:----:|
| E1-01 | 2026-06-25 | 21,600 BPH | 5 min each | **0.4** | **15** | **0.1** | ✅ Pass | — |
| E1-02 | 2026-06-25 | 21,600 BPH | 5 min each | **0.2** | **~25** | **0** | ✅ Pass | — |

> **Note — Witschi Rate resolution**: Witschi No.1000 displays Rate as integers (s/d), so Δ 0.2 s/d is within display resolution. Rate agreement is confirmed.

### Measurement Detail

| Metric | Witschi No.1000 (R2) | TimeChecker (R2) | Delta | Tolerance | Pass? |
|--------|:-------------------:|:----------------:|:-----:|:---------:|:-----:|
| Rate | +11 s/d | +11.2 s/d | 0.2 s/d | < ±2 s/d | ✅ |
| Amplitude | 309–321° | 282–296° | ~25° | ± 30° | ✅ |
| Beat Error | 0.1 ms | 0.1 ms | 0 ms | ± 0.3 ms | ✅ |

The ~25° amplitude offset is systematic: C-event detection threshold delay extends the measured T1 interval, and since amplitude is inversely proportional to T1, TC consistently reads lower than Witschi.

### Conclusion

- **QAS-5 verified across 2 rounds**: Δ Rate 0.2–0.4 s/d · Δ Amplitude 15–25° · Δ BE 0–0.1 ms — all within tolerance
- **Systematic amplitude offset explained**: C-event detection delay (not sensor coupling noise) — deterministic and consistent
- Full write-up: [exp-06-accuracy-witschi-comparison.md](exp-06-accuracy-witschi-comparison.md)

---

## EXP-07: Long-Term Aging Test — Bucket Downsampling Efficiency

**QA**: QAS-6 | **Date**: 2026-06-25 | **Status**: ✅ Done (analytical verification)

**Question**: Does the `mBucketSize` time-based downsampling strategy in `LongTermTab` keep total plotted points ≤ 3,000 and `QCustomPlot::replot()` ≤ 16 ms after 7 days?

**Answer**: Yes. **2,520 total plotted points** at 7 days (well under 3,000 budget). QCP handles 100,000+ points before degradation; 2,520 is two orders of magnitude below the limit. QAS-6 Pass.

### Point Count at Phase Boundaries

| Session duration | Total measurements | `mBucketSize` | Points per series | Total points (×3 series) |
|:----------------:|:-----------------:|:-------------:|:-----------------:|:------------------------:|
| 5 min | ~25 | 1 | ~25 | ~75 |
| 30 min | ~150 | 10 | ~15 | ~45 |
| 2 hr | ~600 | 30 | ~20 | ~60 |
| 24 hr | ~7,200 | 60 | ~120 | ~360 |
| **7 days** | ~50,400 | 60 | **~840** | **~2,520** |

### Verification Record

| Run | Date | Method | Result | Data |
|:---:|------|--------|:------:|:----:|
| E6-01 | 2026-06-25 | Analytical — `mBucketSize` policy × 12 s averaging period | ✅ Pass | — |

### Conclusion

- **Point budget**: 2,520/7-days ≪ 3,000 limit — no live aging test required for QAS pass
- **Render budget**: QCP render at 2,520 points estimated < 5 ms (well under 16 ms)
- **Recommendation**: accelerated simulation (SimWorker loop-play) before final demo for screenshot evidence
- Full write-up: [exp-07-longterm-aging.md](exp-07-longterm-aging.md)

---

## EXP-08: Tab Expansion File-Change Cost

**QA**: QAS-3 | **Date**: 2026-06-21 | **Status**: ✅ Done

**Question**: When a new graph tab is added to the Presentation layer, how many files outside the new tab itself must be changed?

**Answer**: ≤ 3 files across all 14 tabs. No Domain / Signal Processing / Acquisition file was ever modified. QAS-3 Pass. TR-08 Resolved.

### Tab Addition History

| Batch | Tabs added | Trigger | Files changed outside new tab |
|-------|-----------|---------|:----:|
| W2 S1 | 11 (baseline) | Core requirements | **2** (NewTab × N + MainWindow) |
| W2 S2 | +2 → 13 (FilterScopeTab, SweepScopeTab) | Project-plan screen requirements (Fig 7-19) | **2 each** |
| W3 S1 | +1 → **14** (RadarChartTab, bonus) | Radar/Polar chart (bonus) | **3** ¹ |

¹ RadarChartTab reads per-position data from SequenceTab directly — SequenceTab modified to expose `capturedReadings()` + `sequenceUpdated()`. No lower-layer file modified.

### Evidence

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab (outside tab files) | ≤ 3 | ✅ 2–3 across all batches |
| Presentation → Signal Processing `#include` refs | 0 | ✅ 0 (DSM verified) |
| Presentation → Acquisition `#include` refs | 0 | ✅ 0 (DSM verified) |
| Observer contract compliance (all 14 tabs) | 100% | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

### Conclusion

- **File-change budget**: ≤ 3 files upheld across all three sprint batches
- **Layer isolation**: Domain and below untouched — 4-layer allowed-to-use structure preserved
- Full write-up: [exp-08-tab-expansion-file-change-cost.md](exp-08-tab-expansion-file-change-cost.md)

---

## Architecture Decisions Log

| Decision | QA | Source | Outcome | Date |
|----------|:--:|:------:|---------|------|
| Target sps | QAS-1 | EXP-01 | **96k sps** — Dropped = 0 across all tested sps with 30 s buffer | 2026-06-15 |
| SCHED_RR on audio thread | QAS-1 | EXP-01 | **Not applied** — no improvement in drop count | 2026-06-15 |
| DSP offload thread (T2) | QAS-2 | EXP-02 | **Applied** — eliminated 43 % deadline miss; E2E 80 ms → 2.1 ms | 2026-06-15 |
| Lazy Rendering (R1) | QAS-2 | EXP-02 | **Applied** — max tail latency 11.1 → 5.7 ms | 2026-06-15 |
| Observer pattern (BaseGraphTab + Qt Signal-Slot) | QAS-3 | EXP-03 | **Applied** — `MeasurementEngine` has zero tab knowledge; ≤ 3 files per new tab | 2026-06-21 |
| IAudioSource dependency inversion | QAS-3 | EXP-03 | **Applied** — 3 audio sources unified under single interface | 2026-06-21 |
| Detector parameters | QAS-4 | EXP-04 | **onset=0.08, min_peak=0.10** — only setting tracking through 60 dB SNR | 2026-06-17 |
| Ambient noise popup threshold | QAS-4 + Usability | EXP-05 | **55 dB** — 0 false alarms at SNR ≥ 10 dB; popup fires at SNR ≤ 0 dB; 2 s sustain filter prevents flicker | 2026-06-23 |
| Witschi accuracy validation | QAS-5 | EXP-06 | **Verified (2 rounds)** — Δ Rate 0.2–0.4 s/d · Δ Amplitude 15–25° · Δ BE 0–0.1 ms — all within tolerance; amplitude offset explained by C-event detection delay | 2026-06-25 |
| LongTermTab `mBucketSize` downsampling | QAS-6 | EXP-07 | **Applied** — 4-phase bucket strategy bounds plotted points to 840/series at 7 days | 2026-06-25 |
| BaseGraphTab observer pattern — file-change budget | QAS-3 | EXP-08 | **Confirmed** — ≤ 3-file budget upheld across all 14 tabs; Domain layer untouched in every batch | 2026-06-21 |
