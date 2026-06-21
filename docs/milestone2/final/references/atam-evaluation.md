# Architecture Evaluation — ATAM (Lightweight)

**Method**: Architecture Tradeoff Analysis Method (ATAM) — lightweight version  
**Date**: 2026-06-22  
**Team**: Blue Sky (Team 3) — TimeGrapher  
**Scope**: M2 architecture decisions (ADR-001 ~ ADR-006)

---

## 1. Business Drivers

Why this system exists and what constraints we're working under.

| Item | Description |
|------|-------------|
| **Goal** | Measure mechanical watch rate, amplitude, and beat error accurately on Raspberry Pi 5 |
| **Primary constraint** | 5-week project timeline; RPi 5 is the target hardware |
| **Framework** | Qt (C++) — fixed; no alternative considered |
| **Key stakeholder concern** | Measurement values must match a WeiShi No.1000 reference device |

---

## 2. Utility Tree

Priority is written as **(Business Importance, Technical Difficulty)** — H = High, M = Medium, L = Low.

```
Utility
│
├── Measurement Accuracy
│   └── Rate, Amplitude, Beat Error match WeiShi No.1000 within tolerance          (H, H)
│
├── Real-Time Performance
│   └── 0 dropped audio blocks over a 10-minute session at 96kHz on RPi            (H, H)
│
├── Low Latency
│   └── From beat at microphone to GUI update: E2E latency < 100ms                 (H, M)
│
├── Extensibility
│   └── Add a new graph tab in ≤ 3 files with 0 layer violations                   (M, M)
│
└── Correctness
    └── False trigger rate < 1%; true beat detection rate > 99%                    (H, M)
```

---

## 3. Architecture Approaches

Key decisions that address the scenarios above.

| Decision | What it does | QA it targets |
|----------|-------------|---------------|
| ADR-001: DSP Thread | Moves DSP off the Qt Main Thread onto its own thread | QAS-2, QAS-3 |
| ADR-002: Lazy Rendering | Skips `replot()` for tabs the user is not looking at | QAS-2 |
| ADR-003: 96kHz sample rate | Sets the audio sample rate on RPi | QAS-1, QAS-2 |
| ADR-004: Timer rendering (conditional) | Switches to 20 FPS fixed timer if all 14 tabs are visible | QAS-2 |
| ADR-005: IAudioSource interface | One interface for all audio sources; no concrete dependencies above | QAS-4 |
| ADR-006: Observer pattern | One `measurementReady` signal delivers the same struct to all 14 tabs | QAS-4, QAS-5 |

---

## 4. Sensitivity Points

A sensitivity point is a specific property that, if changed, strongly changes a QA outcome.

| ID | Component / Property | If this changes... | QA affected |
|----|---------------------|--------------------|-------------|
| SP-1 | **Which thread runs DSP** — currently its own dedicated thread | If DSP moves back to Qt Main Thread, wait_ms goes from 0.03ms back to 77ms | QAS-2, QAS-3 |
| SP-2 | **Sample rate** — currently 96kHz | Dropping to 48kHz doubles the minimum Beat Error resolution (0.01ms → 0.02ms) | QAS-1 |
| SP-3 | **Measurement struct is immutable** — tabs receive it read-only | If tabs could modify the struct, two tabs showing the same metric could show different values | QAS-5 |

---

## 5. Tradeoff Points

A tradeoff is a decision that helps one QA but puts pressure on another.

| ID | Decision | QA gained | QA under pressure | How we resolved it |
|----|----------|-----------|-------------------|--------------------|
| TP-1 | **96kHz sample rate** | Better Beat Error resolution → QAS-1 ↑ | Higher CPU load → QAS-2 at risk | EXP-02 confirmed 0 dropped blocks at 96kHz on RPi — headroom is sufficient |
| TP-2 | **Ring buffer between threads** | Decouples capture from DSP → QAS-2, QAS-3 ↑ | Adds a synchronization point (complexity ↑) | Mutex covers index only; data copy is lock-free. EXP-03 confirmed no latency impact |
| TP-3 | **Lazy Rendering (ADR-002)** | 85% fewer replot calls → QAS-2 ↑ | Non-visible tabs don't update in real time | Acceptable — user cannot see non-visible tabs. Tab catches up when it becomes visible |
| TP-4 | **Shared Measurement struct (ADR-006)** | All tabs get identical data → QAS-5 ↑ | Changing the struct affects all 14 tabs at once | Struct is split into three immutable Value Objects — each tab only depends on what it needs |

---

## 6. Risks

Decisions where a QA goal is not yet confirmed by evidence.

| ID | What the risk is | Which QA | Status |
|----|-----------------|----------|--------|
| R-1 | **WeiShi accuracy comparison not done yet** — QAS-1 (the governing goal) has no reference hardware validation. Architecture is correct by design, but unconfirmed. | QAS-1 | ⏳ EXP-01 scheduled 06/29 |
| R-2 | **Timer rendering (ADR-004) not activated** — If all 14 tabs are visible at once, rendering behavior under full load is untested. ADR-004 only turns on if EXP-05 shows deadline miss. | QAS-2 | ⏳ Conditional |
| R-3 | **BPH range only partially tested** — Filter parameters confirmed at 28,800 BPH. Full range (18,000–36,000 BPH) not yet validated. | QAS-5 | ⏳ Stretch goal |

---

## 7. Non-Risks

Decisions confirmed safe by experiment results.

| ID | What was confirmed | Evidence |
|----|--------------------|---------|
| NR-1 | DSP Thread removes queue wait completely | EXP-03 RPi: wait_ms 77.4ms → 0.03ms, 0 deadline miss |
| NR-2 | 96kHz is sustainable on RPi | EXP-02: 0 dropped blocks at 48 / 96 / 192kHz |
| NR-3 | Lazy Rendering cuts render calls by 85% | EXP-03: replot/beat 8.22 → 1.20 |
| NR-4 | 14 tabs each fit within the 3-file constraint | EXP-04: 14 tabs verified, 0 layer violations |
| NR-5 | Adding a new audio source requires ≤ 2 files | EXP-04: NetworkWorker prototype — 0 changes above SessionController |
| NR-6 | Architecture is testable in isolation | 142 unit tests across 10 binaries, all passing |

---

## 8. Risk Themes

Patterns found across the individual risks.

| Theme | Risks | What to do |
|-------|-------|-----------|
| **Reference hardware not yet validated** | R-1 | EXP-01 (WeiShi comparison) is the critical path — must complete before the final demo on 07/01 |
| **Conditional architecture not exercised** | R-2 | ADR-004 is idle — normal load stays within budget. Only activates if EXP-05 shows a problem |
| **Stimulus coverage is partial** | R-3 | 28,800 BPH is confirmed. Wider BPH range is a post-M3 stretch goal |

---

## Related Documents

- [QA Scenarios](qa/README.md)
- [ADR-001](adr/ADR-001-t2-dsp-offload-thread.md) · [ADR-002](adr/ADR-002-r1-lazy-rendering.md) · [ADR-003](adr/ADR-003-sample-rate-selection.md) · [ADR-004](adr/ADR-004-r2-timer-decoupled-rendering.md) · [ADR-005](adr/ADR-005-p1-iaudiosource-dependency-inversion.md) · [ADR-006](adr/ADR-006-basegraphtab-observer-pattern.md)
- [Experiment Results](experiments/)
