# M3 Strategy TODO

## Done

| Item | File |
|------|------|
| ✅ Unit test pre-commit hook (TestWatchMath, TestMeasurementEngine) | `.git/hooks/pre-commit` |

---

## Requires Experiment First

### EXP-01: WeiShi Accuracy Comparison
**Planned**: W5 (2026-06-29 ~ 2026-06-30)
**Reference**: [exp-01-accuracy-weishi-comparison.md](../../milestone2/final/references/experiments/exp-01-accuracy-weishi-comparison.md)

After EXP-01 completes:

- [ ] Create golden dataset TCs (WeiShi recording wav + expected Rate/Amplitude/BeatError values)
- [ ] Integrate golden dataset integration test into CI pipeline
- [ ] Update [Deployment View](../../milestone2/final/references/views/view-deployment-build-pipeline.md) — add golden dataset test step

---

### Noise-Mixed Wav CI Test
**Blocker**: noise-mixed wav file not yet prepared (EXP-05 parameter values are confirmed)
**Reference**: [exp-05-correctness-detector-optimization.md](../../milestone2/final/references/experiments/exp-05-correctness-detector-optimization.md)

After wav file is prepared:

- [ ] Create noise-mixed wav TC (watch wav + ambient noise, expected measurement values)
- [ ] Integrate noise-mixed wav test into CI pipeline
- [ ] Update [Deployment View](../../milestone2/final/references/views/view-deployment-build-pipeline.md) — add noise test step
