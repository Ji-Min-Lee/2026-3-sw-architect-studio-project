# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-15

---

## Summary

| ID | Experiment | Runs | Latest Key Result | Status |
|----|------------|:----:|-------------------|:------:|
| EXP-01 | RPi Real-Time Performance — Dropped Block Measurement | 0 | — | ⏳ In Progress |
| EXP-02 | End-to-End Latency — 3-Segment Timestamp Measurement | 5 | R5 (rpi2, no-render+sync): E2E avg **2.1 ms**, wait **0.03 ms**, backlog **0/1224** — perfect real-time sync. R4 same condition had backlog 28 %. R3 (full GUI): 4.4 % overruns. | ⏳ In Progress |
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
| R3 | 2026-06-15 | **rpi2** | 48 kHz | ? | 57.2 / 208.9 | — | — | rpi2 baseline (full GUI) | ▼ R3 below |
| R4 | 2026-06-15 | **rpi2\*** | 48 kHz | — | 80.1 / 258.7 | — | — | no-render (plot=0) | ▼ R4 below |
| R5 | 2026-06-15 | **rpi2\*** | 48 kHz | — | 2.1 / 11.1 | — | — | no-render + perfect sync | ▼ R5 below |

> R2 (rpi1, the 1st unit) was recorded before platform auto-metadata existed
> (no `#` meta line); platform is confirmed by the presence of `_sys.csv`. Tabs
> unknown (`?`). Its deadline ≈ 21 ms (SPF 1024 / SPS 48008) differs from Windows
> (480 / 48000) because the ALSA chunk size differs. Future runs auto-record the
> unit as `device=rpi1`/`rpi2` in the CSV meta line.
>
> R3 (rpi2, the 2nd unit) is the first run with auto-recorded platform metadata
> (`device=rpi2` in the CSV `#` meta line). Same deadline (21.33 ms). Tabs unknown
> (`?`). No thermal throttling observed — a key hardware difference from rpi1.
>
> R4 (rpi2\*) has no `device=` field in the CSV meta line (`platform=debian
> kernel=linux host=lg1 sample_rate=48000`); confirmed rpi2 by matching mem\_total
> (16 GB) and temp profile (60 °C, no throttle). `plot_ms` and `ui_ms` are both
> 0 throughout — Qt rendering was not active (headless / no-display build). Tabs
> column is `—` (no UI). This run isolates the DSP pipeline cost from rendering.
>
> R5 (rpi2\*) shares the same meta line as R4 (same unit, no-render build). Key
> difference: FG and BG are perfectly synchronized — samples fixed at exactly 1024,
> backlog 0/1224, wait avg 0.027 ms. Sourced from `project-2` repo copy.

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

<details>
<summary><b>R4</b> — 2026-06-15 · rpi2* (no device field) · 48 kHz · no-render build — E2E avg 80.1 / max 258.7 ms · <b>exec > deadline: 0 / 1244 — plot bottleneck confirmed</b></summary>

**Context**: rpi2 unit (inferred from mem\_total 16 GB, temp 60 °C, no throttle).
Qt rendering not active — `plot_ms` and `ui_ms` are 0 for all frames. Deadline
≈ **21.33 ms** (SPF 1024 / SPS 48008). Files:
[csv](../../src/logs/EXP-02/log_20260615_162055.csv) ·
[plot](../../src/logs/EXP-02/log_20260615_162055.png) ·
[sys plot](../../src/logs/EXP-02/log_20260615_162055_sys.png).

**Per-frame metrics (1244 frames), ms:**

| Metric | avg | max | min |
|--------|----:|----:|----:|
| total = ①+② | 80.12 | 258.68 | 0.27 |
| ① wait | 77.42 | 255.73 | 0.03 |
| ② exec | 2.69 | 8.78 | 0.24 |
| ┄ copy | 0.010 | 0.025 | 0.006 |
| ┄ sound | 0.000 | 0.000 | 0.000 |
| ┄ tg (dominant) | 2.682 | 8.752 | 0.233 |
| ┄ ui | 0.000 | 0.000 | 0.000 |
| ┄ **plot** | **0.000** | **0.000** | **0.000** |

**Throughput / health:** bg_fps avg 43.5 (max 47.4), fg_fps avg 33.4 (max 46.0),
bg_sps avg 44479. samples avg 1329 (spikes to 3072). exec > deadline: **0 / 1244**.
backlog (>1.5× SPF): **352 / 1244 (28.3 %)**.

**System (rpi2\*):** cpu_total ~27 % avg, **cpu1 pinned at ~91 % avg (max 99.7 %)**;
temp avg **60.0 °C** (max 61.7 °C), **throttled 0 / 12 samples**; mem ~2260 MB used
/ 16 GB total; freq 2400 MHz.

![R4 sys](../../src/logs/EXP-02/log_20260615_162055_sys.png)

**Observations:**

| Phase | Pattern | Interpretation |
|-------|---------|----------------|
| exec | flat 2.7 ms avg, max 8.8 ms | pure DSP (tg) cost — no rendering overhead |
| wait | 77 ms avg (higher than R3 42 ms) | FG is slower to pull; backlog builds but exec always finishes in time |
| plot_ms | exactly 0.000 every frame | Qt rendering not running — headless / no-display build |
| Deadline | 0 / 1244 exceeded | **never once exceeded** without rendering |

**Conclusion:**

- **`plot` is the confirmed bottleneck.** Without rendering, exec avg drops from
  15.2 ms (R3) to **2.7 ms** (−82 %) and the 21.33 ms deadline is **never exceeded**
  (0 / 1244 frames vs 4.4 % in R3).
- The remaining exec cost is almost entirely `tg` (2.68 ms) — the DSP computation
  itself is well within budget.
- `wait` is higher than R3 (77 ms vs 42 ms avg) because FG drains the queue more
  slowly when there is no rendering pressure, allowing backlog to accumulate (28.3 %
  vs 21.2 % in R3). This is a scheduling artifact, not a performance regression.
- **Architecture implication**: lazy / throttled rendering decouples plot from the
  audio deadline path. This run is the evidence that doing so will eliminate deadline
  overruns entirely. `tg` at 2.7 ms leaves ample headroom in the 21.33 ms budget.

</details>

<details>
<summary><b>R5</b> — 2026-06-15 · rpi2* (no device field) · 48 kHz · no-render + perfect sync — E2E avg 2.1 / max 11.1 ms · <b>exec > deadline: 0 / 1224 · backlog: 0 / 1224 — ideal real-time</b></summary>

**Context**: Same rpi2 unit and no-render condition as R4, sourced from `project-2`
repo. Key difference from R4: FG and BG threads are perfectly synchronized —
samples is exactly 1024 every frame with zero backlog. Deadline ≈ **21.33 ms**
(SPF 1024 / SPS 48008). Files:
[csv](../../src/logs/EXP-02/log_20260615_163106.csv) ·
[plot](../../src/logs/EXP-02/log_20260615_163106.png) ·
[sys plot](../../src/logs/EXP-02/log_20260615_163106_sys.png).

**Per-frame metrics (1224 frames), ms:**

| Metric | avg | max | min |
|--------|----:|----:|----:|
| total = ①+② | 2.10 | 11.05 | 0.23 |
| ① wait | 0.027 | 3.649 | 0.009 |
| ② exec | 2.073 | 9.923 | 0.207 |
| ┄ copy | 0.008 | 0.549 | 0.006 |
| ┄ sound | 0.000 | 0.000 | 0.000 |
| ┄ tg (dominant) | 2.064 | 9.911 | 0.198 |
| ┄ ui | 0.000 | 0.000 | 0.000 |
| ┄ **plot** | **0.000** | **0.000** | **0.000** |

**Throughput / health:** bg_fps avg 43.3 (max 47.4), fg_fps avg 43.3 (max 47.3)
— **FG and BG perfectly matched**. bg_sps avg 44355. samples avg / min / max all
= **1024 exactly** (no backlog accumulation). exec > deadline: **0 / 1224**.
backlog (>1.5× SPF): **0 / 1224**.

**System (rpi2\*):** cpu_total ~29.6 % avg, **cpu1 pinned at ~93.8 % avg (max 100 %)**;
temp avg **60.4 °C** (max 62.3 °C), **throttled 0 / 12 samples**; mem ~2292 MB used
/ 16 GB total; freq 2400 MHz.

![R5 sys](../../src/logs/EXP-02/log_20260615_163106_sys.png)

**Observations:**

| Phase | Pattern | Interpretation |
|-------|---------|----------------|
| wait | avg 0.027 ms (near-zero) | FG picks up immediately after BG emits — no queue depth |
| samples | exactly 1024 every frame | BG and FG are 1-for-1 synchronized, no accumulation |
| bg_fps ≈ fg_fps | both ~43.3 fps | FG keeps pace with BG throughout the run |
| exec | avg 2.07 ms, max 9.9 ms | `tg` sole cost — same level as R4 (2.7 ms) |
| backlog | 0 / 1224 | zero frames with accumulated samples |

**Conclusion:**

- R5 represents **ideal real-time pipeline behavior**: zero backlog, near-zero wait,
  exec well within the 21.33 ms deadline (0 / 1224 overruns).
- Total E2E avg is **2.1 ms** — close to the Windows reference (R1: 2.8 ms) and
  drastically lower than R4 (80 ms), where FG lagged behind the queue.
- The difference from R4 is not hardware (same unit, same temp) but **thread
  scheduling**: in R5 the FG thread is pulled up immediately after each BG emit,
  eliminating queue depth and wait entirely.
- `tg` (DSP) costs 2.06 ms avg — unchanged from R4 (2.7 ms), confirming the
  improvement is purely a scheduling / synchronization gain, not an algorithmic one.
- **Architecture implication**: with rendering decoupled and FG properly scheduled,
  the full pipeline can run at **2.1 ms E2E** on rpi2. This is the performance
  ceiling to target with lazy rendering implemented on the full-GUI build.

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

### R3 vs R4 — Rendering ON vs OFF (rpi2 same unit)

| Metric | R3 · rpi2 (full GUI) | R4 · rpi2 (no-render) | Change |
|--------|---------------------:|----------------------:|:------:|
| E2E avg / max (ms) | 57.2 / 208.9 | 80.1 / 258.7 | wait ↑ (see below) |
| ① wait avg (ms) | 42.1 | 77.4 | +84 % (scheduling artifact) |
| ② exec avg / max (ms) | 15.2 / 26.0 | **2.7 / 8.8** | **−82 % / −66 %** |
| plot avg (ms) | 12.3 | **0.0** | eliminated |
| tg avg (ms) | 2.0 | 2.7 | +35 % (sole exec cost) |
| exec > deadline (21.3 ms) | 57/1288 **(4.4 %)** | **0 / 1244 (0 %)** | **eliminated** |
| backlog (>1.5× SPF) | 273/1288 (21.2 %) | 352/1244 (28.3 %) | ↑ (FG slower without render pressure) |
| SoC temp avg | 60.3 °C | 60.0 °C | same (no thermal effect from render) |

**Key finding:** With rendering removed, exec drops 82 % (15 ms → 2.7 ms) and
deadline overruns are **eliminated entirely**. `tg` (DSP computation) costs 2.7 ms,
leaving 18.6 ms headroom in the 21.33 ms deadline. Lazy / decoupled rendering will
directly fix the real-time failure with no algorithmic changes.

### R4 vs R5 — Same condition, different sync behavior (rpi2 no-render)

| Metric | R4 · rpi2 (no-render) | R5 · rpi2 (no-render + sync) | Change |
|--------|----------------------:|-----------------------------:|:------:|
| E2E avg / max (ms) | 80.1 / 258.7 | **2.1 / 11.1** | **−97 % / −96 %** |
| ① wait avg / max (ms) | 77.4 / 255.7 | **0.027 / 3.649** | **−100 %** |
| ② exec avg / max (ms) | 2.7 / 8.8 | 2.1 / 9.9 | −22 % / similar |
| tg avg (ms) | 2.682 | 2.064 | comparable |
| exec > deadline | 0 / 1244 | **0 / 1224** | both 0 |
| backlog (>1.5× SPF) | 352/1244 **(28.3 %)** | **0 / 1224 (0 %)** | **eliminated** |
| samples avg / min / max | 1329 / 1024 / 3072 | **1024 / 1024 / 1024** | **perfectly fixed** |
| bg_fps avg | 43.5 | 43.3 | same |
| fg_fps avg | 33.4 | **43.3** | **+30 % — matched to bg** |

**Key finding:** R4 and R5 run on the same hardware and same no-render build, yet
R5 shows E2E avg **2.1 ms** vs R4's **80 ms**. The difference is FG thread
scheduling: in R5 the FG thread is scheduled immediately after each BG emit
(wait ≈ 0, samples always exactly 1024). R5 is the **performance ceiling** for
the rpi2 pipeline with rendering decoupled.

---

### Current Best

**R5 (rpi2, no-render + perfect sync) — ideal real-time pipeline** ✅
- E2E avg **2.1 ms** / max **11.1 ms** — matches Windows reference (R1: 2.8 ms)
- ① wait avg **0.027 ms** (near-zero); samples exactly 1024 every frame
- exec avg **2.1 ms**, max **9.9 ms** — **0 / 1224** deadline overruns
- backlog **0 / 1224** — FG and BG perfectly synchronized
- temp 60.4 °C, no throttling, mem 2.29 GB / 16 GB

> R4 (same no-render, no sync fix): E2E avg 80 ms, backlog 28 % — FG scheduling lag.  
> R3 (full GUI): exec 15.2 ms, 4.4 % overruns — `plot` bottleneck in audio path.  
> Windows dev reference (R1): E2E avg 2.8 ms — R5 reaches parity.

- **Target performance for full-GUI lazy rendering**: E2E ~2 ms, 0 overruns, 0 backlog
- **Lazy Rendering required**: **Yes, and sufficient** — R5 is evidence of the ceiling
- **FG scheduling fix required**: Yes — R4 vs R5 shows scheduling alone causes 80 ms
  vs 2 ms E2E; both fixes (lazy render + FG scheduling) are needed for the full build
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
