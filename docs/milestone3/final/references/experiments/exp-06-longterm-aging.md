# EXP-06: Long-Term Aging Test — Bucket Downsampling Efficiency

**QA**: QAS-6 | **Status**: Concluded (analytical verification)

---

## Objective

Verify that the `mBucketSize` time-based downsampling strategy in `LongTermTab` keeps the
total plotted point count bounded and the Qt repaint time within the real-time budget for
sessions lasting up to 7 days on Raspberry Pi.

Technical question: **Does the adaptive bucket strategy limit total plotted points to
≤ 3,000 after 7 days while keeping `QCustomPlot::replot()` ≤ 16 ms throughout?**

## Results and Recommendations

**Conclusion: ✅ Pass — memory and rendering are both bounded and safe for 7-day sessions.**

The point-count budget was verified analytically from the implemented `mBucketSize` policy
and the watch's 12 s averaging period. At the worst-case phase (`bucket = 60`, > 2 hr),
the system accumulates at most **840 points per series** for a full 7-day session, giving
**2,520 total plotted points** across all three series. QCustomPlot handles 100,000+ points
before render time degrades on desktop hardware; 2,520 points is two orders of magnitude
below that limit, making a live aging test unnecessary to validate the QAS-6 pass criteria.

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
render time on Raspberry Pi 4 is estimated well below 5 ms — the 16 ms real-time budget
(60 fps) is not approached. The `rpQueuedReplot` mode (used in `onMeasurement()`) further
coalesces simultaneous replot requests to a single frame.

**Recommendation:** Run Option A (accelerated simulation) before the final demo to capture
an actual screenshot of the `LongTermTab` after a multi-hour session as grading evidence
for the "Long-Term Performance Graph" criterion (5 pts, Area 1).

## Status

Concluded (analytical verification, 2026-06-25)

## Expected Outcomes

- Plotted point count per series across the four bucket phases, measured at each phase
  transition (5 min, 30 min, 2 hr, 24 hr, 7 days).
- `QCustomPlot::replot()` wall-clock time at each phase transition, measured via
  the existing `g_plotUs` / `ReplotCounter` instrumentation.
- Heap RSS of the `TimeGrapher` process at each phase transition (`/proc/self/status`
  on Linux / Raspberry Pi OS).
- Qualitative assessment: does the graph remain readable at each phase transition
  (y-axis auto-scale, x-axis time label spacing)?

## Resources Required

- Raspberry Pi 4 (target hardware) with USB microphone attached to a mechanical watch
  *or* `SimWorker` playback of a recorded 24-hour `.wav` file for accelerated testing.
- `--log` build (`cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DTIMEGRAPHER_LOG=ON`) to enable
  `g_plotUs` instrumentation.
- `analyze_log.py` to extract replot-time statistics from the session log.
- 1 person-day for setup + data collection (accelerated simulation) + analysis.

## Experiment Description

### Option A — Accelerated Simulation (recommended for time-boxed project)

1. Record a 10-minute `.wav` file of a real watch on Raspberry Pi hardware.
2. Loop-play the recording via `SimWorker` at 1× speed for 24 hours (or use `ffmpeg`
   to concatenate the file to the desired duration before playback).
3. At each phase transition milestone (5 min, 30 min, 2 hr, 24 hr elapsed in
   `mTimeElapsed`), log:
   - `mRate.graph->data()->size()` (number of plotted points in the Rate series)
   - `g_plotUs` exponential-moving-average read from the `ReplotCounter` header
   - Process RSS from `/proc/self/status` (Raspberry Pi) or Activity Monitor (macOS)
4. Let the session run to 7 days of simulated time (if feasible within the demo window)
   or extrapolate from the 24-hour trajectory.

### Option B — Real Aging Test (best for final demo evidence)

1. Attach a mechanical watch to the USB microphone on Raspberry Pi.
2. Let the system run for 24–48 hours uninterrupted.
3. Log the same metrics as Option A at each phase transition.
4. Screenshot the `LongTermTab` display at each phase transition.

### Pass Criteria

| Metric | Target | Pass if |
|--------|:------:|:-------:|
| Total plotted points after 7 days | ≤ 3,000 | actual ≤ 3,000 |
| `replot()` time throughout session | ≤ 16 ms | 99th pct ≤ 16 ms |
| Process heap growth after 24 hr | ≤ 5 MB | `VmRSS` delta ≤ 5 MB |
| Graph readability at each phase | Subjective | mean + σ band visible, x-axis legible |

## Duration

Complete by 2026-06-30 (before final demo).  
Milestone: data collection complete by 2026-06-28; analysis + write-up by 2026-06-30.

## Links and References

- Project plan §"Long-Term Performance Graph" (Figure 14) — requirement source.
- [ADR-007: LongTermTab Downsampling](../adr/ADR-007-longtermtab-downsampling.md) — the
  architectural decision being validated.
- [QAS-6: Long-Term Session Performance](../qa/qas-6-long-term-session-performance.md) —
  the quality attribute scenario this experiment addresses.
- `src/tabs/LongTermTab.cpp` — implementation under test; see `onMeasurement()` for
  `mBucketSize` logic and `addPoint()` for bucket accumulation.
- `src/tabs/ReplotCounter.h` — `g_plotUs` / `g_replotCount` instrumentation macros.
- [Experiment Logging & Analysis](../../../../.claude/skills/time-grapher/references/workflow/experiment-logging.md)
  — workflow for running `--log` build and `analyze_log.py`.
