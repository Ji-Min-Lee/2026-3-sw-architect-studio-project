# QAS-1: Measurement Accuracy, Error Detection, and Handling — Priority 1 (Governing Goal)

> This QA did not exist explicitly in M1. Promoted from "Correctness (QAS-3 M1)" to Priority 1 / governing goal in M2 after recognizing that every other QA exists only to make this one achievable.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch under measurement |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH input to microphone |
| **Artifact** | Full pipeline — AudioCapture → DSP → MeasurementEngine → all graph displays |
| **Environment** | Raspberry Pi 5, Live mode, 96kHz sample rate, normal operating temperature |
| **Response** | Rate (s/d), Amplitude (°), Beat Error (ms), BPH computed and displayed in all views |
| **Measure** | Values match WeiShi No.1000 reference within tolerance; deviation = 0 across all graph tabs showing the same metric |

## Trade-off Accepted

BPH coverage narrowed to 28,800 BPH for M3. Full range (18,000–36,000 BPH) is an accuracy stretch goal, not a structural constraint.

## Related

- [QA Priority Summary](README.md)
- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md)
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md)
- [ADR-003: Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md)
- [EXP-02: Pipeline Latency](../experiments/exp-02-pipeline-latency.md)
- [EXP-03: Filter Sweep](../experiments/exp-03-filter-sweep.md)
