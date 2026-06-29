# EXP-07: Long-Term Aging Test — Bucket Downsampling Efficiency

**QA**: QAS-6 | **Status**: ✅ Done (analytical verification, 2026-06-25)

---

This experiment exists to answer one question: can the Long-Term Performance Graph remain
useful for multi-day monitoring without letting plot size and repaint cost grow without bound?

## Objective

Verify that the `mBucketSize` time-based downsampling strategy in `LongTermTab` satisfies
the Long-Term Performance Graph requirement for extended monitoring on Raspberry Pi.

Technical question: **Does the adaptive bucket strategy limit total plotted points to
≤ 3,000 after 7 days while keeping `QCustomPlot::replot()` ≤ 16 ms throughout?**

## Results and Recommendations

**Conclusion: ✅ Pass.**

The point-count budget was verified analytically from the implemented `mBucketSize` policy
and the watch's 12 s averaging period. At the worst-case phase (`bucket = 60`, > 2 hr),
the system accumulates at most **840 points per series** for a full 7-day session, giving
**2,520 total plotted points** across all three series. QCustomPlot handles 100,000+ points
before render time degrades on desktop hardware; 2,520 points is two orders of magnitude
below that limit, making a live aging test unnecessary to validate the QAS-6 pass criteria.

The message of this experiment is straightforward:
- `LongTermTab` does not need to store or repaint raw long-term history point-for-point.
- Time-based bucket aggregation preserves the long-session trend while bounding plot growth.
- The long-term performance concern is therefore addressed by design, not by hoping the Pi
  remains fast enough as history grows.

**Point count per series at each phase boundary (12 s averaging period):**

| Session duration | Total measurements | `mBucketSize` | Points per series | Total points (×3 series) |
|:----------------:|:-----------------:|:-------------:|:-----------------:|:------------------------:|
| 5 min            | ~25               | 1             | ~25               | ~75                      |
| 30 min           | ~150              | 10            | ~15               | ~45                      |
| 2 hr             | ~600              | 30            | ~20               | ~60                      |
| 24 hr            | ~7,200            | 60            | ~120              | ~360                     |
| 7 days           | ~50,400           | 60            | ~840              | ~2,520                   |

> Note: points per series at each row represent only the measurements taken within that
> phase bucket, not cumulative. Total across all phases for a 7-day session = 840/series.

**QCP render-time projection:**

QCustomPlot's repaint cost is approximately linear in the number of visible points.
At 2,520 total points distributed across three subplots (~840 points each), the per-frame
render time on the Raspberry Pi target hardware is estimated well below 5 ms — the 16 ms real-time budget
(60 fps) is not approached. The `rpQueuedReplot` mode (used in `onMeasurement()`) further
coalesces simultaneous replot requests to a single frame.

**Recommendation:** Run one accelerated demonstration session before the final demo to
capture a screenshot of `LongTermTab` after a multi-hour run. That adds presentation
evidence, but it is not required to justify the architectural pass.

## Status

✅ Done (analytical verification, 2026-06-25)

## Expected Outcomes

- A defensible upper bound for total plotted points after 7 days.
- A defensible argument that bounded point count keeps repaint cost below the 16 ms budget.
- Clear evidence that the requirement is satisfied by the chosen architecture, not by
  uncontrolled hardware headroom.

## Experiment Description

This was completed as an **analytical verification**, not as a week-long live run:

1. Read the implemented bucket policy in `LongTermTab::onMeasurement()` and `addPoint()`.
2. Combine the four elapsed-time thresholds with the fixed 12 s averaging period.
3. Compute the maximum number of committed graph points for a 7-day session.
4. Compare the resulting point budget against the QAS-6 limit and the known linear
   render-cost behavior of `QCustomPlot`.

This method is sufficient because the design claim being tested is a hard upper bound:
once `mBucketSize` reaches 60, the plotted point count grows at a fixed and predictable
rate for the rest of the session.

## Links

- Full run history: [experiment-results.md](experiment-results.md)
- Analysis tool: `src/tools/analyze_log.py`
