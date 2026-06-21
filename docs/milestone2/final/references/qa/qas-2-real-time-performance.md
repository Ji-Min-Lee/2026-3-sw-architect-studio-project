# QAS-2: Real Time Performance — Priority 2

> M1 status: Provisional (pending EXP-02). M2 status: ✅ RPi confirmed 06/15.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | Continuous acoustic beat signal at 28,800 BPH; each 2048-sample audio block arrives from ALSA |
| **Artifact** | Audio pipeline — AudioCapture → DSPWorker → MeasurementEngine |
| **Environment** | Raspberry Pi 5, 96kHz sample rate, Qt GUI running concurrently |
| **Response** | Every audio block is captured, processed, and a beat measurement emitted without skipping any block |
| **Measure** | Deadline miss = 0%; dropped blocks = 0 over a 10-minute session at 96kHz |

## Real-Time Deadline

The real-time deadline is set by ALSA: ~21ms per block on RPi. Every audio block must complete DSP processing before the next block arrives. The sample rate choice affects Beat Error resolution, not the deadline itself.

## Related

- [QA Priority Summary](README.md)
- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — threading strategy options and trade-offs
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — rendering strategy options and trade-offs
- [EXP-02: RPi Real-Time Performance — Dropped Block Measurement](../experiments/exp-02-realtime-dropped-block.md) — Dropped Block = 0 at 96k sps confirmed
