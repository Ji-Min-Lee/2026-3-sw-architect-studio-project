# QAS-6: Long-Term Session Performance — Priority 6 (Performance)

This scenario protects the Long-Term Performance Graph from one failure mode: a multi-day
session keeps appending points until the GUI becomes slow or unreadable.

| Field | Detail |
|-------|--------|
| **Source** | End user running a multi-hour or multi-day aging test |
| **Stimulus** | A measurement session that has been running continuously for up to 7 days (≈ 50,400 measurements at 12 s averaging period, 3 series each) |
| **Artifact** | `LongTermTab` — in-memory data store (`QCPGraph` data) and Qt repaint pipeline on the Raspberry Pi GUI thread |
| **Environment** | Runtime — normal operation on Raspberry Pi; Qt main thread shared with all other visible tabs |
| **Response** | The system renders each new data point without a perceptible delay; the total number of plotted points across all three series remains bounded; no out-of-memory crash or frame freeze occurs |
| **Measure** | Total plotted points across 3 series after 7 days ≤ **3,000**; repaint cost remains below the **16 ms** interaction budget because visible point count stays bounded and repainting is further reduced by ADR-002; heap growth attributable to plotted history remains bounded for the 7-day session |

## Tactics (Bass/CMK Ch.7 [Bass21] — Performance)

| Tactic (Ch.7) | How it appears in this system |
|---|---|
| **Manage Sampling Rate** | `mBucketSize` adaptive downsampling reduces point density as elapsed time grows — older data is aggregated into larger time buckets, bounding worst-case point count to ≈ 840 per series over 7 days. |
| **Bound Execution Times** | `QCustomPlot::replot()` is gated by `ADR-002` lazy rendering; the `LongTermTab` repaint is skipped when the tab is not visible, keeping GUI-thread frame time within the 16 ms budget. |
| **Limit Event Response** | New data points are appended at the averaging-period cadence (12 s); no per-beat repaint occurs on `LongTermTab`, decoupling render frequency from measurement frequency. |

## Traceability

- **Requirement**: [Functional Requirements](../requirements/functional-requirements.md#long-term-performance-graph)
  — "Record and display rate, amplitude, and beat error change over an extended period"
  and "Benefit from reduced update frequency as elapsed time increases".
- **Primary ADR**: [ADR-007](../adr/ADR-007-longtermtab-downsampling.md) — defines the
  time-based bucket policy that bounds plot growth.
- **Supporting ADR**: [ADR-002](../adr/ADR-002-r1-lazy-rendering.md) — ensures the tab
  does not spend repaint cost when not visible.
- **Validation**: [EXP-07](../experiments/exp-07-longterm-aging.md) — proves the worst
  case remains bounded at **840 points per series / 2,520 total points** after 7 days.
- **Architectural view**: [LongTermTab Downsampling Decomposition View](../views/view-longtermtab-downsampling.md)
  — shows the internal structure and bucket-accumulation flow that realizes ADR-007.
- **Risk note**: the concern addressed here is "multi-day sessions may accumulate unbounded
  plot points and degrade GUI responsiveness." In the M3 package it is tracked through this
  QAS-6 chain rather than as a standalone `TR-*` entry in [risks.md](../risks.md).

## References

- [Bass21] L. Bass, P. Clements, R. Kazman. *Software Architecture in Practice*, Fourth Edition. Addison-Wesley, 2021.
