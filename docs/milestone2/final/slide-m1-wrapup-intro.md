# Section 1 — Milestone 1 Wrap-up & Intro

← [Presentation Index](README.md) | Next: [Architecture Views →](slide-architecture-view.md)

> **Time**: ~3 min | Goal: Confirm M1 feedback addressed → set presentation goals → transition to ARS framing

---

## 1-A. M1 Feedback — What We Fixed

> 📢 **PRESENT** (very brief — one sentence per item)

| Area | M1 Issue | M2 Fix |
|------|----------|--------|
| Project Plan | No owner/date. Experiments not tracked | Owner + date on every task. Experiments tracked as GitHub issues |
| Architecture Diagrams | Source unlabeled. Too detailed | Purpose-driven views: each view targets a specific reader and QA |
| Architectural Drivers | Tactics mixed into QA doc. Provisional numbers | Structured under `references/` — separate files per QA / risk / experiment / ADR |
| Experiments | 1 of 9 risks linked. None started | Short sprints with decide → execute → feedback loops — all scheduled experiments completed |
| Navigation | No README. No cross-links | [README.md](README.md) added as entry point; all docs cross-linked |

---

## 1-B. Updated Presentation Goals

> 📢 **PRESENT**

| Goal | Measure |
|------|---------|
| **Accuracy** | Every architectural claim is backed by experiment data or ADR evidence |
| **On-time delivery** | M2 deliverables complete by 06/22 · Final demo on 07/01 |

---

## 1-C. What This Presentation Covers

> 📢 **PRESENT** — transition slide

M2 progress is organized around three QA priorities plus a structural enabler.

| Priority | QA | Corresponding View |
|---------|-----|--------------------|
| **1st** | [QAS-1 Measurement Accuracy](references/qa/qas-1-measurement-accuracy-error-detection-handling.md) ← governing | Goal of all views |
| **2nd** | [QAS-2 Real-Time Performance](references/qa/qas-2-real-time-performance.md) | C&C: DSP Pipeline |
| **3rd** | [QAS-3 Low Latency](references/qa/qas-3-low-latency-and-low-number-of-missed-beats.md) | C&C: DSP Pipeline |
| **4th** | [QAS-4 Extensibility / Modifiability](references/qa/qas-4-extensibility-modifiability.md) | Layered, IAudioSource |
| Enabler | Deployability | Deployment: Build-Deploy Pipeline |

→ Each architecture view links directly to its QA scenario, experiment, and ADR.

---

*Reference only — not presented:*

- Full QA priority rationale and traceability matrix: [references/qa/](references/qa/README.md)
