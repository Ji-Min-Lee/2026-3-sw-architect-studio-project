# ADR-003: Audio Sample Rate Selection for RPi 5

The system's Beat Error resolution and audio pipeline timing budget are both directly determined
by the sample rate. Higher SPS shortens the block period and increases T1 timestamp resolution,
but increases DSP CPU load per callback — on a thermally constrained board like Raspberry Pi 5,
this trade-off is not verifiable without target hardware measurement.

macOS confirms 96kHz is sustainable with 0 dropped blocks (EXP-02 R2 baseline).
RPi 5 confirmation is pending EXP-01 (target: 2026-06-23).

Two fallback options exist if 96kHz is not achievable on RPi:

| Option | SPS | Beat Error Resolution | Block Period | Risk |
|--------|:---:|:--------------------:|:------------:|:----:|
| A (Primary) | 96,000 | 10.4 µs/sample | ~10 ms | RPi capability unconfirmed |
| B (Fallback) | 48,000 | 20.8 µs/sample | ~20 ms | Confirmed achievable |
| C (Stretch) | 192,000 | 5.2 µs/sample | ~5 ms | RPi capability very uncertain |

## Decision

To be determined after EXP-01 RPi measurement (2026-06-23).

Preliminary: We will adopt **Option A (96kHz)** if EXP-01 confirms 0 dropped blocks on RPi 5
under combined audio + Qt GUI load. If dropped blocks appear at 96kHz, we will adopt
**Option B (48kHz)** as the fallback.

## Rationale

The sample rate sets three downstream constants that cannot be changed without rebuilding
the pipeline:

1. `SignalBuffer` ring buffer size (PCM block size = SPS / callback_freq)
2. `FilterChain` LP/HP cutoff frequencies (normalized to Nyquist = SPS / 2)
3. Beat Error resolution ceiling (1 sample = 1/SPS seconds)

These constants must be fixed before Phase A task A-02 (filter chain configuration) and
A-03 (beat detection validation) can be completed.

Rejected early: Option C (192kHz). RPi 5 DSP load at 192kHz under full Qt GUI has not been
measured and the block period (~5ms) leaves almost no margin for DSP processing spikes.
Deferred unless EXP-01 produces an unexpectedly favorable result.

## Status

Proposed

Pending: EXP-01 RPi result — [EXP-01 Sample Rate Performance](../experiments/exp-01-sample-rate.md)
Expected to transition to **Accepted** on 2026-06-23 after EXP-01 completes.

## Consequences

**If Option A (96kHz) accepted**:
- Beat Error resolution: 10.4 µs/sample — sufficient for WeiShi comparison
- `FilterChain` cutoffs set to 96kHz Nyquist basis
- T1 SCHED_RR (EXP-02 R6) may still be needed to absorb thermal throttle jitter

**If Option B (48kHz) fallback adopted**:
- Beat Error resolution degrades to 20.8 µs/sample — within WeiShi comparison tolerance,
  but reduces precision for high-BPH watches
- `FilterChain` cutoffs recalculated at 48kHz Nyquist basis
- EXP-03 filter sweep must be re-run at 48kHz to confirm cutoff values remain valid
- 192kHz stretch goal abandoned for M3

**Either way**:
- Sample rate constant must be committed before Phase A A-02 begins (target: 2026-06-24)
- ADR-003 must transition from Proposed to Accepted by 2026-06-23 to avoid blocking Phase A
