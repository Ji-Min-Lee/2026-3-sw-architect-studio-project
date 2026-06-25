# Experiment 5: Detector Parameter Optimization Under Noise

## Results and recommendations

**Outcome: ✅ Done** — `onset_fraction = 0.08`, `min_peak_fraction = 0.10` confirmed as optimal.

`onset=0.08` is the only value that maintains tracking at 60 dB SNR. `onset=0.02` and `0.05` fail catastrophically at high noise (rate collapses to −4,264 s/d and −393 s/d respectively). `min_peak` has minimal differential effect within the onset=0.08 group; `0.10` selected for slightly lower Beat Error.

These parameters are now the defaults in `Detector.cpp`.

| Parameter | Tested range | Selected | Reason |
|-----------|-------------|:--------:|--------|
| `onset_fraction` | 0.02, 0.05, 0.08 | **0.08** | Only value stable at 60 dB SNR |
| `min_peak_fraction` | 0.10, 0.20, 0.30 | **0.10** | Lowest Beat Error within onset=0.08 group |

## Objective

Identify the `onset_fraction` and `min_peak_fraction` values for `Detector.cpp` that maintain accurate beat detection across a wide range of ambient noise conditions (0–60 dB SNR), confirming the default parameters for QAS-4 Sub-3 (Noise Resilience).

Technical question: Which (onset, min_peak) pair produces the most stable Rate measurement across noise levels 0 dB–60 dB SNR?

## Status

Concluded

## Expected outcomes

- Rate (s/d) vs. noise level table for each (onset, min_peak) combination
- Identification of the (onset, min_peak) pair with minimum rate variance
- Updated default parameters in `Detector.cpp`

## Resources required

- RPi 5 (host=lg1, device=rpi1)
- WAV source: 28,800 BPH real recording + pink noise (96 kHz, float32), generated with `add_pink_noise.py`
- 315 planned runs: onset {0.02, 0.05, 0.08} × min_peak {0.10, 0.20, 0.30} × noise {0, 10, 20, 30, 40, 50, 60 dB} × 5 reps
- Log directory: `src/logs/EXP-03/` (legacy name)
- Analysis: `src/tools/analyze_exp04_scatter.py`
- ~2 person-days

## Experiment description

1. Generate test WAV files for each noise level using `add_pink_noise.py` (pink noise, seed=42)
2. For each (onset, min_peak, noise_level, rep) combination: run TimeChecker in playback mode, capture Rate / Beat Error / Amplitude
3. Average Rate across 5 reps per condition; identify conditions where the detector loses sync (catastrophic values)
4. Select the (onset, min_peak) pair with lowest rate variance and highest noise floor before failure
5. Update `Detector.cpp` defaults and commit

## Duration

Completed 2026-06-16 ~ 2026-06-17.

## Links and references

- [QAS-4: Correctness — Sub-3 Noise Resilience](../final/references/qa/qas-4-correctness.md)
- [ADR-009: FilterChain Design](../final/references/adr/ADR-009-filterchain-design.md)
- [experiment-results.md](../experiment-results.md) — full noise sweep results table and sample CSVs
- Log files: `src/logs/EXP-03/`
