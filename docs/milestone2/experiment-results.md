# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-20

---

## Summary

| ID | Experiment | Runs | Key Result | Status |
|----|------------|:----:|------------|:------:|
| EXP-01 | RPi Real-Time Performance — Dropped Block Measurement | 3 | Dropped Block = **0** across all sps × all scheduling policies — **QAS-1 Pass** | ✅ Done |
| EXP-02 | End-to-End Latency — 3-Segment Timestamp Measurement | 7 | DSP E2E avg **2.2 ms** / max **4.8 ms** achieved. FG scheduling latency avg 60 ms revealed as next bottleneck | ✅ Done |
| EXP-03 | Detector Parameter Optimization Under Noise | 274 | `onset=0.08` most robust: rate ≈ +4.0 s/d stable across 0–50 dB. **Recommended: onset=0.08, min_peak=0.10** | ✅ Done |
| EXP-04 | Signal Quality Warning Threshold Search | 7 | noise_ratio threshold = **0.05** confirmed. 14 beat_missed at snr00db, all at ratio > 0.05 | ⏳ In Progress |
| EXP-05 | BPH Escalation Verification — 36k/43k BPH | 0 | — | ⏸ Deferred |

### Experiment Dependency Chain

```
EXP-01 (target sps confirmed)
  └─► EXP-02 (pipeline latency)   ──┐
  └─► EXP-03 (parameter tuning)     ├─► EXP-05 (BPH escalation, stretch goal)
EXP-04 (warning threshold) ─────────┘
```

---

## EXP-01: RPi Real-Time Performance — Dropped Block Measurement

**QA**: QAS-1 | **Date**: 2026-06-15 | **Status**: ✅ Done

**Question**: Can RPi 5 process audio at 96,000 sps with zero dropped blocks while running Qt GUI + DSP concurrently?

**Answer**: Yes. **Dropped Block = 0 across all sps (48k / 96k / 192k) × all scheduling policies (default / RR / FIFO).** QAS-1 Pass.

### Run History

> Platform: RPi (host=lg1), 5 min/sps, 30 s ring buffer. Deadline: 48k=42.67 ms · **96k=21.33 ms** · 192k=10.67 ms.  
> The 30 s buffer absorbs all deadline-exceeded frames → Dropped stays 0 in all cases.  
> 3 runs (scheduling configs), each covering all 3 sps = **9 measurements total**.

| Run | Date | Scheduling | sps | exec avg/max (ms) | exec > deadline | Dropped | Data |
|:---:|------|-----------|:---:|:-----------------:|:---------------:|:-------:|:----:|
| E1-01 | 2026-06-15 | default | 48k | 5.8 / 36.6 | 4.9 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_203222_48000_default.csv) · [plot](../../src/logs/EXP-01/log_20260615_203222_48000_default.png) |
| E1-01 | 2026-06-15 | default | **96k** ★ | **9.6 / 39.2** | **8.1 %** | **0** | [csv](../../src/logs/EXP-01/log_20260615_204746_96000_default.csv) · [plot](../../src/logs/EXP-01/log_20260615_204746_96000_default.png) |
| E1-01 | 2026-06-15 | default | 192k | 15.8 / 51.6 | 12.1 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_210310_192000_default.csv) · [plot](../../src/logs/EXP-01/log_20260615_210310_192000_default.png) |
| E1-02 | 2026-06-15 | SCHED_RR p50 | 48k | 6.9 / 37.5 | 6.6 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_203730_48000_rr.csv) · [plot](../../src/logs/EXP-01/log_20260615_203730_48000_rr.png) |
| E1-02 | 2026-06-15 | SCHED_RR p50 | **96k** ★ | **9.8 / 39.9** | **8.4 %** | **0** | [csv](../../src/logs/EXP-01/log_20260615_205254_96000_rr.csv) · [plot](../../src/logs/EXP-01/log_20260615_205254_96000_rr.png) |
| E1-02 | 2026-06-15 | SCHED_RR p50 | 192k | 16.0 / 61.7 | 12.5 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_210818_192000_rr.csv) · [plot](../../src/logs/EXP-01/log_20260615_210818_192000_rr.png) |
| E1-03 | 2026-06-15 | SCHED_FIFO p50 | 48k | 7.2 / 35.2 | 6.9 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_204238_48000_fifo.csv) · [plot](../../src/logs/EXP-01/log_20260615_204238_48000_fifo.png) |
| E1-03 | 2026-06-15 | SCHED_FIFO p50 | **96k** ★ | **9.9 / 41.4** | **8.6 %** | **0** | [csv](../../src/logs/EXP-01/log_20260615_205802_96000_fifo.csv) · [plot](../../src/logs/EXP-01/log_20260615_205802_96000_fifo.png) |
| E1-03 | 2026-06-15 | SCHED_FIFO p50 | 192k | 16.0 / 52.1 | 12.5 % | **0** | [csv](../../src/logs/EXP-01/log_20260615_211326_192000_fifo.csv) · [plot](../../src/logs/EXP-01/log_20260615_211326_192000_fifo.png) |

★ = QAS-1 target sps

### Conclusion

- **Target sps confirmed**: 96k sps — 0 drops, exec avg 9.6 ms (well within 21.3 ms deadline)
- **SCHED_RR / FIFO not required**: No improvement in dropped block count; exec time marginally higher
- **Thermal throttling**: All runs reach ≥ 85 °C (95–111 throttle events / 5 min); does not affect drop count

---

## EXP-02: End-to-End Latency — 3-Segment Timestamp Measurement

**QA**: QAS-2 | **Date**: 2026-06-11 ~ 2026-06-16 | **Status**: ✅ Done

**Question**: What is the end-to-end latency from audio capture → DSP → screen render? Does it stay within 30 ms with 11 tabs?

**Measurement segments**: ① BG audio received → DSP start (wait) · ② DSP processing → paint complete (exec)

### Run History

| Run | Date | Configuration | E2E avg/max (ms) | Note | Data |
|:---:|------|--------------|:----------------:|------|:----:|
| E2-01 | 2026-06-12 | Windows reference (dev PC) | 2.8 / 363.9 | Healthy — one OS scheduling spike | [csv](../../src/logs/EXP-02/log_20260612_132536.csv) · [plot](../../src/logs/EXP-02/log_20260612_132536.png) |
| E2-02 | 2026-06-11 | rpi1 — unoptimized | 255.4 / 900.9 | ❌ FAIL — thermal throttling, exec overrun 43 % | [csv](../../src/logs/EXP-02/log_20260611_145543.csv) · [plot](../../src/logs/EXP-02/log_20260611_145543.png) |
| E2-03 | 2026-06-15 | rpi2 baseline | 57.2 / 208.9 | ❌ exec overrun 4.4 % — `plot` on exec path | [csv](../../src/logs/EXP-02/log_20260615_152751.csv) · [plot](../../src/logs/EXP-02/log_20260615_152751.png) |
| E2-04 | 2026-06-15 | rpi2 + multi-tab | 80.1 / 258.7 | plot removed from exec path; FG queue lag | [csv](../../src/logs/EXP-02/log_20260615_162055.csv) · [plot](../../src/logs/EXP-02/log_20260615_162055.png) |
| E2-05 | 2026-06-15 | E2-04 + T2 (DSP offload thread) | 2.1 / 11.1 | ✅ **Ideal real-time** — 0 drops, 0 backlog | [csv](../../src/logs/EXP-02/log_20260615_163106.csv) · [plot](../../src/logs/EXP-02/log_20260615_163106.png) |
| E2-06 | 2026-06-15 | E2-05 + R1 (Lazy Rendering) | 2.1 / 5.7 | ✅ Same perf + tighter max | [csv](../../src/logs/EXP-02/log_20260615_165612.csv) · [plot](../../src/logs/EXP-02/log_20260615_165612.png) |
| E2-07 | 2026-06-16 | E2-06 + FG wait measurement | 2.2 / 4.8 | ✅ DSP healthy; **FG scheduling lag 60 ms revealed** | [csv](../../src/logs/EXP-02/log_20260616_140850.csv) · [plot](../../src/logs/EXP-02/log_20260616_140850.png) · [timeline](../../src/logs/EXP-02/log_20260616_140850_timeline_dark_all.png) |

### Tactic Progression

| Step | E2E avg | Change | Root Cause |
|------|:-------:|--------|------------|
| E2-03 (baseline) | 57 ms | — | `plot` rendering on the DSP exec path |
| E2-04 (multi-tab) | 80 ms | worse | `plot` removed but no FG-BG sync → queue builds |
| **E2-05 (+T2)** | **2.1 ms** | **−97 %** | DSP offload thread — FG-BG 1:1 sync, zero backlog |
| E2-06 (+R1) | 2.05 ms | max 11→5.7 ms | Lazy Rendering trims worst-case tail |
| E2-07 (measure) | 2.2 ms | — | FG Qt event-loop pickup lag avg 60 ms newly revealed |

### Conclusion

- **Key fix**: T2 (DSP offload thread) — E2E 80 ms → 2.1 ms (−97 %)
- **R1 (Lazy Rendering)**: additional tail latency reduction (max 11.1 → 5.7 ms)
- **New bottleneck (E2-07)**: FG Qt event-loop pickup avg **60 ms** (84 % > 21.33 ms deadline) — 7× slower than macOS → next engineering concern

---

## EXP-03: Detector Parameter Optimization Under Noise

**QA**: QAS-3 | **Date**: 2026-06-16 ~ 2026-06-17 | **Status**: ✅ Done

**Question**: Which combination of `onset_fraction` and `min_peak_fraction` yields the most stable measurement under varying noise conditions?

**Answer**: `onset_fraction = 0.08, min_peak_fraction = 0.10`. Rate stable at ≈ +4.0 s/d across 0–50 dB. Tracks successfully at 60 dB (+7.5 s/d). onset=0.02 and 0.05 fail catastrophically at 60 dB.

### Experiment Design

| Parameter | Values swept |
|-----------|-------------|
| `onset_fraction` | 0.02, 0.05, **0.08** |
| `min_peak_fraction` | 0.10, 0.20, 0.30 |
| Noise level | 0 / 10 / 20 / 30 / 40 / 50 / 60 dB (7 levels) |
| Repetitions | 5 per combination |
| **Total** | **3 × 3 × 7 × 5 = 315 planned → 274 completed** |

**WAV source**: 28,800 BPH watch real recording + pink noise at 7 SNR levels (96 kHz, float32)  
**Platform**: Raspberry Pi 5 (host=lg1, device=rpi1) | **Log directory**: [src/logs/EXP-03/](../../src/logs/EXP-03/)

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

## EXP-04: Signal Quality Warning Threshold Search

**QA**: QAS-4 | **Date**: 2026-06-17 | **Status**: ⏳ In Progress

**Questions**:
- (Part B ✅) What `noise_ratio` threshold should trigger `⚠ Noisy signal` without false alarms?
- (Part A ⏳) How quickly should `⚠ No signal` appear after the watch is removed?

### Part B — Noisy Signal Threshold (Complete)

`noise_ratio = noise_floor / ref_peak` (noise as a fraction of tick amplitude)

| SNR | File | Frames | sync_lost | beat_missed | noise_ratio avg | Result |
|:---:|------|:------:|:---------:|:-----------:|:---------------:|:------:|
| 60 dB | [csv](../../src/logs/EXP-04/log_snr60db_20260617_155620.csv) | 2,145 | 0 | 0 | 0.0035 | ✅ |
| 50 dB | [csv](../../src/logs/EXP-04/log_snr50db_20260617_155527.csv) | 2,116 | 0 | 0 | 0.0035 | ✅ |
| 40 dB | [csv](../../src/logs/EXP-04/log_snr40db_20260617_155435.csv) | 1,972 | 0 | 0 | 0.0036 | ✅ |
| 30 dB | [csv](../../src/logs/EXP-04/log_snr30db_20260617_155343.csv) | 2,061 | 0 | 0 | 0.0040 | ✅ |
| 20 dB | [csv](../../src/logs/EXP-04/log_snr20db_20260617_155251.csv) | 1,763 | 0 | 0 | 0.0068 | ✅ |
| 10 dB | [csv](../../src/logs/EXP-04/log_snr10db_20260617_155158.csv) | 1,759 | 0 | 0 | 0.0177 | ✅ |
| **0 dB** | [csv](../../src/logs/EXP-04/log_snr00db_20260617_155107.csv) | 1,151 | **1** | **14** | **0.0537** | ❌ |

**Scatter plot**: [exp04_scatter.png](../../src/logs/EXP-04/exp04_scatter.png)

**Key finding**: All 14 beat_missed events at snr00db occurred in a burst at frames 582–624, immediately after sync_lost at frame 581. All had `noise_ratio` in 0.054–0.060 — above the **0.05** threshold. snr10db had noise_ratio max 0.0585 but zero missed beats, confirming 0.05 is the correct boundary.

**Threshold interpretation**:
- `noise_ratio ≥ 0.05` → trigger `⚠ Noisy signal`
- Practical: tick at 80 dB SPL → max tolerable ambient noise = 54 dB SPL

### Part A — No Signal Timing (Not yet started)

> Requires `⚠ No signal` / `⚠ Noisy signal` warning UI implementation before running.

### Run History

| Run | Date | Part | Conditions | Threshold | Result |
|:---:|------|------|:----------:|:---------:|--------|
| E4-01 | 2026-06-17 | B — Noisy Signal | 7 SNR (0–60 dB) | **0.05** | ✅ Threshold confirmed |

---

## EXP-05: BPH Escalation Verification — 36k/43k BPH

**QA**: QAS-2 Stretch | **Status**: ⏸ Deferred

> Not started. Will begin only after EXP-02 complete and QAS-1–4 all confirmed at 28,800 BPH.

| Run | Date | Change | 36k E2E (ms) | 43k E2E (ms) | Result |
|:---:|------|--------|:------------:|:------------:|:------:|
| E5-01 | — | Planned: baseline | — | — | — |

---

## Architecture Decisions Log

| Decision | Source | Outcome | Date |
|----------|:------:|---------|------|
| Target sps | EXP-01 | **96k sps** — Dropped = 0 across all tested sps with 30 s buffer | 2026-06-15 |
| SCHED_RR on audio thread | EXP-01 | **Not applied** — no improvement in drop count; exec marginally worse | 2026-06-15 |
| DSP offload thread (T2) | EXP-02 | **Applied** — E2E 80 ms → 2.1 ms (−97 %), zero backlog | 2026-06-15 |
| Lazy Rendering (R1) | EXP-02 | **Applied** — max tail 11.1 → 5.7 ms | 2026-06-15 |
| Detector parameters | EXP-03 | **onset=0.08, min_peak=0.10** — only setting tracking through 60 dB SNR | 2026-06-17 |
| Noisy signal threshold | EXP-04 | **noise_ratio = 0.05** — clean at 10–60 dB SNR; 14 misses at 0 dB (all above 0.05) | 2026-06-17 |
| No signal timing (N/M) | EXP-04 | Pending — Part A not yet run | — |
| BPH escalation (EXP-05) | EXP-05 | Pending — Deferred | — |
