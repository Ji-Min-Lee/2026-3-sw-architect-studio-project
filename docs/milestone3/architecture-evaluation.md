# Architecture Evaluation — ATAM

**Project**: TimeGrapher | **Date**: 2026-06-22 | **Team**: Blue Sky (Team 3)  
**Method**: ATAM (Architecture Tradeoff Analysis Method) — SEI

---

## Executive Summary

The main risk identified was that DSP processing and GUI rendering ran on the same thread. On Raspberry Pi 5, this caused **43% of audio blocks to miss their 21 ms deadline**.

This was addressed by separating DSP into its own thread ([ADR-001](ADRs/ADR001-dsp-offload-thread.md)) and applying lazy rendering ([ADR-002](ADRs/ADR002-lazy-rendering.md)). After the changes: deadline miss dropped to **0%**, and queue wait time dropped by **×2,600**.

One open risk remains: **accuracy against the WeiShi reference device** — EXP-01 is scheduled for 2026-06-29 (Week 5).

---

## 1. Business Drivers

| Item | Detail |
|------|--------|
| Core goal | Measure mechanical watch Rate, Amplitude, and Beat Error accurately |
| Target hardware | Raspberry Pi 5 (4-core ARM, 8 GB RAM) |
| Framework | Qt (C++) — fixed choice |
| Key constraint | 21 ms audio block deadline at 96 kHz |
| Stakeholder concern | Results must match WeiShi No.1000 reference device |

### Critical Requirement and Architectural Drivers

| Type | QA | What it shaped in the architecture |
|------|----|------------------------------------|
| **Critical requirement** | Measurement Accuracy (QAS-2) | The reason the system exists. Every other QA exists to make this one achievable. |
| **Architectural driver** | Real-Time Performance (QAS-1) | The 21 ms deadline forced DSP onto its own thread (ADR-001) and lazy rendering (ADR-002). |
| **Architectural driver** | Extensibility (QAS-5) | The "add a tab in ≤ 3 files" goal forced the Observer pattern (ADR-006): a new tab plugs in by subscribing to one signal. |

---

## 2. Architecture — Before and After

**Before (baseline)** — one thread ran capture, DSP, and rendering together. GUI rendering consumed 79% of the 21 ms exec budget, causing a **43% deadline miss** on RPi 5.

**After (M2)** — DSP moved to its own thread (ADR-001) and rendering became lazy (ADR-002), removing rendering from the audio path entirely.

| Metric | Before | After | Change |
|--------|:------:|:-----:|:------:|
| Queue wait avg | 77.4 ms | 0.03 ms | ×2,600 improvement |
| Deadline miss | 43% | 0% | Eliminated |
| Render calls per beat | 8.22 | 1.20 | ↓85% |

---

## 3. Utility Tree

Priority notation: **(Technical Risk, Business Importance)** — H = High, M = Medium, L = Low

| QA Scenario | Scenario | Priority |
|-------------|----------|:--------:|
| QAS-2 Measurement Accuracy | Rate error vs WeiShi < 5 s/d; Beat Error < 0.1 ms | (H, H) |
| QAS-1 Real-Time Performance | 96 kHz sustained, dropped blocks = 0 | (H, H) |
| QAS-3 Low Latency | E2E latency < 100 ms (28,800 BPH baseline) | (M, H) |
| QAS-5 Extensibility | New graph tab ≤ 3 files, zero lower-layer changes | (L, M) |
| QAS-4 Correctness | All tabs show identical data for same beat | (L, M) |

---

## 4. Architectural Approaches Considered

### Rendering Strategy

| Option | Idea | Status |
|--------|------|:------:|
| R1 — Lazy Rendering | Skip render for tabs the user is not looking at | ✅ Applied (ADR-002) |
| R2 — Timer-Decoupled | Render at fixed 20 FPS, fully separate from audio | ⏳ M3 contingency if full-tab load fails |
| R3 — Double-Buffer | Render into offscreen buffer, flip on demand | ❌ Too complex for timeline |

### Threading Strategy

| Option | Idea | Status |
|--------|------|:------:|
| T1 — SCHED_RR | Give audio thread higher OS scheduling priority | ❌ Not needed — EXP-02 showed no improvement in dropped blocks |
| T2 — DSP Offload Thread | Move DSP to its own thread, pass data via ring buffer | ✅ Applied (ADR-001) |
| T3 — Full Pipeline Split | One thread per pipeline stage | ❌ Too complex for 5-week timeline |

### Structural Decisions

| Decision | Applied |
|----------|:-------:|
| Observer pattern (BaseGraphTab) — one signal to all tabs | ✅ ADR-006 |
| IAudioSource interface — one interface for all audio sources | ✅ ADR-005 (M2) |
| Ring buffer — PCM handoff between threads | ✅ ADR-005 (M3 ring buffer) |

---

## 5. Findings

### Sensitivity Points

A sensitivity point is: changing this one thing significantly changes a QA outcome.

| ID | What is sensitive | What changes if you touch it | QA |
|----|------------------|-----------------------------|----|
| SP-1 | Which thread runs DSP | Move DSP back to Qt main thread → wait_ms jumps from 0.03 ms back to 77 ms | QAS-1, QAS-3 |
| SP-2 | Visibility guard in each tab | Remove the guard from one tab → that tab's render fires every beat, restoring the bottleneck | QAS-1 |
| SP-3 | Sample rate (96 kHz) | Drop to 48 kHz → Beat Error time resolution halves | QAS-2 |
| SP-4 | Measurement struct is immutable | If tabs could modify the struct → two tabs could show different values for the same beat | QAS-4 |

### Tradeoff Points

A tradeoff is: this decision helps one QA goal but puts pressure on another.

| ID | Decision | Helps | Puts pressure on | Resolution |
|----|----------|-------|------------------|------------|
| TP-1 | 96 kHz sample rate | Better Beat Error resolution (QAS-2 ↑) | Higher CPU load (QAS-1 at risk) | EXP-02: 0 dropped blocks at 96 kHz — headroom confirmed |
| TP-2 | Ring buffer between threads | Removes GUI coupling (QAS-1, QAS-3 ↑) | Adds ~21 ms propagation delay per block | Still well within 100 ms E2E budget; EXP-03 confirmed |
| TP-3 | Lazy Rendering | 85% fewer render calls (QAS-1 ↑) | Non-visible tabs do not update in real time | Users cannot see non-visible tabs; tab catches up on switch |
| TP-4 | Shared Measurement struct | All tabs get identical data (QAS-4 ↑) | Changing the struct affects all 14 tabs | Struct split into 3 immutable Value Objects |

### Risks

| ID | Risk | QA | Status |
|----|------|----|--------|
| R-1 | **WeiShi accuracy not validated** — QAS-2 (governing goal) unconfirmed against reference hardware | QAS-2 | ⏳ EXP-01 scheduled 2026-06-29 |
| R-2 | **Ring buffer depth not stress-tested** — too shallow = drops; too deep = added latency | QAS-1, QAS-3 | ⏳ RPi stress test needed |
| R-3 | **Thermal throttling on RPi** — at 85°C clock drops; headroom shrinks under sustained load | QAS-1 | ⏳ Active cooling not yet specified |
| R-4 | **Timer rendering (R2) not activated** — 14 tabs all visible at once is untested | QAS-1 | ⏳ Conditional on EXP-05 follow-up |

### Non-Risks (Confirmed by Experiment)

| ID | What was confirmed | Evidence |
|----|--------------------|----------|
| NR-1 | DSP Thread eliminates queue wait | EXP-03: wait_ms 77.4 ms → 0.03 ms (×2,600); 0 deadline miss |
| NR-2 | 96 kHz is sustainable on RPi | EXP-02: 0 dropped blocks at 48 / 96 / 192 kHz |
| NR-3 | Lazy Rendering cuts render calls by 85% | EXP-03: replot/beat 8.22 → 1.20 |
| NR-4 | 14 tabs each fit within the 3-file constraint | EXP-04: 14 tabs verified, 0 layer violations by DSM |
| NR-5 | Adding a new audio source requires ≤ 2 files | EXP-04: NetworkWorker prototype verified |
| NR-6 | Qt signals deliver data correctly across threads | Qt QueuedConnection is FIFO; bounded latency confirmed well under 125 ms beat interval |
| NR-7 | Architecture is testable in isolation | 142 unit tests across 10 test binaries, all passing |

---

## 6. Risk Themes

### Theme 1 — Rendering-Audio Coupling → RESOLVED ✅

**What it was**: Rendering and DSP shared one thread. Every new graph tab made real-time performance worse. Root cause of the 43% deadline miss.

**How it was fixed**: ADR-001 (DSP Thread) + ADR-002 (Lazy Rendering). Rendering is now fully outside the audio path. Validated by EXP-03.

### Theme 2 — Reference Hardware Validation Gap → OPEN ⏳

**What it is**: QAS-2 (Measurement Accuracy) is the governing goal, but no comparison against the WeiShi No.1000 reference has been run yet. Architecture is correct by design but unconfirmed.

**What to do**: EXP-01 must complete before the final demo on 2026-07-01.

### Theme 3 — Conditional Architecture Not Exercised → LOW RISK ⏳

**What it is**: The timer-decoupled rendering path (R2) is designed but never activated. It only activates if EXP-05 follow-up confirms that all 14 tabs simultaneously visible causes a deadline miss.

**Why it is low risk**: Normal usage never opens all 14 tabs at once.

---

## 7. Actions for M3

| Priority | Action | Addresses |
|----------|--------|-----------|
| **Critical** | Run EXP-01 — WeiShi accuracy comparison before 2026-07-01 | R-1, Theme 2 |
| **High** | Stress-test ring buffer depth on RPi under peak load | R-2 |
| **Medium** | Attach heatsink / fan for sustained RPi operation | R-3 |
| **Low** | Confirm R2 behavior if all 14 tabs open simultaneously | R-4, Theme 3 |

---

## Related Documents

- [Experiment Results](experiment-results.md)
- [ADR-001](ADRs/ADR001-dsp-offload-thread.md) · [ADR-002](ADRs/ADR002-lazy-rendering.md) · [ADR-003](ADRs/ADR003-layered-architecture.md) · [ADR-006](ADRs/ADR006-observer-pattern.md)
- [Module View](architecture/module-view.md) · [Runtime View](architecture/runtime-view.md)
