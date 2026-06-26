# ADR-006: BaseGraphTab Observer Pattern and Tab Registration (AP-3)

The system displays 14 concurrent graph tabs, each visualising a different aspect of the
watch measurement data. Without a shared interface and registration mechanism, adding a new
tab would require touching multiple files across the UI layer — duplicating connection
boilerplate and creating implicit coupling between `MainWindow` and every concrete tab type.

The project plan required 14 tabs to be implemented in parallel by a small team within a
single sprint (W2 Sprint 1, 6/9–6/10). Without a stable contract, concurrent development
would produce integration conflicts at the tab boundary.

## Decision

We define `BaseGraphTab` as the abstract base class (C++/Qt) for all graph tab widgets.
It establishes two contracts:

**Observer contract** — `MeasurementEngine` notifies all tabs via Qt signal-slot:
```cpp
// BaseGraphTab — pure virtual slot; every tab must implement
public slots:
    virtual void onMeasurement(const Measurement &m) = 0;

// R1 Lazy Rendering contract (ADR-002):
// onMeasurement() accumulates data always; skips replot() when !isVisible().
// showEvent() fires replotAll() via QTimer::singleShot(0) for a catch-up frame.
protected:
    void showEvent(QShowEvent *e) override {
        QWidget::showEvent(e);
        if (!mPaused) QTimer::singleShot(0, this, &BaseGraphTab::replotAll);
    }
    virtual void replotAll() {}
```

**Registration contract** — `MainWindow::registerTab<T>()` is the single point of tab
registration. It adds the tab to the `QTabWidget` UI and to `mAllTabs` (the observer list):

```cpp
// MainWindow.cpp — adding a new tab requires exactly these 3 lines:
// 1. Member declaration in MainWindow.h  (e.g. TraceTab *mTraceTab;)
// 2. Construction + registration here    (1 line)
// 3. Include in MainWindow.cpp           (1 line)
mTraceTab = registerTab(new TraceTab(this), "Trace");

// registerTab implementation:
template<typename T>
T* MainWindow::registerTab(T *tab, const QString &label) {
    ui->GraphicsTabWidget->addTab(tab, label);
    mAllTabs.append(tab);
    return tab;
}
```

`SessionController::startSourceThread()` wires `MeasurementEngine → BaseGraphTab::onMeasurement`
for every tab in `mAllTabs` at session start:

```cpp
for (BaseGraphTab *tab : mObserverTabs)
    connect(eng, &MeasurementEngine::measurementReady,
            tab, &BaseGraphTab::onMeasurement, Qt::QueuedConnection);
```

## Rationale

**Modifiability goal**: the 14-tab requirement and sprint-parallel development made a
stable interface non-negotiable. `BaseGraphTab` defines the boundary:

| Concern | Owner |
|---------|-------|
| Measurement data delivery | `MeasurementEngine` → `onMeasurement()` |
| Rendering decision | Each tab — `isVisible()` guard, `replotAll()` |
| Catch-up on tab switch | `BaseGraphTab::showEvent()` — once, in the base |
| Tab registration | `MainWindow::registerTab()` — one call, no other edits |

**Adding a new tab — file change count**:

| Action | File | Lines |
|--------|------|:-----:|
| Implement `BaseGraphTab` subclass | `NewTab.h` + `NewTab.cpp` | N/A |
| Register in MainWindow | `MainWindow.cpp` — 1 `registerTab` call | 1 |
| Declare member | `MainWindow.h` — 1 pointer | 1 |
| Include header | `MainWindow.cpp` — 1 `#include` | 1 |
| **Total outside the tab itself** | | **3 lines** |

Zero changes to `MeasurementEngine`, `SessionController`, `DSPWorker`, or any other tab.
This is verified by the implementation: all 14 tabs follow this pattern as of 2026-06-22.

**`showEvent()` in the base class**: catch-up replot logic is identical for all tabs.
Placing it in `BaseGraphTab` rather than repeating it in each subclass avoids 14 copies of
the same QTimer pattern and ensures new tabs get the R1 contract automatically.

Rejected alternative — separate `GraphTabManager` class: would require an additional
indirection between `MainWindow` and the tab widget. Since `MainWindow` already owns the
`QTabWidget`, extracting a manager adds complexity without reducing coupling. The
`registerTab()` template achieves the same single-point-of-registration goal inline.

## Architectural Pattern Classification (Bass/CMK Ch.8 §8.4)

This ADR implements the **Publish-Subscribe pattern** as described in
Bass, Clements & Kazman *Software Architecture in Practice* (4th ed., Ch.8 §8.4 p.129):

> *"Adding or changing subscribers requires only registering for an event and causes
> no changes to the publisher."*

`MeasurementEngine` is the **publisher** component. All 14 `BaseGraphTab` implementations
are **subscriber** components. The Qt Signal-Slot mechanism (`Qt::QueuedConnection`) acts
as the **event bus**. `MeasurementEngine` has zero compile-time knowledge of any tab.

**Testability tradeoff and mitigation**: Ch.8 p.130 explicitly notes:

> *"Use of the publish-subscribe pattern can negatively impact testability. Seemingly
> small changes in the event bus can have a wide impact on system behavior."*

We mitigate this risk by three means, each corresponding to a Ch.12 testability tactic:

| Risk | Mitigation | Ch.12 Tactic |
|---|---|---|
| Nondeterministic event delivery order | `Qt::QueuedConnection` is FIFO-ordered on the main thread — delivery order is deterministic per beat cycle | **Limit Nondeterminism** |
| Hidden cross-layer signal wiring | DSM `#include` trace confirms zero Presentation→Signal Processing references; signal is wired in `SessionController` only | **Limit Structural Complexity** |
| Observer contract correctness across 14 tabs | `EXP-03 TestAddedTabs 20/20` verifies that every registered tab receives and processes the `Measurement` signal correctly | **Specialized Interfaces** (observable via test) |

## Status

Accepted (2026-06-22)

All 14 tabs implemented and registered using this pattern. Verified: no tab touches
`MeasurementEngine`, `SessionController`, or any layer below Presentation.

## Consequences

**Positive**:
- Parallel tab development across sprint team members with zero merge conflicts at the
  tab boundary — each developer works in their own `.h`/`.cpp` pair
- R1 Lazy Rendering (ADR-002) is enforced at the base class level; `showEvent()` catch-up
  is automatic for all tabs including future ones
- Pause contract (`mPaused` flag) is centralised in `BaseGraphTab`; tabs honour it by
  checking `isPaused()` in `onMeasurement()`
- Observer wiring in `SessionController` is O(n) over `mAllTabs` — no per-tab special
  cases in the connection loop

**Negative**:
- `MainWindow` still owns the tab registry (`mAllTabs`); it is not a separate, testable
  `GraphTabManager`. Tab registration is coupled to UI construction order in `MainWindow`
- `onMeasurement()` is a slot wired via `Qt::QueuedConnection`, so all 14 tab slots fire
  on the Qt main thread per beat event. Under 11 simultaneously visible tabs at high BPH,
  the `isVisible()` guard provides no benefit — R2 (ADR-004) is the contingency
- Each tab must explicitly check `isVisible()` inside `onMeasurement()` and call
  `replotAll()` inside a custom `replotAll()` override — the base class cannot enforce this
  at compile time

## Related ADRs

- [ADR-002: R1 Lazy Rendering](ADR-002-r1-lazy-rendering.md) — `isVisible()` guard and
  `showEvent()` catch-up; this ADR provides the structural home for that pattern
- [ADR-004: R2 Timer-Decoupled Rendering](ADR-004-r2-timer-decoupled-rendering.md) —
  contingency if full-tab load (EXP-04) reveals R1 insufficient

## Related views

- [Decomposition View: Graph Tab](../views/view-decomposition-graph-tab.md) — structural
  decomposition of the Presentation layer; this ADR is the rationale for that structure
- [Layered View: 4-Layer Allowed-to-Use](../views/view-layered-4layer.md) — `BaseGraphTab`
  and all tab implementations live in the Presentation layer; allowed to reference Domain only
