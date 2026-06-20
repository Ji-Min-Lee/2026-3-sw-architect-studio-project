# EXP-05: Measuring Multi-Tab Concurrent Rendering Impact on Real-Time Pipeline Performance on RPi 5

## Results and Recommendations

Not yet conducted. Scheduled: 2026-06-26.

EXP-02 R3/R4 confirmed R1 (Lazy Rendering) reduces replot_count by 75–85% on macOS with 1 active tab. This experiment validates whether R1 is sufficient under full 11-tab load on the target hardware.

If RPi FPS drops below target under 11-tab load → fall back to R2 (Timer-Decoupled Rendering); ADR-002 would be superseded.

## Objective

Measure Qt rendering FPS and CPU load on Raspberry Pi 5 under concurrent multi-tab load (up to 11 active tabs), with R1 Lazy Rendering applied.

**Decision this validates**: [ADR-002](../adr/ADR-002-r1-lazy-rendering.md) (R1 Lazy Rendering) — confirms whether R1 is sufficient under full-tab load, or whether R2 (Timer-Decoupled) is required.

**Risk resolved**: TR-04 (partial — RPi confirmation of rendering strategy)

## Status

Planned (target: 2026-06-26)

## Expected Outcomes

- UI thread FPS at 1 / 3 / 6 / 11 active tabs on RPi 5
- CPU load distribution across cores at each tab count
- Confirmation that deadline miss rate remains 0% with R1 under full-tab load
- If R1 insufficient: updated ADR-002 superseded by R2 decision

## Resources Required

| Item | Detail |
|------|--------|
| Platform | Raspberry Pi 5 |
| Branch | `feature/layer` with T2 + R1 applied |
| Logging | `ENABLE_LOGGING=ON` — per-frame replot_count, plot_ms, exec_ms |
| WAV file | `28800BPH_3235_Starbucks.wav` |
| Tab stubs | At least 11 IGraphTab implementations (or stub tabs) active |

## Experiment Description

1. Build with `ENABLE_LOGGING=ON` + T2 + R1 on RPi
2. Open 1 tab — run 5 min — record: FPS, exec_ms, plot_ms, replot_count, deadline miss %
3. Open 3 tabs — repeat
4. Open 6 tabs — repeat
5. Open 11 tabs — repeat
6. If deadline miss > 0% at any tab count: measure replot_count delta to quantify gap
7. Determine whether R1 alone closes the gap or R2 is required

## Duration

Target: 2026-06-26

## Links and References

- ADR to validate or supersede: [ADR-002](../adr/ADR-002-r1-lazy-rendering.md)
- Risk: [TR-04](../risks.md)
- QA: [QAS-1 Real Time Performance](../qa/qas-2-real-time-performance.md)
- Related: [EXP-02 macOS results](exp-02-realtime-deadline-compliance.md) — baseline for comparison
