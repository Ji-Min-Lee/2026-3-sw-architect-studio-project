# 계획된 실험 / Planned Experiments

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09

---

## 실험 목록 / Summary

**한국어**  
각 실험은 risk-assessment의 미해결 이슈(OI) 또는 기술 리스크(TR)를 해소하고, QA 수치를 확정하기 위해 설계된다.

**English**  
Each experiment resolves a specific open issue (OI) or technical risk (TR) and confirms QA target values.

| ID | 실험 / Experiment | 해결 대상 / Resolves | 검증 QA | 선행 조건 / Prerequisite |
|----|-----------------|---------------------|---------|------------------------|
| EX-01 | RPi 성능 벤치마크 / RPi Performance Benchmark | OI-03, TR-03, TR-04, TR-07 | QAS-1, QAS-3 | — |
| EX-02 | `tg_c_placement_t` 비교 / Placement Setting Comparison | OI-01, OI-02, TR-01, TR-02 | QAS-2 | EX-03 |
| EX-03 | WeiShi & RPi 동시 측정 환경 / Simultaneous Measurement Setup | OI-09, TR-08 | QAS-2 | — |

> **권장 실행 순서 / Recommended Order**: EX-01 & EX-03 (병렬 / parallel) → EX-02

---

## EX-01: RPi 성능 벤치마크 / RPi Performance Benchmark

### 결과 및 권고 / Results and Recommendations

**한국어** *(M2에서 기록)*

**English** *(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

Qt GUI가 실행 중인 RPi 5에서 오디오 블록 드롭 없이 지속할 수 있는 최대 sps는 얼마인가? capture→process 구간 지연은 얼마인가?

이 실험은 다음 결정의 근거 데이터를 제공한다:
- QAS-1: 목표 sps 확정 (96k 유지 / 48k 폴백 여부)
- QAS-3: capture→process 지연 수치 확정 (현재 잠정값 < 70 ms)

**English**

What is the maximum sps the RPi 5 can sustain without dropping audio blocks while running the Qt GUI? What is the capture→process latency?

This experiment provides data for:
- QAS-1: Confirm target sps (sustain 96k or fall back to 48k)
- QAS-3: Finalize capture→process latency (current provisional: < 70 ms)

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- 48k / 96k / 192k sps별 CPU 점유율, 드롭 블록 수, capture→process 지연, GUI FPS 측정표
- 목표 sps 결정 (96k 유지 / 48k 폴백) 및 근거
- QAS-1 / QAS-3 수치 확정 (잠정값 대체)

**English**

- Table of CPU usage, dropped block count, capture→process latency, and GUI FPS at 48k / 96k / 192k sps
- Target sps decision (sustain 96k / fall back to 48k) with rationale
- Finalized QAS-1 / QAS-3 values (replacing provisional figures)

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | RPi 5 (8GB), 8" touchscreen, USB audio sensor |
| 소프트웨어 / Software | TimeGrapher_v10.5 Sim mode, AlsaMixer (AGC off), `htop`, `QElapsedTimer` |
| 공수 / Effort | 0.5 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. AlsaMixer에서 AGC 비활성화 후 TimeGrapher Sim 모드 실행
2. 48k → 96k → 192k sps 순서로 각 5분 실행
3. 각 sps에서 측정: CPU 점유율, 드롭 블록 수, capture→process 지연 (`QElapsedTimer`), GUI FPS
4. FFT 스펙트로그램 탭 활성화 후 동일 측정 반복 (TR-07)
5. 결과 표 작성 및 48k 폴백 여부 결정

**English**

1. Disable AGC in AlsaMixer and run TimeGrapher in Sim mode
2. Run at 48k → 96k → 192k sps, 5 minutes each
3. At each sps, measure: CPU usage, dropped blocks, capture→process latency (`QElapsedTimer`), GUI FPS
4. Enable FFT spectrogram tab and repeat (TR-07)
5. Compile results table and decide on 48k fallback

---

### 완료 기준 / Completion Criteria

**한국어**

- 세 가지 sps에서 모두 측정 데이터 수집 완료 (FFT 조건 포함)
- 목표 sps 팀 합의 완료
- QAS-1 / QAS-3 잠정값이 실측 데이터로 대체됨

**English**

- Measurement data collected at all three sps including FFT-active condition
- Team agreement on target sps
- QAS-1 / QAS-3 provisional values replaced with empirical data

---

### 기간 / Duration

**한국어** 목표 완료: **2026-06-07**

**English** Target completion: **2026-06-07**

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-03, TR-03, TR-04, TR-07](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-1, QAS-3](./architectural-drivers.md)

---

## EX-02: `tg_c_placement_t` 비교 / Placement Setting Comparison

### 결과 및 권고 / Results and Recommendations

**한국어** *(M2에서 기록)*

**English** *(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

`TG_C_PLACEMENT_PEAK`(C 이벤트 최대 진폭 지점)와 `TG_C_PLACEMENT_ONSET`(C peak에서 역방향 half-height 교차점) 중 어느 설정이 WeiShi No.1000 대비 Rate / Beat Error 오차를 최소화하는가?

onset / peak 감지 자체는 `Detector.cpp` / `Timegrapher.h`에 이미 구현되어 있다. `tg_c_placement_t` 파라미터로 primary timing 기준을 전환할 수 있으므로, 이 실험은 **최적 설정 선택**이 목적이다. 결과는 QAS-2 수치 (Rate < 5 s/d, Beat Error < 0.1 ms) 확정의 근거가 된다.

> **선행 조건**: EX-03 완료 후 실행

**English**

Which setting — `TG_C_PLACEMENT_PEAK` (C event at maximum amplitude) or `TG_C_PLACEMENT_ONSET` (half-height crossing found by backward walk from peak) — minimizes Rate and Beat Error error vs WeiShi No.1000?

Onset/peak detection is already implemented in `Detector.cpp` / `Timegrapher.h`. Since `tg_c_placement_t` switches the primary timing reference, this experiment selects the **optimal setting**. Results confirm QAS-2 thresholds (Rate < 5 s/d, Beat Error < 0.1 ms).

> **Prerequisite**: Run after EX-03 is concluded.

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` 설정별 Rate / Beat Error 오차(mean, std dev) 비교표 (2종 이상 시계)
- 최적 `tg_c_placement_t` 설정 확정
- QAS-2 수치 확정 (잠정값 대체)

**English**

- Comparison table of Rate / Beat Error error (mean, std dev) for PEAK vs ONSET setting across 2+ watch models
- Confirmed optimal `tg_c_placement_t` setting
- Finalized QAS-2 thresholds (replacing provisional values)

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | 기계식 시계 (2종 이상), WeiShi No.1000, USB 마이크 |
| 소프트웨어 / Software | TimeGrapher_v10.5 Playback mode (`tg_c_placement_t` 파라미터 전환) |
| 관련 코드 / Code | `src/Detector.cpp`, `src/Detector.h`, `src/Timegrapher.h` |
| 공수 / Effort | 1 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

> EX-03에서 확정된 측정 방식으로 진행한다.

1. WeiShi No.1000으로 레퍼런스 Rate, Beat Error 기록
2. 동일 시계 신호를 RPi USB 마이크로 PCM 녹음 (30초 이상)
3. Playback 모드에서 (a) `TG_C_PLACEMENT_PEAK`, (b) `TG_C_PLACEMENT_ONSET`으로 각각 실행
4. 각 설정의 Rate, Beat Error를 WeiShi 레퍼런스와 비교
5. 오차 통계 (mean, std dev) 산출; 낮은 오차 설정 확정
6. 시계 2종 이상으로 반복하여 일반성 검증

**English**

> Use the measurement method confirmed in EX-03.

1. Record reference Rate and Beat Error from WeiShi No.1000
2. Record PCM audio of the same watch via USB mic (30+ seconds)
3. Run Playback mode with (a) `TG_C_PLACEMENT_PEAK`, (b) `TG_C_PLACEMENT_ONSET`
4. Compare Rate and Beat Error for each setting against WeiShi reference
5. Compute error statistics (mean, std dev); confirm lower-error setting
6. Repeat with 2+ watch models to verify generalizability

---

### 완료 기준 / Completion Criteria

**한국어**

- 두 설정의 오차 수치가 2종 이상 시계에 대해 측정됨
- 최적 `tg_c_placement_t` 설정 팀 합의로 확정
- QAS-2 잠정값이 실측 데이터로 대체됨

**English**

- Error data collected for both settings across 2+ watch models
- Optimal `tg_c_placement_t` setting confirmed by team
- QAS-2 provisional values replaced with empirical data

---

### 기간 / Duration

**한국어** 목표 완료: **2026-06-08** *(EX-03 완료 후)*

**English** Target completion: **2026-06-08** *(after EX-03 concluded)*

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-01, OI-02, TR-01, TR-02](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-2](./architectural-drivers.md)

---

## EX-03: WeiShi & RPi 동시 측정 환경 / Simultaneous WeiShi & RPi Measurement Setup

### 결과 및 권고 / Results and Recommendations

**한국어** *(M2에서 기록)*

**English** *(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

WeiShi No.1000과 RPi가 동일 시계 신호를 동시에 수집하는 환경 구축이 가능한가?

QAS-2는 WeiShi No.1000 대비 오차를 기준으로 한다. 동일 입력 신호 없이는 시계 운동 변동성 때문에 두 장비의 측정값을 의미 있게 비교할 수 없다 (OI-09, TR-08). 이 실험 결과가 EX-02의 측정 방식을 확정한다.

**English**

Is it feasible to simultaneously collect acoustic signals from the same watch using both WeiShi No.1000 and RPi?

QAS-2 uses WeiShi No.1000 as the reference. Without the same input signal, watch movement variation makes comparison meaningless (OI-09, TR-08). This experiment confirms the measurement method for EX-02.

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- 동시 측정 가능 여부 판정 (마이크 스플리터 / 순차 측정 대안 포함)
- 채택된 측정 방식 및 셋업 절차 문서화
- 순차 측정 채택 시 시계 드리프트로 인한 오차 범위 추정

**English**

- Go/no-go verdict on simultaneous measurement (including mic splitter / sequential fallback)
- Confirmed measurement method and setup procedure
- If sequential: estimated error margin from watch drift

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | WeiShi No.1000, RPi 5, USB 마이크, (후보) 마이크 스플리터 |
| 소프트웨어 / Software | TimeGrapher_v10.5 |
| 공수 / Effort | 0.5 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. 마이크 스플리터로 WeiShi No.1000과 RPi USB 마이크에 동시 신호 공급 시도
2. 성공 시: 동일 신호로 두 장비 측정값 비교, 반복 재현성 확인 (3회)
3. 실패 시: 순차 측정 (WeiShi → 즉시 → RPi) 진행, 드리프트 오차 범위 추정
4. 채택된 방식 문서화 → EX-02에 전달

**English**

1. Attempt simultaneous signal feed to WeiShi No.1000 and RPi mic via mic splitter
2. If feasible: compare measurements on same signal, verify repeatability (3 trials)
3. If not feasible: perform sequential measurement (WeiShi → immediately → RPi), estimate drift error
4. Document confirmed method → hand off to EX-02

---

### 완료 기준 / Completion Criteria

**한국어**

- 동시 또는 순차 측정 방식 중 하나 결정 및 문서화
- 채택된 방식으로 3회 반복 재현성 확인 완료

**English**

- One measurement method (simultaneous or sequential) decided and documented
- Repeatability confirmed over 3 trials with the confirmed method

---

### 기간 / Duration

**한국어** 목표 완료: **2026-06-07** *(EX-02 이전)*

**English** Target completion: **2026-06-07** *(before EX-02)*

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-09, TR-08](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-2](./architectural-drivers.md)
