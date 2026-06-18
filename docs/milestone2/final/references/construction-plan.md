# Construction Plan

**Team**: Blue Sky (Team 3) | **Milestone**: M2 → M3 | **Date**: 2026-06-22  
**Scope**: 2026-06-22 (M2 submission) → 2026-07-01 (M3 final demo)

---

## 1. Remaining Schedule

```
06/22 (Mon) — M2 submission + mentor review prep
06/23 (Tue) — M2 mentor feedback; EXP-01 / EXP-02 R5 on RPi
06/24 (Wed) — Address M2 feedback; ADR-003 finalized; Phase A core pipeline
06/25 (Thu) — EXP-03 filter sweep; Phase A A-02/A-03
06/26 (Fri) — EXP-05 rendering FPS on RPi; Phase B graphs
06/29 (Mon) — RPi integration + WeiShi accuracy validation
06/30 (Tue) — Demo rehearsal + latency documentation
07/01 (Wed) — M3 FINAL DEMO
```

---

## 2. Code State at M2 (2026-06-22)

| Item | File / Location | Status |
|------|-----------------|:------:|
| 4-layer structure (audio / engine / tabs / ui / external) | `src/` | ✅ |
| 13 graph tabs (11 required + RateScopeTab + SoundPrintTab) | `src/tabs/` | ✅ |
| `BaseGraphTab` — Pause contract + R1 `isVisible()` interface | `src/tabs/BaseGraphTab.h` | ✅ |
| T2 `DSPWorker` (dedicated DSP thread) | `src/audio/DSPWorker.cpp` | ✅ macOS + RPi (E2-5/6) |
| R1 Lazy Rendering (`isVisible()` guard + `showEvent()` catch-up) | `BaseGraphTab::showEvent()` | ✅ macOS + RPi (E2-6) |
| Pause button + `⚠ No signal` warning | `src/ui/MainWindow.cpp:159,209` | ✅ |
| Watch Position selector | `MainWindow` + `SequenceTab` | ✅ |
| Logger infrastructure (CSV per-frame logging) | `src/logging/` | ✅ |
| Unit tests: 116 tests, 7 binaries, all pass | `src/tests/` | ✅ |
| AGC disabled on RPi boot (`alsamixer`) | RPi boot checklist | ⚠️ Confirm each session |
| `⚠ Noisy signal` warning | — | ❌ Deferred → M3 |
| RPi Live mode end-to-end validation | — | ⏳ Phase A |
| WeiShi accuracy comparison | — | ⏳ Phase A A-04 |

---

## 3. Task Breakdown

### Phase A: Core Pipeline *(must complete before graphs)*

| ID | Task | Owner | Status | Done By |
|----|------|-------|:------:|---------|
| A-01 | Signal acquisition working in Live mode on RPi | | ⏳ | 06/23 |
| A-02 | LP/HP filter chain applied (cutoffs from [EXP-03](experiments/exp-03-filter-sweep.md)) | | ⏳ | 06/25 |
| A-03 | T1/T3 beat detection validated on RPi Live | | ⏳ | 06/25 |
| A-04 | Rate / Amplitude / Beat Error / BPH validated vs WeiShi | | ⏳ | 06/29 |
| A-05 | E2E latency instrumented (3 timestamp points) and reported | | ⏳ | 06/29 |

**Critical constraint**: A-02 cutoff values depend on EXP-03 result (06/25). A-01 must be unblocked first by confirming AGC disabled.

### Phase B: HIGH Priority Graphs

| ID | Graph | Owner | Status | Done By |
|----|-------|-------|:------:|---------|
| B-01 | Trace Display (rate + amplitude over time, smoothed) | | ⏳ | 06/26 |
| B-02 | Vario Display (Min/Max/Avg/σ for rate + amplitude) | | ⏳ | 06/26 |
| B-03 | Beat Error Display & Diagnostic Trace | | ⏳ | 06/27 |
| B-04 | Pause + rewind capability in all tabs | | ⏳ | 06/27 |

### Phase C: MEDIUM Priority Graphs

| ID | Graph | Owner | Status | Done By |
|----|-------|-------|:------:|---------|
| C-01 | Multi-Position Sequence Display (up to 10 positions) | | ⏳ | 06/28 |
| C-02 | Beat-Noise Scope 1 (strip view, 20/200/400ms ranges) | | ⏳ | 06/28 |
| C-03 | Beat-Noise Scope 2 (tic/tac dual axis, averaging) | | ⏳ | 06/28 |
| C-04 | Long-Term Performance Graph (rate/amplitude/BE over time) | | ⏳ | 06/29 |
| C-05 | Escapement Analyzer & Marker-Line Display | | ⏳ | 06/29 |
| C-06 | Scope Mode with Synchronized Sweep | | ⏳ | 06/29 |
| C-07 | Scope Function (F0/F1/F2/F3 filter views) | | ⏳ | 06/29 |

### Phase D: LOWER Priority / Optional

| ID | Task | Owner | Status | Done By |
|----|------|-------|:------:|---------|
| D-01 | Time-Frequency Spectrogram Display | | ⏳ | 06/30 |
| D-02 | Waveform Comparison Display with Timing Markers | | ⏳ | 06/30 |
| D-03 | Watch Position display in GUI (CH/CB/9H/6H/3H/12H) | | ⏳ | 06/29 |
| D-04 | AI Feature: signal quality classification | | ⏳ | If time |

### Phase E: Integration & Demo Prep

| ID | Task | Owner | Status | Done By |
|----|------|-------|:------:|---------|
| E-01 | Full RPi performance test at 96kHz (T2+R1 confirmed) | | ⏳ | 06/29 |
| E-02 | E2E latency documented — avg + worst-case (3 segments) | | ⏳ | 06/29 |
| E-03 | Accuracy validation vs WeiShi No.1000 (Rate/Amp/BE) | | ⏳ | 06/29 |
| E-04 | Demo script prepared | | ⏳ | 06/30 |
| E-05 | M3 presentation slides draft | | ⏳ | 06/30 |

---

## 4. Open Experiments Blocking Phase A

| Experiment | Blocks | Target |
|-----------|--------|:------:|
| [EXP-03](experiments/exp-03-filter-sweep.md) LP/HP filter sweep | A-02 cutoff constants | 06/25 |
| [EXP-05](experiments/exp-05-rendering-fps.md) 11-tab FPS on RPi | [ADR-002](adr/ADR-002-r1-lazy-rendering.md) / [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) decision | 06/26 |
| EXP-02 E2-8 (FG SCHED_RR — fg_wait mitigation) | A-05 latency completeness | 06/24 |

---

## 5. Quality Gates *(must pass before M3 demo)*

| Gate | Criteria | Status |
|------|----------|:------:|
| Core pipeline | Rate/Amplitude/Beat Error match WeiShi within tolerance | ⏳ |
| Real-time | Dropped blocks = 0 at 96kHz on RPi (**confirmed** by [EXP-01](experiments/exp-01-sample-rate.md)) | ✅ |
| DSP latency | E2E DSP avg < 10ms on RPi (**confirmed** E2-6: 2.05ms) | ✅ |
| FG latency | fg_wait avg < 21ms on RPi (currently 60.1ms 🔴 — EXP-02 E2-8) | ⏳ |
| Rendering | 0% deadline miss under 11-tab load on RPi ([EXP-05](experiments/exp-05-rendering-fps.md)) | ⏳ |
| RPi demo | All HIGH-priority graphs running on RPi 5 | ⏳ |
| Extensibility | New graph tab requires ≤ 3 file changes, 0 Domain changes | ✅ |

---

## 6. Explicitly Deferred to M3

Items below are out of M2 scope. They are listed here to make scope decisions transparent.

| Item | Reason for Deferral |
|------|---------------------|
| Timeline scrub (Pause + forward/backward rewind) | Pause freeze implemented; full scrub exceeds M2 scope |
| `⚠ Noisy signal` warning | Requires SNR computation not yet in engine; deferred pending EXP-03/04 |
| Interactive timing point selection in GUI | M2 scope exceeded |
| Sound Print improvements (averaging window, noise reduction) | M2 scope exceeded |
| Raw signal overlay | M2 scope exceeded |
| [EXP-04](experiments/exp-03-filter-sweep.md) signal quality warning threshold | Requires `⚠ Noisy signal` UI prerequisite |
| BPH range expansion beyond 28,800 BPH | Confirmed operating point first; EXP-05 stretch goal |

---

## 7. Critical Path

```
AGC confirmed OFF
  └─► A-01 Live mode on RPi
        └─► EXP-03 (06/25) → A-02 filter cutoffs → A-03 beat detection
              └─► A-04 WeiShi validation → A-05 latency report
                    └─► Phase B/C graphs → Phase E demo prep → M3 Demo (07/01)

EXP-02 E2-8 (fg_wait) ──────────────────────────────────► A-05 latency completeness
EXP-05 (06/26) ─────────────────────────────────────────► ADR-002 / ADR-004 final decision
```
