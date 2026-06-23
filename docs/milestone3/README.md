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

The following views document the TimeGrapher architecture from four complementary perspectives.

| View | Description |
|------|-------------|
| [Module View](architecture/module-view.md) | Code-level structure: four layers, modules, and their dependencies (DSM verified) |
| [Runtime View](architecture/runtime-view.md) | Components and connectors at runtime: T1/T2/Main threads, AudioRingBuffer, Qt QueuedConnection |
| [Deployment View](architecture/deployment-view.md) | Hardware/software allocation: Raspberry Pi 5, USB peripherals, and build pipeline |
| [Graph Tab Decomposition View](architecture/graph-tab-view.md) | Observer pattern: BaseGraphTab abstract class, 14 concrete tabs, MainWindow registry, and wiring |

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
| [ADR 006](ADRs/ADR006-observer-pattern.md) | BaseGraphTab Observer Pattern | Extensibility, Correctness |

---

## Architecture Evaluation

ATAM (Architecture Tradeoff Analysis Method) evaluation of the TimeGrapher architecture, applied at the end of Milestone 2.

- [Architecture Evaluation (ATAM)](architecture-evaluation.md)

Key findings:
- **Resolved**: Rendering-Audio coupling on the main thread (43% deadline miss → 0% after ADR-001 + ADR-002)
- **Open**: WeiShi accuracy comparison (EXP-01) scheduled for 2026-06-29

---

## Experiment Results

Five experiments were conducted during M1–M2 to validate architectural assumptions. Results drove the two primary architecture decisions (ADR-001 and ADR-002).

- [Experiment Results](experiment-results.md)

| Experiment | QA | Key Finding |
|------------|----|-----------  |
| EXP-01 | Measurement Accuracy | Planned — WeiShi comparison scheduled 2026-06-29 |
| EXP-02 | Real-Time Performance | 0 dropped blocks at 48 / 96 / 192 kHz on RPi 5 |
| EXP-03 | Low Latency | E2E latency 80 ms → 2.2 ms avg after ADR-001 + ADR-002 (↓97%) |
| EXP-04 | Extensibility | ≤ 3 files per tab · 0 layer violations · 14 tabs verified (37 test cases passing) |
| EXP-05 | Measurement Accuracy | onset=0.08, min_peak=0.10 — stable through 60 dB noise |
