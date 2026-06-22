# QA Strategy: Real-Time Performance

## Requirement

The system shall acquire, process, analyze, and display watch acoustic data in real time on
Raspberry Pi 5 while maintaining a responsive GUI.

| Target | Sample Rate |
|--------|-------------|
| Minimum | 48,000 sps |
| Objective | 96,000 sps |
| Stretch | 192,000 sps |

**Reference**: [QAS-2: Real-Time Performance](../../milestone2/final/references/qa/qas-2-real-time-performance.md)

---

## Architectural Decision: Separate Audio Capture, Signal Processing, and GUI Rendering into Independent Threads

### Rationale

The three pipeline stages — audio capture, signal processing, and GUI rendering — operate at
different speeds and have different timing constraints:

- **Audio capture** must run at a fixed sample rate without interruption. Any delay causes
  dropped samples that cannot be recovered.
- **Signal processing** is CPU-intensive and variable in duration depending on algorithm
  complexity and sample rate.
- **GUI rendering** is driven by Qt's event loop and can block unpredictably on layout,
  painting, or user interaction.

If these stages share a single thread, a slow GUI frame or a heavy processing burst will
stall audio capture, causing missed beats and incorrect measurements. Thread separation
ensures each stage runs at its own pace without blocking the others.

### Trade-offs

| | Single Thread | Multi-Thread (chosen) |
|--|--------------|----------------------|
| Throughput | Limited by slowest stage | Each stage runs at full speed |
| Complexity | Simple | Requires thread-safe queues and synchronization |
| Dropped samples | Likely at high sps | Avoidable with sufficient queue depth |
| Debugging | Straightforward | Harder to reproduce race conditions |

### Alternatives Considered

**Single-thread with Qt timer callbacks**
- Qt's `QTimer` can schedule periodic processing without explicit threads.
- Not applied because: at 96,000+ sps the audio callback interval is too short (~10 ms per
  block). Qt's event loop cannot guarantee this latency, especially when GUI painting is
  in progress. Dropped blocks are likely at the target sample rate on Raspberry Pi 5.
  See [ADR-001](../../milestone2/final/references/adr/ADR-001-t2-dsp-offload-thread.md) for
  measured evidence (47% QueuedConnection backlog, 420ms wait_ms on single-thread baseline).

### ADRs

| ADR | Decision | Status |
|-----|----------|--------|
| [ADR-001: DSP Offload Thread](../../milestone2/final/references/adr/ADR-001-t2-dsp-offload-thread.md) | Dedicate a separate thread for DSP processing | ✅ Applied |
| [ADR-002: Lazy Rendering](../../milestone2/final/references/adr/ADR-002-r1-lazy-rendering.md) | Skip replot() for non-visible tabs | ✅ Applied |
| [ADR-003: Sample Rate Selection](../../milestone2/final/references/adr/ADR-003-sample-rate-selection.md) | Select audio sample rate balancing accuracy and CPU budget | ✅ Applied |
| [ADR-004: Timer-Decoupled Rendering](../../milestone2/final/references/adr/ADR-004-r2-timer-decoupled-rendering.md) | Decouple rendering from beat events via fixed-interval timer | ✅ Applied |

### View

**C&C View** — shows the three runtime threads, the queues between them, and the data flow
from raw samples to computed measurements to display.

- [C&C View: DSP Pipeline Thread Model](../../milestone2/final/references/views/view-cc-dsp-pipeline.md) ✅

---

## Experiments

| ID | Description | Status |
|----|-------------|--------|
| [EXP-02](../../milestone2/final/references/experiments/exp-02-realtime-dropped-block.md) | RPi Real-Time Performance — Dropped Block Measurement | ✅ Done (2026-06-15) |

---

## Verification

- Metric: sustained FPS + processing time logged per block at 48k / 96k / 192k sps
- Pass condition: zero dropped audio blocks at 96,000 sps under normal load on Raspberry Pi 5
- Evidence: EXP-02 results
