# Architecture Evaluation — ATAM (v3)

**Project**: TimeGrapher  
**Date**: 2026-06-22  
**Team**: Blue Sky (Team 3)  
**Method**: ATAM (Architecture Tradeoff Analysis Method) — SEI [Kazman00]

> **Snapshot document** — This report captures the ATAM evaluation conducted on **2026-06-22**.
> It will not be updated. For the current architecture state, refer to the [Architecture Views](../views/README.md) and [ADRs](../adr/).

---

## Executive Summary

The main risk we found was that DSP processing and GUI rendering ran on the same thread.
On Raspberry Pi 5, this caused **43% of audio blocks to miss their deadline**.

We addressed this by separating DSP into its own thread (ADR-001) and adding lazy rendering (ADR-002).
After the changes: deadline miss dropped to **0%**, and queue wait time dropped by **×2,600**.

The one open risk is **accuracy against the Witschi reference device** — this validation is scheduled for Week 5 (06/29).

---

## What is ATAM?

ATAM is a method for finding risks in an architecture before they become real problems
[Kazman00].

It does **not** measure the system precisely.
It finds: which decisions could cause trouble, and where decisions force a tradeoff between two goals.

**How this document is organized:**

| Section | Contents |
|---------|----------|
| Section 1 | Business goals, constraints, architectural drivers |
| Section 2 | The architecture we evaluated (before / after) |
| Section 3 | Utility Tree — QA priorities |
| Section 4 | Architectural approaches considered |
| Section 5 | Sensitivity Points, Tradeoffs, Risks, Non-Risks |
| Section 6 | Risk Themes |
| Section 7 | Suggested next steps |

---

## 1. Business Drivers

| Item | Detail |
|------|--------|
| **Core goal** | Measure mechanical watch Rate, Amplitude, and Beat Error accurately |
| **Target hardware** | Raspberry Pi 5 (4-core ARM, 8 GB RAM) |
| **Framework** | Qt (C++) — fixed choice |
| **Key constraint** | 21ms audio deadline per block (ALSA, ~96kHz) |
| **Stakeholder concern** | Results must match Witschi No.1000 reference device |

### Critical Requirement & Architectural Drivers

ATAM Step 2 asks us to separate two things: the one goal the system exists for, and the
QAs that actually shaped the architecture [Kazman00].

In Bass, Clements & Kazman terminology (*Software Architecture in Practice*, Ch.3),
Real-Time Performance, Low Latency, and Correctness are **enabling QAs** for Accuracy:
each removes a failure mode that would otherwise corrupt the measurement output.
Extensibility is an independent architectural driver [Bass21].

| Type | QA | What it shaped in the architecture |
|------|----|------------------------------------|
| **Governing goal** | Measurement Accuracy (QAS-5) | The user-facing outcome — Rate / Amplitude / Beat Error matching Witschi No.1000. Verified by EXP-06. Not an architectural driver; it is the acceptance criterion that all enabling QAs serve. |
| **Enabling QA** | Real-Time Performance (QAS-1) | Dropped audio blocks cause missed beats → wrong Rate and Beat Error. Forced DSP onto its own thread (ADR-001) and lazy rendering (ADR-002). |
| **Enabling QA** | Low Latency (QAS-2) | Stale display values mislead the user about current watch state. Resolved by ADR-001 (E2E avg 2.2ms, EXP-02). |
| **Enabling QA** | Correctness (QAS-4) | Formula errors and noise-triggered false beats corrupt Rate/Beat Error. Resolved by WatchMath isolation (ADR-008) and detector parameter tuning (ADR-003, ADR-009, EXP-04). |
| **Independent driver** | Extensibility / Modifiability (QAS-3) | The "add a tab in ≤ 3 files" goal forced the Observer pattern (ADR-006) and IAudioSource interface (ADR-005). |

---

## 2. Architecture — Before and After

![Architecture before and after (ADR-001 + ADR-002)](../assets/atam-before-after.png)

**Before** — one thread ran capture, DSP, and rendering together. GUI rendering consumed 79% of the 21ms budget, causing a **43% deadline miss** on RPi 5.

**After** — DSP moved to its own thread (ADR-001) and rendering became lazy (ADR-002), removing rendering from the audio path entirely. Queue wait dropped **77.4ms → 0.03ms**; deadline miss **43% → 0%**.

---

## 3. Utility Tree

Priority notation: **(Technical Risk, Business Importance)** — H = High, M = Medium, L = Low

![Utility Tree](../assets/atam-utility-tree.png)

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
| T1 — SCHED_RR | Give audio thread higher OS priority | ❌ Not needed (EXP-01: no improvement in dropped blocks) |
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
| SP-1 | **Which thread runs DSP** | Move DSP back to Qt Main Thread → wait_ms jumps from 0.03ms back to 77ms | QAS-1, QAS-2 |
| SP-2 | **isVisible() guard in each tab** | Remove the guard from one tab → that tab's render fires on every beat, restoring the bottleneck | QAS-1 |
| SP-3 | **Sample rate (96kHz)** | Drop to 48kHz → Beat Error resolution halves (0.01ms → 0.02ms) | QAS-5 |
| SP-4 | **Measurement struct is immutable** | If tabs could modify the struct → two tabs could show different values for the same beat | QAS-4 |

### Tradeoff Points

A tradeoff is: this decision helps one QA goal but puts pressure on another.

Accuracy was the tiebreaker in every tradeoff: when a decision improved accuracy (or prevented a failure mode that would corrupt measurement output), it was chosen even at cost to another QA.

| ID | Decision | Helps | Puts pressure on | Accuracy rationale | How we resolved it |
|----|----------|-------|------------------|--------------------|--------------------|
| TP-1 | **96kHz sample rate** | Beat Error resolution 0.01ms (Accuracy ↑) | Higher CPU load (Real-Time Performance at risk) | 48kHz halves timing resolution — unacceptable for accuracy | EXP-01: 0 dropped blocks at 96kHz — CPU headroom confirmed |
| TP-2 | **Ring buffer between threads** | Removes GUI coupling (Real-Time Performance ↑) | Adds ~21ms propagation delay (Latency at risk) | Delay is bounded and within 100ms E2E — accuracy unaffected | EXP-02: E2E avg 2.2ms, well within target |
| TP-3 | **Lazy Rendering** | 85% fewer render calls (Real-Time Performance ↑) | Non-visible tabs don't update in real time (Latency for background tabs) | Non-visible tab data is not used for decisions — no accuracy impact | Users can't see non-visible tabs; tab catches up on show |
| TP-4 | **Shared Measurement struct** | All tabs show identical values (Correctness / Consistency ↑) | Changing the struct affects all 14 tabs (Modifiability at risk) | Single source of truth prevents tabs from showing divergent values — required for accuracy | Struct split into 3 immutable Value Objects — each tab only depends on what it needs |

### Risks

| ID | Risk | QA | Status |
|----|------|----|--------|
| R-1 | **Witschi accuracy not validated** — QAS-5 is the governing goal but no comparison against reference hardware has been done yet | QAS-5 | ⏳ EXP-06 scheduled 06/29 |
| R-2 | **Ring buffer depth not stress-tested** — Too shallow = dropped blocks; too deep = added latency. Set conservatively but not validated under peak load | QAS-1, QAS-2 | ⏳ Needs RPi stress test |
| R-3 | **Timer rendering (ADR-004) not activated** — Rendering under all 14 tabs visible at once is untested | QAS-1 | ⏳ Conditional on EXP-04 |

### Non-Risks

| ID | What was confirmed | Evidence |
|----|--------------------|---------|
| NR-1 | DSP Thread removes queue wait | EXP-02 RPi: wait_ms 77.4ms → 0.03ms, 0 deadline miss |
| NR-2 | 96kHz is sustainable on RPi | EXP-01: 0 dropped blocks at 48 / 96 / 192kHz |
| NR-3 | Lazy Rendering cuts render calls by 85% | EXP-02: replot/beat 8.22 → 1.20 |
| NR-4 | 14 tabs each fit within the 3-file constraint | EXP-03: 14 tabs verified, 0 layer violations |
| NR-5 | Adding a new audio source requires ≤ 2 files | EXP-03: NetworkWorker prototype verified |
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

**What it is**: QAS-5 (Measurement Accuracy) has never been compared against a real Witschi watch. Architecture is correct by design, but unconfirmed.

**Why it matters**: This is the governing goal — the most important QA.

**What to do**: EXP-06 must complete before the final demo on 07/01.

---

### Theme 3 — Conditional Architecture Not Exercised → LOW RISK ⏳

**What it is**: ADR-004 (timer-decoupled rendering) is wired in but never activated. It only turns on if 14 tabs are all visible at once.

**Why it's low risk**: Normal usage never triggers it.

**What to watch**: If EXP-04 shows deadline miss under full tab load, activate ADR-004.

---

## 7. What's Left

| Priority | Action | Addresses |
|----------|--------|-----------|
| **Critical** | Run EXP-06 — Witschi accuracy comparison | R-1, Theme 2 |
| **High** | Stress-test ring buffer depth on RPi under peak load | R-2 |
| **Low** | Confirm ADR-004 behavior if all 14 tabs open simultaneously | R-3, Theme 3 |

---

## Related Documents

- [QA Scenarios](qa/README.md)
- [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) · [ADR-002](adr/ADR-002-r1-lazy-rendering.md) · [ADR-003](adr/ADR-003-sample-rate-selection.md) · [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) · [ADR-005](adr/ADR-005-p1-iaudiosource-dependency-inversion.md) · [ADR-006](adr/ADR-006-basegraphtab-observer-pattern.md)
- [Experiment Results](experiments/)

## References

- [Bass21] L. Bass, P. Clements, R. Kazman. *Software Architecture in Practice*, Fourth Edition. Addison-Wesley, 2021.
- [Kazman00] R. Kazman, M. Klein, P. Clements. "ATAM: Method for Architecture Evaluation". CMU/SEI-2000-TR-004, August 2000.
