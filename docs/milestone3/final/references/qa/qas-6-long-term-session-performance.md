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
