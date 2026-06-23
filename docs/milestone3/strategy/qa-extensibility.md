# QA Strategy: Extensibility

## Requirement

The system shall support adding new measurements, filters, graphs, and display modes
without major redesign of existing code. Because the team begins with a baseline GUI and
is expected to add substantial new capabilities within the project timeline, enhancements
must be implementable incrementally, testable independently, and addable with limited
impact on existing modules.

**Reference**: [QAS-4: Extensibility / Modifiability](../../milestone2/final/references/qa/qas-4-extensibility-modifiability.md)

---

## Architectural Decision: Observer Pattern — New Graphs Subscribe to MeasurementModel via BaseGraphTab

### Rationale

The baseline GUI hard-codes a fixed set of graph tabs. Each new graph added during the
project requires a new tab widget and needs access to the same computed measurement data
(rate, amplitude, beat error, timestamps) that existing graphs already display.

If new graphs reach directly into the processing pipeline or call into other graph modules
to obtain data, adding one graph creates dependencies across unrelated modules. A change
to the measurement data format then requires updating every graph that touches it.

Using the Observer pattern, `MeasurementModel` is the single source of measurement data.
Any new graph module registers as an observer via `BaseGraphTab` and receives measurement
updates via a stable interface. Adding a new graph requires:

1. Creating a new widget class that implements `BaseGraphTab`.
2. Registering it in `MainWindow`'s tab registry.
3. No changes to any existing graph module or the processing pipeline.

The Correctness / Internal Consistency decision shares the same Observer structure.
Extensibility and internal consistency are achieved by the same architectural choice,
not two separate mechanisms.

### Trade-offs

| | Direct coupling to pipeline | Observer / BaseGraphTab (chosen) |
|--|----------------------------|--------------------------------------|
| Adding a new graph | Must modify pipeline or existing graphs | New class + registration only |
| Impact on existing code | High — pipeline interface changes affect all consumers | None — existing observers unaffected |
| Internal consistency | Not guaranteed | Guaranteed — all observers share one source |
| Complexity | Low initially | Requires defining BaseGraphTab and observer interface upfront |
| Testability | Graph logic mixed with data access | Graph rendering testable independently of data source |

### Alternatives Considered

**Pub/sub message bus (e.g., Qt signals/slots with a central event bus)**
- All modules publish and subscribe to typed events on a shared bus, with no explicit
  dependency on `MeasurementModel`.
- Not applied because: for this project's scale, a message bus introduces indirection
  without benefit. Debugging event flow is harder than tracing direct observer
  registrations. Qt signals/slots are used as the *mechanism* for observer notifications
  while keeping `MeasurementModel` as the explicit owner of state.

**Pull model — graphs poll MeasurementModel on a timer**
- Each graph queries `MeasurementModel` at its own refresh rate rather than being notified.
- Not applied because: polling at different rates means graphs may display values from
  different beats simultaneously, violating internal consistency.

### ADRs

| ADR | Decision | Status |
|-----|----------|--------|
| [ADR-006: BaseGraphTab Observer Pattern](../../milestone2/final/references/adr/ADR-006-basegraphtab-observer-pattern.md) | Introduce `BaseGraphTab` interface; all tabs register and receive updates via observer notification | ✅ Applied |
| [ADR-005: IAudioSource Dependency Inversion](../../milestone2/final/references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) | Audio source extensibility — new source types addable without modifying the pipeline | ✅ Applied |

### Views

| View | What it shows | Status |
|------|--------------|--------|
| [Decomposition View: Graph Tab](../../milestone2/final/references/views/view-decomposition-graph-tab.md) | `BaseGraphTab` interface; what a developer must implement to add a new tab | ✅ Exists |
| [Module View: IAudioSource](../../milestone2/final/references/views/view-iaudiosource.md) | AS-IS vs TO-BE audio source extensibility structure | ✅ Exists |
| [Layered View: 4-Layer](../../milestone2/final/references/views/view-layered-4layer.md) | Allowed-to-use rules; new graph tabs added to Presentation layer only | ✅ Exists |

---

## Experiments

| ID | Description | Status |
|----|-------------|--------|
| [EXP-04](../../milestone2/final/references/experiments/exp-04-extensibility-observer-pattern.md) | Observer Pattern Compliance — Tab Extension Cost Measurement (≤ 3 file changes, zero cross-layer references) | ✅ Done (2026-06-21) |

---

## Verification

- Impact analysis: for each new graph added during the project, record which existing
  files were modified. Pass condition: zero modifications to existing graph modules or
  the processing pipeline.
- EXP-04 evidence: adding a new tab required ≤ 3 file changes with zero references from
  Presentation to Signal Processing or Acquisition layers.
