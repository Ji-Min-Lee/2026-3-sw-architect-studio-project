# Architectural Drivers — TimeGrapher

**Milestone**: M1 | **Due**: 2026-06-09 | **Updated**: 2026-06-03

---

## 1. Project Context & Objectives

TimeGrapher는 기계식 시계의 탈진기 진동에서 발생하는 음향 신호(beat noise)를 분석하여 시계 성능을 실시간으로 진단하는 시스템이다. 마이크로 시계의 tick-tock 소리를 수집하고, A(T1)·C(T3) 이벤트를 감지하여 Rate, Amplitude, Beat Error를 계산하고 GUI에 표시한다.

| 목표 | 설명 |
|------|------|
| 실시간 진단 | Raspberry Pi 5 위에서 음향 신호를 실시간으로 수집·처리·표시 |
| 정확한 측정 | Rate, Amplitude, Beat Error 값이 WeiShi No.1000 레퍼런스 디바이스와 일치 |
| 확장 가능한 구조 | 새로운 그래프·분석 기능을 기존 코드 수정 없이 추가 가능 |
| 아키텍처 원칙 실증 | CMU MSE 소프트웨어 아키텍처 설계 원칙을 적용한 시스템 설계 |

---

## 2. Functional Requirements

| ID | 기능 요건 | 우선순위 | 현재 구현 상태 |
|----|---------|---------|-------------|
| FR-01 | T1(A), T3(C) 음향 이벤트 감지 | HIGH | ✅ 구현됨 (`Detector.cpp`) |
| FR-02 | Rate (s/d), Amplitude (°), Beat Error (ms), BPH 계산 | HIGH | ✅ 구현됨 |
| FR-03 | Live / Playback / Sim 운영 모드 지원 | HIGH | ✅ 구현됨 |
| FR-04 | Low-pass / High-pass 필터링 | HIGH | ⚠️ 부분 구현 (HPF만) |
| FR-05 | Trace Display (Rate + Amplitude 실시간 기록) | MEDIUM | ❌ 미구현 |
| FR-06 | Rate & Amplitude Stability / Vario (Min/Max/Avg/σ) | MEDIUM | ❌ 미구현 |
| FR-07 | Beat Error Display & Diagnostic Trace | MEDIUM | ❌ 미구현 |
| FR-08 | Pause + 시간축 탐색 (앞/뒤 이동) | LOW | ❌ 미구현 |
| FR-09 | AI 신호 품질 분류 — 수집 신호의 노이즈 수준·품질 분류 및 경고 표시 (선택적) | LOW | ❌ 미구현 |

---

## 3. Quality Attribute Scenarios

### 3.1 Priority & Rationale

5개의 QA를 "아키텍처 결정에 가장 큰 제약을 주는 순서"로 우선순위를 결정했다.

| 순위 | QA | 한 줄 근거 |
|------|-----|---------|
| **1** | Real-Time Performance | 나머지 모든 QA의 선행 조건. 이것이 무너지면 데이터 자체가 없음 |
| **2** | Measurement Accuracy | 프로젝트의 존재 이유. 아키텍처 결정(sps, 감지 알고리즘)에 가장 큰 영향 |
| **3** | Low Latency | Beat 주기라는 hard threshold 존재. 초과 시 기능 붕괴. 실험으로만 검증 가능 |
| **4** | Correctness | Accuracy + 단일 데이터 소스 구조로 자동 확보. 독립적 아키텍처 목표가 아님 |
| **5** | Extensibility | 개발자 편의성이지만 11개 그래프 구현 일정 리스크를 직접 통제 |

---

### QAS-1: Real-Time Performance ★ Priority 1

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (음향 신호 발생) |
| **Stimulus** | USB 센서를 통해 연속적인 음향 샘플 스트림 입력 |
| **Artifact** | 신호 수집 → 처리 → 분석 → GUI 표시 파이프라인 전체 |
| **Environment** | Raspberry Pi 5 (8GB RAM), 정상 운영 상태 |
| **Response** | 수집된 샘플을 끊김 없이 실시간으로 처리하고 GUI에 표시 |
| **Response Measure** | - **Objective**: 96,000 sps 처리 유지<br>- **Minimum**: 48,000 sps (이 이하면 프로젝트 실패)<br>- **Stretch**: 192,000 sps<br>- Dropped audio block: **0개** |

---

### QAS-2: Measurement Accuracy ★ Priority 2

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (escapement 진동 발생) |
| **Stimulus** | T1(impulse), T3(lock+banking) 이벤트 포함 음향 신호 입력 |
| **Artifact** | 신호 처리 파이프라인의 beat event detection 모듈 |
| **Environment** | 정상 운영 상태 / 신호가 약하거나 노이즈가 있는 열화 환경 |
| **Response** | T1·T3 onset/peak를 정확히 감지하여 Rate, Amplitude, Beat Error 계산. 신호 열화 시 불안정한 값 대신 명확한 경고 표시 |
| **Response Measure** | - WeiShi No.1000 대비 Rate 오차: **< 5 s/d** *(잠정, [EX-02](./planned-experiments.md#ex-02-beat-event-detection-accuracy)·[EX-03](./planned-experiments.md#ex-03-filter-parameter-sweep) 결과 후 확정)*<br>- WeiShi No.1000 대비 Beat Error 오차: **< 0.1 ms** *(잠정, [EX-02](./planned-experiments.md#ex-02-beat-event-detection-accuracy)·[EX-03](./planned-experiments.md#ex-03-filter-parameter-sweep) 결과 후 확정)*<br>- 신호 열화 시 graceful degradation (불안정 출력 없음) |

---

### QAS-3: Low Latency ★ Priority 3

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (beat 이벤트 발생) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처 시작 |
| **Artifact** | capture → beat detection → 계산 → GUI 렌더링 전 구간 |
| **Environment** | Raspberry Pi 5, 실시간 운영 상태 |
| **Response** | 캡처된 샘플에 대한 파형, 마커, 계산값이 GUI에 표시됨 |
| **Response Measure** | - ① capture→process: **< 70 ms** *(잠정)*<br>- ② process→display: **< 30 ms** *(잠정)*<br>- ③ end-to-end: **< 100 ms** *(잠정, 28,800 BPH 기준)*<br>- Dropped audio block: 0, Missed beat: 0<br>*([EX-01](./planned-experiments.md#ex-01-sample-rate-performance-on-raspberry-pi-5)·[EX-02](./planned-experiments.md#ex-02-beat-event-detection-accuracy) 결과 후 확정)* |

---

### QAS-4: Correctness ★ Priority 4

| 항목 | 내용 |
|------|------|
| **Source** | 사용자 (동일 시계, 동일 조건에서 측정) |
| **Stimulus** | 실시간 음향 신호 수신 중 다수의 GUI 뷰 동시 표시 |
| **Artifact** | Rate, Amplitude, Beat Error 계산 로직 및 GUI 전체 뷰 |
| **Environment** | 정상 운영 상태 / ambient noise가 존재하는 환경 |
| **Response** | 모든 GUI 뷰에서 동일 데이터 기반으로 일관된 수치 표시 |
| **Response Measure** | 동일 beat 데이터에서 파생된 모든 뷰의 수치 편차: **0** (허용 없음)<br>단일 데이터 소스 구조로 구조적 보장 |

---

### QAS-5: Extensibility ★ Priority 5

| 항목 | 내용 |
|------|------|
| **Source** | 개발자 (새 그래프 또는 분석 기능 추가) |
| **Stimulus** | 기존 코드베이스에 새로운 그래프/필터/디스플레이 모드 추가 요청 |
| **Artifact** | GUI 모듈 구조 전체 (신호 수집 / 처리 / 계산 / 표시 레이어) |
| **Environment** | 개발 환경 (PC 또는 RPi), 개발 기간 내 |
| **Response** | 기존 모듈의 major redesign 없이 새 기능을 독립적으로 추가·테스트 가능 |
| **Response Measure** | 새 그래프 1개 추가 시 변경 파일 수: **≤ 3개** *(잠정)*<br>기존 계산 로직 수정 없이 표시 레이어만 추가 가능 |

---

## 4. Design Constraints

> Design Constraint = 이미 내려진 설계 결정. 아키텍처 논의 대상이 아니며 변경 불가.

| 제약 | 내용 | 변경 불가 이유 |
|------|------|-------------|
| 플랫폼 | Raspberry Pi 5 (ARM64, 8GB RAM) | 프로젝트에서 하드웨어 지급 |
