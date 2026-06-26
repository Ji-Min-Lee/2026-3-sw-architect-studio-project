# Final Demo Grading Evidence Map

**Project**: TimeGrapher — Blue Sky (Team 3)
**Date**: 2026-06-26
**Reference**: Draft LG SW Architect Final Demo scoring rubric (Areas 3, 4, 5)

---

## Area 3 — Quality Attribute Tradeoff Discussion (20 pts)

| Sub-criterion | Pts | Evidence | Notes |
|---|:---:|---|---|
| Clearly identifies the major quality attributes relevant to the project | 5 | [atam-evaluation-v3.md §1](references/atam-evaluation-v3.md) | QAS-1–5 defined; Governing / Enabling / Independent driver classification |
| Clearly explains tradeoffs among quality attributes | 5 | [atam-evaluation-v3.md §5 Tradeoff Points](references/atam-evaluation-v3.md) | TP-1–TP-4: 96 kHz, Ring Buffer, Lazy Rendering, Shared Measurement struct — each with helped/pressured QA |
| Shows that **accuracy** was treated as the highest-priority attribute in the architecture and implementation | 5 | [atam-evaluation-v3.md §1, §5](references/atam-evaluation-v3.md) | "Accuracy was the tiebreaker in every tradeoff" explicitly stated; Real-Time, Latency, Correctness all classified as enabling QAs serving Accuracy |
| Explains what was actually achieved and what limitations remain | 5 | [atam-evaluation-v3.md §5 Non-Risks, §7](references/atam-evaluation-v3.md) | NR-1–NR-7 confirmed; R-1 (Witschi), R-2 (ring buffer stress), R-3 (ADR-004 inactive) remain open |

**Watch out**: TP-4 claims Measurement struct was split into 3 immutable Value Objects — be ready to name them specifically (Measurement, SignalFrame, WatchMetrics) when asked.

---

## Area 4 — Performance, Latency, and Correctness (25 pts)

| Sub-criterion | Pts | Evidence | Notes |
|---|:---:|---|---|
| Demonstrates real-time performance on the Raspberry Pi | 8 | [exp-02-realtime-dropped-block.md](references/experiments/exp-02-realtime-dropped-block.md) | RPi 5 @ 96 kHz: Dropped Blocks = 0 across all 9 runs; exec avg 9.6 ms < 21.3 ms deadline |
| Demonstrates low latency from signal capture to display/update | 6 | [exp-03-latency-e2e.md](references/experiments/exp-03-latency-e2e.md) | E2E avg **2.2 ms** / max **4.8 ms** (E3-07); before optimization: 255 ms → 97% reduction |
| Demonstrates correctness of calculations, event detection, and displayed values | 6 | [exp-06-accuracy-witschi-comparison.md](references/experiments/exp-06-accuracy-witschi-comparison.md), [exp-05-correctness-detector-optimization.md](references/experiments/exp-05-correctness-detector-optimization.md) | Witschi comparison: Rate Δ0.4 s/d ✅, Amp Δ15° ✅, Beat Error Δ0.1 ms ✅; detector onset=0.08 stable at 60 dB SNR |
| Presents evidence, measurements, experiments, or observations supporting these claims | 5 | All EXPs above + [thread-latency-chart.png](assets/thread-latency-chart.png) | CSV logs, before/after plots, 7-run latency history, 274-run detector grid |

**Watch out**:
- EXP-01 used only **1 round** (plan called for ≥ 3). Prepare explanation: same BPH and rate direction confirmed on both devices; sequential transfer < 5 min minimizes drift.
- Rate delta 0.4 s/d slightly exceeds the 0.3 s/d tolerance written in the Pass Condition. Reframe against the corrected ±2 s/d tolerance or explain the acoustic sensor coupling gap.

---

## Area 5 — Extensibility of the Architecture (20 pts)

| Sub-criterion | Pts | Evidence | Notes |
|---|:---:|---|---|
| Architecture is modular and separates major concerns clearly | 6 | [view-layered-4layer.md](references/views/view-layered-4layer.md) | 4-layer structure; DSM verified — all dependencies in lower triangle, zero violations |
| Architecture supports adding new measurements, filters, graphs, or displays with limited redesign | 6 | [exp-04-extensibility-observer-pattern.md](references/experiments/exp-04-extensibility-observer-pattern.md) | All 14 tabs added with ≤ 3 file changes; NetworkWorker prototype: ≤ 2 files; DSM: 0 layer violations |
| Team explains how the structure supports future requirements or enhancements | 4 | [view-layered-4layer.md §Modifiability Tactics](references/views/view-layered-4layer.md) | Bass/CMK Ch.8 "Restrict Dependencies" tactic cited; change cost table: new tab ≤3, new audio source 1–2, new formula 1 |
| Code organization and interfaces make the system understandable and maintainable | 4 | [view-iaudiosource.md](references/views/view-iaudiosource.md), [view-layered-4layer.md §Testability](references/views/view-layered-4layer.md) | IAudioSource AS-IS vs TO-BE; 142 unit tests across 10 binaries as structural testability evidence |

**Watch out**: RadarChartTab addition required modifying SequenceTab directly (footnote ¹ in EXP-04) — a boundary case of the ≤ 3-file rule. Prepare explanation: RadarChartTab reads per-position data from SequenceTab as a data provider, not a layer dependency violation.

---

## Predicted Score Summary

| Area | Max | Predicted | Risk |
|------|:---:|:---------:|------|
| 3 — Quality Attribute Tradeoff | 20 | 18–20 | Low — documentation is thorough |
| 4 — Performance, Latency, Correctness | 25 | 20–23 | Medium — EXP-01 single round; Rate tolerance gap |
| 5 — Extensibility | 20 | 17–19 | Low — RadarChartTab boundary case needs explanation |
| **Total (Areas 3+4+5)** | **65** | **55–62** | |

---

## Related Documents

- [ATAM Evaluation v3](references/atam-evaluation-v3.md)
- [QA Scenarios](references/qa/README.md)
- [Experiments](references/experiments/)
- [Architecture Views](references/views/README.md)
- ADRs: [001](references/adr/ADR-001-t2-dsp-offload-thread.md) · [002](references/adr/ADR-002-r1-lazy-rendering.md) · [003](references/adr/ADR-003-sample-rate-selection.md) · [005](references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) · [006](references/adr/ADR-006-basegraphtab-observer-pattern.md) · [008](references/adr/ADR-008-watchmath-module-isolation.md) · [009](references/adr/ADR-009-filterchain-design.md)
