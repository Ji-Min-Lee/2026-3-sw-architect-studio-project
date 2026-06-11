# 테스트 범위 및 TC 커버리지 현황 / Test Scope and TC Coverage Status

---

## 1. 테스트 범위 정의 / Test Scope Definition

**한국어**

테스트 범위는 `main` 브랜치 대비 현재 브랜치(`feature/layer-value-validation`)에서 **신규 구현되거나 리팩토링된 코드**로 한정한다.

`main` 브랜치에 이미 존재했던 외부 라이브러리 코드는 테스트 범위에서 제외한다.

| 경로 | 성격 | 테스트 범위 |
|------|------|------------|
| `src/external/` (Timegrapher, Detector, Bph, Dsp, qcustomplot, kissfft 등) | 원본 외부 라이브러리 (Witschi DSP, Qt plotting) | ❌ 제외 |
| `src/engine/` | 신규 구현 (God Object 분리 결과) | ✅ 포함 |
| `src/tabs/` | 신규 구현 (11개 탭 분리) | ✅ 포함 |
| `src/audio/`, `src/ui/` | 구조 재배치 (리팩토링 범위 아님) | ❌ 제외 |

**English**

The test scope is limited to code that is **newly implemented or refactored** on the current branch (`feature/layer-value-validation`) relative to `main`.

External library code already present in `main` is excluded.

| Path | Nature | In Scope |
|------|--------|----------|
| `src/external/` (Timegrapher, Detector, Bph, Dsp, qcustomplot, kissfft, etc.) | Original external libraries (Witschi DSP, Qt plotting) | ❌ Excluded |
| `src/engine/` | Newly implemented (result of God Object decomposition) | ✅ Included |
| `src/tabs/` | Newly implemented (11 tabs extracted) | ✅ Included |
| `src/audio/`, `src/ui/` | Restructured (not a refactoring target) | ❌ Excluded |

---

## 2. 테스트 목적 / Test Objectives

**한국어**

| 목적 / Objective | 우선순위 / Priority | 대응 구간 / Stage |
|-----------------|-------------------|-----------------|
| 개별 equation의 구현 정확도 확보 (도메인 지식 부족 보완) | H / H | Stage 1 — WatchMath |
| God Object 분리 시 원하지 않는 로직 변경 방지 | H / M | Stage 2 — MeasurementEngine |
| 탭별 plot 값 정확성 검증 | M / M | Stage 3 — Tabs |

**English**

| Objective | Priority (Impact/Effort) | Stage |
|-----------|--------------------------|-------|
| Validate equation accuracy (compensate for domain knowledge gap) | H / H | Stage 1 — WatchMath |
| Prevent unintended logic changes during God Object decomposition | H / M | Stage 2 — MeasurementEngine |
| Verify per-tab plot value correctness | M / M | Stage 3 — Tabs |

---

## 3. 구간별 TC 현황 / TC Status by Stage

### Stage 1 — 순수 수식 계산 / Pure Math Equations

**한국어**  
대상: `src/engine/WatchMath.h/cpp` (신규 추출 — main에 없던 파일)  
파일: `src/tests/test_watch_math.cpp`

**English**  
Target: `src/engine/WatchMath.h/cpp` (new file — not present in main)  
File: `src/tests/test_watch_math.cpp`

| 함수 / Function | TC 수 / TC Count | 커버 항목 / Coverage |
|----------------|-----------------|---------------------|
| `beatErrorMs` | 6 | 정상, worked example, 비대칭(양/음), 큰 비대칭, 다른 샘플레이트 |
| `amplitudeDeg` | 5 | worked example(230°), t_AC≈0 무효, ≥360° 무효, 근사식 수렴, 다른 BPH |
| `escapementMs` | 6 | 정상, 샘플레이트 스케일링, 비영 aPos, 구간=0, 역순(음수 반환 2건) |
| `wrapInRange` | 7 | 범위 내, 상한 경계, 하한 경계, 상한 초과, 하한 미달, 먼 범위 초과, 큰 양수 |
| `applyZeroOffset` | 4 | beat 0 = 0, 상대값, 빠른 시계 누적, 음수 앵커 |
| `halfBeatInterval` | 3 | 28800/21600/36000 BPH |
| `instErrorSec` | 5 | beat 0, 완벽 시계, 빠른 시계, 느린 시계, worked example(p.2) |
| `rateSpdFromPhase` | 4 | worked example(p.6), 완벽 시계, 빠른 시계, Tic-Tac 비대칭 평균 |
| 근사식 직접 검증 / Approx. formula | 2 | worked example, t_AC 반비례 |
| **합계 / Total** | **42** | |

---

### Stage 2 — 엔진 상태 및 평균 / Engine State and Averaging

**한국어**  
대상: `MeasurementEngine`, `RollingAverage`, `RollingLeastSquares`  
파일: `test_measurement_engine.cpp`, `test_rolling_average.cpp`, `test_rolling_least_squares.cpp`

**English**  
Target: `MeasurementEngine`, `RollingAverage`, `RollingLeastSquares`  
Files: `test_measurement_engine.cpp`, `test_rolling_average.cpp`, `test_rolling_least_squares.cpp`

#### 2a. MeasurementEngine

| TC 이름 / TC Name | 검증 항목 / What It Verifies |
|------------------|------------------------------|
| `beatError_workedExample_matchesEquation` | computeBeatError → RollingAverage에 올바른 값 누적 |
| `computeAmplitude_ticAndTocProduceSplitAndAverage` | Tic/Toc 분리 저장 및 평균 emit 동작 |
| `computeRateError_perfectWatch_setsZeroWrappedPointsAndZeroRate` | 완벽 시계 → rate=0, wrappedError=0, isTic/isToc 분류 |
| `computeRateError_knownDeviation_rateSpd_8p64` | Equations p.6 worked example (+8.640 s/day) |
| `computeRateError_multibeat_rateConverges` | 20-beat 평균 시 수렴 정밀도(±0.01 s/day) |
| `computeRateError_slowWatch_rateSpd_negative` | 느린 시계 → rate < 0 방향성 검증 |
| **합계 / Total** | **6** |

#### 2b. RollingAverage

| 카테고리 / Category | TC 수 / Count |
|--------------------|--------------|
| 기본 평균 동작 / Basic averaging | 3 |
| 윈도우 슬라이딩 / Window sliding | 2 |
| Reset / Resize | 4 |
| 빈 상태, 음수값 / Empty, negative | 2 |
| 실사용 시나리오 (Beat Error) / Real scenario | 1 |
| **합계 / Total** | **12** |

#### 2c. RollingLeastSquares

| 카테고리 / Category | TC 수 / Count |
|--------------------|--------------|
| Slope 계산 / Slope calculation | 3 |
| 롤링 윈도우 / Rolling window | 2 |
| 엣지 케이스 (1점, 0점, 특이행렬) / Edge cases | 3 |
| Reset | 2 |
| 실사용 시나리오 (Rate s/day) / Real scenario | 1 |
| **합계 / Total** | **11** |

---

### Stage 3 — 탭별 그래프 렌더링 / Per-Tab Graph Rendering

**한국어**  
대상: `src/tabs/` 11개 탭  
파일: `test_graph_tabs.cpp`, `test_measurement_summaries.cpp`, `test_remaining_tabs.cpp`

**English**  
Target: 11 tabs under `src/tabs/`  
Files: `test_graph_tabs.cpp`, `test_measurement_summaries.cpp`, `test_remaining_tabs.cpp`

| 탭 / Tab | TC 수 / Count | 커버 항목 / Coverage | 상태 / Status |
|----------|--------------|---------------------|--------------|
| `TraceTab` | 8 | rate/amplitude plot 값, 누적 순서, invalid 무시, reset, X축 시간 간격 | ✅ |
| `BeatErrorTab` | 4 | 값 plot, 복수 누적, invalid 무시, reset | ✅ |
| `VarioTab` | 6 | Tic/Toc 분리 plot, X축 증가, 통계 누적, reset, non-split 이벤트 무시 | ✅ |
| `EscapementTab` | 5 | C이벤트 plot, hasEsc=false 무시, A이벤트 무시, X축 증가, reset | ✅ |
| `LongTermTab` | 12 | 버킷 커밋 동작, 3채널 평균, 버킷 내 누적 평균, invalid 필터, reset, 인터벌 정책(1s/10s/60s), X축 버킷 중간값 | ✅ |
| `BeatNoiseScopeTab` | 7 | A/C 쌍 scope1 데이터, 범위별 윈도우 크기(20/200/400ms), scope2 Tic-Tac 분리, reset, 방어(empty rawPcm) | ✅ |
| `SpectrogramTab` | 5 | 빈 입력, 출력 크기(nfft/2), 정규화 최대값=1, 사인파 피크 bin, freqStep 검증 | ✅ |
| `WaveformCompTab` | 5 | rawPcm 없음 방어, A→Tic graph, A+C→양쪽 graph, C만→Toc 미그림, reset | ✅ |
| `RateScopeTab` | 5 | PCM scope plot, Tic→graph(0), Toc→graph(1), wrappedValue 보존, reset | ✅ |
| `SoundPrintTab` | 4 | null widget 안전성(construction/measurement/reset/setBph) | ⚠️ 안전성만, 렌더링 로직 없음 |
| `SequenceTab` | 8 | 첫 A 이벤트 단독(데이터 없음), A 쌍 interval 계산, 복수 interval 순서, X축 증가, C 이벤트 무시, A-C-A 혼합, 블록 간 상태 보존, reset 후 인덱스 초기화 | ✅ |

> **비고 / Notes**
> - `SoundPrintTab`: `SoundImageWidget`이 외부 렌더러 의존 → 렌더링 로직 TC 추가 어려움. 안전성 TC(4건)만 존재.
> - `SequenceTab`: `MeasurementSummaries.h`의 `SequenceSummary`는 Stage 2c에서 별도 검증됨. 탭 자체의 TC는 미작성.

---

## 4. 전체 요약 / Overall Summary

| 구간 / Stage | 대상 파일 / Target Files | TC 수 / TC Count | 상태 / Status |
|-------------|------------------------|-----------------|--------------|
| Stage 1 — WatchMath | `WatchMath.cpp` | 42 | ✅ |
| Stage 2a — MeasurementEngine | `MeasurementEngine.cpp` | 6 | ✅ |
| Stage 2b — RollingAverage | `RollingAverage.cpp` | 12 | ✅ |
| Stage 2c — RollingLeastSquares | `RollingLeastSquares.cpp` | 11 | ✅ |
| Stage 3 — Tabs (10개 / 10 tabs) | `tabs/*.cpp` × 10 | 65 | ✅ |
| Stage 3 — SoundPrintTab | `SoundPrintTab.cpp` | 4 | ⚠️ |
| **전체 / Total** | | **140** | |

---

## 5. 미작성 / 보완 필요 TC / Gaps to Address

**한국어**

| 항목 / Item | 내용 / Detail | 우선순위 / Priority |
|------------|--------------|-------------------|
| ~~`SequenceTab`~~ | ~~`onMeasurement()` plot 값, reset 동작~~ | ~~Medium~~ → ✅ 완료 |
| `SoundPrintTab` 렌더링 | `SoundImageRenderer` 외부 코드, 주입 seam 없음 → 설계 변경 필요 | Low |
| `MeasurementEngine` noSignal | `QElapsedTimer`가 엔진 내부에 직접 내장 → 시간 주입 seam 없어 실시간 3초 대기 없이 테스트 불가 **[설계 이슈]** | Medium — 테스트 가능하게 하려면 timer를 주입 가능하게 추출 필요 |
| `SoundPrintTab` 렌더링 | `SoundImageRenderer`가 external 코드이고 생성자 주입 seam 없음 **[설계 이슈]** | Low — 렌더러 인터페이스 추출 후 mock 주입 필요 |

**English**

| Item | Detail | Priority |
|------|--------|----------|
| `SequenceTab` | `onMeasurement()` plot values, position label, reset | Medium |
| `SoundPrintTab` rendering | Addable if `SoundImageWidget` is decoupled | Low |
| `MeasurementEngine` noSignal | 3-second no-signal → `noSignal=true` emit path not covered | Medium |
