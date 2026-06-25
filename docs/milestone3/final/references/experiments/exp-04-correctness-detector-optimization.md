# EXP-04: Detector Parameter Optimization Under Noise

**QA**: QAS-4 (Sub-3: Noise Resilience) | **Status**: ✅ Done (2026-06-16 ~ 2026-06-17)

---

## Objective

Identify `onset_fraction` and `min_peak_fraction` values that maintain accurate beat detection across 0–60 dB SNR noise conditions, confirming the default parameters for `Detector.cpp`.

## Result

**Recommended: `onset_fraction = 0.08`, `min_peak_fraction = 0.10`**

`onset=0.08` is the only setting that maintains tracking at 60 dB SNR. `onset=0.02` and `0.05` fail catastrophically at high noise. `min_peak` has negligible effect within the `onset=0.08` group.

## Run History

WAV source: 28,800 BPH real recording + pink noise (96 kHz, float32). Platform: RPi 5 (host=lg1, device=rpi1).

| Run | Date | Scope | Measurements | Data |
|:---:|------|-------|:------------:|:----:|
| E5-01 | 2026-06-15 | Pilot — default params, 48 kHz | 3 | — |
| E5-02 | 2026-06-16 | Early grid — onset {0.02, 0.08} × noise {0, 60} dB | 8 | — |
| E5-03 | 2026-06-17 | Full grid — onset×min_peak×noise×5 reps | **274** | [logs](../../../../../src/logs/EXP-04/full-grid/) |

## Architecture Decisions

`Detector.cpp` default parameters updated to `onset=0.08`, `min_peak=0.10`.

- **`onset_fraction`**: fraction of peak amplitude used as the beat detection threshold. `0.08` is the only value that maintains stable Rate across 0–60 dB SNR; `0.02` and `0.05` fail at 60 dB (Rate −4,264 / −393 s/d).
- **`min_peak_fraction`**: minimum amplitude to qualify as a valid peak. Negligible effect within the `onset=0.08` group — `0.10` selected for lowest Beat Error.

## Links

- Full results table (7 noise levels): [experiment-results.md](../../../../milestone2/experiment-results.md#exp-05-detector-parameter-optimization-under-noise)
- Log directory: `src/logs/EXP-04/full-grid/` (274 CSVs) · `src/logs/EXP-04/pilot/` (pilot runs)
- Analysis tool: `src/tools/analyze_exp04_scatter.py`
