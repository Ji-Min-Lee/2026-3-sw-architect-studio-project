# 아키텍처 드라이버 / Architectural Drivers — TimeGrapher

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **작성일 / Date**: 2026-06-07

---

## 0. 이 문서의 구조 / Document Structure

**한국어**

이 문서는 아래 5가지 멘토 리뷰 질문에 순서대로 답하는 방식으로 구성된다.

1. QA 요구사항이 "actionable"한가? (설계 검증 가능한 형태로 표현되었는가?)
2. 드라이버가 프로젝트의 전체 목표와 연관되는가?
3. Response Measure가 프로젝트 목표에서 명확하게 도출되었는가?
4. 기능 요구사항이 이해되었는가?
5. 요구사항에 우선순위가 부여되었는가?

**데이터 소스 / Data Sources** (이 문서에서 참조한 파일):

| 파일 | 도출 QA |
|------|--------|
| `docs/milestone1/m1-qa-redefine-performance.md` | Real-Time Performance |
| `docs/milestone1/quality_attribute_scenarios_latency.md` | Low Latency |
| `docs/milestone1/correctness-analysis.md` | Correctness |
| `docs/milestone1/quality_attribute_scenarios_usability.md` | Usability |
| `docs/milestone1/quality_attribute_scenarios_extensibility.md` | Extensibility |

**English**

This document answers the five mentor review questions in order:

1. Are QA requirements "actionable"? (Expressed so the team can verify a design supports them?)
2. Do the drivers relate to the overall objectives of the project?
3. Are the measures clearly derived from the overall goals?
4. Are the functional requirements understood?
5. Are requirements prioritized?

---

## 1. 프로젝트 목표 / Project Objectives

**한국어**

TimeGrapher는 기계식 시계의 탈진기 진동에서 발생하는 음향 신호(beat noise)를 실시간으로 분석하여 시계 성능을 진단하는 시스템이다. 마이크로 tick-tock 소리를 수집하고, A(T1)·C(T3) 이벤트를 감지하여 Rate, Amplitude, Beat Error를 계산해 GUI에 표시한다.

**팀의 핵심 목표 (우선순위 순):**

| 순위 | 목표 | 설명 |
|:---:|------|------|
| **1st** | **정확한 측정** | 일부 BPH 시계라도 Rate / Amplitude / Beat Error를 정확하게 측정하는 것이 최우선. 정확도를 희생하고 더 많은 BPH를 커버하는 선택은 하지 않는다 |
| **2nd** | **실시간 성능** | 모든 오디오 블록을 블록 주기 내에 처리하여 링 버퍼 오버플로 방지. 블록 드롭 시 T1/T3 타임스탬프 손실 → 측정 불가. 측정 정확도의 전제 조건 |
| **3rd** | **확장 가능한 구조** | 11개 그래프를 5주 내에 병렬 개발할 수 있는 구조 확보 |
| **4th** | **아키텍처 원칙 실증** | CMU MSE 소프트웨어 아키텍처 설계 원칙 적용 |

**English**

TimeGrapher analyzes acoustic signals (beat noise) from the escapement vibration of a mechanical watch to diagnose watch performance in real time. It captures tick-tock sounds via microphone, detects A(T1)·C(T3) events, and computes Rate, Amplitude, and Beat Error for GUI display.

**Team's core objectives (in priority order):**

| Rank | Objective | Description |
|:---:|-----------|-------------|
| **1st** | **Accurate Measurement** | Priority is to measure Rate / Amplitude / Beat Error accurately for at least some BPH range. Sacrificing accuracy to cover more BPH is not an acceptable trade-off |
| **2nd** | **Real-Time Performance** | Process every audio block within the block period to prevent Ring Buffer overflow. Dropped blocks destroy T1/T3 timestamps and make measurement impossible — a prerequisite for measurement accuracy |
| **3rd** | **Extensible Architecture** | Enable parallel development of 11 graphs within 5 weeks |
| **4th** | **Architecture Principles** | Apply CMU MSE software architecture design principles |

---

## 2. 기능 요구사항 / Functional Requirements

**한국어**

아키텍처를 이해하기 위해 파악된 기능 요구사항. 현재 샘플 코드(`TimeGrapher_v10.5`) 기준 구현 상태 포함.

| ID | 기능 요건 / Functional Requirement | 티어 | 우선순위 | 현재 구현 상태 |
|----|----------------------------------|:----:|:------:|:----------:|
| FR-01 | 시스템은 T1(A), T3(C) 음향 이벤트를 감지해야 한다 | — | HIGH | ✅ 구현됨 (`Detector.cpp`) |
| FR-02 | 시스템은 Rate (s/d), Amplitude (°), Beat Error (ms), BPH를 계산해야 한다 | — | HIGH | ✅ 구현됨 |
| FR-03 | 시스템은 Live / Playback / Sim 운영 모드를 지원해야 한다 | — | HIGH | ✅ 구현됨 |
| FR-04 | 시스템은 신호 필터링 (HPF + Envelope)을 수행해야 한다 | — | HIGH | ⚠️ 부분 구현 (HPF만) |
| FR-05 | 시스템은 Trace Display (Rate + Amplitude 실시간 기록)를 제공해야 한다 | **Core** | HIGH | ❌ 미구현 |
| FR-06 | 시스템은 Rate & Amplitude Stability / Vario (Min/Max/Avg/σ)를 제공해야 한다 | **Core** | HIGH | ❌ 미구현 |
| FR-07 | 시스템은 Beat Error Display & Diagnostic Trace를 제공해야 한다 | **Core** | HIGH | ❌ 미구현 |
| FR-08 | 시스템은 신호 품질 경고 (No signal / Noisy signal)를 표시해야 한다 | — | MEDIUM | ❌ 미구현 |
| FR-09 | 시스템은 Pause + 시간축 탐색 기능을 제공해야 한다 — 라이브 표시를 일시 정지하고 커서로 과거 beat 데이터를 앞뒤 탐색할 수 있어야 하며, 정지 중에도 백그라운드 수집은 계속 유지되어야 한다 | — | LOW | ❌ 미구현 |
| FR-10 | 시스템은 여러 자세(Dial-Up, Crown-Left 등)에서 포지션별 Rate 편차를 측정해야 한다 | **Required** | MEDIUM | ❌ 미구현 |
| FR-11 | 시스템은 beat 파형을 오실로스코프 형태로 표시해야 한다 (Scope 1: 원시 파형, Scope 2: 필터 후 파형) | **Required** | MEDIUM | ❌ 미구현 |
| FR-12 | 시스템은 복수 포지션 측정 결과를 순차적으로 비교 표시해야 한다 | **Required** | MEDIUM | ❌ 미구현 |
| FR-13 | 시스템은 수 시간~수 일에 걸친 Rate/Amplitude 장기 추이를 기록 및 표시해야 한다 | **Stretch** | LOW | ❌ 미구현 |
| FR-14 | 시스템은 탈진기 동작 분석 및 마커라인 오버레이를 표시해야 한다 | **Stretch** | LOW | ❌ 미구현 |
| FR-15 | 시스템은 beat 신호의 시간-주파수 스펙트로그램을 표시해야 한다 | **Stretch** | LOW | ❌ 미구현 |
| FR-16 | 시스템은 기준 파형 대비 현재 파형을 타이밍 마커와 함께 비교 표시해야 한다 | **Stretch** | LOW | ❌ 미구현 |
| FR-17 | 시스템은 트리거 동기화된 스윕 모드 스코프를 표시해야 한다 | **Stretch** | LOW | ❌ 미구현 |
| FR-18 | 시스템은 복수 필터 조합을 동시에 비교하는 스코프 뷰를 제공해야 한다 | **Stretch** | LOW | ❌ 미구현 |

**English**

| ID | Functional Requirement | Tier | Priority | Status |
|----|----------------------|:----:|:-------:|:------:|
| FR-01 | The system shall detect T1(A) and T3(C) acoustic events | — | HIGH | ✅ Implemented (`Detector.cpp`) |
| FR-02 | The system shall compute Rate (s/d), Amplitude (°), Beat Error (ms), and BPH | — | HIGH | ✅ Implemented |
| FR-03 | The system shall support Live / Playback / Sim operating modes | — | HIGH | ✅ Implemented |
| FR-04 | The system shall apply signal filtering (HPF + Envelope) | — | HIGH | ⚠️ Partial (HPF only) |
| FR-05 | The system shall provide Trace Display (real-time Rate + Amplitude recording) | **Core** | HIGH | ❌ Not implemented |
| FR-06 | The system shall provide Rate & Amplitude Stability / Vario (Min/Max/Avg/σ) | **Core** | HIGH | ❌ Not implemented |
| FR-07 | The system shall provide Beat Error Display & Diagnostic Trace | **Core** | HIGH | ❌ Not implemented |
| FR-08 | The system shall display signal quality warnings (No signal / Noisy signal) | — | MEDIUM | ❌ Not implemented |
| FR-09 | The system shall support Pause + timeline navigation — the user shall be able to freeze the live display and move backward/forward through captured beat data using a cursor; background data collection shall continue while paused | — | LOW | ❌ Not implemented |
| FR-10 | The system shall measure Rate deviation across multiple watch positions (Dial-Up, Crown-Left, etc.) | **Required** | MEDIUM | ❌ Not implemented |
| FR-11 | The system shall display the beat waveform in oscilloscope style (Scope 1: raw, Scope 2: filtered) | **Required** | MEDIUM | ❌ Not implemented |
| FR-12 | The system shall sequentially compare and display measurement results across multiple positions | **Required** | MEDIUM | ❌ Not implemented |
| FR-13 | The system shall record and display Rate/Amplitude trends over hours to days | **Stretch** | LOW | ❌ Not implemented |
| FR-14 | The system shall display escapement action analysis with marker-line overlay | **Stretch** | LOW | ❌ Not implemented |
| FR-15 | The system shall display a time-frequency spectrogram of the beat signal | **Stretch** | LOW | ❌ Not implemented |
| FR-16 | The system shall overlay reference and current waveforms with timing markers for comparison | **Stretch** | LOW | ❌ Not implemented |
| FR-17 | The system shall display a trigger-synchronized sweep mode scope | **Stretch** | LOW | ❌ Not implemented |
| FR-18 | The system shall provide a scope view for simultaneously comparing multiple filter combinations | **Stretch** | LOW | ❌ Not implemented |

---

## 3. 품질 속성 시나리오 / Quality Attribute Scenarios

### 3.1 우선순위 결정 기준 / Prioritization Criteria

**한국어**

팀 목표("정확한 데이터")와 "아키텍처 결정에 가장 큰 제약을 주는 순서"를 두 축으로 삼아 우선순위를 결정했다.

| 평가 축 / Axis | 설명 / Description |
|:-------------|:-----------------|
| **Business Importance** | 팀 목표("정확한 데이터")와의 직접적 연관성. 이것이 무너지면 프로젝트 목적 자체가 없어지는가? |
| **Technical Difficulty / Risk** | 달성 여부가 불확실하고, 실험으로만 검증 가능하며, 미달 시 시스템 전체 기능이 붕괴되는가? |

**English**

Priority is determined by two axes aligned with the team goal ("accurate data first") and architectural constraint impact.

---

### 3.2 우선순위 요약 / Priority Summary

**한국어**

| 순위 / Rank | QA | 핵심 요건 | Business Importance | Technical Difficulty / Risk | 한 줄 근거 / One-Line Rationale |
|:-----------:|----|---------| :------------------:| :-------------------------:|-------------------------------|
| **1** | Real-Time Performance | 시스템은 Ring Buffer 오버플로가 발생하지 않도록 각 오디오 블록을 블록 주기 이내에 처리해야 한다 | H | H | 나머지 모든 QA의 선행 조건. Dropped Block 발생 시 T1/T3 타임스탬프 자체가 소실되어 측정 불가 |
| **2** | Low Latency | 시스템은 오디오 캡처부터 GUI 표시까지 end-to-end 지연을 100ms 이내로 유지해야 한다 (28,800 BPH 기준) | H | H | Beat 주기라는 hard threshold 존재. 초과 시 실시간 표시 기능 붕괴. RPi + Qt 구조에서 달성 여부 미검증 |
| **3** | Correctness | 시스템은 동일한 beat 데이터를 기반으로 모든 GUI 뷰에서 동일한 수치를 표시해야 한다 (뷰 간 편차 = 0) | H | M | 팀 제1목표(정확한 데이터)와 직결. QA-C1은 Observer 패턴으로 구조적 보장, QA-C2(소음 환경 파라미터)만 미해결 |
| **4** | Usability | 시스템은 신호 없음(⚠ No signal) 및 노이즈 과다(⚠ Noisy signal) 상태를 구분하여 표시해야 한다 | M | M | 사용자가 잘못된 측정 상황을 즉시 인지하게 도와 정확한 데이터 수집을 간접 지원. 임계값 환경 의존적 |
| **5** | Extensibility | 시스템은 새로운 표시 기능을 구현할 때, 기존 전처리 로직 수정 없이 최소의 파일 수정으로 가능해야 한다 | M | M | 개발자 편의성이지만 11개 그래프 병렬 개발 일정 리스크를 직접 통제 |

**English**

| Rank | QA | Key Requirement | Business Importance | Technical Difficulty / Risk | One-Line Rationale |
|:----:|----|----------------| :------------------:| :-------------------------:|-------------------|
| **1** | Real-Time Performance | The system shall process each audio block within the block period to prevent Ring Buffer overflow | H | H | Prerequisite for all other QAs — Dropped Block destroys T1/T3 timestamps, making measurement impossible |
| **2** | Low Latency | The system shall maintain end-to-end latency from audio capture to GUI display within 100ms (based on 28,800 BPH) | H | H | Hard threshold defined by beat period; exceeding it causes functional failure; unverified on RPi + Qt |
| **3** | Correctness | The system shall display identical values across all GUI views derived from the same beat data (inter-view deviation = 0) | H | M | Directly tied to team's primary goal (accurate data); QA-C1 structurally guaranteed by Observer pattern; only QA-C2 (noise-condition parameters) remains open |
| **4** | Usability | The system shall distinguish and display ⚠ No signal and ⚠ Noisy signal states separately | M | M | Indirectly supports accurate data collection by alerting users to unreliable measurement conditions; threshold is environment-dependent |
| **5** | Extensibility | The system shall allow new display features to be implemented with minimal file changes and without modifying existing preprocessing logic | M | M | Developer productivity; directly controls schedule risk for 11-graph parallel implementation |

---

### QAS-1: 실시간 성능 / Real-Time Performance — Priority 1 (Business: H / Risk: H)

> ⚠️ **잠정값**: 목표 SPS 및 Dropped Block 달성 여부는 **EXP-01** 실측 전까지 확정 불가.

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (음향 신호 발생원) |
| **Stimulus** | USB 마이크를 통해 연속적인 음향 샘플 스트림 입력 |
| **Artifact** | 오디오 캡처 파이프라인 — ALSA 드라이버 → Audio Thread → Ring Buffer |
| **Environment** | Raspberry Pi 5 (8GB RAM), Ubuntu 24.04, Live 모드 실행 중, 28,800 BPH 기계식 시계, 연속 10분 이상, Qt GUI + DSP 동시 실행<br>• **Minimum**: 48,000 sps (이 이하면 프로젝트 실패)<br>• **Objective**: 96,000 sps *(잠정, EXP-01 확정)*<br>• **Stretch**: 192,000 sps |
| **Response** | 각 오디오 블록을 블록 주기 이내에 처리하여 Ring Buffer 오버플로 방지 |
| **Response Measure** | Dropped audio block: **0** (Ring Buffer 오버플로 없음) |

> SPS가 높을수록 블록 주기가 짧아져 DSP에 허용되는 시간이 줄어든다. SPS는 Dropped Block의 **원인**(환경 조건)이고, Ring Buffer 오버플로 여부가 **측정 지점**이다.

**SPS ↔ 블록 주기 관계** (SPS는 환경 조건):

| SPS (환경 조건) | 블록 주기 | T1 감지 분해능 | 비고 |
|:-------------:|:-------:|:----------:|-----|
| 48,000 | ~20 ms | 20.8 µs/sample | Minimum — 96k 달성 불가 시 폴백 |
| **96,000** | **~10 ms** | **10.4 µs/sample** | **Objective** |
| 192,000 | ~5 ms | 5.2 µs/sample | Stretch — RPi 성능 미확인 |

**actionability 평가:**

| 기준 | 결과 |
|------|------|
| 측정 가능한 지표인가? | ✅ Dropped Block 수 — 직접 측정 가능 |
| pass/fail 기준이 명확한가? | ✅ Dropped Block = 0이면 PASS |
| 실험으로 검증 가능한가? | ✅ EXP-01: 48k/96k/192k sps × 10분 연속 실행 |
| 아키텍처 결정을 유발하는가? | ✅ Lock-Free Ring Buffer, Priority Scheduling, Graceful Degradation |

**English**

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (acoustic signal source) |
| **Stimulus** | Continuous audio sample stream via USB microphone |
| **Artifact** | Audio capture pipeline — ALSA driver → Audio Thread → Ring Buffer |
| **Environment** | Raspberry Pi 5 (8GB RAM), Ubuntu 24.04, Live mode, 28,800 BPH mechanical watch, continuous ≥ 10 min, Qt GUI + DSP running concurrently<br>• **Minimum**: 48,000 sps (below this = project failure)<br>• **Objective**: 96,000 sps *(provisional, confirmed by EXP-01)*<br>• **Stretch**: 192,000 sps |
| **Response** | Process each audio block within the block period to prevent Ring Buffer overflow |
| **Response Measure** | Dropped audio block: **0** (no Ring Buffer overflow) |

**프로젝트 목표와의 연결:** Dropped Block 발생 → T1/T3 타임스탬프 소실 → Rate / Amplitude / Beat Error 계산 불가 → 팀 제1목표 완전 붕괴. **모든 QA의 선행 조건.**

**관련 아키텍처 전술:**

| 전술 / Tactic | 적용 이유 |
|-------------|---------|
| Lock-Free Ring Buffer | Audio Thread ↔ DSP Thread 간 mutex 제거 → 락 경합으로 인한 DSP 지연 방지 |
| Priority Scheduling | 오디오 처리 스레드 우선순위 상향 → Linux 스케줄러 지터 흡수 |
| Graceful Degradation | 96k sps 달성 불가 시 48k sps 자동 폴백 → Dropped Block = 0 보장 |

---

### QAS-2: 낮은 지연 / Low Latency — Priority 2 (Business: H / Risk: H)

> ⚠️ **잠정값**: 아래 수치는 **EXP-02** 실측 전까지 확정 불가.

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (beat 이벤트 발생원) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처 시작 |
| **Artifact** | TimeGrapher 전체 — 오디오 캡처 → DSP 파이프라인 → Qt GUI 렌더링 |
| **Environment** | Raspberry Pi 5 (8GB RAM), 정상 운영 상태 — Qt GUI 실행 중, 라이브 캡처 중 |
| **Response** | 캡처된 샘플에 대응하는 파형, beat 마커, Rate·Amplitude·Beat Error가 GUI에 표시됨 |
| **Response Measure** | 아래 BPH별 목표 참조 |

**BPH별 지연 목표:**

> 핵심 제약: end-to-end 지연 > beat 주기 시 실시간 표시 기능 붕괴. 80% 안전 마진 적용 (OS 스케줄러 지터, Qt 렌더링 변동 흡수).

| BPH | Beat 주기 | end-to-end 목표 (80%) | ① capture→process (70%) | ② process→display (30%) |
|-----|:-------:|:-------------------:|:----------------------:|:----------------------:|
| **28,800** | **125 ms** | **< 100 ms** *(1차 목표)* | **< 70 ms** | **< 30 ms** |
| 36,000 | 100 ms | 80 ms | 56 ms | 24 ms |
| 43,200 | 83 ms | 66 ms | 46 ms | 20 ms |

> **1차 목표 (Option A)**: 28,800 BPH 기준 end-to-end < 100 ms (보유 시계 3종 중 기준)
> **Stretch (Option B)**: EXP-02 결과 후 36,000 / 43,200 BPH까지 상향 검토

**구간 분리 근거:**

| 구간 | 시간 유형 | 측정 경계 | 병목 원인 |
|------|---------|---------|---------|
| ① capture→process | **Wait** (OS 콜백 주기 ~20ms) + **Execute** (DSP 처리) | ALSA 콜백 수신 → T1/T3 이벤트 타임스탬프 | OS 콜백 주기(~20ms), Ring Buffer 대기, DSP 처리 시간 |
| ② process→display | **Execute** (Qt 렌더링) | T1/T3 이벤트 타임스탬프 → GUI `paintEvent()` 완료 | Qt 렌더링 시간, FPS, 렌더링 스레드 경합 |
| ③ end-to-end | Wait + Execute 합산 | ALSA 콜백 수신 → GUI 화면 갱신 완료 | ①+② 합산 |

**actionability 평가:**

| 기준 | 결과 |
|------|------|
| 측정 가능한 지표인가? | ✅ 타임스탬프 3점(TS1/TS2/TS3) 삽입으로 구간별 실측 |
| pass/fail 기준이 명확한가? | ✅ end-to-end < 100 ms (28,800 BPH 기준) |
| 실험으로 검증 가능한가? | ✅ EXP-02: 3구간 × SPS 3단계 × 탭 구성 2가지 × 10분 |
| 아키텍처 결정을 유발하는가? | ✅ Thread 분리, Lock-Free Ring Buffer, Lazy Rendering |

**English**

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (beat event source) |
| **Stimulus** | Audio sample block capture begins from microphone |
| **Artifact** | Full TimeGrapher application — audio capture → DSP pipeline → Qt GUI rendering |
| **Environment** | Raspberry Pi 5 (8GB RAM), normal operation — Qt GUI active, live capture running |
| **Response** | Waveform, beat markers, and computed Rate·Amplitude·Beat Error displayed in GUI |
| **Response Measure** | See BPH-based latency targets above |

**프로젝트 목표와의 연결:** end-to-end 지연이 beat 주기를 초과하면 실시간 표시 불가 → 팀 제1목표(정확한 데이터)를 위한 실시간 피드백 루프 붕괴. 지연 누적은 T1/T3 타임스탬프 정확도에도 간접 영향.

**관련 아키텍처 전술:**

| ID | 전술 / Tactic | 적용 이유 |
|----|-------------|---------|
| LT-01 | Introduce Concurrency (Audio/DSP/GUI 스레드 분리) | 캡처 콜백이 DSP/GUI에 의해 블로킹되지 않도록 |
| LT-02 | Lock-Free Ring Buffer | 스레드 간 mutex 제거 → ① 구간 지연 급증 방지 |
| LT-03 | Reduce Computational Overhead | 실시간 경로의 불필요한 연산 제거 |
| LT-04 | Lazy Rendering (현재 탭만 업데이트) | Qt 메인 스레드 렌더링 부하 감소 → ② 구간 지연 감소 |

---

### QAS-3: 정확성 / Correctness — Priority 3 (Business: H / Risk: M)

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 사용자 (동일 시계, 동일 조건) / 소음 환경 |
| **Stimulus** | 실시간 음향 신호 수신 중 다수의 GUI 뷰 동시 표시 / 소음이 있는 환경에서 beat 감지 |
| **Artifact** | Rate·Amplitude·Beat Error 계산 로직 및 GUI 전체 뷰 / DSP 파이프라인 |
| **Environment** | 정상 운영 상태 / ambient noise가 존재하는 환경 |
| **Response** | 모든 GUI 뷰에서 동일 데이터 기반 일관된 수치 표시 + 소음 환경에서도 beat 감지 품질 유지 |
| **Response Measure** | • **QA-C1**: 동일 beat 데이터에서 파생된 모든 뷰의 수치 편차: **0** — Observer 패턴으로 구조적 보장<br>• **QA-C2**: 소음 3조건에서 Δ Rate / Δ Amplitude / Δ Beat Error를 최소화하는 Detector 파라미터(`onset_fraction`, `min_peak_fraction`) 값 — **EXP-03 결과 후 확정** |

**actionability 평가:**

| 기준 | QA-C1 | QA-C2 |
|------|:-----:|:-----:|
| 측정 가능한 지표인가? | ✅ 뷰 간 값 비교 (편차 = 0) | ✅ Δ Rate / Δ Amplitude / Δ Beat Error |
| pass/fail 기준이 명확한가? | ✅ Observer 패턴 적용 시 자동 PASS | ⚠️ 허용 Δ 수치는 EXP-03 후 확정 |
| 실험으로 검증 가능한가? | ✅ 구조적 보장 (실험 불필요) | ✅ EXP-03: 소음 3조건 × Detector 파라미터 조합 |
| 아키텍처 결정을 유발하는가? | ✅ Observer / Qt Signal-Slot | ✅ Adaptive threshold, 파라미터 튜닝 |

**English**

| Item | Detail |
|------|--------|
| **Source** | User (same watch, same conditions) / Noisy environment |
| **Stimulus** | Multiple GUI views displayed simultaneously / beat detection under ambient noise |
| **Artifact** | Rate·Amplitude·Beat Error computation logic and all GUI views / DSP pipeline |
| **Environment** | Normal operating state / environment with ambient noise |
| **Response** | All GUI views display consistent values from same data + beat detection quality maintained under noise |
| **Response Measure** | • **QA-C1**: Value deviation across all views derived from same beat data: **0** — structurally guaranteed by Observer pattern<br>• **QA-C2**: Optimal Detector parameters (`onset_fraction`, `min_peak_fraction`) minimizing Δ across 3 noise conditions — **finalized after EXP-03** |

**프로젝트 목표와의 연결:** 팀 제1목표(정확한 데이터)를 두 측면에서 지원. QA-C1은 "어느 뷰를 보더라도 동일한 정확한 값"을 보장. QA-C2는 소음 환경에서도 beat 감지 품질을 유지하여 정확도를 구조적으로 확보.

**관련 아키텍처 패턴:**

| 패턴 / Pattern | 연결 QA | 적용 이유 |
|--------------|:------:|---------|
| Observer / Qt Signal-Slot (GoF) | QA-C1 | MeasurementEngine이 단일 Measurement 구조체 발행 → 모든 탭이 동일 신호 구독 |
| Pipes and Filters (POSA) | QA-C2 | Raw PCM → HPF → Envelope → Detector 단방향 파이프라인으로 beat 감지 품질 유지 |

---

### QAS-4: 사용성 / Usability — Priority 4 (Business: M / Risk: M)

> ⚠️ **잠정값**: N·M 수치 및 임계값은 실험으로 확정 필요.

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 마이크 입력 신호 |
| **Stimulus** | 마이크 입력 신호가 정상 범위를 벗어남 — 신호 없음 또는 노이즈 과다 |
| **Artifact** | GUI 상태 표시 영역 |
| **Environment** | Live 모드, 실시간 정상 및 비정상 운영 상태 |
| **Response** | GUI에 신호 품질 경고 메시지 즉시 표시, 정상 복귀 시 자동 해제 |
| **Response Measure** | • 비정상 신호 발생 후 **≤ N초** 이내 경고 표시 *(실험으로 확정)*<br>• 정상 복귀 후 **≤ M초** 이내 경고 해제 *(실험으로 확정)*<br>• 경고 조건: `⚠ No signal` (N초 동안 beat 이벤트 없음) / `⚠ Noisy signal` (노이즈/신호 비율 임계값 초과) |

**actionability 평가:**

| 기준 | 결과 |
|------|------|
| 측정 가능한 지표인가? | ✅ 경고 표시까지 걸린 시간 (초 단위) |
| pass/fail 기준이 명확한가? | ⚠️ N, M 수치 TBD — 실험 후 확정 |
| 실험으로 검증 가능한가? | ✅ 시계 제거/복원 실험으로 N·M 측정, 다환경 임계값 탐색 |
| 아키텍처 결정을 유발하는가? | ✅ Heartbeat 패턴, 경고 상태 관리 로직 |

**English**

| Item | Detail |
|------|--------|
| **Source** | Microphone input signal |
| **Stimulus** | Microphone input outside normal range — no signal or excessive noise |
| **Artifact** | GUI status display area |
| **Environment** | Live mode, normal and abnormal operating states |
| **Response** | Immediate signal quality warning in GUI; auto-cleared on signal recovery |
| **Response Measure** | • Warning displayed within **≤ N seconds** *(confirmed by experiment)*<br>• Warning cleared within **≤ M seconds** *(confirmed by experiment)*<br>• Conditions: `⚠ No signal` / `⚠ Noisy signal` |

**프로젝트 목표와의 연결:** 사용자가 측정값이 신뢰할 수 없는 상황을 즉시 인지 → 시계 위치 조정 등 조치 → 정확한 데이터 수집 가능. 팀 제1목표를 간접 지원.

**관련 아키텍처 전술:**

| 전술 / Tactic | 적용 이유 |
|-------------|---------|
| Heartbeat | 기존 A/C 이벤트를 heartbeat로 재사용 → N초 동안 이벤트 없으면 신호 소실 판단, 별도 감지 로직 불필요 |

---

### QAS-5: 확장성 / Extensibility — Priority 5 (Business: M / Risk: M)

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 개발자 (새 그래프 또는 표시 기능 추가) |
| **Stimulus** | 기존 코드베이스에 새로운 그래프 탭 추가 요청 (예: Spectrogram, Long-Term, Sequence) |
| **Artifact** | 소스코드 (GUI 모듈 구조 전체) |
| **Environment** | 개발 환경 (Qt Creator, Windows PC / RPi), 5주 프로젝트 기간 내 |
| **Response** | 기존 전처리 모듈 수정 없이 새 그래프 위젯을 독립적으로 생성·등록·테스트 가능 |
| **Response Measure** | • 새 그래프 1개 추가 시 변경되는 파일 수: **≤ 3개** *(Observer 패턴 리팩터링 완료 후 실측 확정)*<br>• 추가된 그래프의 의존성: Presentation → Domain 방향 준수 (Signal Processing / Acquisition 직접 참조 = **0**) |

**"≤ 3개" 근거:**

| 변경 유형 | 대상 | 파일 수 |
|---------|------|:------:|
| 새 그래프 위젯 파일 신규 생성 | `GraphFoo.cpp` / `GraphFoo.h` | 1 |
| 탭 컨테이너에 새 탭 등록 | `TabPanel.cpp` | 1 |
| 데이터 구독 연결 | `DataBroker.cpp` | 1 |
| **합계** | | **3** |

**actionability 평가:**

| 기준 | 결과 |
|------|------|
| 측정 가능한 지표인가? | ✅ `git diff --stat`으로 변경 파일 수 직접 측정 |
| pass/fail 기준이 명확한가? | ✅ ≤ 3개이면 PASS, 4개 이상이면 FAIL (모듈화 불충분 신호) |
| 실험으로 검증 가능한가? | ✅ Observer 패턴 리팩터링 완료 후 신규 그래프 추가 실측 |
| 아키텍처 결정을 유발하는가? | ✅ Split Module, Observer/Signal-Slot, Restrict Dependencies |

**English**

| Item | Detail |
|------|--------|
| **Source** | Developer (adding new graph or display feature) |
| **Stimulus** | Request to add new graph tab (e.g., Spectrogram, Long-Term, Sequence) |
| **Artifact** | Source code (full GUI module structure) |
| **Environment** | Development environment (Qt Creator, Windows PC / RPi), within 5-week project timeline |
| **Response** | New graph widget can be created, registered, and tested independently without modifying preprocessing modules |
| **Response Measure** | • Files changed when adding 1 new graph: **≤ 3** *(confirmed after Observer pattern refactoring)*<br>• Dependencies follow Presentation → Domain direction (direct Signal Processing / Acquisition references = **0**) |

**프로젝트 목표와의 연결:** 11개 그래프를 Week 3~4에 구현해야 하는 일정 리스크를 직접 통제. 각 개발자가 담당 그래프를 독립적으로 구현·테스트 가능 → 팀 개발 속도 유지.

**관련 아키텍처 전술:**

| 전술 / Tactic | 적용 이유 |
|-------------|---------|
| Split Module | God Object → 기능별 모듈 분리 (전처리·표시·수집) |
| Observer / Signal-Slot | 새 그래프 추가 시 구독만 추가, 기존 로직 수정 불필요 |
| Restrict Dependencies (Layered Architecture) | Presentation Layer는 Domain Layer만 참조 → 표시 레이어 독립 교체 가능 |

---

## 4. QA–아키텍처 전술 종합 매핑 / QA–Architecture Tactic Mapping

**한국어**

| QA | 우선순위 | 핵심 전술 | 미해결 실험 |
|----|:------:|---------|----------|
| Real-Time Performance | 1 | Lock-Free Ring Buffer, Priority Scheduling, Graceful Degradation | EXP-01 (SPS별 Dropped Block) |
| Low Latency | 2 | Thread 분리 (Concurrency), Lock-Free Ring Buffer, Lazy Rendering | EXP-02 (3구간 타임스탬프 측정) |
| Correctness | 3 | Observer/Signal-Slot GoF (QA-C1), Pipes and Filters POSA (QA-C2) | EXP-03 (소음 조건 × 파라미터 Δ) |
| Usability | 4 | Heartbeat 패턴 | 임계값 실험 (N·M 수치 확정) |
| Extensibility | 5 | Split Module, Observer/Signal-Slot, Layered Architecture | ≤ 3파일 검증 실험 |

**English**

| QA | Priority | Core Tactic(s) | Open Experiment |
|----|:-------:|---------------|----------------|
| Real-Time Performance | 1 | Lock-Free Ring Buffer, Priority Scheduling, Graceful Degradation | EXP-01 (Dropped Block per SPS) |
| Low Latency | 2 | Thread separation (Concurrency), Lock-Free Ring Buffer, Lazy Rendering | EXP-02 (3-segment timestamp measurement) |
| Correctness | 3 | Observer/Signal-Slot GoF (QA-C1), Pipes and Filters POSA (QA-C2) | EXP-03 (noise × parameter Δ) |
| Usability | 4 | Heartbeat pattern | Threshold experiment (N·M values) |
| Extensibility | 5 | Split Module, Observer/Signal-Slot, Layered Architecture | ≤ 3-file verification experiment |

---

## 5. 설계 제약사항 / Design Constraints

**한국어**

Design Constraint = 이미 내려진 설계 결정. 아키텍처 논의 대상이 아니며 변경 불가.

| 제약 | 내용 | 변경 불가 이유 |
|------|------|-------------|
| 타겟 하드웨어 | Raspberry Pi 5 (ARM64, 16GB RAM) + USB 오디오 센서 + 8" 터치스크린 | 프로그램에서 하드웨어 지급 |
| 개발 환경 | Qt Creator (C++/Qt6) | 기존 코드베이스 (`TimeGrapher_v10.5`) 기반 |

**English**

| Constraint | Detail | Rationale |
|------------|--------|-----------|
| Target Hardware | Raspberry Pi 5 (ARM64, 16GB RAM) + USB audio sensor + 8" touchscreen | Hardware provided by program |
| Development Environment | Qt Creator (C++/Qt6) | Based on existing codebase (`TimeGrapher_v10.5`) |

---

## 6. 미결 사항 요약 / Open Issues Summary

**한국어**

| ID | 질문 | 해결 방법 | 영향 QA |
|----|------|---------|--------|
| OI-P1 | RPi 5에서 96k sps Dropped Block = 0 달성 가능한가? | EXP-01 | QAS-1 |
| OI-L1 | QAudioSource 라이브 캡처의 실제 콜백 주기는? | EXP-02 실측 | QAS-2 |
| OI-L2 | 11탭 동시 렌더링 시 ② process→display < 30ms인가? | EXP-02 실측 | QAS-2 |
| OI-L3 | 28,800 BPH 달성 후 36,000 / 43,200 BPH까지 상향 가능한가? | EXP-02 결과 후 팀 합의 | QAS-2 |
| OI-C1 | 소음 3조건에서 Δ를 최소화하는 Detector 파라미터 최적값은? | EXP-03 | QAS-3 |
| OI-U1 | 경고 발생·해제 N·M 수치는? | 시계 제거/복원 실험 | QAS-4 |
| OI-U2 | No signal / Noisy signal 임계값은? | 다환경 임계값 탐색 실험 | QAS-4 |

**English**

| ID | Open Question | Resolution | Affected QA |
|----|--------------|-----------|------------|
| OI-P1 | Can RPi 5 achieve Dropped Block = 0 at 96k sps? | EXP-01 | QAS-1 |
| OI-L1 | What is the actual QAudioSource live callback period? | EXP-02 measurement | QAS-2 |
| OI-L2 | Is ② process→display < 30ms with 11 tabs rendering? | EXP-02 measurement | QAS-2 |
| OI-L3 | After achieving 28,800 BPH target, can the target be raised to 36,000 / 43,200 BPH? | EXP-02 result → team decision | QAS-2 |
| OI-C1 | What Detector parameter values minimize Δ across 3 noise conditions? | EXP-03 | QAS-3 |
| OI-U1 | What are the N and M second values for warning onset/clear? | Watch removal/restore experiment | QAS-4 |
| OI-U2 | What are the thresholds for No signal / Noisy signal? | Multi-environment threshold search | QAS-4 |
