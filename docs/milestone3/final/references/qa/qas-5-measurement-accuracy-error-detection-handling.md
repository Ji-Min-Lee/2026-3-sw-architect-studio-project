# QAS-5: Measurement Accuracy, Error Detection, and Handling — Priority 5 (Usability)

> This QA did not exist explicitly in M1. Promoted from "Correctness (QAS-3 M1)" to Priority 1 / governing goal in M2. Reclassified in M3 to **Priority 5 / Usability** — it is the user-facing outcome that structural QAs 1–4 collectively enable.

**Note on scope:** "Accuracy" is the overarching project goal, not a standalone architectural QA. The structural QAs (QAS-1 through QAS-4) each represent an architectural decision made *in service of* that goal. This scenario captures the user-observable result: can the user trust the displayed values? Verification is empirical (EXP-01), not structural.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch under measurement |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH input to microphone |
| **Artifact** | Full pipeline — AudioCapture → DSP → MeasurementEngine → all graph displays |
| **Environment** | Raspberry Pi 5, Live mode, 96kHz sample rate, normal operating temperature |
| **Response** | Rate (s/d), Amplitude (°), Beat Error (ms), BPH computed and displayed in all views |
| **Measure** | Δ Rate < 0.3 s/d vs. WeiShi No.1000; Δ Amplitude < 0.01°; Δ Beat Error = 0 ms; deviation = 0 across all graph tabs showing the same metric (see EXP-01 for protocol) |

## Trade-off Accepted

BPH coverage narrowed to 28,800 BPH for M3. Full range (18,000–36,000 BPH) is an accuracy stretch goal, not a structural constraint.

## Related

- [QA Priority Summary](README.md)
- [ADR-003: Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md) — 96kHz chosen over 48kHz to improve timing resolution, directly enabling the Δ Beat Error tolerance above
- [EXP-01: WeiShi Accuracy Comparison](../experiments/exp-01-accuracy-weishi-comparison.md) — dual-system comparison protocol and pass/fail criteria
- [EXP-05: Detector Parameter Optimization](../experiments/exp-05-correctness-detector-optimization.md)
