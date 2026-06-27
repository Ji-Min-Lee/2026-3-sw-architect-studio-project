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
