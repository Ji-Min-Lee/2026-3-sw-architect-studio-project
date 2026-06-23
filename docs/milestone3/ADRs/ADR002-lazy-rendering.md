# ADR 002: Reduce Computational Overhead — Lazy Rendering (Active Tab Only)

The TimeGrapher GUI has up to 11 graph tabs. In the baseline implementation every tab calls `replot()` on every beat event, regardless of whether the tab is visible. On Raspberry Pi 5, `plot_ms` consumed an average of 16 ms per beat — 79% of the 20 ms exec budget — leaving insufficient margin for the DSP pipeline and causing 43% deadline misses.

## Decision

Apply a **Lazy Rendering** strategy: only the currently visible (active) tab repaints on each beat. Non-visible tabs update their internal data model but skip the rendering step. When the user switches tabs, the newly visible tab immediately performs a catch-up repaint on the next event loop iteration to show the latest data without a noticeable delay.

## Rationale

The bottleneck is **rendering**, not computation. All 11 tabs share the same UI thread. Rendering hidden tabs burns GPU/CPU time that the user never sees.

| Metric | Before (all tabs render) | After (lazy rendering) |
|--------|:------------------------:|:----------------------:|
| `replot_count` per beat | 8.22 (macOS baseline) | 2.08 (macOS R1) |
| `replot_count` reduction | — | ↓ 75% |
| Expected RPi `plot_ms` saving | 16 ms / beat | ~14 ms saving |

## Rejected Alternatives

- **R2 — Timer-Decoupled 20 FPS**: A separate `QTimer` drives `replot()` at a fixed frame rate, decoupled from beat events. Not applied yet: no render spike was observed on macOS after R1, so R2 provides no additional benefit at this stage. Trigger criteria for re-evaluation after RPi R5: `replot_count` instantaneous max > 20, exec spike on tab-switch > 2× deadline, or UI freeze > 200 ms on switch.
- **R3 — Double-Buffer Async Rendering**: Pre-render each tab into a `QPixmap`; the UI thread composites the pixmap rather than re-rendering. Eliminates all render cost from the audio path but requires a shared-pixmap locking scheme and is significantly more complex. Beyond MVP scope.

## Status

Implemented. Validated by EXP-02 macOS R1 (75% replot reduction). RPi validation pending EXP RPi R5.

## Consequences

- **Positive**: Rendering load on the UI thread reduced by ~75%; expected ~14 ms `plot_ms` saving on RPi per beat.
- **Positive**: Checking visibility before rendering and refreshing on tab switch is a standard Qt idiom; low implementation risk.
- **Trade-off**: A tab switch shows a briefly stale view for one frame until the catch-up repaint fires on the next event loop iteration — imperceptible to the user.
- **Trade-off**: Rapid tab switching may queue multiple catch-up repaints. Not a concern at human interaction speed.

## Related ADRs
- [ADR 001 — DSP Offload Thread](ADR001-dsp-offload-thread.md)
- [ADR 003 — Four-Layer Architecture](ADR003-layered-architecture.md)
