# Architecture Evaluation — ATAM (v3)

**Project**: TimeGrapher  
**Date**: 2026-06-22  
**Team**: Blue Sky (Team 3)  
**Method**: ATAM (Architecture Tradeoff Analysis Method) — SEI

---

## Executive Summary

The main risk we found was that DSP processing and GUI rendering ran on the same thread.
On Raspberry Pi 5, this caused **43% of audio blocks to miss their deadline**.

We addressed this by separating DSP into its own thread (ADR-001) and adding lazy rendering (ADR-002).
After the changes: deadline miss dropped to **0%**, and queue wait time dropped by **×2,600**.

The one open risk is **accuracy against the WeiShi reference device** — this validation is scheduled for Week 5 (06/29).

---

## What is ATAM?

ATAM is a method for finding risks in an architecture before they become real problems.

It does **not** measure the system precisely.
It finds: which decisions could cause trouble, and where decisions force a tradeoff between two goals.

**Output of this evaluation:**

| Section | Output |
|---------|--------|
| Section 1 | Business goals and constraints |
| Section 3 | Utility Tree — QA priorities |
| Section 4 | Architectural approaches considered |
| Section 5 | Risks, Non-Risks, Sensitivity Points, Tradeoffs |
| Section 6 | Risk Themes |

---

## 1. Business Drivers

| Item | Detail |
|------|--------|
| **Core goal** | Measure mechanical watch Rate, Amplitude, and Beat Error accurately |
| **Target hardware** | Raspberry Pi 5 (4-core ARM, 8 GB RAM) |
| **Framework** | Qt (C++) — fixed choice |
| **Key constraint** | 21ms audio deadline per block (ALSA, ~96kHz) |
| **Stakeholder concern** | Results must match WeiShi No.1000 reference device |

---

## 2. Architecture — Before and After

### Before (single thread)

```
One thread (cpu2):
  AudioCapture → FilterChain → BeatDetector → MeasurementEngine
              → renderAllTabs()    ← bottleneck
```

**Problem**: GUI rendering consumed 79% of the 21ms exec budget.
Result: 43% deadline miss on RPi 5.

### After (three threads — ADR-001 + ADR-002)

```
Audio Source Thread:
  AudioCapture → [write] Audio Buffer

DSP Thread (ADR-001):
  [read] Audio Buffer → FilterChain → BeatDetector → MeasurementEngine
                                                    → measurementReady signal

Qt Main Thread:
  [receive signal] → render visible tab only (ADR-002)
```

**Result**: Rendering is fully removed from the audio path.
Queue wait: 77.4ms → 0.03ms. Deadline miss: 43% → 0%.

---

## 3. Utility Tree

Priority notation: **(Technical Risk, Business Importance)** — H = High, M = Medium, L = Low

```
Utility
│
├── Measurement Accuracy
│   └── Rate, Amplitude, Beat Error match WeiShi No.1000 within tolerance    (H, H)
│
├── Real-Time Performance
│   └── 0 dropped audio blocks in a 10-minute session at 96kHz on RPi        (H, H)
│
├── Low Latency
│   └── From beat at microphone to GUI update: E2E latency < 100ms            (H, M)
│
├── Extensibility
│   └── Add a new graph tab in ≤ 3 files with 0 layer violations              (M, M)
│
└── Correctness
    └── False trigger rate < 1%;  true beat detection rate > 99%              (M, H)
```

---

## 4. Architectural Approaches Considered

### Rendering strategy

| Option | Idea | Chosen? |
|--------|------|:-------:|
| R1 — Lazy Rendering | Skip render for tabs the user is not looking at | ✅ M2 |
| R2 — Timer-Decoupled | Render at fixed 20 FPS, fully separate from audio | ⏳ M3 if needed |
| R3 — Double-Buffer | Render into offscreen buffer, flip on demand | ❌ Too complex |

### Threading strategy

| Option | Idea | Chosen? |
|--------|------|:-------:|
| T1 — SCHED_RR | Give audio thread higher OS priority | ❌ Not needed (EXP-02: no improvement in dropped blocks) |
| T2 — DSP Offload Thread | Move DSP to its own thread, pass data via buffer | ✅ M2 (ADR-001) |
| T3 — Full Pipeline Split | One thread per pipeline stage | ❌ Too complex for timeline |

### Structural decisions

| Option | Idea | Applied? |
|--------|------|:--------:|
| Observer (Qt signals) | MeasurementEngine sends one signal to all tabs | ✅ ADR-006 |
| IAudioSource interface | One interface for all audio sources | ✅ ADR-005 |
| Ring Buffer | PCM handoff between threads (mutex for index sync only) | ✅ ADR-001 |

---

## 5. Findings

### Sensitivity Points

A sensitivity point is: if you change this one thing, a QA goal changes significantly.

| ID | What is sensitive | What changes if you touch it | QA |
|----|------------------|-----------------------------|----|
| SP-1 | **Which thread runs DSP** | Move DSP back to Qt Main Thread → wait_ms jumps from 0.03ms back to 77ms | QAS-2, QAS-3 |
| SP-2 | **isVisible() guard in each tab** | Remove the guard from one tab → that tab's render fires on every beat, restoring the bottleneck | QAS-2 |
| SP-3 | **Sample rate (96kHz)** | Drop to 48kHz → Beat Error resolution halves (0.01ms → 0.02ms) | QAS-1 |
| SP-4 | **Measurement struct is immutable** | If tabs could modify the struct → two tabs could show different values for the same beat | QAS-5 |

### Tradeoff Points

A tradeoff is: this decision helps one QA goal but puts pressure on another.

| ID | Decision | Helps | Puts pressure on | How we resolved it |
|----|----------|-------|------------------|--------------------|
| TP-1 | **96kHz sample rate** | Better Beat Error resolution (QAS-1 ↑) | Higher CPU load (QAS-2 at risk) | EXP-02: 0 dropped blocks at 96kHz — headroom confirmed |
| TP-2 | **Ring buffer between threads** | Removes GUI coupling (QAS-2, QAS-3 ↑) | Adds ~21ms propagation delay | Still well within 100ms E2E target. EXP-03 confirmed. |
| TP-3 | **Lazy Rendering** | 85% fewer render calls (QAS-2 ↑) | Non-visible tabs don't update in real time | Users can't see non-visible tabs. Tab catches up on show. |
| TP-4 | **Shared Measurement struct** | All tabs get identical data (QAS-5 ↑) | Changing the struct affects all 14 tabs | Struct split into 3 immutable Value Objects — each tab only depends on what it needs |

### Risks

| ID | Risk | QA | Status |
|----|------|----|--------|
| R-1 | **WeiShi accuracy not validated** — QAS-1 is the governing goal but no comparison against reference hardware has been done yet | QAS-1 | ⏳ EXP-01 scheduled 06/29 |
| R-2 | **Ring buffer depth not stress-tested** — Too shallow = dropped blocks; too deep = added latency. Set conservatively but not validated under peak load | QAS-2, QAS-3 | ⏳ Needs RPi stress test |
| R-3 | **Thermal throttling on RPi** — At 85°C the CPU clock drops. Headroom may shrink under sustained operation with no cooling specified | QAS-2 | ⏳ No active cooling yet |
| R-4 | **Timer rendering (ADR-004) not activated** — Rendering under all 14 tabs visible at once is untested | QAS-2 | ⏳ Conditional on EXP-05 |

### Non-Risks

| ID | What was confirmed | Evidence |
|----|--------------------|---------|
| NR-1 | DSP Thread removes queue wait | EXP-03 RPi: wait_ms 77.4ms → 0.03ms, 0 deadline miss |
| NR-2 | 96kHz is sustainable on RPi | EXP-02: 0 dropped blocks at 48 / 96 / 192kHz |
| NR-3 | Lazy Rendering cuts render calls by 85% | EXP-03: replot/beat 8.22 → 1.20 |
| NR-4 | 14 tabs each fit within the 3-file constraint | EXP-04: 14 tabs verified, 0 layer violations |
| NR-5 | Adding a new audio source requires ≤ 2 files | EXP-04: NetworkWorker prototype verified |
| NR-6 | Qt signals deliver data correctly across threads | Qt QueuedConnection is FIFO — bounded latency well under 125ms beat interval |
| NR-7 | Architecture is testable in isolation | 142 unit tests across 10 binaries, all passing |

---

## 6. Risk Themes

### Theme 1 — Rendering-Audio Coupling → RESOLVED ✅

**What it was**: Rendering and DSP shared one thread. Every new graph tab made real-time performance worse.

**Why it mattered**: Root cause of the 43% deadline miss.

**How we fixed it**: ADR-001 (DSP Thread) + ADR-002 (Lazy Rendering). Rendering is now fully outside the audio path.

---

### Theme 2 — Reference Hardware Validation Gap → OPEN ⏳

**What it is**: QAS-1 (Measurement Accuracy) has never been compared against a real WeiShi watch. Architecture is correct by design, but unconfirmed.

**Why it matters**: This is the governing goal — the most important QA.

**What to do**: EXP-01 must complete before the final demo on 07/01.

---

### Theme 3 — Conditional Architecture Not Exercised → LOW RISK ⏳

**What it is**: ADR-004 (timer-decoupled rendering) is wired in but never activated. It only turns on if 14 tabs are all visible at once.

**Why it's low risk**: Normal usage never triggers it.

**What to watch**: If EXP-05 shows deadline miss under full tab load, activate ADR-004.

---

## 7. What's Left

| Priority | Action | Addresses |
|----------|--------|-----------|
| **Critical** | Run EXP-01 — WeiShi accuracy comparison | R-1, Theme 2 |
| **High** | Stress-test ring buffer depth on RPi under peak load | R-2 |
| **Medium** | Add cooling (heatsink/fan) for sustained RPi operation | R-3 |
| **Low** | Confirm ADR-004 behavior if all 14 tabs open simultaneously | R-4, Theme 3 |

---

## Related Documents

- [v1 — Our original evaluation](atam-evaluation.md)
- [v2 — Full evaluation by jimin318.lee](atam-evaluation-full.md)
- [QA Scenarios](qa/README.md)
- [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) · [ADR-002](adr/ADR-002-r1-lazy-rendering.md) · [ADR-003](adr/ADR-003-sample-rate-selection.md) · [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) · [ADR-005](adr/ADR-005-p1-iaudiosource-dependency-inversion.md) · [ADR-006](adr/ADR-006-basegraphtab-observer-pattern.md)
- [Experiment Results](experiments/)
