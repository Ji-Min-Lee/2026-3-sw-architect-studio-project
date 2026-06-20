# QAS-4: Extensibility, Modifiability — Priority 4 (Execution Enabler)

> M1 name: "Extensibility (QAS-5)". M2: uses exact Project Plan terminology "Extensibility, Modifiability".
> M1 status: Provisional (post-refactoring). M2 status: ✅ Verified by 06/22.

| Field | Detail |
|-------|--------|
| **Source** | Developer (adding a new graph display tab) |
| **Stimulus** | Request to implement a new graph type (e.g., Spectrogram, Long-Term Trend, Sequence Display) |
| **Artifact** | Source code — Presentation layer and its relationship to Domain and below |
| **Environment** | Development environment (Qt Creator, macOS/Windows), within the 5-week project timeline |
| **Response** | New graph widget implemented, registered, and tested independently; no existing preprocessing modules modified |
| **Measure** | Files changed = ≤ 3; direct references from the new tab to Signal Processing or Acquisition = 0 |

## Related

- [QA Priority Summary](README.md)
- [ADR-005: IAudioSource Dependency Inversion](../adr/ADR-005-p1-iaudiosource-dependency-inversion.md)
- [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md)
- [Layered View: 4-Layer Allowed-to-Use](../views/view-layered-4layer.md)
- [Decomposition View: Graph Tab](../views/view-decomposition-graph-tab.md)
- [Module View: IAudioSource Dependency Inversion](../views/view-iaudiosource.md)
