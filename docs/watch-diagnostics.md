# Watch Diagnostics (AI Feature — Step 1: Rule-Based Baseline)

A rule-based watch-condition classifier that consumes the existing
Rate / Amplitude / Beat-Error measurements and produces a coarse
diagnosis label, shown live in the GUI. This is the first step of the
AI feature track: it stands in for the ONNX model that will later
replace `WatchDiagnostics::Evaluate()` without touching any other code
(`MainWindow` only depends on the `DiagnosisInput -> DiagnosisResult`
contract).

---

## Why this exists

The project goal is an AI feature that runs on RPi5 without competing
with the real-time audio pipeline for CPU/RAM. Before adding a trained
model, step 1 builds the **integration point** with a deterministic
stand-in: zero inference cost, but the same interface a future
classifier will sit behind. See the project's AI options discussion
for the full roadmap (rule-based -> ONNX classifier -> anomaly
detection -> tiny LLM report).

---

## Module: `WatchDiagnostics`

| File | Contents |
|------|----------|
| [`src/engine/WatchDiagnostics.h`](../src/engine/WatchDiagnostics.h) | `DiagnosisInput`, `DiagnosisResult`, `DiagnosisLevel`, `WatchDiagnostics::Evaluate()` |
| [`src/engine/WatchDiagnostics.cpp`](../src/engine/WatchDiagnostics.cpp) | Threshold logic |

```cpp
struct DiagnosisInput {
    double    rate_spd;        bool rate_valid;
    double    amplitude_deg;   bool amplitude_valid;
    double    beat_error_ms;   bool beat_error_valid;
    WatchType watch_type = WatchType::Men;  // Men (default) | Women
};

struct DiagnosisResult {
    DiagnosisLevel level;   // Unknown | Excellent | Good | NeedsService
    QString        label;   // human-readable, ready for the UI
};

class WatchDiagnostics {
public:
    DiagnosisResult Evaluate(const DiagnosisInput &input) const;
};
```

`Evaluate()` is a pure function: no state, no I/O, no Qt event
dependency. That is what makes it a safe drop-in target for an ONNX
model later — same inputs, same output shape, different
implementation inside.

---

## Watch type (Men / Women)

`MainWindow.ui` adds a `WatchTypeComboBox` inside the existing
`WatchFrame` panel (below `BPH` and `Lift Angle`), defaulting to
**Men**. Selecting it fires `on_WatchTypeComboBox_currentIndexChanged`,
which sets `mWatchType` and is read into `DiagnosisInput::watch_type`
on every `DisplayResults()` call.

Witschi's table (p.15) gives a separate rate column per watch type —
only Gent's (Men) and Lady's (Women) are modeled here; Chronometer and
Chronograph rows exist in the source table but aren't exposed in the
UI:

| Watch type | Witschi rate band | This build |
|------------|-------------------|-------------|
| Gent's (Men, default) | -5..+15 s/d | Excellent: -5..+15, Good: -10..+10 |
| Lady's (Women) | -5..+25 s/d | Excellent: -5..+25, Good: -10..+20 |

Amplitude and beat-error bands are identical across all four rows in
Witschi's table, so only the rate thresholds change with watch type;
amplitude/beat-error stay as in the table below regardless of
selection. The Good-tier upper bound for Women (+20) isn't from any
source — it's widened by the same +10 s/d Witschi gives the Excellent
tier, to keep Good proportionally as loose relative to Excellent for
both types.

---

## Diagnosis states

Only the **rate** condition changes between Men and Women (see
[Watch type](#watch-type-men--women) above); amplitude and beat-error
conditions are identical for both, so they're listed once in the
shared columns.

### Men (default)

| State | Rate | Amplitude | Beat Error | Meaning |
|-------|------|-----------|------------|---------|
| **Unknown** | not yet valid | not yet valid | not yet valid | Not enough beats measured yet (e.g. right after Start, or sync just lost). Shown as `"DIAGNOSIS: Unknown"` (deliberately explicit, unlike the `"---"` placeholders elsewhere in `DisplayResults()`). |
| **Excellent** | `-5 ≤ rate ≤ +15` | `≥ 270°` | `≤ 0.5 ms` | Matches Witschi's "watch movement ok" criteria for a Gent's watch. Rate band is asymmetric (more tolerant of running fast than slow). |
| **Good** | `-10 ≤ rate ≤ +10` | `≥ 220°` | `≤ 0.8 ms` | Usable-but-not-optimal middle band (see sources below). |
| **Needs Service** | outside the Good range | outside the Good range | outside the Good range | Fails the Good band on at least one of the three — flag for inspection/regulation. |

### Women

| State | Rate | Amplitude | Beat Error | Meaning |
|-------|------|-----------|------------|---------|
| **Unknown** | not yet valid | not yet valid | not yet valid | Same as Men. |
| **Excellent** | `-5 ≤ rate ≤ +25` | `≥ 270°` | `≤ 0.5 ms` | Witschi's Lady's-watch rate band; amplitude/beat-error unchanged from Men (identical across all rows in Witschi's table). |
| **Good** | `-10 ≤ rate ≤ +20` | `≥ 220°` | `≤ 0.8 ms` | Same widening logic as Men's Good band, shifted by the same +10 s/d Witschi gives Women's Excellent tier. Not independently sourced. |
| **Needs Service** | outside the Good range | outside the Good range | outside the Good range | Same as Men. |

### Sources and rationale per threshold

No single source covers all three tiers (Excellent / Good / Needs
Service) consistently, so each threshold below is picked from
whichever source covers that specific band, with the conflicts and
judgment calls called out explicitly.

| Axis | Tier | Value | Source | Why |
|------|------|-------|--------|-----|
| Rate | Excellent | `-5..+15 s/d` | [Witschi Training Course](../.claude/skills/time-grapher/assets/Witschi-Training-Course.pdf) p.15, "Gent's watch" row | Witschi is the project's own reference device's documentation — most authoritative source available for an exact pass band. Asymmetric on purpose (Witschi's table is asymmetric for every watch type, not just Gent's). |
| Rate | Good | `-10 ≤ rate ≤ 10 s/d` | No single source gives a clean middle value here. [BeyondTheDial](https://www.beyondthedial.com/post/collector-guide-interpreting-timegrapher-results/) calls "within 2 s/d" acceptable and "many minutes/day" a failure, with nothing in between; [rotatewatches.com](https://rotatewatches.com/blogs/blog/what-is-beat-error-in-a-watch) and COSC figures (-4..+6 s/d) only cover certified-grade movements. `10` is a project judgment call: roughly double the Excellent band's width, comfortably between "a few seconds" (good) and "minutes" (bad). |
| Amplitude | Excellent | `≥ 270°` | [overview.md](../.claude/skills/time-grapher/references/project/overview.md) ("strong" band, 270–310°) | Witschi's own amplitude bands are split by test position (H/V) which this build doesn't track (see note above), so the project's single-value "strong" band is used instead, kept consistent with the Excellent tier's strictness. |
| Amplitude | Good | `≥ 220°` | [overview.md](../.claude/skills/time-grapher/references/project/overview.md) ("acceptable" band, 220–250°), corroborated by [BeyondTheDial](https://www.beyondthedial.com/post/collector-guide-interpreting-timegrapher-results/)'s "200°–240°: usable but needs servicing soon" | Two independent sources put the acceptable floor in the 200–240° range; 220° sits inside both, so no real conflict here. |
| Beat Error | Excellent | `≤ 0.5 ms` | [Witschi Training Course](../.claude/skills/time-grapher/assets/Witschi-Training-Course.pdf) p.14–15 ("watch movement ok": 0.0–0.5 ms) | Same Witschi pass band as the rate/excellent tier — kept together since Witschi presents them as one combined "ok" condition, not independent axes. |
| Beat Error | Good | `≤ 0.8 ms` | Conflicting sources: [rotatewatches.com](https://rotatewatches.com/blogs/blog/what-is-beat-error-in-a-watch) calls "up to 1.0 ms" still "very usable", while [BeyondTheDial](https://www.beyondthedial.com/post/collector-guide-interpreting-timegrapher-results/) calls anything "above 0.8 ms" a "serious issue". `0.8 ms` is the boundary value itself — it sits inside rotatewatches' "usable" range while staying just under BeyondTheDial's "serious" cutoff, so a value that either source would flag as bad is excluded. |

This is a fixed-threshold stand-in (step 1 of the AI feature track,
see below) — once labeled measurement data is available, the Good/
Needs-Service boundary in particular is a good candidate to replace
with a trained classifier rather than refine by hand further, since
the sources above disagree more than they agree.

Evaluation order is **Excellent → Good → else NeedsService**, so a
watch must clear the strict band on *all three* axes to be called
Excellent; failing any one of them drops it to the next tier.

---

## Integration point

`WatchDiagnostics` is called once per beat, inside the same place that
already renders the `RATE / AMPLITUDE / BEAT ERROR` text
(`MainWindow::DisplayResults(const Measurement &m)`,
[`src/ui/MainWindow.cpp`](../src/ui/MainWindow.cpp)) — `Measurement` is
the struct `MeasurementEngine` publishes once per processed block (MVC
Observer pattern; see `src/engine/Measurement.h`), and already carries
the rolling-average rate/amplitude/beat-error values this module needs:

```cpp
DiagnosisInput diagInput;
diagInput.rate_valid       = m.rateValid;
diagInput.rate_spd         = m.rateErrorSpd;
diagInput.amplitude_valid  = m.amplitudeValid;
diagInput.amplitude_deg    = m.amplitudeDeg;
diagInput.beat_error_valid = m.beatErrorValid;
diagInput.beat_error_ms    = m.beatErrorMs;
diagInput.watch_type       = mWatchType;  // set by WatchTypeComboBox, default Men

DiagnosisResult diagResult = mWatchDiagnostics.Evaluate(diagInput);
ui->DiagnosisLabel->setText(diagResult.label);
```

No new thread, no new timer, no new data path — it reuses the same
`Measurement` instance `DisplayResults()` already reads for the
`Results` label. Resource cost is a handful of comparisons per beat
(negligible next to the audio DSP pipeline).

A new `DiagnosisLabel` (`src/ui/MainWindow.ui`) sits beside the
existing `Results` label, on the same row, and shows the current state
text (`"DIAGNOSIS: Excellent"`, etc.) with a traffic-light background
color.

---

## Verification logging

`DisplayResults()` only logs to console when the diagnosis **level
changes** (not every beat), to avoid log spam while still proving the
classifier reacts to the live measurement stream:

```cpp
if (diagResult.level != mLastDiagnosisLevel)
{
    qInfo() << "[WatchDiagnostics]" << diagResult.label
             << "rate=" << diagInput.rate_spd
             << "amplitude=" << diagInput.amplitude_deg
             << "beatError=" << diagInput.beat_error_ms;
    mLastDiagnosisLevel = diagResult.level;
}
```

Sample output from a Playback-mode run cycling through several
recordings:

```
[WatchDiagnostics] "DIAGNOSIS: Excellent" rate= -3.5298 amplitude= 303.366 beatError= 0.0042305
[WatchDiagnostics] "DIAGNOSIS: Unknown" rate= -0.346345 amplitude= 0 beatError= 0
[WatchDiagnostics] "DIAGNOSIS: Needs Service" rate= -4.45735 amplitude= 203.437 beatError= 0.194884
[WatchDiagnostics] "DIAGNOSIS: Good" rate= 9.94129 amplitude= 226.538 beatError= 0.194333
```

This confirms all four states are reachable from real measurement
data, and that the `Unknown` state correctly appears whenever
a switch between recordings resets the rolling averages.

This `qInfo` call is independent of the per-frame performance
`Logger` described in [logging-design.md](logging-design.md) — it
fires once per state change, not once per frame, so it carries
negligible cost and is compiled into both `build/` and `build-log/`.
The difference between the two is only whether a console window
exists to show it: `build/` is a GUI-subsystem binary (no visible
console), while `build-log/` is built with `ENABLE_LOGGING=ON` and
therefore links as a console-subsystem binary where this output (and
the per-frame performance log) is visible.

---

## What this step does *not* do

- No trained model, no inference runtime (ONNX Runtime / llama.cpp) —
  those are later steps.
- Thresholds are fixed constants, not learned from data.
- No anomaly/time-series reasoning — each beat is judged independently.

These are exactly the gaps the next step (ONNX decision-tree
classifier trained on simulated/labeled measurement data) is meant to
fill, behind the same `WatchDiagnostics` interface.
