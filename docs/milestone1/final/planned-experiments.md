# 계획된 기술 실험 / Planned Technical Experiments — TimeGrapher

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **작성일 / Date**: 2026-06-07

---

## 0. 이 문서의 구조 / Document Structure

**한국어**

이 문서는 아래 세 가지 질문에 순서대로 답한다.

1. 기술 실험이 구체적으로 정의되어 있고 템플릿을 따르는가?
2. 각 실험이 해결하려는 질문/이슈가 명확한가?
3. 실험 완료 시점을 명확히 알 수 있는가?

**데이터 소스**: 모든 실험은 아래 QA 분석 및 아키텍처 문서에서 직접 도출했다.

| 파일 | 도출 실험 |
|------|---------|
| `docs/milestone1/m1-qa-redefine-performance.md` | EXP-01 |
| `docs/milestone1/quality_attribute_scenarios_latency.md` | EXP-02 |
| `docs/milestone1/correctness-analysis.md` | EXP-03 |
| `docs/milestone1/quality_attribute_scenarios_usability.md` | EXP-04 |
| `docs/milestone1/final/architectural-drivers.md` | 전체 실험 — 미결 이슈(OI-*) 및 QAS 연결 |
| `docs/milestone1/final/risk-assessment.md` | 전체 실험 — 리스크(TR-*) 및 액션 연결 |

**English**

This document answers three questions in order:

1. Are the technical experiments articulated concretely and following a template?
2. Is it clear what question/issue is being addressed by each experiment?
3. Will it be clear when the experiments are complete?

**Data sources**: All experiments are derived directly from the QA analyses and architecture documents listed above.

---

## 1. 실험 목록 요약 / Experiment Overview

**한국어**

| ID | 실험명 / Name | 연결 QA | 연결 리스크 | 해소 OI | 수행 시점 (참고) |
|----|-------------|:-------:|:---------:|:-------:|:-------------|
| **EXP-01** | RPi 실시간 성능 — Dropped Block 측정 | QAS-1 | TR-01, TR-02 | OI-P1 | Sprint 1 (Week 1) |
| **EXP-02** | end-to-end 지연 3구간 타임스탬프 측정 | QAS-2 | TR-03, TR-04 | OI-L1, OI-L2 | Sprint 2~3 (Week 1~2) |
| **EXP-03** | Detector 파라미터 소음 조건 최적화 | QAS-3 | TR-05 | OI-C1 | Sprint 3~4 (Week 2) |
| **EXP-04** | 신호 품질 경고 임계값 탐색 | QAS-4 | TR-09 | OI-U1, OI-U2 | Sprint 3~4 (Week 2) |
| **EXP-05** | BPH 상향 검증 — 36k/43k BPH 지연 측정 | QAS-2 (Stretch) | — | OI-L3 | 28,800 BPH 기준 QA 전부 충족 후 (Week 3 이후) |

> **참고**: 수행 시점은 프로젝트 플랜 초안 기준이며 확정 일정이 아니다. 선행 조건이 있는 실험(EXP-02, EXP-03, EXP-04)은 EXP-01 결과 및 Observer 패턴 리팩터링 완료 후 진행한다. **EXP-05**는 EXP-02에서 28,800 BPH 기준 QA가 전부 충족된 것이 확인된 이후에만 진행한다.

> **Observer 패턴 리팩터링 ≤ 3파일 검증**은 구현 체크리스트에서 관리하며 별도 실험으로 분류하지 않는다.

**English**

| ID | Experiment Name | Linked QA | Linked Risk | Resolved OI | Timing (Reference) |
|----|----------------|:---------:|:-----------:|:-----------:|:------------------|
| **EXP-01** | RPi Real-Time Performance — Dropped Block Measurement | QAS-1 | TR-01, TR-02 | OI-P1 | Sprint 1 (Week 1) |
| **EXP-02** | End-to-End Latency — 3-Segment Timestamp Measurement | QAS-2 | TR-03, TR-04 | OI-L1, OI-L2 | Sprint 2–3 (Week 1–2) |
| **EXP-03** | Detector Parameter Optimization Under Noise Conditions | QAS-3 | TR-05 | OI-C1 | Sprint 3–4 (Week 2) |
| **EXP-04** | Signal Quality Warning Threshold Search | QAS-4 | TR-09 | OI-U1, OI-U2 | Sprint 3–4 (Week 2) |
| **EXP-05** | BPH Escalation Verification — 36k/43k BPH Latency Measurement | QAS-2 (Stretch) | — | OI-L3 | After all 28,800 BPH QA targets confirmed (Week 3+) |

> **Note**: Timing references the draft project plan and is not final. Experiments with prerequisites (EXP-02, EXP-03, EXP-04) proceed after EXP-01 results are confirmed and Observer pattern refactoring is complete. **EXP-05** is conducted only after EXP-02 confirms that all 28,800 BPH QA targets are met.

> **Observer pattern refactoring ≤ 3-file verification** is managed as an implementation checklist item and is not classified as a separate experiment.

---

## Experiment EXP-01: RPi 실시간 성능 — Dropped Block 측정 / RPi Real-Time Performance — Dropped Block Measurement

---

### Results and recommendations / 결과 및 권고사항

**한국어**

(미완료 — 실험 수행 후 기록 예정)

**English**

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective / 목적

**한국어**

Raspberry Pi 5 환경에서 Qt GUI와 DSP 파이프라인을 동시 실행할 때 48,000 / 96,000 / 192,000 sps 각 샘플링 레이트 조건에서 Dropped Block(Ring Buffer 오버플로)이 발생하지 않는지 검증한다.

**실험이 답해야 할 기술 질문**: "RPi 5에서 Qt GUI + DSP 동시 실행 조건으로 96,000 sps 처리 시 Dropped Block = 0을 달성할 수 있는가? 불가능하다면 어느 sps까지 안정적으로 처리 가능한가?"

이 실험의 결과는 다음 설계 결정에 직접 사용된다:
- QAS-1 Response Measure 확정 (현재 Objective: 96,000 sps는 잠정값)
- Graceful Degradation 폴백 기준 확정 (48k sps로 폴백할지 여부)
- EXP-02, EXP-03의 선행 조건 — sps 방향 결정 후 지연 측정 및 파라미터 튜닝이 의미를 갖는다

**English**

Verify that zero Dropped Blocks (Ring Buffer overflow) occur on Raspberry Pi 5 when running Qt GUI + DSP pipeline concurrently at 48,000 / 96,000 / 192,000 sps.

**Technical question**: "Can RPi 5 achieve Dropped Block = 0 at 96,000 sps while running Qt GUI + DSP concurrently? If not, what is the maximum sps that can be processed stably?"

The results of this experiment directly inform:
- Finalizing QAS-1 Response Measure (current Objective: 96,000 sps is provisional)
- Confirming the Graceful Degradation fallback threshold (whether 48k sps fallback is triggered)
- Prerequisites for EXP-02 and EXP-03 — latency measurement and parameter tuning are only meaningful after SPS direction is confirmed

---

### Status / 상태

**Planned**

---

### Expected outcomes / 예상 산출물

**한국어**

- 48,000 / 96,000 / 192,000 sps 각 조건에서 10분 연속 실행 시 Dropped Block 수 측정값 표
- 달성 가능한 최대 sps 결론 및 QAS-1 Response Measure 확정값
- Graceful Degradation 폴백 기준 결정 (96k 달성 가능 여부 → 48k 폴백 여부)
- Priority Scheduling(`SCHED_RR` / `SCHED_FIFO`) 적용 전·후 비교 결과

**English**

- Table of Dropped Block counts for each condition (48k / 96k / 192k sps, 10-minute continuous run)
- Conclusion on maximum achievable sps and finalized QAS-1 Response Measure
- Graceful Degradation fallback decision (whether 96k is achievable → whether 48k fallback is needed)
- Before/after comparison of Priority Scheduling (`SCHED_RR` / `SCHED_FIFO`) application

---

### Resources required / 필요 자원

**한국어**

| 자원 / Resource | 상세 / Detail |
|----------------|-------------|
| 하드웨어 | Raspberry Pi 5 (8GB RAM), USB 오디오 센서(마이크), 8" 터치스크린 |
| 소프트웨어 | Ubuntu 24.04, Qt6, ALSA, `TimeGrapher_v10.5` 코드베이스 |
| 기계식 시계 | 28,800 BPH 시계 1종 (라이브 캡처 음원) |
| 계측 도구 | Ring Buffer 오버플로 카운터 코드 삽입, `chrono` 타임스탬프, Linux `perf` |
| 노력 | 약 1~2 person-days (환경 세팅 + 측정 + 결과 분석) |

**English**

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5 (8GB RAM), USB audio sensor (microphone), 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, ALSA, `TimeGrapher_v10.5` codebase |
| Mechanical watch | One 28,800 BPH watch (live capture audio source) |
| Measurement tools | Ring Buffer overflow counter injection, `chrono` timestamps, Linux `perf` |
| Effort | ~1–2 person-days (setup + measurement + analysis) |

---

### Experiment description / 실험 방법

**한국어**

1. **환경 준비**: RPi 5에 Ubuntu 24.04 + Qt6 빌드 환경 구성. 28,800 BPH 시계를 USB 마이크 앞에 배치.
2. **Ring Buffer 오버플로 카운터 삽입**: `AudioCapture` 모듈에 `dropped_block_count` 변수를 추가하여 오버플로 발생 시 증가.
3. **sps 조건 설정**: `QAudioFormat::setSampleRate()`로 48,000 / 96,000 / 192,000 sps 순서로 설정.
4. **10분 연속 실행**: 각 sps 조건에서 Qt GUI + DSP(필터링 + T1/T3 감지 + Rate·Amplitude·Beat Error 계산) 동시 실행 상태로 10분 유지.
5. **Priority Scheduling 비교**: `SCHED_RR` 적용 전·후 동일 조건 반복하여 스케줄러 지터 영향 측정(TR-02 검증).
6. **결과 기록**: sps별 Dropped Block 수를 표로 정리. 96k 달성 가능 여부 → QAS-1 Response Measure 확정값 기록.
7. **폴백 결정**: 96k 달성 불가 시 Graceful Degradation 폴백(48k sps 자동 전환) 구현 여부를 Architecture Committee에 보고.

**English**

1. **Environment setup**: Configure Ubuntu 24.04 + Qt6 build environment on RPi 5. Position 28,800 BPH watch in front of USB microphone.
2. **Inject Ring Buffer overflow counter**: Add `dropped_block_count` variable to `AudioCapture` module; increment on overflow event.
3. **Configure sps conditions**: Use `QAudioFormat::setSampleRate()` to set 48,000 / 96,000 / 192,000 sps in sequence.
4. **10-minute continuous run**: For each sps condition, maintain Qt GUI + DSP (filtering + T1/T3 detection + Rate·Amplitude·Beat Error computation) running concurrently for 10 minutes.
5. **Priority Scheduling comparison**: Repeat same conditions before and after applying `SCHED_RR` to measure Linux scheduler jitter effect (verifies TR-02).
6. **Record results**: Tabulate Dropped Block counts per sps. Confirm whether 96k is achievable → record finalized QAS-1 Response Measure.
7. **Fallback decision**: If 96k is not achievable, report Graceful Degradation fallback (automatic switch to 48k sps) decision to Architecture Committee.

---

### Duration / 기간

**한국어**

- **목표 완료**: Sprint 1 이내 (2026-06-09 기준)
- **선행 조건**: RPi 5 하드웨어 세팅 + 기계식 시계 연결 완료
- **이 실험 완료 없이 진행 불가한 작업**: EXP-02 (지연 측정), EXP-03 (파라미터 튜닝) — sps 방향 결정 전까지 두 실험의 측정 조건 미확정

**English**

- **Target completion**: Within Sprint 1 (reference: 2026-06-09)
- **Prerequisites**: RPi 5 hardware setup + mechanical watch connected
- **Work blocked until this completes**: EXP-02 (latency measurement), EXP-03 (parameter tuning) — measurement conditions for both are undefined until SPS direction is decided

---

### Links and references / 참고 자료

**한국어**

- `docs/milestone1/final/architectural-drivers.md` — QAS-1, OI-P1
- `docs/milestone1/final/risk-assessment.md` — TR-01, TR-02, 액션 계획 순서 1
- `docs/milestone1/m1-qa-redefine-performance.md` — 원본 QA 분석
- Linux ALSA documentation — `snd_pcm_hw_params_set_rate`
- Qt6 QAudioSource API — callback period behavior

**English**

- `docs/milestone1/final/architectural-drivers.md` — QAS-1, OI-P1
- `docs/milestone1/final/risk-assessment.md` — TR-01, TR-02, Action Plan step 1
- `docs/milestone1/m1-qa-redefine-performance.md` — source QA analysis
- Linux ALSA documentation — `snd_pcm_hw_params_set_rate`
- Qt6 QAudioSource API — callback period behavior

---

## Experiment EXP-02: end-to-end 지연 3구간 타임스탬프 측정 / End-to-End Latency — 3-Segment Timestamp Measurement

---

### Results and recommendations / 결과 및 권고사항

**한국어**

(미완료 — 실험 수행 후 기록 예정)

**English**

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective / 목적

**한국어**

TimeGrapher 전체 파이프라인(오디오 캡처 → DSP → Qt GUI 렌더링)의 end-to-end 지연을 3개 구간으로 분리하여 실측한다. 각 구간의 병목을 파악하고, 28,800 BPH 기준 end-to-end < 100 ms 달성 가능 여부를 검증한다.

**실험이 답해야 할 기술 질문들**:
- "QAudioSource 라이브 캡처 콜백의 실제 주기는 얼마인가? (OI-L1)"
- "11개 탭이 동시 렌더링될 때 ② process→display 구간이 30 ms를 초과하는가? (OI-L2)"
- "28,800 BPH 달성 후 36,000 / 43,200 BPH까지 목표를 상향할 수 있는가? (OI-L3 — 데이터 수집, 확정은 EXP-05)"

이 실험의 결과는 다음 설계 결정에 직접 사용된다:
- QAS-2 Response Measure 확정 (BPH별 지연 목표 최종 확정)
- Lazy Rendering 전술 필수 적용 여부 결정 (OI-L2 결과 기반)
- BPH 상향 가능성 예비 데이터 제공 (OI-L3 — 확정은 EXP-05에서 수행)

**English**

Measure the end-to-end latency of the full TimeGrapher pipeline (audio capture → DSP → Qt GUI rendering) by breaking it into 3 segments. Identify the bottleneck in each segment and verify whether end-to-end < 100 ms at 28,800 BPH is achievable.

**Technical questions this experiment must answer**:
- "What is the actual QAudioSource live capture callback period? (OI-L1)"
- "Does ② process→display exceed 30 ms when 11 tabs render simultaneously? (OI-L2)"
- "After achieving 28,800 BPH, can the target be raised to 36,000 / 43,200 BPH? (OI-L3 — data collection only; final resolution deferred to EXP-05)"

The results of this experiment directly inform:
- Finalizing QAS-2 Response Measure (BPH-based latency targets)
- Decision on whether Lazy Rendering tactic is mandatory (based on OI-L2 result)
- Preliminary data for BPH escalation feasibility (OI-L3 — final resolution in EXP-05)

---

### Status / 상태

**Planned**

---

### Expected outcomes / 예상 산출물

**한국어**

- 3구간(① capture→process, ② process→display, ③ end-to-end) × sps 3단계(48k/96k/192k) × 탭 구성 2가지(1탭/11탭)의 평균 + worst-case 지연 측정표
- QAudioSource 콜백 주기 실측값 (OI-L1 해소)
- Lazy Rendering 필요 여부 판정 (OI-L2: 11탭 기준 ② 구간 30 ms 초과 여부)
- BPH 상향 가능성 팀 보고용 수치 (OI-L3)
- QAS-2 Response Measure 최종 확정값

**English**

- Latency measurement table: 3 segments × 3 sps tiers × 2 tab configurations (1-tab / 11-tab), mean + worst-case
- QAudioSource callback period measured value (resolves OI-L1)
- Lazy Rendering necessity decision (OI-L2: whether ② segment exceeds 30 ms at 11 tabs)
- Figures for team report on BPH escalation feasibility (OI-L3)
- Finalized QAS-2 Response Measure

---

### Resources required / 필요 자원

**한국어**

| 자원 / Resource | 상세 / Detail |
|----------------|-------------|
| 하드웨어 | Raspberry Pi 5 (8GB RAM), USB 오디오 센서, 8" 터치스크린 |
| 소프트웨어 | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` 코드베이스 |
| 기계식 시계 | 28,800 BPH 시계 (1차 목표), 추가 BPH 시계 (OI-L3 검토 시) |
| 계측 도구 | `std::chrono::high_resolution_clock` 타임스탬프 3점(TS1/TS2/TS3), Qt `QElapsedTimer` |
| 탭 구성 | 1탭 빌드 vs. 11탭 완성 빌드 — 두 바이너리 또는 런타임 탭 수 조절 |
| 선행 조건 | EXP-01 완료 (sps 방향 결정), Observer 패턴 리팩터링 완료 |
| 노력 | 약 2~3 person-days |

**English**

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5 (8GB RAM), USB audio sensor, 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` codebase |
| Mechanical watch | 28,800 BPH watch (primary target); additional BPH watches (if OI-L3 is explored) |
| Measurement tools | `std::chrono::high_resolution_clock` timestamps at 3 points (TS1/TS2/TS3), Qt `QElapsedTimer` |
| Tab configuration | 1-tab build vs. 11-tab complete build — two binaries or runtime tab count control |
| Prerequisites | EXP-01 complete (SPS direction decided), Observer pattern refactoring complete |
| Effort | ~2–3 person-days |

---

### Experiment description / 실험 방법

**한국어**

**타임스탬프 삽입 위치 (3점)**:

| 포인트 | 위치 | 측정 구간 |
|--------|------|---------|
| **TS1** | ALSA 콜백 수신 직후 (`audioDataAvailable()` 진입 시점) | ① 시작 |
| **TS2** | T1/T3 이벤트 타임스탬프 확정 직후 (DSP 처리 완료) | ① 끝 / ② 시작 |
| **TS3** | Qt `paintEvent()` 종료 직후 (GUI 렌더링 완료) | ② 끝 = ③ 끝 |

**실험 절차**:

1. **타임스탬프 코드 삽입**: TS1·TS2·TS3 위치에 `std::chrono::high_resolution_clock::now()` 기록 코드 삽입. 각 beat 이벤트마다 3개 값을 로그 파일에 기록.
2. **1탭 구성 측정**: 탭 1개만 활성화한 빌드에서 sps 3단계(48k/96k/192k) × 28,800 BPH 시계 × 5분 연속 실행. 각 구간 평균·worst-case 수집.
3. **11탭 구성 측정**: 11개 탭이 모두 활성화된 빌드에서 동일 조건 반복. ② 구간(process→display)의 차이를 1탭 대비 비교.
4. **콜백 주기 측정 (OI-L1)**: TS1 간격 히스토그램으로 QAudioSource 콜백 실제 주기 및 지터 분포 파악.
5. **Lazy Rendering 판정 (OI-L2)**: 11탭 기준 ② 구간 > 30 ms 발생 빈도 계산. 기준 초과 시 Lazy Rendering 전술 적용 결정.
6. **BPH 상향 검토 (OI-L3)**: 28,800 BPH end-to-end < 100 ms 달성 시, 36,000 / 43,200 BPH 시계로 동일 측정 반복하여 상향 가능성 팀 보고.
7. **결과 정리**: QAS-2 BPH별 지연 목표 표에 실측값 반영, Response Measure 확정.

**English**

**Timestamp injection points (3 points)**:

| Point | Location | Measured segment |
|-------|----------|-----------------|
| **TS1** | Immediately after ALSA callback received (entry of `audioDataAvailable()`) | ① start |
| **TS2** | Immediately after T1/T3 event timestamp is finalized (DSP processing complete) | ① end / ② start |
| **TS3** | Immediately after Qt `paintEvent()` completes (GUI rendering done) | ② end = ③ end |

**Experiment procedure**:

1. **Inject timestamp code**: Insert `std::chrono::high_resolution_clock::now()` at TS1·TS2·TS3 positions. Log all three values per beat event to a log file.
2. **1-tab configuration measurement**: Build with only one tab active; run for 5 minutes at each sps tier (48k/96k/192k) × 28,800 BPH watch. Collect mean and worst-case per segment.
3. **11-tab configuration measurement**: Repeat identical conditions with all 11 tabs active. Compare ② segment (process→display) against 1-tab baseline.
4. **Callback period measurement (OI-L1)**: Plot histogram of TS1 intervals to characterize actual QAudioSource callback period and jitter distribution.
5. **Lazy Rendering decision (OI-L2)**: Calculate frequency of ② segment > 30 ms at 11 tabs. If threshold exceeded, trigger Lazy Rendering tactic application decision.
6. **BPH escalation review (OI-L3)**: If 28,800 BPH end-to-end < 100 ms is achieved, repeat identical measurement with 36,000 / 43,200 BPH watches and report feasibility to team.
7. **Finalize results**: Populate QAS-2 BPH-based latency target table with measured values; confirm Response Measure.

---

### Duration / 기간

**한국어**

- **목표 완료**: Sprint 2~3 이내 (2026-06-12 전후)
- **선행 조건**: ① EXP-01 완료 (sps 방향 결정) ② Observer 패턴 리팩터링 완료 (11탭 빌드가 있어야 탭 구성 비교 가능)
- **완료 판정 기준**: 3구간 × 탭 구성 2가지 측정표 완성 + OI-L1·OI-L2 결론 기록 + QAS-2 Response Measure 확정

**English**

- **Target completion**: Within Sprint 2–3 (reference: ~2026-06-12)
- **Prerequisites**: ① EXP-01 complete (SPS direction confirmed) ② Observer pattern refactoring complete (11-tab build needed for tab comparison)
- **Completion criteria**: 3-segment × 2-tab-configuration measurement table complete + OI-L1·OI-L2 conclusions recorded + QAS-2 Response Measure finalized

---

### Links and references / 참고 자료

**한국어**

- `docs/milestone1/final/architectural-drivers.md` — QAS-2, OI-L1, OI-L2, OI-L3
- `docs/milestone1/final/risk-assessment.md` — TR-03, TR-04, 액션 계획 순서 4
- `docs/milestone1/quality_attribute_scenarios_latency.md` — 원본 QA 분석
- Qt6 QAudioSource API — callback period documentation
- Linux `perf` tool documentation

**English**

- `docs/milestone1/final/architectural-drivers.md` — QAS-2, OI-L1, OI-L2, OI-L3
- `docs/milestone1/final/risk-assessment.md` — TR-03, TR-04, Action Plan step 4
- `docs/milestone1/quality_attribute_scenarios_latency.md` — source QA analysis
- Qt6 QAudioSource API — callback period documentation
- Linux `perf` tool documentation

---

## Experiment EXP-03: Detector 파라미터 소음 조건 최적화 / Detector Parameter Optimization Under Noise Conditions

---

### Results and recommendations / 결과 및 권고사항

**한국어**

(미완료 — 실험 수행 후 기록 예정)

**English**

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective / 목적

**한국어**

`Detector.cpp`의 `onset_fraction`과 `min_peak_fraction` 파라미터 조합을 소음 3조건(저/중/고 ambient noise) 하에서 체계적으로 탐색하여, Δ Rate / Δ Amplitude / Δ Beat Error를 최소화하는 최적 파라미터 조합을 확정한다.

**실험이 답해야 할 기술 질문**: "저/중/고 소음 3가지 환경에서 `onset_fraction`과 `min_peak_fraction`의 어떤 조합이 Rate / Amplitude / Beat Error 측정 편차(Δ)를 최소화하는가? (OI-C1)"

이 실험의 결과는 다음 설계 결정에 직접 사용된다:
- QAS-3 QA-C2 Response Measure 확정 (현재 "EXP-03 결과 후 확정"으로 표시된 잠정값)
- `Detector.cpp` 기본값 파라미터 업데이트 (TR-05 해소)
- Adaptive threshold 알고리즘의 실환경 유효성 검증

**English**

Systematically search combinations of `onset_fraction` and `min_peak_fraction` parameters in `Detector.cpp` across three noise conditions (low / medium / high ambient noise) to identify the optimal combination that minimizes Δ Rate / Δ Amplitude / Δ Beat Error.

**Technical question**: "Across low / medium / high noise environments, which combination of `onset_fraction` and `min_peak_fraction` minimizes measurement deviation (Δ) for Rate / Amplitude / Beat Error? (OI-C1)"

The results of this experiment directly inform:
- Finalizing QAS-3 QA-C2 Response Measure (currently provisional, noted as "confirmed after EXP-03")
- Updating default parameters in `Detector.cpp` (resolves TR-05)
- Validating adaptive threshold algorithm effectiveness under real-world noise conditions

---

### Status / 상태

**Planned**

---

### Expected outcomes / 예상 산출물

**한국어**

- `onset_fraction` × `min_peak_fraction` 격자 탐색 결과표 (소음 3조건 × 파라미터 조합별 Δ Rate / Δ Amplitude / Δ Beat Error)
- 소음 조건별 최적 파라미터 조합 + 권장 기본값
- Adaptive threshold(noise floor = 최근 256 ms 75th percentile, reference_peak = 최근 16 beat median)의 소음 조건별 유효성 판정
- QAS-3 QA-C2 Response Measure 확정값 (허용 Δ 수치 포함)

**English**

- Grid search result table: `onset_fraction` × `min_peak_fraction` combinations × 3 noise conditions, with Δ Rate / Δ Amplitude / Δ Beat Error per cell
- Optimal parameter combination per noise condition + recommended default values
- Validity assessment of adaptive threshold (noise floor = 75th percentile of last 256 ms, reference_peak = median of last 16 beats) under each noise condition
- Finalized QAS-3 QA-C2 Response Measure (including acceptable Δ thresholds)

---

### Resources required / 필요 자원

**한국어**

| 자원 / Resource | 상세 / Detail |
|----------------|-------------|
| 하드웨어 | Raspberry Pi 5, USB 오디오 센서, 8" 터치스크린 |
| 소프트웨어 | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` 코드베이스 |
| 기계식 시계 | 28,800 BPH 시계 (기준 시계 — Detector 파라미터 튜닝 대상) |
| 소음 발생 환경 | 저/중/고 ambient noise 3가지 환경 조성 방안 (예: 조용한 실험실 / 사무실 / 소음원 추가) |
| 계측 도구 | Rate·Amplitude·Beat Error 각 조건별 N회 반복 측정 로그, Witschi 또는 기준 장비 비교값 |
| 선행 조건 | EXP-01 완료 (측정에 사용할 sps 확정) |
| 노력 | 약 2~3 person-days |

**English**

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5, USB audio sensor, 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` codebase |
| Mechanical watch | One 28,800 BPH watch (reference watch for parameter tuning) |
| Noise environment | Three ambient noise levels (e.g., quiet lab / office / added noise source) |
| Measurement tools | Rate·Amplitude·Beat Error log across N repetitions per condition; reference values from Witschi or equivalent |
| Prerequisites | EXP-01 complete (SPS to be used for measurement confirmed) |
| Effort | ~2–3 person-days |

---

### Experiment description / 실험 방법

**한국어**

**파라미터 탐색 범위 (격자 탐색)**:

| 파라미터 | 현재 기본값 | 탐색 범위 | 탐색 단계 |
|---------|:----------:|---------|:-------:|
| `onset_fraction` | 0.03 | 0.01 ~ 0.10 | 0.01 간격 |
| `min_peak_fraction` | 0.20 | 0.10 ~ 0.40 | 0.05 간격 |

**소음 조건 정의**:

| 조건 | 환경 설명 | 기대 noise floor |
|------|---------|:-------------:|
| 저소음 (Low) | 문 닫힌 조용한 실험실, 주변 장비 꺼짐 | ~30 dB SPL |
| 중소음 (Medium) | 일반 사무실 환경, 공조 및 대화 소음 | ~50 dB SPL |
| 고소음 (High) | 주변 소음원 추가 (음악 재생 등) | ~65 dB SPL |

**실험 절차**:

1. **기준값 수집**: 저소음 환경 + 기본 파라미터(`onset_fraction=0.03`, `min_peak_fraction=0.20`)로 Rate·Amplitude·Beat Error 30회 측정하여 기준값(baseline) 확보.
2. **소음 조건 순서 고정**: 저 → 중 → 고 순서로 각 소음 조건을 조성. 소음 조건 변경 시 2분 안정화 대기.
3. **격자 탐색**: 각 소음 조건에서 `onset_fraction` × `min_peak_fraction` 조합 전체를 순서대로 적용. 각 조합당 Rate·Amplitude·Beat Error 10회 측정.
4. **Δ 계산**: 각 조합의 평균 측정값과 기준값(저소음 + 기본 파라미터) 대비 Δ Rate / Δ Amplitude / Δ Beat Error 산출.
5. **최적 조합 선정**: 3개 소음 조건 전반에서 Δ 합산이 최소인 파라미터 조합을 최적값으로 선정.
6. **Adaptive threshold 검증**: adaptive threshold 활성화 상태에서 최적 파라미터 조합이 소음 급변 시(저→고 전환)에도 유효한지 추가 확인.
7. **결과 기록**: 최적 파라미터 조합 + QAS-3 QA-C2 허용 Δ 수치를 Architectural Drivers에 반영.

**English**

**Parameter search range (grid search)**:

| Parameter | Current default | Search range | Step |
|-----------|:--------------:|-------------|:----:|
| `onset_fraction` | 0.03 | 0.01 ~ 0.10 | 0.01 |
| `min_peak_fraction` | 0.20 | 0.10 ~ 0.40 | 0.05 |

**Noise condition definitions**:

| Condition | Environment description | Expected noise floor |
|-----------|------------------------|:-------------------:|
| Low noise | Quiet closed lab, surrounding equipment off | ~30 dB SPL |
| Medium noise | Typical office, HVAC and conversation noise | ~50 dB SPL |
| High noise | Added noise source (e.g., music playback) | ~65 dB SPL |

**Experiment procedure**:

1. **Collect baseline**: Measure Rate·Amplitude·Beat Error 30 times in low-noise environment with default parameters (`onset_fraction=0.03`, `min_peak_fraction=0.20`) to establish baseline.
2. **Fix noise condition order**: Low → Medium → High. Allow 2-minute stabilization after each noise condition change.
3. **Grid search**: For each noise condition, apply all `onset_fraction` × `min_peak_fraction` combinations sequentially. Measure Rate·Amplitude·Beat Error 10 times per combination.
4. **Compute Δ**: For each combination, calculate Δ Rate / Δ Amplitude / Δ Beat Error against the baseline (low noise + default parameters).
5. **Select optimal combination**: Choose the parameter combination with the minimum sum of Δ across all three noise conditions.
6. **Validate adaptive threshold**: With adaptive threshold active, verify that the optimal parameter combination also holds under abrupt noise transitions (low→high switch).
7. **Record results**: Reflect optimal parameter combination + QAS-3 QA-C2 acceptable Δ values into Architectural Drivers.

---

### Duration / 기간

**한국어**

- **목표 완료**: Sprint 3~4 이내 (2026-06-18 전후)
- **선행 조건**: EXP-01 완료 (측정에 사용할 sps 확정)
- **완료 판정 기준**: 3조건 × 전체 파라미터 조합 측정표 완성 + 최적 파라미터 확정 + QAS-3 QA-C2 Response Measure 기록

**English**

- **Target completion**: Within Sprint 3–4 (reference: ~2026-06-18)
- **Prerequisites**: EXP-01 complete (SPS for measurement confirmed)
- **Completion criteria**: 3-condition × full parameter combination measurement table complete + optimal parameters confirmed + QAS-3 QA-C2 Response Measure recorded

---

### Links and references / 참고 자료

**한국어**

- `docs/milestone1/final/architectural-drivers.md` — QAS-3, QA-C2, OI-C1
- `docs/milestone1/final/risk-assessment.md` — TR-05, 액션 계획 순서 5
- `docs/milestone1/correctness-analysis.md` — 원본 QA 분석
- `TimeGrapher_v10.5/Detector.cpp` — `onset_fraction`, `min_peak_fraction` 파라미터 구현

**English**

- `docs/milestone1/final/architectural-drivers.md` — QAS-3, QA-C2, OI-C1
- `docs/milestone1/final/risk-assessment.md` — TR-05, Action Plan step 5
- `docs/milestone1/correctness-analysis.md` — source QA analysis
- `TimeGrapher_v10.5/Detector.cpp` — `onset_fraction`, `min_peak_fraction` parameter implementation

---

## Experiment EXP-04: 신호 품질 경고 임계값 탐색 / Signal Quality Warning Threshold Search

---

### Results and recommendations / 결과 및 권고사항

**한국어**

(미완료 — 실험 수행 후 기록 예정)

**English**

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective / 목적

**한국어**

`⚠ No signal` / `⚠ Noisy signal` 경고의 발생·해제 임계값(N초, M초, noise/signal 비율)을 실제 환경에서 실측하여 오경보(false alarm)와 미감지(missed warning)를 최소화하는 값을 확정한다.

**실험이 답해야 할 기술 질문들**:
- "시계를 마이크 앞에서 제거한 후 몇 초 이내에 `⚠ No signal` 경고를 표시해야 사용자가 즉시 인지할 수 있는가? (OI-U1 — N 수치)"
- "시계를 복원한 후 몇 초 이내에 경고를 해제해야 안정적인 신호 복귀를 확인할 수 있는가? (OI-U1 — M 수치)"
- "noise/signal 비율의 어느 임계값을 초과할 때 `⚠ Noisy signal`을 표시해야 실제 환경에서 오경보 없이 작동하는가? (OI-U2)"

이 실험의 결과는 다음 설계 결정에 직접 사용된다:
- QAS-4 Response Measure 확정 (현재 N·M 수치 미결)
- Heartbeat 패턴 파라미터 설정 (N초 timeout 값)
- `⚠ Noisy signal` 임계값 코드 상수화

**English**

Measure the optimal onset/clear thresholds (N seconds, M seconds, noise/signal ratio) for `⚠ No signal` / `⚠ Noisy signal` warnings in real environments to minimize false alarms and missed warnings.

**Technical questions**:
- "After removing the watch from the microphone, within how many seconds should `⚠ No signal` appear for the user to notice immediately? (OI-U1 — N value)"
- "After restoring the watch, within how many seconds should the warning clear to confirm stable signal recovery? (OI-U1 — M value)"
- "What noise/signal ratio threshold triggers `⚠ Noisy signal` without false alarms in real environments? (OI-U2)"

The results of this experiment directly inform:
- Finalizing QAS-4 Response Measure (N·M values currently unresolved)
- Setting Heartbeat pattern parameter (N-second timeout value)
- Hardening `⚠ Noisy signal` threshold as a code constant

---

### Status / 상태

**Planned**

---

### Expected outcomes / 예상 산출물

**한국어**

- `⚠ No signal` 경고 발생까지 걸린 시간 분포 (시계 제거 후 N초 실측)
- `⚠ No signal` 경고 해제까지 걸린 시간 분포 (시계 복원 후 M초 실측)
- 소음 다환경(저/중/고) 조건에서 `⚠ Noisy signal` 임계값 후보군과 오경보율
- QAS-4 Response Measure 확정값 (N·M 수치 + noise/signal 임계값)
- Heartbeat 파라미터 권장값

**English**

- Distribution of time from watch removal to `⚠ No signal` warning appearance (measured N seconds)
- Distribution of time from watch restore to warning clearance (measured M seconds)
- `⚠ Noisy signal` threshold candidates and false-alarm rates across multiple noise environments (low/medium/high)
- Finalized QAS-4 Response Measure (N·M values + noise/signal threshold)
- Recommended Heartbeat parameter value

---

### Resources required / 필요 자원

**한국어**

| 자원 / Resource | 상세 / Detail |
|----------------|-------------|
| 하드웨어 | Raspberry Pi 5, USB 오디오 센서, 8" 터치스크린 |
| 소프트웨어 | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` 코드베이스 + 경고 표시 구현 완료 버전 |
| 기계식 시계 | 28,800 BPH 시계 (시계 제거/복원 조작 대상) |
| 소음 환경 | EXP-03과 동일 — 저/중/고 ambient noise 3가지 환경 |
| 계측 도구 | 경고 표시 시점 타임스탬프 로그, Heartbeat N 값 조정용 설정 파라미터 |
| 선행 조건 | Observer 패턴 리팩터링 완료 + `⚠ No signal` / `⚠ Noisy signal` 경고 UI 구현 완료 |
| 노력 | 약 1~2 person-days |

**English**

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5, USB audio sensor, 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` codebase with warning display implementation complete |
| Mechanical watch | One 28,800 BPH watch (subject of removal/restore operations) |
| Noise environment | Same as EXP-03 — three ambient noise levels (low/medium/high) |
| Measurement tools | Warning display timestamp log; configurable Heartbeat N-second parameter for sweep |
| Prerequisites | Observer pattern refactoring complete + `⚠ No signal` / `⚠ Noisy signal` warning UI implemented |
| Effort | ~1–2 person-days |

---

### Experiment description / 실험 방법

**한국어**

**Part A — `⚠ No signal` N·M 수치 탐색 (OI-U1)**:

1. Heartbeat N 파라미터를 1 / 2 / 3 / 5 초로 설정한 각 빌드 준비.
2. 각 N 설정에서 28,800 BPH 시계를 마이크 앞에서 제거하고 경고 표시까지 걸린 시간 측정 (10회 반복).
3. 시계를 복원하고 경고 해제까지 걸린 M 시간 측정 (10회 반복).
4. "너무 느림 (사용자 인지 지연)"과 "너무 빠름 (신호 변동으로 오경보)" 사이에서 최적 N·M 값 결정.

**Part B — `⚠ Noisy signal` 임계값 탐색 (OI-U2)**:

1. 저/중/고 소음 3조건 하에서 noise/signal 비율을 실시간 로그로 수집 (10분 × 3조건).
2. 소음 조건별 비율 분포(히스토그램)에서 저소음 정상 범위의 99th percentile과 고소음 범위의 1st percentile 사이 간격 확인.
3. 후보 임계값 3~5개를 적용하여 각 조건에서 오경보율(false alarm rate)과 미감지율(miss rate) 측정.
4. 오경보율 + 미감지율의 가중합이 최소인 임계값을 최종 선정.
5. QAS-4 Response Measure에 N·M 수치 + noise/signal 임계값 기록.

**English**

**Part A — `⚠ No signal` N·M value search (OI-U1)**:

1. Prepare builds with Heartbeat N parameter set to 1 / 2 / 3 / 5 seconds.
2. For each N setting, remove the 28,800 BPH watch from the microphone and measure time until warning appears (10 repetitions).
3. Restore the watch and measure time M until warning clears (10 repetitions).
4. Decide optimal N·M balancing "too slow (delayed user awareness)" vs. "too fast (false alarms from signal fluctuation)."

**Part B — `⚠ Noisy signal` threshold search (OI-U2)**:

1. Collect real-time noise/signal ratio logs under 3 noise conditions (10 minutes × 3 conditions).
2. Plot ratio distribution histogram per condition; check gap between 99th percentile of low-noise normal range and 1st percentile of high-noise range.
3. Apply 3–5 candidate thresholds and measure false-alarm rate and miss rate per condition.
4. Select final threshold with minimum weighted sum of false-alarm rate + miss rate.
5. Record N·M values + noise/signal threshold in QAS-4 Response Measure.

---

### Duration / 기간

**한국어**

- **목표 완료**: Sprint 3~4 이내 (2026-06-18 전후)
- **선행 조건**: Observer 패턴 리팩터링 완료 + `⚠ No signal` / `⚠ Noisy signal` 경고 UI 구현 완료
- **완료 판정 기준**: N·M 수치 확정 + noise/signal 임계값 확정 + QAS-4 Response Measure에 수치 기록 완료

**English**

- **Target completion**: Within Sprint 3–4 (reference: ~2026-06-18)
- **Prerequisites**: Observer pattern refactoring complete + `⚠ No signal` / `⚠ Noisy signal` warning UI implemented
- **Completion criteria**: N·M values confirmed + noise/signal threshold confirmed + QAS-4 Response Measure populated with confirmed values

---

### Links and references / 참고 자료

**한국어**

- `docs/milestone1/final/architectural-drivers.md` — QAS-4, OI-U1, OI-U2
- `docs/milestone1/final/risk-assessment.md` — TR-09, 액션 계획 순서 6
- `docs/milestone1/quality_attribute_scenarios_usability.md` — 원본 QA 분석
- Heartbeat 패턴 설명 — QAS-4 관련 아키텍처 전술

**English**

- `docs/milestone1/final/architectural-drivers.md` — QAS-4, OI-U1, OI-U2
- `docs/milestone1/final/risk-assessment.md` — TR-09, Action Plan step 6
- `docs/milestone1/quality_attribute_scenarios_usability.md` — source QA analysis
- Heartbeat pattern description — QAS-4 related architecture tactic

---

## Experiment EXP-05: BPH 상향 검증 — 36k/43k BPH 지연 측정 / BPH Escalation Verification — 36k/43k BPH Latency Measurement

---

### Results and recommendations / 결과 및 권고사항

**한국어**

(미완료 — 28,800 BPH 기준 QA 전부 충족 후 수행 예정)

**English**

(Not yet conducted — to be performed after all 28,800 BPH QA targets are confirmed.)

---

### Objective / 목적

**한국어**

28,800 BPH 기준 QA(QAS-1~4) 전부 충족을 전제로, 36,000 / 43,200 BPH 기계식 시계에서 end-to-end 지연이 각 beat 주기의 80% 이내에 완료되는지 검증한다. 팀 제2목표(BPH 범위 확대) 달성 가능 여부를 확정한다.

**실험이 답해야 할 기술 질문**: "36,000 / 43,200 BPH 시계에서 EXP-02와 동일한 3구간 측정 조건 하에 end-to-end 지연 목표를 달성할 수 있는가? (OI-L3)"

이 실험의 결과는 다음 결정에 사용된다:
- QAS-2 Response Measure Stretch 목표 달성 여부 확정
- 팀 제2목표(2nd: BPH 범위 확대) 달성 선언 또는 포기 결정

**English**

Assuming all 28,800 BPH QA targets (QAS-1~4) are confirmed, verify that end-to-end latency completes within 80% of each beat period for 36,000 / 43,200 BPH mechanical watches. Confirms whether the team's 2nd goal (wider BPH coverage) is achievable.

**Technical question**: "Under the same 3-segment measurement conditions as EXP-02, can end-to-end latency targets be met for 36,000 / 43,200 BPH watches? (OI-L3)"

Results inform:
- Confirming whether QAS-2 Stretch target is achieved
- Decision to declare or abandon the team's 2nd goal (BPH range expansion)

---

### Status / 상태

**Deferred** — 28,800 BPH 기준 QA 전부 충족 후 진행 / Blocked until all 28,800 BPH QA targets are confirmed

---

### Expected outcomes / 예상 산출물

**한국어**

- 36,000 / 43,200 BPH 시계 × 3구간(①②③) × sps 확정값 조건의 지연 측정표
- beat 주기 대비 end-to-end 지연 비율 (< 80% 달성 여부)
- QAS-2 Stretch 목표 달성·미달 판정 + 팀 제2목표 선언 여부 결정

**English**

- Latency measurement table: 36,000 / 43,200 BPH watches × 3 segments (①②③) × confirmed sps
- End-to-end latency as a fraction of beat period (whether < 80% is met)
- QAS-2 Stretch target pass/fail judgment + team 2nd goal declaration decision

---

### Resources required / 필요 자원

**한국어**

| 자원 / Resource | 상세 / Detail |
|----------------|-------------|
| 하드웨어 | Raspberry Pi 5 (8GB RAM), USB 오디오 센서, 8" 터치스크린 |
| 소프트웨어 | EXP-02 완료 기준 빌드 (타임스탬프 삽입 코드 재활용) |
| 기계식 시계 | 36,000 BPH 시계, 43,200 BPH 시계 |
| 선행 조건 | EXP-02 완료 + 28,800 BPH 기준 QAS-1~4 전부 충족 확인 |
| 노력 | 약 1 person-day (EXP-02 셋업 재활용) |

**English**

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5 (8GB RAM), USB audio sensor, 8" touchscreen |
| Software | EXP-02-complete build (reuse timestamp injection code) |
| Mechanical watches | 36,000 BPH watch, 43,200 BPH watch |
| Prerequisites | EXP-02 complete + QAS-1~4 all confirmed at 28,800 BPH |
| Effort | ~1 person-day (reuses EXP-02 setup) |

---

### Duration / 기간

**한국어**

- **목표 완료**: 28,800 BPH QA 충족 확인 후 가능한 빠른 시점 (Week 3 이후)
- **선행 조건**: EXP-02 완료 + QAS-1~4 28,800 BPH 기준 전부 통과
- **완료 판정 기준**: 36k/43k BPH 측정표 완성 + OI-L3 결론 기록 + QAS-2 Stretch 달성 여부 확정

**English**

- **Target completion**: As soon as possible after 28,800 BPH QA confirmation (Week 3+)
- **Prerequisites**: EXP-02 complete + all QAS-1~4 passed at 28,800 BPH
- **Completion criteria**: 36k/43k BPH measurement table complete + OI-L3 conclusion recorded + QAS-2 Stretch confirmed

---

### Links and references / 참고 자료

**한국어**

- `docs/milestone1/final/architectural-drivers.md` — QAS-2 Stretch, OI-L3
- `docs/milestone1/final/risk-assessment.md` — OI-L3
- `docs/milestone1/final/planned-experiments.md` — EXP-02 (선행 실험, 셋업 재활용)

**English**

- `docs/milestone1/final/architectural-drivers.md` — QAS-2 Stretch, OI-L3
- `docs/milestone1/final/risk-assessment.md` — OI-L3
- `docs/milestone1/final/planned-experiments.md` — EXP-02 (prerequisite experiment, setup reused)
