# Week 2 계획 개요 및 아키텍처 평가 / Week 2 Plan Overview & Architecture Evaluation

> **작성일 / Date**: 2026-06-10  
> **기간 / Period**: 2026-06-10 (수) ~ 2026-06-12 (금) 실험 스프린트 (근무일 3일) + 2026-06-15 (월)~ ADD 스프린트

---

## 1. 계획 평가 / Plan Evaluation

**한국어**

6월 10일~14일 실험 완료 → 6월 15일부터 QA 우선순위 기반 ADD 스프린트 가동 계획은 ADD 원칙 및 UP Elaboration Phase와 정확히 일치한다.

ADD는 "Quality Attribute 기반으로 아키텍처 결정을 내린다"는 원칙을 갖는다. 현재 QAS-1(Real-Time), QAS-2(Latency)의 Response Measure는 모두 ⚠️ Provisional 상태다. 실험 없이 tactics(Lock-Free Ring Buffer, Priority Scheduling, Lazy Rendering 등)의 적용 여부를 결정하는 것은 근거 없는 설계 결정이다. 따라서 **실험을 먼저 완료하고 아키텍처 결정을 확정하는 이 계획은 타당하다**.

UP 관점에서도 이는 Elaboration Phase의 목표인 "아키텍처적으로 중요한 Use Case를 구현하고, 기술적 리스크를 해소한다"와 정확히 맞는다.

**English**

The plan — complete all experiments by June 14, then start QA-priority ADD sprints from June 15 — aligns precisely with ADD principles and the UP Elaboration Phase.

ADD requires that architecture decisions be driven by Quality Attributes with verified data. Currently, all Response Measures for QAS-1 (Real-Time) and QAS-2 (Latency) are marked ⚠️ Provisional. Deciding whether to apply tactics (Lock-Free Ring Buffer, Priority Scheduling, Lazy Rendering, etc.) without experiment data is an unfounded architectural decision. **This plan — resolving experiments first, then finalizing architectural decisions — is architecturally sound.**

From a UP perspective, this matches the Elaboration Phase goal: "implement architecturally significant use cases and resolve technical risks."

---

## 2. 리뷰어 코멘트 대응 / Reviewer Comment Response

**한국어**

M1 발표에서 받은 코멘트와 대응 방향:

| 코멘트 | 근본 원인 | 대응 |
|--------|----------|------|
| 실험 세부 계획이 없다 | EXP 문서에 "언제/누가/어떤 순서로" 가 명시되지 않음 | `01-experiment-execution-plan.md`에 일별 실행 계획 작성 |
| Realtime: queue 메모리 사이즈에만 집중했나? 뭐랑 뭐 사이의 통신이냐? | Ring Buffer가 어느 스레드 경계에 있는지, 어느 두 컴포넌트 사이인지 명시 부족 | EXP-01/02에 통신 아키텍처 컨텍스트 추가 |
| risk들 사이의 관계는? | Risk Assessment가 risk를 독립 항목으로만 나열, 전파 관계 미표현 | `02-risk-relationships.md`에 risk dependency map 작성 |
| 리스크 우선순위가 QA 중요도 순이 아님 | Overall = max(Prob, Impact) 기준인데 QA 우선순위와 혼용 | Risk를 Prob×Impact 기준으로 재정렬 (02 문서) |
| 실험 빨리 해라 | EXP-01 미착수 → 모든 downstream 결정이 Provisional | Week 2 첫날 EXP-01 즉시 착수 |

**English**

Reviewer comments from M1 presentation and response plan:

| Comment | Root Cause | Response |
|---------|-----------|---------|
| No detailed experiment plan | EXP docs lack "when / who / in what order" | Write daily execution plan in `01-experiment-execution-plan.md` |
| Realtime: only focused on queue memory size? What communicates with what? | Ring Buffer's thread boundary and owning components not specified | Add communication architecture context to EXP-01/02 |
| Relationships between risks? | Risk Assessment lists risks as independent items, no propagation shown | Write risk dependency map in `02-risk-relationships.md` |
| Risk priority ≠ QA importance order | Overall = max(Prob, Impact) criterion was mixed with QA priority | Re-rank risks by Prob×Impact in doc 02 |
| Do experiments quickly | EXP-01 not started → all downstream decisions remain Provisional | Start EXP-01 immediately on Week 2 Day 1 |

---

## 3. Week 2 구조 / Week 2 Structure

**한국어**

Week 2는 두 단계로 구성된다:

### 3.1 실험 스프린트 (6/10–6/14)
모든 실험(EXP-01~04)과 FR 기본 구현(Observer 리팩토링)을 완료한다. 자세한 내용은 [`01-experiment-execution-plan.md`](01-experiment-execution-plan.md) 참조.

**목표**: EXP-01~04 결과로 모든 Provisional 값을 확정하고, ADD 스프린트에서 필요한 아키텍처 결정의 근거를 확보한다.

### 3.2 ADD 스프린트 가동 (6/15~)
QA 우선순위에 따라 ADD Steps 2–6을 반복 수행한다. 스프린트 순서:

| Sprint | 기간 | QA Focus | 확정할 결정 |
|:------:|:----:|---------|-----------|
| **S1** | 6/15–16 | QAS-1 Real-Time Performance | Lock-Free Ring Buffer 적용 여부, 최종 SPS 결정 |
| **S2** | 6/17–18 | QAS-2 Low Latency | Lazy Rendering 적용 여부, Thread 분리 확정 |
| **S3** | 6/22–23 | QAS-3 Correctness + QAS-5 Extensibility | Detector 파라미터 확정, 11-tab 병렬 개발 착수 |
| **S4** | 6/24–25 | QAS-4 Usability + 그래프 완성 | Signal Quality Warning 파라미터 확정 |

**English**

Week 2 consists of two phases:

### 3.1 Experiment Sprint (6/10–6/14)
Complete all experiments (EXP-01~04) and FR baseline implementation (Observer refactoring). See [`01-experiment-execution-plan.md`](01-experiment-execution-plan.md) for details.

**Goal**: Finalize all Provisional values using EXP-01~04 results, securing the evidence base for architectural decisions in subsequent ADD sprints.

### 3.2 ADD Sprint Launch (6/15~)
Iterate ADD Steps 2–6 by QA priority. Sprint sequence:

| Sprint | Period | QA Focus | Decision to Confirm |
|:------:|:------:|---------|-------------------|
| **S1** | 6/15–16 | QAS-1 Real-Time Performance | Lock-Free Ring Buffer apply/skip, final SPS |
| **S2** | 6/17–18 | QAS-2 Low Latency | Lazy Rendering apply/skip, thread separation finalized |
| **S3** | 6/22–23 | QAS-3 Correctness + QAS-5 Extensibility | Detector params confirmed, 11-tab parallel dev starts |
| **S4** | 6/24–25 | QAS-4 Usability + graph completion | Signal Quality Warning params confirmed |

---

## 4. 산출물 / Deliverables

**한국어**

| 파일 | 내용 |
|------|------|
| [`01-experiment-execution-plan.md`](01-experiment-execution-plan.md) | EXP-01~04 일별 실행 계획 (담당자, 순서, 의존성) |
| [`02-risk-relationships.md`](02-risk-relationships.md) | Risk 간 전파 관계 + Prob×Impact 기반 우선순위 재정렬 |

**English**

| File | Content |
|------|---------|
| [`01-experiment-execution-plan.md`](01-experiment-execution-plan.md) | EXP-01~04 daily execution plan (owner, sequence, dependencies) |
| [`02-risk-relationships.md`](02-risk-relationships.md) | Risk propagation map + re-ranked by Prob×Impact |
