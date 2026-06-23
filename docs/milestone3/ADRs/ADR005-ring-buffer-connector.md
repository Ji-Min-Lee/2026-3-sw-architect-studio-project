# ADR 005: Use a Ring Buffer as the Thread Boundary Connector Between Audio Capture and DSP

With the introduction of a dedicated DSP Thread ([ADR 001](ADR001-dsp-offload-thread.md)), the Audio Thread (producer) and DSP Thread (consumer) must exchange PCM blocks across a thread boundary without either thread blocking the other. A naive mutex-protected queue would cause the Audio Thread to stall if the DSP Thread is momentarily slow — which is precisely the type of blocking that caused the original backlog.

## Decision

Use a **thread-safe ring buffer** (`SignalBuffer`) as the sole connector between `AudioCapture` (Audio Thread) and `DSPWorker` (DSP Thread).

Properties:
- **Non-blocking write**: AudioCapture writes a PCM block and returns immediately; if the buffer is full, the oldest block is overwritten (drop-oldest policy).
- **Blocking read**: DSPWorker waits until at least one block is available, then processes it.
- **Fixed capacity**: Buffer depth is sized to cover worst-case DSP processing time, preventing unbounded memory growth.

## Rationale

A ring buffer with non-blocking write is the idiomatic solution for audio pipelines. Audio capture interrupts have a hard deadline — they must complete before the next hardware interrupt or samples are dropped. Blocking the Audio Thread on a full buffer violates this deadline.

The drop-oldest policy is acceptable: if DSP falls behind (e.g., during a thermal throttle burst), discarding stale audio blocks is preferable to stopping the capture stream. A single missed block causes at most one missed beat; a stopped stream causes measurement loss for the entire duration of the stoppage.

| Alternative | Problem |
|-------------|---------|
| Mutex-protected `std::deque` | Write blocks if deque is full → Audio Thread misses ALSA interrupt → buffer underrun |
| Lock-free atomic queue (SPSC) | Correct and fast, but more complex to implement; ring buffer is sufficient for this throughput |
| Direct function call (no buffer) | Requires Audio Thread and DSP Thread to run synchronously — eliminates the concurrency benefit of ADR 001 |

## Status

Accepted. Implemented as `SignalBuffer` in the Signal Processing layer.

## Consequences

- **Positive**: Audio Thread is never blocked by DSP processing time; dropped audio blocks = 0 in normal operation (QAS-1).
- **Positive**: The ring buffer absorbs transient DSP delays (e.g., OS scheduling jitter, Qt event bursts) without propagating them to the audio path.
- **Positive**: Fixed buffer capacity bounds memory usage and makes latency predictable: max queuing latency = buffer_depth × block_duration.
- **Trade-off**: Drop-oldest policy means a severe DSP stall (e.g., sustained thermal throttle) can cause missed beats. Mitigated by thermal management (heatsink on RPi) and T1 SCHED_RR CPU affinity as a complementary tactic.
- **Trade-off**: Buffer capacity must be tuned: too small → frequent drops under normal jitter; too large → increases end-to-end latency (QAS-3). Default 4–8 blocks at 96 kHz adds < 10 ms of queuing latency — within the QAS-3 budget.

## Related ADRs
- [ADR 001 — DSP Offload Thread](ADR001-dsp-offload-thread.md)
- [ADR 004 — Qt as Application Framework](ADR004-qt-framework.md)
