# Milestone 1 워크샵 후 액션 플랜 / Post-Workshop Action Plan

> **작성일 / Date**: 2026-06-02  
> **출처 / Source**: 워크샵 논의 및 Project Plan (Draft)

---

## 1. 화요일 워크샵 확정 사항 / Tuesday Workshop Decisions

**한국어**

워크샵 자리에서 반드시 확정해야 할 항목이다.

| 항목 / Item | 내용 / Description |
|-------------|-------------------|
| **QA 5개 합의 / QA Agreement** | 측정 방법 + 의도 수준으로 합의 (수치는 실험 후 M2에서 확정) |
| **QA 우선순위 / QA Priority** | 5개 중 아키텍처에 가장 큰 영향을 주는 순서 결정 |
| **M1 문서 담당자 / M1 Doc Owners** | Project Plan / Drivers / Risk / Experiments / Approaches 각 담당자 지정 |
| **실험 담당자 / Experiment Owners** | Exp 1(RPi sps) / Exp 2(Qt FPS) / Exp 3(T1/T3 정확도) 담당자 지정 |

**English**

The following must be finalized during the workshop session itself.

| 항목 / Item | 내용 / Description |
|-------------|-------------------|
| **QA 5개 합의 / QA Agreement** | Agree on measurement method + intent (specific values to be confirmed after experiments at M2) |
| **QA 우선순위 / QA Priority** | Decide priority order based on architectural impact |
| **M1 문서 담당자 / M1 Doc Owners** | Assign owners for each of the 5 M1 documents |
| **실험 담당자 / Experiment Owners** | Assign owners for Exp 1 (RPi sps) / Exp 2 (Qt FPS) / Exp 3 (T1/T3 accuracy) |

---

## 2. 수요일(06/03)까지 개인 초안 작업 / Individual Draft Work by Wednesday (06/03)

### Architectural Drivers (담당자 / Owner)

**한국어**

- QA 5개를 측정 가능한 시나리오 형태로 작성
- 워크샵에서 합의한 우선순위 반영

**English**

- Write each of the 5 QAs as a measurable scenario
- Reflect the priority order agreed upon at the workshop

---

### Risk Assessment (담당자 / Owner)

**한국어**

- 코드 분석 + QA 불확실 항목에서 기술 리스크 도출
- 각 리스크 H/M/L 평가 + 대응 방안 정의

**English**

- Derive technical risks from codebase analysis + uncertain QA items
- Assess each risk at H/M/L level and define mitigation actions

---

### Planned Experiments (담당자 / Owner)

**한국어**

실험 3개 각각에 대해 아래 항목 작성:
- 목적 — 어떤 불확실성을 해소하는가
- 측정 방법
- 완료 기준

**English**

For each of the 3 experiments, write:
- Purpose — which uncertainty does this resolve
- Measurement method
- Completion criteria

---

### Architectural Approaches (담당자 / Owner)

**한국어**

- 현재 코드 구조 기반 아키텍처 개요 작성
- QA별 패턴/전술 선택 및 연결 (예: Plugin→Extensibility, Pipeline→Real-Time)

**English**

- Write architecture overview based on current codebase structure
- Select and link patterns/tactics per QA (e.g., Plugin→Extensibility, Pipeline→Real-Time)

---

### Project Plan (담당자 / Owner)

**한국어**

- 역할 분담 + 태스크 목록 작성
- 실험 일정 반영

**English**

- Write role assignments and task list
- Reflect experiment schedule

---

## 3. 이후 일정 / Remaining Schedule

**한국어**

| 날짜 / Date | 활동 / Activity |
|------------|----------------|
| 06/03 (수 / Wed) | 초안 팀 공유 / Share individual drafts with team |
| 06/04 (목 / Thu) 오후 | 중간 리뷰 — 문서 간 일관성 확인 (QA ↔ Risk ↔ Experiments ↔ Approaches) |
| 06/05 (금 / Fri) 오후 | 주간 마무리 싱크 — 피드백 반영 / Weekly wrap-up sync |
| 06/08 (월 / Mon) | M1 문서 최종화 / Finalize M1 documents |
| 06/09 (화 / Tue) | **M1 제출 / M1 Submission** |

**English**

| 날짜 / Date | 활동 / Activity |
|------------|----------------|
| 06/03 (수 / Wed) | Share individual drafts with team |
| 06/04 (목 / Thu) afternoon | Mid-week review — cross-document consistency check (QA ↔ Risk ↔ Experiments ↔ Approaches) |
| 06/05 (금 / Fri) afternoon | Weekly wrap-up sync — incorporate feedback |
| 06/08 (월 / Mon) | Finalize M1 documents |
| 06/09 (화 / Tue) | **M1 Submission** |

---

## 4. QA 수치 관련 원칙 / QA Value Principles

**한국어**

- QA 수치는 M1에 채우지 않아도 됨 → 측정 방법 + 의도만 명시
- 실험 결과로 수치 확정 → M2(06/22)에 반영
- Planned Experiments = QA 불확실성 해소 + 아키텍처 결정 검증 둘 다
- Risk의 주요 인풋 = 코드 분석 결과 + QA 불확실 항목

**English**

- QA target values do not need to be filled in for M1 → state measurement method + intent only
- Values to be confirmed from experiment results → reflected in M2 (06/22)
- Planned Experiments serve dual purpose: resolving QA uncertainty + validating architecture decisions
- Main inputs for Risk Assessment = codebase analysis findings + uncertain QA items
