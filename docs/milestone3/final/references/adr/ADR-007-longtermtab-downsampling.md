# ADR-007: Time-Based Bucket Downsampling for LongTermTab

The Long-Term Performance Graph must record and display rate, amplitude, and beat error
over sessions lasting from minutes up to seven days. At the watch's 12 s averaging period
that yields ≈ 50,400 raw measurements per week. Storing and repainting every raw data point
in `QCustomPlot` on the Raspberry Pi's Qt GUI thread would cause unbounded frame-time growth
and eventual UI freeze — making extended aging tests impractical.

An earlier implementation (commit f6d78c1, 2026-06-11) used a point-count-based bucket
threshold: the bucket size was coarsened after N accumulated points. This approach was
correct in principle but produced inconsistent bucket boundaries depending on how many
measurements had been skipped or paused, making the displayed timeline non-uniform.

## Decision

We aggregate consecutive raw measurements into "buckets" inside `LongTermTab` using
**elapsed-time thresholds** rather than point counts. `mBucketSize` is set each cycle:

```
elapsed time        mBucketSize   plotted as
──────────────────  ───────────   ──────────────────────────────
0 – 5 min                1        every measurement → 1 point
5 min – 30 min          10        10 measurements averaged → 1 point
30 min – 2 hr           30        30 measurements averaged → 1 point
> 2 hr                  60        60 measurements averaged → 1 point
```

Each `Series` struct maintains a running `bucketSum` / `bucketN` accumulator.
A point is added to `QCPGraph` only when `bucketN == mBucketSize`; the plotted value
is the bucket average. The global running mean (µ) and σ band use all raw values,
not the downsampled points, so statistical overlays remain accurate regardless of
bucket size.

Time-based thresholds are determined once per measurement event from
`mTimeElapsed += pcm.size() / samplesPerSecond`, giving a threshold that scales with
actual audio clock time and is independent of missed beats or pauses.

## Rationale

**Why time-based, not point-count-based?**

The previous point-count approach (replaced in commit 48ddc0d, 2026-06-12) coarsened
the bucket after a fixed number of accumulated points. If measurements were missed or
the tab was paused, the effective wall-clock duration of a bucket was variable.
Time-based thresholds guarantee that each bucket always represents the same real-world
duration regardless of measurement gaps, making the x-axis (time in seconds) semantically
correct.

**Why these specific thresholds (5 min / 30 min / 2 hr)?**

The thresholds correspond to natural watch-observation horizons:
- **0–5 min** (`bucket = 1`): real-time inspection; every tick visible.
- **5–30 min** (`bucket = 10`): short warm-up; trends become visible.
- **30 min–2 hr** (`bucket = 30`): power-reserve effect window; 30-point average ≈ 6 min.
- **> 2 hr** (`bucket = 60`): full aging test; 60-point average ≈ 12 min.

**Worst-case point count** (7 days, bucket = 60, 12 s averaging period):
```
50,400 measurements / 60 = 840 points per series × 3 series = 2,520 total
```
QCustomPlot handles 100,000+ points before frame time noticeably degrades on desktop
hardware; 2,520 points on Raspberry Pi is well within the safe operating range.

**Rejected alternative — server-side ring buffer / LTTB downsampling**:
Largest-Triangle-Three-Buckets (LTTB) would preserve visual fidelity better than
simple averaging, but requires O(n) over the full history each cycle — too expensive
on the Pi's single GUI thread. The averaging approach is O(bucket_size) per cycle.

**Rejected alternative — fixed bucket size (e.g., always 60)**:
Would lose the live, per-beat detail in the first five minutes, where the user
most needs to see whether measurement is stable before committing to a long run.

## Status

Accepted (2026-06-12)

Implemented in `src/tabs/LongTermTab.h` / `LongTermTab.cpp` (commits f6d78c1, 48ddc0d).
Verified by QAS-6 experiment EXP-07.

## Consequences

**Positive**:
- Total plotted points after 7 days ≤ 2,520 — render time stays bounded and well below
  observable threshold on Raspberry Pi.
- Statistical overlays (mean dotted line, ±σ band) accumulate over all raw values, so
  accuracy of the summary statistics is not reduced by downsampling.
- Implementation is self-contained in `LongTermTab`; no changes to `MeasurementEngine`,
  `DSPWorker`, or any other tab. Bucket logic is ≤ 15 lines in `addPoint()` /
  `onMeasurement()`.
- Time-based thresholds make the x-axis semantically uniform; bucket boundaries align
  with real elapsed time independent of missed beats.

**Negative**:
- Fine-grained oscillations (< bucket duration) are invisible in the late-session view.
  A user who wants to inspect a specific hour must either zoom in on the graph or restart
  the session.
- `mBucketSize` is a scalar shared across all three series. If series receive measurements
  at different rates, bucket boundaries may not align perfectly between rate, amplitude,
  and beat-error subplots.
- The mean/σ overlay is updated only when a full bucket is committed, not on every raw
  measurement. For `bucket = 60` (> 2 hr phase), the overlay lags by up to 12 minutes.

## Related ADRs

- [ADR-006: BaseGraphTab Observer Pattern](ADR-006-basegraphtab-observer-pattern.md) —
  `LongTermTab` is a concrete `BaseGraphTab` subclass; ADR-007 logic lives inside
  `onMeasurement()`, the observer callback defined by ADR-006.
- [ADR-002: R1 Lazy Rendering](ADR-002-r1-lazy-rendering.md) —
  `LongTermTab::onMeasurement()` guards with `isVisible()` before calling
  `rpQueuedReplot`, consistent with the lazy rendering contract.

## Related views

- [Decomposition View: LongTermTab Downsampling](../views/view-longtermtab-downsampling.md) —
  structural view of the `LongTermTab` internals, bucket accumulation flow, and
  three-series layout.
- [Decomposition View: Graph Tab](../views/view-decomposition-graph-tab.md) —
  `LongTermTab` within the full Presentation-layer observer hierarchy.
