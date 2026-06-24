# Experiment 4: Ambient-Noise Warning Threshold Calibration

## Results and Recommendations

**Conducted: 2026-06-23.** The ambient-noise popup threshold `kNoiseThresholdDb`
was changed from an arbitrary **55** to an empirically calibrated **51**.

- Measurement tracks the watch perfectly up to `noiseDb ≈ 50` and fails
  catastrophically at `noiseDb ≈ 54` (rate error jumps from the true +4.4 s/d to
  −3,200…−22,000 s/d; beat error 0.2 ms → 18–20 ms; sync drops to 86–95 %).
- The previous **55** was **above the failure point** (peak `noiseDb` even at full
  breakdown was 54.8 < 55), so the popup would **never fire when measurement is
  actually broken**. The calibrated **51** sits in the valid window (50.1, 54.2),
  above the noisiest still-working signal and below the failure onset, biased
  (conservatively) toward catching failures.

## Objective

Replace the hand-picked ambient-noise warning threshold with a value grounded in
measured detector behaviour: find the `noiseDb` level at which rate/beat error
exceeds tolerance (the watch becomes unmeasurable), and set the popup threshold
just below it.

**Decision this resolves**: `MainWindow::kNoiseThresholdDb` — the level at which
the "High ambient noise — please move to a quieter location" popup is raised.
Affects QAS-4 (measurement-readiness feedback).

`noiseDb = 20·log10(onset_threshold) + 100`, where `onset_threshold` is the
detector's adaptive floor between beats (≈ ambient noise level, signal-independent
by design). The `+100` offset is an uncalibrated dBFS→SPL-like shift.

## Method

Offline, no GUI — the `EngineFileCheck` harness feeds each WAV through the real
`MeasurementEngine` in 4096-sample blocks (identical to `MainWindow`) and logs
per-block `synced, detectedBph, rateErrorSpd, beatErrorMs, noiseDb`.

- **Stimuli**: 14 recordings of one watch (28,800 BPH, 3235 movement, "Starbucks"
  ambient), pink noise mixed in at two label families — `snr{00..60}db` (SNR: 60 =
  clean → 0 = noisy) and `noise{00..60}db` — under
  `src/TimeGrapherTestFilesWeishiMic/Noise/`.
- **X-axis is the measured `noiseDb`, not the file label**, so the two label
  conventions are reconciled automatically.
- **Detector parity**: the calibration was run on the shipping detector
  (`onset_fraction = 0.03`; `Detector.cpp` / `Timegrapher.cpp` are byte-identical
  between this branch and `main`), so the result is valid for the version that
  ships the popup.
- Per file, aggregated over the stable region (t ≥ 8 s): median `noiseDb`, median
  rate/beat error, sync %.

## Results

Sorted by measured `noiseDb` (low = quiet → high = noisy):

| noiseDb (med) | sync % | rateErr median | \|rate\| max | beat median | verdict |
|:---:|:---:|:---:|:---:|:---:|:---|
| 46.5 – 47.2 | 100 | **+4.4 s/d** | 6 | 0.195 ms | ✅ tracks (snr20–60, noise00–40) |
| 49.4 | 100 | +4.4 | 6 | 0.198 | ✅ tracks (noise50) |
| **49.5** | 100 | +4.6 | 16 | 0.201 | ✅ **last good** (snr10) |
| **54.2** | 86 | **−22,045** | 34,308 | 20.05 | ❌ **breakdown** (noise60) |
| **54.4** | 95 | **−3,206** | 8,780 | 17.68 | ❌ **breakdown** (snr00) |

- The `snr` and `noise` sets — independent stimuli — break down at the **same**
  measured `noiseDb ≈ 54`, cross-validating the result.
- Clean/working signals read `noiseDb` 45.7–50.1; broken signals read 53.7–54.8.
- **Valid threshold window = (50.1, 54.2)**, midpoint ≈ 52.

## Decision

`kNoiseThresholdDb = 51` (conservative end of the window):

- `> 50.1` → no false alarm on the noisiest signal that still measures (snr10).
- `< 54.2` → fires before/at the breakdown the old 55 missed.
- Biased low (51 rather than the 52 midpoint) because a missed warning (user
  trusts garbage data) is worse than an occasional early warning.

## Limitations / Follow-up

- **Data gap 50.1 → 54.2**: no stimulus lands between the last-good and first-fail
  levels, so the exact cliff is bracketed, not pinpointed. Generating intermediate
  levels (e.g. `snr05`, `snr15`) would tighten it.
- Single recording / movement / ambient profile; other watches or rooms may shift
  the curve. Re-run across more recordings before treating 51 as final.
- The `+100` offset is still uncalibrated to true SPL — "51" is an internal-scale
  threshold, but it is now tied to a measured failure point rather than a guess.

## Reproduce

```bash
# build the offline harness (emits per-block noiseDb in its CSV / summary)
cmake --build build --target EngineFileCheck -j4

WAV=src/TimeGrapherTestFilesWeishiMic/Noise
for f in "$WAV"/28800BPH_3235_Starbucks_{snr,noise}{00,10,20,30,40,50,60}db.wav; do
    build/EngineFileCheck "$f" 28800 "/tmp/$(basename "$f" .wav).csv"   # prints noise dB avg/max
done
# then aggregate median noiseDb vs rate/beat error per file (stable region t >= 8 s)
```
