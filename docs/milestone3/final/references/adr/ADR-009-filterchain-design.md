# ADR-009: FilterChain Design — HPF + Envelope Detector

The microphone signal contains watch ticks (T1/A and T3/C events) superimposed on
environmental noise: AC mains hum (50–60 Hz), table rumble, handling vibration,
and ambient speech. The detector must fire only on genuine beat impulses. Signal
conditioning before the detector determines what reaches the threshold comparison.

The baseline codebase used a bandpass filter approach (inherited from the Java-era
implementation). Bandpass filtering improves SNR for steady-state periodic signals
but performs poorly on short broadband impulses: a 4th-order bandpass rings for
approximately 3–5 ms after each impulse, spreading energy that arrives as two distinct
short events (A and C, typically 8–15 ms apart at 28,800 BPH) into a single blurred
envelope hump. This made A and C indistinguishable at the detector, causing false
amplitude readings.

## Decision

We replace the bandpass approach with a two-stage pipeline: **single-pole HPF → envelope
detector**, implemented in `src/external/Dsp.h` / `Dsp.cpp`.

### Stage 1 — Single-Pole High-Pass Filter (DC blocker)

```
y[n] = x[n] - x[n-1] + a * y[n-1]     a = exp(-2π·f_c / fs)
```

Default cutoff `f_c = 200 Hz`. This rejects:
- DC bias from microphone preamp
- AC mains hum (50/60 Hz)
- Table rumble and low-frequency handling vibration

Watch tick energy peaks in the 1–5 kHz range; a 200 Hz cutoff leaves all tick content
untouched. The single-pole form introduces no ringing — phase response is monotonic.

### Stage 2 — Envelope Detector

```
env[n] = env[n-1] + alpha * (|x[n]| - env[n-1])     alpha = 1 - exp(-1 / (tau · fs))
```

Full-wave rectification followed by a one-pole low-pass smoother with default time
constant `tau = 0.15 ms`. This approximates the instantaneous amplitude envelope:
positive, slow-moving baseline with sharp rises on impulse onsets.

0.15 ms is chosen such that:
- A and C events (≥ 8 ms apart at 28,800 BPH) produce **two distinct envelope peaks**
- The high-frequency carrier inside each impulse collapses into a single measurable hump

### Combined Pipeline

```
PCM (96kHz) → HPF (200 Hz cutoff) → envelope (0.15 ms tau) → threshold detector
```

The detector fires on envelope peaks exceeding the configured onset threshold. Because
both filters are streaming single-pole IIR structures with only two state variables each,
the per-sample overhead is two multiplies and two adds per stage — negligible at 96kHz.

## Rationale

**Why not bandpass?**

A 4th-order bandpass (e.g., 500 Hz – 5 kHz) has a group delay of ~3–5 ms at its
transition bands and rings on impulse input. Watch ticks are broadband acoustic impulses,
not narrow-band sinusoids. After bandpass filtering, A and C events within the same beat
packet merge into a single envelope peak, making it impossible to detect their individual
timing. DC removal is the only frequency-domain operation the signal actually needs.

**Why 200 Hz HPF cutoff?**

The lowest meaningful tick energy is around 500 Hz. A cutoff of 200 Hz keeps the full
tick spectrum while rejecting mains hum (50/60 Hz) and typical table rumble (< 150 Hz).
Lowering to 100 Hz risks leaving residual hum; raising above 400 Hz risks attenuating
lower spectral content of the tick.

**Why 0.15 ms envelope smoothing?**

At 28,800 BPH the A–C interval is approximately 10–13 ms (depending on lift angle).
A smoothing constant of 0.15 ms collapses the carrier inside each impulse (~0.05–0.1 ms
duration at 96kHz) without spanning the inter-event gap. Constants > 1 ms begin to merge
the two events. Constants < 0.05 ms produce a noisy envelope that is difficult to
threshold reliably.

**Why expose HPF cutoff and smoothing to the user?**

Each microphone–watch combination has different coupling, gain, and ambient noise floor.
A fixed cutoff that works for a tight-contact condenser microphone may pass too much hum
for a loosely-placed dynamic microphone. The GUI exposes High Pass cutoff and C Onset
Amplitude so that users can tune detection for their hardware without recompiling.
This is a deliberate accuracy–usability tradeoff: tunable parameters increase usability
in varied environments while preserving measurement accuracy.

## Status

Accepted (implementation inherited from baseline; design rationale documented 2026-06-25)

Implemented in `src/external/Dsp.h` / `Dsp.cpp`.
Default parameters verified via EXP-04 (274 runs, onset=0.08, min_peak=0.10 confirmed):

| onset | 0–50 dB SNR | 60 dB SNR |
|-------|:-----------:|:---------:|
| 0.02 | unstable (+8–12 s/d) | ❌ −4,264 s/d |
| 0.05 | stable (+4.1–4.2 s/d) | ❌ −393 s/d |
| **0.08** ✅ | **stable (+3.9–4.1 s/d)** | **+7.5 s/d** |

`onset=0.08` is the only value that maintains tracking at 60 dB SNR.
`min_peak` has negligible effect within the `onset=0.08` group; `0.10` selected for lowest Beat Error.

## Consequences

**Positive**:
- A and C events remain distinguishable as separate envelope peaks at all supported BPH
  rates (18,000–43,200 BPH); amplitude computation requires both
- Two-stage pipeline is stateless between beats (reset on session start); no accumulated
  error from prior beats
- Per-sample cost is O(1) with no buffering — compatible with streaming 96kHz capture
- Tunable parameters allow accuracy adaptation to different microphone setups

**Negative**:
- No bandpass means broadband impulse noise (e.g., sharp desk tap, speech sibilants)
  that falls above the 200 Hz HPF can reach the detector. Mitigation: onset threshold
  tuning (EXP-04) and the `⚠ Noisy signal` warning (QAS-4 Sub-3)
- The 0.15 ms smoothing constant is fixed at compile time and not user-configurable.
  For unusual watches with very short A–C intervals (high BPH, small lift angle), this
  may need adjustment.

## Related

- [QAS-4: Correctness — Sub-3 Noise Resilience](../qa/qas-4-correctness.md)
- [ADR-003: Sample Rate Selection](ADR-003-sample-rate-selection.md) —
  96kHz chosen in part because it allows the 0.15 ms smoothing constant to
  span enough samples (14.4 samples) to be effective; at 48kHz the same
  constant spans only 7.2 samples, reducing smoothing stability
- [EXP-04: Detector Parameter Optimization](../experiments/exp-04-correctness-detector-optimization.md)
