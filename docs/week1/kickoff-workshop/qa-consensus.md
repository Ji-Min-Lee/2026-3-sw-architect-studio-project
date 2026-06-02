# QA 팀 합의 준비 / QA Team Consensus Preparation

> **작성일 / Date**: 2026-06-02  
> **목적 / Purpose**: 워크샵에서 팀 합의를 이끌기 위한 사전 분석 및 논의 안건 / Pre-analysis and agenda items to drive team consensus at the workshop

---

## 1. QA 우선순위 / QA Priority

### 판단 기준 / Decision Criteria

**한국어**

> 아키텍처에 가장 큰 제약을 주는 QA = 1순위  
> 이 결정이 나머지 설계 방향의 전제가 됨

**English**

> The QA that most constrains the architecture = top priority  
> This decision becomes the premise for all other design directions

### 확정 우선순위 / Confirmed Priority

> **업데이트 / Updated**: 2026-06-02 — 팀 논의 후 확정 / Confirmed after team discussion

**한국어**

| 순위 | QA | 근거 |
|------|----|------|
| **1** | Real-Time Performance | 나머지 모든 QA의 선행 조건. 이것이 무너지면 데이터 자체가 없음 |
| **2** | Measurement Accuracy | 프로젝트의 존재 이유. WeiShi 비교가 최종 데모 핵심. 아키텍처 결정에 가장 큰 영향 |
| **3** | Low Latency | Hard threshold — 초과 시 기능 붕괴. 실험으로만 검증 가능 |
| **4** | Correctness | Accuracy + 단일 데이터 소스 구조로 자동 확보. 독립적 아키텍처 목표가 아님 |
| **5** | Extensibility | 개발자 편의성이지만 11개 그래프 일정 리스크를 직접 통제 |

```
Real-Time Performance   → 시스템이 살아있는가?           (존재 조건)
Measurement Accuracy    → 시스템이 올바른가?              (목적 달성)
Low Latency             → 시스템이 쓸 만한가?             (사용자 체감)
Correctness             → 시스템이 내부적으로 일관적인가?  (신뢰성)
Extensibility           → 시스템을 계속 만들 수 있는가?   (개발 지속성)
```

**English**

| Rank | QA | Rationale |
|------|----|-----------|
| **1** | Real-Time Performance | Precondition for all other QAs. Without it, no data exists to measure anything else |
| **2** | Measurement Accuracy | The reason the project exists. WeiShi comparison is the core final demo requirement. Greatest influence on architectural decisions |
| **3** | Low Latency | Hard threshold — exceeding it causes functional failure. Requires empirical validation via experiments |
| **4** | Correctness | Automatically secured by Accuracy + single data source structure. Not an independent architectural target |
| **5** | Extensibility | Developer-facing, but directly controls schedule risk across 11 graph implementations |

```
Real-Time Performance   → Is the system alive?               (existence condition)
Measurement Accuracy    → Is the system correct?             (purpose achieved)
Low Latency             → Is the system usable?              (user experience)
Correctness             → Is the system internally coherent? (trustworthiness)
Extensibility           → Can development continue?          (development sustainability)
```

---

## 2. QA Trade-off

### 충돌 관계 / Conflicting Relationships

**한국어**

| QA A | QA B | 관계 | 내용 |
|------|------|------|------|
| Real-Time Performance | Low Latency | ⚡ 충돌 | 처리량 늘리면 버퍼 크기 증가 → latency 증가 |
| Real-Time Performance | Extensibility | ⚡ 충돌 | 모듈 분리(Extensibility)는 호출 오버헤드 추가 → 처리 성능 저하 가능 |
| Real-Time Performance | Measurement Accuracy | ⚡ 충돌 | 정밀 감지(high sps)는 연산량 증가 → 처리 부하 상승 |
| Measurement Accuracy | Correctness | ✅ 보완 | T1/T3 정확히 잡으면 내부 일관성도 자연히 확보 |
| Low Latency | Correctness | ✅ 보완 | latency 낮추면 stale data 감소 → 일관성 향상 |

**English**

| QA A | QA B | Relationship | Description |
|------|------|-------------|-------------|
| Real-Time Performance | Low Latency | ⚡ Conflict | Increasing throughput grows buffer size → latency increases |
| Real-Time Performance | Extensibility | ⚡ Conflict | Module separation (Extensibility) adds call overhead → potential processing performance degradation |
| Real-Time Performance | Measurement Accuracy | ⚡ Conflict | High-precision detection (high sps) increases computation → higher processing load |
| Measurement Accuracy | Correctness | ✅ Complementary | Accurate T1/T3 detection naturally secures internal consistency |
| Low Latency | Correctness | ✅ Complementary | Lower latency reduces stale data → improved consistency |

### 핵심 Trade-off / Core Trade-off

**한국어**

```
Real-Time Performance (1순위)를 지키기 위해
  → 모듈 분리 수준(Extensibility)을 어디까지 허용할 것인가?
  → 버퍼 크기(Low Latency)를 얼마나 희생할 것인가?
```

**English**

```
To preserve Real-Time Performance (priority 1):
  → How much module separation (Extensibility) can we allow?
  → How much buffer size (Low Latency) are we willing to sacrifice?
```

---

## 3. 워크샵 합의 안건 / Workshop Consensus Agenda

### 안건 1 — QA 우선순위 확정 / Finalize QA Priority

**한국어**

> 위 제안 순서에 동의하는가? 다르게 생각하는 부분이 있는가?

- 논의 포인트: Extensibility vs Measurement Accuracy 순서
  - 일정 리스크를 더 중요하게 보면 → Extensibility 2순위 유지
  - 제품 품질을 더 중요하게 보면 → Measurement Accuracy 2순위로 변경

**English**

> Do we agree with the proposed order? Are there any points of disagreement?

- Discussion point: Extensibility vs. Measurement Accuracy ordering
  - If schedule risk is weighted more heavily → keep Extensibility at rank 2
  - If product quality is weighted more heavily → move Measurement Accuracy to rank 2

### 안건 2 — 미확정 수치 확정 / Finalize Undecided Numbers

**한국어**

| QA | 미확정 항목 | 확정 방법 |
|----|-----------|---------|
| Low Latency | 3구간 각 목표 ms | Experiment 1·2 결과 후 확정 (잠정 수치 설정 가능) |
| Correctness | 허용 편차 범위 | 오늘 팀 합의 |
| Measurement Accuracy | WeiShi 대비 오차 (s/d, ms) | Experiment 3 결과 후 확정 |
| Extensibility | 변경 파일 수 상한 | 오늘 팀 합의 |

**English**

| QA | Undecided item | How to finalize |
|----|---------------|----------------|
| Low Latency | Target ms for each of the 3 segments | After Experiment 1·2 results (tentative values can be set today) |
| Correctness | Acceptable deviation range | Team consensus today |
| Measurement Accuracy | Error vs. WeiShi (s/d, ms) | After Experiment 3 results |
| Extensibility | Upper limit on changed files | Team consensus today |

### 안건 3 — 핵심 Trade-off 입장 결정 / Decide Team Position on the Core Trade-off

**한국어**

> Real-Time Performance(1순위)와 충돌 시, 팀의 기본 입장은?

- **Option A**: 성능 우선 — 모듈 분리 최소화, latency 허용
- **Option B**: 구조 우선 — 성능 일부 희생하더라도 Extensibility 확보
- **Option C**: 실험 후 결정 — Experiment 1 결과 보고 판단

**English**

> When conflict arises with Real-Time Performance (rank 1), what is the team's default stance?

- **Option A**: Performance first — minimize module separation, accept higher latency
- **Option B**: Structure first — secure Extensibility even at some performance cost
- **Option C**: Decide after experiments — wait for Experiment 1 results before deciding

---

## 4. 합의 결과 기록 / Consensus Record

> 워크샵 당일 아래 표를 채울 것 / Fill in the table below on the day of the workshop

| 항목 / Item | 합의 내용 / Decision | 비고 / Notes |
|------------|---------------------|-------------|
| QA 우선순위 / QA priority | | |
| Low Latency 목표 ms (잠정) / Low Latency target ms (tentative) | | |
| Correctness 허용 편차 / Correctness acceptable deviation | | |
| Extensibility 파일 수 상한 / Extensibility file count upper limit | | |
| Trade-off 기본 입장 / Trade-off default stance | | |
