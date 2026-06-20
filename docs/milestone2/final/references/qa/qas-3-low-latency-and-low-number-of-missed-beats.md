# QAS-3: Low Latency and Low Number of Missed Beats — Priority 3

> M1 status: Provisional (pending EXP-02). M2 status: macOS segment ① confirmed; RPi confirmed 06/15.

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | A single beat event (tick) occurs at the microphone |
| **Artifact** | Full pipeline — from AudioCapture callback to GUI display update |
| **Environment** | Raspberry Pi 5, normal operating conditions, Qt GUI active |
| **Response** | Corresponding waveform, beat markers, and computed values appear in the GUI |
| **Measure** | ① capture→detect < 20.8ms (one beat period at 28,800 BPH); ③ E2E < 100ms |

## Latency Segment Breakdown

| Segment | Boundary | Target |
|---------|----------|:------:|
| ① capture→process | ALSA callback → T1/T3 timestamp | < 20.8ms |
| ② process→display | T1/T3 timestamp → `paintEvent()` complete | < 30ms |
| ③ end-to-end | ALSA callback → screen update | < 100ms |

## Latency Targets by BPH

| BPH | Beat Period | E2E Target (80% margin) | ① capture→process | ② process→display |
|:---:|:-----------:|:-----------------------:|:-----------------:|:-----------------:|
| 28,800 | 125 ms | **< 100 ms** (Primary) | < 70 ms | < 30 ms |
| 36,000 | 100 ms | < 80 ms | < 56 ms | < 24 ms |
| 43,200 | 83 ms | < 66 ms | < 46 ms | < 20 ms |

## Related

- [QA Priority Summary](README.md)
- [Architectural Approaches](../approaches.md) — tactics and design decisions addressing this QA
- [EXP-02: Pipeline Latency](../experiments/exp-02-pipeline-latency.md) — segment ① wait_ms 420ms → 0.013ms after T2
