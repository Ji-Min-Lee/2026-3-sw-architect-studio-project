# TimeGrapher — Architecture Document

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09

---

# Overview

## Introduction

### Purpose

TimeGrapher 프로젝트는 기계식 시계의 탈진기(escapement) 진동에서 발생하는 음향 신호를 분석하여 시계 성능을 실시간으로 진단하는 시스템을 설계하고 구현하는 것을 목적으로 한다.

기존 베이스코드(`TimeGrapher_v10.5`)는 단일 `MainWindow` 클래스에 신호 수집·처리·계산·렌더링이 모두 집중된 God Object 구조로, 새로운 기능을 추가하거나 팀원이 병렬로 개발하기 어려운 상태이다. 본 과제는 이 구조를 **모듈화·확장 가능한 아키텍처로 재설계**하고, Raspberry Pi 5 위에서 11개 그래프를 실시간으로 처리·표시하는 시스템을 완성하는 것이다.

---

### Objectives

#### Goals

**팀 협업 / Team Collaboration**
- 모든 팀원이 아키텍처 설계 과정에 참여하고, 설계 결정의 근거를 함께 이해한다
- 코딩팀과 아키텍처팀 간 의견을 정기적으로 조율하여 설계 결정이 구현에 반영되도록 한다

**기술 목표 / Technical Goals**
- 기존 베이스코드를 분석하고, 팀 합의 하에 모듈화 방향을 결정한다
- Raspberry Pi 5 위에서 실시간으로 동작하는 시스템을 팀 전체가 함께 완성한다

**학습 목표 / Learning Goals**
- CMU MSE 소프트웨어 아키텍처 설계 원칙(ADD)을 실제 프로젝트에 적용하고, 그 결과를 발표로 공유한다


---

## Plan

| 주차 | 기간 | 주요 목표 | 산출물 |
|------|------|---------|--------|
| Week 0 | 05/27~05/29 | 환경 구축, 베이스코드 빌드, 문서 읽기 | 개발 환경 완성 |
| Week 1 | 06/01~06/05 | 킥오프 워크샵, 코드 분석, M1 문서 초안 | Drivers / Risk / Experiments / Approaches 초안 |
| Week 2 | 06/08~06/12 | **M1 제출 (06/09)**, EX-01·EX-02 실험 시작 | M1 제출 완료, 실험 데이터 수집 시작 |
| Week 3 | 06/15~06/19 | EX-03, 실험 결과 반영, 아키텍처 뷰 작성, Graphs 1~4 구현 | Module/C&C/Deployment View, Graphs 1~4 |
| Week 4 | 06/22~06/26 | **M2 제출 (06/22)**, Graphs 5~11, RPi 통합 검증 | M2 제출 완료, 전체 기능 구현 |
| Week 5 | 06/29~07/01 | 최종 검증, 발표 준비, **Final Demo (07/01)** | QA 증거 수집, 발표 자료 |

---

---

# Requirements Analysis

## Functional Requirements

| ID | 기능 요건 | 우선순위 | 현재 상태 |
|----|---------|---------|---------|
| FR-01 | T1(A), T3(C) 음향 이벤트 감지 | HIGH | ✅ 구현됨 (`Detector.cpp`) |
| FR-02 | Rate(s/d) / Amplitude(°) / Beat Error(ms) / BPH 계산 | HIGH | ✅ 구현됨 |
| FR-03 | Live / Playback / Sim 운영 모드 | HIGH | ✅ 구현됨 |
| FR-04 | Low-pass / High-pass 필터링 | HIGH | ⚠️ HPF만 존재 |
| FR-05 | Trace Display (Rate + Amplitude 실시간 기록) | HIGH | ❌ 미구현 |
| FR-06 | Rate & Amplitude Stability / Vario (Min/Max/Avg/σ) | HIGH | ❌ 미구현 |
| FR-07 | Beat Error Display & Diagnostic Trace | HIGH | ❌ 미구현 |
| FR-08 | Beat-Noise Scope (Scope 1 & 2) | MEDIUM | ⚠️ 부분 구현 |
| FR-09 | Multi-Position Sequence Display (최대 10포지션) | MEDIUM | ❌ 미구현 |
| FR-10 | Long-Term Performance Graph | MEDIUM | ❌ 미구현 |
| FR-11 | Escapement Analyzer & Marker-Line Display | MEDIUM | ❌ 미구현 |
| FR-12 | Time-Frequency Spectrogram | MEDIUM | ❌ 미구현 |
| FR-13 | Waveform Comparison Display + Timing Markers | MEDIUM | ❌ 미구현 |
| FR-14 | Scope Mode (Synchronized Sweep) | MEDIUM | ❌ 미구현 |
| FR-15 | Scope Function (F0/F1/F2/F3 동시 표시) | MEDIUM | ❌ 미구현 |
| FR-16 | Watch-Position Testing (CH/CB/9H/6H/3H/12H) | MEDIUM | ❌ 미구현 |
| FR-17 | Pause + 시간축 탐색 (앞/뒤 이동) | HIGH | ❌ 미구현 |
| FR-18 | Latency 3구간 계측 보고 | HIGH | ❌ 미구현 |
| FR-19 | AI 신호 품질 분류 (optional) | LOW | ❌ 미구현 |

---

## Quality Attribute Requirements (Scenarios)

### QA 우선순위

| 순위 | QA | 한 줄 근거 |
|------|-----|---------|
| 1 | Real-Time Performance | 나머지 모든 QA의 선행 조건. 이것이 무너지면 데이터 자체가 없음 |
| 2 | Measurement Accuracy | 프로젝트의 존재 이유. sps·감지 알고리즘 결정에 가장 큰 영향 |
| 3 | Low Latency | Beat 주기라는 hard threshold 존재. 초과 시 기능 붕괴 |
| 4 | Correctness | Accuracy + 단일 데이터 소스 구조로 자동 확보 |
| 5 | Extensibility | 11개 그래프 구현 일정 리스크를 직접 통제 |
| 6 | Modifiability | 모듈화 재설계의 핵심 목표. 병렬 개발 가능 여부 결정 |

---

### Performance

#### QA Attributes

| # | QA Attribute | 목표 수치 | 상태 |
|---|-------------|---------|------|
| P-1 | Real-Time Performance | 96,000 sps 유지, Dropped block = 0 | ✅ 확정 |
| P-2 | Low Latency — capture→process | < 70 ms (28,800 BPH 기준) | ⚠️ 잠정 |
| P-3 | Low Latency — process→display | < 30 ms | ⚠️ 잠정 |
| P-4 | Low Latency — end-to-end | < 100 ms (28,800 BPH 기준) | ⚠️ 잠정 |
| P-5 | Throughput | 4,096 샘플 블록을 85.3 ms(48k) / 42.7 ms(96k) 이내 처리 완료 | ✅ 확정 |
| P-6 | Graceful Degradation | 96k sps 불가 시 48k sps 자동 폴백, 시스템 중단 없음 | ✅ 확정 |

**Priority:** P-1 > P-5 > P-4 > P-2 > P-3 > P-6

**Risk:**

| 리스크 | 내용 |
|--------|------|
| TR-03 | RPi 5에서 Qt GUI + DSP 동시 실행 시 96k sps 지속 불가 |
| TR-04 | Qt 11개 탭 동시 렌더링 시 FPS 저하 |
| TR-07 | FFT 스펙트로그램 활성화 시 CPU 과부하 |

**Experiment:**

| 실험 | 검증 대상 |
|------|---------|
| EX-01 | 48k / 96k / 192k sps별 CPU 점유율, Dropped block, GUI FPS, 3구간 지연 실측 |

**Tactic / Pattern:**

| 전술 | 적용 |
|------|------|
| AP-4 Background Thread | Layer 1~3 백그라운드, Layer 4 UI 스레드 분리 → 렌더링이 오디오 처리 블로킹 방지 |
| AP-1 Pipe-and-Filter | 4,096 샘플 최소 버퍼 유지 → 단계 간 지연 최소화 |
| AP-6 Graceful Degradation | 192k → 96k → 48k sps 단계적 폴백 |

---

#### Scenario: Real-Time Performance

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (음향 신호 발생) |
| **Stimulus** | USB 센서를 통해 연속적인 음향 샘플 스트림 입력 |
| **Artifact** | 신호 수집 → 처리 → 분석 → GUI 표시 파이프라인 전체 |
| **Environment** | Raspberry Pi 5, 정상 운영 상태 |
| **Response** | 수집된 샘플을 끊김 없이 실시간으로 처리하고 GUI에 표시 |
| **Response Measure** | 96,000 sps 유지, Dropped audio block = **0** |

> 시스템은 96,000 sps의 음향 데이터를 중단 없이 처리하여 GUI에 표시해야 한다. 오디오 블록이 1개라도 누락되면 A·C 이벤트가 손실되어 Rate·Amplitude·Beat Error 계산이 불가능해지므로, Dropped block = 0은 선택이 아닌 **시스템 생존 조건**이다.

---

#### Scenario: Low Latency

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (beat 이벤트 발생) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처 시작 |
| **Artifact** | capture → beat detection → 계산 → GUI 렌더링 전 구간 |
| **Environment** | Raspberry Pi 5, 실시간 운영 상태 |
| **Response** | 파형, 마커, 계산값이 GUI에 표시됨 |
| **Response Measure** | end-to-end **< 100 ms** (28,800 BPH 기준, 잠정) |

```
beat 주기 도출:
  28,800 BPH → 1 beat = 125 ms
  안전 마진 80% 적용 → end-to-end < 100 ms

  ① capture → process : < 70 ms (전체의 70%)
  ② process → display : < 30 ms (전체의 30%)

end-to-end가 beat 주기를 초과하면:
  이전 beat 처리 중 다음 beat 도착 → backlog 누적 → dropped beat
  → Real-Time Performance 실패와 동일한 결과
```

---

### Usability

#### QA Attributes

| # | QA Attribute | 목표 수치 | 상태 |
|---|-------------|---------|------|
| U-1 | Modifiability | 새 그래프 추가 시 변경 파일 수 ≤ 3개 | ⚠️ 잠정 |
| U-2 | Testability | MeasurementEngine이 QWidget 의존성 없이 단독 컴파일·테스트 가능 | ❌ 미달성 |
| U-3 | Integrability | 팀원 3명 동시 작업 시 merge conflict 파일 수 = 0 | ❌ 미달성 |
| U-4 | Correctness | 동일 beat 데이터 기반 모든 뷰의 수치 편차 = 0 | ✅ 확정 (구조적 보장) |
| U-5 | Measurement Accuracy | Rate 오차 < 5 s/d, Beat Error 오차 < 0.1 ms (WeiShi No.1000 대비) | ⚠️ 잠정 |
| U-6 | Portability | macOS·Windows·RPi 빌드 지원, 오디오 기능 플랫폼별 정상 동작 | ⚠️ 부분 달성 |

**Priority:** U-4 > U-5 > U-1 > U-2 > U-3 > U-6

**Risk:**

| 리스크 | 내용 |
|--------|------|
| TR-05 | MainWindow God Object — 3명 동시 수정 시 merge conflict 상시 발생 |
| TR-06 | macOS 빌드 분기 없음 — 팀원 2명 오디오 동작 불가 |
| TR-09 | Rate/Amplitude/Beat Error 계산 로직이 MainWindow에 묻혀 단위 테스트 불가 |
| TR-01 | C 이벤트 placement 설정 미최적화 — WeiShi 대비 오차 margin 미검증 |

**Experiment:**

| 실험 | 검증 대상 |
|------|---------|
| EX-02 | WeiShi No.1000 & RPi 동시 측정 환경 구축 가능 여부 |
| EX-03 | `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` 설정별 Rate/Amplitude 오차 비교 |

**Tactic / Pattern:**

| 전술 | 적용 |
|------|------|
| AP-3 Layered Architecture | Presentation 레이어만 추가 → 하위 3개 레이어 수정 없음 → ≤ 3 파일 보장 |
| AP-2 Observer / Signal-Slot | `connect` 한 줄로 새 탭 데이터 연결 → 계산 코드 수정 없음 |
| AP-5 Single Data Source | MeasurementEngine 단일 계산 → 모든 뷰 동일 BeatResult 수신 → Correctness 구조적 보장 |

---

#### Scenario: Modifiability

| 항목 | 내용 |
|------|------|
| **Source** | 개발자 (새 그래프 추가) |
| **Stimulus** | 기존 코드베이스에 새 그래프 탭 1개 추가 요청 |
| **Artifact** | 전체 모듈 구조 |
| **Environment** | 개발 기간 내, 팀원 3명 병렬 작업 중 |
| **Response** | 새 파일 1개 생성 + 기존 파일 2곳 등록 라인 추가로 완성 |
| **Response Measure** | 변경 파일 수 **≤ 3개**, 기존 계산·신호처리 모듈 수정 없음 |

> 개발자가 새 그래프 탭(예: Long-Term Performance)을 추가할 때, 시스템은 신호 처리·계산 모듈을 수정하지 않고 새 위젯 파일 1개 생성과 탭 등록 2줄 추가만으로 기능이 완성되어야 한다.
>
> 현재 구조에서는 `MainWindow.cpp` 1개에 모든 로직이 집중되어 있어 새 그래프 추가마다 1,749줄 전체를 수정해야 한다. 모듈화 이후에는 추가된 그래프가 기존 그래프 동작에 영향을 주지 않음이 보장되어야 한다.

```
새 그래프 추가 시 변경 범위:
  Layer 1~3  ← 수정 없음
  Layer 4
    ① NewGraphTab.h/.cpp  ← 신규 생성
    ② TabPanel.cpp        ← 탭 등록 1줄
    ③ MainWindow.cpp      ← connect 1줄
                             합계: 3개
```

---

#### Scenario: Correctness

| 항목 | 내용 |
|------|------|
| **Source** | 사용자 (동일 시계, 동일 조건) |
| **Stimulus** | 실시간 음향 신호 수신 중 다수의 GUI 뷰 동시 표시 |
| **Artifact** | Rate / Amplitude / Beat Error 계산 로직 및 GUI 전체 뷰 |
| **Environment** | 정상 운영 상태 |
| **Response** | 모든 GUI 뷰에서 동일 데이터 기반으로 일관된 수치 표시 |
| **Response Measure** | 동일 beat 데이터 기반 모든 뷰의 수치 편차 = **0** |

> 시스템은 11개 그래프 탭 어디서 Rate를 표시하더라도 동일한 값을 보여야 한다. MeasurementEngine이 단일 `BeatResult`를 emit하고 모든 탭이 이를 그대로 수신하는 구조이므로, 수치 불일치는 구조적으로 발생할 수 없다.

---

#### Scenario: Measurement Accuracy

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (escapement 진동) |
| **Stimulus** | T1(impulse), T3(lock+banking) 이벤트 포함 음향 신호 입력 |
| **Artifact** | 신호 처리 파이프라인의 beat event detection 모듈 |
| **Environment** | 정상 운영 상태 / 신호가 약하거나 노이즈가 있는 환경 |
| **Response** | T1·T3 onset/peak를 정확히 감지하여 Rate, Amplitude, Beat Error 계산 |
| **Response Measure** | Rate 오차 < **5 s/d**, Beat Error 오차 < **0.1 ms** (WeiShi No.1000 대비, 잠정) |

> 시스템은 동일한 시계에 대해 WeiShi No.1000이 측정한 값과 ±5 s/d 이내의 Rate 오차, ±0.1 ms 이내의 Beat Error 오차를 달성해야 한다. `TG_C_PLACEMENT_ONSET`과 `TG_C_PLACEMENT_PEAK` 중 어느 설정이 오차를 최소화하는지는 EX-03 실험으로 확정한다.

---

## Experiments

| ID | 실험 | 해결 대상 | 검증 QA | 완료 목표 |
|----|------|---------|---------|---------|
| EX-01 | RPi 성능 벤치마크 — 48k/96k/192k sps × Qt GUI 부하 + FFT 조건 | TR-03, TR-04, TR-07 | P-1, P-2, P-3, P-4, P-5 | 2026-06-07 |
| EX-02 | WeiShi No.1000 & RPi 동시 측정 환경 구축 | TR-01 | U-5 | 2026-06-07 |
| EX-03 | `tg_c_placement_t` 비교 — PEAK vs ONSET | TR-01 | U-5 | 2026-06-08 |

> **권장 실행 순서**: EX-01 & EX-02 병렬 → EX-03 (EX-02 완료 후)

### EX-01 요약: RPi 성능 벤치마크

**질문:** Qt GUI가 실행 중인 RPi 5에서 오디오 블록 드롭 없이 지속 가능한 최대 sps는 얼마인가? QA-3의 3구간 지연은 각각 얼마인가?

**절차:**
1. AlsaMixer에서 AGC 비활성화 후 Sim 모드 실행
2. 48k → 96k → 192k sps 순서로 각 5분 측정
3. 각 sps에서 CPU 점유율, Dropped block, GUI FPS, 3구간 지연 측정
4. FFT 스펙트로그램 탭 활성화 후 동일 측정 반복

**완료 기준:** 목표 sps 팀 합의 완료, QA-1/QA-3 잠정값이 실측값으로 대체됨

---

### EX-02 요약: WeiShi & RPi 동시 측정 환경

**질문:** WeiShi No.1000과 RPi가 동일 시계 신호를 동시에 수집하는 환경 구축이 가능한가?

**절차:**
1. 마이크 스플리터로 두 장비 동시 연결 시도
2. 성공 시: 동일 신호 비교, 재현성 3회 확인
3. 실패 시: 순차 측정 방식 채택 + 드리프트 오차 추정

**완료 기준:** 측정 방식 결정 및 문서화, 3회 재현성 확인

---

### EX-03 요약: tg_c_placement_t 비교

**질문:** `TG_C_PLACEMENT_PEAK`와 `TG_C_PLACEMENT_ONSET` 중 어느 설정이 WeiShi No.1000 대비 Rate/Amplitude 오차를 최소화하는가?

**절차:**
1. WeiShi No.1000으로 레퍼런스 Rate, Beat Error 기록
2. 동일 시계 신호를 PCM 녹음 (30초 이상)
3. Playback 모드에서 PEAK / ONSET 각각 실행 후 오차 비교
4. 시계 2종 이상 반복하여 일반성 검증

**완료 기준:** 최적 placement 설정 팀 합의, QA-2 잠정값이 실측값으로 대체됨
