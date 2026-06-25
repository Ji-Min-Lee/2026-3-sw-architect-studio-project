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

## Rationale

The project plan requires the Long-Term Performance Graph to "support longer test durations by reducing update frequency as elapsed time increases, allowing the system to monitor performance over many hours while remaining readable and efficient." Without a bounded point count, `QCPGraph` would accumulate tens of thousands of data points on the Qt GUI thread, causing frame time to grow without bound and eventually freezing the Raspberry Pi display.

The `mBucketSize` adaptive strategy (ADR-007) bounds the worst-case point count to ≈ 2,520 points for 7 days (≤ 840 per series), well within QCustomPlot's empirical limit of ~100,000 points before render time degrades. Memory is proportional to point count and remains negligible relative to the system's total heap.

## Related

[QA Priority Summary](README.md)

| Architecture | Rationale | Experiment | View |
|---|---|---|---|
| Time-Based Bucket Downsampling (`mBucketSize`) | [ADR-007: LongTermTab Downsampling](../adr/ADR-007-longtermtab-downsampling.md) | [EXP-07: Long-Term Aging Test](../experiments/exp-07-longterm-aging.md) | [Decomposition View: LongTermTab Downsampling](../views/view-longtermtab-downsampling.md) |
