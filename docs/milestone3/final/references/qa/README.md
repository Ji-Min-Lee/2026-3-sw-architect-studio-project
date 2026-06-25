# Quality Attribute Requirements

**Team**: Blue Sky (Team 3) | **Milestone**: M3 (Final) | **Date**: 2026-06-25

---

## Priority Summary

| Priority | QA | Architecture | Rationale | Experiment | View |
|:--------:|----|---|---|---|---|
| **1** | [Real Time Performance](qas-1-real-time-performance.md) | T2 DSP Offload Thread | [ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md) | [EXP-02](../experiments/exp-02-realtime-dropped-block.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **1** | [Real Time Performance](qas-1-real-time-performance.md) | R1 Lazy Rendering | [ADR-002](../adr/ADR-002-r1-lazy-rendering.md) | [EXP-02](../experiments/exp-02-realtime-dropped-block.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **2** | [Low Latency and Low Number of Missed Beats](qas-2-low-latency-and-low-number-of-missed-beats.md) | T2 DSP Offload Thread | [ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md) | [EXP-03](../experiments/exp-03-latency-e2e.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **3** | [Extensibility, Modifiability](qas-3-extensibility-modifiability.md) | 4-Layer Allowed-to-Use | [ADR-006](../adr/ADR-006-basegraphtab-observer-pattern.md) | [EXP-04](../experiments/exp-04-extensibility-observer-pattern.md) | [Layered View: 4-Layer](../views/view-layered-4layer.md) |
| **3** | [Extensibility, Modifiability](qas-3-extensibility-modifiability.md) | Observer Pattern (BaseGraphTab) | [ADR-006](../adr/ADR-006-basegraphtab-observer-pattern.md) | [EXP-04](../experiments/exp-04-extensibility-observer-pattern.md) | [Decomposition View: Graph Tab](../views/view-decomposition-graph-tab.md) |
| **4** | [Correctness](qas-4-correctness.md) | Sample Rate Selection | [ADR-003](../adr/ADR-003-sample-rate-selection.md) | [EXP-05](../experiments/exp-05-correctness-detector-optimization.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **5** | [Measurement Accuracy, Error Detection, and Handling](qas-5-measurement-accuracy-error-detection-handling.md) | — | — | [EXP-01](../experiments/exp-01-accuracy-weishi-comparison.md) | [Domain Entity / Value Object](../views/view-domain-entity-vo.md) |
