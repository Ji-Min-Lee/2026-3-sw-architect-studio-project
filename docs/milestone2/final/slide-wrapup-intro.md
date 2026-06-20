# Section 1 — Wrap-up M1 & Intro

← [Presentation Index](README.md) | Next: [Architecture Views →](slide-architecture-view.md)

> **발표 시간**: ~3분 | 목표: M1 피드백 반영 확인 → 이번 발표 목표 설정 → ARS 관점으로 전환

---

## 1-A. M1 Feedback — What We Fixed

> 📢 **PRESENT** (매우 짧게 — 항목당 한 문장)

| Area | M1 Issue | M2 Fix |
|------|----------|--------|
| Project Plan | No owner/date. Experiments not tracked | Owner + date on every task. Experiments as GitHub issues |
| Architecture Diagrams | Source unlabeled. Too detailed | Purpose-driven views: each view targets a specific reader and QA |
| Architectural Drivers | Tactics mixed into QA doc. Provisional numbers | Structured under `references/` — separate files per QA / risk / experiment / ADR |
| Experiments | 1 of 9 risks linked. None started | Short sprints with decide → execute → feedback loops — all scheduled experiments completed |
| Navigation | No README. No cross-links | [README.md](README.md) as entry point; all docs cross-linked |

---

## 1-B. Updated Presentation Goals

> 📢 **PRESENT**

| Goal | Measure |
|------|---------|
| **Accuracy** | Every architectural claim is backed by experiment data or ADR evidence |
| **On-time delivery** | M2 deliverables complete by 06/22 · Final demo on 07/01 |

---

## 1-C. What This Presentation Covers

> 📢 **PRESENT** — 전환 슬라이드

M2 진행사항을 세 개의 QA 우선순위 + 구조적 enabler 중심으로 설명합니다.

| 우선순위 | QA | 대응 View |
|---------|-----|----------|
| **1st** | [QAS-1 Measurement Accuracy](references/qa/qas-1-measurement-accuracy-error-detection-handling.md) ← governing | 모든 view의 목표 |
| **2nd** | [QAS-2 Real-Time Performance](references/qa/qas-2-real-time-performance.md) | C&C: DSP Pipeline |
| **3rd** | [QAS-3 Low Latency](references/qa/qas-3-low-latency-and-low-number-of-missed-beats.md) | C&C: DSP Pipeline |
| **4th** | [QAS-4 Extensibility / Modifiability](references/qa/qas-4-extensibility-modifiability.md) | Layered, Decomposition, IAudioSource |
| Enabler | Deployability | Deployment: Build-Deploy Pipeline |

→ 각 architecture view는 해당 QA 시나리오 · 실험 · ADR과 직접 연결됩니다.

---

*Reference only — not presented:*

- Full QA priority rationale and traceability matrix: [references/qa/](references/qa/README.md)
