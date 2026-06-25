# Quality Attribute Requirements

**Team**: Blue Sky (Team 3) | **Milestone**: M3 (Final) | **Date**: 2026-06-25

---

## Accuracy as the Governing Project Goal

Accuracy — producing Rate, Amplitude, and Beat Error values that faithfully represent the watch's true mechanical behavior — is the overarching objective of this project, not a single architectural QA scenario.

Accuracy is not directly achievable through one architectural decision. Instead, it is delivered by the combined effect of multiple structural QAs, each of which removes a failure mode that would otherwise corrupt the output:

| QA | How it serves Accuracy |
|----|----------------------|
| QAS-1 Real-Time Performance | Dropped audio blocks cause missed beats → wrong Rate and Beat Error |
| QAS-2 Low Latency | Stale display values mislead the user about current watch state |
| QAS-4 Correctness (Sub-1) | Formula errors in WatchMath produce systematically wrong values |
| QAS-4 Correctness (Sub-3) | Noise-triggered false beats corrupt Rate and Beat Error statistics |
| ADR-003 96kHz Sample Rate | Higher timing resolution reduces quantization error in Beat Error computation |

When architectural decisions conflicted with other QAs, accuracy concerns broke the tie:
- **Performance cost vs. Accuracy**: 96kHz adopted over 48kHz despite higher CPU/memory cost (ADR-003)
- **Complexity vs. Accuracy**: WatchMath isolated as a pure calculation module to enable formula-level unit testing (QAS-4 Sub-1)
- **Usability vs. Accuracy**: FilterChain parameters exposed to the user so the signal can be tuned for each environment rather than locked to a default that may degrade measurement quality

QAS-5 captures the user-observable verification of this goal. Its measure (Δ Rate < 0.3 s/d vs. WeiShi No.1000) is an acceptance criterion, not an architectural driver.

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
| **4** | [Long-Term Session Performance](qas-6-long-term-session-performance.md) | Time-Based Bucket Downsampling (`mBucketSize`) | [ADR-007](../adr/ADR-007-longtermtab-downsampling.md) | [EXP-06](../experiments/exp-06-longterm-aging.md) | [Decomposition View: LongTermTab](../views/view-longtermtab-downsampling.md) |
