# Construction Plan — M3

**Milestone**: M3 | **Due**: 2026-07-01 | **Updated**: 2026-06-25

> Implementation task status for the M2 → M3 construction phase.

---

## 1. Schedule

```
06/22 (Mon) — M2 submission
06/23 (Tue) — M2 mentor review feedback; core pipeline finalization
06/24 (Wed) — Graph implementations begin
06/25 (Thu) — EXP-01 (WeiShi) executed; all experiments complete
06/26 (Fri) — Documentation: architecture views, ATAM, ADRs
06/27 (Sat) — Documentation: risks, QA scenarios
06/28 (Sun) — Buffer / review pass
06/29 (Mon) — RPi integration; performance tuning
06/30 (Tue) — Demo rehearsal; LongTermTab screenshot
07/01 (Wed) — M3 FINAL DEMO
```

---

## 2. Phase A: Core Pipeline

| ID | Task | Status | Notes |
|----|------|:------:|-------|
| A-01 | Signal acquisition working in Live mode on RPi | ✅ Done | IAudioSource + AlsaAudioSource unified via ADR-005 |
| A-02 | LP/HP FilterChain applied (cutoffs from EXP-05) | ✅ Done | ADR-009; user-configurable via UI |
| A-03 | T1/T3 beat detection working (onset=0.08 from EXP-05) | ✅ Done | Detector.cpp defaults updated |
| A-04 | Rate / Amplitude / Beat Error / BPH validated vs WeiShi | ✅ Done | EXP-01 result: Δ Rate 0.4 s/d — within ±2 s/d |
| A-05 | T2 DSP offload thread (ADR-001) | ✅ Done | E2E 80 ms → 2.2 ms |
| A-06 | R1 Lazy Rendering (ADR-002) | ✅ Done | max tail latency 11.1 → 5.7 ms |
| A-07 | WatchMath module isolation (ADR-008) | ✅ Done | Unit-testable formula layer |

---

## 3. Phase B: HIGH Priority Graphs

| ID | Graph | Status | Notes |
|----|-------|:------:|-------|
| B-01 | Trace Display (rate + amplitude over time, smoothed) | ✅ Done | |
| B-02 | Vario Display (Min/Max/Avg/σ for rate + amplitude) | ✅ Done | |
| B-03 | Beat Error Display & Diagnostic Trace | ✅ Done | |
| B-04 | Pause + rewind capability in all tabs | ✅ Done | |

---

## 4. Phase C: MEDIUM Priority Graphs

| ID | Graph | Status | Notes |
|----|-------|:------:|-------|
| C-01 | Multi-Position Sequence Display (up to 10 positions) | ✅ Done | |
| C-02 | Beat-Noise Scope 1 (strip view, 20/200/400 ms ranges) | ✅ Done | |
| C-03 | Beat-Noise Scope 2 (tic/tac dual axis, averaging) | ✅ Done | |
| C-04 | Long-Term Performance Graph (LongTermTab) | ✅ Done | ADR-007; EXP-06 verified analytically |
| C-05 | Escapement Analyzer & Marker-Line Display | ✅ Done | |
| C-06 | Scope Mode with Synchronized Sweep | ✅ Done | |
| C-07 | Scope Function (F0/F1/F2/F3 filter views) | ✅ Done | |

---

## 5. Phase D: LOWER Priority / Optional

| ID | Task | Status | Notes |
|----|------|:------:|-------|
| D-01 | Time-Frequency Spectrogram Display | ✅ Done | |
| D-02 | Waveform Comparison Display with Timing Markers | ✅ Done | |
| D-03 | Watch Position display in GUI (CH/CB/9H/6H/3H/12H) | ✅ Done | |
| D-04 | AI Feature: signal quality classification | ✅ Done | feature/ai-local branch |
| D-05 | Signal Quality Warning popup (55 dB threshold) | ✅ Done | feature/noise branch; EXP-06 from M2 |

---

## 6. Phase E: Integration & Demo Preparation

| ID | Task | Status | Due |
|----|------|:------:|-----|
| E-01 | Full RPi performance test at 96k sps (EXP-02) | ✅ Done | 2026-06-15 |
| E-02 | Latency numbers documented (avg + worst-case) | ✅ Done | 2026-06-16 |
| E-03 | Accuracy validation vs WeiShi No.1000 (EXP-01) | ✅ Done | 2026-06-25 |
| E-04 | ATAM evaluation documented | ✅ Done | 2026-06-22 |
| E-05 | Architecture views complete (6 views) | ✅ Done | 2026-06-25 |
| E-06 | LongTermTab accelerated simulation screenshot | ⏳ Pending | 2026-06-30 |
| E-07 | Demo script finalized | ⏳ In Progress | 2026-06-30 |
| E-08 | Presentation slides finalized | ⏳ In Progress | 2026-06-30 |
| E-09 | Final demo rehearsal on RPi | ⏳ Pending | 2026-06-30 |

---

## 7. Quality Gates

| Gate | Criteria | Status |
|------|----------|:------:|
| Real-time | Dropped Block = 0 at 96k sps on RPi | ✅ Pass |
| Latency | E2E avg < 30 ms with 11+ tabs | ✅ Pass (2.2 ms) |
| Extensibility | New tab ≤ 3 files, 0 cross-layer deps | ✅ Pass |
| Correctness | onset=0.08 stable at 60 dB SNR | ✅ Pass |
| Accuracy | Δ Rate < ±2 s/d vs WeiShi | ✅ Pass (0.4 s/d) |
| Long-term | ≤ 3,000 plotted points at 7 days | ✅ Pass (2,520) |
| Demo readiness | All tabs on RPi; demo rehearsal done | ⏳ Pending |
