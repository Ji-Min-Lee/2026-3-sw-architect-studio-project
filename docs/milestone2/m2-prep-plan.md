# M2 발표 준비 계획 / M2 Presentation Prep Plan

**작성일 / Date**: 2026-06-15  
**마감일 / Due**: 2026-06-22 (Mon)  
**남은 기간 / Remaining**: 7일

---

## 1. M1 피드백 핵심 요약 / M1 Feedback Summary

### 1.1 부정적 지적 (반드시 M2에서 개선) / Critical Issues to Fix

| 영역 | M1 피드백 | M2 대응 방향 |
|------|-----------|-------------|
| **Project Plan** | Task에 담당자·날짜 없음. 실험이 task로 없음. Kanban 링크 없음. ADD 다이어그램 출처 미표기 | Updated Project Plan에 task별 담당자·기간 명시. 실험도 task로 등록 |
| **Architectural Drivers** | QA 문서에 tactics 혼재. "Provisional" QA. 솔루션 설명 침투. Overall 컬럼 불필요. Risk→QAS 1:1 매핑 불필요 | QA는 문제(goal)만 기술. Tactics는 Approaches 문서로 분리. Provisional 제거 → 실험 결과로 수치 확정 |
| **Experiments** | 9개 Risk 중 실험 연결 1개뿐. 실험 미시작 red flag | EXP-02 결과로 Risk 해소 매핑 명시. 각 실험이 어느 Risk를 해소했는지 1:N 관계로 기술 |
| **Navigation** | README 없음. 문서 간 연결 없음 | **README.md 신규 작성** 필수. 각 문서 상단에 상호 링크 |
| **Architecture 다이어그램** | M1에서 과도하게 상세했음. M2에서 clarity 개선 필요 | 다이어그램 단순화 + 범례 추가 + 핵심 메시지 명확화 |

### 1.2 긍정적 평가 (유지·강화) / Strengths to Keep

- QA 목표와의 관련성이 good → M2에서도 QA↔Architecture 연결 유지
- Measurability 고려 → 실험 수치로 QA 달성 여부 증명
- Risk와 목표 연결 → M2에서 Risk 해소 결과로 마무리
- Tactics↔QAS 관계 있음 → ADD 설계 결정(ADD-2-01, ADD-2-02)으로 심화

---

## 2. M2 스토리라인 / M2 Narrative

### 2.1 팀 목표 업데이트 (M1 → M2)

**M1 목표**: 시계 커버리지를 줄이더라도 정확한 수치를 제공하는 SW 개발

**M2 추가 목표**: **5주 내 완성 가능한 구조**를 먼저 확보

> M1 발표 이후, 일정 지연(실험 미시작)이 red flag로 지적됨.
> "주어진 시간 안에 완성한다"를 팀 목표에 명시적으로 추가.

**Trade-off 전략**:

```
완성 우선순위: 구조 완성 > 일정 내 기능 구현 > 정확도 극대화 > BPH 커버리지
```

| 조정 항목 | M1 | M2 |
|-----------|----|----|
| 지원 BPH 범위 | 넓게 목표 | **좁힘** (28,800 BPH 집중) |
| Optional 기능 | 동등 우선순위 | **후순위** (M3에서 시간 허용 시) |
| 정확도 | 최우선 | **QA 4순위** (trade-off 감수) |
| 구현 구조 | 후행 | **선행** (Modifiability 1순위) |

### 2.2 QA 우선순위 재정의 / Revised QA Priority

| 순위 | QA | Business | Risk | Rationale |
|:---:|---|:---:|:---:|---|
| **1** | **Modifiability** | M | H | 11개 그래프 병렬 구현의 **선행 조건**. 레이어 구조 없이는 개발자 간 블로킹 발생 → 일정 실패. 구조가 먼저 있어야 나머지 QA tactic 교체·적용이 가능 |
| **2** | **Real-Time + Low Latency** | H | H | EXP-02 RPi에서 deadline miss 43% 확인 → 가장 큰 설계 변경 필요. T2·R1 적용으로 해소 방향 검증 완료. 둘은 trade-off 관계 (SPS 높이면 latency ↑) |
| **3** | **Usability** | M | M | 신호 품질 경고 등. 구조 확보 후 추가. Optional |
| **4** | **Correctness** | L | L | 조용한 환경에서는 측정 가능 → 명시적 trade-off 감수. 시간 내 미달성 시 "조용한 환경 한정 동작"으로 범위 제한 |

> **M1 대비 변화**: Real-Time이 1순위 → 2순위로 이동. Modifiability가 1순위로 승격.
> 이유: 빠른 구현이 가능한 구조가 없으면 Real-Time tactic 자체를 적용할 수 없음.

### 2.3 스토리 흐름 (발표 내러티브) / Story Arc

```
[문제 인식]
M1 이후 일정 지연 위험 → "5주 내 완성"을 명시적 목표 추가

    ↓

[전략 선택: Modifiability 1순위]
11개 그래프 병렬 구현 + tactic 교체를 위한 레이어 구조 선구축
feature/layer-ex-baseline 브랜치: Presentation → Domain → Signal Processing 분리 완료

    ↓

[실험으로 검증: EXP-02]
RPi baseline: exec 20ms, plot 79%, deadline miss 43%  ← 구조 문제 확인
T2 (DSP Offload Thread) 적용: wait_ms 420ms → 0.013ms (×32,000), backlog 0%
R1 (Lazy Rendering) 적용: replot 75~85% 감소
→ Real-Time + Low Latency QA 달성 방향 확인

    ↓

[도메인 지식 리스크 완화]
Rate/Amplitude/Beat Error 산식 전체를 완전히 이해하기 어려움
→ AI를 활용하여 산식 해석 위임 + 단위 테스트로 안전 울타리 구축
→ 개발자는 구조·성능 집중, 도메인 정확도는 테스트가 보장

    ↓

[앞으로: Construction Plan]
Modifiability 구조 위에 그래프 빠르게 추가 → M3 RPi Demo
```

---

## 3. 현재 진행 상황 / Current Status

### 3.1 완료된 실험 / Completed Experiments

| 실험 | 결과 요약 | 해소된 Risk | 아키텍처 결정 |
|------|-----------|-------------|--------------|
| **EXP-02 Baseline (macOS)** | exec_ms 0.57ms, deadline miss 0%, wait_ms 420ms | TR-04 (Qt 렌더링 병목 구조 확인) | feature/layer 구조로 plot 분리 검증 |
| **EXP-02 T2 DSP Offload** | wait_ms 420ms → 0.013ms, backlog 47% → 0% | TR-01 (Real-Time 달성 가능성) | ADD-2-01: DSPWorker 별도 스레드 |
| **EXP-02 R1 Lazy Rendering** | replot 75~85% 감소 | TR-04 (렌더링 병목) | ADD-2-02: isVisible() guard |
| **EXP-02 RPi Baseline** | exec 20ms, plot 16ms(79%), miss 43%, 85°C | TR-01 구체화 | 구조적 문제 → T2 적용 근거 |

### 3.2 진행 중인 작업 / In Progress

| 브랜치 | 내용 | 상태 |
|--------|------|------|
| `feature/layer-ex-baseline` | 레이어 분리 + Logger 인프라 | ✅ 완료 |
| EXP-02 T2 적용 | DSPWorker 스레드 분리 | ✅ macOS 검증 완료 |
| EXP-02 R1 적용 | Lazy Rendering | ✅ macOS 검증 완료 |
| RPi T2 검증 | RPi에서 T2 효과 확인 | ⏳ 미완료 |

### 3.3 M2 제출 산출물 현황 / Deliverable Status

| 산출물 | 파일 | 현황 | 필요 작업 |
|--------|------|------|-----------|
| Updated Project Plan | `updated-project-plan.md` | 🔴 빈 템플릿 | QA 재정의 + task 담당자·날짜 채우기 |
| Experiment Results | `experiment-results.md` | 🔴 빈 템플릿 | EXP-02 실제 수치 채우기 (데이터 있음) |
| Architecture — Module View | `architecture-module-view.md` | 🟡 초안 있음 | Modifiability 1순위 반영, clarity 개선 |
| Architecture — C&C View | `architecture-runtime-view.md` | 🟡 초안 있음 | T2 스레드 구조 반영 |
| Architecture — Deployment View | `architecture-deployment-view.md` | 🟡 초안 있음 | RPi 배치 + 통신 채널 명확화 |
| Construction Plan | `construction-plan.md` | 🟡 초안 있음 | 남은 7일 현실적 일정 반영 |
| **README.md** | **없음** | 🔴 **없음** | **신규 작성 필수** (M1 피드백 핵심) |

---

## 4. M2 준비 작업 목록 / M2 Prep Tasks

### 우선순위 1 — 없으면 제출 불가 (by 06/18)

- [ ] **README.md 작성** — M2 산출물 전체 목차 + 각 문서 링크 + 스토리 요약 1페이지
- [ ] **experiment-results.md 작성** — EXP-02 T2·R1 수치 채우기 + Risk 해소 매핑
  - 데이터 출처: `exp-02-baseline-results.md`, `tactic-analysis.md`
  - RPi T2 결과가 없으면 "macOS 검증 완료, RPi 검증 예정" 명시

### 우선순위 2 — QA 재정의 반영 (by 06/19)

- [ ] **updated-project-plan.md 작성**
  - QA 우선순위 재정의 (Modifiability 1순위) + rationale
  - 팀 목표 업데이트 (5주 내 완성 + trade-off 명시)
  - task별 담당자·날짜 추가 (M1 피드백 핵심)
  - Risk 해소 현황 업데이트
- [ ] **architectural-approaches.md 보강**
  - Modifiability 접근법 추가 (레이어 분리, BaseGraphTab 인터페이스)
  - T2·R1 ADD 결정을 Approaches 문서로 통합
  - AI + 단위 테스트 전략 명시

### 우선순위 3 — 다이어그램 clarity 개선 (by 06/20)

- [ ] **architecture-module-view.md**
  - 레이어 경계 명확화 + 금지 의존성(❌) 강조
  - BaseGraphTab 인터페이스 위치 명시 (Modifiability 핵심)
- [ ] **architecture-runtime-view.md (C&C View)**
  - T2 DSPWorker 스레드 추가 (Audio → DSP → GUI 3-thread 구조)
  - Qt::QueuedConnection 커넥터 명시
- [ ] **architecture-deployment-view.md**
  - RPi 5 하드웨어 배치 + USB 마이크 연결 채널 명확화

### 우선순위 4 — Construction Plan 현실화 (by 06/21)

- [ ] **construction-plan.md 업데이트**
  - 남은 7일(06/15~06/22) 기준 현실적 일정
  - Phase A (Core Pipeline) / Phase B (Core Graphs) / Phase C (RPi 검증) 분리
  - 담당자 배정

### 우선순위 5 — 마무리 검토 (06/21~06/22)

- [ ] 전체 문서 cross-reference 확인 (README → 각 문서 링크 동작)
- [ ] M1 mentor 질문 체크리스트 기준 self-review
- [ ] Git repo collaborator 추가 (Jason, Steve, Dan, Choonsik Lee)

---

## 5. M2 Mentor 질문 대비 / Mentor Q&A Prep

### Project Plan 관련

| 예상 질문 | 대응 |
|-----------|------|
| "실험이 언제 시작됐나, 누가 담당했나?" | EXP-02 담당자·날짜 task로 명시 |
| "일정 지연 어떻게 만회할 건가?" | Modifiability 구조로 병렬 구현 속도 확보 설명 |
| "5주 안에 진짜 가능한가?" | Construction Plan의 현실적 scope down 보여주기 |

### Experiments 관련

| 예상 질문 | 대응 |
|-----------|------|
| "어떤 실험을 했나?" | EXP-02 4단계(Baseline macOS → RPi → T2 → R1) |
| "실험 결과가 open question을 해소했나?" | deadline miss 43% → 0% 수치로 답변 |
| "아직 남은 실험이 있나?" | RPi T2 검증, EXP-01 (96k sps on RPi) 미완료 명시 |
| "실험이 시스템 목표와 연관되나?" | QAS-1·QAS-2 해소 → QA 매핑 표로 답변 |

### Architecture 관련

| 예상 질문 | 대응 |
|-----------|------|
| "실험 결과가 아키텍처 refinement로 이어졌나?" | RPi miss 43% → T2 tactic 선택 → ADD-2-01 결정 |
| "선택한 접근법의 trade-off는?" | T2: 동기화 복잡도 증가 / R1: 탭 전환 시 일시적 stale view |
| "Critical concern이 남아 있나?" | RPi 96k sps 검증 미완 → Construction Plan에 반영 |
| "아키텍처 평가를 수행했나?" | EXP-02 수치 기반 before/after 비교 = informal evaluation |

---

## 6. 도메인 지식 리스크 대응 전략 / Domain Knowledge Risk Mitigation

**문제**: Rate/Amplitude/Beat Error 산식이 복잡. 팀 전체가 완전히 이해하기 어려움.

**전략**:

```
1. 기본 도메인 지식 습득 (현재 진행 중)
   - Witschi Training Course pp.14-19
   - TimeGrapher Equations_v0.docx.pdf
   - 기존 코드 분석 (Detector.cpp, Measurement.cpp)

2. AI 활용으로 산식 해석 위임
   - 복잡한 산식(Phase Error, Lift Angle 보정)은 AI에게 단계별 검증 요청
   - 개발자는 구조/성능 집중

3. 단위 테스트로 안전 울타리 구축
   - Rate / Amplitude / Beat Error 계산 로직에 단위 테스트 작성
   - 기존 동작(WeiShi 비교값)을 golden test로 고정
   - 구조 변경 시 회귀 방지
```

> **발표 포인트**: "도메인 지식 부족 리스크를 AI + 테스트로 완화했다"는 명시적 trade-off 결정으로 제시

---

## 7. 참고 문서 / References

| 문서 | 용도 |
|------|------|
| `exp-02-baseline-results.md` | EXP-02 수치 데이터 원본 |
| `tactic-analysis.md` | T2·R1 ADD 결정 기록 |
| `architectural-approaches.md` | 전술 옵션·trade-off |
| `docs/milestone1/final/` | M1 최종 제출본 (비교 참조) |
| `docs/milestone1/final/02-architectural-drivers.md` | 기존 QA 정의 (재정의 기준) |
