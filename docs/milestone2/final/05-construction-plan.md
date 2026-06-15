# Construction Plan

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Remaining Schedule

```
06/22 (Mon) — M2 submission
06/23 (Tue) — M2 mentor feedback + RPi R5 (T2+R1 on RPi)
06/24 (Wed) — Address M2 feedback + RPi R6 (T1 SCHED_RR) + core pipeline finalize
06/25 (Thu) — Filter param sweep (EXP-03) + HIGH-priority graphs begin
06/26 (Fri) — HIGH-priority graphs continue + EXP-05 (11-tab FPS on RPi)
06/29 (Mon) — MEDIUM-priority graphs + RPi integration + performance tuning
06/30 (Tue) — Demo rehearsal + E2E latency numbers documented
07/01 (Wed) — M3 FINAL DEMO
```

---

## Task Breakdown

### Phase A: Core Pipeline — Must complete before graphs (target: 06/24)

| ID | Task | Done By |
|----|------|---------|
| A-01 | Signal acquisition working in Live mode on RPi | 06/23 |
| A-02 | LP/HP filter chain applied (cutoffs from EXP-03) | 06/24 |
| A-03 | T1/T3 beat detection working (onset/peak from EXP-02) | 06/23 |
| A-04 | Rate / Amplitude / Beat Error / BPH validated vs WeiShi 1000 | 06/24 |
| A-05 | Latency measurement instrumentation (3-point timestamp) | 06/24 |

### Phase B: HIGH Priority Graphs (target: 06/27)

| ID | Graph | Done By |
|----|-------|---------|
| B-01 | Trace Display — rate + amplitude over time (smoothed) | 06/26 |
| B-02 | Vario Display — Min/Max/Avg/σ for rate + amplitude | 06/26 |
| B-03 | Beat Error Display + Diagnostic Trace | 06/27 |
| B-04 | Pause + rewind in all tabs | 06/27 |

### Phase C: MEDIUM Priority Graphs (target: 06/29)

| ID | Graph | Done By |
|----|-------|---------|
| C-01 | Multi-Position Sequence Display (up to 10 positions) | 06/28 |
| C-02 | Beat-Noise Scope 1 (strip view, 20/200/400ms ranges) | 06/28 |
| C-03 | Beat-Noise Scope 2 (tic/tac dual axis, averaging) | 06/28 |
| C-04 | Long-Term Performance Graph (rate/amplitude/BE over time) | 06/29 |
| C-05 | Escapement Analyzer + Marker-Line Display | 06/29 |
| C-06 | Scope Mode with Synchronized Sweep | 06/29 |
| C-07 | Scope Function (F0/F1/F2/F3 filter views) | 06/29 |

### Phase D: Optional (time permitting)

| ID | Task | Done By |
|----|------|---------|
| D-01 | Time-Frequency Spectrogram Display | 06/30 |
| D-02 | Waveform Comparison Display | 06/30 |
| D-03 | Watch position display in GUI (CH/CB/9H/6H/3H/12H) | 06/29 |

### Phase E: Integration & Demo Prep (target: 07/01)

| ID | Task | Done By |
|----|------|---------|
| E-01 | Full RPi performance test at target sample rate | 06/29 |
| E-02 | E2E latency numbers documented (avg + worst-case) | 06/29 |
| E-03 | Accuracy validation vs WeiShi 1000 | 06/29 |
| E-04 | Demo script preparation | 06/30 |
| E-05 | M3 presentation slides | 06/30 |

---

## Completion Priority

```
Must:     Phase A (core pipeline) + Phase B (HIGH graphs) + E-01, E-02, E-03
Should:   Phase C (MEDIUM graphs)
Could:    Phase D (Optional)
```

The demo survives on Phase A + B. Everything above that strengthens it.

---

## Quality Gates — Must Pass Before M3 Demo

| Gate | Criteria | Status |
|------|----------|:------:|
| Core pipeline | Rate/Amplitude/Beat Error match WeiShi within tolerance | [ ] |
| Real-time | No dropped audio blocks at 96kHz on RPi | [ ] |
| Latency | E2E latency measured and documented (avg + worst-case) | [ ] |
| RPi demo | All HIGH-priority graphs running on RPi 5 | [ ] |
| Extensibility | New graph tab requires ≤ 3 file changes, 0 Domain changes | [ ] |
