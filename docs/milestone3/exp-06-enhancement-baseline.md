# Experiment 6: Sound Print & Rate/Scope Enhancement — Baseline Capture

## Results and recommendations

Baseline data captured on 2026-06-17 (macOS, SIM mode, 48000 Hz, git `5ff5a70`).
Key observations before any enhancement:

- **Sound Print**: A/C event markers are binary-colored (green/blue) with no confidence gradient.
  Beat lines compressed into a narrow band — no grid overlay to show period drift.
  Dynamic range not normalized: signal intensity varies with simulated amplitude only.
- **Rate/Scope**: Rate error scatter is flat at 0.0 ms (SIM ideal signal, no drift).
  No trend line, no statistics overlay. Y-axis is fixed ±10 ms regardless of actual spread.
  Scope scale controlled only by manual spinbox (no slider, no live feedback).
- **Performance (macOS baseline)**: `sound_ms avg=0.000 ms` (Sound Print render effectively
  free in SIM), `plot_ms avg=0.007 ms` (max spike 70.5 ms — QCustomPlot replot outlier).
  Overall `exec_ms avg=0.094 ms`, well within 10 ms deadline (1/9611 frames exceeded).

After-enhancement targets: visible improvement in event readability (SP) and measurement
clarity (RS), with no regression in `exec_ms` or deadline miss rate.

## Objective

Capture a reproducible before-state for the following Area 2 enhancements so that
before/after comparisons can be demonstrated in the M3 final demo:

- **SP-1** Dynamic per-column normalization (Sound Print brightness)
- **SP-2** Confidence-gradient A/C event markers
- **SP-3** Beat period grid overlay
- **RS-1** Rolling average trend line on Rate Error plot
- **RS-2** Mean ± σ statistics overlay on Rate plot
- **RS-4** Scope zoom slider (replace fixed spinbox scale)

The experiment answers: *What does the baseline look like visually and quantitatively,
and what metrics should improve after each enhancement?*

## Status

Concluded (baseline capture only)

## Expected outcomes

- Screenshot pair (before/after) for each enhancement
- CSV log + analysis plot for performance regression check after each enhancement
- Updated `experiment-results.md` with EXP-06 baseline row

## Resources required

- macOS build with `ENABLE_LOGGING=ON` (`build-mac-log/`)
- SIM mode (28800 BPH, Error Rate 0 s/d, Amplitude 300°, Beat Error 0.0 ms)
- `src/tools/analyze_log.py`

## Experiment description

### Baseline run (2026-06-17, completed)

| Item | Value |
|------|-------|
| Build | `build-mac-log/` (`git 5ff5a70`, `ENABLE_LOGGING=ON`) |
| Mode | SIM — 28800 BPH, 0 s/d error, 300° amplitude |
| Sample rate | 48000 Hz |
| Duration | ~96 s (9611 frames) |
| Log | `src/logs/EXP-06/log_20260617_045128_baseline.csv` |
| Plot | `src/logs/EXP-06/log_20260617_045128_baseline.png` |

### Baseline screenshots

| Screen | File | Notes |
|--------|------|-------|
| Rate/Scope tab | `screenshots/baseline_rate_scope.png` | Flat scatter at 0 ms, no trend line, no stats |
| Sound Print tab | `screenshots/baseline_sound_print.png` | Narrow band, binary A/C markers, no beat grid |

### Performance baseline (macOS, SIM mode)

| Metric | avg | max |
|--------|-----|-----|
| `total_ms` | 0.114 ms | 23.1 ms |
| `exec_ms` | 0.094 ms | 13.2 ms |
| `sound_ms` | 0.000 ms | 0.000 ms |
| `plot_ms` | 0.007 ms | 70.5 ms |
| `tg_ms` | 0.092 ms | 13.2 ms |
| Deadline misses | 1 / 9611 | — |
| Block drops | 0 | — |

### After-enhancement procedure (per enhancement)

1. Implement enhancement on `feature/enhancements` branch
2. Re-run `build-mac-log/` with same SIM config
3. Capture same-tab screenshot → save as `screenshots/<id>_after.png`
4. Run `analyze_log.py` → record `sound_ms` / `plot_ms` delta
5. Add one row to `experiment-results.md` under EXP-06

## Duration

Baseline: 2026-06-17 (done). After-runs: during M3 implementation sprint.

## Links and references

- `docs/milestone2/experiment-results.md` — prior EXP-01 through EXP-05 results
- `src/logs/EXP-06/` — all baseline and post-enhancement logs
- `src/tools/analyze_log.py` — analysis script
- Grading rubric: `assets/Draft LG SW Architect Final Demo.pdf` — Area 2, 8+8 pts
