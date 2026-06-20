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

We adopt **Option A (96kHz)**. EXP-01 (2026-06-15) confirmed Dropped Block = 0 at 96kHz on RPi 5
under combined audio + Qt GUI load across all 3 scheduling policies (default / SCHED_RR / SCHED_FIFO).
Beat Error resolution: **10.4 µs/sample**. Option B (48kHz) fallback is no longer required.

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

**Accepted** (2026-06-15)

EXP-01 RPi result confirmed — [EXP-01: Verifying Sustained High-Resolution Sampling on RPi 5 for Beat Error Timestamp Resolution](../experiments/exp-01-sample-rate.md).
Transitioned from Proposed to Accepted on 2026-06-15 (2 days ahead of target).

## Consequences

**Option A (96kHz) adopted**:
- Beat Error resolution: **10.4 µs/sample** — sufficient for WeiShi comparison
- `FilterChain` cutoffs set on 96kHz Nyquist basis
- SCHED_RR for audio thread: **not required** — EXP-01 showed no improvement in Dropped Block count
- Thermal throttling at 85 °C does not affect Dropped Block count (30 s ring buffer absorbs all overruns)
- 192kHz stretch goal remains possible (0 dropped blocks confirmed), but not required for M3

**Option B (48kHz) fallback**: no longer needed.
