# QAS-2: Real Time Performance — Priority 2

> M1 status: Provisional (pending EXP-01). M2 status: macOS confirmed; RPi confirmed 06/15.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH; each 2048-sample audio block arrives from ALSA |
| **Artifact** | Audio pipeline — AudioCapture → DSPWorker → MeasurementEngine |
| **Environment** | Raspberry Pi 5, 96kHz sample rate, Qt GUI running concurrently |
| **Response** | Every audio block is captured, processed, and a beat measurement emitted without skipping any block |
| **Measure** | Deadline miss = 0%; dropped blocks = 0 over a 10-minute session at 96kHz |

On RPi with ALSA, the block period is fixed at ~21ms regardless of sample rate. At 96kHz, ALSA delivers 2048 samples per block (2048 / 96,000 = 21.33ms). Every audio block must complete DSP processing before the next block arrives — this 21ms window is the real-time deadline.

## Related

- [QA Priority Summary](README.md)
- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — threading strategy options and trade-offs
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — rendering strategy options and trade-offs
- [EXP-02: Pipeline Latency](../experiments/exp-02-realtime-deadline-compliance.md) — confirmed 43% baseline miss; 0% after T2+R1
