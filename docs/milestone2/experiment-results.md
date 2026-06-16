# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-15

---

## Summary

| ID | Experiment | Runs | Latest Key Result | Status |
|----|------------|:----:|-------------------|:------:|
| EXP-01 | RPi Real-Time Performance — Dropped Block Measurement | 3 | Dropped Block = **0** across all sps (48k/96k/192k) × all scheduling (default/RR/FIFO) — **QAS-1 Pass** | ✅ Done |
| EXP-02 | End-to-End Latency — 3-Segment Timestamp Measurement | 2 | Baseline R2 (RPi): **real-time FAIL** — exec ~20 ms overruns ~21 ms deadline (43 %), single-core + thermal throttle. R1 (Windows) ref: 2.8 ms | ⏳ In Progress |
| EXP-03 | Detector Parameter Optimization Under Noise Conditions | 0 | — | 📅 Planned |
| EXP-04 | Signal Quality Warning Threshold Search | 0 | — | 📅 Planned |
| EXP-05 | BPH Escalation Verification — 36k/43k BPH | 0 | — | ⏸ Deferred |

> Status legend: ✅ Done · ⏳ In Progress · 📅 Planned · ⏸ Deferred · ❌ Cancelled  
> Update **Runs** count and **Latest Key Result** after each run.

### Experiment Dependency Chain

```
EXP-01 (SPS confirmed)
  └─► EXP-02 (latency measurement)   ──┐
  └─► EXP-03 (parameter tuning)        ├─► EXP-05 (BPH escalation, stretch goal)
EXP-04 (warning threshold) ────────────┘
       └─ prerequisite: warning UI implemented
```

> EXP-03, EXP-04 begin after EXP-01 is complete.  
> EXP-05 begins only after EXP-02 is complete AND QAS-1~4 all confirmed.

---

## EXP-01: RPi Real-Time Performance — Dropped Block Measurement

**Linked QA**: QAS-1 | **Linked Risk**: TR-01, TR-02  
**Status**: ✅ Done  
**Date**: 2026-06-15

**Question**: Can RPi 5 achieve Dropped Block = 0 at 96,000 sps while running Qt GUI + DSP concurrently? If not, what is the maximum sps that can be processed stably?

**Answer**: Yes. Dropped Block = 0 at all tested sps (48k / 96k / 192k) under all scheduling policies. QAS-1 Pass.

### Run History

> 3 runs executed 2026-06-15 on RPi (host=lg1, platform=debian). Each run covers all 3 sps simultaneously (run_experiment1.sh). Duration = 5 min per sps. Buffer = 30 s.

| Run | Date | Change from Previous | 48k Dropped/min | 96k Dropped/min | 192k Dropped/min | Scheduling | Better? | Next Action |
|:---:|------|----------------------|:---------------:|:---------------:|:----------------:|:----------:|:-------:|-------------|
| R1 | 2026-06-15 | Baseline (default OS scheduling) | **0** | **0** | **0** | default | — | Try RT scheduling |
| R2 | 2026-06-15 | SCHED_RR priority 50 | **0** | **0** | **0** | SCHED_RR | = | Try FIFO |
| R3 | 2026-06-15 | SCHED_FIFO priority 50 | **0** | **0** | **0** | SCHED_FIFO | = | Complete |

### Per-Run Detail

<details>
<summary><b>R1</b> — 2026-06-15 · default scheduling · all sps</summary>

**Files**: [48k csv](../../src/logs/EXP-01/log_20260615_203222_48000_default.csv) · [96k csv](../../src/logs/EXP-01/log_20260615_204746_96000_default.csv) · [192k csv](../../src/logs/EXP-01/log_20260615_210310_192000_default.csv)

| sps | frames | Dropped | exec avg (ms) | exec > deadline | temp avg / max | throttle events | CPU dominant core |
|-----|-------:|:-------:|:-------------:|:---------------:|:--------------:|:---------------:|:-----------------:|
| 48k | 11,810 | **0** | 5.8 | 578 / 11,810 (4.9%) | 83.7 °C / 85.9 °C | 111 | cpu2 46% |
| 96k | 10,577 | **0** | 9.6 | 853 / 10,577 (8.1%) | 85.1 °C / 86.5 °C | 105 | cpu2 66% |
| 192k | 9,855 | **0** | 15.8 | 1,190 / 9,855 (12.1%) | 85.4 °C / 87.5 °C | 98 | cpu0 46%, cpu1 43% |

**Per-core CPU (avg)**: 48k: cpu0=14% cpu1=32% **cpu2=46%** cpu3=26% | 96k: cpu0=18% cpu1=18% **cpu2=66%** cpu3=12% | 192k: cpu0=46% cpu1=43% cpu2=16% cpu3=7%

**Observations**: At 48k/96k audio path is pinned to cpu2 (single-core pattern). At 192k load spreads across cpu0/1. Thermal throttling occurs in all runs (temp ≥ 85 °C), consistent with EXP-02 baseline. exec > deadline grows with sps (4.9% → 8.1% → 12.1%) but 30 s buffer absorbs all backlog → Dropped = 0.

</details>

<details>
<summary><b>R2</b> — 2026-06-15 · SCHED_RR priority 50 · all sps</summary>

**Files**: [48k csv](../../src/logs/EXP-01/log_20260615_203730_48000_rr.csv) · [96k csv](../../src/logs/EXP-01/log_20260615_205254_96000_rr.csv) · [192k csv](../../src/logs/EXP-01/log_20260615_210818_192000_rr.csv)

| sps | frames | Dropped | exec avg (ms) | exec > deadline | temp avg / max | throttle events | CPU pattern |
|-----|-------:|:-------:|:-------------:|:---------------:|:--------------:|:---------------:|:------------|
| 48k | 10,357 | **0** | 6.9 | 682 / 10,357 (6.6%) | 84.7 °C / 86.5 °C | 103 | balanced (cpu0-3 ≈ 26-31%) |
| 96k | 10,320 | **0** | 9.8 | 866 / 10,320 (8.4%) | 85.3 °C / 86.5 °C | 103 | balanced (cpu0-3 ≈ 24-31%) |
| 192k | 9,585 | **0** | 16.0 | 1,197 / 9,585 (12.5%) | 85.4 °C / 87.0 °C | 95 | balanced (cpu0-3 ≈ 21-31%) |

**Observation**: SCHED_RR distributes CPU load evenly across all 4 cores (no single-core dominance), but exec time is slightly higher than default — likely because `vcgencmd get_throttled` subprocess is more expensive to fork under RT scheduling. Dropped = 0 unchanged.

</details>

<details>
<summary><b>R3</b> — 2026-06-15 · SCHED_FIFO priority 50 · all sps</summary>

**Files**: [48k csv](../../src/logs/EXP-01/log_20260615_204238_48000_fifo.csv) · [96k csv](../../src/logs/EXP-01/log_20260615_205802_96000_fifo.csv) · [192k csv](../../src/logs/EXP-01/log_20260615_211326_192000_fifo.csv)

| sps | frames | Dropped | exec avg (ms) | exec > deadline | temp avg / max | throttle events | CPU pattern |
|-----|-------:|:-------:|:-------------:|:---------------:|:--------------:|:---------------:|:------------|
| 48k | 10,299 | **0** | 7.2 | 715 / 10,299 (6.9%) | 84.9 °C / 87.5 °C | 102 | balanced (cpu0-3 ≈ 24-30%) |
| 96k | 9,921 | **0** | 9.9 | 858 / 9,921 (8.6%) | 85.4 °C / 87.0 °C | 99 | balanced (cpu0-3 ≈ 27-30%) |
| 192k | 9,596 | **0** | 16.0 | 1,197 / 9,596 (12.5%) | 85.4 °C / 87.0 °C | 95 | balanced (cpu0-3 ≈ 22-30%) |

**Observation**: Nearly identical to SCHED_RR. No additional benefit over RR.

</details>

### Current Best

- **Run**: R1 / R2 / R3 — all equivalent (Dropped = 0 in all conditions)
- **Recommended sample rate**: **96k sps** (QAS-1 target; 0 drops confirmed, exec avg 9.6 ms well under 21.3 ms deadline)
- **Graceful degradation fallback needed**: No — 192k also achieves 0 drops with 30 s buffer
- **SCHED_RR effect**: No improvement in Dropped Block count. CPU load is distributed more evenly but exec time is marginally higher. **SCHED_RR not required.**
- **Thermal throttling**: All runs operate at ≥ 85 °C with sustained throttling (95–111 events / 5 min). Cooling improvement would reduce throttling but does not affect drop count.
- **Architecture Decision**: → see [Architecture Decisions Log](#architecture-decisions-log)

---

## EXP-02: End-to-End Latency — 3-Segment Timestamp Measurement

**Linked QA**: QAS-2 | **Linked Risk**: TR-03, TR-04  
**Status**: ⏳ In Progress  
**Prerequisite**: EXP-01 complete (SPS confirmed)

**Question**: What is the end-to-end latency across the full pipeline (capture → DSP → render)? Does ② process→display exceed 30 ms with 11 tabs? Is 36k/43k BPH feasible?

Timestamp injection points:

| Point | Location | Segment |
|-------|----------|---------|
| TS1 | Entry of `audioDataAvailable()` | ① start |
| TS2 | T1/T3 event timestamp finalized | ① end / ② start |
| TS3 | Qt `paintEvent()` complete | ② end |

### Runs

Core comparison only — full per-run numbers and analysis are in the collapsible
detail blocks below. `E2E = ① wait + ② exec` (avg / max, ms). Deadline = chunk
period (`BG_SPF / BG_SPS`): Windows ≈ 10 ms, RPi ≈ 21 ms.

**R2 (RPi) is the baseline for all future experiments.** R1 (Windows) is kept
only as a dev-machine reference.

| Run | Date | Platform | Rate | Tabs | E2E avg/max (ms) | Dropped | Missed | Role | Detail |
|:---:|------|----------|:----:|:----:|:----------------:|:-------:|:------:|------|:------:|
| R1 | 2026-06-12 | Windows | 48 kHz | 1 | 2.8 / 363.9 | — | — | dev reference | ▼ R1 below |
| R2 | 2026-06-11 | **RPi** | 48 kHz | ? | 255.4 / 900.9 | — | — | **baseline** | ▼ R2 below |

> R2 (RPi) was recorded before platform auto-metadata existed (no `#` meta line);
> platform is confirmed by the presence of `_sys.csv`. Tabs unknown (`?`). Its
> deadline ≈ 21 ms (SPF 1024 / SPS 48008) differs from Windows (480 / 48000)
> because the ALSA chunk size differs.

> `Dropped` (audio blocks) and `Missed` (beat detections) are required by the
> Low-Latency QA but not yet instrumented — shown as `—`. See backlog % in the
> detail block as the current proxy for "falling behind".

### Run details

<details>
<summary><b>R1</b> — 2026-06-12 · Windows · 48 kHz · 1-tab · dev reference — E2E avg 2.8 / max 363.9 ms</summary>

**Context**: 1-tab, 48 kHz, logging build with auto-recorded platform metadata.
Deadline = 10.00 ms (480 / 48000, computed from data). CSV meta line:
`platform=windows kernel=winnt sample_rate=48000`.
Files: [csv](../../src/logs/EXP-02/log_20260612_132536.csv) ·
[plot](../../src/logs/EXP-02/log_20260612_132536.png).

**Per-frame metrics (analyze_log.py, window=100, 2104 frames), ms:**

| Metric | avg | max | min |
|--------|----:|----:|----:|
| total = ①+② | 2.81 | 363.87 | 0.07 |
| ① wait (BG→FG queue + sched) | 1.89 | 359.53 | 0.01 |
| ② exec (process→display) | 0.92 | 4.34 | 0.03 |
| ┄ copy | 0.003 | 0.067 | 0.001 |
| ┄ sound | 0.000 | 0.011 | 0.000 |
| ┄ tg | 0.117 | 3.303 | 0.009 |
| ┄ ui | 0.014 | 2.074 | 0.000 |
| ┄ plot (dominant) | 0.784 | 2.356 | 0.012 |

**Throughput / health:** bg_fps avg 93.7 (max 100.6), fg_fps avg 85.6 (max 100.0),
bg_sps avg 44990. samples avg 527 (≈ SPF 480). exec > deadline: **0 / 2104**.
backlog (>1.5× SPF): **91 / 2104 (4.3 %)**.

![R2 log analysis](../../src/logs/EXP-02/log_20260612_132536.png)

**Observations:**

| Phase | Pattern | Interpretation |
|-------|---------|----------------|
| Startup (0–250) | samples↑, wait↑ to ~15 ms | warmup; FG draining initial backlog |
| Steady (250+) | wait ≈ 0, total ≈ exec ≈ 1 ms | FG keeps up; latency dominated by exec, not wait |
| Single spike | one frame wait ≈ 360 ms | isolated OS preemption, not structural |

**Conclusion:**

- Much healthier than R1: avg total **2.8 ms** (R1 11.5), wait avg **1.9 ms**
  (R1 10.1), backlog **4.3 %** (R1 22.4 %). The machine kept up frame-by-frame.
- ② (`exec`) never exceeded the 10 ms deadline (0/2104) — processing is not the
  constraint; `plot` remains the dominant exec component (~0.78 ms).
- Worst-case is still a single ~360 ms `wait` spike (OS scheduling), so max E2E
  is jitter-bound, not load-bound.
- Validates the toolchain end-to-end on Windows: platform auto-metadata,
  data-driven deadline (10 ms), and the analysis graphs.
- **11-tab and RPi runs still required** for the definitive EXP-02 answer.

</details>

<details>
<summary><b>R2</b> — 2026-06-11 · RPi · 48 kHz · pre-metadata build — E2E avg 255.4 / max 900.9 ms · <b>baseline · real-time FAIL</b></summary>

**Context**: RPi run, before platform auto-metadata. Deadline ≈ **21.33 ms**
(SPF 1024 / SPS 48008). Files:
[csv](../../src/logs/EXP-02/log_20260611_145543.csv) ·
[plot](../../src/logs/EXP-02/log_20260611_145543.png) ·
[sys plot](../../src/logs/EXP-02/log_20260611_145543_sys.png).

**Per-frame metrics (window=100, 1015 frames), ms:**

| Metric | avg | max | min |
|--------|----:|----:|----:|
| total = ①+② | 255.45 | 900.92 | 0.14 |
| ① wait | 235.20 | 879.61 | 0.03 |
| ② exec | 20.24 | 62.47 | 0.10 |
| ┄ copy | 0.014 | 0.105 | 0.007 |
| ┄ sound | 0.456 | 9.531 | 0.000 |
| ┄ tg | 3.111 | 27.925 | 0.062 |
| ┄ ui | 0.679 | 6.359 | 0.000 |
| ┄ plot (dominant) | 15.979 | 29.651 | 0.025 |

**Throughput / health:** bg_fps avg 43.5, fg_fps avg 31.2, samples avg 1427.
**exec > deadline: 441 / 1015 (43 %)**. backlog (>1.5× SPF): 312 / 1015.

**System (RPi):** cpu_total 24 % but **cpu2 pinned at 91 % (max 99 %)**; temp
**84.7 °C**, **throttled on all 10 samples**; mem 1651 MB; freq 2400 MHz.

![R2 system](../../src/logs/EXP-02/log_20260611_145543_sys.png)

**Conclusion:** RPi **fails real-time performance**, not just latency. `exec`
(plot ~16 ms) overruns the 21 ms deadline 43 % of the time; one core (cpu2)
saturated (~92 %) while the others idle; SoC thermally throttled (85 °C)
throughout. The bottleneck is structural: heavy `plot`, single-core audio path,
thermal throttling. **This run is the baseline for all future RPi experiments.**

</details>

### Current Best

**Baseline — R2 (RPi, target platform): real-time FAIL** ⚠
- E2E: mean **255 ms** / worst **901 ms** at 28,800 BPH, 48 kHz
- ② exec mean **20 ms** overruns the **21 ms** deadline **43 %** of frames
  (0/2104 on Windows by contrast) — processing itself cannot keep up
- `plot` alone ≈ **16 ms** (≈20× the Windows dev machine)
- one core saturated (cpu2 ~92 %) while the others idle; SoC **thermally
  throttled** (85 °C) for the entire run; mem ~1.65 GB
- Root causes (structural): heavy `plot`, single-core audio path, thermal throttle
- Improvement directions for QAS-2: **lazy / throttled rendering**, off-load
  `plot` from the audio path, multi-core distribution, cooling

> Windows dev reference (R1): E2E avg 2.8 ms, exec never exceeds the 10 ms
> deadline — confirms the bottleneck is the Pi, not the algorithm.

- **Dominant cost on target (RPi)**: `exec` (`plot` ~16 ms) + resulting backlog
- **Lazy Rendering required**: **Yes** (RPi exec overruns the deadline)
- **Architecture Decision**: → see [Architecture Decisions Log](#architecture-decisions-log)

---

## EXP-03: Detector Parameter Optimization Under Noise Conditions

**Linked QA**: QAS-3 | **Linked Risk**: TR-05  
**Status**: 📅 Planned  
**Prerequisite**: EXP-01 complete (SPS for measurement confirmed)  
**Expected start**: After EXP-01 concludes

**Question**: Which combination of `onset_fraction` and `min_peak_fraction` minimizes Δ Rate / Δ Amplitude / Δ Beat Error across low / medium / high noise?

### Planned Approach

| Parameter | Default | Planned Search Range | Step |
|-----------|:-------:|:--------------------:|:----:|
| `onset_fraction` | 0.03 | 0.01 ~ 0.10 | 0.01 |
| `min_peak_fraction` | 0.20 | 0.10 ~ 0.40 | 0.05 |

| Planned Noise Condition | Environment | Expected Noise Floor |
|------------------------|-------------|:-------------------:|
| Low | Quiet closed lab | ~30 dB SPL |
| Medium | Typical office | ~50 dB SPL |
| High | Added noise source | ~65 dB SPL |

> R1: Full grid search with default params as baseline.  
> R2: Narrow range around R1 best result.  
> R3: Validate optimal params under abrupt noise transition (low → high).

### Run History

> Fill in when experiment begins.

| Run | Date | Change from Previous | `onset_fraction` tested | `min_peak_fraction` tested | Best Δ Sum (Low+Med+High) | Converging? | Next Action |
|:---:|------|----------------------|:-----------------------:|:--------------------------:|:-------------------------:|:-----------:|-------------|
| R1 | — | Planned: full grid search | — | — | — | — | |
| R2 | — | Planned: narrow around R1 best | — | — | — | — | |
| R3 | — | Planned: abrupt noise transition validation | — | — | — | — | |

### Current Best

- **Run**: —
- **Recommended `onset_fraction`**: —
- **Recommended `min_peak_fraction`**: —
- **Adaptive threshold valid under abrupt noise transition**: —
- **Architecture Decision**: → see [Architecture Decisions Log](#architecture-decisions-log)

---

## EXP-04: Signal Quality Warning Threshold Search

**Linked QA**: QAS-4 | **Linked Risk**: TR-09  
**Status**: 📅 Planned  
**Prerequisite**: Observer pattern refactoring complete + `⚠ No signal` / `⚠ Noisy signal` warning UI implemented  
**Expected start**: After warning UI is implemented

**Questions**:
- After removing the watch, within how many seconds should `⚠ No signal` appear?
- After restoring the watch, within how many seconds should the warning clear?
- What noise/signal ratio threshold triggers `⚠ Noisy signal` without false alarms?

### Planned Approach

| Part | What to sweep | Metric |
|------|--------------|--------|
| A — No Signal | Heartbeat N parameter: 1 / 2 / 3 / 5 s | Warning appear time, warning clear time M |
| B — Noisy Signal | 3–5 noise/signal ratio threshold candidates | False-alarm rate, miss rate |

> R1: Sweep all N values (Part A) + all threshold candidates (Part B) under 3 noise conditions.  
> R2: Narrow to 2 best N candidates; refine threshold.  
> R3: Validate chosen N·M + threshold under abrupt noise condition changes.

### Run History

> Fill in when experiment begins.

| Run | Date | Change from Previous | Best N (s) | Best M (s) | Noisy threshold | False-Alarm Rate | Better? | Next Action |
|:---:|------|----------------------|:----------:|:----------:|:---------------:|:----------------:|:-------:|-------------|
| R1 | — | Planned: full sweep | — | — | — | — | — | |
| R2 | — | Planned: narrow candidates | — | — | — | — | — | |
| R3 | — | Planned: abrupt condition validation | — | — | — | — | — | |

### Current Best

- **Run**: —
- **Finalized N (⚠ No signal delay)**: — s
- **Finalized M (warning clear delay)**: — s
- **Finalized noisy signal threshold**: —
- **Architecture Decision**: → see [Architecture Decisions Log](#architecture-decisions-log)

---

## EXP-05: BPH Escalation Verification — 36k/43k BPH Latency Measurement

**Linked QA**: QAS-2 Stretch  
**Status**: ⏸ Deferred  
**Prerequisite**: EXP-02 complete + QAS-1~4 all confirmed at 28,800 BPH

> Not started. Will begin only after all 28,800 BPH QA targets are confirmed.

### Run History

> Fill in when prerequisite is met.

| Run | Date | Change from Previous | 36k E2E Mean (ms) | 43k E2E Mean (ms) | < 80% beat period? | Better? | Next Action |
|:---:|------|----------------------|:-----------------:|:-----------------:|:------------------:|:-------:|-------------|
| R1 | — | Planned: baseline | — | — | — | — | |

### Current Best

- **Run**: —
- **QAS-2 Stretch target**: Pass / Fail
- **Team 2nd goal (BPH range expansion)**: Declared / Abandoned

---

## Remaining Experiments

| ID | Title | Reason Not Complete | Plan |
|----|-------|---------------------|------|
| | | | |

---

## Architecture Decisions Log

> Consolidated record of all architecture decisions derived from experiments.  
> Update each row as experiments conclude. Reference this section in Architecture Views.

| Decision | Source Experiment | QA Impacted | Decision Made | Date |
|----------|:-----------------:|:-----------:|---------------|------|
| QAS-1 Response Measure: confirmed max sps | EXP-01 | QAS-1 | 192k sps achieves 0 Dropped Block (30 s buffer). Recommended operating point: **96k sps**. | 2026-06-15 |
| Graceful degradation fallback threshold | EXP-01 | QAS-1 | Not required — 0 drops at all tested sps with 30 s buffer. | 2026-06-15 |
| SCHED_RR applied to audio capture thread | EXP-01 | QAS-1 | **No** — SCHED_RR/FIFO show no improvement in Dropped Block count; exec time marginally worse. | 2026-06-15 |
| QAS-2 Response Measure: confirmed E2E latency target | EXP-02 | QAS-2 | Partial — 1-tab avg 11.5 ms; ② avg 1.5 ms (< 30 ms). 11-tab pending. | 2026-06-11 |
| Lazy Rendering tactic: required or not | EXP-02 | QAS-2 | Inconclusive — 11-tab test required | 2026-06-11 |
| `Detector.cpp` default params updated | EXP-03 | QAS-3 | — | — |
| QAS-3 QA-C2 acceptable Δ thresholds | EXP-03 | QAS-3 | — | — |
| Heartbeat N parameter hardened as constant | EXP-04 | QAS-4 | — | — |
| Noisy signal threshold hardened as constant | EXP-04 | QAS-4 | — | — |
| QAS-2 Stretch target: pass or abandon | EXP-05 | QAS-2 | — | — |

---

## Review Checklist

- [ ] All planned experiments have results or documented reason for incompletion
- [ ] Each result clearly resolves (or fails to resolve) the original question
- [ ] Architecture Decisions Log updated for all completed experiments
- [ ] Remaining experiments listed if any
- [ ] Results are relevant to overall system goals
