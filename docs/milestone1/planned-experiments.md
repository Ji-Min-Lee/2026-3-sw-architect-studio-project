# 계획된 실험 / Planned Experiments

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09 | **상태 / Status**: [ ] 초안 Draft / [ ] 최종 Final

> **한국어**
> 각 실험은 특정 미해결 이슈(OI) 또는 리스크(TR)에 답하기 위해 설계된다. 실험 결과는 M2 아키텍처 뷰와 QA 수치 확정에 직접 반영된다.
>
> **English**
> Each experiment is designed to answer a specific open issue (OI) or risk (TR). Results feed directly into M2 architecture views and finalized QA target values.

---

## 실험 목록 요약 / Experiment Summary

| ID | 제목 / Title | 해결 대상 / Resolves | M1 마감 / Due | 상태 / Status |
|----|-------------|---------------------|--------------|--------------|
| EX-01 | RPi 5 오디오 성능 및 캡처 지연 / RPi 5 Audio Performance & Capture Latency | TR-04, OI-03 | 06-09 (계획 완료) | [ ] 계획 Planned |
| EX-02 | Qt 렌더링 성능 및 표시 지연 / Qt Rendering Performance & Display Latency | TR-05, OI-03 | 06-09 (계획 완료) | [ ] 계획 Planned |
| EX-03 | 비트 감지 방식 비교 및 측정 정확도 / Beat Detection Method Comparison & Accuracy | TR-01, OI-01 | 06-09 (계획 완료) | [ ] 계획 Planned |
| EX-04 | AGC 비활성화 지속성 검증 / AGC Disable Persistence Verification | TR-02, OI-02 | 06-09 (계획 완료) | [ ] 계획 Planned |
| EX-05 | 신호 필터 파라미터 최적화 / Signal Filter Parameter Optimization | TR-01 | 06-09 (계획 완료) | [ ] 계획 Planned |
| EX-06 | 빌드 환경 검증 / Build Environment Verification | TR-10, OI-05 | 06-09 (계획 완료) | [ ] 계획 Planned |

> **EX-01 + EX-02** 결과 → Low Latency QA 3구간 수치 확정 (architectural-drivers QA-3)
> **EX-03** 결과 → Measurement Accuracy QA 수치 확정 (architectural-drivers QA-2)

---

## EX-01: RPi 5 오디오 성능 및 캡처 지연 / RPi 5 Audio Performance & Capture Latency

### 결과 및 권고 / Results and Recommendations

**한국어**
*(M2에서 작성)*

**English**
*(To be filled at M2)*

---

### 목적 / Objective

**한국어**

Raspberry Pi 5가 Qt GUI를 실행하는 동안 오디오 블록 누락 없이 지속할 수 있는 최대 샘플레이트(sps)는 얼마인가? 그리고 각 sps에서 캡처→처리(capture→process) 구간의 지연 시간은 얼마인가?

이 실험은 두 가지 아키텍처 결정을 내리기 위한 데이터를 제공한다:
1. 목표 sps 확정 (96k sps 유지 가능 여부, 또는 48k sps 폴백 필요 여부)
2. Low Latency QA-3의 ① capture→process 구간 수치 확정 (현재 잠정값: < 46 ms)

해결 대상 리스크: **TR-04** (RPi 96k sps 미검증), **OI-03** (GUI 실행 중 RPi 최대 sps 미확인)

**English**

What is the maximum sample rate (sps) that Raspberry Pi 5 can sustain without dropping audio blocks while running the Qt GUI? And what is the capture→process latency at each sps?

This experiment provides data for two architectural decisions:
1. Confirming the target sps (whether 96k sps is achievable, or whether a 48k sps fallback is needed)
2. Finalizing the ① capture→process segment of Low Latency QA-3 (current provisional value: < 46 ms)

Risks addressed: **TR-04** (RPi 96k sps unverified), **OI-03** (max sps with GUI running unconfirmed)

---

### 상태 / Status

**[ ] Planned**

---

### 예상 산출물 / Expected Outcomes

**한국어**

- sps별(48k / 96k / 192k) CPU 점유율, 드랍 블록 수, capture→process 지연 측정값 표
- 목표 sps 결정 (96k 유지 / 48k 폴백) 및 근거 문서
- Low Latency QA-3 ① 구간 수치 (< 46 ms 잠정값의 확정 또는 조정)
- 아키텍처 결정 사항: 스레드 분리 구조 필요 여부

**English**

- Table of CPU usage, dropped block count, and capture→process latency per sps (48k / 96k / 192k)
- Target sps decision (sustain 96k / fall back to 48k) with rationale document
- Finalized Low Latency QA-3 ① segment value (confirm or adjust < 46 ms provisional)
- Architectural decision: whether thread separation is required

---

### 필요 리소스 / Resources Required

**한국어**

- 하드웨어: Raspberry Pi 5 (8GB RAM), 8인치 터치스크린, USB 오디오 인터페이스
- 소프트웨어: TimeGrapher_v10.5, AlsaMixer (AGC 비활성화 상태), Sim 모드
- 계측 도구: `perf` / `htop` (CPU), Qt `QElapsedTimer` (구간 지연 측정), ALSA 버퍼 모니터
- 노력: 약 0.5 person-day

**English**

- Hardware: Raspberry Pi 5 (8GB RAM), 8" touchscreen, USB audio interface
- Software: TimeGrapher_v10.5, AlsaMixer (AGC disabled), Sim mode
- Measurement tools: `perf` / `htop` (CPU), Qt `QElapsedTimer` (segment latency), ALSA buffer monitor
- Effort: ~0.5 person-day

---

### 실험 절차 / Experiment Description

**한국어**

1. RPi 5에서 TimeGrapher를 Sim 모드로 실행 (실제 오디오 하드웨어 없이 처리 부하만 측정)
2. AlsaMixer에서 AGC 비활성화 확인 후 실험 시작
3. 48k sps → 96k sps → 192k sps 순서로 각 sps에서 5분간 실행
4. 각 sps에서 측정:
   - 드랍된 오디오 블록 수 (ALSA `xrun` 카운트)
   - CPU 점유율 평균 및 최대값
   - capture→process 구간 지연: 오디오 콜백 시작 → beat event 타임스탬프 생성 시점까지 `QElapsedTimer`로 측정
5. 터치스크린 GUI 활성화 상태에서 동일 측정 반복 (GUI 오버헤드 분리)
6. 결과 표 작성 및 48k sps 폴백 필요 여부 결정

**English**

1. Run TimeGrapher on RPi 5 in Sim mode (measures processing load without real audio hardware)
2. Confirm AGC is disabled in AlsaMixer before starting
3. Run sequentially at 48k → 96k → 192k sps, 5 minutes per rate
4. Measure at each sps:
   - Dropped audio block count (ALSA `xrun` count)
   - CPU usage: average and peak
   - capture→process latency: from audio callback start to beat event timestamp generation, measured with `QElapsedTimer`
5. Repeat measurements with touchscreen GUI active (to isolate GUI overhead)
6. Compile results table and decide whether 48k sps fallback is required

---

### 기간 / Duration

**한국어**

- 계획 완료: 2026-06-09 (M1 제출)
- 실험 실행: 2026-06-09 ~ 2026-06-13 (Week 2)
- 결과 보고: 2026-06-16 (M2 준비 시작 시점)

**English**

- Plan finalized: 2026-06-09 (M1 submission)
- Experiment execution: 2026-06-09 ~ 2026-06-13 (Week 2)
- Results reported: 2026-06-16 (M2 preparation start)

---

### 참고 자료 / Links and References

- `architectural-drivers.md` QA-1 (Real-Time Performance), QA-3 (Low Latency)
- `risk-assessment-revision.md` TR-04, OI-03
- ALSA buffer configuration: [https://www.alsa-project.org/wiki/FramesPeriods](https://www.alsa-project.org/wiki/FramesPeriods)

---

## EX-02: Qt 렌더링 성능 및 표시 지연 / Qt Rendering Performance & Display Latency

### 결과 및 권고 / Results and Recommendations

**한국어**
*(M2에서 작성)*

**English**
*(To be filled at M2)*

---

### 목적 / Objective

**한국어**

RPi 5에서 Qt GUI가 여러 그래프 탭을 동시에 렌더링할 때 FPS와 처리→표시(process→display) 구간 지연이 얼마인가? 탭 수가 늘어날수록 성능이 어떻게 변화하는가?

이 실험은 다음 아키텍처 결정을 내리기 위한 데이터를 제공한다:
1. Low Latency QA-3의 ② process→display 구간 수치 확정 (현재 잠정값: < 20 ms)
2. 렌더링 구조 결정: 공유 타이머 vs. 탭별 독립 타이머, GPU 가속 필요 여부

EX-01과 함께 Low Latency QA-3 end-to-end 수치 (< 66 ms 잠정)를 확정한다.

해결 대상 리스크: **TR-05** (Qt 렌더링 FPS 저하), **OI-03** (GUI 실행 중 성능 미확인)

**English**

What are the FPS and process→display latency of the Qt GUI rendering multiple graph tabs simultaneously on RPi 5? How does performance degrade as the number of active tabs increases?

This experiment provides data for two architectural decisions:
1. Finalizing the ② process→display segment of Low Latency QA-3 (current provisional: < 20 ms)
2. Rendering architecture: shared timer vs. per-tab independent timer, whether GPU acceleration is needed

Together with EX-01, this experiment finalizes the end-to-end Low Latency QA-3 value (provisional: < 66 ms).

Risks addressed: **TR-05** (Qt rendering FPS drops), **OI-03** (performance unverified with GUI active)

---

### 상태 / Status

**[ ] Planned**

---

### 예상 산출물 / Expected Outcomes

**한국어**

- 탭 수(1 / 2 / 4 / 8 / 11)별 렌더링 FPS 및 CPU 점유율 측정값 표
- process→display 구간 지연 수치 (< 20 ms 잠정값의 확정 또는 조정)
- end-to-end 지연 확정 (EX-01 결과와 합산)
- 렌더링 아키텍처 결정: 공유 타이머 / 독립 타이머 / 레이지 렌더링 선택

**English**

- Table of rendering FPS and CPU usage per tab count (1 / 2 / 4 / 8 / 11)
- process→display latency value (confirm or adjust < 20 ms provisional)
- Finalized end-to-end latency (combined with EX-01 result)
- Rendering architecture decision: shared timer / independent timer / lazy rendering

---

### 필요 리소스 / Resources Required

**한국어**

- 하드웨어: Raspberry Pi 5 (8GB RAM), 8인치 터치스크린
- 소프트웨어: Qt 프로토타입 (QCustomPlot 기반 스텁 탭 다수), TimeGrapher_v10.5
- 계측 도구: Qt `QElapsedTimer` (렌더링 구간), `htop` (CPU), Qt FPS 카운터
- 노력: 약 1 person-day (스텁 탭 프로토타입 구현 포함)

**English**

- Hardware: Raspberry Pi 5 (8GB RAM), 8" touchscreen
- Software: Qt prototype (multiple QCustomPlot stub tabs), TimeGrapher_v10.5
- Measurement tools: Qt `QElapsedTimer` (rendering segment), `htop` (CPU), Qt FPS counter
- Effort: ~1 person-day (including stub tab prototype implementation)

---

### 실험 절차 / Experiment Description

**한국어**

1. QCustomPlot 기반 스텁 그래프 탭 구현 (실제 DSP 없이 더미 데이터로 렌더링)
2. 탭 수를 1 → 2 → 4 → 8 → 11개로 순차 증가하며 각 조건에서 5분간 실행
3. 각 조건에서 측정:
   - 렌더링 FPS (Qt 타이머 콜백 주기 기준)
   - process→display 지연: beat event 타임스탬프 생성 → 화면 업데이트 완료까지 `QElapsedTimer`로 측정
   - CPU 점유율 평균 및 최대값
4. 백그라운드 스레드에서 신호 처리 시뮬레이션 동시 실행 (실제 운영 조건 재현)
5. 렌더링 FPS 임계값 확인: 허용 가능한 최대 탭 수 결정
6. EX-01 결과와 합산하여 end-to-end 지연 계산

**English**

1. Implement QCustomPlot-based stub graph tabs (render with dummy data, no real DSP)
2. Incrementally increase tab count: 1 → 2 → 4 → 8 → 11, run 5 minutes per condition
3. Measure at each condition:
   - Rendering FPS (based on Qt timer callback interval)
   - process→display latency: beat event timestamp generation → screen update complete, measured with `QElapsedTimer`
   - CPU usage: average and peak
4. Run signal processing simulation concurrently in a background thread (replicate real operating conditions)
5. Identify FPS threshold: determine maximum acceptable number of active tabs
6. Combine with EX-01 results to calculate end-to-end latency

---

### 기간 / Duration

**한국어**

- 계획 완료: 2026-06-09 (M1 제출)
- 실험 실행: 2026-06-10 ~ 2026-06-15 (Week 2, EX-01 이후)
- 결과 보고: 2026-06-16

**English**

- Plan finalized: 2026-06-09 (M1 submission)
- Experiment execution: 2026-06-10 ~ 2026-06-15 (Week 2, after EX-01)
- Results reported: 2026-06-16

---

### 참고 자료 / Links and References

- `architectural-drivers.md` QA-3 (Low Latency), QA-5 (Extensibility)
- `risk-assessment-revision.md` TR-05, OI-03
- QCustomPlot performance notes: [https://www.qcustomplot.com/index.php/support/forum](https://www.qcustomplot.com/index.php/support/forum)

---

## EX-03: 비트 감지 방식 비교 및 측정 정확도 / Beat Detection Method Comparison & Accuracy

### 결과 및 권고 / Results and Recommendations

**한국어**
*(M2에서 작성)*

**English**
*(To be filled at M2)*

---

### 목적 / Objective

**한국어**

T1 이벤트 감지 시 onset 방식과 peak 방식 중 어느 것이 Rate 및 Beat Error 측정에서 더 안정적이고 정확한 결과를 내는가? 감지된 결과가 WeiShi No.1000 레퍼런스 디바이스와 비교하여 목표 오차 범위 내에 있는가?

이 실험(architectural-drivers에서 "Experiment 3"으로 참조)은 다음을 확정한다:
- Measurement Accuracy QA-2의 Rate 오차 상한 (현재 잠정: < 5 s/d)
- Measurement Accuracy QA-2의 Beat Error 오차 상한 (현재 잠정: < 0.1 ms)
- T1 감지 알고리즘 설계 결정 (onset vs peak)

해결 대상 리스크: **TR-01** (T1/T3 감지 부정확), **OI-01** (T1 감지 기준점 미결정)

**English**

Between the onset method and the peak method for detecting T1 events, which produces more stable and accurate Rate and Beat Error measurements? Are the detected results within the target error margin compared to the WeiShi No.1000 reference device?

This experiment (referenced as "Experiment 3" in architectural-drivers) finalizes:
- Rate error ceiling for Measurement Accuracy QA-2 (current provisional: < 5 s/d)
- Beat Error error ceiling for Measurement Accuracy QA-2 (current provisional: < 0.1 ms)
- T1 detection algorithm design decision (onset vs peak)

Risks addressed: **TR-01** (T1/T3 detection inaccurate), **OI-01** (T1 detection reference point undecided)

---

### 상태 / Status

**[ ] Planned**

---

### 예상 산출물 / Expected Outcomes

**한국어**

- onset vs peak 방식별 Rate 오차 및 Beat Error 오차 비교 표 (평균, 표준편차, WeiShi 대비)
- 선택된 T1 감지 방식 및 근거 문서
- Measurement Accuracy QA-2 수치 확정 (< 5 s/d, < 0.1 ms 또는 조정값)
- C(T3) 감지 안정성 평가 결과

**English**

- Comparison table of Rate error and Beat Error per detection method (mean, std dev, vs WeiShi)
- Selected T1 detection method with rationale document
- Finalized Measurement Accuracy QA-2 values (< 5 s/d, < 0.1 ms, or adjusted)
- C(T3) detection stability assessment

---

### 필요 리소스 / Resources Required

**한국어**

- 하드웨어: 기계식 시계 (알려진 Rate), WeiShi No.1000 레퍼런스 디바이스, USB 오디오 인터페이스
- 소프트웨어: TimeGrapher_v10.5 Playback 모드, 분석 스크립트 (Python/C++ 중 택일)
- 데이터: 동일 시계의 PCM 녹음 파일 (최소 5분, 노이즈 있는 환경 / 없는 환경 각 1개)
- 노력: 약 1 person-day

**English**

- Hardware: mechanical watch (known rate), WeiShi No.1000 reference device, USB audio interface
- Software: TimeGrapher_v10.5 Playback mode, analysis script (Python or C++)
- Data: PCM recording of the same watch (minimum 5 minutes, one with ambient noise, one without)
- Effort: ~1 person-day

---

### 실험 절차 / Experiment Description

**한국어**

1. 기계식 시계를 USB 마이크로 5분간 녹음 (동시에 WeiShi No.1000으로 레퍼런스 Rate 기록)
2. 녹음된 PCM을 Playback 모드로 재생하여 두 가지 방식으로 T1 감지:
   - (A) onset 방식: 신호 포락선이 임계값을 처음 초과하는 시점
   - (B) peak 방식: 신호 포락선의 최대값 시점
3. 각 방식으로 계산된 Rate 및 Beat Error를 WeiShi No.1000 기준값과 비교
4. 평균 오차, 표준편차, 이상값 발생 빈도 측정
5. 노이즈 환경 녹음에 동일 실험 반복 (환경 노이즈 영향도 평가)
6. C(T3) 감지 안정성 별도 평가: T3 감지 실패율 및 오감지율 측정
7. 결과 기반 T1 감지 방식 결정 및 Measurement Accuracy QA 수치 확정

**English**

1. Record the mechanical watch via USB microphone for 5 minutes (simultaneously record reference Rate using WeiShi No.1000)
2. Play back the PCM in Playback mode and detect T1 using two methods:
   - (A) Onset: the moment the signal envelope first exceeds a threshold
   - (B) Peak: the moment of the maximum signal envelope value
3. Compare Rate and Beat Error calculated by each method against the WeiShi No.1000 reference
4. Measure mean error, standard deviation, and outlier frequency
5. Repeat the experiment with the noisy environment recording (assess ambient noise impact)
6. Separately evaluate C(T3) detection stability: measure T3 miss rate and false-positive rate
7. Decide T1 detection method based on results and finalize Measurement Accuracy QA values

---

### 기간 / Duration

**한국어**

- 계획 완료: 2026-06-09 (M1 제출)
- 실험 실행: 2026-06-11 ~ 2026-06-17 (WeiShi 디바이스 접근 가능 시점 기준)
- 결과 보고: 2026-06-18

**English**

- Plan finalized: 2026-06-09 (M1 submission)
- Experiment execution: 2026-06-11 ~ 2026-06-17 (contingent on WeiShi device access)
- Results reported: 2026-06-18

---

### 참고 자료 / Links and References

- `architectural-drivers.md` QA-2 (Measurement Accuracy), Section 6 (Confirmed Values)
- `risk-assessment-revision.md` TR-01, OI-01
- `TimeGrapher Equations_v0.docx.pdf` — Rate / Amplitude / Beat Error 계산 공식
- `Witschi-Training-Course.pdf` pp.14-19 — Escapement 원리 및 T1/T3 이벤트 정의

---

## EX-04: AGC 비활성화 지속성 검증 / AGC Disable Persistence Verification

### 결과 및 권고 / Results and Recommendations

**한국어**
*(M2에서 작성)*

**English**
*(To be filled at M2)*

---

### 목적 / Objective

**한국어**

AlsaMixer에서 AGC(Auto Gain Control)를 비활성화한 설정이 RPi 5 재부팅 후에도 유지되는가? 또한 AGC 활성화 상태와 비활성화 상태 사이의 신호 품질 차이는 측정값에 얼마나 영향을 미치는가?

AGC가 활성화되면 신호 진폭이 자동 조정되어 Beat Event 타이밍이 왜곡된다. 이는 모든 측정값을 신뢰 불가 상태로 만드는 Design Constraint 사항이다. 이 실험이 실패하면 소프트웨어적 보완책(앱 시작 시 AGC 경고 진단)이 필수가 된다.

해결 대상 리스크: **TR-02** (AGC 재활성화로 신호 왜곡), **OI-02** (AGC 비활성화 지속 여부 미확인)

**English**

Does the AGC (Auto Gain Control) disable setting in AlsaMixer persist across Raspberry Pi 5 reboots? And how much does the difference in signal quality between AGC-on and AGC-off states affect measurement values?

When AGC is active, it automatically adjusts signal amplitude, distorting Beat Event timing and making all measurements unreliable. This is a Design Constraint item. If this experiment fails, a software-level mitigation (AGC warning diagnostic at app startup) becomes mandatory.

Risks addressed: **TR-02** (AGC re-activation distorts signal), **OI-02** (AGC disable persistence unconfirmed)

---

### 상태 / Status

**[ ] Planned**

---

### 예상 산출물 / Expected Outcomes

**한국어**

- 재부팅 후 AGC 비활성화 지속 여부 확인 결과 (지속됨 / 지속 안 됨)
- 지속되지 않을 경우: 부팅 시 자동 적용 스크립트 (`asound.state` 또는 systemd 서비스)
- AGC ON vs OFF 상태에서의 Rate 오차 비교값
- 앱 시작 시 AGC 활성화 여부 진단 기능 필요 여부 결정

**English**

- Verification of whether AGC disable persists after reboot (persists / does not persist)
- If not persistent: auto-apply boot script (`asound.state` or systemd service)
- Rate error comparison between AGC ON and AGC OFF states
- Decision on whether AGC diagnostic warning at app startup is required

---

### 필요 리소스 / Resources Required

**한국어**

- 하드웨어: Raspberry Pi 5, USB 오디오 인터페이스, 기계식 시계
- 소프트웨어: AlsaMixer, `alsactl`, TimeGrapher_v10.5
- 노력: 약 0.5 person-day

**English**

- Hardware: Raspberry Pi 5, USB audio interface, mechanical watch
- Software: AlsaMixer, `alsactl`, TimeGrapher_v10.5
- Effort: ~0.5 person-day

---

### 실험 절차 / Experiment Description

**한국어**

1. AlsaMixer에서 AGC 비활성화 후 `alsactl store`로 설정 저장
2. RPi 5 재부팅 후 `amixer` 명령으로 AGC 상태 확인 → 지속 여부 기록
3. 지속되지 않을 경우: `alsactl restore`를 systemd `rc-local` 서비스에 등록
4. 동일 시계 PCM을 AGC ON / AGC OFF 두 조건에서 각 2분간 녹음
5. 두 녹음에서 Rate 및 Beat Error 계산 후 비교 → AGC 영향도 수치화
6. 앱 시작 시 AGC 상태 자동 감지 및 경고 출력 기능 구현 여부 결정

**English**

1. Disable AGC in AlsaMixer and save the configuration with `alsactl store`
2. Reboot RPi 5 and check AGC state with `amixer` → record whether setting persisted
3. If not persistent: register `alsactl restore` as a systemd `rc-local` service
4. Record the same watch PCM for 2 minutes each under AGC ON and AGC OFF conditions
5. Calculate Rate and Beat Error from both recordings and compare → quantify AGC impact
6. Decide whether to implement automatic AGC state detection and warning at app startup

---

### 기간 / Duration

**한국어**

- 계획 완료: 2026-06-09 (M1 제출)
- 실험 실행: 2026-06-09 ~ 2026-06-11 (Week 2 초반, 하드웨어 셋업 단계)
- 결과 보고: 2026-06-12

**English**

- Plan finalized: 2026-06-09 (M1 submission)
- Experiment execution: 2026-06-09 ~ 2026-06-11 (early Week 2, hardware setup phase)
- Results reported: 2026-06-12

---

### 참고 자료 / Links and References

- `architectural-drivers.md` Section 7 (Design Constraints — AGC)
- `risk-assessment-revision.md` TR-02, OI-02
- ALSA state persistence: `man alsactl`

---

## EX-05: 신호 필터 파라미터 최적화 / Signal Filter Parameter Optimization

### 결과 및 권고 / Results and Recommendations

**한국어**
*(M2에서 작성)*

**English**
*(To be filled at M2)*

---

### 목적 / Objective

**한국어**

시계 비트 이벤트(T1, T3)를 가장 잘 보존하면서 주변 노이즈를 효과적으로 제거하는 HPF 및 LPF 컷오프 주파수 조합은 무엇인가?

현재 베이스코드(`Dsp.cpp`)에는 HPF만 구현되어 있다. 이 실험은:
1. LPF 구현 필요 여부와 최적 컷오프 값을 결정한다
2. EX-03(비트 감지 정확도)을 위한 전처리 파라미터를 확정한다

해결 대상 리스크: **TR-01** (T1/T3 감지 부정확 — 노이즈 환경에서 악화)

**English**

What combination of HPF and LPF cutoff frequencies best preserves watch beat events (T1, T3) while effectively rejecting ambient noise?

The current base code (`Dsp.cpp`) implements only an HPF. This experiment:
1. Determines whether LPF implementation is needed and what the optimal cutoff value is
2. Finalizes the preprocessing parameters for EX-03 (beat detection accuracy)

Risk addressed: **TR-01** (T1/T3 detection inaccurate — worsens in noisy environments)

---

### 상태 / Status

**[ ] Planned**

---

### 예상 산출물 / Expected Outcomes

**한국어**

- HPF/LPF 컷오프 조합별 T1/T3 감지 SNR 비교 표
- 권장 HPF 컷오프 값 (현재 설정 확인 또는 조정)
- LPF 구현 필요 여부 결정 및 권장 컷오프 값
- EX-03에 사용할 전처리 파라미터 확정

**English**

- Comparison table of T1/T3 detection SNR per HPF/LPF cutoff combination
- Recommended HPF cutoff value (confirm current setting or adjust)
- Decision on whether LPF implementation is needed and recommended cutoff value
- Finalized preprocessing parameters for use in EX-03

---

### 필요 리소스 / Resources Required

**한국어**

- 소프트웨어: TimeGrapher_v10.5, Python 분석 스크립트 (scipy.signal), 오디오 분석 도구 (Audacity 등)
- 데이터: 노이즈 환경 포함 시계 PCM 녹음 파일 (EX-03과 공유 가능)
- 노력: 약 0.5 person-day

**English**

- Software: TimeGrapher_v10.5, Python analysis script (scipy.signal), audio analysis tool (Audacity, etc.)
- Data: watch PCM recordings including ambient noise (can be shared with EX-03)
- Effort: ~0.5 person-day

---

### 실험 절차 / Experiment Description

**한국어**

1. 시계 PCM 녹음 파일에 HPF 컷오프를 100 / 200 / 400 / 800 Hz 구간으로 스윕 적용
2. LPF 컷오프를 4k / 8k / 16k Hz 구간으로 스윕 적용
3. 각 조합에서 T1/T3 이벤트 감지 SNR 계산 (신호 대 노이즈 비율)
4. 노이즈 환경 녹음에 동일 스윕 반복
5. SNR이 가장 높고 T1/T3 감지 실패율이 가장 낮은 조합 선택
6. EX-03 실험 전 베이스코드 `Dsp.cpp`에 선택된 파라미터 적용

**English**

1. Apply HPF cutoff sweep (100 / 200 / 400 / 800 Hz) to watch PCM recording
2. Apply LPF cutoff sweep (4k / 8k / 16k Hz)
3. Calculate T1/T3 detection SNR (signal-to-noise ratio) for each combination
4. Repeat the same sweep on noisy environment recording
5. Select the combination with the highest SNR and lowest T1/T3 miss rate
6. Apply selected parameters to `Dsp.cpp` in the base code before running EX-03

---

### 기간 / Duration

**한국어**

- 계획 완료: 2026-06-09 (M1 제출)
- 실험 실행: 2026-06-10 ~ 2026-06-12 (EX-03 선행 실험)
- 결과 보고: 2026-06-13

**English**

- Plan finalized: 2026-06-09 (M1 submission)
- Experiment execution: 2026-06-10 ~ 2026-06-12 (prerequisite for EX-03)
- Results reported: 2026-06-13

---

### 참고 자료 / Links and References

- `architectural-drivers.md` QA-2 (Measurement Accuracy)
- `risk-assessment-revision.md` TR-01
- `TimeGrapher Equations_v0.docx.pdf` — T1/T3 이벤트 정의
- scipy.signal filter design: [https://docs.scipy.org/doc/scipy/reference/signal.html](https://docs.scipy.org/doc/scipy/reference/signal.html)

---

## EX-06: 빌드 환경 검증 / Build Environment Verification

### 결과 및 권고 / Results and Recommendations

**한국어**
*(M2에서 작성)*

**English**
*(To be filled at M2)*

---

### 목적 / Objective

**한국어**

Qt 프로젝트를 macOS/Windows에서 크로스 컴파일하여 RPi에 배포할 수 있는가? 아니면 RPi에서 네이티브 빌드가 유일한 경로인가? 또한 macOS 팀원이 `Q_OS_MAC` 분기 처리 없이 오디오 기능을 제외한 나머지 기능을 로컬에서 개발할 수 있는가?

이 실험은 팀 전체의 개발 환경 전략을 확정한다. macOS 팀원 2명이 로컬에서 빌드·테스트 불가 상태이면 개발 속도에 직접 영향을 준다.

해결 대상 리스크: **TR-10** (PC-RPi 빌드 환경 불일치), **OI-05** (macOS `Q_OS_MAC` 처리 방안 미결정)

**English**

Can the Qt project be cross-compiled on macOS/Windows and deployed to RPi, or is native build on RPi the only viable path? Additionally, can macOS team members develop non-audio features locally without `Q_OS_MAC` branch handling?

This experiment finalizes the development environment strategy for the entire team. If 2 macOS team members cannot build and test locally, it directly impacts development velocity.

Risks addressed: **TR-10** (PC-RPi build environment mismatch), **OI-05** (macOS `Q_OS_MAC` handling undecided)

---

### 상태 / Status

**[ ] Planned**

---

### 예상 산출물 / Expected Outcomes

**한국어**

- 빌드 경로 결정: 크로스 컴파일 가능 / RPi 네이티브 빌드 전용
- macOS 빌드 성공 여부 (`Q_OS_MAC` 스텁 또는 분기 처리 후)
- 팀 개발 환경 가이드 문서 (SSH 접속, 빌드 명령, 배포 절차)
- macOS 팀원이 오디오 비활성화 상태로 DSP/UI 개발 가능한지 여부 확인

**English**

- Build path decision: cross-compilation viable / RPi native build only
- macOS build success (after `Q_OS_MAC` stub or branch handling)
- Team development environment guide document (SSH access, build commands, deploy procedure)
- Confirmation of whether macOS team members can develop DSP/UI with audio disabled

---

### 필요 리소스 / Resources Required

**한국어**

- 하드웨어: macOS 개발 머신, Raspberry Pi 5, SSH 접속 환경
- 소프트웨어: Qt Creator, Qt for RPi 크로스 컴파일 툴체인 (시도), TimeGrapher_v10.5
- 노력: 약 0.5 person-day

**English**

- Hardware: macOS development machine, Raspberry Pi 5, SSH access
- Software: Qt Creator, Qt for RPi cross-compilation toolchain (attempt), TimeGrapher_v10.5
- Effort: ~0.5 person-day

---

### 실험 절차 / Experiment Description

**한국어**

1. macOS에서 `Q_OS_MAC` 분기 없는 현재 상태로 빌드 시도 → 실패 지점 기록
2. `MacAudio.cpp/h` 스텁 파일 추가 및 `Q_OS_MAC` 분기 처리 적용 후 재빌드
3. macOS에서 오디오 비활성화 상태로 DSP·UI 기능 정상 동작 여부 확인
4. RPi에서 SSH를 통한 네이티브 빌드 시도: `qmake` → `make` 실행 시간 측정
5. (선택) Qt 크로스 컴파일 툴체인 설정 시도: 성공 여부 및 복잡도 평가
6. 팀 표준 개발 워크플로우 결정 및 문서화

**English**

1. Attempt to build on macOS without `Q_OS_MAC` branch → record failure points
2. Add `MacAudio.cpp/h` stub files and apply `Q_OS_MAC` branch handling, then rebuild
3. Verify that DSP/UI features work correctly on macOS with audio disabled
4. Attempt native build on RPi via SSH: measure `qmake` → `make` execution time
5. (Optional) Attempt Qt cross-compilation toolchain setup: evaluate success and complexity
6. Decide and document the standard team development workflow

---

### 기간 / Duration

**한국어**

- 계획 완료: 2026-06-09 (M1 제출)
- 실험 실행: 2026-06-09 ~ 2026-06-11 (Week 2 초반, 개발 환경 셋업 단계)
- 결과 보고: 2026-06-12

**English**

- Plan finalized: 2026-06-09 (M1 submission)
- Experiment execution: 2026-06-09 ~ 2026-06-11 (early Week 2, dev environment setup phase)
- Results reported: 2026-06-12

---

### 참고 자료 / Links and References

- `architectural-drivers.md` Section 7 (Design Constraints — Platform, Language)
- `risk-assessment-revision.md` TR-10, TR-08, OI-05
- `TimeGrapher GUI Set Up Instructions.pdf` — Qt 환경 셋업 가이드
- Qt cross-compilation for RPi: [https://wiki.qt.io/Cross-Compile_Qt_6_for_Raspberry_Pi](https://wiki.qt.io/Cross-Compile_Qt_6_for_Raspberry_Pi)

---

## 실험 전체 요약 / Experiment Status Summary

| ID | 제목 / Title | 해결 대상 / Resolves | 실행 시기 / When | M2 결과 / M2 Result |
|----|-------------|---------------------|-----------------|---------------------|
| EX-01 | RPi 5 오디오 성능 및 캡처 지연 / RPi 5 Audio Performance & Capture Latency | TR-04, OI-03 | Week 2 (06-09~13) | [ ] |
| EX-02 | Qt 렌더링 성능 및 표시 지연 / Qt Rendering Performance & Display Latency | TR-05, OI-03 | Week 2 (06-10~15) | [ ] |
| EX-03 | 비트 감지 방식 비교 및 측정 정확도 / Beat Detection Method Comparison & Accuracy | TR-01, OI-01 | Week 2~3 (06-11~17) | [ ] |
| EX-04 | AGC 비활성화 지속성 검증 / AGC Disable Persistence Verification | TR-02, OI-02 | Week 2 (06-09~11) | [ ] |
| EX-05 | 신호 필터 파라미터 최적화 / Signal Filter Parameter Optimization | TR-01 | Week 2 (06-10~12) | [ ] |
| EX-06 | 빌드 환경 검증 / Build Environment Verification | TR-10, OI-05 | Week 2 (06-09~11) | [ ] |

### QA 수치 확정 매핑 / QA Value Confirmation Mapping

| QA 수치 / QA Value | 확정 실험 / Confirming Experiment | 현재 잠정값 / Current Provisional |
|-------------------|----------------------------------|----------------------------------|
| Low Latency ① capture→process | EX-01 | < 46 ms |
| Low Latency ② process→display | EX-02 | < 20 ms |
| Low Latency ③ end-to-end | EX-01 + EX-02 합산 | < 66 ms |
| Measurement Accuracy — Rate | EX-03 | < 5 s/d |
| Measurement Accuracy — Beat Error | EX-03 | < 0.1 ms |

---

## 리뷰 체크리스트 / Review Checklist

- [ ] 각 실험이 템플릿 형식을 따르고 있는가 / Each experiment follows the template format
- [ ] 각 실험이 명확하게 risk/OI와 연결되어 있는가 / Each experiment is clearly linked to a risk/OI
- [ ] 완료 기준(Expected Outcomes)이 측정 가능한가 / Completion criteria are measurable
- [ ] 실험 일정이 M1→M2 기간(06-09~06-22) 내에 실현 가능한가 / Experiments are feasible within M1→M2 period
- [ ] EX-01+EX-02가 Low Latency QA 수치를 확정하는가 / EX-01+EX-02 confirm Low Latency QA values
- [ ] EX-03이 Measurement Accuracy QA 수치를 확정하는가 / EX-03 confirms Measurement Accuracy QA values
- [ ] OI-01~OI-03, OI-05가 모두 커버되는가 / OI-01~OI-03, OI-05 are all covered
