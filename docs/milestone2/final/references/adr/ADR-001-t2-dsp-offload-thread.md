# ADR-001: Introduce a Dedicated DSP Offload Thread (T2)

The original audio pipeline runs `AudioCapture → FilterChain → BeatDetector → MeasurementEngine`
entirely on the Qt main thread (cpu2). EXP-02 R1 measured a Qt `QueuedConnection` backlog of 47%
and a sustained wait_ms of 420ms on macOS — meaning the DSP queue accumulates faster than
it is drained. On Raspberry Pi 5, cpu2 reached 91% load with 43% of frames exceeding the
21ms exec deadline. The remaining three cores were idle.

The root cause is structural: a single thread serializes audio I/O, signal processing,
and the Qt event loop. Thermal throttling (85°C on RPi) compounds the problem by further
reducing effective clock speed.

## Decision

We will introduce a dedicated `DSPWorker` thread that runs the signal processing pipeline
(`FilterChain → BeatDetector → MeasurementEngine`) independently of the audio capture callback.

`AudioCapture` writes raw PCM blocks into a thread-safe ring buffer immediately on callback return.
`DSPWorker` reads from the ring buffer on a separate OS thread and emits measurement results
via `Qt::QueuedConnection` to the UI thread.

```
Audio Capture Thread (POSIX, lightweight):
  AudioCapture → [PCM Ring Buffer write]

DSP Worker Thread (new):
  [PCM Ring Buffer read] → FilterChain → BeatDetector → MeasurementEngine
                                                              ↓ Qt::QueuedConnection

UI Thread (Qt Main):
  GraphTabManager → IGraphTab::updateData()
```

## Rationale

EXP-02 Run R2 (macOS, 96kHz Playback mode, T2 applied) produced the following results:

| Metric | Before (R1) | After T2 (R2) | Change |
|--------|:-----------:|:-------------:|:------:|
| wait_ms avg | 420 ms | 0.013 ms | ×32,000 reduction |
| backlog | 47% | 0% | fully eliminated |
| bg_fps ≈ fg_fps | — | 95.6 ≈ 95.6 | DSP tracks worker in real time |

The wait_ms reduction confirms that the Qt event loop was the congestion point, not the DSP
computation itself (exec_ms was already 0.57ms before T2). Separating capture from DSP
eliminates the backlog and distributes load across cores.

### Options Considered

| Item | A1: SCHED_RR + CPU Affinity | A2: DSP Offload Thread | A3: Full Pipeline Split |
|------|-----------------------------|------------------------|-------------------------|
| Core tactic | OS real-time scheduling + core pinning | Separate capture from DSP onto two threads | One thread per pipeline stage |
| Core distribution | Partial (capture core isolated) | Medium (capture + DSP on separate cores) | High (stage-per-core) |
| Implementation complexity | Low (OS API only) | Medium | High |
| Direct exec reduction | None (scheduling improvement only) | Medium | High |
| Module structure change | None | Ring buffer added | Full pipeline redesign |
| M2 feasibility | ✅ Immediate | ✅ Feasible | ⚠️ High risk |

**A1 rejected**: Improves OS scheduling jitter but does not reduce exec_ms or eliminate single-core saturation. [EXP-01](../experiments/exp-02-realtime-dropped-block.md) confirmed SCHED_RR did not reduce processing time — frames over deadline stayed at ~8% across all scheduling policies (default: 8.1%, SCHED_RR: 8.4%, SCHED_FIFO: 8.6%). The problem is not scheduling priority; one thread simply has too much work to do. A1 applied to the audio capture thread is not required. May be added on top of A2 as a supplementary measure for the FG thread (TR-10) but cannot replace A2.

**A3 rejected**: Provides higher parallelism but introduces three inter-stage queues and significantly higher design complexity. M2 deadline risk rated High. Deferred to post-M3 consideration.

### Combined Trade-off Matrix (Threading × Rendering)

| Combo | exec reduction | Core distribution | Complexity | M2 risk | Recommendation |
|:-----:|:--------------:|:-----------------:|:----------:|:-------:|----------------|
| **A2 + R1** | ★★★★☆ | ★★★★☆ | ★★★☆☆ | Medium | **Selected — balanced** |
| A2 + R2 | ★★★★★ | ★★★★☆ | ★★★★☆ | Medium | Structural completeness priority |
| A1 + R1 | ★★★★☆ | ★★☆☆☆ | ★☆☆☆☆ | Low | Fast MVP, weaker threading |
| A1 + R2 | ★★★☆☆ | ★★☆☆☆ | ★★☆☆☆ | Low | Render timing control priority |
| A3 + R3 | ★★★★★ | ★★★★★ | ★★★★★ | High | Post-M3 only |

Selected combination **A2 + R1** confirmed by EXP-02 on both macOS and RPi (E2-5/E2-6): E2E avg 2.05ms, 0 deadline miss, 0 backlog.

## Status

Accepted (2026-06-15, macOS validated)

RPi R5 confirmation scheduled: 2026-06-23.

## Consequences

**Positive**:
- wait_ms reduced ×32,000; frame backlog eliminated
- DSP and capture now distributed across at least two cores
- Thermal load expected to distribute, reducing throttle risk
- AudioCapture callback is lightweight — only ring buffer write; lowest latency path preserved

**Negative**:
- One additional thread to manage (lifecycle: start on pipeline open, join on close)
- PCM ring buffer adds a data structure and synchronization point (lock-free or mutex-guarded)
- SignalBuffer design must be clarified: existing buffer vs. new PCM ring buffer — risk of
  role overlap; resolved by scoping SignalBuffer to DSP-internal use and ring buffer to
  capture-to-DSP handoff only
- RPi validation still pending — macOS results establish confidence but do not substitute
  for target hardware measurement (EXP-02 R5)
