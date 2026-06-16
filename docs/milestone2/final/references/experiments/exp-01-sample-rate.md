# Experiment 1: Sample Rate Performance on Raspberry Pi 5

## Results and Recommendations

macOS result (96kHz Playback mode): 0 dropped blocks sustained. RPi 5 measurement pending (target: 2026-06-23).

Recommendation: confirm 96kHz on RPi. If dropped blocks appear, fall back to 48kHz. ADR-003 will be issued after RPi result is confirmed.

## Objective

Determine the maximum sustained audio sample rate the Raspberry Pi 5 can process without dropping blocks while the Qt GUI is running concurrently.

**Decision this resolves**: ADR-003 (sample rate selection) — sets `SignalBuffer` ring buffer size and `FilterChain` cutoff calculation basis. Also sets Beat Error resolution: 96kHz = 10.4µs/sample; 48kHz = 20.8µs/sample.

**Risk resolved**: TR-01

## Status

In Progress (RPi measurement target: 2026-06-23)

## Expected Outcomes

- Dropped block count per minute at 48kHz / 96kHz / 192kHz on RPi 5
- CPU load and temperature at each sample rate under combined audio + Qt load
- Confirmed maximum sustained sample rate with 0 dropped blocks
- ADR-003 issued with chosen sample rate and rationale

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
