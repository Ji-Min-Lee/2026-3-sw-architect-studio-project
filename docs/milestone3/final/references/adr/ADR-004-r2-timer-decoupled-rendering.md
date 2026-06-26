# ADR-004: R2 Timer-Decoupled Rendering (Conditional Replacement for ADR-002)

[ADR-002](ADR-002-r1-lazy-rendering.md) (R1 Lazy Rendering) was accepted based on macOS
validation: replot_count reduced 75–85% with 1 active tab. However, ADR-002 explicitly
notes a limitation: if all tabs are simultaneously visible, the isVisible() guard provides
no benefit.

EXP-04 (2026-06-26) measures Qt rendering FPS on RPi 5 under full 11-tab load with R1 applied.
If deadline miss rate > 0% persists under full-tab load, R1 is insufficient and a rendering
strategy with tighter exec-path decoupling is required.

R2 addresses this by removing the beat-event trigger from rendering entirely:
a fixed-interval Qt timer drives `update()` independently of the DSP pipeline.

## Decision

We will adopt R2 (Timer-Decoupled Rendering) **only if** EXP-04 confirms that R1 is
insufficient under 11-tab full load on RPi 5 (deadline miss > 0%).

If adopted, R2 replaces R1. The `isVisible()` guard is removed. A `QTimer` (target: 20 FPS,
50ms interval) fires `update()` on all visible tabs at a fixed rate. Beat events update
the data model only — no `update()` call from the audio path.

```
BeatEvent → IGraphTab::updateData(m) → store in data model (no update() call)
QTimer(50ms) → all visible tabs → update() → paintEvent() → replot()
```

## Rationale

R1 limitation confirmed by EXP-01 analysis: replot_count at 8.22/beat with no guard.
With R1, 1-tab load drops to 1.20–2.08. Under 11 simultaneously visible tabs, if all tabs
call `update()` per beat, total rendering load scales with tab count × BPH.

R2 breaks the beat-count dependency entirely: rendering rate is fixed at 20 FPS regardless
of BPH or tab count. exec budget is never consumed by `update()` calls.

Trade-off vs R1:
- R2 requires timer lifecycle management (start on pipeline open, stop on close)
- At low BPH (e.g., 18,000 BPH = 5 beats/sec), 20 FPS timer over-renders vs. R1
- Tab switch UX is simpler under R2 (timer ensures refresh without `showEvent()` override)
- Code change scope is wider than R1: `TabManager` (timer control) + each tab (remove update() from updateData)

Rejected alternative — R3 (Double-Buffer Async Rendering): maximum exec isolation but
requires off-screen QPixmap rendering on a worker thread. Qt restricts QPixmap creation
to the UI thread — R3 needs significant redesign. Deferred to post-M3 review.

## Status

Proposed

Conditional on EXP-04 result (2026-06-26):
- If EXP-04 confirms R1 sufficient (0% deadline miss under 11-tab load): **this ADR is withdrawn; ADR-002 remains Accepted**
- If EXP-04 shows R1 insufficient (deadline miss > 0% persists): **this ADR transitions to Accepted; ADR-002 is Superseded**

## Consequences

**If adopted (ADR-002 superseded)**:
- ADR-002 status changes to Superseded; this file becomes the active decision
- `isVisible()` guards removed from all `updateData()` implementations
- `showEvent()` overrides removed
- `QTimer` added to `GraphTabManager` — start/stop lifecycle tied to pipeline state
- Rendering rate decoupled from BPH: consistent 20 FPS regardless of watch speed
- `updateData()` implementations simplified (data model update only, no UI call)

**If not adopted (R1 confirmed sufficient)**:
- This ADR remains Proposed/Withdrawn — no implementation
- ADR-002 continues as the active rendering decision
