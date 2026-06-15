# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-11

---

## Summary

| ID | Experiment | Runs | Latest Key Result | Status |
|----|------------|:----:|-------------------|:------:|
| EXP-01 | RPi Real-Time Performance — Dropped Block Measurement | 0 | — | ⏳ In Progress |
| EXP-02 | End-to-End Latency — 3-Segment Timestamp Measurement | 3 | R2 (rpi1): **real-time FAIL** — exec 20 ms / 43 % overruns, throttled 85 °C. R3 (rpi2): exec 15 ms / 4.4 % overruns, no throttle 60 °C — significant improvement, still marginal FAIL. R1 (Windows) ref: 2.8 ms | ⏳ In Progress |
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
**Status**: ⏳ In Progress

**Question**: Can RPi 5 achieve Dropped Block = 0 at 96,000 sps while running Qt GUI + DSP concurrently? If not, what is the maximum sps that can be processed stably?

### Run History

> Add a new row after each run. `Change` describes what was different from the previous run.

| Run | Date | Change from Previous | 48k Dropped/min | 96k Dropped/min | 192k Dropped/min | SCHED_RR applied? | Better? | Next Action |
|:---:|------|----------------------|:---------------:|:---------------:|:----------------:|:-----------------:|:-------:|-------------|
| R1 | | Baseline | — | — | — | No | — | |
| R2 | | | | | | | ↑/↓/= | |
| R3 | | | | | | | ↑/↓/= | |

### Current Best

> Update this block after each run that improves the result.

- **Run**: —
- **Recommended sample rate**: — sps
- **Graceful degradation fallback needed**: —
- **SCHED_RR effect**: —
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
| R2 | 2026-06-11 | **rpi1** | 48 kHz | ? | 255.4 / 900.9 | — | — | **rpi1 baseline** | ▼ R2 below |
| R3 | 2026-06-15 | **rpi2** | 48 kHz | ? | 57.2 / 208.9 | — | — | rpi2 baseline | ▼ R3 below |

> R2 (rpi1, the 1st unit) was recorded before platform auto-metadata existed
> (no `#` meta line); platform is confirmed by the presence of `_sys.csv`. Tabs
> unknown (`?`). Its deadline ≈ 21 ms (SPF 1024 / SPS 48008) differs from Windows
> (480 / 48000) because the ALSA chunk size differs. Future runs auto-record the
> unit as `device=rpi1`/`rpi2` in the CSV meta line.
>
> R3 (rpi2, the 2nd unit) is the first run with auto-recorded platform metadata
> (`device=rpi2` in the CSV `#` meta line). Same deadline (21.33 ms). Tabs unknown
> (`?`). No thermal throttling observed — a key hardware difference from rpi1.

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
<summary><b>R2</b> — 2026-06-11 · rpi1 (1st unit) · 48 kHz · pre-metadata build — E2E avg 255.4 / max 900.9 ms · <b>baseline · real-time FAIL</b></summary>

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

<details>
<summary><b>R3</b> — 2026-06-15 · rpi2 (2nd unit) · 48 kHz · auto-metadata build — E2E avg 57.2 / max 208.9 ms · <b>marginal real-time FAIL (4.4 % overruns)</b></summary>

**Context**: RPi 2nd unit (rpi2), first run with platform auto-metadata
(`platform=debian kernel=linux host=lg1 device=rpi2 sample_rate=48000`).
Deadline ≈ **21.33 ms** (SPF 1024 / SPS 48000). Files:
[csv](../../src/logs/EXP-02/log_20260615_152751.csv) ·
[plot](../../src/logs/EXP-02/log_20260615_152751.png) ·
[sys plot](../../src/logs/EXP-02/log_20260615_152751_sys.png).

**Per-frame metrics (1288 frames), ms:**

| Metric | avg | max | min |
|--------|----:|----:|----:|
| total = ①+② | 57.23 | 208.92 | 0.16 |
| ① wait | 42.07 | 186.93 | 0.03 |
| ② exec | 15.17 | 26.04 | 0.12 |
| ┄ copy | 0.011 | 0.041 | 0.006 |
| ┄ sound | 0.367 | 8.131 | 0.000 |
| ┄ tg | 1.973 | 7.535 | 0.063 |
| ┄ ui | 0.490 | 6.933 | 0.000 |
| ┄ plot (dominant) | 12.322 | 19.109 | 0.025 |

**Throughput / health:** bg_fps avg 46.9 (max 47.4), fg_fps avg 38.5 (max 42.1),
bg_sps avg 48041. samples avg 1245 (≈ SPF 1024). exec > deadline: **57 / 1288 (4.4 %)**.
backlog (>1.5× SPF): **273 / 1288 (21.2 %)**.

**System (rpi2):** cpu_total ~28 % avg but **cpu3 pinned at ~99 %**; temp avg
**60.3 °C** (max 61.1 °C), **throttled 0 / 12 samples** (no throttling); mem
~1120 MB used / 16 GB total; freq 2400 MHz.

![R3 sys](../../src/logs/EXP-02/log_20260615_152751_sys.png)

**Observations:**

| Phase | Pattern | Interpretation |
|-------|---------|----------------|
| Startup (0–100) | samples↑, exec↑ to ~20 ms | warmup / initial backlog drain |
| Steady (100+) | exec avg ~15 ms, occasional spikes to ~26 ms | load stable; `plot` ~12 ms is the dominant cost |
| Deadline overruns | 4.4 % of frames | structural but sporadic — not every frame fails |

**Conclusion:**

- Dramatically better than R2 (rpi1): E2E avg **57 ms** vs 255 ms (−78 %), exec avg
  **15.2 ms** vs 20.2 ms (−25 %), exec overruns **4.4 %** vs 43 % (−90 %).
- Root cause of improvement: **no thermal throttling** (60 °C vs 85 °C on rpi1).
  rpi2 runs consistently at 2400 MHz while rpi1 was throttled for the entire run.
- Still structurally failing: exec avg 15 ms with `plot` dominating at 12 ms. Max
  exec 26 ms exceeds the 21.33 ms deadline, and 4.4 % overruns remain.
- `plot` remains the dominant bottleneck on both units — lazy / throttled rendering
  is still required regardless of hardware unit.
- **16 GB RAM** (vs rpi1's smaller capacity) — memory pressure not a factor on rpi2.

</details>

### R2 vs R3 Comparison (rpi1 vs rpi2)

| Metric | R2 · rpi1 | R3 · rpi2 | Change |
|--------|----------:|----------:|:------:|
| E2E avg / max (ms) | 255.4 / 900.9 | 57.2 / 208.9 | −78 % / −77 % |
| ① wait avg (ms) | 235.2 | 42.1 | −82 % |
| ② exec avg / max (ms) | 20.2 / 62.5 | 15.2 / 26.0 | −25 % / −58 % |
| plot avg (ms) | 16.0 | 12.3 | −23 % |
| exec > deadline (21.3 ms) | 441/1015 **(43 %)** | 57/1288 **(4.4 %)** | −90 % |
| backlog (>1.5× SPF) | 312/1015 | 273/1288 | fewer absolute |
| bg_fps avg | 43.5 | 46.9 | +8 % |
| fg_fps avg | 31.2 | 38.5 | +23 % |
| CPU pinned core | cpu2 ~91 % | cpu3 ~99 % | same pattern |
| SoC temp avg | **84.7 °C** | **60.3 °C** | −29 °C |
| Thermal throttle | **10/10 samples** | **0/12 samples** | eliminated |
| RAM (used / total) | ~1651 MB / ? | ~1120 MB / 16 GB | more headroom |

**Key finding:** The performance gap between rpi1 and rpi2 is primarily explained by
thermal throttling. rpi2 sustains 2400 MHz throughout while rpi1 was throttled
from the start. The `plot` bottleneck (~12–16 ms) is structural and present on both
units — lazy rendering remains the required fix regardless of which unit is used.

---

### Current Best

**R3 (rpi2, 2nd unit): marginal real-time FAIL — 4.4 % overruns, no throttling** ⚠
- E2E: mean **57 ms** / worst **209 ms** at 28,800 BPH, 48 kHz
- ② exec mean **15.2 ms** overruns the **21.3 ms** deadline **4.4 %** of frames
  (max 26 ms) — structurally failing but far less than rpi1 (43 %)
- `plot` alone ≈ **12.3 ms** — dominant bottleneck on both units
- one core saturated (cpu3 ~99 %) while others idle; SoC **stable at 60 °C, no
  throttling** throughout; mem ~1.12 GB used / 16 GB total
- Root causes (structural): heavy `plot`, single-core audio path
- Improvement directions for QAS-2: **lazy / throttled rendering**, off-load
  `plot` from the audio path, multi-core distribution

> rpi1 baseline (R2) for comparison: E2E avg 255 ms, exec 43 % overruns,
> throttled at 85 °C — thermal throttling was a major compounding factor on rpi1.

> Windows dev reference (R1): E2E avg 2.8 ms, exec never exceeds the 10 ms
> deadline — confirms the bottleneck is the Pi, not the algorithm.

- **Dominant cost on target (RPi)**: `exec` (`plot` ~12 ms) + resulting backlog
- **Lazy Rendering required**: **Yes** (exec still overruns deadline even on rpi2)
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
| QAS-1 Response Measure: confirmed max sps | EXP-01 | QAS-1 | — | — |
| Graceful degradation fallback threshold | EXP-01 | QAS-1 | — | — |
| SCHED_RR applied to audio capture thread | EXP-01 | QAS-1 | Yes / No | — |
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
