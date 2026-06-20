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
| **Accuracy** | Computed Rate / Amplitude / Beat Error must match Witschi within tolerance | [**Measurement Accuracy, Error Detection, and Handling**](references/qa.md#qas-0-measurement-accuracy-error-detection-and-handling--priority-1-governing-goal) ← governing |
| | Pipeline must process beats without missing the 21ms deadline | [Real Time Performance](references/qa.md#qas-1-real-time-performance--priority-2) |
| | Capture-to-detect latency must be low enough for correct timestamps | [Low Latency and Low Number of Missed Beats](references/qa.md#qas-2-low-latency-and-low-number-of-missed-beats--priority-3) |
| | Correct results even under noise | [Correctness](references/qa.md#qas-4-correctness--priority-5) |
| **On-Schedule Delivery** | Shorten dev machine ↔ RPi deploy cycle | Deployability |
| | Apply architecture decisions fast enough to stay on schedule | [Extensibility, Modifiability](references/qa.md#qas-3-extensibility-modifiability--priority-4-execution-enabler) |
| **Usability** | Inputs the system cannot handle must be clearly communicated | Usability |

### QA Priority Order and Governing Goal

→ Full priority rationale, QAS scenarios, and traceability matrix: [references/qa.md](references/qa.md)

### What This Presentation Covers

M2 progress is organized around three quality attribute priorities — **Accuracy** (QAS-0/1/2), **Real-time Performance** (QAS-1), **Latency** (QAS-2) — plus **Extensibility** (QAS-3) as a structural enabler. Each architecture view links directly to the QA scenario, experiment, and ADR that motivated it.
