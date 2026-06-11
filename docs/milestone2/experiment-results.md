# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-11

---

## Summary

| ID | Experiment | Runs | Latest Key Result | Status |
|----|------------|:----:|-------------------|:------:|
| EXP-01 | RPi Real-Time Performance — Dropped Block Measurement | 0 | — | ⏳ In Progress |
| EXP-02 | End-to-End Latency — 3-Segment Timestamp Measurement | 1 | E2E avg 11.5 ms (1-tab); ② avg 1.5 ms — ② >30 ms: sporadic (1/1102 frames) | ⏳ In Progress |
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

### Run History

| Run | Date | Log | Plot | Change from Previous | E2E Mean 1-tab (ms) | E2E Mean 11-tab (ms) | ② >30ms? | Better? | Next Action |
|:---:|------|-----|------|----------------------|:-------------------:|:--------------------:|:--------:|:-------:|-------------|
| R1 | 2026-06-11 | [log_20260611_131240](../../src/logs/EXP-02/log_20260611_131240.csv) | [log_20260611_131240](../../src/logs/EXP-02/log_20260611_131240.png) | Baseline — 1-tab, 48 kHz, logging build | 11.5 | — (not yet) | sporadic (1 / 1102 frames, isolated ui_ms spike) | — | Run with 11 tabs; run on RPi if this is macOS baseline |
| R2 | | | | | | | | ↑/↓/= | |
| R3 | | | | | | | | ↑/↓/= | |

#### R1 — Statistics (analyze_log.py, window=100)

```
Analyzing: log_20260611_131240.csv  (overview window=100)
Frames: 1102

=== Latency per frame (ms) ===
  total_ms   avg=  11.542  max= 266.724  min=   0.198 ms
  wait_ms    avg=  10.060  max= 265.189  min=   0.016 ms   ← ① BG→FG queue + scheduling
  exec_ms    avg=   1.482  max=  74.409  min=   0.093 ms   ← ② process→display

=== exec breakdown per frame (ms) ===
  copy_ms    avg=   0.005  max=   0.210  min=   0.001 ms
  sound_ms   avg=   0.000  max=   0.006  min=   0.000 ms
  tg_ms      avg=   0.184  max=   2.544  min=   0.009 ms
  ui_ms      avg=   0.180  max=  73.790  min=   0.001 ms   ← source of 74 ms exec spike
  plot_ms    avg=   1.112  max=   3.562  min=   0.020 ms   ← dominant exec component

=== Throughput ===
  bg_fps     avg=  86.799  max= 100.300                    ← BG capture ~48 kHz
  fg_fps     avg=  54.252  max=  82.500                    ← FG processing rate
  bg_sps     avg= 41822.895  max= 49341.300

  exec > 10ms frames  : 1 / 1102   (0.09 %)
  backlog (>1.5x SPF) : 247 / 1102 (22.4 %)               ← ⚠ backlog accumulates in startup/spike phases
```

**R1 Analysis Plot**

![R1 log analysis](../../src/logs/EXP-02/log_20260611_131240.png)

**Key observations / 주요 관찰:**

| Phase | Frames | Pattern | Interpretation |
|-------|:------:|---------|----------------|
| Startup / warmup | 1 – 144 | samples=480, total<2 ms | BG not yet stable; FG draining small backlog |
| BG stabilizes | 145 – 220 | bg_fps=100.3, fg_fps=72.3 | Steady 48 kHz capture; FG keeping up |
| fg_fps drop | 221 – 326 | fg_fps drops to 37.8 | FG falls behind BG → backlog grows; samples bursts up to 7200 |
| Recovery | 327+ | fg_fps=52.9, total ~2 ms | FG recovers; latency returns to normal |
| Exec spike | frame 95 | exec_ms=74 ms (ui_ms=73.8 ms) | Isolated Qt repaint jitter — not structural |
| wait_ms spike | frame ~450+ | wait_ms up to 265 ms | OS scheduling jitter; FG thread preempted |

**Conclusion for EXP-02 R1 / R1 결론:**

- ② (`exec_ms`) is structurally fast: avg **1.5 ms**, well under the 30 ms target.
- The 74 ms exec spike is a single-frame UI paint anomaly, not a steady-state issue.
- ① (`wait_ms`) is the dominant latency source (87 % of avg total) and drives worst-case E2E to 266 ms — this is OS scheduling jitter, not DSP load.
- 22 % backlog rate during the fg_fps-drop phase indicates the FG thread cannot sustain realtime throughput when competing with other loads. SCHED_RR or thread priority tuning may be needed (→ see EXP-01).
- **11-tab measurement still required** to answer the EXP-02 question definitively.

### Current Best

- **Run**: R1 (2026-06-11, 1-tab baseline)
- **E2E latency at 28,800 BPH (1-tab)**: mean **11.5 ms** / worst **266.7 ms** (wait_ms spike)
- **② process→display (exec_ms)**: mean **1.5 ms** / worst **74.4 ms** (1 isolated frame)
- **② >30 ms structurally**: No — 1/1102 frames, isolated Qt paint spike
- **Dominant latency source**: `wait_ms` (OS scheduling / queue) — 87 % of avg E2E
- **Lazy Rendering required**: Inconclusive — 11-tab test pending
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
