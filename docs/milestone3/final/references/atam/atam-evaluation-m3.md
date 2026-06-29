# Architecture Evaluation — ATAM (M3)

**Project**: TimeGrapher  
**Date**: 2026-06-28  
**Team**: Blue Sky (Team 3)  
**Method**: ATAM (Architecture Tradeoff Analysis Method) — SEI [Kazman00]

> **Snapshot document** — This report captures the ATAM evaluation conducted on **2026-06-28**.
> It will not be updated after publication. For the current architecture state, refer to the
> [Architecture Views](../views/README.md) and [ADRs](../adr/).
>
> **Relation to M2 ATAM** — The [M2 ATAM](atam-evaluation-m2.md) identified three open risks
> (R-1 Witschi accuracy, R-2 ring buffer depth, R-3 ADR-004 activation). This document closes
> those risks and evaluates the three new architectural decisions introduced in M3
> (ADR-007, ADR-008, ADR-009) together with the new QA scenario QAS-6.

---

## Executive Summary

The M2 evaluation ended with one critical risk open: accuracy against the Witschi reference
device (QAS-5) had never been validated on real hardware. That risk is now closed — EXP-06
confirmed agreement within tolerance across two independent rounds (Δ Rate 0.2–0.4 s/d,
Δ Amplitude 15–25°, Δ Beat Error 0–0.1 ms).

M3 introduced three architectural decisions that extend the validated design:

| ADR | Decision | QA |
|-----|----------|----|
| ADR-007 | Time-based bucket downsampling in `LongTermTab` | QAS-6 |
| ADR-008 | `WatchMath` pure-function module isolation | QAS-4 (Correctness) |
| ADR-009 | FilterChain design for detector parameters | QAS-4 (Correctness) |

All M2 open risks are resolved. All M3 risks are resolved. The architecture enters the
final demo with **zero open risk items** as of 2026-06-25.

The one structural sensitivity that remains is the Qt main-thread GUI scheduling lag
(avg 60 ms from signal emission to Qt event-loop pickup). This does not affect QAS-1
or QAS-2 targets because DSP and latency are measured inside the DSP thread, but it means
the display refresh is subject to OS scheduler variance. It is a known, bounded, accepted
non-risk.

---

## What is ATAM?

ATAM is a method for finding risks in an architecture before they become real problems [Kazman00].

It does **not** measure the system precisely.
It finds: which decisions could cause trouble, and where decisions force a tradeoff between
two goals.

**How this document is organized:**

| Section | Contents |
|---------|----------|
| Section 1 | Business goals, constraints, architectural drivers |
| Section 2 | The architecture as of M3 (delta from M2) |
| Section 3 | Utility Tree — QA priorities |
| Section 4 | Architectural approaches considered (M3 additions) |
| Section 5 | Sensitivity Points, Tradeoffs, Risks, Non-Risks |
| Section 6 | Risk Themes — updated from M2 |
| Section 7 | What's left (M3 close-out) |

---

## 1. Business Drivers

| Item | Detail |
|------|--------|
| **Core goal** | Measure mechanical watch Rate, Amplitude, and Beat Error accurately |
| **Target hardware** | Raspberry Pi 5 (4-core ARM, 8 GB RAM) |
| **Framework** | Qt (C++) — fixed choice |
| **Key constraint** | 21 ms audio deadline per block (ALSA, ~96 kHz) |
| **M3 addition** | Multi-day aging tests (up to 7 days) without GUI freeze or data loss |
| **Stakeholder concern** | Results must match Witschi No.1000 reference device — **now confirmed** |

### Critical Requirement & Architectural Drivers

| Type | QA | What it shaped in the architecture |
|------|----|------------------------------------|
| **Governing goal** | Measurement Accuracy (QAS-5) | User-facing outcome — Rate / Amplitude / Beat Error matching Witschi No.1000. **Verified by EXP-06** (M3). |
| **Enabling QA** | Real-Time Performance (QAS-1) | Dropped audio blocks cause missed beats → wrong Rate and Beat Error. Addressed by ADR-001 + ADR-003. **All risks resolved.** |
| **Enabling QA** | Low Latency (QAS-2) | Stale display misleads user. Addressed by ADR-001 (E2E avg 2.2 ms, EXP-02). **All risks resolved.** |
| **Enabling QA** | Correctness (QAS-4) | Formula errors and noise-triggered false beats corrupt output. Extended in M3 by ADR-008 (WatchMath isolation), ADR-009 (FilterChain), EXP-05 (noise popup). |
| **Independent driver** | Extensibility / Modifiability (QAS-3) | "Add a tab in ≤ 3 files" goal. 14 tabs verified. Extended in M3 by EXP-08. |
| **New M3 QA** | Long-Term Session Performance (QAS-6) | Multi-day aging tests must not freeze the GUI. Addressed by ADR-007 (time-based downsampling). Verified analytically by EXP-07. |

---

## 2. Architecture — M3 State

### What changed from M2

The threading model and rendering strategy are unchanged from M2. M3 added three decisions
that extend correctness guarantees and add the long-term data management layer.

```
M2 baseline (unchanged)
  ├── ADR-001  DSP Offload Thread (ring buffer, BG/FG split)
  ├── ADR-002  Lazy Rendering (isVisible() guard in each tab)
  ├── ADR-003  96 kHz sample rate
  ├── ADR-004  Timer-Decoupled Rendering (conditional, standby)
  ├── ADR-005  IAudioSource dependency inversion
  └── ADR-006  BaseGraphTab Observer Pattern (Qt signals → 14 tabs)

M3 additions
  ├── ADR-007  LongTermTab time-based bucket downsampling
  ├── ADR-008  WatchMath pure-function module isolation
  └── ADR-009  FilterChain detector parameter design
```

### M2 open risks — resolution status

| M2 Risk | Description | Resolution in M3 |
|---------|-------------|-----------------|
| R-1 | Witschi accuracy not validated | ✅ EXP-06 (2026-06-25): Δ Rate 0.2–0.4 s/d, Δ Amplitude 15–25°, Δ Beat Error 0–0.1 ms — all within tolerance |
| R-2 | Ring buffer depth not stress-tested under peak load | ✅ EXP-01 RPi (9 runs, all sps × all scheduling policies): Dropped = 0 — buffer depth confirmed sufficient |
| R-3 | ADR-004 timer rendering not activated; 14 tabs all visible untested | ✅ EXP-02/03: 14-tab load confirmed, exec avg 2.2 ms / max 4.8 ms on RPi — ADR-004 trigger condition never reached |

---

## 3. Utility Tree

Priority notation: **(Technical Risk, Business Importance)** — H = High, M = Medium, L = Low

```
Utility
├── Measurement Accuracy [QAS-5] — Governing goal
│   └── Rate / Amplitude / Beat Error match Witschi No.1000 (H, H) ✅ EXP-06
│
├── Real-Time Performance [QAS-1] — Enabling QA
│   └── Zero dropped audio blocks at 96 kHz on RPi 5 (H, H) ✅ EXP-01
│
├── Low Latency [QAS-2] — Enabling QA
│   └── E2E capture→display ≤ 30 ms with 11+ tabs (M, H) ✅ EXP-02 (avg 2.2 ms)
│
├── Correctness [QAS-4] — Enabling QA
│   ├── Sub-1: Formula correctness — pre-commit gate (H, H) ✅ ADR-008 + 142 tests
│   ├── Sub-2: Beat detection stable under noise (M, H) ✅ EXP-04 (onset=0.08)
│   └── Sub-3: Noise warning fires at correct threshold (L, M) ✅ EXP-05
│
├── Extensibility / Modifiability [QAS-3] — Independent driver
│   ├── New tab ≤ 3 file changes (M, M) ✅ EXP-03 / EXP-08 (14 tabs)
│   └── New audio source ≤ 2 file changes (L, M) ✅ EXP-03 (NetworkWorker prototype)
│
└── Long-Term Session Performance [QAS-6] — New M3 QA
    └── Plotted points ≤ 3,000 after 7 days; replot ≤ 16 ms (M, M) ✅ EXP-07 (2,520 pts)
```

---

## 4. Architectural Approaches Considered (M3)

### Long-term data management

| Option | Idea | Chosen? |
|--------|------|:-------:|
| A1 — Time-based bucket downsampling | Coarsen bucket size at elapsed-time thresholds (5 min / 30 min / 2 hr) | ✅ M3 (ADR-007) |
| A2 — LTTB downsampling | Largest-Triangle-Three-Buckets visual fidelity over history | ❌ O(n) per cycle — too expensive on Pi GUI thread |
| A3 — Fixed bucket size (always 60) | Simple, predictable | ❌ Loses live detail in first 5 min |
| A4 — Server-side ring buffer / eviction | Drop oldest points when threshold reached | ❌ Loses aging history the user explicitly wants |

### Formula isolation strategy

| Option | Idea | Chosen? |
|--------|------|:-------:|
| F1 — WatchMath namespace (pure functions) | Extract formulas into `WatchMath.h/cpp`, no Qt/DSP deps, directly linkable | ✅ M3 (ADR-008) |
| F2 — Private methods on MeasurementEngine | Keep formulas inside engine | ❌ Requires audio context + Qt event loop to test |
| F3 — Inline lambda per tab | Each tab owns its formula | ❌ Divergence risk — 14 copies with no single source of truth |

### Detector parameter approach

| Option | Idea | Chosen? |
|--------|------|:-------:|
| D1 — FilterChain with tunable parameters | `onset_fraction` and `min_peak_fraction` exposed; optimized by EXP-04 | ✅ M3 (ADR-009) |
| D2 — Fixed hard-coded thresholds | Simple; no tuning surface | ❌ Fails catastrophically at 60 dB SNR (EXP-04) |
| D3 — ML-based adaptive detector | Learns per-watch signal profile | ❌ Outside scope; requires labeled data |

---

## 5. Findings

### Sensitivity Points

| ID | What is sensitive | What changes if you touch it | QA |
|----|------------------|-----------------------------|----|
| SP-1 | **Which thread runs DSP** (unchanged from M2) | Move DSP back to Qt Main Thread → wait_ms jumps from 0.03 ms back to ~77 ms | QAS-1, QAS-2 |
| SP-2 | **isVisible() guard in each tab** (unchanged from M2) | Remove from one tab → that tab's render fires on every beat | QAS-1 |
| SP-3 | **Sample rate (96 kHz)** (unchanged from M2) | Drop to 48 kHz → Beat Error resolution halves (0.01 ms → 0.02 ms) | QAS-5 |
| SP-4 | **Measurement struct is immutable** (unchanged from M2) | Mutable struct → two tabs could show different values for the same beat | QAS-4 |
| SP-5 | **mBucketSize time-thresholds in LongTermTab** (new M3) | Remove time-gating → bucket stays at 1 forever → 50,400 raw points after 7 days → QCP render freeze | QAS-6 |
| SP-6 | **WatchMath as pure functions** (new M3) | Inline formulas into MeasurementEngine → formula regression untestable without audio context | QAS-4 Sub-1 |
| SP-7 | **onset_fraction = 0.08** (new M3) | Lower to 0.02 or 0.05 → catastrophic Rate error at 60 dB SNR (−4,264 s/d at onset=0.02) | QAS-4 Sub-2 |

### Tradeoff Points

| ID | Decision | Helps | Puts pressure on | Accuracy rationale | How we resolved it |
|----|----------|-------|------------------|--------------------|--------------------|
| TP-1 | **96 kHz sample rate** (unchanged) | Beat Error resolution 0.01 ms (Accuracy ↑) | Higher CPU load | 48 kHz halves timing resolution — unacceptable | EXP-01: 0 dropped blocks at 96 kHz |
| TP-2 | **Ring buffer between threads** (unchanged) | Removes GUI coupling (Real-Time Performance ↑) | Adds ~21 ms propagation delay | Delay bounded and within 100 ms E2E — accuracy unaffected | EXP-02: E2E avg 2.2 ms |
| TP-3 | **Lazy Rendering** (unchanged) | 85% fewer render calls (Real-Time Performance ↑) | Non-visible tabs don't update in real time | Non-visible tab data not used for decisions | Users can't see non-visible tabs; tab catches up on show |
| TP-4 | **Shared Measurement struct** (unchanged) | All tabs show identical values (Correctness ↑) | Changing struct affects all 14 tabs | Single source of truth required for accuracy | Struct split into 3 immutable Value Objects |
| TP-5 | **Time-based bucket downsampling** (new M3) | Bounds plotted points to 840/series at 7 days (Long-Term Performance ↑) | Fine-grained oscillations < bucket duration are invisible late in session; mean/σ overlay lags up to 12 min at bucket=60 | Statistical overlays computed over all raw values — accuracy of summary stats unaffected by display downsampling | EXP-07: 2,520 total plotted points << 3,000 budget |
| TP-6 | **WatchMath pure-function isolation** (new M3) | Formula regression testable without audio context (Correctness ↑) | One more source file to maintain; function signature must be stable | Enables pre-commit gate to block formula regressions before they reach any tab | 142 unit tests across 10 binaries, all passing |

### Risks

No open risks as of M3.

| ID | Risk | QA | Status |
|----|------|----|--------|
| R-1 | Witschi accuracy not validated | QAS-5 | ✅ Resolved — EXP-06 (2026-06-25) |
| R-2 | Ring buffer depth not stress-tested | QAS-1, QAS-2 | ✅ Resolved — EXP-01 (9 runs, Dropped=0 all sps) |
| R-3 | ADR-004 timer rendering not activated | QAS-1 | ✅ Closed — trigger condition (14 tabs all visible) never reached in practice; exec avg 2.2 ms confirms headroom |

### Non-Risks

| ID | What was confirmed | Evidence |
|----|--------------------|---------|
| NR-1 | DSP Thread removes queue wait | EXP-02 RPi: wait_ms 77.4 ms → 0.03 ms, 0 deadline miss |
| NR-2 | 96 kHz is sustainable on RPi | EXP-01: 0 dropped blocks at 48/96/192 kHz |
| NR-3 | Lazy Rendering cuts render calls by 85% | EXP-02: replot/beat 8.22 → 1.20 |
| NR-4 | 14 tabs each fit within the 3-file constraint | EXP-08: 14 tabs verified, 0 layer violations |
| NR-5 | Adding a new audio source requires ≤ 2 files | EXP-03: NetworkWorker prototype verified |
| NR-6 | Qt signals deliver data correctly across threads | Qt QueuedConnection is FIFO — bounded latency well under 125 ms beat interval |
| NR-7 | Architecture is testable in isolation | 142 unit tests across 10 binaries, all passing |
| NR-8 | Rate / Amplitude / Beat Error match Witschi No.1000 | EXP-06: Δ Rate 0.2–0.4 s/d · Δ Amplitude 15–25° · Δ Beat Error 0–0.1 ms — all within tolerance (2 rounds) |
| NR-9 | LongTermTab plot point count is bounded for 7-day sessions | EXP-07: 2,520 total plotted points at 7 days ≤ 3,000 budget; render time << 16 ms |
| NR-10 | Beat detection stable under 0–60 dB ambient noise | EXP-04: onset=0.08 / min_peak=0.10 tracks correctly at all noise levels tested |
| NR-11 | Noise warning fires at correct SNR boundary with zero false alarms | EXP-05: 0 false alarms at SNR ≥ 10 dB; popup fires correctly at SNR ≤ 0 dB |
| NR-12 | Formula regressions are blocked before commit acceptance | ADR-008 + pre-commit gate: `TestWatchMath` and `TestMeasurementEngine` run on every commit |

---

## 6. Risk Themes

### Theme 1 — Rendering-Audio Coupling → RESOLVED ✅

**What it was**: Rendering and DSP shared one thread. Every new graph tab made real-time
performance worse.

**Why it mattered**: Root cause of the M1 43% deadline miss.

**How we fixed it**: ADR-001 (DSP Thread) + ADR-002 (Lazy Rendering). Rendering is now fully
outside the audio path.

**M3 confirmation**: EXP-02 E3-07 on RPi with all 14 tabs: exec avg 2.2 ms / max 4.8 ms.
ADR-004 (timer-decoupled rendering) standby condition never reached — performance headroom
confirmed under full tab load.

---

### Theme 2 — Reference Hardware Validation Gap → RESOLVED ✅

**What it was**: QAS-5 (Measurement Accuracy) had never been compared against a real
Witschi watch. Architecture was correct by design, but unconfirmed.

**Why it mattered**: This is the governing goal — the most important QA.

**How we closed it**: EXP-06 (2026-06-25): two sequential rounds on a 21,600 BPH watch.
Δ Rate 0.2 s/d (R2) / 0.4 s/d (R1), Δ Amplitude 15–25° (systematic C-event detection
delay — not measurement error), Δ Beat Error 0–0.1 ms. All within tolerance.

---

### Theme 3 — Conditional Architecture Not Exercised → CLOSED ✅

**What it was**: ADR-004 (timer-decoupled rendering) wired in but never activated. Normal
usage (any one tab visible) never triggers it.

**Why it was low risk**: No user scenario reaches 14 tabs all visible simultaneously.

**M3 close-out**: EXP-02/03 confirmed 14-tab load stays well within deadline. The
conditional branch in ADR-004 is a correctly-placed safety net, not an unexercised unknown.
No further action required.

---

### Theme 4 — Formula Correctness Without Test Isolation → RESOLVED ✅ (new M3)

**What it was**: Watch-math formulas (Rate, Amplitude, Beat Error) lived inside
`MeasurementEngine` as private methods, entangled with Qt types and DSP state. A formula
error would silently corrupt all 14 display tabs with no automated catch.

**Why it mattered**: Formulas are the core correctness guarantee for QAS-4 Sub-1.
Without isolation, any regression required a running audio context + Qt event loop to test.

**How we fixed it**: ADR-008 extracted formulas into `namespace WatchMath` (pure functions,
no Qt/DSP deps). `TestWatchMath` links directly against `WatchMath.cpp` and runs in the
pre-commit gate. Combined with EXP-06 external validation, formula correctness is verified
both analytically and empirically.

---

### Theme 5 — Long-Term GUI Stability Under Multi-Day Sessions → RESOLVED ✅ (new M3)

**What it was**: The Long-Term Performance Graph had no strategy to bound its data. At the
watch's 12 s averaging period, a 7-day session would accumulate ≈ 50,400 raw points per
series. `QCustomPlot::replot()` on the Raspberry Pi's GUI thread would freeze.

**Why it mattered**: Aging tests are a primary use case for the device. A GUI freeze
during a multi-day test would lose all accumulated data.

**How we fixed it**: ADR-007 introduced time-based bucket downsampling with four phases
(0–5 min / 5–30 min / 30 min–2 hr / > 2 hr). Verified analytically by EXP-07: worst-case
2,520 total plotted points at 7 days — two orders of magnitude below QCustomPlot's
degradation threshold. Mean/σ statistical overlays accumulate over all raw values, so
display downsampling does not reduce statistical accuracy.

---

## 7. What's Left

All risks are resolved as of M3. The architecture is ready for final demo.

| Priority | Action | Status |
|----------|--------|--------|
| **Demo prep** | Accelerated simulation via SimWorker loop-play to generate LongTermTab screenshot evidence | Recommended before 07/01 demo |
| **No open risks** | All technical and non-technical risks closed as of 2026-06-25 | ✅ |

---

## Comparison: M2 → M3

| Dimension | M2 State | M3 State |
|-----------|----------|----------|
| Open risks | 3 (R-1 Witschi, R-2 ring buffer, R-3 ADR-004) | **0** |
| QA scenarios validated | QAS-1, 2, 3, 4 (partial) | QAS-1, 2, 3, 4, **5, 6** |
| Experiments completed | EXP-01 through EXP-04 | EXP-01 through **EXP-08** |
| ADRs accepted | ADR-001 through ADR-006 | ADR-001 through **ADR-009** |
| Graph tabs | 11 | **14** |
| Unit test binaries | 6 | **10** |
| Unit tests | 116 | **142** |
| Accuracy vs. Witschi | Not validated | **Validated (2 rounds)** |
| LongTermTab point bound | Not designed | **2,520 / 7 days (≤ 3,000)** |

---

## Related Documents

- [M2 ATAM Evaluation](atam-evaluation-m2.md)
- [QA Scenarios](qa/README.md)
- [ADR-007](adr/ADR-007-longtermtab-downsampling.md) · [ADR-008](adr/ADR-008-watchmath-module-isolation.md) · [ADR-009](adr/ADR-009-filterchain-design.md)
- [Architecture Views](../views/README.md)
- [Experiments](experiments/README.md)
- [Risk Register](risks.md)

## References

- [Bass21] L. Bass, P. Clements, R. Kazman. *Software Architecture in Practice*, Fourth Edition. Addison-Wesley, 2021.
- [Kazman00] R. Kazman, M. Klein, P. Clements. "ATAM: Method for Architecture Evaluation". CMU/SEI-2000-TR-004, August 2000.
