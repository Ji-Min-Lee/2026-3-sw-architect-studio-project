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

Rejected alternative — Option T1 (SCHED_RR + CPU Affinity only): improves OS scheduling jitter
but does not reduce exec_ms or eliminate the single-core saturation. T1 may be added on top
of T2 as a supplementary measure (EXP-02 R6) but cannot replace it.

Rejected alternative — Option T3 (Full Pipeline Thread Split): provides higher parallelism
but introduces three inter-stage queues and significantly higher design complexity.
M2 deadline risk rated High. Deferred to post-M3 consideration.

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
