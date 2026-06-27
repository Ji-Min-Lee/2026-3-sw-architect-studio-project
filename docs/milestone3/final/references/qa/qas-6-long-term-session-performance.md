# QAS-6: Long-Term Session Performance — Priority 4 (Performance)

> Added M3 for the Long-Term Performance Graph feature (project plan §"Long-Term Performance Graph", Figure 14).
> Status: ✅ Verified analytically (2026-06-25). See EXP-07 for point-count derivation.

| Field | Detail |
|-------|--------|
| **Source** | End user running a multi-hour or multi-day aging test |
| **Stimulus** | A measurement session that has been running continuously for up to 7 days (≈ 50,400 measurements at 12 s averaging period, 3 series each) |
| **Artifact** | `LongTermTab` — in-memory data store (`QCPGraph` data) and Qt repaint pipeline on the Raspberry Pi GUI thread |
| **Environment** | Runtime — normal operation on Raspberry Pi; Qt main thread shared with all other visible tabs |
| **Response** | The system renders each new data point without a perceptible delay; the total number of plotted points across all three series remains bounded; no out-of-memory crash or frame freeze occurs |
| **Measure** | Total plotted points across 3 series after 7 days ≤ **3,000**; `QCustomPlot::replot()` call time ≤ **16 ms** throughout the session; heap growth of `LongTermTab` after 7 days ≤ **5 MB** |

## Tactics (Bass/CMK Ch.7 — Performance)

| Tactic (Ch.7) | How it appears in this system |
|---|---|
| **Manage Sampling Rate** | `mBucketSize` adaptive downsampling reduces point density as elapsed time grows — older data is aggregated into larger time buckets, bounding worst-case point count to ≈ 840 per series over 7 days. |
| **Bound Execution Times** | `QCustomPlot::replot()` is gated by `ADR-002` lazy rendering; the `LongTermTab` repaint is skipped when the tab is not visible, keeping GUI-thread frame time within the 16 ms budget. |
| **Limit Event Response** | New data points are appended at the averaging-period cadence (12 s); no per-beat repaint occurs on `LongTermTab`, decoupling render frequency from measurement frequency. |
