# QAS-1: Real Time Performance — Priority 1

> M1 status: Provisional (pending EXP-01). M2 status: ✅ RPi confirmed 06/15.

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
