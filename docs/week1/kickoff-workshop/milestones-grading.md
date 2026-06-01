# Milestones & Grading

> **작성일 / Date**: 2026-06-02  
> **출처 / Source**: Time Grapher Project Plan (Draft).pdf — p.30-33 / LG SW Architect Final Demo Grading Score Sheet p.3

---

## 채점 스케일 / Grading Scale

> 항목별 배점은 다르지만, 등급 기준은 모든 항목에 동일하게 적용

**한국어**

| 등급 | 기준 |
|------|------|
| **Outstanding** | 모든 요소 완수 + 독창적 분석 + 설계 근거를 논리적으로 제시 |
| **Satisfactory** | 모든 요소 완수 + 종합적 분석 + 일관된 논리 |
| **Marginal** | 기존 코드 변경 없이 기능만 동작 |
| **Unsatisfactory** | 성능 또는 품질 요구사항 미달 |
| **Not Acceptable** | 기능 동작 안 함 |

> **Satisfactory → Outstanding 차이**: 구현 여부가 아니라 **설계 근거(design rationale)를 얼마나 논리적으로 설명하느냐**

**English**

> Point values differ per item, but the grade criteria apply equally to all items.

| Grade | Criteria |
|-------|----------|
| **Outstanding** | All elements completed + original analysis + design rationale presented with clear logic |
| **Satisfactory** | All elements completed + comprehensive analysis + consistent logic |
| **Marginal** | Functionality works without changing existing code |
| **Unsatisfactory** | Performance or quality requirements not met |
| **Not Acceptable** | Functionality does not work |

> **Satisfactory → Outstanding gap**: Not whether something was implemented, but **how logically the design rationale is explained**

---

## Milestone 1 — `2026-06-09 (Tue)`

### Deliverables

**한국어**

| 문서 | 핵심 내용 |
|------|---------|
| Project Plan | 역할 분담 + 태스크 + 마일스톤 / 아키텍처 기반 구현 태스크 / 실험 계획 반영 |
| Architectural Drivers | QA 5개를 측정 가능한 형태로 정의 + 우선순위 |
| Risk Assessment | 기술·비기술 리스크 H/M/L 평가 + 해결 액션 정의 |
| Planned Experiments | 실험 목적 + 해결 질문 + 완료 기준 |
| Architectural Approaches | 아키텍처 개요 + 패턴/전술 + QA driver와의 연결 |

**English**

| Document | Key Content |
|----------|------------|
| Project Plan | Role assignment + tasks + milestones / architecture-driven implementation tasks / experiment plan included |
| Architectural Drivers | 5 QAs defined in measurable form + prioritization |
| Risk Assessment | Technical and non-technical risks rated H/M/L + mitigation actions defined |
| Planned Experiments | Experiment purpose + questions to resolve + completion criteria |
| Architectural Approaches | Architecture overview + patterns/tactics + linkage to QA drivers |

### Outstanding을 받으려면 / To Achieve Outstanding

**한국어**

- QA가 단순 서술이 아닌 **측정·검증 가능한 수치**로 표현되어야 함
- 각 Architectural Approach가 **어떤 QA를 어떻게 지원하는지** 명확히 연결
- 실험 계획의 **완료 기준**이 구체적으로 정의되어 있어야 함

**English**

- QAs must be expressed as **measurable, verifiable numbers**, not plain descriptions
- Each Architectural Approach must **clearly map to which QA it supports and how**
- Experiment **completion criteria** must be concretely defined

### 멘토 체크 질문 / Mentor Check Questions

**한국어**

- QA가 "actionable"한가? (수치 없으면 바로 지적)
- Architectural Approach와 QA driver가 연결되어 있는가?
- 실험이 완료되었음을 어떻게 판단할 수 있는가?
- 리스크에 대한 구체적 대응 액션이 있는가?

**English**

- Is each QA "actionable"? (will be flagged immediately if no numbers)
- Are Architectural Approaches linked to QA drivers?
- How can you tell when an experiment is complete?
- Are there concrete mitigation actions for each risk?

---

## Milestone 2 — `2026-06-22 (Mon)`

### Deliverables

**한국어**

| 문서 | 핵심 내용 |
|------|---------|
| Updated Project Plan | 리스크 기반 계획 업데이트 + 현실적 구현 계획 |
| Experiment Results | 실험 1·2·3 결과 + 미해결 항목 목록 |
| Architecture — Module View | 코드 레벨 구조 + 의존성 (최소 1개 필수) |
| Architecture — C&C View | 컴포넌트-커넥터 런타임 관점 (최소 1개 필수) |
| Architecture — Deployment View | RPi 기반 하드웨어 배치 + 통신 채널 |
| Construction Plan | 상세 구현 태스크 + 잔여 일정 |

**English**

| Document | Key Content |
|----------|------------|
| Updated Project Plan | Risk-based plan update + realistic implementation schedule |
| Experiment Results | Results of Experiments 1·2·3 + list of unresolved items |
| Architecture — Module View | Code-level structure + dependencies (at least 1 required) |
| Architecture — C&C View | Component-connector runtime perspective (at least 1 required) |
| Architecture — Deployment View | RPi-based hardware placement + communication channels |
| Construction Plan | Detailed implementation tasks + remaining schedule |

### Outstanding을 받으려면 / To Achieve Outstanding

**한국어**

- 실험 결과가 **아키텍처 결정을 어떻게 바꿨는지** 명확히 서술
- 3개 Architecture View가 서로 일관성 있게 연결
- 선택한 아키텍처 접근법의 **trade-off를 인식하고 설명**할 수 있어야 함

**English**

- Clearly articulate **how experiment results changed architectural decisions**
- All three Architecture Views must be consistent and cross-referenced
- Must be able to **recognize and explain the trade-offs** of the chosen architectural approaches

### 멘토 체크 질문 / Mentor Check Questions

**한국어**

- 실험 결과가 open question을 실제로 해결했는가?
- Architecture View 3개가 모두 있는가? (Module / C&C / Deployment)
- 실험이 아키텍처 개선으로 이어졌는가?
- 아직 해결되지 않은 critical concern이 있는가?

**English**

- Did the experiment results actually resolve the open questions?
- Are all three Architecture Views present? (Module / C&C / Deployment)
- Did the experiments lead to architectural improvements?
- Are there any critical concerns still unresolved?

---

## Milestone 3 — `2026-07-01 (Wed)`

### Team Presentation (20분 / 20 minutes)

**한국어**

| 섹션 | 내용 |
|------|------|
| QA Requirements | 우선순위 높은 QA + 아키텍처에 미친 영향 |
| Architecture | Views + 핵심 접근법 + 설계 근거 |
| Experiments & Evaluation | 실험 결과 + 아키텍처 평가 활동 |
| Lessons Learned | 잘된 것 / 잘못된 것 / 다시 한다면 |

> 20분 = 모든 항목을 깊게 다룰 수 없음 → 섹션별 1~2개 핵심만 선택해서 깊게

**English**

| Section | Content |
|---------|---------|
| QA Requirements | High-priority QAs + their influence on the architecture |
| Architecture | Views + key approaches + design rationale |
| Experiments & Evaluation | Experiment results + architecture evaluation activities |
| Lessons Learned | What went well / what didn't / what we would do differently |

> 20 minutes is not enough to cover everything in depth → select 1–2 key points per section and go deep

### Final Demo (RPi에서 실행 / runs on RPi)

**한국어**

| QA | 증명 방법 |
|----|---------|
| Low Latency | 3구간 지연 수치(ms) 제시 — average + worst-case |
| Real-Time Performance | RPi에서 실시간 동작 확인 |
| Correctness | 동일 시계·조건에서 측정값 안정성 확인 |
| Measurement Accuracy | WeiShi No.1000과 수치 비교 |
| Extensibility | 새 그래프 추가 시 기존 코드 변경 범위 설명 |

**English**

| QA | How to demonstrate |
|----|-------------------|
| Low Latency | Present 3-segment latency numbers (ms) — average + worst-case |
| Real-Time Performance | Confirm real-time operation on RPi |
| Correctness | Verify measurement stability under identical watch and conditions |
| Measurement Accuracy | Compare values against WeiShi No.1000 |
| Extensibility | Explain the scope of existing code changes when adding a new graph |

### Outstanding을 받으려면 / To Achieve Outstanding

**한국어**

- 기능 동작뿐 아니라 **아키텍처·구현 선택이 QA를 어떻게 지원하는지** 설명
- 새 기능이 별도 프로토타입이 아닌 **기존 앱에 통합**되어 있어야 함
- 각 추가 기능이 사용자에게 **무엇을 보여주는지** 명확히 설명

**English**

- Explain not just that things work, but **how the architectural and implementation choices support each QA**
- New features must be **integrated into the existing app**, not separate prototypes
- Clearly explain **what each additional feature shows the user**

### 채점 루브릭 주의 / Grading Rubric Note

**한국어**

> TimeGrapher 전용 루브릭은 **Week 2 or 3 배포 예정** (p.33)  
> 현재 assets의 Grading Score Sheet은 ADS-B 프로젝트용 — 적용 불가  
> 루브릭 받는 즉시 팀 전체 공유 필요

**English**

> The TimeGrapher-specific rubric is **scheduled for release in Week 2 or 3** (p.33)  
> The current Grading Score Sheet in assets is for the ADS-B project — do not apply it  
> Share with the full team as soon as the rubric is received

---

## 마일스톤별 Outstanding 핵심 요약 / Outstanding Summary per Milestone

```
M1: "QA 수치 + Approach-Driver 연결"
    "QA numbers + Approach-Driver linkage"

M2: "실험 → 아키텍처 개선 흐름 + View 3개 일관성"
    "Experiments → architectural improvements flow + 3-View consistency"

M3: "기능 동작 + QA 증거 수치 + 설계 근거 설명"
    "Functional demo + QA evidence numbers + design rationale explanation"
```
