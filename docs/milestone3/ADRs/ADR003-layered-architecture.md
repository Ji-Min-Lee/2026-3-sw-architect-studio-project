# ADR 003: Four-Layer Architecture (Acquisition / Processing / Domain / Presentation)

The TimeGrapher system must support 11 graph tabs implemented by different team members over a compressed 5-week schedule. Without a clear module boundary, adding new visualizations risks inadvertently modifying the signal processing pipeline and breaking measurement accuracy. A structural rule is needed that makes the "safe zone" for new code obvious and enforced.

## Decision

Organize all source code into four strictly ordered layers. Dependencies flow **downward only** — no layer may import from a layer above it.

| Layer | Responsibility | Example Modules |
|-------|---------------|-----------------|
| **Presentation** | GUI widgets, tab rendering, user controls | `MainWindow`, `GraphTabManager`, all graph tabs |
| **Domain** | Measurement computation and history | `MeasurementEngine`, `BeatDetector`, `MeasurementStore` |
| **Signal Processing** | Digital signal processing on raw PCM | `FilterChain`, `SignalBuffer` |
| **Acquisition** | Audio capture abstraction | `AudioCapture`, `LiveCapture`, `PlaybackCapture`, `SimCapture` |

**Extensibility contract**: Adding a new graph tab requires changes **exclusively** in the Presentation layer. Zero modifications to Domain, Signal Processing, or Acquisition layers are permitted.

## Rationale

The project has a single data pipeline (capture → filter → detect → compute) and a variable number of visualizations. A layered structure cleanly separates the stable pipeline (lower layers) from the volatile display code (Presentation). This directly enables QAS-5 (Extensibility) and protects QAS-2 (Measurement Accuracy) by preventing visualization logic from inadvertently modifying detection parameters.

The existing `TimeGrapher_v10.5` sample codebase already exhibits an informal layering. This decision formalizes and enforces it.

## Rejected Alternatives

- **Monolithic structure**: All code in a flat namespace. Fast to start, but any graph tab developer can accidentally call `Detector::setThreshold()` — violating accuracy. Rejected due to 11-tab implementation risk.
- **Pipe-and-filter style**: Each processing stage is a self-contained filter with explicit data connectors. More formally correct but introduces significant boilerplate for a system where the pipeline topology is fixed. Overkill for a single-machine Qt application.
- **Plugin architecture**: Graph tabs loaded as shared libraries at runtime. Would maximize extensibility but requires a stable binary ABI, a plugin manager, and significant infrastructure. Out of scope for a 5-week project.

## Status

Accepted. Applied since Milestone 1 design; enforced via code review.

## Consequences

- **Positive**: Any team member can add a graph tab safely without understanding the DSP pipeline.
- **Positive**: The lower three layers can be unit-tested independently of Qt GUI machinery.
- **Positive**: The pipeline code is insulated from GUI refactors (e.g., switching from QWidget to QML in the future).
- **Trade-off**: Strict layer boundaries require passing data structures (e.g., `Measurement`) across all layers. Changes to the `Measurement` struct propagate to all tabs. Mitigated by keeping `Measurement` stable after M1.
- **Trade-off**: Some tabs need to access both live `Measurement` objects and historical data from `MeasurementStore` — the Domain layer must expose two interfaces. Accepted complexity.

## Related ADRs
- [ADR 002 — Lazy Rendering](ADR002-lazy-rendering.md)
- [ADR 004 — Qt as Application Framework](ADR004-qt-framework.md)
