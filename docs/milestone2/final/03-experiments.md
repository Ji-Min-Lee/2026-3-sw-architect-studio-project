# Experiment Results

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Experiment Status Overview

| ID | Experiment | Risk Resolved | Status |
|----|------------|---------------|--------|
| EXP-01 | Sample Rate Performance on RPi 5 | TR-01 | ⏳ Pending (RPi R5) |
| **EXP-02** | **E2E Audio Pipeline Latency Baseline** | **TR-02, TR-04** | **✅ Complete (macOS)** |
| EXP-03 | Filter Parameter Sweep (LP/HP cutoffs) | TR-05 | ⏳ Pending |
| EXP-04 | Cross-Compilation & RPi Deploy | TR-03 | ⏳ In progress |
| EXP-05 | Qt Multi-Tab Rendering Performance | TR-04 | ⏳ Pending (RPi R5) |

---

## EXP-02: The Core Finding — [Full Experiment Doc](references/exp-02.md)

### Problem Confirmed — RPi Baseline (single-threaded)

| Metric | Value | Meaning |
|--------|:-----:|---------|
| exec avg | **20 ms** | 95% of 21ms deadline consumed |
| plot share of exec | **16 ms (79%)** | Rendering is the bottleneck |
| Deadline miss | **43%** | 441 / 1,015 frames failed |
| CPU core 2 load | **91%** | All pipeline work on one core |
| Other cores | **idle** | Multi-core unused |
| Temperature | **85°C** | Thermal throttle — clock degraded |

**Root cause is structural, not algorithmic.** Algorithm tuning cannot fix this.

```
Root Cause 1: Rendering coupled to audio exec path
  → plot (16ms) × N_tabs runs inside AudioThread exec budget

Root Cause 2: Single-core saturation
  → AudioCapture + DSP + Qt rendering all on cpu2

Root Cause 3: Thermal throttle
  → 85°C → clock reduced → effective performance further degraded
```

---

### Experiment Runs (macOS, 96kHz Playback)

| Run | Tactic Applied | wait_ms avg | exec_ms avg | Deadline Miss | Backlog | replot_count avg |
|:---:|----------------|:-----------:|:-----------:|:-------------:|:-------:|:----------------:|
| R1 | Baseline (main thread DSP) | 420 ms | 0.57 ms | 0% | **47%** | — |
| R2 | **T2: DSP Offload Thread** | **0.013 ms** | 0.40 ms | 0% | **0%** | — |
| R2b | T2 + no R1 guard (replot baseline) | 0.013 ms | ~0.40 ms | 0% | 0% | **8.22** |
| R3 | **T2 + R1: Lazy Rendering** (Scenario A, 1 tab) | 0.013 ms | 0.395 ms | 0% | 0% | **2.08 (↓75%)** |
| R4 | **T2 + R1: Lazy Rendering** (Scenario B, tab switch) | 0.013 ms | 0.395 ms | 0% | 0% | **1.20 (↓85%)** |

---

### Key Results

**T2 (DSP Offload Thread)**

- wait_ms: 420ms → **0.013ms** (×32,000 reduction)
- backlog: 47% → **0%** (frame accumulation fully eliminated)
- bg_fps ≈ fg_fps (95.6 ≈ 95.6): DSP thread tracks worker in real time

**R1 (Lazy Rendering — isVisible() guard)**

- replot_count: 8.22 → 2.08 per beat (Scenario A: **75% reduction**)
- replot_count: 8.22 → 1.20 per beat (Scenario B: **85% reduction**)
- Tab switch UX: `QTimer::singleShot(0)` in `showEvent()` — instant catch-up, no UI block

---

## Experiment → Architecture Causal Chain

```
EXP-02 RPi Baseline
  └─ Deadline miss 43%
       ├─ Root Cause 1: plot in exec path (16ms, 79%)
       │    └─ → ADD-2-02: R1 Lazy Rendering
       │         Tactic: Reduce Computational Overhead
       │         Result: replot ↓75–85% (macOS validated)
       │         Expected RPi impact: ~14ms plot_ms saving
       │
       └─ Root Cause 2: Single-core saturation (cpu2 91%)
            └─ → ADD-2-01: T2 DSP Offload Thread
                 Tactic: Introduce Concurrency
                 Result: wait_ms ×32,000 reduction, backlog 0% (macOS validated)
```

Both decisions are validated on macOS. Next step: RPi R5 — run T2+R1 on target hardware.

---

## Remaining Questions

| Question | Experiment | Target |
|----------|------------|--------|
| What is the maximum sustained sample rate on RPi 5 without block drop? | EXP-01 (RPi R5) | 06/23 |
| Does T2+R1 resolve deadline miss on RPi? | EXP-02 RPi R5 | 06/23 |
| Is T1 (SCHED_RR + CPU Affinity) needed on top of T2+R1? | EXP-02 RPi R6 | 06/24 |
| What LP/HP filter cutoffs minimize noise while preserving beat signal? | EXP-03 | 06/25 |
| What is Qt rendering FPS under 11-tab load on RPi? | EXP-05 | 06/26 |
