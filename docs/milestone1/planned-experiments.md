# 계획된 실험 / Planned Experiments

> **작성일 / Date**: 2026-06-04  
> **팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09

---

## 개요 / Overview

**한국어**

각 실험은 risk-assessment에서 식별된 특정 미결 질문 또는 기술 리스크를 해소하기 위해 설계되었다.
실험 결과는 M2에서의 아키텍처 결정(sample rate 선택, beat 감지 방식, 필터 파라미터, 스레딩 모델 등)의 근거 데이터로 활용된다.

**English**

Each experiment is designed to resolve a specific open question or technical risk identified in the risk assessment.
Results feed into M2 architecture decisions (sample rate target, beat detection method, filter parameters, threading model, etc.).

---

## EX-01: RPi 5 샘플레이트 성능 검증 / Sample Rate Performance on Raspberry Pi 5

### 결과 및 권고 / Results and Recommendations

**한국어**

*(M2에서 기록 예정)*

**English**

*(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

Qt GUI가 실행 중인 Raspberry Pi 5에서 오디오 블록 드롭 없이 유지할 수 있는 최대 샘플레이트는 무엇인가?

QAS-1 (Real-Time Performance)은 96,000 sps 처리 유지를 목표로 하고, 최소 기준은 48,000 sps로 정의되어 있다.
그러나 RPi 5에서 Qt GUI와 신호 처리를 동시에 실행할 때의 실제 처리 한계가 검증되지 않았다 (OI-03, TR-03).
본 실험은 다음 결정을 위한 데이터를 제공한다:

- 아키텍처 목표 샘플레이트 (96k vs 48k) 확정
- 오디오 캡처 스레드와 GUI 렌더링 스레드의 분리 필요 여부 판단
- FFT 스펙트로그램 활성화 시의 CPU 과부하 여부 확인 (TR-07)

**English**

What is the maximum sustained audio sample rate the RPi 5 can process without dropping blocks while running the Qt GUI?

QAS-1 (Real-Time Performance) targets 96,000 sps with a minimum threshold of 48,000 sps.
However, the actual processing capacity of the RPi 5 running Qt GUI and signal processing concurrently has not been verified (OI-03, TR-03).
This experiment provides data for the following decisions:

- Confirm architecture target sample rate (96k vs 48k)
- Determine whether audio capture and GUI rendering threads must be separated
- Verify whether FFT spectrogram causes CPU overload (TR-07)

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- 48k / 96k / 192k sps 각각에서의 CPU 점유율, 메모리 사용량, 드롭된 오디오 블록 수, GUI FPS 측정 데이터
- Qt GUI 동시 실행 조건에서의 처리 가능 최대 sps 결론
- FFT 활성화 시 CPU 과부하 여부 판정
- M2 아키텍처 결정을 위한 목표 sps 권고

**English**

- Measured CPU usage, memory, dropped audio blocks, and GUI FPS at 48k / 96k / 192k sps
- Conclusion on maximum sustainable sps with concurrent Qt GUI
- Go/no-go on FFT spectrogram under load
- Target sps recommendation for M2 architecture decisions

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | Raspberry Pi 5 (8GB), 8" touchscreen, USB audio sensor |
| 소프트웨어 / Software | TimeGrapher_v10.5, AlsaMixer (AGC disabled), `htop`, Qt FPS counter |
| 데이터 / Data | TimeGrapher Sim mode (no watch required) |
| 공수 / Effort | 0.5 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. RPi 5에서 AlsaMixer로 AGC 비활성화 (OI-02 관련)
2. TimeGrapher를 Sim 모드로 실행
3. 샘플레이트를 48k → 96k → 192k sps 순으로 설정하고 각 5분간 실행
4. 각 설정에서 측정: CPU 점유율 (`htop`), 메모리, 드롭된 오디오 블록 수, GUI 렌더링 FPS
5. FFT 스펙트로그램 탭 활성화 후 동일 측정 반복
6. 결과를 표로 정리하고 48k 폴백 여부 결정

**English**

1. Disable AGC on RPi 5 via AlsaMixer (related to OI-02)
2. Launch TimeGrapher in Sim mode
3. Set sample rate to 48k → 96k → 192k sps, run each for 5 minutes
4. At each setting, measure: CPU usage (`htop`), memory, dropped audio blocks, GUI rendering FPS
5. Enable FFT spectrogram tab and repeat measurements
6. Summarize results in a comparison table and decide on 48k fallback

---

### 완료 기준 / Completion Criteria

**한국어**

- 세 가지 샘플레이트(48k / 96k / 192k)에서 모두 측정 데이터 수집 완료
- FFT 활성화 조건 포함
- "목표 sps 확정 + 48k 폴백 여부" 결정이 팀 내 합의됨
- dropped block = 0 달성 가능한 최대 sps가 문서에 명시됨

**English**

- Measurement data collected at all three sample rates (48k / 96k / 192k) including FFT-active condition
- Team agreement on target sps and 48k fallback decision
- Maximum sps achieving dropped block = 0 explicitly documented

---

### 기간 / Duration

**한국어**

M1 제출 전 완료 목표: **2026-06-07**

**English**

Target completion before M1 submission: **2026-06-07**

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-03, TR-03, TR-04, TR-07](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-1 (Real-Time Performance)](./architectural-drivers.md#qas-1-real-time-performance--priority-1)
- [Architectural Drivers — QAS-3 (Low Latency)](./architectural-drivers.md#qas-3-low-latency--priority-3)

---

## EX-02: Beat 이벤트 감지 정확도 / Beat Event Detection Accuracy

### 결과 및 권고 / Results and Recommendations

**한국어**

*(M2에서 기록 예정)*

**English**

*(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

`tg_c_placement_t` 설정 — `TG_C_PLACEMENT_PEAK`(C 이벤트의 최대 진폭 지점)와 `TG_C_PLACEMENT_ONSET`(C 이벤트에서 역방향으로 반높이를 교차하는 지점) — 중 어느 것이 WeiShi No.1000 대비 Rate와 Beat Error 오차를 최소화하는가?

onset/peak 감지 자체는 `Detector.cpp` / `Timegrapher.h`에 이미 구현되어 있다. C 이벤트는 두 개의 타이밍 정보를 동시에 제공한다:
- **C-peak**: burst 내 최대 진폭 지점 (parabolic interpolation)
- **C-onset**: C peak에서 역방향으로 half-height를 교차하는 지점 (V5.4 backward walk)

`tg_c_placement_t` 파라미터로 primary timing 기준을 전환할 수 있으므로, 본 실험은 **어느 설정이 측정 정확도를 높이는지** 확인하는 것이 목적이다 (OI-01, TR-01).
결과는 QAS-2 수치 임계값 (< 5 s/d, < 0.1 ms) 확정의 근거가 된다.

**English**

Which `tg_c_placement_t` setting — `TG_C_PLACEMENT_PEAK` (C-event at maximum amplitude) or `TG_C_PLACEMENT_ONSET` (C-event at half-height backward walk from peak) — minimizes Rate and Beat Error against WeiShi No.1000?

Onset and peak detection are already implemented in `Detector.cpp` / `Timegrapher.h`. Each C event carries two timing values:
- **C-peak**: maximum amplitude within burst (parabolic interpolation)
- **C-onset**: half-height crossing found by backward walk from C peak (V5.4)

Since `tg_c_placement_t` switches the primary timing reference, this experiment determines **which setting yields lower measurement error** (OI-01, TR-01).
Results confirm the numeric thresholds for QAS-2 (< 5 s/d, < 0.1 ms).

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` 설정별 Rate 오차(mean, std dev) 및 Beat Error 오차(mean, std dev) 비교표
- WeiShi No.1000 기준 측정값과의 수치 비교
- `tg_c_placement_t` 최적 설정 확정 결론
- QAS-2 수치 임계값 확정 (< 5 s/d, < 0.1 ms 또는 실측 기반 조정)

**English**

- Comparison table of Rate error (mean, std dev) and Beat Error error (mean, std dev) for `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET`
- Numeric comparison against WeiShi No.1000 reference values
- Confirmed optimal `tg_c_placement_t` setting
- Confirmed QAS-2 numeric thresholds (< 5 s/d, < 0.1 ms or empirically adjusted)

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | 기계식 시계 (1종 이상), WeiShi No.1000, USB 마이크 |
| 소프트웨어 / Software | TimeGrapher_v10.5 (Playback mode) — `tg_c_placement_t` 파라미터 전환 기능 활용 |
| 관련 코드 / Code | `src/Detector.cpp`, `src/Detector.h`, `src/Timegrapher.h` (`tg_c_placement_t`, `tg_raw_event_t`) |
| 데이터 / Data | 동일 시계의 PCM 녹음 파일 (30초 이상) — EX-06 동시 측정 환경 확보 후 진행 |
| 공수 / Effort | 1 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

> **전제**: EX-06(동시 측정 환경 구축)이 먼저 완료되어야 동일 입력 기반 비교가 가능하다.

1. EX-06에서 확정된 방식(동시 또는 순차)으로 WeiShi No.1000 레퍼런스 Rate, Beat Error 값 기록
2. 동일 시계 신호로 RPi에서 PCM 녹음 (30초 이상)
3. TimeGrapher Playback 모드에서 `tg_c_placement_t`를 (a) `TG_C_PLACEMENT_PEAK`, (b) `TG_C_PLACEMENT_ONSET`으로 각각 설정하여 실행
4. 각 설정에서 Rate, Beat Error 계산 결과를 WeiShi 레퍼런스와 비교
5. 오차 통계(mean, std dev) 산출; 더 낮은 오차를 보이는 설정을 확정
6. 시계 기종 2종 이상으로 반복하여 일반성 검증

**English**

> **Prerequisite**: EX-06 (simultaneous measurement setup) must be completed first to enable same-input comparison.

1. Record reference Rate and Beat Error from WeiShi No.1000 using the method confirmed in EX-06
2. Record PCM audio of the same watch on RPi (30+ seconds)
3. In TimeGrapher Playback mode, run with (a) `TG_C_PLACEMENT_PEAK` and (b) `TG_C_PLACEMENT_ONSET`
4. Compare calculated Rate and Beat Error against WeiShi reference for each setting
5. Compute error statistics (mean, std dev); confirm the lower-error setting
6. Repeat with 2+ watch models to verify generalizability

---

### 완료 기준 / Completion Criteria

**한국어**

- `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` 설정별 Rate 오차 및 Beat Error 오차 수치가 2종 이상의 시계에 대해 측정됨
- WeiShi No.1000 레퍼런스 대비 수치 비교표 완성
- `tg_c_placement_t` 최적 설정 팀 합의로 확정
- QAS-2의 "잠정" 수치가 실측 데이터로 대체됨

**English**

- Rate and Beat Error errors for `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` measured across 2+ watch models
- Comparison table against WeiShi No.1000 reference completed
- Optimal `tg_c_placement_t` setting confirmed by team agreement
- QAS-2 "tentative" thresholds replaced with empirical data

---

### 기간 / Duration

**한국어**

M1 제출 전 완료 목표: **2026-06-08**  
*(WeiShi No.1000 장비 접근 필요 — 장비 사용 가능 일정에 따라 조정)*

**English**

Target completion before M1 submission: **2026-06-08**  
*(Requires access to WeiShi No.1000 — adjust based on equipment availability)*

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-01, TR-01](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-2 (Measurement Accuracy)](./architectural-drivers.md#qas-2-measurement-accuracy--priority-2)
- TimeGrapher Equations_v0 (assets/)

---

## EX-03: 필터 파라미터 최적화 / Filter Parameter Sweep

### 결과 및 권고 / Results and Recommendations

**한국어**

*(M2에서 기록 예정)*

**English**

*(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

주변 노이즈를 효과적으로 제거하면서 T1·T3 beat 이벤트를 가장 잘 보존하는 Low-pass / High-pass 컷오프 주파수 조합은 무엇인가?

현재 코드에는 HPF만 부분 구현되어 있으며 (FR-04: ⚠️ 부분 구현), LPF는 미구현 상태이다.
필터 파라미터가 잘못 설정되면 beat 이벤트 신호가 왜곡되어 EX-02의 감지 정확도에 직접 영향을 미친다.
본 실험은 기본 컷오프 값을 확정하고, 필터 아키텍처 설계의 근거 데이터를 제공한다.

**English**

What LP/HP cutoff frequency combination best preserves T1·T3 beat events while rejecting ambient noise?

Currently, only HPF is partially implemented (FR-04: ⚠️ partial), and LPF is not implemented.
Incorrectly set filter parameters will distort beat event signals and directly affect detection accuracy in EX-02.
This experiment determines the default cutoff values and provides the empirical basis for the filter architecture design.

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- HP / LP 컷오프 조합별 T1·T3 신호 가시성 및 SNR 비교표
- 권장 기본 HP / LP 컷오프 값 (예: HP ~200 Hz, LP ~8,000 Hz)
- 시각화 자료 (필터 적용 전후 파형 비교)
- FR-04 완전 구현을 위한 파라미터 기준

**English**

- Comparison table of T1·T3 signal visibility and SNR across HP/LP cutoff combinations
- Recommended default HP/LP cutoff values (e.g., HP ~200 Hz, LP ~8,000 Hz)
- Visualizations: waveform before and after filter application
- Parameter baseline for full implementation of FR-04

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | USB 마이크, 기계식 시계 |
| 소프트웨어 / Software | Python (scipy, matplotlib) 또는 Qt 분석 스크립트 |
| 데이터 / Data | 주변 노이즈 포함/미포함 시계 PCM 녹음 파일 |
| 공수 / Effort | 0.5 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. 조용한 환경과 주변 노이즈가 있는 환경 각각에서 시계 PCM 녹음
2. HP 컷오프: 100 / 200 / 500 Hz, LP 컷오프: 4k / 8k / 16k Hz 조합 적용
3. 각 조합에서 T1·T3 이벤트의 SNR 및 감지 성공률 측정
4. 파형 시각화로 필터 효과 직관적 비교
5. 최적 컷오프 조합 확정; 이 값을 EX-02에 반영

**English**

1. Record watch PCM in a quiet environment and in an ambient-noise environment
2. Apply combinations of HP cutoffs (100 / 200 / 500 Hz) and LP cutoffs (4k / 8k / 16k Hz)
3. Measure T1·T3 event SNR and detection success rate for each combination
4. Visualize waveforms to compare filter effects intuitively
5. Confirm optimal cutoff combination; feed results into EX-02

---

### 완료 기준 / Completion Criteria

**한국어**

- 9개 이상의 HP/LP 조합에 대한 SNR 및 감지 성공률 데이터 수집
- 권장 HP / LP 기본값이 근거 데이터와 함께 문서화됨
- EX-02에 적용할 필터 파라미터 확정

**English**

- SNR and detection success rate data collected for 9+ HP/LP combinations
- Recommended HP/LP default values documented with supporting data
- Filter parameters for use in EX-02 confirmed

---

### 기간 / Duration

**한국어**

M1 제출 전 완료 목표: **2026-06-07** *(EX-02 이전에 완료)*

**English**

Target completion before M1 submission: **2026-06-07** *(must complete before EX-02)*

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-01, TR-01](./risk-assessment-revision.md)
- [Architectural Drivers — FR-04](./architectural-drivers.md)
- TimeGrapher Equations_v0 (assets/)

---

## EX-04: 크로스 컴파일 및 RPi 배포 / Cross-Compilation & RPi Deploy

### 결과 및 권고 / Results and Recommendations

**한국어**

*(M2에서 기록 예정)*

**English**

*(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

Qt 프로젝트를 macOS 또는 Windows에서 크로스 컴파일하여 RPi에 배포하는 것이 가능한가?
아니면 RPi에서 네이티브로 빌드해야 하는가?

팀원 2명이 macOS를 사용 중이며, RPi에 네이티브 빌드만 가능할 경우 로컬 오디오 테스트가 불가능해진다 (OI-05, TR-06).
개발 환경 전략 확정이 늦어지면 팀 전체의 개발 및 디버깅 속도에 직접 영향을 미친다.

**English**

Is it feasible to cross-compile the Qt project on macOS or Windows and deploy to RPi?
Or must we build natively on RPi?

Two team members use macOS; if only native RPi builds are possible, local audio testing becomes unavailable for them (OI-05, TR-06).
Delay in confirming the build strategy directly impacts the team's development and debugging speed.

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- macOS → RPi 크로스 컴파일 가능 여부 판정
- RPi 네이티브 빌드 시간 측정
- 권장 빌드 전략 (크로스 컴파일 vs 네이티브) 및 macOS 팀원을 위한 대안 (예: `Q_OS_MAC` 분기, `MacAudio` 스텁)

**English**

- Go/no-go verdict on macOS → RPi cross-compilation
- Measured RPi native build time
- Recommended build strategy (cross-compile vs native) and alternatives for macOS members (e.g., `Q_OS_MAC` branch, `MacAudio` stub)

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | macOS 개발 머신, Raspberry Pi 5, SSH 접속 |
| 소프트웨어 / Software | Qt Creator, RPi 크로스 컴파일 툴체인 (`aarch64-linux-gnu-g++`) |
| 공수 / Effort | 0.5 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. macOS에서 Qt Creator + RPi sysroot로 크로스 컴파일 시도
2. 성공 시: RPi에서 바이너리 실행 확인 및 빌드 절차 문서화
3. 실패 시: RPi 네이티브 빌드 확인, 빌드 시간 측정
4. macOS 팀원을 위한 `Q_OS_MAC` 분기 또는 `MacAudio` 스텁 프로토타입 작성
5. 최종 권장 빌드 전략 결정

**English**

1. Attempt Qt cross-compilation on macOS with RPi sysroot
2. If successful: verify binary runs on RPi and document the build procedure
3. If failed: confirm native build on RPi and measure build time
4. Prototype `Q_OS_MAC` branch or `MacAudio` stub for macOS team members
5. Decide final recommended build strategy

---

### 완료 기준 / Completion Criteria

**한국어**

- RPi 5에서 실행 가능한 바이너리 확인 (크로스 컴파일 또는 네이티브)
- macOS에서의 로컬 빌드 또는 스텁 방안 확정
- 빌드 전략 문서화 완료

**English**

- Confirmed working binary on RPi 5 (via cross-compile or native build)
- Local build strategy or stub solution confirmed for macOS members
- Build strategy documented

---

### 기간 / Duration

**한국어**

M1 제출 전 완료 목표: **2026-06-08**

**English**

Target completion before M1 submission: **2026-06-08**

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-05, TR-06](./risk-assessment-revision.md)
- [TimeGrapher GUI Set Up Instructions](../../.claude/skills/time-grapher/assets/) (assets/)

---

## EX-05: Qt 멀티탭 렌더링 성능 / Qt Multi-Tab Rendering Performance

### 결과 및 권고 / Results and Recommendations

**한국어**

*(M2에서 기록 예정)*

**English**

*(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

RPi 5에서 신호 처리 스레드가 백그라운드에서 실행되는 동안 Qt 멀티탭 렌더링이 목표 FPS를 유지할 수 있는가?

11개의 그래프 탭을 구현해야 하는 일정 리스크 (NR-02, OI-07) 가운데, 렌더링 성능이 구조적으로 보장 가능한지 먼저 검증해야 스레딩 모델 결정이 가능하다 (QAS-5 Extensibility, TR-04).
본 실험은 "탭 수 증가 → FPS 저하" 임계점을 파악하여, 별도 렌더 타이머 또는 lazy rendering 전략 채택 여부를 결정한다.

**English**

Can Qt multi-tab rendering sustain the target FPS on RPi 5 while signal processing runs in a background thread?

Given the schedule risk of implementing 11 graph tabs (NR-02, OI-07), we must verify whether rendering performance is structurally guaranteed before making threading model decisions (QAS-5 Extensibility, TR-04).
This experiment identifies the FPS degradation threshold as tab count increases, and informs whether to adopt separate render timers or lazy rendering.

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- 1 / 2 / 3 / 4 활성 탭 조건에서의 FPS 및 CPU 사용률 측정 데이터
- 신호 처리 스레드 실행 중 vs 미실행 중 비교
- FPS 저하 임계 탭 수 파악
- 스레딩 모델 결정 (separate render timers 또는 lazy rendering) 근거

**English**

- FPS and CPU usage data at 1 / 2 / 3 / 4 active tabs
- Comparison: with vs without signal processing thread running
- Identified FPS degradation threshold tab count
- Basis for threading model decision (separate render timers or lazy rendering)

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | Raspberry Pi 5 (8GB), 8" touchscreen |
| 소프트웨어 / Software | Qt Creator, FPS 측정 유틸리티, `QElapsedTimer` |
| 구현 / Implementation | placeholder `paint()` 구현된 stub QWidget 탭 2~4개 |
| 공수 / Effort | 1 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. placeholder `paint()` (단색 채우기)를 가진 stub QWidget 탭 4개 구현
2. RPi 5에서 1 → 2 → 3 → 4 탭 순으로 활성화하며 FPS 측정 (`QElapsedTimer`)
3. 신호 처리 스레드 없이 측정 후, Sim 모드 신호 처리 활성화하여 동일 측정 반복
4. FPS가 목표치 이하로 떨어지는 탭 수 파악
5. 결과를 바탕으로 렌더링 전략 (separate timer, lazy rendering 등) 결정

**English**

1. Implement 4 stub QWidget tabs with placeholder `paint()` (solid fill)
2. On RPi 5, activate tabs 1 → 2 → 3 → 4 and measure FPS (`QElapsedTimer`)
3. Measure without signal processing thread; then repeat with Sim mode signal processing active
4. Identify the tab count at which FPS drops below target
5. Based on results, decide rendering strategy (separate timers, lazy rendering, etc.)

---

### 완료 기준 / Completion Criteria

**한국어**

- 1~4 탭 × 신호 처리 유/무 조건에서 FPS 데이터 수집 완료
- FPS 목표치 유지 가능한 최대 탭 수 명시
- 스레딩 및 렌더링 모델 결정이 근거 데이터와 함께 문서화됨

**English**

- FPS data collected for 1–4 tabs × with/without signal processing
- Maximum tab count maintaining target FPS explicitly documented
- Threading and rendering model decision documented with supporting data

---

### 기간 / Duration

**한국어**

M1 제출 전 완료 목표: **2026-06-08** *(EX-01 이후 실행)*

**English**

Target completion before M1 submission: **2026-06-08** *(run after EX-01)*

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-03, TR-04](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-5 (Extensibility)](./architectural-drivers.md#qas-5-extensibility--priority-5)

---

## EX-06: WeiShi 1000 & RPi 동시 측정 환경 구축 / Simultaneous WeiShi 1000 & RPi Measurement Setup

### 결과 및 권고 / Results and Recommendations

**한국어**

*(M2에서 기록 예정)*

**English**

*(To be recorded at M2)*

---

### 목적 / Objective

**한국어**

WeiShi No.1000과 RPi 5가 동일 시계의 음향 신호를 동시에 수집하여 accuracy를 비교하는 환경 구축이 가능한가?

QAS-2는 WeiShi No.1000 대비 Rate 오차 < 5 s/d, Beat Error 오차 < 0.1 ms를 기준으로 한다.
동일 입력 신호를 사용하지 않으면 시계 운동 변동성 때문에 두 장비의 측정값을 의미 있게 비교할 수 없다 (OI-09, TR-08).
동시 수집이 불가능할 경우 순차 측정(sequential) 방식으로 대안을 확정하고, 그에 따른 오차 기준을 조정해야 한다.

**English**

Is it feasible to simultaneously collect acoustic signals from the same watch using both WeiShi No.1000 and the RPi 5 for accuracy comparison?

QAS-2 requires Rate error < 5 s/d and Beat Error error < 0.1 ms compared to WeiShi No.1000.
Without using the same input signal, watch movement variation makes meaningful comparison between the two devices impossible (OI-09, TR-08).
If simultaneous collection is infeasible, a sequential measurement alternative must be confirmed and accuracy thresholds adjusted accordingly.

---

### 상태 / Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### 기대 산출물 / Expected Outcomes

**한국어**

- 동시 측정 환경 구축 가능 여부 판정 (마이크 스플리터 / 멀티채널 ADC / 순차 측정)
- 채택된 측정 방식 및 셋업 절차 문서화
- 동시 측정 불가 시 순차 측정으로 인한 오차 한계 추정 및 QAS-2 임계값 조정 근거

**English**

- Go/no-go verdict on simultaneous measurement (mic splitter / multi-channel ADC / sequential)
- Confirmed measurement method and setup procedure documented
- If simultaneous not feasible: estimated error margin from sequential measurement and basis for QAS-2 threshold adjustment

---

### 필요 자원 / Resources Required

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| 하드웨어 / Hardware | WeiShi No.1000, RPi 5, USB 마이크, (후보) 마이크 스플리터 또는 멀티채널 USB ADC |
| 소프트웨어 / Software | TimeGrapher_v10.5, WeiShi 디스플레이 |
| 공수 / Effort | 0.5 person-day |

---

### 실험 절차 / Experiment Description

**한국어**

1. 마이크 스플리터를 사용하여 WeiShi No.1000과 RPi USB 마이크에 동시 신호 공급 시도
2. 동시 수집이 가능하면: 동일 시계 신호로 두 장비의 측정값 비교하고 오차 기록
3. 동시 수집 불가 시: WeiShi → 즉시 → RPi 순차 측정 진행; 시계 드리프트로 인한 오차 범위 추정
4. 채택된 방식의 반복 재현성 검증 (동일 조건 3회 반복)
5. 최종 측정 방식 및 QAS-2 임계값 조정 여부 결정

**English**

1. Attempt to simultaneously feed signal to WeiShi No.1000 and RPi USB mic using a mic splitter
2. If simultaneous collection is feasible: compare measurements from both devices on the same watch signal and record errors
3. If not feasible: perform sequential measurement (WeiShi → immediately → RPi); estimate error range from watch drift
4. Verify repeatability of the adopted method (3 repetitions under same conditions)
5. Confirm final measurement method and decide whether to adjust QAS-2 thresholds

---

### 완료 기준 / Completion Criteria

**한국어**

- 동시 측정 또는 순차 측정 방식 중 하나가 결정되고 문서화됨
- 채택된 방식으로 WeiShi No.1000 대비 RPi 측정 오차 데이터 수집 완료
- QAS-2 임계값 (< 5 s/d, < 0.1 ms) 유지 또는 실측 근거 기반으로 조정됨

**English**

- One measurement method (simultaneous or sequential) decided and documented
- RPi vs WeiShi No.1000 error data collected using the confirmed method
- QAS-2 thresholds (< 5 s/d, < 0.1 ms) maintained or empirically adjusted

---

### 기간 / Duration

**한국어**

M1 제출 전 완료 목표: **2026-06-08** *(WeiShi No.1000 장비 접근 필요)*

**English**

Target completion before M1 submission: **2026-06-08** *(requires access to WeiShi No.1000)*

---

### 참고 링크 / Links and References

- [Risk Assessment — OI-09, TR-08](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-2 (Measurement Accuracy)](./architectural-drivers.md#qas-2-measurement-accuracy--priority-2)

---

## 실험 현황 요약 / Experiment Status Summary

| ID | 제목 / Title | 상태 / Status | 해소 리스크 / Resolves | 완료 기한 / Deadline | M2 결과 |
|----|-------------|:-------------:|----------------------|:-------------------:|:-------:|
| EX-01 | RPi 5 샘플레이트 성능 / Sample Rate Performance | Planned | OI-03, TR-03, TR-04, TR-07 | 2026-06-07 | [ ] |
| EX-02 | Beat 감지 정확도 / Beat Event Detection Accuracy | Planned | OI-01, TR-01 | 2026-06-08 | [ ] |
| EX-03 | 필터 파라미터 최적화 / Filter Parameter Sweep | Planned | OI-01, TR-01 | 2026-06-07 | [ ] |
| EX-04 | 크로스 컴파일 & RPi 배포 / Cross-Compilation & RPi Deploy | Planned | OI-05, TR-06 | 2026-06-08 | [ ] |
| EX-05 | Qt 멀티탭 렌더링 성능 / Qt Multi-Tab Rendering Performance | Planned | OI-03, TR-04 | 2026-06-08 | [ ] |
| EX-06 | WeiShi 1000 & RPi 동시 측정 / Simultaneous Measurement Setup | Planned | OI-09, TR-08 | 2026-06-08 | [ ] |

> **실행 순서 권고 / Recommended Execution Order**: EX-03 → EX-01 → EX-02 → EX-04, EX-05, EX-06 (병렬 / parallel)

---

## 리뷰 체크리스트 / Review Checklist

**한국어**

- [ ] 모든 실험이 템플릿 구조(목적 / 상태 / 기대 산출물 / 필요 자원 / 실험 절차 / 완료 기준 / 기간 / 참고)를 따름
- [ ] 각 실험의 목적이 구체적인 질문 형태로 명시됨
- [ ] 각 실험이 risk-assessment의 특정 리스크 또는 OI와 연결됨
- [ ] 완료 기준이 명확하고 측정 가능함
- [ ] M1→M2 일정 내에 실행 가능한 범위

**English**

- [ ] All experiments follow the template structure (objective / status / expected outcomes / resources / description / completion criteria / duration / references)
- [ ] Each experiment's objective is articulated as a specific technical question
- [ ] Each experiment is linked to a specific risk or open issue from the risk assessment
- [ ] Completion criteria are clear and measurable
- [ ] Experiments are feasible within the M1→M2 timeline
