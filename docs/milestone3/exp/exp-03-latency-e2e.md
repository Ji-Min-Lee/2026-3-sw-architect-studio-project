# Experiment 3: End-to-End Latency — 2-Segment Timestamp Measurement

## Results and recommendations

**Outcome: ✅ Pass** — E2E avg 2.2 ms / max 4.8 ms after T2 offload thread + R1 Lazy Rendering.

The key fix was separating DSP into its own background thread (ADR-001), which reduced queue wait time from ~420 ms to 0.013 ms (×32,000). Lazy rendering (ADR-002) further tightened max tail latency.

**Open concern**: FG Qt event-loop pickup averages 60 ms (84% of beats exceed the 21.33 ms display-update budget). This is a render scheduling issue, not a DSP accuracy issue, and does not affect measurement correctness.

| Tactic | E2E avg/max | Improvement |
|--------|:-----------:|:-----------:|
| Baseline RPi (unoptimized) | 255.4 / 900.9 ms | — |
| RPi baseline (after plot fix) | 57.2 / 208.9 ms | — |
| +T2 DSP Offload | 2.1 / 11.1 ms | −97% |
| +R1 Lazy Rendering | 2.1 / 5.7 ms | tighter max |
| Final (E3-07, with FG measurement) | **2.2 / 4.8 ms** | ✅ QAS-2 Pass |

## Objective

Measure end-to-end latency from audio block ready to beat result delivered, identify the bottleneck causing deadline misses, and validate the T2 and R1 tactic decisions (ADR-001, ADR-002).

Technical question: What is the actual end-to-end latency path, where is the bottleneck, and do the implemented tactics bring it within the 30 ms QAS-2 threshold with 11 tabs?

## Status

Concluded

## Expected outcomes

- Per-segment latency breakdown (wait_ms / exec_ms) for each configuration
- Identification of the dominant bottleneck causing deadline misses
- Quantitative before/after comparison for each architectural tactic

## Resources required

- RPi 5 (rpi1, rpi2), macOS dev PC (Windows reference)
- TimeChecker built with 2-point timestamp instrumentation (`emit PlaybackDataReady` → `HandleInputData()` entry → exit)
- `src/tools/analyze_log.py` for latency statistics
- Log directory: `src/logs/EXP-02/`
- ~2 person-days

## Experiment description

**Measurement segments:**
- `wait_ms`: `emit PlaybackDataReady` → `MainWindow::HandleInputData()` entry (inter-thread queue wait)
- `exec_ms`: `HandleInputData()` entry → exit (DSP + render on FG thread)

**Runs (sequential, each building on the previous):**
1. E3-01: Windows reference (dev PC) — establish healthy baseline
2. E3-02: RPi rpi1 unoptimized — reproduce the reported latency problem
3. E3-03: RPi rpi2 baseline — isolate hardware vs. software effects
4. E3-04: + multi-tab configuration — measure tab scaling overhead
5. E3-05: + T2 DSP offload thread — validate ADR-001
6. E3-06: + R1 Lazy Rendering — validate ADR-002
7. E3-07: + FG wait instrumentation — expose remaining display-update bottleneck

## Duration

Completed 2026-06-11 ~ 2026-06-16.

## Links and references

- [QAS-2: Low Latency](../final/references/qa/qas-2-low-latency-and-low-number-of-missed-beats.md)
- [ADR-001: T2 DSP Offload Thread](../final/references/adr/ADR-001-t2-dsp-offload-thread.md)
- [ADR-002: R1 Lazy Rendering](../final/references/adr/ADR-002-r1-lazy-rendering.md)
- [experiment-results.md](../experiment-results.md) — full run history with all 7 run logs
- Log files: `src/logs/EXP-02/`
