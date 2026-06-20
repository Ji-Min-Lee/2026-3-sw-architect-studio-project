# Experiment 1: Sample Rate Performance on Raspberry Pi 5

## Results and Recommendations

**RPi result (2026-06-15)**: Dropped Block = **0** at all tested sps (48k / 96k / 192k) under all scheduling policies (default / SCHED_RR / SCHED_FIFO). QAS-1 Pass.

**Recommended operating point**: 96k sps — exec avg 9.6 ms (well under 21.3 ms deadline), 0 dropped blocks, 30 s ring buffer absorbs all transient overruns.

**SCHED_RR / SCHED_FIFO**: No improvement in dropped block count. CPU load is distributed more evenly but exec avg is marginally higher. RT scheduling for the audio thread is **not required**.

**Thermal note**: All runs operate at ≥ 85 °C (rpi1) or ~60 °C (rpi2) with some throttling at 85 °C. Throttling does not affect Dropped Block count — the ring buffer absorbs exec spikes.

→ **ADR-003 transitioned to Accepted on 2026-06-15.** See [ADR-003](../adr/ADR-003-sample-rate-selection.md).

## Objective

Determine the maximum sustained audio sample rate the Raspberry Pi 5 can process without dropping blocks while the Qt GUI is running concurrently.

**Decision this resolves**: ADR-003 (sample rate selection) — sets `SignalBuffer` ring buffer size and `FilterChain` cutoff calculation basis. Also sets Beat Error resolution: 96kHz = 10.4µs/sample; 48kHz = 20.8µs/sample.

**Risk resolved**: TR-01

## Status

**Done** (2026-06-15)

## Expected Outcomes

- Dropped block count at 48kHz / 96kHz / 192kHz under default, SCHED_RR, and SCHED_FIFO scheduling
- Maximum sustained sample rate with 0 dropped blocks under stable thermal conditions
- RT scheduling (SCHED_RR / SCHED_FIFO) necessity determination
- ADR-003 sample rate decision (96kHz vs 48kHz) and Beat Error resolution (10.4µs vs 20.8µs/sample)

## Run History

3 runs executed 2026-06-15 on RPi (host=lg1, platform=debian). Each run covers all 3 sps sequentially. Duration = 5 min per sps. Buffer = 30 s. Deadline = **21.33 ms** for all sps (ALSA scales SPF proportionally).

| Run | Scheduling | 96k exec avg/max (ms) | 96k exec > DL | Dropped | Temp avg (°C) |
|:---:|-----------|:---------------------:|:-------------:|:-------:|:-------------:|
| R1 | default | 9.6 / 39.2 | 8.1 % | **0** | 85.1 |
| R2 | SCHED_RR p50 | 9.8 / 39.9 | 8.4 % | **0** | 85.3 |
| R3 | SCHED_FIFO p50 | 9.9 / 41.4 | 8.6 % | **0** | 85.4 |

exec > DL frames are absorbed by the 30 s ring buffer — Dropped stays 0 in all cases. Full 3-sps breakdown (48k/96k/192k) available in [experiment-results.md](../../../../milestone2/experiment-results.md).

## Resources Required

| Item | Detail |
|------|--------|
| Platform | Raspberry Pi 5 (target hardware) |
| Branch | `feature/layer` |
| Logging | `ENABLE_LOGGING=ON` — dropped block counter in AudioCapture |
| WAV file | `28800BPH_3235_Starbucks.wav` |
| Qt GUI | Running in parallel (combined load scenario) |

## Experiment Description

1. Build with `ENABLE_LOGGING=ON` on RPi
2. Run at 48kHz for 5 minutes — record dropped block count, CPU %, temperature
3. Repeat at 96kHz
4. Repeat at 192kHz
5. Identify the highest rate with 0 dropped blocks under stable temperature (< 80°C)
6. Record result in ADR-003

## Duration

Target: 2026-06-23

## Links and References

- Risk: [TR-01](../risks.md)
- ADR to be issued: ADR-003 (pending result)
- macOS baseline: confirmed 96kHz, 0 dropped blocks (AudioCapture logging, 2026-06-13)
