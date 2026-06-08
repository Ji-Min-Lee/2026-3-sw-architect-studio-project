# 리스크 평가 / Risk Assessment — TimeGrapher

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **작성일 / Date**: 2026-06-07

---

## 0. 이 문서의 구조 / Document Structure

**한국어**

이 문서는 아래 세 가지 질문에 순서대로 답한다.

1. 기술적·비기술적 리스크는 무엇이고, 확률(Probability)·영향(Impact)은 어떻게 평가하는가?
2. 미결 질문/이슈가 프로젝트 결과에 영향을 주는 사안과 명확히 연결되어 있는가?
3. 미결 질문/이슈를 해소하기 위한 액션이 도출되어 있는가?

**데이터 소스**: 모든 리스크는 아래 QA 분석에서 직접 도출했다.

| 파일 | 도출 리스크 |
|------|-----------|
| `docs/milestone1/m1-qa-redefine-performance.md` | TR-01, TR-02 |
| `docs/milestone1/quality_attribute_scenarios_latency.md` | TR-03, TR-04 |
| `docs/milestone1/correctness-analysis.md` | TR-05 |
| `docs/milestone1/quality_attribute_scenarios_extensibility.md` | TR-06, TR-07, TR-08 |
| `docs/milestone1/quality_attribute_scenarios_usability.md` | TR-09, NTR-03 |
| `docs/milestone1/final/architectural-drivers.md` | NTR-01, NTR-02 (종합) |

**English**

This document answers three questions in order:

1. What are the technical and non-technical risks, and how are Probability and Impact rated?
2. Are open questions/issues clearly linked to things that will affect the project outcome?
3. Have actions been identified to address the open questions/issues?

**Data sources**: All risks are derived directly from the QA analyses listed above.

---

## 1. 리스크 평가 기준 / Risk Assessment Criteria

**한국어**

리스크는 Probability(발생 가능성)와 Impact(영향) 두 축으로 H / M / L 3단계 평가한다.

| 수준 / Level | Probability 기준 | Impact 기준 |
|:-----------:|----------------|-----------|
| **H (High)** | 실험 없이는 달성 보장 불가 / 이미 사전 지식으로 발생 예상 | QA 목표 달성 불가 → 팀 제1목표(정확한 측정) 또는 납기 직접 위협 |
| **M (Medium)** | 설계 결정으로 완화 가능하지만 실험 결과에 따라 악화 가능 | 일부 QA 목표 저하 / 개발 속도 저하 |
| **L (Low)** | 알려진 완화책이 존재하거나 발생 가능성이 낮음 | 부분 기능 미달 또는 특정 환경에서만 영향 |

**Overall Risk = max(Probability, Impact)** (어느 한쪽이 H이면 Overall H)

**English**

Each risk is rated on Probability and Impact on a three-point scale: H / M / L.

Overall Risk = max(Probability, Impact). If either axis is H, the risk is rated H overall.

---

## 2. 기술적 리스크 / Technical Risks

### 2.1 리스크 요약 / Technical Risk Summary

**한국어**

| ID | 리스크 / Risk | Prob | Impact | Overall | 연결 QA / Linked QA | 미결 이슈 / Open Issue |
|----|-------------|:----:|:------:|:-------:|:-------------------:|:--------------------:|
| TR-01 | RPi 5가 96k sps에서 Dropped Block = 0 달성 불가 | H | H | **H** | QAS-1 | OI-P1 |
| TR-02 | Linux 스케줄러 지터로 인한 간헐적 Dropped Block | M | H | **H** | QAS-1 | — |
| TR-03 | end-to-end 지연이 beat 주기 초과 (RPi + Qt 미검증) | H | H | **H** | QAS-2 | OI-L1, OI-L2 |
| TR-04 | 11탭 동시 렌더링 시 ② process→display > 30 ms | M | M | **M** | QAS-2 | OI-L2 |
| TR-05 | Detector 기본값 파라미터가 소음 환경에서 동작 불가 | M | M | **M** | QAS-3 | OI-C1 |
| TR-06 | Observer 패턴 리팩터링 후 잔존 결합으로 ≤ 3파일 목표 미달 | M | M | **M** | QAS-5 | — |
| TR-07 | God Object 분리 중 기존 그래프 기능 회귀 | M | H | **H** | QAS-5 | — |
| TR-08 | 신규 그래프가 MeasurementEngine 미제공 데이터 요구 | L | M | **M** | QAS-5 | — |
| TR-09 | 신호 품질 경고 임계값이 실제 환경에서 맞지 않음 | M | L | **M** | QAS-4 | OI-U1, OI-U2 |

**English**

| ID | Risk | Prob | Impact | Overall | Linked QA | Open Issue |
|----|------|:----:|:------:|:-------:|:---------:|:----------:|
| TR-01 | RPi 5 cannot achieve Dropped Block = 0 at 96k sps | H | H | **H** | QAS-1 | OI-P1 |
| TR-02 | Linux scheduler jitter causes intermittent Dropped Blocks | M | H | **H** | QAS-1 | — |
| TR-03 | End-to-end latency exceeds beat period (RPi + Qt unverified) | H | H | **H** | QAS-2 | OI-L1, OI-L2 |
| TR-04 | 11-tab simultaneous rendering pushes ② process→display > 30 ms | M | M | **M** | QAS-2 | OI-L2 |
| TR-05 | Detector default parameters fail under ambient noise | M | M | **M** | QAS-3 | OI-C1 |
| TR-06 | Residual coupling after Observer refactoring violates ≤ 3-file constraint | M | M | **M** | QAS-5 | — |
| TR-07 | Regression in existing graphs during God Object decomposition | M | H | **H** | QAS-5 | — |
| TR-08 | New graph requires data not currently provided by MeasurementEngine | L | M | **M** | QAS-5 | — |
| TR-09 | Signal quality warning thresholds mismatched to real environment | M | L | **M** | QAS-4 | OI-U1, OI-U2 |

---

### TR-01 — RPi 5가 96k sps에서 Dropped Block = 0 달성 불가

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | DSP 파이프라인(필터링 → T1/T3 감지 → Rate·Amplitude·Beat Error 계산)이 96k sps 블록 주기(~10 ms)를 초과하면 Ring Buffer가 오버플로되어 Dropped Block 발생 |
| **Probability: H** | RPi 5에서 Qt GUI + DSP 동시 실행 시 96k sps 처리 가능 여부가 실험 전까지 검증되지 않음 |
| **Impact: H** | Dropped Block 발생 → T1/T3 타임스탬프 소실 → Rate·Amplitude·Beat Error 계산 불가 → **팀 제1목표 완전 붕괴**. 모든 QA의 선행 조건 |
| **완화 전술** | ① Graceful Degradation: 96k sps 달성 불가 시 48k sps 자동 폴백 (Dropped Block = 0 보장, 분해능은 낮아짐) ② Lock-Free Ring Buffer: mutex 제거로 DSP 지연 방지 |
| **잔존 리스크** | 48k sps 폴백 시 T1 감지 분해능이 20.8 µs/sample으로 저하 — 36,000 / 43,200 BPH 지원 불가 |
| **연결 이슈** | OI-P1 → **EXP-01**로 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | If the DSP pipeline exceeds the 96k sps block period (~10 ms), Ring Buffer overflows, causing Dropped Blocks |
| **Probability: H** | Whether RPi 5 can sustain 96k sps while running Qt GUI + DSP concurrently is unverified before experiment |
| **Impact: H** | Dropped Block → T1/T3 timestamp lost → Rate·Amplitude·Beat Error computation impossible → **team primary goal collapses**. Prerequisite for all QAs |
| **Mitigation tactics** | ① Graceful Degradation: auto-fallback to 48k sps if 96k is unachievable (guarantees Dropped Block = 0, lower resolution) ② Lock-Free Ring Buffer: eliminates mutex to prevent DSP delays |
| **Residual risk** | At 48k sps fallback, T1 detection resolution degrades to 20.8 µs/sample — 36,000 / 43,200 BPH support becomes infeasible |
| **Linked issue** | OI-P1 → resolved by **EXP-01** |

---

### TR-02 — Linux 스케줄러 지터로 인한 간헐적 Dropped Block

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | Linux 범용 스케줄러가 오디오 스레드 우선순위를 낮게 취급하여 간헐적으로 블록 주기를 초과 → 산발적 Dropped Block → 장기 Rate 측정 오차 누적 |
| **Probability: M** | Linux에서 범용 스케줄러의 RT 부적합성은 알려진 사실. 오디오 스레드 우선순위 설정이 기본값이면 발생 가능 |
| **Impact: H** | 산발적 블록 손실도 장기 Rate 측정의 누적 오차를 유발 → 정확도 저하. 재현이 어려워 디버깅 비용도 높음 |
| **완화 전술** | Priority Scheduling: 오디오 처리 스레드 우선순위 상향 (`SCHED_RR` 또는 `SCHED_FIFO`) → Linux 스케줄러 지터 흡수 |
| **잔존 리스크** | RPi 환경에서 우선순위 설정 효과는 EXP-01로 함께 검증 필요 |
| **연결 이슈** | EXP-01 결과에서 간헐적 Dropped Block 패턴으로 간접 확인 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Linux general-purpose scheduler treats audio thread priority low, causing intermittent block period violations → sporadic Dropped Blocks → accumulated long-term Rate measurement error |
| **Probability: M** | Known characteristic of Linux general-purpose scheduler; occurs if audio thread priority is left at default |
| **Impact: H** | Sporadic block loss accumulates Rate measurement error over time → accuracy degradation; difficult to reproduce, increasing debugging cost |
| **Mitigation tactic** | Priority Scheduling: elevate audio thread priority (`SCHED_RR` or `SCHED_FIFO`) to absorb Linux scheduler jitter |
| **Residual risk** | Effectiveness on RPi must be co-verified through EXP-01 |
| **Linked issue** | Indirectly confirmed from sporadic Dropped Block patterns in EXP-01 results |

---

### TR-03 — end-to-end 지연이 beat 주기 초과 (RPi + Qt 미검증)

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | 28,800 BPH 기준 beat 주기 125 ms의 80%인 100 ms 이내에 오디오 캡처 → DSP → Qt GUI 렌더링 전 과정을 완료하지 못하면 실시간 표시 기능 붕괴 |
| **Probability: H** | RPi 5 + Qt6 환경에서 ① capture→process (< 70 ms) + ② process→display (< 30 ms) 동시 달성 여부가 EXP-02 실측 전까지 미검증. QAudioSource 라이브 캡처 콜백 주기 자체가 미확정 |
| **Impact: H** | 지연 초과 → 이전 beat 표시 완료 전에 다음 beat 처리 시작 → 실시간 표시 기능 붕괴 → T1/T3 타임스탬프 정확도 간접 영향 |
| **완화 전술** | ① Introduce Concurrency (Audio/DSP/GUI 스레드 분리) ② Lock-Free Ring Buffer ③ Lazy Rendering (현재 탭만 렌더링) ④ Reduce Computational Overhead |
| **연결 이슈** | OI-L1 (콜백 주기 미확정), OI-L2 (11탭 렌더링 부하) → **EXP-02**로 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | If the full pipeline (audio capture → DSP → Qt GUI rendering) cannot complete within 100 ms (80% of 125 ms beat period at 28,800 BPH), real-time display fails |
| **Probability: H** | Whether ① capture→process (< 70 ms) + ② process→display (< 30 ms) can both be met on RPi 5 + Qt6 is unverified before EXP-02; QAudioSource live callback period itself is unconfirmed |
| **Impact: H** | Latency violation → next beat processing starts before previous beat display completes → real-time display breakdown → indirect effect on T1/T3 timestamp accuracy |
| **Mitigation tactics** | ① Introduce Concurrency (Audio/DSP/GUI thread separation) ② Lock-Free Ring Buffer ③ Lazy Rendering (render active tab only) ④ Reduce Computational Overhead |
| **Linked issues** | OI-L1 (callback period unknown), OI-L2 (11-tab rendering overhead) → resolved by **EXP-02** |

---

### TR-04 — 11탭 동시 렌더링 시 ② process→display > 30 ms

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | 11개 그래프 탭이 완성된 이후 Qt 메인 스레드가 동시 렌더링 부하를 감당하지 못해 ② process→display 구간이 30 ms를 초과할 수 있음 |
| **Probability: M** | 1~2개 탭 기준으로는 문제없을 수 있으나, 11탭 완성 후 부하가 누적될 경우 발생 가능. 현재 코드에서 미검증 |
| **Impact: M** | end-to-end 지연 초과로 이어지면 TR-03과 동일 결과. 그러나 Lazy Rendering 전술로 비활성 탭 렌더링을 생략하면 완화 가능 |
| **완화 전술** | Lazy Rendering: 현재 활성 탭만 `paintEvent()` 실행, 비활성 탭 데이터만 업데이트하고 렌더링 보류 |
| **연결 이슈** | OI-L2 → EXP-02에서 탭 구성 2가지(1탭/11탭) 비교 측정으로 확인 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Once 11 graph tabs are complete, Qt main thread rendering load may cause ② process→display to exceed 30 ms |
| **Probability: M** | 1-2 tabs may be fine; accumulated load after 11 tabs is plausible but unverified |
| **Impact: M** | If it causes end-to-end violation → same outcome as TR-03; however, Lazy Rendering can mitigate it |
| **Mitigation tactic** | Lazy Rendering: execute `paintEvent()` only for the active tab; update data for inactive tabs but defer rendering |
| **Linked issue** | OI-L2 → confirmed in EXP-02 by comparing 1-tab vs. 11-tab configurations |

---

### TR-05 — Detector 기본값 파라미터가 소음 환경에서 동작 불가

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | `Detector.cpp`의 `onset_fraction` (0.03) / `min_peak_fraction` (0.20) 기본값이 ambient noise가 있는 환경에서 beat 감지 품질 저하 또는 오감지 유발 |
| **Probability: M** | adaptive threshold 알고리즘은 구현되어 있으나, 기본 파라미터가 3가지 소음 조건(저/중/고)에서 유효한지 실험으로 미검증 |
| **Impact: M** | Δ Rate / Δ Amplitude / Δ Beat Error 증가 → 팀 제1목표(정확한 측정) 부분 저하. 완전 측정 불가까지는 아니나 신뢰성 하락 |
| **완화 전술** | Adaptive threshold (noise floor = 최근 256 ms 75th percentile, reference_peak = 최근 16 beat median) + EXP-03로 최적 파라미터 확정 |
| **연결 이슈** | OI-C1 → **EXP-03** (소음 3조건 × Detector 파라미터 조합 실험)로 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Default `onset_fraction` (0.03) / `min_peak_fraction` (0.20) in `Detector.cpp` may degrade beat detection quality under ambient noise |
| **Probability: M** | Adaptive threshold algorithm is implemented, but whether default parameters hold under 3 noise conditions (low/medium/high) is experimentally unverified |
| **Impact: M** | Δ Rate / Δ Amplitude / Δ Beat Error increases → partial degradation of team primary goal; reliability decreases but measurement does not fail completely |
| **Mitigation tactic** | Adaptive threshold (noise floor = 75th percentile of last 256 ms, reference_peak = median of last 16 beats) + confirm optimal parameters via EXP-03 |
| **Linked issue** | OI-C1 → resolved by **EXP-03** (3 noise conditions × Detector parameter combinations) |

---

### TR-06 — Observer 패턴 리팩터링 후 잔존 결합으로 ≤ 3파일 목표 미달

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | God Object 구조를 Observer 패턴(Signal-Slot)으로 리팩터링한 후에도 Presentation Layer와 Signal Processing / Acquisition 레이어 간 잔존 결합이 남아, 그래프 추가 시 변경 파일 수가 ≤ 3개를 초과하는 경우 |
| **Probability: M** | 완전한 레이어 분리는 기존 코드 구조에 대한 깊은 이해가 필요. 리팩터링 과정에서 부분적 결합이 남을 가능성 있음 |
| **Impact: M** | ≤ 3파일 목표 미달 → 개발 후반부로 갈수록 그래프 추가 비용 증가, 병렬 개발 일정 리스크 현실화 |
| **완화 전술** | Restrict Dependencies (Layered Architecture): Presentation Layer는 Domain Layer(MeasurementEngine 인터페이스)만 참조 가능 규칙 강제 |
| **연결 이슈** | Observer 패턴 리팩터링 완료 후 ≤ 3파일 검증 실험으로 직접 확인 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Residual coupling between Presentation Layer and Signal Processing / Acquisition layers persists after Observer pattern refactoring, causing new graph additions to touch more than 3 files |
| **Probability: M** | Full layer separation requires deep understanding of existing code; partial coupling likely to remain during refactoring |
| **Impact: M** | ≤ 3-file constraint violated → graph addition cost grows in later development weeks, parallel development schedule risk materializes |
| **Mitigation tactic** | Restrict Dependencies (Layered Architecture): enforce rule that Presentation Layer may only reference Domain Layer (MeasurementEngine interface) |
| **Linked issue** | Directly verified by ≤ 3-file experiment after Observer pattern refactoring is complete |

---

### TR-07 — God Object 분리 중 기존 그래프 기능 회귀

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | God Object 구조를 레이어 분리 / Signal-Slot 구독 방식으로 전환하는 과정에서 기존 그래프(Trace, Beat Error 등)의 동작에 사이드이펙트 발생 |
| **Probability: M** | 대규모 구조 변경에 수반되는 일반적 리스크. 기존 코드베이스(`TimeGrapher_v10.5`)에 자동화된 회귀 테스트가 없어 감지 어려움 |
| **Impact: H** | 회귀 발생 시 롤백 → 기존 God Object 구조 복귀 → Extensibility 목표 달성 불가 → QAS-5 포기 → 11그래프 병렬 개발 일정 전체 붕괴 |
| **완화 전술** | ① 증분 리팩터링 (기능 단위 분리, 단계별 검증) ② 리팩터링 전·후 동일 시계/동일 조건에서 Rate·Amplitude·Beat Error 값 비교로 회귀 여부 수동 확인 ③ 핵심 계산 로직(Rate·Amplitude·Beat Error) 단위 테스트 작성 — 리팩터링 전 기준값 확보, 리팩터링 후 동일 테스트로 회귀 자동 감지 |
| **연결 이슈** | 별도 open issue 없음 — 리팩터링 실행 단계에서 내재된 리스크 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Side effects in existing graphs (Trace, Beat Error, etc.) during the structural transition from God Object to layered Signal-Slot subscription model |
| **Probability: M** | Common risk in large-scale structural refactoring; existing codebase (`TimeGrapher_v10.5`) lacks automated regression tests, making detection difficult |
| **Impact: H** | Regression → rollback → revert to God Object structure → QAS-5 (Extensibility) abandoned → parallel development of 11 graphs collapses entirely |
| **Mitigation tactics** | ① Incremental refactoring (decompose by feature, verify at each step) ② Manually compare Rate·Amplitude·Beat Error values on the same watch under the same conditions before and after refactoring ③ Write unit tests for core computation logic (Rate·Amplitude·Beat Error) — establish baseline before refactoring, run same tests after to automatically detect regressions |
| **Linked issue** | No separate open issue — risk is intrinsic to the refactoring execution phase |

---

### TR-08 — 신규 그래프가 MeasurementEngine 미제공 데이터 요구

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | 추가하려는 그래프(예: Spectrogram)가 MeasurementEngine이 현재 제공하지 않는 원시 데이터(raw audio)를 필요로 하여 전처리 레이어까지 수정 범위 확대 |
| **Probability: L** | 11개 그래프 중 대부분은 Rate·Amplitude·Beat Error 파생값만 필요. Spectrogram처럼 raw audio가 필요한 경우를 스코프에서 제외하면 발생 가능성 낮음 |
| **Impact: M** | ≤ 3파일 목표 달성 불가 + 전처리 레이어 수정에 따른 TR-07 회귀 리스크 병발 |
| **완화 전술** | 11개 그래프 목록을 사전에 검토하여 MeasurementEngine 제공 데이터로 충족 가능한지 확인. 미충족 시 Domain Layer 인터페이스 확장 먼저 수행 |
| **연결 이슈** | 그래프 구현 계획 확정 시 사전 검토로 대응 |

**English**

| Item | Detail |
|------|--------|
| **Description** | A new graph (e.g., Spectrogram) requires raw audio data not currently published by MeasurementEngine, expanding the modification scope into the preprocessing layer |
| **Probability: L** | Most of the 11 graphs need only Rate·Amplitude·Beat Error derivatives; excluding raw-audio-dependent graphs from scope keeps probability low |
| **Impact: M** | ≤ 3-file constraint violated + TR-07 regression risk co-occurs due to preprocessing layer modification |
| **Mitigation tactic** | Review the 11-graph list in advance to confirm whether MeasurementEngine's current output covers all data needs; expand Domain Layer interface first if not |
| **Linked issue** | Addressed by pre-implementation review when graph implementation plan is finalized |

---

### TR-09 — 신호 품질 경고 임계값이 실제 환경에서 부정확

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | `⚠ No signal` / `⚠ Noisy signal` 경고의 발생·해제 임계값(N·M 초, noise/signal 비율)이 실제 사용 환경의 ambient noise에 맞지 않아 오경보(false alarm) 또는 미감지(missed warning) 발생 |
| **Probability: M** | 임계값은 환경 의존적. 연구실과 현장 환경의 noise floor 차이가 클 수 있음 |
| **Impact: L** | 사용자 편의성 저하 — 잘못된 경고로 측정값 불신 또는 경고 무시 습관 유발. 측정 정확도에 직접 영향은 없음 (QAS-3 Correctness와 독립) |
| **완화 전술** | Heartbeat 패턴 (기존 A/C 이벤트 재활용): 별도 감지 로직 없이 N초 동안 beat 이벤트 없으면 `⚠ No signal` 표시 → 임계값 조정이 단일 파라미터로 단순화 |
| **연결 이슈** | OI-U1 (N·M 수치), OI-U2 (noise/signal 임계값) → 시계 제거/복원 실험 + 다환경 임계값 탐색 실험으로 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Warning onset/clear thresholds (N·M seconds, noise/signal ratio) for `⚠ No signal` / `⚠ Noisy signal` may not match real-environment ambient noise levels, causing false alarms or missed warnings |
| **Probability: M** | Threshold is environment-dependent; noise floor may differ significantly between lab and field environments |
| **Impact: L** | User experience degradation — false alarms undermine trust in measurements or cultivate habit of ignoring warnings; no direct effect on measurement accuracy (independent of QAS-3 Correctness) |
| **Mitigation tactic** | Heartbeat pattern (reuse existing A/C events): display `⚠ No signal` if no beat event for N seconds without additional detection logic — threshold tuning simplified to a single parameter |
| **Linked issues** | OI-U1 (N·M values), OI-U2 (noise/signal thresholds) → resolved by watch removal/restore experiment + multi-environment threshold search |

---

## 3. 비기술적 리스크 / Non-Technical Risks

### 3.1 리스크 요약 / Non-Technical Risk Summary

**한국어**

| ID | 리스크 / Risk | Prob | Impact | Overall | 연결 QA / Linked QA | 연관 이슈 |
|----|-------------|:----:|:------:|:-------:|:-------------------:|:--------:|
| NTR-01 | 5주 내 11개 그래프 병렬 구현 일정 과부하 | H | H | **H** | QAS-5 | — |
| NTR-02 | RPi 5 단일 장비 의존으로 실험 병목 | M | M | **M** | QAS-1, QAS-2 | — |
| NTR-03 | 실험 선행 조건 지연으로 잠정값 확정 불가 | M | M | **M** | QAS-1, QAS-2, QAS-3 | — |
| NTR-04 | 코딩팀–아키텍처팀 경계 불명확 — 설계가 구현에 미반영 | H | H | **H** | 전체 QA | OI-06 |
| NTR-05 | 11개 그래프 전부 구현 시 핵심 기능 품질 저하 (스코프 과잉) | H | M | **H** | QAS-5 | OI-07 |
| NTR-06 | 팀 영어 커뮤니케이션 부담 — 설계 결정 전달 누락 위험 | M | H | **H** | 전체 QA | OI-08 |

**English**

| ID | Risk | Prob | Impact | Overall | Linked QA | Open Issue |
|----|------|:----:|:------:|:-------:|:---------:|:----------:|
| NTR-01 | Schedule overload for parallel implementation of 11 graphs in 5 weeks | H | H | **H** | QAS-5 | — |
| NTR-02 | Single RPi 5 device creates experiment bottleneck | M | M | **M** | QAS-1, QAS-2 | — |
| NTR-03 | Delayed experiments prevent finalizing provisional values | M | M | **M** | QAS-1, QAS-2, QAS-3 | — |
| NTR-04 | Coding/architecture team boundary unclear — design decisions not reflected in implementation | H | H | **H** | All QAs | OI-06 |
| NTR-05 | Scope overextension — implementing all 11 graphs degrades core feature quality | H | M | **H** | QAS-5 | OI-07 |
| NTR-06 | English communication overhead — risk of design decisions not reaching all team members | M | H | **H** | All QAs | OI-08 |

---

### NTR-01 — 5주 내 11개 그래프 병렬 구현 일정 과부하

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | Week 3~4에 11개 그래프를 팀원이 분담하여 병렬 구현해야 하는 일정 구조. God Object 구조 유지 시 그래프 간 코드 충돌과 디버깅 비용이 개발 후반부로 갈수록 급증 |
| **Probability: H** | 11개 그래프 × 복잡한 의존성 = 병렬 작업 시 충돌 필연적. QAS-5 Extensibility 미달 시 발생 |
| **Impact: H** | 일정 초과 시 미완성 그래프로 데모 → 팀 납기 위협. M1 이후 진행 전체에 영향 |
| **완화** | QAS-5 Extensibility 목표 달성(≤ 3파일 구조)이 이 리스크의 직접 완화책. 그래프 추가가 독립적이면 병렬 작업 충돌이 사라짐 |
| **연결 이슈** | Architectural Drivers 팀 목표 3rd (확장 가능한 구조)와 직결. TR-06·TR-07과 연계됨 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Schedule requires 11 graphs to be divided among team members and developed in parallel during Weeks 3–4; under God Object structure, code conflicts and debugging costs escalate as development progresses |
| **Probability: H** | 11 graphs × complex dependencies = conflicts inevitable in parallel work; occurs if QAS-5 Extensibility is not met |
| **Impact: H** | Schedule overrun → incomplete graph demo → delivery threat; affects all subsequent milestones |
| **Mitigation** | Achieving QAS-5 Extensibility (≤ 3-file structure) is the direct mitigation for this risk; independent graph addition eliminates parallel work conflicts |
| **Linked issues** | Directly tied to Architectural Drivers team objective 3rd (extensible architecture); linked to TR-06 and TR-07 |

---

### NTR-02 — RPi 5 단일 장비 의존으로 실험 병목

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | EXP-01 (실시간 성능), EXP-02 (지연 측정), EXP-03 (Detector 파라미터) 모두 RPi 5에서 실행해야 하지만, 팀이 보유한 RPi 5가 한 대인 경우 실험을 순차 진행해야 하여 일정 병목 발생 |
| **Probability: M** | 실험 간 선행 조건(EXP-01 → Observer 리팩터링 완료 → EXP-02 순서 의존성 등)이 있어 완전 병렬화는 어렵지만, 장비 공유 관리로 일부 완화 가능 |
| **Impact: M** | 실험 지연 → 잠정값(⚠️ 표시)으로 설계 결정 지연 → Architectural Drivers 업데이트 지연 → 이후 마일스톤 시작 지연 |
| **완화** | ① 실험 우선순위 결정: EXP-01 (TR-01 해소) 최우선, EXP-02, EXP-03 순 ② Windows PC에서 가능한 개발(GUI 레이아웃, Observer 리팩터링)은 RPi와 독립적으로 진행 |
| **연결 이슈** | OI-P1, OI-L1, OI-L2, OI-C1 전반에 영향 |

**English**

| Item | Detail |
|------|--------|
| **Description** | EXP-01 (real-time performance), EXP-02 (latency), EXP-03 (Detector parameters) all require RPi 5; if the team owns only one device, experiments must run sequentially, creating a schedule bottleneck |
| **Probability: M** | Sequential dependencies between experiments (e.g., EXP-01 → Observer refactoring → EXP-02) make full parallelization difficult; partially mitigated through shared device scheduling |
| **Impact: M** | Experiment delays → design decisions deferred with provisional values → Architectural Drivers update delayed → subsequent milestone start delayed |
| **Mitigation** | ① Prioritize experiments: EXP-01 (resolves TR-01) first, then EXP-02, then EXP-03 ② Develop RPi-independent work (GUI layout, Observer refactoring) on Windows PC in parallel |
| **Linked issues** | Affects OI-P1, OI-L1, OI-L2, OI-C1 broadly |

---

### NTR-03 — 실험 선행 조건 지연으로 잠정값 확정 불가

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | Architectural Drivers 문서에 `⚠️ 잠정값` 으로 표시된 수치(96k sps 목표, 100 ms 지연 목표, Detector 파라미터 최적값)가 실험 지연으로 M2 진입 전까지 확정되지 않는 경우 |
| **Probability: M** | 실험 실행은 Observer 리팩터링, 하드웨어 세팅 등 선행 조건이 있어 지연 가능 |
| **Impact: M** | 잠정값 기반 설계 결정 → 실험 결과와 불일치 시 아키텍처 재작업 발생 → M2 일정 압박 |
| **완화** | EXP-01을 M1 기간 내 가장 빠르게 실행. 잠정값은 Conservative 방향(48k sps 폴백, 100 ms 상한)으로 설계하여 실험 결과와 무관하게 최소 동작 보장 |
| **연결 이슈** | OI-P1, OI-L1, OI-L2, OI-L3, OI-C1 — Architectural Drivers의 미결 이슈 섹션과 직결 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Provisional values (⚠️) in Architectural Drivers — 96k sps target, 100 ms latency target, optimal Detector parameters — are not confirmed before M2 due to experiment delays |
| **Probability: M** | Experiments depend on prerequisites (Observer refactoring, hardware setup) that can delay execution |
| **Impact: M** | Designs based on provisional values → rework required if experiments contradict assumptions → M2 schedule pressure |
| **Mitigation** | Execute EXP-01 as early as possible within M1; design conservatively (48k sps fallback, 100 ms upper bound) so minimum behavior is guaranteed regardless of experiment outcome |
| **Linked issues** | OI-P1, OI-L1, OI-L2, OI-L3, OI-C1 — directly linked to Open Issues section of Architectural Drivers |

---

### NTR-04 — 코딩팀–아키텍처팀 경계 불명확

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | 코딩팀과 아키텍처팀 간 역할 경계가 명확히 정의되지 않아, 아키텍처 결정(모듈 분리 방향, QA 전술 선택)이 실제 구현에 반영되지 않는 경우 |
| **Probability: H** | 현재 동기화 프로세스가 미확정. 두 팀이 독립적으로 작업하면 설계와 구현의 분기가 즉각 발생 |
| **Impact: H** | 설계 결정이 구현에 반영되지 않으면 M2 진입 전 전면 재작업 위험. QA 전술(Lock-Free Ring Buffer, Observer 패턴 등)이 코드에 적용되지 않으면 모든 QA 목표 미달 |
| **완화** | 매일 오후 동기화 회의 고정 + Teams 채널 소통. Project Plan에 코딩팀–아키텍처팀 역할 경계 확정 및 문서화 |
| **연결 이슈** | OI-06 → 역할 경계 확정 및 동기화 프로세스 수립으로 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Role boundary between coding team and architecture team is undefined; architecture decisions (module separation direction, QA tactic selection) may not be reflected in implementation |
| **Probability: H** | Synchronization process is currently undefined; working independently causes immediate design–implementation divergence |
| **Impact: H** | Unimplemented architecture decisions → full rework risk before M2; QA tactics (Lock-Free Ring Buffer, Observer pattern, etc.) not applied in code → all QA goals missed |
| **Mitigation** | Fix daily afternoon sync meeting + communicate via Teams channel; confirm and document coding/architecture role boundaries in Project Plan |
| **Linked issue** | OI-06 → resolved by establishing role boundaries and sync process |

---

### NTR-05 — 11개 그래프 전부 구현 시 핵심 기능 품질 저하

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | 5주 일정 내에 11개 그래프를 모두 구현하려다 핵심 측정 기능(Rate·Amplitude·Beat Error 정확도)의 품질이 저하되거나 QA 목표 달성이 불가능해지는 스코프 과잉 리스크 |
| **Probability: H** | 11개 그래프 전부 구현은 팀 인원과 주어진 기간 대비 높은 부하. 우선순위 분류 없이 진행하면 마감 직전 품질 저하 필연 |
| **Impact: M** | 핵심 기능(QAS-1~3) 완성도 저하는 데모 품질에 직접 영향. 단, Extensibility(QAS-5) 달성 시 병렬 개발로 일부 완화 가능 |
| **완화** | 11개 그래프를 **Core / Required / Stretch** 3단계로 분류. Core(팀 제1목표 직결)만 M2 데모 필수 목표로 설정, Stretch는 시간 여유 시 추가 |
| **연결 이슈** | OI-07 → 그래프 우선순위를 Project Plan에 Core / Required / Stretch로 반영하여 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Attempting to implement all 11 graphs within 5 weeks degrades quality of core measurement features (Rate·Amplitude·Beat Error accuracy) or makes QA targets unachievable — scope overextension risk |
| **Probability: H** | Implementing all 11 graphs is high load relative to team size and timeline; without priority classification, quality degradation near deadline is inevitable |
| **Impact: M** | Lower completeness of core features (QAS-1~3) directly affects demo quality; partially mitigated if QAS-5 (Extensibility) enables parallel development |
| **Mitigation** | Classify 11 graphs into **Core / Required / Stretch**; only Core graphs (directly tied to team's primary goal) are mandatory for M2 demo; Stretch added if time permits |
| **Linked issue** | OI-07 → resolved by reflecting graph priorities as Core / Required / Stretch in Project Plan |

---

### NTR-06 — 팀 영어 커뮤니케이션 부담

**한국어**

| 항목 | 내용 |
|------|------|
| **설명** | 교수 및 심사위원이 영어권이고 마일스톤 제출·발표가 영어 기준이나, 팀 내 영어 커뮤니케이션 부담으로 설계 결정 사항이 영어권 팀원 또는 외부에 정확히 전달되지 않는 경우 |
| **Probability: M** | 팀 내 한국어 중심 소통은 자연스럽지만, 마일스톤 제출물이 한국어 단독으로 작성되면 심사 기준 미달 |
| **Impact: H** | 영어권 팀원 이해 누락 → 설계와 구현 불일치. 마일스톤 제출물 품질 저하 → 평가 점수 직접 영향 |
| **완화** | 모든 산출물을 **한영 병기**로 작성 (CLAUDE.md 문서 작성 규칙 준수). 마일스톤 제출·발표는 영어 기준. 팀 내 소통은 한국어 허용 단, 설계 결정 요약은 Teams에 한영 병기로 기록 |
| **연결 이슈** | OI-08 → 작성 기준 합의 + M1 제출 산출물 전체 한/영 병기 규칙 준수 확인으로 해소 |

**English**

| Item | Detail |
|------|--------|
| **Description** | Instructors and reviewers are English-speaking and milestone submissions/presentations are in English; Korean-dominant team communication risks design decisions not being accurately conveyed to English-speaking members or evaluators |
| **Probability: M** | Korean-dominant internal communication is natural, but Korean-only deliverables fall short of evaluation criteria |
| **Impact: H** | English-speaking team member misses context → design–implementation mismatch; lower deliverable quality → direct impact on evaluation score |
| **Mitigation** | Write all deliverables in **bilingual (KO/EN)** format (per CLAUDE.md documentation rule); milestone submissions and presentations in English; internal team communication in Korean is acceptable, but design decision summaries must be recorded bilingually in Teams |
| **Linked issue** | OI-08 → resolved by agreeing on writing standard and verifying all M1 deliverables comply with bilingual rule |

---

## 4. 미결 이슈 — 프로젝트 결과 연결 / Open Issues — Project Outcome Linkage

**한국어**

Architectural Drivers의 미결 이슈(OI-*)와 리스크·액션을 연결한 전체 매핑이다. 모든 OI는 팀 목표(정확한 측정)와 직접·간접으로 연결된다.

| OI ID | 미결 질문 / Open Question | 프로젝트 영향 / Project Impact | 연결 리스크 | 액션 / Action |
|:-----:|--------------------------|-------------------------------|:-----------:|:-------------:|
| **OI-P1** | RPi 5에서 96k sps Dropped Block = 0 달성 가능한가? | 달성 불가 시 48k sps 폴백 → 36k/43k BPH 지원 불가 → 팀 제2목표 포기 | TR-01 | **EXP-01** 실행 |
| **OI-L1** | QAudioSource 라이브 캡처 콜백 주기는? | 콜백 주기가 ~20 ms보다 길면 capture→process 구간 하한 상향 → 지연 목표 초과 가능성 ↑ | TR-03 | **EXP-02** 타임스탬프 TS1 실측 |
| **OI-L2** | 11탭 동시 렌더링 시 ② process→display < 30 ms인가? | 초과 시 Lazy Rendering 강제 적용 → 비활성 탭 데이터 freshness 제한 | TR-03, TR-04 | **EXP-02** 탭 구성 2가지 비교 |
| **OI-L3** | 28,800 BPH 달성 후 36k/43k BPH까지 상향 가능한가? | 달성 실패 시 팀 제2목표 포기 → QAS-2 Stretch 미확정 | — | 28,800 BPH 기준 QA 전부 충족 후 **EXP-05**(BPH 상향 검증)로 해소 예정 |
| **OI-C1** | 소음 3조건에서 Δ를 최소화하는 Detector 파라미터 최적값은? | 최적값 미확정 시 소음 환경에서 정확도 저하 → QAS-3 Response Measure 미확정 | TR-05 | **EXP-03** 실행 |
| **OI-U1** | 경고 발생·해제 N·M 수치는? | N·M 미결 → QAS-4 Response Measure 미확정 → Usability 목표 달성 여부 검증 불가 | TR-09 | 시계 제거/복원 실험 |
| **OI-U2** | No signal / Noisy signal 임계값은? | 임계값 미결 → 오경보/미감지 → 사용자 신뢰성 저하 | TR-09 | 다환경 임계값 탐색 실험 |

**English**

The following table maps all Architectural Drivers open issues (OI-*) to risks and actions. All OIs are directly or indirectly linked to the team's primary goal (accurate measurement).

| OI ID | Open Question | Project Impact | Linked Risk | Action |
|:-----:|--------------|:-------------:|:-----------:|:------:|
| **OI-P1** | Can RPi 5 achieve Dropped Block = 0 at 96k sps? | If not → 48k sps fallback → 36k/43k BPH unsupported → team 2nd goal abandoned | TR-01 | Run **EXP-01** |
| **OI-L1** | What is QAudioSource live callback period? | Longer than ~20 ms → capture→process lower bound rises → latency target may be exceeded | TR-03 | **EXP-02** TS1 measurement |
| **OI-L2** | Is ② process→display < 30 ms with 11 tabs? | If not → Lazy Rendering enforced → inactive tab data freshness limited | TR-03, TR-04 | **EXP-02** 1-tab vs 11-tab comparison |
| **OI-L3** | After 28,800 BPH, can target be raised to 36k/43k? | If not achieved → team 2nd goal abandoned → QAS-2 Stretch unconfirmed | — | Deferred to **EXP-05** (BPH escalation verification) after all 28,800 BPH QA targets are met |
| **OI-C1** | What Detector parameters minimize Δ across 3 noise conditions? | Unresolved → accuracy degrades under noise → QAS-3 Response Measure unconfirmed | TR-05 | Run **EXP-03** |
| **OI-U1** | What are the N·M second values for warning onset/clear? | Unresolved → QAS-4 Response Measure unconfirmed → Usability goal unverifiable | TR-09 | Watch removal/restore experiment |
| **OI-U2** | What are No signal / Noisy signal thresholds? | Unresolved → false alarms or missed warnings → user trust degraded | TR-09 | Multi-environment threshold search |
| **OI-06** | 팀 구조 미확정 / Team structure undefined | 설계 결정이 구현에 미반영 → M2 전 재작업 위험 / Design decisions not implemented → rework risk before M2 | NTR-04 | ✅ project-plan.md 기반 팀 1·팀 2 병렬 애자일(ADD 기반 2-day Scrum) 구성으로 해소 / Resolved by forming Team 1 + Team 2 parallel Agile structure (ADD-based 2-day Scrum) per project-plan.md |
| **OI-07** | 11개 그래프 우선순위 미분류 / Graph priority not classified | 핵심 기능 미완성 상태로 데모 진행 위험 / Core features incomplete at demo | NTR-05 | ✅ project-plan.md 섹션 6에서 Core(3개) / Required(3개) / Stretch(2개+)로 분류 완료 / Classified in project-plan.md Section 6: Core (3) / Required (3) / Stretch (2+) |
| **OI-08** | 산출물 작성 언어 기준 미정 / No standard for deliverable writing language | 영어권 팀원 이해 누락 + 마일스톤 제출물 품질 저하 / English-speaking member misses context + milestone quality degraded | NTR-06 | ✅ 팀내 소통·산출물 한영 병기, 발표/제출 자료 영어로 합의 완료 / Agreed: team communication and deliverables in bilingual (KO/EN); presentations and submissions in English |

---

## 5. 액션 계획 / Action Plan

**한국어**

리스크 해소를 위해 도출된 실험 및 설계 액션의 실행 순서이다.

| 순서 | 액션 / Action | 대상 리스크 | 선행 조건 | 해소 OI |
|:---:|-------------|:-----------:|---------|:-------:|
| **1** | **EXP-01**: 48k/96k/192k sps × 10분 Dropped Block 측정 | TR-01, TR-02 | RPi 5 세팅, 기계식 시계 연결 | OI-P1 |
| **2** | **Observer 패턴 리팩터링** (증분, 회귀 검증 병행) | TR-06, TR-07, NTR-01 | EXP-01 결과 (SPS 방향 결정 후) | — |
| **3** | **≤ 3파일 구조 검증** (신규 그래프 실제 추가) | TR-06 | Observer 리팩터링 완료 | — |
| **4** | **EXP-02**: 3구간 타임스탬프 × 탭 1개/11개 × SPS 3단계 | TR-03, TR-04 | Observer 리팩터링 완료 | OI-L1, OI-L2, OI-L3 |
| **5** | **EXP-03**: 소음 3조건 × Detector 파라미터 조합 | TR-05 | EXP-01 완료 (SPS 결정 후) | OI-C1 |
| **6** | **경고 임계값 실험**: 시계 제거/복원 + 다환경 탐색 | TR-09 | Observer 리팩터링 완료 | OI-U1, OI-U2 |
| ~~**즉시**~~ **✅ 완료** | **팀 구조 확정**: project-plan.md 기반 팀 1·팀 2 병렬 애자일 구성 | NTR-04 | — | OI-06 |
| ~~**즉시**~~ **✅ 완료** | **그래프 우선순위 분류**: project-plan.md 섹션 6에서 Core(3) / Required(3) / Stretch(2+) 확정 | NTR-05 | — | OI-07 |
| ~~**즉시**~~ **✅ 완료** | **산출물 작성 기준 합의**: 팀내 소통·산출물 한영 병기, 발표/제출 영어로 합의 | NTR-06 | — | OI-08 |

**English**

Execution order of experiments and design actions to resolve risks.

| Order | Action | Target Risk | Prerequisite | Resolved OI |
|:-----:|--------|:-----------:|-------------|:-----------:|
| **1** | **EXP-01**: 48k/96k/192k sps × 10 min Dropped Block measurement | TR-01, TR-02 | RPi 5 setup, mechanical watch connected | OI-P1 |
| **2** | **Observer pattern refactoring** (incremental, with unit tests + regression verification) | TR-06, TR-07, NTR-01 | EXP-01 result (SPS direction decided) | — |
| **3** | **≤ 3-file structure verification** (add a new graph for real) | TR-06 | Observer refactoring complete | — |
| **4** | **EXP-02**: 3-segment timestamps × 1-tab/11-tab × 3 SPS tiers | TR-03, TR-04 | Observer refactoring complete | OI-L1, OI-L2, OI-L3 |
| **5** | **EXP-03**: 3 noise conditions × Detector parameter combinations | TR-05 | EXP-01 complete (SPS decided) | OI-C1 |
| **6** | **Warning threshold experiment**: watch removal/restore + multi-environment search | TR-09 | Observer refactoring complete | OI-U1, OI-U2 |
| ~~**Immediate**~~ **✅ Done** | **Team structure definition**: Team 1 + Team 2 parallel Agile structure per project-plan.md | NTR-04 | — | OI-06 |
| ~~**Immediate**~~ **✅ Done** | **Graph priority classification**: Core (3) / Required (3) / Stretch (2+) finalized in project-plan.md Section 6 | NTR-05 | — | OI-07 |
| ~~**Immediate**~~ **✅ Done** | **Writing standard agreement**: bilingual (KO/EN) for all deliverables; English for presentations and submissions | NTR-06 | — | OI-08 |

---

## 6. 리스크–QA 종합 매핑 / Risk–QA Consolidated Mapping

**한국어**

| QA (Architectural Drivers 기준) | 우선순위 | 연결 기술 리스크 | 연결 비기술 리스크 | Overall 최고 등급 |
|:-------------------------------|:-------:|:--------------:|:-----------------:|:----------------:|
| Real-Time Performance (QAS-1) | 1 | TR-01 (**H**), TR-02 (**H**) | NTR-02, NTR-03, NTR-04 (**H**) | **H** |
| Low Latency (QAS-2) | 2 | TR-03 (**H**), TR-04 (M) | NTR-02, NTR-03 | **H** |
| Correctness (QAS-3) | 3 | TR-05 (M) | NTR-03 | **M** |
| Usability (QAS-4) | 4 | TR-09 (M) | — | **M** |
| Extensibility (QAS-5) | 5 | TR-06 (M), TR-07 (**H**), TR-08 (M) | NTR-01 (**H**), NTR-04 (**H**), NTR-05 (**H**) | **H** |
| 전체 프로젝트 / All QAs | — | — | NTR-06 (**H**) | **H** |

**English**

| QA (per Architectural Drivers) | Priority | Linked Technical Risk | Linked Non-Technical Risk | Highest Overall |
|:------------------------------|:-------:|:--------------------:|:-------------------------:|:---------------:|
| Real-Time Performance (QAS-1) | 1 | TR-01 (**H**), TR-02 (**H**) | NTR-02, NTR-03, NTR-04 (**H**) | **H** |
| Low Latency (QAS-2) | 2 | TR-03 (**H**), TR-04 (M) | NTR-02, NTR-03 | **H** |
| Correctness (QAS-3) | 3 | TR-05 (M) | NTR-03 | **M** |
| Usability (QAS-4) | 4 | TR-09 (M) | — | **M** |
| Extensibility (QAS-5) | 5 | TR-06 (M), TR-07 (**H**), TR-08 (M) | NTR-01 (**H**), NTR-04 (**H**), NTR-05 (**H**) | **H** |
| All QAs | — | — | NTR-06 (**H**) | **H** |
