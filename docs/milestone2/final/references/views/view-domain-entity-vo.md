# Module View: Domain Entity / Value Object

This view shows the module structure of the Domain layer's `Measurement` aggregate and the three Value Objects that compose it. It is the primary tool for two qualities: **correctness** (AP-4 single-source consistency — every tab receives the same immutable instance) and **modifiability** (QAS-4 — Presentation components can be added or replaced with zero impact on domain types).

![Domain Entity / Value Object Module View](../../assets/view6-domain-entity-vo.png)

## Element Catalog

#### `Measurement` (aggregate root)
- Emitted once per `tg_process` call by `MeasurementEngine` and delivered to every tab through the Observer signal (`measurement(Measurement)`).
- Composed-of `SignalFrame signal`, `QVector<AcousticEvent> events`, and `WatchMetrics metrics`, plus sync/BPH flags (`synced`, `detectedBph`) and the `noSignal` flag (QAS-4: set when no A-event for ≥ 3 s).
- All tabs receive the same instance — **AP-4 single-source consistency guarantee**.
- Source: [`src/engine/Measurement.h`](../../../../../src/engine/Measurement.h)

#### `SignalFrame` «Value Object»
- Immutable snapshot of one audio block: envelope-filtered `pcm`, `threshold`, bipolar `hpfPcm`, pre-DSP `rawPcm`, `tickStart`/`tickEnd`, and `samplesPerSecond`.
- Consumed by waveform tabs (ScopePlot, SoundPrintTab). Immutable once produced by the DSP block.

#### `WatchMetrics` «Value Object»
- Rolling-average measurement outcomes: `rate` (s/day), `amplitude` (°), `beatError` (ms), each as `std::optional` (absent = not yet valid).
- Consumed by the numeric/trace tabs (TraceTab, VarioTab, BeatErrorTab). Immutable publish snapshot.

#### `AcousticEvent` «Value Object»
- Per-event data for one A(T1) or C(T3) acoustic event: `samplePos`, `isA`, rate-scatter point (`wrappedRateError`, `isTic`), escapement interval (`escapementMs`), and per-beat amplitude split.
- Consumed per-beat by scatter/diagnostic tabs (RateScopeTab, BeatErrorTab). Immutable once detected.

#### Relations

| Relation | From | To | Meaning |
|---|---|---|---|
| produces | `MeasurementEngine` | `Measurement` | One instance per processed block |
| composed-of | `Measurement` | `SignalFrame`, `WatchMetrics`, `AcousticEvent[]` | Aggregate ownership by value |
| uses (read-only) | Presentation tabs | `Measurement` | Received as `const &`; cannot mutate |

- **Immutability → correctness**: Tabs receive `Measurement` as `const &` and cannot mutate results — a display bug in one tab can never corrupt data another tab sees.
- **Domain isolation → modifiability**: VOs live entirely in the Domain layer. Adding or replacing a Presentation tab requires zero changes to `Measurement`, `SignalFrame`, `WatchMetrics`, or `AcousticEvent`.
- **Value semantics → safe cross-thread delivery**: Immutable value types allow the `QueuedConnection` from T2 to Qt main thread to copy a self-contained snapshot — no shared mutable state across threads.

## Related ADRs

- [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) — Observer that delivers `Measurement` to all 14 tabs

## Related Views

- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — where the Domain layer sits in the dependency hierarchy
- [C&C View: DSP Pipeline](view-cc-dsp-pipeline.md) — how `Measurement` is published via the Observer signal
- [Module View: IAudioSource Dependency Inversion](view-iaudiosource.md) — the sibling extension-point module view
