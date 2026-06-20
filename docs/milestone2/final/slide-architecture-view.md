# Architecture Views

← [Wrap-up & Intro](slide-wrapup-intro.md) | [Presentation Index](README.md)

---

All views follow the **Merson 7-section template** and are ordered by QA priority.  
QAS-1 (Measurement Accuracy) is the governing goal supported by all views.

| # | View | Type | Primary QA |
|---|------|------|------------|
| 1 | [C&C: DSP Pipeline Thread Model](references/views/view-cc-dsp-pipeline.md) | C&C / Runtime | QAS-2 Real Time Performance, QAS-3 Low Latency |
| 2 | [Layered: 4-Layer Allowed-to-Use](references/views/view-layered-4layer.md) | Module — Layered | QAS-4 Extensibility, Modifiability |
| 3 | [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) | Module — Decomposition | QAS-4 Extensibility, Modifiability |
| 4 | [Module: IAudioSource Dependency Inversion](references/views/view-iaudiosource.md) | Module — Decomposition | QAS-4 Extensibility, Modifiability |
| 5 | [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) | Deployment | Deployability |
