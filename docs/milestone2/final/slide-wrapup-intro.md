# Wrap-up M1 & Intro

← [Presentation Index](README.md) | Next: [Architecture Views →](slide-architecture-view.md)

---

## 1. M1 Feedback & Improvements

### What We Fixed

| Area | M1 Issue | M2 Fix |
|------|----------|--------|
| Project Plan | No owner/date. Experiments not tracked. No Kanban link | Owner + date on every task. Experiments as GitHub issues → [Board](references/github-project-status.md) |
| Architecture Diagrams | Source unlabeled. Too detailed | Purpose-driven views: each view targets a specific reader and quality attribute |
| Architectural Drivers | Tactics mixed into QA doc. Provisional numbers. Solution language | Structured by type and item under `references/` — separate files per QA, risk, experiment, ADR |
| Experiments | 1 of 9 risks linked. None started | Short sprints with fast decide → execute → feedback loops — all experiments completed on schedule |
| Navigation | No README. No cross-links | [README.md](README.md) added as entry point with document structure |

### Updated Presentation Goals

Two concrete targets for this presentation:

| Goal | Measure |
|------|---------|
| **Accuracy** | Every architectural claim is backed by experiment data or ADR evidence |
| **On-time delivery** | Architecture decisions support staying on schedule — M2 deliverables complete by 06/22; Final demo on 07/01 |

---

## 2. Our Goals

> The architecture exists to serve these goals — every structural decision can be traced back to one of them.

### Goal Map

| Category | Goal | Quality Attribute |
|----------|------|-------------------|
| **Accuracy** | Computed Rate / Amplitude / Beat Error must match Witschi within tolerance | [**Measurement Accuracy, Error Detection, and Handling**](references/qa/qas-1-measurement-accuracy-error-detection-handling.md) ← governing |
| | Pipeline must process beats without missing the 21ms deadline | [Real Time Performance](references/qa/qas-2-real-time-performance.md) |
| | Capture-to-detect latency must be low enough for correct timestamps | [Low Latency and Low Number of Missed Beats](references/qa/qas-3-low-latency-and-low-number-of-missed-beats.md) |
| | Correct results even under noise | [Correctness](references/qa/qas-5-correctness.md) |
| **On-Schedule Delivery** | Shorten dev machine ↔ RPi deploy cycle | Deployability |
| | Apply architecture decisions fast enough to stay on schedule | [Extensibility, Modifiability](references/qa/qas-4-extensibility-modifiability.md) |
| **Usability** | Inputs the system cannot handle must be clearly communicated | Usability |

### QA Priority Order and Governing Goal

→ Full priority rationale, QAS scenarios, and traceability matrix: [references/qa/](references/qa/README.md)

### What This Presentation Covers

M2 progress is organized around three quality attribute priorities — **Accuracy** ([QAS-1](references/qa/qas-1-measurement-accuracy-error-detection-handling.md)), **Real-time Performance** ([QAS-2](references/qa/qas-2-real-time-performance.md)), **Latency** ([QAS-3](references/qa/qas-3-low-latency-and-low-number-of-missed-beats.md)) — plus **Extensibility** ([QAS-4](references/qa/qas-4-extensibility-modifiability.md)) as a structural enabler. Each architecture view links directly to the QA scenario, experiment, and ADR that motivated it.
