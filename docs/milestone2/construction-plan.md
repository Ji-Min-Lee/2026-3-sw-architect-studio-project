# Construction Plan

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [ ] Draft / [ ] Final

> Detailed implementation tasks for M2 → M3 (2026-06-22 ~ 2026-07-01).

---

## 1. Remaining Schedule

```
06/22 (Mon) — M2 submission
06/23 (Tue) — M2 mentor review feedback
06/24 (Wed) — Address M2 feedback, finalize core pipeline
06/25 (Thu) — Graph implementations begin
06/26 (Fri) — Graph implementations continue
06/29 (Mon) — RPi integration + performance tuning
06/30 (Tue) — Demo rehearsal
07/01 (Wed) — M3 FINAL DEMO
```

---

## 2. Task Breakdown

### Phase A: Core Pipeline (must complete before graphs)

| ID | Task | Owner | Status | Est. Hours | Done By |
|----|------|-------|--------|------------|---------|
| A-01 | Signal acquisition working in Live mode on RPi | | | | 06/23 |
| A-02 | LP/HP filter chain applied (cutoffs from EX-03) | | | | 06/23 |
| A-03 | T1/T3 beat detection working (onset/peak from EX-02) | | | | 06/23 |
| A-04 | Rate / Amplitude / Beat Error / BPH calculation validated vs WeiShi | | | | 06/24 |
| A-05 | Latency measurement instrumentation (3 timestamp points) | | | | 06/24 |

### Phase B: HIGH Priority Graphs

| ID | Graph | Owner | Status | Est. Hours | Done By |
|----|-------|-------|--------|------------|---------|
| B-01 | Trace Display (rate + amplitude over time, smoothed) | | | | 06/26 |
| B-02 | Vario Display (Min/Max/Avg/σ for rate + amplitude) | | | | 06/26 |
| B-03 | Beat Error Display & Diagnostic Trace | | | | 06/27 |
| B-04 | Pause + rewind capability in all tabs | | | | 06/27 |

### Phase C: MEDIUM Priority Graphs

| ID | Graph | Owner | Status | Est. Hours | Done By |
|----|-------|-------|--------|------------|---------|
| C-01 | Multi-Position Sequence Display (up to 10 positions) | | | | 06/28 |
| C-02 | Beat-Noise Scope 1 (strip view, 20/200/400ms ranges) | | | | 06/28 |
| C-03 | Beat-Noise Scope 2 (tic/tac dual axis, averaging) | | | | 06/28 |
| C-04 | Long-Term Performance Graph (rate/amplitude/BE over time) | | | | 06/29 |
| C-05 | Escapement Analyzer & Marker-Line Display | | | | 06/29 |
| C-06 | Scope Mode with Synchronized Sweep | | | | 06/29 |
| C-07 | Scope Function (F0/F1/F2/F3 filter views) | | | | 06/29 |

### Phase D: LOWER Priority / Optional

| ID | Task | Owner | Status | Done By |
|----|------|-------|--------|---------|
| D-01 | Time-Frequency Spectrogram Display | | | 06/30 |
| D-02 | Waveform Comparison Display with Timing Markers | | | 06/30 |
| D-03 | Watch Position display in GUI (CH/CB/9H/6H/3H/12H) | | | 06/29 |
| D-04 | AI Feature: signal quality classification | | | If time |

### Phase E: Integration & Demo Prep

| ID | Task | Owner | Status | Done By |
|----|------|-------|--------|---------|
| E-01 | Full RPi performance test at target sample rate | | | 06/29 |
| E-02 | Latency numbers documented (avg + worst-case) | | | 06/29 |
| E-03 | Accuracy validation vs WeiShi 1000 | | | 06/29 |
| E-04 | Demo script preparation | | | 06/30 |
| E-05 | Presentation slides draft | | | 06/30 |

---

## 3. Quality Gates (must pass before M3 demo)

| Gate | Criteria | Status |
|------|----------|--------|
| Core pipeline | Rate/Amplitude/Beat Error match WeiShi within tolerance | [ ] |
| Real-time | No dropped audio blocks at target sample rate | [ ] |
| Latency | End-to-end latency measured and reported | [ ] |
| RPi demo | All HIGH-priority graphs running on RPi 5 | [ ] |
| Extensibility | Adding new graph tab requires ≤ N file changes (no Domain changes) | [ ] |

---

## 4. Review Checklist

- [ ] All tasks have owners and target dates
- [ ] Critical path identified (Phase A must complete first)
- [ ] Quality gates defined with measurable criteria
- [ ] Schedule is realistic for remaining time (06/22 → 07/01)
