# TimeGrapher — Architecture Documentation

> **LG SW Architect Training Program × CMU MSE** | Milestone 3 — 2026-07-01  
> "From Tick to Trace: Real-Time Acoustic Analysis of Mechanical Watches"

TimeGrapher captures acoustic beat-noise from a mechanical watch via USB microphone, detects T1(A) and T3(C) escapement events, computes Rate / Amplitude / Beat Error in real time, and displays the results through an extensible GUI running on Raspberry Pi 5.

---

## Requirements

This section contains the requirements distilled from the project specification, experiment findings, and mentor feedback.

- [Functional requirements](requirements/functional-rqmts.md)
- [Quality attribute requirements](requirements/quality-attribute-rqmts.md)

---

## Architecture

### Context

The context diagram below shows the scope of the TimeGrapher system and the external entities it interacts with.

- [Context Diagram](architecture/context-diagram.md)

### Architecture Views

The following views document the TimeGrapher architecture from three complementary perspectives.

| View | Description |
|------|-------------|
| [Module View](architecture/module-view.md) | Code-level structure: layers, modules, and their dependencies |
| [Runtime View](architecture/runtime-view.md) | Components and connectors at runtime: threads, queues, and data flow |
| [Deployment View](architecture/deployment-view.md) | Hardware/software allocation: Raspberry Pi 5, USB peripherals, and build pipeline |

---

## ADRs

The linked ADRs record the key architectural decisions made during the project, including their context, rationale, and trade-offs.

| ADR | Title | QA Impact |
|-----|-------|-----------|
| [ADR 001](ADRs/ADR001-dsp-offload-thread.md) | DSP Offload Thread | Real-Time Performance, Low Latency |
| [ADR 002](ADRs/ADR002-lazy-rendering.md) | Lazy Rendering (Active Tab Only) | Real-Time Performance, Usability |
| [ADR 003](ADRs/ADR003-layered-architecture.md) | Four-Layer Architecture | Extensibility, Correctness |
| [ADR 004](ADRs/ADR004-qt-framework.md) | Qt as Application Framework | Real-Time Performance, Extensibility |
| [ADR 005](ADRs/ADR005-ring-buffer-connector.md) | Ring Buffer as Thread Boundary Connector | Real-Time Performance, Low Latency |

---

## Experiment Results

Key experiments that drove architectural decisions:

| Experiment | Finding | Decision Triggered |
|------------|---------|-------------------|
| EXP-02 macOS Baseline | `wait_ms` 420 ms → DSP starvation in main thread | ADR 001 (DSP Offload Thread) |
| EXP-02 R1 Lazy Rendering | `replot_count` 8.22 → 2.08 (↓75%) on macOS | ADR 002 (Lazy Rendering) |
| EXP-02 RPi Baseline | `exec` avg 20 ms / deadline miss 43% / thermal throttle 85°C | Architecture refinement required for RPi |

Detailed results: [docs/milestone2/experiment-results.md](../milestone2/experiment-results.md)
