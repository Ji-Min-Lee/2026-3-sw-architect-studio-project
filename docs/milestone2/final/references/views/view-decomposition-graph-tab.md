# Decomposition View: Graph Tab

This view decomposes the Presentation layer into its internal components, focusing on the `IGraphTab` interface and the `GraphTabManager` registry. It answers: "What must a developer implement to add a new graph tab?"

![Graph Tab Decomposition View](../../assets/view2-decomposition.png)

## Element Catalog

#### IGraphTab (interface)
- Abstract interface that every graph tab must implement.
- Key method: `updateData(const Measurement& m)` — called by `GraphTabManager` when a new measurement arrives from `MeasurementEngine`.
- Optionally overrides `showEvent(QShowEvent*)` to render a catch-up frame when the tab becomes visible (ADR-002 R1).
- No direct reference to Signal Processing or Acquisition layers.

#### GraphTabManager
- Owns the tab widget container and the list of registered `IGraphTab` instances.
- Receives `Measurement` signals from `MeasurementEngine` (Domain layer) via Qt Signal-Slot.
- Iterates registered tabs and calls `updateData()` on each.
- Single point of tab registration — a new tab is added here and nowhere else.

#### Concrete Tab Implementations (11 tabs)
Each implements `IGraphTab`:

| Tab Class | Display |
|-----------|---------|
| TraceDisplay | Waveform trace |
| VarioDisplay | Rate deviation (s/d) |
| BeatErrorDisplay | Beat error (ms) |
| AmplitudeDisplay | Amplitude (°) |
| SpectrumDisplay | Frequency spectrum |
| LongTermTrendDisplay | Long-term rate trend |
| SequenceDisplay | Beat sequence |
| DialDisplay | Analog-style dial |
| RadarDisplay | Multi-metric radar |
| DiagnosticDisplay | AI diagnostic output |
| ClassificationDisplay | Watch condition classification |

All 11 tabs were implemented by 2026-06-22.

## Behavior

**Normal data flow per beat event:**

```
MeasurementEngine (Domain)
    │  Qt Signal: measurement(Measurement)
    ▼
GraphTabManager::onMeasurement(m)
    │  for each tab in registry
    ├─▶ TraceDisplay::updateData(m)      [isVisible() guard — ADR-002]
    ├─▶ VarioDisplay::updateData(m)      [isVisible() guard]
    └─▶ … (all 11 tabs)
```

**Tab switch catch-up (ADR-002 R1):**

```
User switches to tab T
    → QTabWidget::currentChanged(T)
    → T::showEvent()
    → T::update()           ← single catch-up frame rendered
```

## Related ADRs

- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) — `isVisible()` guard in `updateData()`; `showEvent()` catch-up frame
- [ADR-004: R2 Timer-Decoupled Rendering](../adr/ADR-004-r2-timer-decoupled-rendering.md) — conditional replacement for ADR-002 if EXP-05 confirms R1 insufficient

## Related views

- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — parent view; shows where Presentation fits in the full layer stack
- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — shows the runtime path that produces the `Measurement` struct consumed here
