# Experiment 3: LP/HP Filter Parameter Sweep

## Results and Recommendations

Not yet conducted. Scheduled: 2026-06-25.

Preliminary: adaptive threshold is implemented and tunable via a single parameter. Default LP/HP cutoffs are set conservatively. Sweep will confirm or replace these defaults.

## Objective

Determine the LP (low-pass) and HP (high-pass) cutoff frequencies that best preserve beat events while rejecting ambient noise, across multiple watch BPH values.

**Decision this resolves**: Filter cutoff constants in `FilterChain` — affects [QAS-4 Correctness](../qa.md#qas-4-correctness--priority-5) and downstream accuracy of Beat Error / Amplitude measurements.

**Risk resolved**: TR-05

## Status

Planned (target: 2026-06-25)

## Expected Outcomes

- SNR (signal-to-noise ratio) table at each LP/HP combination
- Detection rate (%) for known beat events at each cutoff
- Recommended LP cutoff (Hz) and HP cutoff (Hz) for 28,800 BPH primary target
- Secondary recommendations for 18,000–36,000 BPH range (M3 stretch)
- Phase A task A-02 unblocked: filter chain configured with confirmed cutoffs

## Resources Required

| Item | Detail |
|------|--------|
| Platform | macOS (sweep) + RPi 5 (confirmation) |
| Branch | `feature/layer` |
| WAV file | `28800BPH_3235_Starbucks.wav` + ambient noise recording |
| Analysis tool | `src/tools/analyze_log.py` |
| Logging | Beat detection log with per-frame SNR |

## Experiment Description

1. Record ambient noise baseline (no watch) — measure noise floor at microphone
2. For LP cutoff in [500, 1000, 2000, 4000] Hz and HP cutoff in [20, 50, 100, 200] Hz:
   - Run pipeline with watch WAV
   - Record: T1 detection count, false positive count, mean SNR at T1 events
3. Identify combination with highest true positive rate and lowest false positive rate
4. Confirm on RPi under identical conditions
5. Update `FilterChain` constants; issue ADR supplement if cutoffs differ significantly from defaults

## Duration

Target: 2026-06-25 (blocks Phase A task A-02)

## Links and References

- Risk: [TR-05](../risks.md)
- Phase A dependency: task A-02 (LP/HP filter chain cutoffs)
- QA: [QAS-3 Signal Quality](../qa.md)
