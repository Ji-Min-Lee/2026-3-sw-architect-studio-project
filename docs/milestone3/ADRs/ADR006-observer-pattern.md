# ADR-006: BaseGraphTab Observer Pattern

## Status

Accepted (2026-06-22)

## Context

The system displays 14 concurrent graph tabs, each visualising a different aspect of watch measurement data (Rate, Amplitude, Beat Error, Spectrogram, etc.). Without a shared interface and registration mechanism, adding a new tab would require touching multiple files across the Presentation layer — duplicating connection boilerplate and creating implicit coupling between `MainWindow` and every concrete tab type.

The project required all 14 tabs to be implemented in parallel by a small team within a single sprint (W2 Sprint 1, 2026-06-09 to 2026-06-10). Without a stable contract, concurrent development would produce integration conflicts at the tab boundary.

Two problems required a solution at the same time:
1. **Delivery contract** — how does `MeasurementEngine` notify tabs when a new beat is computed?
2. **Registration contract** — where is the single place to wire a new tab into the system?

## Decision

We define `BaseGraphTab` as the abstract C++ base class for all graph tab widgets. It establishes two contracts:

**Observer contract**: `MeasurementEngine` emits `measurementReady(Measurement)`. `SessionController` wires this signal to every tab's `onMeasurement` slot at session start using `Qt::QueuedConnection`. Each tab must implement `onMeasurement()`. `MeasurementEngine` has no compile-time knowledge of any tab type.

**Lazy Rendering contract (from ADR-002)**: `onMeasurement()` always accumulates incoming data. It skips the render call when the tab is not currently visible. On tab switch, `showEvent()` fires a catch-up render immediately. The `showEvent()` logic lives once in `BaseGraphTab`, not in each subclass.

**Registration contract**: `MainWindow::registerTab()` is the single point of registration. A new tab requires exactly three changes in `MainWindow`: declare a member pointer, construct and register the tab, include the tab header. No changes to `MeasurementEngine`, `SessionController`, `DSPWorker`, or any other existing tab.

## Rationale

`BaseGraphTab` defines the boundary between the tab and the rest of the system:

| Concern | Owner |
|---------|-------|
| Measurement data delivery | `MeasurementEngine` → `onMeasurement()` |
| Rendering decision | Each tab — visibility guard inside `onMeasurement()` |
| Catch-up on tab switch | `BaseGraphTab::showEvent()` — implemented once in the base |
| Tab registration | `MainWindow::registerTab()` — one call per tab, no other edits |

**Adding a new tab — change count**:

| Action | File count |
|--------|:----------:|
| Implement BaseGraphTab subclass | 2 (header + source) |
| Register in MainWindow | 1 (3 lines: member, include, registration call) |
| **Total** | **≤ 3 files** |

Zero changes to `MeasurementEngine`, `SessionController`, `DSPWorker`, or any other tab. All 14 existing tabs follow this pattern, verified by EXP-04.

**Why `showEvent()` in the base class**: The catch-up render logic is identical for all tabs. Placing it in `BaseGraphTab` eliminates 14 copies of the same Qt timer pattern and guarantees that every future tab gets the Lazy Rendering contract automatically.

**Why not a separate `GraphTabManager` class**: `MainWindow` already owns the `QTabWidget`. Extracting a separate manager adds indirection without reducing coupling — the `registerTab()` template achieves the same single-point-of-registration goal with less complexity.

## Consequences

**Positive**:
- Parallel tab development with zero merge conflicts at the tab boundary — each developer works in their own header and source file
- Lazy Rendering (ADR-002) is enforced at the base class level; the catch-up logic is automatic for all tabs including future ones
- The pause contract (`mPaused` flag) is centralised in `BaseGraphTab`; tabs honour it by checking `isPaused()` inside `onMeasurement()`
- Observer wiring in `SessionController` is uniform across all tabs — no per-tab special cases

**Negative**:
- `MainWindow` still owns the tab registry (`mAllTabs`); it is not a separate, independently testable `GraphTabManager`
- All 14 tab slots fire on the Qt main thread per beat event. If all 14 tabs are visible simultaneously at high BPH, the visibility guard provides no benefit — R2 timer-decoupled rendering is the contingency
- The base class cannot enforce at compile time that each tab correctly implements the visibility guard inside `onMeasurement()` — this relies on developer discipline

## Verification (EXP-04)

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 across all 14 tabs |
| Signal Processing / Acquisition refs from Presentation | 0 | ✅ 0 — DSM verified |
| Observer contract compliance (all 14 tabs) | 100% | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

## Related ADRs
- [ADR-002 — Lazy Rendering](ADR002-lazy-rendering.md) — visibility guard and `showEvent()` catch-up; BaseGraphTab provides the structural home for that pattern
- [ADR-003 — Four-Layer Architecture](ADR003-layered-architecture.md) — `BaseGraphTab` and all 14 tabs live in Presentation; they are only allowed to reference Domain

## Related Views
- [Graph Tab Decomposition View](../architecture/graph-tab-view.md)
- [Module View](../architecture/module-view.md)
