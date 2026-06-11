# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [ ] Draft  [ ] Final  
**Last Updated**: —

---

## Summary

| ID | Experiment | Runs | Latest Key Result | Status |
|----|------------|:----:|-------------------|:------:|
| EXP-01 | RPi Real-Time Performance — Dropped Block Measurement | 0 | — | ⏳ In Progress |
| EXP-02 | End-to-End Latency — 3-Segment Timestamp Measurement | 0 | — | ⏳ In Progress |
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

| Run | Date | Change from Previous | E2E Mean 1-tab (ms) | E2E Mean 11-tab (ms) | ② >30ms? | Better? | Next Action |
|:---:|------|----------------------|:-------------------:|:--------------------:|:--------:|:-------:|-------------|
| R1 | | Baseline | — | — | — | — | |
| R2 | | | | | | ↑/↓/= | |
| R3 | | | | | | ↑/↓/= | |

### Current Best

- **Run**: —
- **E2E latency at 28,800 BPH**: mean — ms / worst — ms (11-tab)
- **Lazy Rendering required**: —
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
| QAS-2 Response Measure: confirmed E2E latency target | EXP-02 | QAS-2 | — | — |
| Lazy Rendering tactic: required or not | EXP-02 | QAS-2 | — | — |
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
