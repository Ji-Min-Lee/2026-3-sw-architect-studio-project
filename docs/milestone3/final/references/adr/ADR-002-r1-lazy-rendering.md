# ADR-002: Lazy Rendering — Skip replot() for Non-Visible Tabs (R1)

Even after T2 (ADR-001) offloads DSP to a separate thread, the UI thread still calls
`replot()` on every beat event for all graph tabs, regardless of visibility.
EXP-01 Run R2b measured an average of 8.22 `replot()` calls per beat with no visibility guard.
On Raspberry Pi 5, the `plot` step alone consumed 16ms — 79% of the 21ms exec deadline —
before T2 was applied. After T2, the replot count remains the primary variable that
determines whether the UI thread keeps pace with the audio rate.

## Decision

We will add an `isVisible()` guard at the entry of each tab's `updateData()` method.
Non-visible tabs update their internal data model but skip `replot()`.
On `showEvent()`, a `QTimer::singleShot(0)` schedules a single catch-up `replot()` to
render accumulated data before the tab becomes interactive.

```cpp
// Each IGraphTab implementation
void TraceDisplay::updateData(const Measurement& m) {
    store(m);           // always update data model
    if (!isVisible()) return;
    update();           // schedules paintEvent() — triggers replot()
}

void TraceDisplay::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    QTimer::singleShot(0, this, [this]{ update(); });  // catch-up frame
}
```

## Rationale

EXP-01 Runs R3 and R4 (macOS, 96kHz Playback mode, T2 + R1 applied):

| Run | Setup | replot/beat avg | Reduction |
|:---:|-------|:---------------:|:---------:|
| R2b | T2, no guard | 8.22 | baseline |
| R3 | T2 + R1, Scenario A (1 active tab) | 2.08 | **↓75%** |
| R4 | T2 + R1, Scenario B (tab switch) | 1.20 | **↓85%** |

The expected RPi impact: if `plot` consumed 16ms with replot_count 8.22, then at 1.20/beat
the proportional saving is approximately 14ms — which would bring `plot` contribution well
within the 21ms exec budget.

Tab switch UX: `QTimer::singleShot(0)` fires after the event loop processes the show event,
delivering a catch-up frame with no UI blocking. Stale display duration is less than
one beat period (< 21ms at 28,800 BPH), which is imperceptible.

### Options Considered

| Item | R1: Lazy Rendering | R2: Timer-Decoupled Rendering | R3: Double-Buffer Async Rendering |
|------|--------------------|-------------------------------|-----------------------------------|
| Core tactic | Repaint visible tab only | Fixed-FPS timer drives repaint | Render to QPixmap off-screen; UI blits finished pixmap |
| Implementation complexity | Low | Medium | High |
| exec reduction | High (single tab) | Medium (bounded FPS) | Very high (full audio-path isolation) |
| 11 tabs all open | Only 1 renders | N_active × (1/FPS) | Async — no audio-path impact |
| Data consistency risk | None | None | Requires lock on QPixmap sharing |
| M2 feasibility | ✅ Immediate | ✅ Feasible | ⚠️ Design change too large |

**R2 rejected**: Provides cleaner temporal decoupling but requires timer lifecycle management (start/stop with pipeline) and introduces over-rendering at low BPH. R1 preferred for M2 due to minimal change scope. R2 remains a viable upgrade path if EXP-04 reveals R1 insufficient under 11-tab full load — documented in [ADR-004](ADR-004-r2-timer-decoupled-rendering.md).

**R3 rejected**: Maximum isolation between audio path and rendering. QPixmap creation is UI-thread-only in Qt, requiring a worker thread for off-screen rendering and a separate blit step. Design change scope rated High; M2 deadline risk rated High. Deferred to post-M3 review.

## Status

Accepted

RPi impact confirmation via EXP-01 R5 scheduled: 2026-06-23.
Full 11-tab load test via EXP-04 scheduled: 2026-06-26.

## Consequences

**Positive**:
- replot_count reduced 75–85% — proportional reduction in UI thread CPU load
- Change scope is minimal: one guard line per tab `updateData()`, one `showEvent()` override
- No changes to Domain layer or below; IGraphTab interface unchanged
- Modifiability QA preserved: pattern applied uniformly to every new tab

**Negative**:
- Non-visible tabs display a stale frame on switch until the catch-up frame arrives
  (duration < 1 beat period, i.e., < 21ms at 28,800 BPH — imperceptible in practice)
- If all 11 tabs are simultaneously visible (e.g., multi-window layout), the guard provides
  no benefit — R2 or R3 would be needed in that scenario (not a current use case)
- Catch-up frame fires once on showEvent(); rapid tab cycling could produce visible
  one-frame lag on very fast switches (not observed in testing)
- RPi effectiveness depends on the ratio of active to total tabs — must be confirmed by EXP-04

## Supersedes

None. If EXP-04 confirms R1 insufficient under 11-tab full load, this ADR will be superseded
by a new ADR adopting R2 (Timer-Decoupled Rendering).
