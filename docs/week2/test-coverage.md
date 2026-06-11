---
title: 테스트 범위 및 커버리지 현황 / Test Scope and Coverage Status
updated: 2026-06-12
branch: feature/layer-value-validation-v2
---

# 테스트 범위 및 커버리지 현황 / Test Scope and Coverage Status

---

## 1. 테스트 목적 / Test Objectives

**한국어**
이 테스트 스위트는 두 가지 리스크를 해소하기 위해 작성되었습니다.

| 리스크 | 내용 | 우선순위 |
|---|---|---|
| 도메인 지식 부족 | 개별 계산 공식(Equation)을 완전히 이해하기 어려움 → AI + 단위 테스트로 구현 정확도를 보완 | High |
| God Object 분리 리스크 | 탭 로직 분리 시 원치 않는 동작 변경이 발생할 수 있음 → 단위 테스트로 회귀 방지 | High |

**English**
This test suite was written to address two key risks.

| Risk | Description | Priority |
|---|---|---|
| Domain knowledge gap | Individual equations are hard to fully verify manually → AI-assisted unit tests improve implementation accuracy | High |
| God Object decomposition risk | Splitting tab logic may silently change behaviour → unit tests prevent regressions | High |

---

## 2. 테스트 범위 정의 / Test Scope Definition

**한국어**
테스트 범위는 `main` 브랜치 대비 `feature/layer` 브랜치에서 **신규 구현되었거나 리팩토링된 코드**에 한정합니다.

| 포함 / Included | 제외 / Excluded |
|---|---|
| `src/engine/` — WatchMath, MeasurementEngine, Rolling* | `src/external/` — 원본 Witschi DSP (변경 없음) |
| `src/tabs/` — 11개 탭 전체 | `src/audio/` — AudioWorker, PlaybackWorker 등 |
| | `src/ui/` — MainWindow UI 레이어 |

**English**
The scope covers only code that is **newly implemented or refactored** on `feature/layer` compared to `main`.

---

## 3. 구간(Stage) 정의 / Stage Definitions

**한국어**
신호 처리 파이프라인을 3개 구간으로 분류합니다.

```
[Stage 1]  수식(Equation) 계층
           WatchMath 순수 함수, RollingAverage, RollingLeastSquares
           (공통 — 모든 탭의 수치 계산 기반)

[Stage 2]  엔진(Engine) 계층
           MeasurementEngine: Sample → Beat → rate/beatError/amplitude 계산
           (공통 — 단일 Measurement 객체를 생성하여 모든 탭에 배포)

[Stage 3]  탭(Tab) 계층
           Measurement → 탭별 그래프 값 (탭별 별개, 일부 데이터 공유)
           Stage 3A: 핵심 탭 (TraceTab, BeatErrorTab, VarioTab, RateScopeTab)
           Stage 3B: 추가 탭 (SequenceTab, EscapementTab, LongTermTab, BeatNoiseScopeTab,
                              WaveformCompTab, SpectrogramTab)
           Stage 3C: 신규 탭 (SweepScopeTab, FilterScopeTab, SoundPrintTab)
```

**English**
The signal-processing pipeline is divided into three stages.

```
[Stage 1]  Equation layer
           WatchMath pure functions, RollingAverage, RollingLeastSquares
           (shared — numeric foundation for every tab)

[Stage 2]  Engine layer
           MeasurementEngine: Sample → Beat → rate/beatError/amplitude
           (shared — produces a single Measurement object broadcast to all tabs)

[Stage 3]  Tab layer
           Measurement → per-tab plot values (per-tab, some data shared)
           Stage 3A: Core tabs (TraceTab, BeatErrorTab, VarioTab, RateScopeTab)
           Stage 3B: Added tabs (SequenceTab, EscapementTab, LongTermTab, BeatNoiseScopeTab,
                                  WaveformCompTab, SpectrogramTab)
           Stage 3C: New tabs  (SweepScopeTab, FilterScopeTab, SoundPrintTab)
```

---

## 4. 구간별 TC 목록 / TC List by Stage

### Stage 1 — 수식(Equation) 계층 / Equation Layer

#### 1-A. WatchMath  (`TestWatchMath` · 42 TCs)

**한국어** / **English**

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | WM-01 | `beatError_perfectSymmetry_isZero` | t1=t2 → BE = 0 ms |
| 2 | WM-02 | `beatError_workedExample_0p8ms` | 수식 직접 대입 결과 0.8 ms 일치 |
| 3 | WM-03 | `beatError_t1GreaterThanT2_positive` | t1 > t2 시 양수 반환 |
| 4 | WM-04 | `beatError_t2GreaterThanT1_absoluteValue` | 절댓값 적용 확인 |
| 5 | WM-05 | `beatError_largeAsymmetry` | 큰 비대칭 값 처리 |
| 6 | WM-06 | `beatError_differentSampleRate_44100` | 44 100 Hz 샘플레이트 스케일링 |
| 7 | WM-07 | `amplitude_workedExample_230deg` | 230° 진폭 수식 대입 검증 |
| 8 | WM-08 | `amplitude_nearZero_tAC_returnsInvalid` | t_AC ≈ 0 → 음수(무효) 반환 |
| 9 | WM-09 | `amplitude_over360_returnsInvalid` | ≥ 360° → 무효 반환 |
| 10 | WM-10 | `amplitude_convergesWithApprox_forSmallTAC` | 소각도에서 근사값 수렴 |
| 11 | WM-11 | `amplitude_21600bph_validResult` | 21 600 bph 정상 계산 |
| 12 | WM-12 | `escapement_9ms_exact` | T1→T3 = 9 ms 정확도 |
| 13 | WM-13 | `escapement_sampleRateScaling` | 샘플레이트 변환 선형성 |
| 14 | WM-14 | `escapement_nonZeroAPos` | aPos ≠ 0 기준점 처리 |
| 15 | WM-15 | `escapement_zero_interval` | A=C 위치 → 0 ms |
| 16 | WM-16 | `wrap_valueInsideRange_unchanged` | 범위 내 값 변경 없음 |
| 17 | WM-17 | `wrap_exactUpperBound_wrapsToLower` | 상한 정확히 도달 시 하한으로 랩 |
| 18 | WM-18 | `wrap_exactLowerBound_unchanged` | 하한 정확히 도달 시 변경 없음 |
| 19 | WM-19 | `wrap_slightlyAboveUpper` | 상한 초과 시 랩 |
| 20 | WM-20 | `wrap_slightlyBelowLower` | 하한 미만 시 랩 |
| 21 | WM-21 | `wrap_farOutOfRange` | 크게 벗어난 값 랩 |
| 22 | WM-22 | `wrap_largePositive` | 매우 큰 양수 랩 |
| 23 | WM-23 | `zeroOffset_firstBeat_plotsAtZero` | 첫 비트 기준점 0 |
| 24 | WM-24 | `zeroOffset_subsequentBeat_isRelative` | 후속 비트 상대 오프셋 |
| 25 | WM-25 | `zeroOffset_watchFast_growsPositive` | 빠른 시계 → 양수 증가 |
| 26 | WM-26 | `zeroOffset_negativeAnchor_correctsOffset` | 음수 앵커 보정 |
| 27 | WM-27 | `escapement_reverseOrder_returnsNegative` | C < A 순서 → 음수 |
| 28 | WM-28 | `escapement_reverseOrder_magnitudeCorrect` | 역순 절댓값 일치 |
| 29 | WM-29 | `halfBeat_28800bph_is125ms` | 28 800 bph → 125 ms |
| 30 | WM-30 | `halfBeat_21600bph_is166ms` | 21 600 bph → ≈166 ms |
| 31 | WM-31 | `halfBeat_36000bph_is100ms` | 36 000 bph → 100 ms |
| 32 | WM-32 | `instError_beat0_isZero` | 첫 비트 순시 오차 = 0 |
| 33 | WM-33 | `instError_perfectWatch_isZero` | 완벽한 시계 0 오차 |
| 34 | WM-34 | `instError_fastWatch_growsNegative` | 빠른 시계 음수 누적 |
| 35 | WM-35 | `instError_slowWatch_growsPositive` | 느린 시계 양수 누적 |
| 36 | WM-36 | `instError_workedExample_p2` | 수식 대입 +0.2 s 검증 |
| 37 | WM-37 | `rateSpd_workedExample_p6` | +0.6 s/day 수식 검증 |
| 38 | WM-38 | `rateSpd_perfectWatch_isZero` | 완벽한 시계 → 0 s/day |
| 39 | WM-39 | `rateSpd_fastWatch_isNegative` | 빠른 시계 → 음수 |
| 40 | WM-40 | `rateSpd_ticTacAsymmetry_averaged` | Tic/Toc 비대칭 평균 |
| 41 | WM-41 | `ampApprox_workedExample_230deg` | 근사 공식 230° 검증 |
| 42 | WM-42 | `ampApprox_inverseProportional_toTAC` | t_AC 역비례 확인 |

#### 1-B. RollingAverage  (`TestRollingAverage` · 12 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | RA-01 | `singleValue_returnsItself` | 단일 값 → 그대로 반환 |
| 2 | RA-02 | `twoValues_returnsArithmicMean` | 2개 값 산술 평균 |
| 3 | RA-03 | `constantStream_returnsConstant` | 상수 스트림 수렴 |
| 4 | RA-04 | `window3_slidesCorrectly` | 윈도우 3 슬라이딩 |
| 5 | RA-05 | `window_doesNotExceedCapacity` | 윈도우 용량 초과 방지 |
| 6 | RA-06 | `reset_clearsAllState` | reset 후 상태 초기화 |
| 7 | RA-07 | `afterReset_acceptsNewValues` | reset 후 재사용 |
| 8 | RA-08 | `resize_shrink_trimsOldestValues` | 윈도우 축소 시 오래된 값 제거 |
| 9 | RA-09 | `resize_grow_keepsExistingValues` | 윈도우 확장 시 기존 값 유지 |
| 10 | RA-10 | `emptyWindow_returns0` | 빈 윈도우 → 0 반환 |
| 11 | RA-11 | `negativeValues_averagedCorrectly` | 음수 값 평균 정확도 |
| 12 | RA-12 | `beatErrorScenario_constantError` | 박동 오차 시나리오 |

#### 1-C. RollingLeastSquares  (`TestRollingLeastSquares` · 11 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | RLS-01 | `twoPoints_perfectLine_exactSlope` | 2점 직선 기울기 정확도 |
| 2 | RLS-02 | `fivePoints_perfectLine_exactSlope` | 5점 직선 기울기 정확도 |
| 3 | RLS-03 | `negativeSlope_detectedCorrectly` | 음수 기울기 감지 |
| 4 | RLS-04 | `rollingWindow_dropsOldestPoint` | 롤링 윈도우 오래된 점 제거 |
| 5 | RLS-05 | `window_sizeCapped_atCapacity` | 윈도우 크기 상한 |
| 6 | RLS-06 | `onePoint_returnsFalse` | 1점 → 유효하지 않음 |
| 7 | RLS-07 | `noPoints_returnsFalse` | 0점 → 유효하지 않음 |
| 8 | RLS-08 | `allSameX_singular_returnsFalse` | X 동일 → singular 처리 |
| 9 | RLS-09 | `reset_clearsAllPoints` | reset 후 상태 초기화 |
| 10 | RLS-10 | `afterReset_acceptsNewPoints` | reset 후 재사용 |
| 11 | RLS-11 | `rateScenario_slopeToSecondsPerDay` | 기울기 → s/day 변환 |

---

### Stage 2 — 엔진(Engine) 계층 / Engine Layer

#### 2. MeasurementEngine  (`TestMeasurementEngine` · 6 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | ME-01 | `beatError_workedExample_matchesEquation` | computeBeatError → RollingAverage 누적 |
| 2 | ME-02 | `computeAmplitude_ticAndTocProduceSplitAndAverage` | Tic/Toc 분리 진폭 + 평균 emit |
| 3 | ME-03 | `computeRateError_perfectWatch_setsZeroWrappedPointsAndZeroRate` | 완벽 시계 → rate = 0, wrapped = 0 |
| 4 | ME-04 | `computeRateError_knownDeviation_rateSpd_8p64` | +8.640 s/day 수식 검증 |
| 5 | ME-05 | `computeRateError_multibeat_rateConverges` | 다중 비트 수렴 |
| 6 | ME-06 | `computeRateError_slowWatch_rateSpd_negative` | 느린 시계 → 음수 rate |

---

### Stage 3A — 핵심 탭 / Core Tabs

#### 3A-1. TraceTab  (`TestGraphTabs` · 6 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | TT-01 | `traceTab_rateValue_appearsInPlot` | rate 값이 plot에 반영됨 |
| 2 | TT-02 | `traceTab_negativeRate_appearsInPlot` | 음수 rate 처리 |
| 3 | TT-03 | `traceTab_multiplePoints_accumulateInOrder` | 다중 포인트 순서 보존 |
| 4 | TT-04 | `traceTab_invalidRate_notDrawn` | 무효 rate 미반영 |
| 5 | TT-05 | `traceTab_reset_clearsData` | reset 후 데이터 소거 |
| 6 | TT-06 | `traceTab_xAxis_advancesWithBlockSize` | X축 시간 블록 단위 진행 |

#### 3A-2. BeatErrorTab  (`TestGraphTabs` · 4 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | BE-01 | `beatErrorTab_value_appearsInPlot` | 박동 오차 값 plot 반영 |
| 2 | BE-02 | `beatErrorTab_multipleValues_inOrder` | 다중 값 순서 보존 |
| 3 | BE-03 | `beatErrorTab_invalidBeatError_notDrawn` | 무효 박동 오차 미반영 |
| 4 | BE-04 | `beatErrorTab_reset_clearsData` | reset 후 데이터 소거 |

#### 3A-3. VarioTab  (`TestGraphTabs` · 5 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | VA-01 | `varioTab_stats_trackMinMeanMax` | Stats 구조체 min/mean/max 추적 |
| 2 | VA-02 | `varioTab_sigma_matchesSampleStdDev` | 표본 표준편차 수식 일치 |
| 3 | VA-03 | `varioTab_invalidRate_notCounted` | 무효 rate 미집계 |
| 4 | VA-04 | `varioTab_elapsedTime_accumulates` | 경과 시간 누적 |
| 5 | VA-05 | `varioTab_reset_clearsStats` | reset 후 통계 초기화 |

#### 3A-4. RateScopeTab  (`TestRemainingTabs` · 5 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | RS-01 | `pcmBlock_appearsInScopePlot` | PCM 블록 수신 후 scope plot 그래프 생성 |
| 2 | RS-02 | `ticEvent_appendsToTicSeries` | Tic 이벤트 → ratePlot graph(0) |
| 3 | RS-03 | `tocEvent_appendsToTocSeries` | Toc 이벤트 → ratePlot graph(1) |
| 4 | RS-04 | `wrappedValue_isPreserved` | wrappedRateError 값 왜곡 없음 |
| 5 | RS-05 | `reset_clearsSeries` | reset 후 rate 시리즈 소거 |

---

### Stage 3B — 추가 탭 / Added Tabs

#### 3B-1. SequenceTab  (`TestAddedTabs` · 2 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | SQ-01 | `sequenceTab_capture_recordsAtActivePosition` | 현재 포지션에 rate/amp 기록 |
| 2 | SQ-02 | `sequenceTab_reset_clearsTable` | reset 후 테이블 소거 |

#### 3B-2. EscapementTab  (`TestAddedTabs` · 2 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | ET-01 | `escapementTab_deltaMs_matchesEventSpacing` | T1→T3 간격 ms 일치 |
| 2 | ET-02 | `escapementTab_reset_clearsState` | reset 후 상태 소거 |

#### 3B-3. LongTermTab  (`TestAddedTabs` · 3 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | LT-01 | `longTermTab_timeAndRate_matchInput` | 시간/rate 입력값 plot 반영 |
| 2 | LT-02 | `longTermTab_threeSeries_populateIndependently` | rate/amp/beat 3개 시리즈 독립 |
| 3 | LT-03 | `longTermTab_invalidRate_notDrawn` | 무효 rate 미반영 |

#### 3B-4. BeatNoiseScopeTab  (`TestAddedTabs` · 2 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | BN-01 | `beatNoise_capturesBeatWaveformWindow` | 비트 파형 윈도우 캡처 |
| 2 | BN-02 | `beatNoise_reset_clearsCaptures` | reset 후 캡처 소거 |

#### 3B-5. WaveformCompTab  (`TestAddedTabs` · 2 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | WC-01 | `waveformComp_ticWindow_matchesRawPcm` | Tic 윈도우 raw PCM 일치 |
| 2 | WC-02 | `waveformComp_tocPair_alsoPlotted` | Toc 파형도 plot |

#### 3B-6. SpectrogramTab  (`TestAddedTabs` · 1 TC)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | SP-01 | `spectrogram_peakRowAtInputFrequency` | 입력 주파수에서 color map 피크 |

---

### Stage 3C — 신규 탭 / New Tabs

#### 3C-1. SweepScopeTab  (`TestRemainingTabs` · 4 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | SW-01 | `pcmBlock_producesPlotData` | PCM 블록 후 plot 데이터 생성 |
| 2 | SW-02 | `bufferLength_matchesBphMultiple` | 버퍼 길이 = beatSamples × multiple |
| 3 | SW-03 | `reset_clearsSweepAndPlot` | reset 후 버퍼·plot 소거 |
| 4 | SW-04 | `absoluteValue_storedInSweep` | 음수 PCM → 절댓값 저장 |

#### 3C-2. FilterScopeTab  (`TestRemainingTabs` · 5 TCs)

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | FS-01 | `f0_outputSizeMatchesInput` | F0 출력 크기 = 입력 크기 |
| 2 | FS-02 | `f0_mirroredGraph1HasData` | F0 graph(1) 음수 미러 |
| 3 | FS-03 | `f1_allValuesNonNegative` | F1 이동 평균 모두 ≥ 0 |
| 4 | FS-04 | `f1_graph1IsEmpty` | F1 graph(1) 비어 있음 |
| 5 | FS-05 | `reset_clearsBothGraphs` | reset 후 두 그래프 소거 |

#### 3C-3. SoundPrintTab  (`TestRemainingTabs` · 4 TCs)

**한국어** `SoundImageRenderer`에 인터페이스 시임(seam)이 없어 렌더링 출력 검증이 불가합니다. null-safety TC만 작성합니다.

**English** `SoundImageRenderer` has no interface seam, so rendering output cannot be verified. Only null-safety TCs are written.

| # | TC ID | 테스트 케이스 / Test Case | 검증 내용 / What is verified |
|---|---|---|---|
| 1 | SO-01 | `construction_withNullWidget_doesNotCrash` | null widget 생성 시 crash 없음 |
| 2 | SO-02 | `onMeasurement_emptyPcm_doesNotCrash` | 빈 PCM 수신 시 crash 없음 |
| 3 | SO-03 | `reset_withNullWidget_doesNotCrash` | null widget reset 시 crash 없음 |
| 4 | SO-04 | `setBph_setSampleRate_doNotCrash` | setBph/setSampleRate crash 없음 |

---

## 5. 커버리지 요약 / Coverage Summary

**한국어** / **English**

| 구간 / Stage | 테스트 바이너리 / Binary | 대상 / Target | TC 수 / Count | 상태 / Status |
|---|---|---|---|---|
| 1-A Equation — WatchMath | `TestWatchMath` | `WatchMath.h/cpp` | 42 | ✅ Pass |
| 1-B Equation — RollingAverage | `TestRollingAverage` | `RollingAverage.h/cpp` | 12 | ✅ Pass |
| 1-C Equation — RollingLeastSquares | `TestRollingLeastSquares` | `RollingLeastSquares.h/cpp` | 11 | ✅ Pass |
| 2 Engine — MeasurementEngine | `TestMeasurementEngine` | `MeasurementEngine.h/cpp` | 6 | ✅ Pass |
| 3A Core tabs | `TestGraphTabs` | TraceTab, BeatErrorTab, VarioTab, RateScopeTab | 20 | ✅ Pass |
| 3B Added tabs | `TestAddedTabs` | SequenceTab, EscapementTab, LongTermTab, BeatNoiseScopeTab, WaveformCompTab, SpectrogramTab | 12 | ✅ Pass |
| 3C New tabs | `TestRemainingTabs` | SweepScopeTab, FilterScopeTab, SoundPrintTab | 13 | ✅ Pass |
| **합계 / Total** | | | **116** | **✅ 7/7 Pass** |

---

## 6. 설계 이슈 — 테스트 불가 항목 / Design Issues — Untestable Items

**한국어**

| 항목 / Item | 이유 / Reason | 제안 / Suggestion |
|---|---|---|
| `MeasurementEngine::noSignal` | `QElapsedTimer`가 직접 임베드되어 있어 시간 주입(injection) 불가 | 타이머를 인터페이스로 추상화 |
| `SoundPrintTab` 렌더링 출력 | `SoundImageRenderer`가 외부 코드이며 인터페이스 시임(seam) 없음 | `ISoundRenderer` 인터페이스 도입 |

**English**

| Item | Reason | Suggestion |
|---|---|---|
| `MeasurementEngine::noSignal` | `QElapsedTimer` is directly embedded — no time-injection seam | Abstract timer behind an interface |
| `SoundPrintTab` rendering output | `SoundImageRenderer` is external code with no interface seam | Introduce `ISoundRenderer` interface |
