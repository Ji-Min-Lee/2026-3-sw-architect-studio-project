# QAS-3: Extensibility, Modifiability — Priority 3 (Execution Enabler)

> M1 name: "Extensibility (QAS-5)". M2: uses exact Project Plan terminology "Extensibility, Modifiability".
> M1 status: Provisional (post-refactoring). M2 status: ✅ Verified by 06/22.

| Field | Detail |
|-------|--------|
| **Source** | Developer (adding a new graph display tab) |
| **Stimulus** | Request to implement a new graph type (e.g., Spectrogram, Long-Term Trend, Sequence Display) |
| **Artifact** | Source code — Presentation layer and its relationship to layers below |
| **Environment** | Development environment (Qt Creator, macOS/Windows), within the 5-week project timeline |
| **Response** | New graph widget implemented, registered, and tested independently; no existing modules in lower layers modified |
| **Measure** | Files changed outside the new tab itself = **≤ 3**; references from the new tab to Signal Processing or Acquisition = **0** |

## Tactics (Bass/CMK Ch.8 §8.2 — Modifiability)

The following tactics from Bass, Clements & Kazman *Software Architecture in Practice* (4th ed.)
were applied to achieve this QAS:

| Tactic (Ch.8) | How it appears in this system |
|---|---|
| **Restrict Dependencies** (Reduce Coupling) | 4-Layer allowed-to-use rule enforced by DSM — Presentation may only reference Domain; zero references to Signal Processing or Acquisition verified by `#include` trace. Ch.8 p.124: *"seen in layered architectures, a layer is allowed to use only lower layers."* |
| **Abstract Common Services** (Reduce Coupling) | `IAudioSource` interface unifies all audio sources under a single abstract contract. Adding `NetworkWorker` requires 1–2 files with zero other changes — Ch.8 p.123: *"Abstract common services ... to make the system portable across different ... variations."* |
| **Publish-Subscribe Pattern** (Ch.8 §8.4) | `MeasurementEngine` publishes `measurementReady` signal; all 14 `BaseGraphTab` subscribers register without any change to the publisher. Ch.8 p.129: *"Adding or changing subscribers requires only registering for an event and causes no changes to the publisher."* |
| **Encapsulate** (Reduce Coupling) | `SessionController` owns a single `IAudioSource*` and one `connect()` site; `MainWindow::registerTab<T>()` is the single tab registration point. Neither exposes internal implementation details to callers. |

### Testability Connection (Bass/CMK Ch.12 §12.2)

These same modifiability tactics produce testability as a direct consequence.
Ch.12 p.191: *"Ensuring that the system has high cohesion, loose coupling, and separation
of concerns — all modifiability tactics (see Chapter 8) — can also help with testability."*

In this system, the causal chain is:

```
[Ch.8 tactics applied]              [Ch.12 consequence]
Restrict Dependencies      →   each layer testable in isolation
  (4-Layer, DSM 0 violations)        ↓
Abstract Common Services   →   IAudioSource injectable → SimWorker in tests
  (IAudioSource)                     ↓
WatchMath module isolation →   44 unit tests, no hardware required (ADR-008)
  (ADR-008, Limit Structural         ↓
   Complexity tactic Ch.12)    142 unit tests across 10 independent binaries,
                                all passing — evidence that coupling is low
                                enough for each module to be tested alone
```

The 142 unit tests across 10 binaries are therefore **evidence of architectural quality**,
not merely test coverage: they are only possible because the architecture achieves the low
coupling and separation of concerns that the modifiability tactics prescribe.

**Publish-Subscribe testability tradeoff**: Ch.8 p.130 notes that *"use of the
publish-subscribe pattern can negatively impact testability."* We mitigate this via:
- `Qt::QueuedConnection` (FIFO) limits nondeterminism — Ch.12 *Limit Nondeterminism* tactic
- DSM verification confirms zero cross-layer signal wiring (no hidden pub-sub coupling)
- `EXP-04 TestAddedTabs 20/20` verifies observer contract compliance end-to-end

## Related

[QA Priority Summary](README.md)

| Architecture | Rationale | Experiment | View |
|---|---|---|---|
| 4-Layer Allowed-to-Use | [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) | [EXP-04: Observer Pattern Compliance](../experiments/exp-04-extensibility-observer-pattern.md) | [Layered View: 4-Layer Allowed-to-Use](../views/view-layered-4layer.md) |
| Observer Pattern (BaseGraphTab) | [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) | [EXP-04: Observer Pattern Compliance](../experiments/exp-04-extensibility-observer-pattern.md) | [Decomposition View: Graph Tab](../views/view-decomposition-graph-tab.md) |
