# QAS-5: Measurement Accuracy, Error Detection, and Handling — Priority 5 (Usability)

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch under measurement |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH input to microphone |
| **Artifact** | Full pipeline — AudioCapture → DSP → MeasurementEngine → all graph displays |
| **Environment** | Raspberry Pi 5, Live mode, 96kHz sample rate, normal operating temperature |
| **Response** | Rate (s/d), Amplitude (°), Beat Error (ms), BPH computed and displayed in all views |
| **Measure** | Δ Rate < 0.3 s/d vs. Witschi No.1000; Δ Amplitude < 0.01°; Δ Beat Error = 0 ms; deviation = 0 across all graph tabs showing the same metric (see EXP-06 for protocol) |

**Note on scope:** "Accuracy" is the overarching project goal, not a standalone architectural QA. The structural QAs (QAS-1 through QAS-4) each represent an architectural decision made *in service of* that goal. This scenario captures the user-observable result: can the user trust the displayed values? Verification is empirical (EXP-06), not structural.