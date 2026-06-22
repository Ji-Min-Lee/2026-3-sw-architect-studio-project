# QA Strategy: Measurement Accuracy

## Requirement

The system shall detect T1 (A) and T3 (C) acoustic events with sufficient accuracy to
support meaningful measurement of small timing differences.

The software must accurately identify the start/onset and peak of these signals, which are
the basis for computing rate, beat error, amplitude, lift angle, beats per hour, and
balance-wheel frequency. Timing precision must be preserved throughout the full pipeline:
acquisition → filtering → event detection → calculation.

WeiShi No.1000 is used as ground truth. The goal is not to outperform WeiShi, but to
confirm that our system's measurements are within an acceptable error margin of it.
The acceptable margin is determined empirically through EXP-01.

**Reference**: [QAS-1: Measurement Accuracy, Error Detection, and Handling](../../milestone2/final/references/qa/qas-1-measurement-accuracy-error-detection-handling.md)

---

## Architectural Decision 1: Abstract AudioSource as an Interface (IAudioSource)

### Rationale

Validating measurement accuracy against WeiShi requires replaying a recorded watch signal
(wav file) through the exact same processing pipeline that handles live microphone input.
If Live and Playback modes follow different code paths, a test passing in Playback mode
does not prove that the Live pipeline is correct.

By defining `IAudioSource` as an interface with `MicSource`, `WavSource`, and `SimSource`
as concrete implementations, all three modes feed into an identical downstream pipeline.
A golden dataset test (WeiShi recording + expected measurement values) then becomes a
valid regression test for the live system.

### Trade-offs

| | Mode-specific branching | IAudioSource interface (chosen) |
|--|------------------------|-------------------------------|
| Test validity | Playback tests a different path | Playback tests the same path as Live |
| Complexity | Simple, no abstraction needed | Requires interface definition and DI |
| Extensibility | Adding a new source requires modifying pipeline code | New source is a new class only |
| Noise injection | Requires separate test harness | WavSource accepts any wav, including noise-mixed |

### Alternatives Considered

**Separate test harness that bypasses the normal pipeline**
- A standalone test binary could call the detection algorithm directly with pre-loaded
  samples, skipping the IAudioSource layer.
- Not applied because: this bypasses filtering and buffering stages that are part of the
  pipeline and can affect timing. Accuracy must be validated end-to-end, not algorithm-only.

### ADRs

| ADR | Decision | Status |
|-----|----------|--------|
| [ADR-005: IAudioSource Dependency Inversion](../../milestone2/final/references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) | Introduce IAudioSource interface; invert dependency from MainWindow | ✅ Applied |

### Views

| View | What it shows | Status |
|------|--------------|--------|
| [Module View: IAudioSource](../../milestone2/final/references/views/view-iaudiosource.md) | AS-IS vs TO-BE structure; what a developer must do to add a new audio source | ✅ Exists |

---

## Architectural Decision 2: Golden Dataset Integration Test in CI Pipeline

### Rationale

T1/T3 detection accuracy cannot be verified by unit tests alone because the correctness
of a detected timestamp depends on the full signal context: sample rate, filter state,
and signal shape across multiple beats. A golden dataset test — pairing a recorded WeiShi
wav with the measurements WeiShi reported for that recording — verifies the complete
pipeline against a known reference.

Integrating this test into the CI pipeline (pre-commit) ensures that algorithm changes
do not silently regress accuracy. Without this gate, a filter parameter change or
detection threshold adjustment might improve one metric while degrading another, and the
regression would only be discovered during a manual demo.

### Trade-offs

| | Manual comparison only | CI golden dataset test (chosen) |
|--|----------------------|--------------------------------|
| Regression detection | Discovered during demo | Caught before commit |
| Setup cost | None | Requires WeiShi recording and expected-value mapping |
| Maintenance | None | Golden dataset must be updated if WeiShi values change |
| Confidence | Low | High — same pipeline as production |

### Alternatives Considered

**Compare against a software reference implementation instead of WeiShi**
- A second independent implementation of the detection algorithm could serve as the oracle.
- Not applied because: two implementations of the same algorithm tend to share the same
  bugs. WeiShi is a hardware reference that is independent of our software assumptions.

### Views

| View | What it shows | Status |
|------|--------------|--------|
| [Deployment View: Build-Deploy Pipeline](../../milestone2/final/references/views/view-deployment-build-pipeline.md) | CI pipeline structure; deploy path from dev machine to RPi 5 | ✅ Exists |

> ⚠️ **TODO**: The current deployment view does not yet include the golden dataset CI test step.
> Update `view-deployment-build-pipeline.md` to add the WeiShi wav → integration test assertion
> stage once EXP-01 is complete and golden TCs are created.

---

## Experiments

| ID | Description | Status |
|----|-------------|--------|
| [EXP-01](../../milestone2/final/references/experiments/exp-01-accuracy-weishi-comparison.md) | WeiShi Accuracy Comparison — TimeChecker vs WeiShi No.1000 | ⏸ Planned (W5, 2026-06-29 ~ 2026-06-30) |

---

## Risks

| Risk | Mitigation |
|------|-----------|
| WeiShi measurements vary across repeated runs | EXP-01: measure same watch 5+ times with WeiShi and record variance. If variance is small (< 1 s/d rate, < 1° amplitude), WeiShi is stable enough as ground truth. |
| WeiShi display resolution is coarser than our system (e.g., 1 s/d steps) | Record WeiShi's resolution per metric before defining expected values. Assertion tolerance must account for WeiShi's quantization. |
| Recording wav + WeiShi result mapping is labor-intensive | One high-quality recording per BPH value (21600, 28800, 36000) is sufficient for regression coverage. |

---

## Verification

- EXP-01: play WeiShi recording through system → assert rate, amplitude, beat error
  within agreed tolerance of WeiShi reference values
- Evidence: EXP-01 results + CI test pass logs presented in demo

> ⚠️ **TODO**: EXP-01 not yet executed. Golden TCs and CI integration step pending completion
> of WeiShi comparison experiment.
