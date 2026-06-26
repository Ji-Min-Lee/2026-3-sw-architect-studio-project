# Graph Tab Module Uses View

This view decomposes the Presentation layer into its internal components, focusing on the `BaseGraphTab` interface and the `MainWindow` tab registry. It answers: "What must a developer implement to add a new graph tab?"

[Open draw.io source](../../assets/view2-graph-tab-observer-contract.drawio)

![Graph Tab Observer Contract View](../../assets/view2-graph-tab-observer-contract.png)

> No compile-time link `MeasurementEngine → BaseGraphTab`. `SessionController` is the wiring coordinator — it stores `mObserverTabs` and applies `connect()` at session start. Per-beat delivery remains Qt signal-slot only.

## Element Catalog

#### BaseGraphTab (abstract class / Observer)
- Abstract C++ base class that every graph tab must implement.
- Key slot: `onMeasurement(const Measurement& m)` — wired from `MeasurementEngine::measurementReady` via `SessionController`.
- `isVisible()` guard inside `onMeasurement()` skips `replot()` for non-visible tabs (ADR-002 R1), reducing replot/beat from 8.22 to 1.20 (↓85%).
- `showEvent()` triggers `replotAll()` for a catch-up frame when the tab becomes visible.
- No direct reference to Signal Processing or Acquisition layers.

#### MainWindow (tab registry + results observer)
- Owns `mAllTabs` and `registerTab()` — single point of tab registration.
- Calls `SessionController::connectObservers(mAllTabs, this, onMeasurementReady)` at startup.
- `onMeasurementReady(m)` updates the Results label (second Observer on the same signal).

#### SessionController (wiring coordinator)
- Stores observer list from `connectObservers()`; applies `connect()` in `startSourceThread()`.
- Not in the per-beat data path after wiring completes.
- `MeasurementEngine` has zero compile-time knowledge of tabs — emits signal only; `SessionController` wires the abstract `onMeasurement` slot → Subject and Observer decoupled at build time.

#### Concrete Tab Implementations (14 tabs)
Each extends `BaseGraphTab`:

| Group | Tab Class | Display |
|-------|-----------|---------|
| Signal / Scope | `TraceTab` | Raw waveform trace |
| | `RateScopeTab` | Rate deviation scope |
| | `SweepScopeTab` | Sweep oscilloscope |
| | `FilterScopeTab` | Filtered signal scope |
| | `BeatNoiseScopeTab` | Beat noise scope |
| | `SoundPrintTab` | Acoustic fingerprint |
| Measurement | `VarioTab` | Rate deviation (s/d) |
| | `BeatErrorTab` | Beat error (ms) |
| | `EscapementTab` | Escapement analysis |
| | `LongTermTab` | Long-term rate trend |
| | `SequenceTab` | Beat sequence |
| Analysis / AI | `SpectrogramTab` | Frequency spectrogram |
| | `WaveformCompTab` | Waveform comparison |
| | `RadarChartTab` | Multi-metric radar |

Adding a new tab = 1 new subclass + 3 lines in `MainWindow` → zero changes to `MeasurementEngine` or any other tab (ADR-006).

Observer contract compliance validated by AI-generated unit tests (see Behavior section).

## Behavior

[Open draw.io source](../../assets/view2-measurement-broadcast-to-graph-tabs.drawio)

![Measurement Broadcast to Graph Tabs View](../../assets/view2-measurement-broadcast-to-graph-tabs.png)

This UML sequence diagram shows the narrow behavior that matters for this view: one published `Measurement` event fan-outs through the observer contract to all registered graph tabs and to the summary display. It exists to explain why Observer supports the project goal of low-cost tab extension.

The trace makes three points:

1. `MainWindow` and `SessionController` perform setup once by wiring the observer list.
2. `MeasurementEngine` publishes one `Measurement` event without referencing any concrete tab class.
3. A UML `loop` frame makes the fan-out explicit: each registered `BaseGraphTab` subscriber receives the same event, so adding a new tab means adding one subscriber rather than modifying the publisher.

Representative beat-event trace:

```
MainWindow
    -> SessionController::connectObservers(mAllTabs, this, onMeasurementReady)

SessionController
    -> MeasurementEngine::startSourceThread() / connect(...)

loop [for each registered tab]
    MeasurementEngine
        ->> BaseGraphTab::onMeasurement(m)

MeasurementEngine
    ->> ResultsSummary::onMeasurementReady(m)
```

Observer contract validation: AI-generated unit tests verify that every registered tab receives the shared `Measurement` snapshot and honors the `BaseGraphTab` contract.

Measured results: → [EXP-03: Observer Pattern Compliance](../experiments/exp-03-extensibility-observer-pattern.md)

## Related ADRs

- [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) — rationale for the `BaseGraphTab` interface and `registerTab()` registration pattern
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard in `onMeasurement()`; `showEvent()` catch-up via `replotAll()`
- [ADR-004: R2 Timer-Decoupled Rendering](../adr/ADR-004-r2-timer-decoupled-rendering.md) — conditional replacement for ADR-002 if EXP-04 confirms R1 insufficient

## Related views

- [Layered and Module Decomposition View](view-layered-4layer.md) — parent view; shows where Presentation fits in the full layer stack
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — shows the runtime path that produces the `Measurement` struct consumed here
- [Module View: Domain Entity / Value Object](view-domain-entity-vo.md) — shows the structure of the `Measurement` snapshot delivered through this observer design
