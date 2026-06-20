# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-20

---

## Summary

| ID | QA | Experiment | Runs | Key Result | Status |
|----|----|------------|:----:|------------|:------:|
| EXP-01 | QAS-1 | WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000 | 0 | — | ⏸ Planned (W5 S1 6/29) |
| EXP-02 | QAS-2 | RPi Real-Time Performance — Dropped Block Measurement | 9 | Dropped Block = **0** across all sps × all scheduling policies — **QAS-2 Pass** | ✅ Done |
| EXP-03 | QAS-3 | End-to-End Latency — 3-Segment Timestamp Measurement | 7 | DSP E2E avg **2.2 ms** / max **4.8 ms** achieved. FG scheduling latency avg 60 ms revealed as next bottleneck | ✅ Done |
| EXP-04 | QAS-4 | Observer Pattern Compliance — Tab Extension Cost Measurement | — | ≤ 3 files per new tab · 0 Signal Processing references · 14 tabs all pass · DSM no violations | ✅ Done |
| EXP-05 | QAS-5 | Detector Parameter Optimization Under Noise | 274 | `onset=0.08` most robust: rate ≈ +4.0 s/d stable across 0–50 dB. **Recommended: onset=0.08, min_peak=0.10** | ✅ Done |

### Experiment Dependency Chain

```
EXP-02 (QAS-2: target sps)
EXP-03 (QAS-3: pipeline latency)   ──┐
EXP-05 (QAS-5: detector params)   ──► EXP-01 (QAS-1: WeiShi accuracy)
EXP-04 (QAS-4: extensibility — independent)
```

---

## EXP-01: WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000

**QA**: QAS-1 | **Date**: Planned 2026-06-29 ~ 2026-06-30 | **Status**: ⏸ Planned

**Question**: Do Rate, Amplitude, and Beat Error values computed by TimeChecker match WeiShi No.1000 within measurement tolerance?

**Answer**: Not yet run — scheduled W5 S1 (2026-06-29).

### Run History

| Run | Date | Watch | Duration | Δ Rate (s/d) | Δ Amplitude (°) | Δ Beat Error (ms) | Result | Data |
|:---:|------|-------|:--------:|:------------:|:---------------:|:-----------------:|:------:|:----:|
| E1-01 | — | 28,800 BPH reference | — | — | — | — | — | — |

### Experiment Plan

1. Place the same mechanical watch on the WeiShi No.1000 reference device and on the TimeChecker (RPi 5, 96kHz, real microphone) simultaneously
2. Run both for ≥ 5 minutes
3. Record Rate (s/d), Amplitude (°), Beat Error (ms) from both devices
4. Compare: |TimeChecker − WeiShi| for each metric
5. Pass condition: Δ Rate < 0.5 s/d · Δ Amplitude < 5° · Δ Beat Error < 0.1 ms

---

## EXP-02: RPi Real-Time Performance — Dropped Block Measurement

**QA**: QAS-2 | **Date**: 2026-06-15 | **Status**: ✅ Done

**Question**: Can RPi 5 process audio at 96,000 sps with zero dropped blocks while running Qt GUI + DSP concurrently?

**Answer**: Yes. **Dropped Block = 0 across all sps (48k / 96k / 192k) × all scheduling policies (default / RR / FIFO).** QAS-2 Pass.

### Run History

> Platform: RPi (host=lg1), 5 min/sps, 30 s ring buffer. Deadline: 48k=42.67 ms · **96k=21.33 ms** · 192k=10.67 ms.  
> The 30 s buffer absorbs all deadline-exceeded frames → Dropped stays 0 in all cases.  
> **9 runs total** (E2-01 ~ E2-09): 3 scheduling configs × 3 sps. Log files in `src/logs/EXP-01/` (legacy directory name).

| Run | Date | Scheduling | sps | exec avg/max (ms) | exec > deadline | Dropped | Data |
|:---:|------|-----------|:---:|:-----------------:|:---------------:|:-------:|:----:|
| E2-01 | 2026-06-15 | default | 48k | 5.8 / 36.6 | 4.9 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_203222_48000_default.csv) · [plot](../../src/logs/EXP-01/log_20260615_203222_48000_default.png) |
| E2-02 | 2026-06-15 | default | **96k** ★ | **9.6 / 39.2** | **8.1 %** | **0** | [csv](../../src/logs/EXP-01/log_20260615_204746_96000_default.csv) · [plot](../../src/logs/EXP-01/log_20260615_204746_96000_default.png) |
| E2-03 | 2026-06-15 | default | 192k | 15.8 / 51.6 | 12.1 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_210310_192000_default.csv) · [plot](../../src/logs/EXP-01/log_20260615_210310_192000_default.png) |
| E2-04 | 2026-06-15 | SCHED_RR p50 | 48k | 6.9 / 37.5 | 6.6 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_203730_48000_rr.csv) · [plot](../../src/logs/EXP-01/log_20260615_203730_48000_rr.png) |
| E2-05 | 2026-06-15 | SCHED_RR p50 | **96k** ★ | **9.8 / 39.9** | **8.4 %** | **0** | [csv](../../src/logs/EXP-01/log_20260615_205254_96000_rr.csv) · [plot](../../src/logs/EXP-01/log_20260615_205254_96000_rr.png) |
| E2-06 | 2026-06-15 | SCHED_RR p50 | 192k | 16.0 / 61.7 | 12.5 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_210818_192000_rr.csv) · [plot](../../src/logs/EXP-01/log_20260615_210818_192000_rr.png) |
| E2-07 | 2026-06-15 | SCHED_FIFO p50 | 48k | 7.2 / 35.2 | 6.9 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_204238_48000_fifo.csv) · [plot](../../src/logs/EXP-01/log_20260615_204238_48000_fifo.png) |
| E2-08 | 2026-06-15 | SCHED_FIFO p50 | **96k** ★ | **9.9 / 41.4** | **8.6 %** | **0** | [csv](../../src/logs/EXP-01/log_20260615_205802_96000_fifo.csv) · [plot](../../src/logs/EXP-01/log_20260615_205802_96000_fifo.png) |
| E2-09 | 2026-06-15 | SCHED_FIFO p50 | 192k | 16.0 / 52.1 | 12.5 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_211326_192000_fifo.csv) · [plot](../../src/logs/EXP-01/log_20260615_211326_192000_fifo.png) |

★ = QAS-2 target sps

### Conclusion

- **Target sps confirmed**: 96k sps — 0 drops, exec avg 9.6 ms (well within 21.3 ms deadline)
- **SCHED_RR / FIFO not required**: No improvement in dropped block count; exec time marginally higher
- **Thermal throttling**: All runs reach ≥ 85 °C (95–111 throttle events / 5 min); does not affect drop count

---

## EXP-03: End-to-End Latency — 3-Segment Timestamp Measurement

**QA**: QAS-3 | **Date**: 2026-06-11 ~ 2026-06-16 | **Status**: ✅ Done

**Question**: What is the end-to-end latency from audio capture → DSP → screen render? Does it stay within 30 ms with 11 tabs?

**Measurement segments**: ① BG audio received → DSP start (wait) · ② DSP processing → paint complete (exec)

### Run History

> Log files in `src/logs/EXP-02/` (legacy directory name).

| Run | Date | Configuration | E2E avg/max (ms) | Note | Data |
|:---:|------|--------------|:----------------:|------|:----:|
| E3-01 | 2026-06-12 | Windows reference (dev PC) | 2.8 / 363.9 | Healthy — one OS scheduling spike | [csv](../../src/logs/EXP-02/log_20260612_132536.csv) · [plot](../../src/logs/EXP-02/log_20260612_132536.png) |
| E3-02 | 2026-06-11 | rpi1 — unoptimized | 255.4 / 900.9 | ❌ FAIL — thermal throttling, exec overrun 43 % | [csv](../../src/logs/EXP-02/log_20260611_145543.csv) · [plot](../../src/logs/EXP-02/log_20260611_145543.png) |
| E3-03 | 2026-06-15 | rpi2 baseline | 57.2 / 208.9 | ❌ exec overrun 4.4 % — `plot` on exec path | [csv](../../src/logs/EXP-02/log_20260615_152751.csv) · [plot](../../src/logs/EXP-02/log_20260615_152751.png) |
| E3-04 | 2026-06-15 | rpi2 + multi-tab | 80.1 / 258.7 | plot removed from exec path; FG queue lag | [csv](../../src/logs/EXP-02/log_20260615_162055.csv) · [plot](../../src/logs/EXP-02/log_20260615_162055.png) |
| E3-05 | 2026-06-15 | E3-04 + T2 (DSP offload thread) | 2.1 / 11.1 | ✅ **Ideal real-time** — 0 drops, 0 backlog | [csv](../../src/logs/EXP-02/log_20260615_163106.csv) · [plot](../../src/logs/EXP-02/log_20260615_163106.png) |
| E3-06 | 2026-06-15 | E3-05 + R1 (Lazy Rendering) | 2.1 / 5.7 | ✅ Same perf + tighter max | [csv](../../src/logs/EXP-02/log_20260615_165612.csv) · [plot](../../src/logs/EXP-02/log_20260615_165612.png) |
| E3-07 | 2026-06-16 | E3-06 + FG wait measurement | 2.2 / 4.8 | ✅ DSP healthy; **FG scheduling lag 60 ms revealed** | [csv](../../src/logs/EXP-02/log_20260616_140850.csv) · [plot](../../src/logs/EXP-02/log_20260616_140850.png) · [timeline](../../src/logs/EXP-02/log_20260616_140850_timeline_dark_all.png) |

### Optimization Progression (E3-03 → E3-07)

> E3-01 (Windows reference) and E3-02 (RPi unoptimized — thermal throttling) are baselines, not part of the RPi optimization chain.

| Step | E2E avg | Change | Root Cause |
|------|:-------:|--------|------------|
| E3-03 (baseline) | 57 ms | — | `plot` rendering on the DSP exec path |
| E3-04 (multi-tab) | 80 ms | worse | `plot` removed but no FG-BG sync → queue builds |
| **E3-05 (+T2)** | **2.1 ms** | **−97 %** | DSP offload thread — FG-BG 1:1 sync, zero backlog |
| E3-06 (+R1) | 2.05 ms | max 11→5.7 ms | Lazy Rendering trims worst-case tail |
| E3-07 (measure) | 2.2 ms | — | FG Qt event-loop pickup lag avg 60 ms newly revealed |

### Conclusion

- **Key fix**: T2 (DSP offload thread) — E2E 80 ms → 2.1 ms (−97 %)
- **R1 (Lazy Rendering)**: additional tail latency reduction (max 11.1 → 5.7 ms)
- **New bottleneck (E3-07)**: FG Qt event-loop pickup avg **60 ms** (84 % > 21.33 ms deadline) — 7× slower than macOS → next engineering concern

---

## EXP-04: Observer Pattern Compliance — Tab Extension Cost Measurement

**QA**: QAS-4 | **Date**: 2026-06-21 | **Status**: ✅ Done

**Question**: Can a new graph tab be added with ≤ 3 file changes and zero direct dependencies on Signal Processing or Acquisition layers?

**Answer**: Yes. All 14 tabs were implemented under this constraint. Verified by unit tests and DSM showing zero layer violations.

### Run History

| Run | Date | Scope | Result | Data |
|:---:|------|-------|:------:|:----:|
| E4-01 | 2026-06-21 | Unit test suite — all 14 tabs · DSM boundary check | ✅ Pass | [unit-test-results](final/references/unit-test-results.md) |

### Evidence

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 (header + source + registration) |
| Signal Processing / Acquisition references from Presentation | 0 | ✅ 0 — DSM verified |
| Observer contract compliance (all 14 tabs) | 100 % | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

### Conclusion

- **Extension cost**: ≤ 3 files — new tab is fully self-contained in the Presentation layer
- **Observer pattern**: `MeasurementEngine` has zero knowledge of tabs; all 14 tabs receive identical `Measurement` via Qt Signal-Slot (`QueuedConnection`)
- **Layer boundary**: DSM confirms no upward dependencies from Signal Processing to Presentation ✅

---

## EXP-05: Detector Parameter Optimization Under Noise

**QA**: QAS-5 | **Date**: 2026-06-16 ~ 2026-06-17 | **Status**: ✅ Done

**Question**: Which combination of `onset_fraction` and `min_peak_fraction` yields the most stable measurement under varying noise conditions?

**Answer**: `onset_fraction = 0.08, min_peak_fraction = 0.10`. Rate stable at ≈ +4.0 s/d across 0–50 dB. Tracks successfully at 60 dB (+7.5 s/d). onset=0.02 and 0.05 fail catastrophically at 60 dB.

### Run History

> WAV source: 28,800 BPH watch real recording + pink noise (96 kHz, float32). Platform: RPi 5 (host=lg1, device=rpi1).  
> Parameter grid: onset {0.02, 0.05, 0.08} × min_peak {0.10, 0.20, 0.30} × noise {0–60 dB, 7 levels} × 5 reps = 315 planned.  
> Log files in `src/logs/EXP-03/` (legacy directory name).

| Run | Date | Scope | Measurements | Key Result | Data |
|:---:|------|-------|:------------:|------------|:----:|
| E5-01 | 2026-06-15 | Pilot — default params, 48 kHz | 3 | File format validation only; no detection data | — |
| E5-02 | 2026-06-16 | Early grid — onset {0.02, 0.08} × noise {0, 60} dB, 96 kHz | 8 | 96 kHz playback confirmed; no detection data | — |
| E5-03 | 2026-06-17 | Full grid — 3×3×7 onset×min_peak×noise × 5 reps | 274 | **onset=0.08/min_peak=0.10 best across all noise levels** | [logs](../../src/logs/EXP-03/) |

### Results

Rate (s/d) averaged across all reps. Values for `min_peak=0.10` (best within each onset group; `min_peak` has minimal effect).

| Noise | onset=0.02 rate (s/d) | onset=0.05 rate (s/d) | onset=0.08 rate (s/d) ✅ |
|:-----:|:---------------------:|:---------------------:|:------------------------:|
| 00 dB | +12.11 | +4.24 | +4.06 |
| 10 dB | +8.07 | +4.18 | +4.02 |
| 20 dB | +7.94 | +4.19 | +4.02 |
| 30 dB | +8.11 | +4.15 | +4.02 |
| 40 dB | +4.21 | +4.13 | +4.04 |
| 50 dB | +14.65 | +4.02 | +3.86 |
| **60 dB** | **−4,264** ❌ | **−393** ❌ | **+7.51** ✅ |

> onset=0.08 is the only setting that maintains tracking at 60 dB SNR. onset=0.02 and 0.05 fail catastrophically. `min_peak` has negligible effect within the onset=0.08 group — `min_peak=0.10` selected for slightly lowest beat_error.

### Sample Data Files

Representative CSVs (onset=0.08 / min_peak=0.10, best setting):

| Noise | Rep 1 | Rep 2 | Rep 3 |
|:-----:|-------|-------|-------|
| 00 dB | [csv](../../src/logs/EXP-03/log_20260617_111942_onset008_minpk010_noise00db_r2.csv) | [csv](../../src/logs/EXP-03/log_20260617_112029_onset008_minpk010_noise00db_r3.csv) | [csv](../../src/logs/EXP-03/log_20260617_112116_onset008_minpk010_noise00db_r4.csv) |
| 30 dB | [csv](../../src/logs/EXP-03/log_20260617_113041_onset008_minpk010_noise30db_r1.csv) | [csv](../../src/logs/EXP-03/log_20260617_113128_onset008_minpk010_noise30db_r2.csv) | [csv](../../src/logs/EXP-03/log_20260617_131415_onset008_minpk010_noise30db_r3.csv) |
| 60 dB | [csv](../../src/logs/EXP-03/log_20260617_142920_onset008_minpk010_noise60db_r2.csv) | [csv](../../src/logs/EXP-03/log_20260617_143006_onset008_minpk010_noise60db_r3.csv) | [csv](../../src/logs/EXP-03/log_20260617_143053_onset008_minpk010_noise60db_r4.csv) |

> Full dataset (274 CSV files): [src/logs/EXP-03/](../../src/logs/EXP-03/)

### Conclusion

- **Recommended**: `onset_fraction = 0.08`, `min_peak_fraction = 0.10`
- **Noise robustness**: onset=0.08 tracks at 60 dB SNR; onset=0.05 and 0.02 fail completely
- Detector.cpp default parameters updated accordingly

---

## Architecture Decisions Log

| Decision | QA | Source | Outcome | Date |
|----------|:--:|:------:|---------|------|
| Target sps | QAS-2 | EXP-02 | **96k sps** — Dropped = 0 across all tested sps with 30 s buffer | 2026-06-15 |
| SCHED_RR on audio thread | QAS-2 | EXP-02 | **Not applied** — no improvement in drop count; exec marginally worse | 2026-06-15 |
| DSP offload thread (T2) | QAS-2 | EXP-03 | **Applied** — eliminated 43 % deadline miss; E2E 80 ms → 2.1 ms | 2026-06-15 |
| Lazy Rendering (R1) | QAS-3 | EXP-03 | **Applied** — max tail latency 11.1 → 5.7 ms | 2026-06-15 |
| Detector parameters | QAS-5 | EXP-05 | **onset=0.08, min_peak=0.10** — only setting tracking through 60 dB SNR | 2026-06-17 |
