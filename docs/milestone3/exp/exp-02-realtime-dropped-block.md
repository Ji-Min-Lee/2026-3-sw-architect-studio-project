# Experiment 2: RPi Real-Time Performance — Dropped Block Measurement

## Results and recommendations

**Outcome: ✅ Pass** — Dropped Block = 0 across all 9 runs (3 sps × 3 scheduling policies).

**Confirmed operating point**: 96k sps, default scheduling — exec avg 9.6 ms (< 21.33 ms deadline). SCHED_RR / FIFO are not required.

The 30-second ring buffer absorbs all deadline-exceeded frames without any block loss. 96k sps is confirmed as the target sample rate (ADR-003).

## Objective

Verify that RPi 5 sustains zero dropped audio blocks at 96k sps under continuous Qt GUI + DSP concurrent operation, confirming QAS-1 (Real-Time Performance).

Technical question: Can the RPi 5 process audio at 96,000 sps with zero dropped blocks across all realistic scheduling configurations?

## Status

Concluded

## Expected outcomes

- Dropped block count per run for each (sps, scheduling policy) combination
- Quantitative pass/fail verdict
- Confirmed target sps for ADR-003

## Resources required

- RPi 5 (host=lg1, device=rpi1)
- TimeChecker built with `--log` flag and `droppedBlockCount` counter
- 30-second ring buffer in ALSA layer
- ~1 person-day

## Experiment description

1. Build TimeChecker with logging enabled (`cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DTIMEGRAPHER_LOG=ON`)
2. For each (sps, scheduling) combination: start TimeChecker, run for 5 minutes, record `droppedBlockCount` and exec avg/max
3. Scheduling configurations: default (SCHED_OTHER), SCHED_RR p50, SCHED_FIFO p50
4. Sample rates: 48k, 96k, 192k
5. Pass condition: Dropped Block = 0 at 96k sps under default scheduling

## Duration

Completed 2026-06-15.

## Links and references

- [QAS-1: Real-Time Performance](../final/references/qa/qas-1-real-time-performance.md)
- [ADR-001: T2 DSP Offload Thread](../final/references/adr/ADR-001-t2-dsp-offload-thread.md)
- [ADR-003: 96kHz Sample Rate](../final/references/adr/ADR-003-sample-rate-selection.md)
- [experiment-results.md](../experiment-results.md) — full run history with all 9 run logs
- Log files: `src/logs/EXP-01/` (legacy directory name)
