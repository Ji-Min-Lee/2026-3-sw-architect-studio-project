# Quality Attribute Requirements

**Team**: Blue Sky (Team 3) | **Milestone**: M3 (Final) | **Date**: 2026-06-25

---

## Priority Summary

| Priority | QA | Architecture Decision | Experiment | View |
|:--------:|----|---|---|---|
| **H** | [Real Time Performance](qas-1-real-time-performance.md) | [ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread, [ADR-002](../adr/ADR-002-r1-lazy-rendering.md) R1 Lazy Rendering | [EXP-01](../experiments/exp-01-realtime-dropped-block.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **H** | [Low Latency and Low Number of Missed Beats](qas-2-low-latency-and-low-number-of-missed-beats.md) | [ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread | [EXP-02](../experiments/exp-02-latency-e2e.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **M** | [Extensibility, Modifiability](qas-3-extensibility-modifiability.md) | [ADR-006](../adr/ADR-006-basegraphtab-observer-pattern.md) Observer Pattern, Layered and Module Decomposition | [EXP-03](../experiments/exp-03-extensibility-observer-pattern.md) | [Layered and Module Decomposition View](../views/view-layered-4layer.md), [Graph Tab Module Uses View](../views/view-decomposition-graph-tab.md) |
| **M** | [Correctness](qas-4-correctness.md) | [ADR-003](../adr/ADR-003-sample-rate-selection.md) Sample Rate, [ADR-005](../adr/ADR-005-p1-iaudiosource-dependency-inversion.md) IAudioSource Consistency, [ADR-008](../adr/ADR-008-watchmath-module-isolation.md) WatchMath Isolation | [EXP-04](../experiments/exp-04-correctness-detector-optimization.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md), [IAudioSource View](../views/view-iaudiosource.md) |
| **M** | [Measurement Accuracy, Error Detection, and Handling](qas-5-measurement-accuracy-error-detection-handling.md) | [ADR-003](../adr/ADR-003-sample-rate-selection.md) Sample Rate Selection | [EXP-06](../experiments/exp-06-accuracy-witschi-comparison.md) | [C&C View: DSP Pipeline](../views/view-cc-dsp-pipeline.md) |
| **L** | [Long-Term Session Performance](qas-6-long-term-session-performance.md) | [ADR-007](../adr/ADR-007-longtermtab-downsampling.md) Time-Based Bucket Downsampling | [EXP-07](../experiments/exp-07-longterm-aging.md) | [Decomposition View: LongTermTab](../views/view-longtermtab-downsampling.md) |

---

## Accuracy as the Governing Project Goal

Accuracy — producing Rate, Amplitude, and Beat Error values that faithfully represent the watch's true mechanical behavior — is the overarching objective of this project, not a single architectural QA scenario.

Accuracy is not directly achievable through one architectural decision. In Bass, Clements & Kazman terminology (*Software Architecture in Practice*, Ch.3), Real-Time Performance, Low Latency, and Correctness are **enabling QAs** for Accuracy: each one removes a failure mode that would otherwise corrupt the output. Extensibility is an independent architectural driver.

```
Measurement Accuracy (governing goal — QAS-5)
├── Real-Time Performance (QAS-1)   ← dropped audio block → missed beat → wrong Rate / Beat Error
│     ADR-001 (DSP Offload Thread), ADR-002 (Lazy Rendering), EXP-01
├── Low Latency (QAS-2)             ← stale display value → user misreads current watch state
│     ADR-001, EXP-02
└── Correctness (QAS-4)
      ├── Sub-1 Testability          ← formula error in WatchMath → systematically wrong values
      │     ADR-008, EXP-04
      └── Sub-3 Noise Resilience     ← false beat trigger → corrupted Rate / Beat Error statistics
            ADR-003, ADR-009, EXP-04

Extensibility / Modifiability (QAS-3) — independent architectural driver
      ADR-005, ADR-006, EXP-03
```

QAS-5 is the **acceptance criterion**: Δ Rate < ±2 s/d vs. Witschi No.1000 confirms that all enabling QAs are working together correctly. It is a verification step, not an architectural driver.

### Enabling QA Summary

| Enabling QA | Failure mode prevented | Architecture decision |
|-------------|----------------------|-----------------------|
| QAS-1 Real-Time Performance | Dropped audio block → missed beat → wrong Rate and Beat Error | ADR-001, ADR-002 |
| QAS-2 Low Latency | Stale display value → user misreads current watch state | ADR-001 |
| QAS-4 Sub-1 Testability | Formula error in WatchMath → systematically wrong values | ADR-008 |
| QAS-4 Sub-3 Noise Resilience | False beat trigger → corrupted Rate / Beat Error statistics | ADR-003, ADR-009 |
| ADR-003 96kHz Sample Rate | Quantization error in Beat Error computation too coarse at 48kHz | ADR-003 |

### Accuracy Broke the Tie

When architectural decisions conflicted with other QAs, accuracy concerns resolved the tradeoff:

| Decision | Conflicting QA | Accuracy rationale |
|----------|---------------|--------------------|
| 96kHz over 48kHz | Performance (higher CPU/memory cost) | Beat Error resolution: 0.01ms at 96kHz vs 0.02ms at 48kHz — halving resolution is unacceptable (ADR-003) |
| WatchMath module isolation | Simplicity (extra abstraction layer) | Formula-level unit testing impossible without isolation; undetected formula error directly corrupts output (ADR-008) |
| FilterChain parameters exposed to user | Usability (more complex UI) | Default parameters that degrade measurement quality in one environment must be tunable per deployment (ADR-009) |
