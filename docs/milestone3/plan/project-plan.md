# Updated Project Plan — M3

**Milestone**: M3 | **Due**: 2026-07-01 | **Updated**: 2026-06-25

---

## 1. Plan Changes Since M2

| Change | Reason | Impact |
|--------|--------|--------|
| EXP-01 (WeiShi accuracy) completed 2026-06-25 | Sequential measurement round conducted on available hardware | QAS-5 verified; no architectural changes required |
| EXP-06 (Long-Term Aging) resolved analytically | Time budget did not allow full live aging run; analytical verification from `mBucketSize` policy is sufficient for pass | Accelerated simulation screenshot target by 2026-06-30 |
| Signal Quality Warning (EXP-06 from M2) shipped in `feature/noise` | Threshold 55 dB validated; popup fires correctly at SNR ≤ 0 dB | QAS-4 sub-requirement closed |
| 14 graph tabs implemented | All HIGH/MEDIUM/LOWER priority graphs delivered | QAS-3 extensibility demonstrated at scale |
| LongTermTab added (M3-new) | QAS-6 requirement added during M3 | ADR-007 added; EXP-06 added |
| ATAM evaluation completed | Architecture evaluated against QAS-1 through QAS-6 | v3 document in `final/references/atam-evaluation-v3.md` |

---

## 2. Risk Resolution Status

| Risk ID | Description | Resolution | Status |
|---------|-------------|------------|:------:|
| TR-01 | RPi real-time performance at target sps | EXP-02: 96k sps confirmed, Dropped = 0 | ✅ Resolved |
| TR-02 | End-to-end latency exceeds 30 ms | EXP-03: T2 offload thread reduces E2E to 2.2 ms | ✅ Resolved |
| TR-03 | Cross-compilation environment stability | Cross-compile toolchain stable; RPi deployment scripted | ✅ Resolved |
| TR-04 | Qt rendering FPS with multiple tabs | R1 Lazy Rendering + T2 offload; 14 tabs verified | ✅ Resolved |
| TR-05 | Beat detector fails under ambient noise | EXP-05: onset=0.08 stable through 60 dB SNR | ✅ Resolved |
| TR-06 | Accuracy vs. WeiShi reference device | EXP-01: Δ Rate 0.4 s/d, Δ Amp 15°, Δ BE 0.1 ms — within tolerance | ✅ Resolved |
| TR-07 | LongTermTab memory growth unbounded | EXP-06: mBucketSize caps points at 840/series at 7 days | ✅ Resolved |

---

## 3. Remaining Open Items

| ID | Item | Priority | Owner | Due |
|----|------|:--------:|-------|-----|
| O-01 | Accelerated simulation screenshot for LongTermTab (grading evidence) | Medium | — | 2026-06-30 |
| O-02 | Final demo rehearsal on RPi | High | All | 2026-06-30 |
| O-03 | Presentation slides finalized | High | All | 2026-06-30 |

---

## 4. Final Task Status

### Core Pipeline

| Task | Status |
|------|:------:|
| Signal acquisition (Live mode, RPi 5, 96 kHz) | ✅ Done |
| FilterChain (LP/HP cutoffs, configurable) | ✅ Done |
| T1/T3 beat detection (onset=0.08, min_peak=0.10) | ✅ Done |
| Rate / Amplitude / Beat Error / BPH calculation | ✅ Done |
| T2 DSP offload thread (ADR-001) | ✅ Done |
| R1 Lazy Rendering (ADR-002) | ✅ Done |
| IAudioSource dependency inversion (ADR-005) | ✅ Done |
| WatchMath module isolation (ADR-008) | ✅ Done |

### Graph Tabs (14 total, via BaseGraphTab observer, ADR-006)

| Priority | Graph | Status |
|:--------:|-------|:------:|
| HIGH | Trace Display (rate + amplitude over time) | ✅ Done |
| HIGH | Vario Display (Min/Max/Avg/σ) | ✅ Done |
| HIGH | Beat Error Display & Diagnostic Trace | ✅ Done |
| MEDIUM | Multi-Position Sequence Display | ✅ Done |
| MEDIUM | Beat-Noise Scope 1 (strip view) | ✅ Done |
| MEDIUM | Beat-Noise Scope 2 (tic/tac dual axis) | ✅ Done |
| MEDIUM | Long-Term Performance Graph (LongTermTab) | ✅ Done |
| MEDIUM | Escapement Analyzer & Marker-Line Display | ✅ Done |
| MEDIUM | Scope Mode with Synchronized Sweep | ✅ Done |
| MEDIUM | Scope Function (F0/F1/F2/F3 filter views) | ✅ Done |
| LOWER | Time-Frequency Spectrogram Display | ✅ Done |
| LOWER | Waveform Comparison Display | ✅ Done |
| LOWER | Watch Position display in GUI | ✅ Done |
| LOWER | AI Feature: signal quality classification | ✅ Done |

### Demo Preparation

| Task | Status |
|------|:------:|
| Demo script | ✅ Draft complete — [demo-script.md](../demo-script.md) |
| Presentation outline | ✅ In progress — [presentation-outline.md](../presentation-outline.md) |
| LongTermTab accelerated screenshot | ⏳ Pending (due 2026-06-30) |
| Final demo rehearsal on RPi | ⏳ Pending (2026-06-30) |

---

## 5. Remaining Schedule

```
2026-06-25 (Wed) — EXP-01 complete; all experiments done
2026-06-26 (Thu) — Documentation finalization
2026-06-27 (Fri) — ATAM presentation slides
2026-06-28 (Sat) — Slack / buffer
2026-06-29 (Sun) — Buffer
2026-06-30 (Mon) — Demo rehearsal; LongTermTab screenshot
2026-07-01 (Tue) — M3 FINAL DEMO
```

---

## 6. Review Checklist

- [x] All 7 risks resolved
- [x] All 6 experiments concluded
- [x] 14 graph tabs implemented and tested
- [x] Architecture evaluation (ATAM) complete
- [x] ATAM utility tree, before/after, and tradeoff table documented
- [ ] Accelerated LongTermTab screenshot captured
- [ ] Demo rehearsal on RPi
- [ ] Presentation slides final
