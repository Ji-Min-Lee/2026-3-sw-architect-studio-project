# QA Strategy: Low Latency

## Requirement

The system shall minimize end-to-end latency between acoustic capture at the microphone
and presentation of the corresponding waveform, markers, and computed values in the GUI.

Latency is measured at three points:

| Point | Description |
|-------|-------------|
| (1) Capture | Audio sample block is captured from microphone |
| (2) Process | Block is processed for beat detection and measurement |
| (3) Display | Waveform segment and computed readings appear in GUI |

Teams shall report capture-to-process latency, process-to-display latency, and total
end-to-end latency in milliseconds, including average and worst-case values, as well as
counts of dropped audio blocks and missed beat detections.

**Reference**: [QAS-3: Low Latency and Low Number of Missed Beats](../../milestone2/final/references/qa/qas-3-low-latency-and-low-number-of-missed-beats.md)

---

## Architectural Decision: Thread Separation with Timestamp Measurement Points at Each Stage Boundary

### Rationale

Latency cannot be reduced without first knowing where it originates. Each stage boundary
(capture → process, process → display) must record a timestamp so that per-stage latency
is observable. This measurement infrastructure must be part of the architecture, not an
afterthought, because:

- Adding timestamps after the fact requires modifying thread boundaries and data structures.
- Worst-case latency (not just average) is needed to detect backlog and stale data; this
  requires per-block timestamps to be carried through the pipeline.
- Dropped block counts and missed beat counts are only accurate if tracked at the point
  of handoff between threads.

Thread separation (shared with the Real-Time Performance decision) is a prerequisite:
without separated threads, the stage boundaries where timestamps are inserted do not exist
as distinct architectural points.

### Trade-offs

| | No measurement points | Timestamp at each boundary (chosen) |
|--|----------------------|-------------------------------------|
| Observability | Latency is invisible | Per-stage and end-to-end latency visible |
| Overhead | None | Small per-block timestamp cost |
| Debugging | Guesswork | Bottleneck stage identifiable directly |
| Implementation cost | None | Requires carrying timestamp metadata through pipeline |

### Alternatives Considered

**Measure latency only at the end (capture timestamp vs. display timestamp)**
- Simpler: only two timestamps per block.
- Not applied because: end-to-end measurement alone does not identify which stage is the
  bottleneck. The requirement explicitly calls for capture-to-process and process-to-display
  latency reported separately.

**External profiling tools (e.g., perf, Qt Profiler)**
- Can measure latency without modifying the code.
- Not applied because: these tools are not available in the production runtime on Raspberry
  Pi 5 and cannot produce per-beat latency logs needed for the demo evidence.

### ADRs

| ADR | Decision | Status |
|-----|----------|--------|
| [ADR-001: DSP Offload Thread](../../milestone2/final/references/adr/ADR-001-t2-dsp-offload-thread.md) | Dedicate a separate thread for DSP — eliminates the primary source of backlog latency | ✅ Applied |
| [ADR-002: Lazy Rendering](../../milestone2/final/references/adr/ADR-002-r1-lazy-rendering.md) | Skip replot() for non-visible tabs — reduces process-to-display latency | ✅ Applied |
| [ADR-004: Timer-Decoupled Rendering](../../milestone2/final/references/adr/ADR-004-r2-timer-decoupled-rendering.md) | Decouple rendering from beat events — bounds worst-case display latency | ✅ Applied |

### View

**C&C View** — same thread diagram as Real-Time Performance, annotated with timestamp
insertion points at each queue handoff and the latency metrics derived from them.

- [C&C View: DSP Pipeline Thread Model](../../milestone2/final/references/views/view-cc-dsp-pipeline.md) ✅

---

## Experiments

| ID | Description | Status |
|----|-------------|--------|
| [EXP-03](../../milestone2/final/references/experiments/exp-03-latency-e2e.md) | End-to-End Latency — 2-Segment Timestamp Measurement | ✅ Done (2026-06-11 ~ 2026-06-16) |

---

## Verification

- Metric: capture-to-process, process-to-display, end-to-end latency (avg + worst-case) in ms
- Metric: dropped audio block count, missed beat detection count per session
- Evidence: EXP-03 results presented in demo
