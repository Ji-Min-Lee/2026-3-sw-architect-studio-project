# QA 팀 합의 준비

> **작성일**: 2026-06-02  
> **목적**: 워크샵에서 팀 합의를 이끌기 위한 사전 분석 및 논의 안건

---

## 1. QA 우선순위

### 판단 기준

> 아키텍처에 가장 큰 제약을 주는 QA = 1순위  
> 이 결정이 나머지 설계 방향의 전제가 됨

### 제안 우선순위

| 순위 | QA | 근거 |
|------|----|------|
| **1** | Real-Time Performance | RPi 하드웨어 한계가 고정값. 48k sps 미달 시 프로젝트 자체가 실패. 모든 설계의 전제 조건 |
| **2** | Extensibility | 그래프 11개 + Enhanced 기능 추가가 전체 일정과 직결. 구조가 나쁘면 매 추가마다 일정 리스크 발생 |
| **3** | Measurement Accuracy | 시스템의 존재 이유. 수치가 틀리면 TimeGrapher로서 의미 없음 |
| **4** | Low Latency | Real-Time Performance와 연동. 단독으로는 덜 결정적 |
| **5** | Correctness | Measurement Accuracy가 확보되면 따라오는 속성 |

---

## 2. QA Trade-off

### 충돌 관계

| QA A | QA B | 관계 | 내용 |
|------|------|------|------|
| Real-Time Performance | Low Latency | ⚡ 충돌 | 처리량 늘리면 버퍼 크기 증가 → latency 증가 |
| Real-Time Performance | Extensibility | ⚡ 충돌 | 모듈 분리(Extensibility)는 호출 오버헤드 추가 → 처리 성능 저하 가능 |
| Real-Time Performance | Measurement Accuracy | ⚡ 충돌 | 정밀 감지(high sps)는 연산량 증가 → 처리 부하 상승 |
| Measurement Accuracy | Correctness | ✅ 보완 | T1/T3 정확히 잡으면 내부 일관성도 자연히 확보 |
| Low Latency | Correctness | ✅ 보완 | latency 낮추면 stale data 감소 → 일관성 향상 |

### 핵심 Trade-off

```
Real-Time Performance (1순위)를 지키기 위해
  → 모듈 분리 수준(Extensibility)을 어디까지 허용할 것인가?
  → 버퍼 크기(Low Latency)를 얼마나 희생할 것인가?
```

---

## 3. 워크샵 합의 안건

### 안건 1 — QA 우선순위 확정

> 위 제안 순서에 동의하는가? 다르게 생각하는 부분이 있는가?

- 논의 포인트: Extensibility vs Measurement Accuracy 순서
  - 일정 리스크를 더 중요하게 보면 → Extensibility 2순위 유지
  - 제품 품질을 더 중요하게 보면 → Measurement Accuracy 2순위로 변경

### 안건 2 — 미확정 수치 확정

| QA | 미확정 항목 | 확정 방법 |
|----|-----------|---------|
| Low Latency | 3구간 각 목표 ms | Experiment 1·2 결과 후 확정 (잠정 수치 설정 가능) |
| Correctness | 허용 편차 범위 | 오늘 팀 합의 |
| Measurement Accuracy | WeiShi 대비 오차 (s/d, ms) | Experiment 3 결과 후 확정 |
| Extensibility | 변경 파일 수 상한 | 오늘 팀 합의 |

### 안건 3 — 핵심 Trade-off 입장 결정

> Real-Time Performance(1순위)와 충돌 시, 팀의 기본 입장은?

- **Option A**: 성능 우선 — 모듈 분리 최소화, latency 허용
- **Option B**: 구조 우선 — 성능 일부 희생하더라도 Extensibility 확보
- **Option C**: 실험 후 결정 — Experiment 1 결과 보고 판단

---

## 4. 합의 결과 기록

> 워크샵 당일 아래 표를 채울 것

| 항목 | 합의 내용 | 비고 |
|------|---------|------|
| QA 우선순위 | | |
| Low Latency 목표 ms (잠정) | | |
| Correctness 허용 편차 | | |
| Extensibility 파일 수 상한 | | |
| Trade-off 기본 입장 | | |
