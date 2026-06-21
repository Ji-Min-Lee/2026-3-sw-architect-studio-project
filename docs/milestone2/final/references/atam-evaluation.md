# Architecture Evaluation — ATAM (Lightweight)

**Method**: Architecture Tradeoff Analysis Method (ATAM) — simplified for M2 scope  
**Date**: 2026-06-22  
**Evaluator**: TimeGrapher Team (Blue Sky, Team 3)  
**Architecture under review**: TimeGrapher M2 — DSP Pipeline, Layered Structure, Observer Pattern

---

## 1. Utility Tree

Quality attributes prioritized by (Importance / Difficulty to achieve):

| Priority | QA | Scenario | Importance | Difficulty |
|:---:|---|---|:---:|:---:|
| 1 | **Measurement Accuracy** | Rate, Amplitude, Beat Error match WeiShi No.1000 within tolerance | High | High |
| 2 | **Real-Time Performance** | 0 dropped blocks over 10-minute session at 96kHz on RPi | High | High |
| 3 | **Low Latency** | E2E latency < 100ms (beat at microphone → GUI update) | High | Medium |
| 4 | **Extensibility** | New graph tab in ≤ 3 files, 0 layer violations | Medium | Medium |
| 5 | **Correctness** | False trigger rate < 1%; true beat detection rate > 99% | High | Medium |

---

## 2. Architecture Approaches

Key architectural decisions evaluated:

| ID | Decision | Primary QA |
|----|----------|-----------|
| ADR-001 | DSP Thread separation (Audio Source Thread + DSP Thread + Qt Main Thread) | QAS-2, QAS-3 |
| ADR-002 | Lazy Rendering — `isVisible()` guard skips `replot()` for non-visible tabs | QAS-2 |
| ADR-003 | 96kHz sample rate selection on RPi | QAS-1, QAS-2 |
| ADR-004 | Timer-decoupled rendering (R2) — 20 FPS timer if 14 tabs all visible (conditional) | QAS-2 |
| ADR-005 | IAudioSource dependency inversion | QAS-4 |
| ADR-006 | BaseGraphTab Observer pattern — single `measurementReady` signal to all tabs | QAS-4, QAS-5 |

---

## 3. Sensitivity Points

Decisions that have a strong and direct effect on a single QA:

| Sensitivity Point | Decision | QA Affected | Effect |
|-------------------|----------|-------------|--------|
| SP-1 | ADR-001: DSP Thread isolation | QAS-2, QAS-3 | Primary driver — removing GUI coupling eliminates queue wait entirely |
| SP-2 | ADR-003: Sample rate (96kHz) | QAS-1 | Beat Error resolution = 1/96000s ≈ 0.01ms; halving rate doubles minimum error |
| SP-3 | ADR-006: Observer + immutable `Measurement` | QAS-5 | Structural guarantee — all 14 tabs receive identical struct; deviation is impossible |

---

## 4. Tradeoff Points

Decisions that improve one QA but create tension with another:

| Tradeoff Point | Decision | QA Gained | QA at Risk | Resolution |
|----------------|----------|-----------|-----------|------------|
| TP-1 | ADR-003: 96kHz vs 48kHz | QAS-1 ↑ (finer Beat Error) | QAS-2 ↓ (higher CPU load) | EXP-02: 0 dropped blocks at 96kHz — CPU headroom confirmed sufficient |
| TP-2 | ADR-001: Ring buffer between threads | QAS-2, QAS-3 ↑ | Complexity ↑ (synchronization point added) | Mutex covers index sync only; data copy is lock-free. Verified by EXP-03. |
| TP-3 | ADR-002: Lazy Rendering | QAS-2 ↑ (85% replot reduction) | Non-visible tabs not updated in real time | Acceptable — user cannot observe non-visible tabs. `showEvent` triggers catch-up render. |
| TP-4 | ADR-006: Shared `Measurement` struct | QAS-5 ↑ (tab consistency) | QAS-4 ↓ (changing struct affects all tabs) | Struct decomposed into immutable Value Objects (DSP math / audio capture / beat detection) — each tab depends only on what it needs. |

---

## 5. Risks

Architectural decisions where QA achievement is not yet confirmed:

| ID | Risk | QA | Status |
|----|------|----|--------|
| R-1 | **QAS-1 unvalidated against reference hardware** — WeiShi No.1000 accuracy comparison (EXP-01) is scheduled for Week 5 (06/29). Architecture is correct by design but the governing goal remains unconfirmed. | QAS-1 | ⏳ Open |
| R-2 | **ADR-004 (R2 timer rendering) not yet activated** — Conditional on EXP-05 showing deadline miss under full 14-tab load. Rendering behavior under maximum tab load is untested. | QAS-2 | ⏳ Conditional |
| R-3 | **BPH range coverage is partial** — Filter parameters confirmed at 28,800 BPH (EXP-05). Coverage of full range (18,000–36,000 BPH) not yet validated. | QAS-5 | ⏳ Stretch goal |

---

## 6. Non-Risks

Decisions confirmed safe by experiment evidence:

| ID | Non-Risk | Evidence |
|----|----------|---------|
| NR-1 | DSP Thread isolation eliminates queue wait | EXP-03 RPi: wait_ms 77.4ms → 0.03ms (×2,600), 0 deadline miss |
| NR-2 | 96kHz is sustainable on RPi without SCHED_RR | EXP-02: 0 dropped blocks at 48/96/192kHz |
| NR-3 | Lazy Rendering reduces render cost significantly | EXP-03: replot/beat 8.22 → 1.20 (↓85%) |
| NR-4 | Observer pattern scales to 14 tabs within 3-file constraint | EXP-04: 14 tabs, ≤3 files each, 0 layer violations confirmed |
| NR-5 | IAudioSource inversion reduces source addition to ≤ 2 files | EXP-04: NetworkWorker prototype — 0 changes above `SessionController` |
| NR-6 | Architecture is testable in isolation | 142 unit tests across 10 binaries, all passing |

---

## 7. Risk Themes

| Theme | Risks Covered | Mitigation |
|-------|---------------|------------|
| **Reference hardware validation gap** | R-1 | EXP-01 scheduled 06/29 (Week 5 Sprint 1). Critical path for final demo. |
| **Conditional architecture not exercised** | R-2 | ADR-004 activates only if EXP-05 shows deadline miss. Currently inactive — normal 14-tab load is within budget. |
| **Partial stimulus coverage** | R-3 | 28,800 BPH confirmed. Broader BPH range is a stretch goal for post-M3. |

---

## 8. Summary

| Category | Count |
|----------|:-----:|
| Sensitivity Points | 3 |
| Tradeoff Points | 4 |
| Risks (open) | 3 |
| Non-Risks (confirmed) | 6 |

**Overall assessment**: The core architecture (DSP Thread + Observer + Layered structure) is confirmed by experiment evidence across QAS-2, QAS-3, QAS-4, and QAS-5. The primary open risk is QAS-1 (Measurement Accuracy against reference hardware), which is the governing goal and is scheduled for validation in Week 5.

---

## Related Documents

- [QA Scenarios](qa/README.md)
- [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) · [ADR-002](adr/ADR-002-r1-lazy-rendering.md) · [ADR-003](adr/ADR-003-sample-rate-selection.md) · [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) · [ADR-005](adr/ADR-005-p1-iaudiosource-dependency-inversion.md) · [ADR-006](adr/ADR-006-basegraphtab-observer-pattern.md)
- [Experiment Results](experiments/)
