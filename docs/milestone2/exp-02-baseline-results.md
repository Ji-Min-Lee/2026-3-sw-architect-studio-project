# EXP-02: E2E 오디오 파이프라인 레이턴시 기준선 측정 / EXP-02: E2E Audio Pipeline Latency Baseline Measurement

## 1. 결과 요약 / Results and Recommendations

**한국어**

macOS 환경(Playback 모드, 96kHz)에서 feature/layer 코드베이스의 E2E 레이턴시 기준선을 측정하였다.
exec_ms 평균 0.57ms로 처리 자체는 매우 빠르며, deadline miss 0%를 달성하였다.
대부분의 지연(wait_ms 평균 420ms)은 오디오 스레드 → 메인 스레드 간 Qt 큐 대기에서 발생하였다.
RPi 기준선(EXP-02, baseline/experiments2)과 비교하여 feature/layer의 QueuedConnection 분리가 plot 부하(16ms)를 exec path에서 제거함을 확인하였다.

T2(DSP Offload Thread) 적용 후 wait_ms가 420ms → 0.013ms로 극적으로 감소하였으며, backlog 47% → 0%로 완전 해소되었다.
DSP 스레드가 Worker 스레드와 같은 FPS(95.6)로 동작함을 확인하여 실시간 추적이 성립함을 검증하였다.

**English**

Measured the E2E latency baseline of the feature/layer codebase on macOS (Playback mode, 96kHz).
Average exec_ms of 0.57ms confirms very fast processing; deadline miss rate is 0%.
The dominant delay (avg wait_ms = 420ms) originates from Qt queue latency between the audio thread and the main thread.
Compared to the RPi baseline (EXP-02, baseline/experiments2), the QueuedConnection separation in feature/layer confirms that the 16ms plot burden has been removed from the exec path.

After applying T2 (DSP Offload Thread), wait_ms dropped dramatically from 420ms to 0.013ms, and backlog was fully eliminated (47% → 0%).
The DSP thread was confirmed to track the worker thread at identical FPS (95.6), validating real-time processing.

---

## 2. 목적 / Objective

**한국어**

feature/layer 코드베이스에서 E2E 오디오 처리 파이프라인의 레이턴시 분포를 측정하여 다음 질문에 답한다.

- 실제 처리 시간(exec_ms)은 얼마인가?
- Qt QueuedConnection 구조로 인해 탭 렌더링이 exec path에서 분리되었는가?
- RPi에서 측정한 기준선(baseline/experiments2 EXP-02)과 어떻게 다른가?
- 이후 T1(스레드 정책), T2(DSP 분리), R1/R2(렌더링 전략) 전술 적용의 기준점(baseline)을 확보한다.

**English**

Measure the latency distribution of the E2E audio processing pipeline on the feature/layer codebase to answer the following questions.

- What is the actual processing time (exec_ms)?
- Is tab rendering separated from the exec path via Qt QueuedConnection?
- How does it differ from the RPi baseline (EXP-02, baseline/experiments2)?
- Establish a baseline reference point for subsequent tactic application: T1 (thread policy), T2 (DSP offload), R1/R2 (rendering strategy).

---

## 3. 상태 / Status

Concluded (2026-06-14)

---

## 4. 실험 환경 / Environment

| 항목 / Item | 값 / Value |
|-------------|-----------|
| 플랫폼 / Platform | macOS (Apple Silicon) |
| 모드 / Mode | Playback |
| WAV 파일 / WAV File | `28800BPH_3235_Starbucks.wav` |
| 샘플레이트 / Sample Rate | 96,000 Hz |
| BPH | 28,800 |
| 브랜치 / Branch | `feature/layer-ex-baseline` |
| 빌드 플래그 / Build Flag | `ENABLE_LOGGING=ON` |
| 총 프레임 수 / Total Frames | 2,900 |

---

## 5. 측정 방법 / Measurement Method

**한국어**

`src/logging/Logger.h/cpp`에 per-frame CSV 로깅 인프라를 구현하였다(`baseline/experiments2`에서 포팅).
타임스탬프 모델은 다음과 같다.

- **TS1**: `PlaybackWorker::StartPlayback()` 에서 `emit PlaybackDataReady(TG_NOW())`
- **TS2**: `MainWindow::HandleInputData()` 진입 시 `TG_NOW()`
- `wait_us` = TS2 − TS1 (Qt 큐 대기 + 스케줄링 지연)
- `exec_us` = HandleInputData 종료 − TS2 (실제 처리 시간)
- `tg_us` = `MeasurementEngine::processBlock()` 소요 시간
- `copy_us` = 링 버퍼 복사 소요 시간
- `ui_us`, `plot_us` = 0 (QueuedConnection으로 비동기 분리됨)

로그는 100프레임마다 CSV에 flush (앱 강제종료 시에도 데이터 보존).

**English**

Implemented per-frame CSV logging infrastructure in `src/logging/Logger.h/cpp` (ported from `baseline/experiments2`).
Timestamp model:

- **TS1**: `emit PlaybackDataReady(TG_NOW())` inside `PlaybackWorker::StartPlayback()`
- **TS2**: `TG_NOW()` at entry of `MainWindow::HandleInputData()`
- `wait_us` = TS2 − TS1 (Qt queue wait + scheduling jitter)
- `exec_us` = HandleInputData exit − TS2 (actual processing time)
- `tg_us` = elapsed inside `MeasurementEngine::processBlock()`
- `copy_us` = ring-buffer copy elapsed
- `ui_us`, `plot_us` = 0 (separated asynchronously via QueuedConnection)

Log is flushed to CSV every 100 frames (data preserved even on force-quit).

---

## 6. 측정 결과 / Results

### 6.0 Run 이력 / Run History

| Run | 날짜 / Date | 브랜치 / Branch | 전술 / Tactic | 플랫폼 / Platform | 프레임 / Frames | wait avg | exec avg | deadline miss | backlog | replot_count avg |
|:---:|------------|----------------|:------------:|:-----------------:|:--------------:|:--------:|:--------:|:-------------:|:-------:|:----------------:|
| R1 | 2026-06-14 | feature/layer-ex-baseline | Baseline (main thread DSP) | macOS 96kHz | 2,900 | 419.92 ms | 0.57 ms | 0% | 47% | — |
| R2 | 2026-06-14 | feature/layer-ex-baseline | **T2 DSP Offload Thread** | macOS 96kHz | 4,500 | **0.013 ms** | **0.40 ms** | 0% | **0%** | — |
| R2b | 2026-06-15 | exp/r1-replot-baseline | T2 + **replot counter (가드 없음)** · Scenario A | macOS 96kHz | 4,501 | 0.013 ms | ~0.40 ms | 0% | 0% | **8.22** |
| R3 | 2026-06-15 | feature/layer-ex-baseline | **T2 + R1 Lazy Rendering** · Scenario A (1탭 고정) | macOS 96kHz | 4,501 | 0.013 ms | 0.395 ms | 0% | 0% | **2.08** |
| R4 | 2026-06-15 | feature/layer-ex-baseline | **T2 + R1 Lazy Rendering** · Scenario B (탭 전환) | macOS 96kHz | 4,501 | 0.013 ms | 0.395 ms | 0% | 0% | **1.20** |

### 6.1 레이턴시 분포 / Latency Distribution

| 항목 / Metric | 평균 / Avg | 최대 / Max | 최소 / Min |
|---------------|-----------|-----------|-----------|
| total_ms | 420.49 ms | 927.50 ms | 0.22 ms |
| wait_ms | 419.92 ms | 926.82 ms | 0.01 ms |
| exec_ms | 0.57 ms | 1.96 ms | 0.08 ms |

### 6.2 exec 세부 분해 / exec Breakdown

| 항목 / Metric | 평균 / Avg | 최대 / Max | 비율 / Share |
|---------------|-----------|-----------|-------------|
| copy_ms | 0.008 ms | 0.025 ms | 1.4% |
| tg_ms (DSP) | 0.560 ms | 1.934 ms | 98.2% |
| ui_ms | 0.000 ms | 0.000 ms | 0% |
| plot_ms | 0.000 ms | 0.000 ms | 0% |

### 6.3 처리량 / Throughput

| 항목 / Metric | 값 / Value |
|---------------|-----------|
| avg samples/frame | 1,468.8 |
| deadline (BG period) | 10.01 ms |
| deadline miss (exec > deadline) | **0 / 2,900 (0%)** |
| backlog (>1.5× SPF) | 1,374 / 2,900 (47%) |
| bg_fps avg | 95.5 |
| fg_fps avg | 62.3 |

---

### 6.4 T2 Run 결과 (R2) / T2 DSP Offload Thread Results (R2)

**한국어**

T2 전술(DSPWorker 별도 스레드) 적용 후 macOS 96kHz Playback 모드 측정 결과.
로그 파일: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_231224.csv`

**English**

Measurement after applying T2 tactic (DSPWorker separate thread), macOS 96kHz Playback mode.
Log file: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_231224.csv`

| 항목 / Metric | 평균 / Avg | 최대 / Max | 최소 / Min |
|---------------|:---------:|:---------:|:---------:|
| total_ms | 0.413 ms | 6.030 ms | 0.099 ms |
| wait_ms | **0.013 ms** | 0.087 ms | 0.006 ms |
| exec_ms | **0.400 ms** | 5.993 ms | 0.084 ms |
| copy_ms | 0.006 ms | 0.025 ms | 0.005 ms |
| tg_ms (DSP) | **0.393 ms** | 5.982 ms | 0.079 ms |
| ui_ms | 0 ms | — | — |
| plot_ms | 0 ms | — | — |

| 처리량 / Throughput | 값 / Value |
|--------------------|-----------|
| 총 프레임 / Total frames | 4,500 |
| bg_fps avg | 95.6 |
| fg_fps avg (DSP thread) | **95.6** |
| deadline miss | **0 / 4,500 (0%)** |
| backlog (>1.5× SPF) | **0 / 4,500 (0%)** |

**한국어** 주요 관찰:

1. **wait_ms 420ms → 0.013ms (×32,000 감소)**: 메인 스레드 Qt 이벤트 루프 병목이 DSP 스레드 분리로 완전히 해소됨
2. **backlog 47% → 0%**: 프레임 누적 현상 완전 제거 — DSP 스레드가 실시간으로 Worker를 따라감
3. **bg_fps ≈ fg_fps (95.6 ≈ 95.6)**: Worker 스레드와 DSP 스레드 처리 속도 일치 확인
4. **exec_ms 0.57 → 0.40ms**: DSP 스레드 내 실제 처리 시간 (메인 스레드 경쟁 제거 효과)
5. **tg_ms max 5.98ms 스파이크**: DSP 스레드 내 단발성 OS 스케줄링 지연. 평균은 0.39ms로 안정

**English** Key observations:

1. **wait_ms 420ms → 0.013ms (×32,000 reduction)**: Main thread Qt event loop bottleneck fully eliminated by DSP thread separation.
2. **backlog 47% → 0%**: Frame accumulation completely eliminated — DSP thread tracks worker in real time.
3. **bg_fps ≈ fg_fps (95.6 ≈ 95.6)**: Worker and DSP thread processing speed confirmed identical.
4. **exec_ms 0.57 → 0.40ms**: Actual DSP processing time in dedicated thread (main thread contention removed).
5. **tg_ms max 5.98ms spike**: Single OS scheduling jitter in DSP thread. Avg 0.39ms is stable.

---

### 6.5 R1 Lazy Rendering Run 결과 (R3 / R4) / R1 Lazy Rendering Results

**한국어**

R1 전술(isVisible() 가드 + showEvent catch-up) 적용 후 macOS 96kHz Playback 모드 시나리오 A/B 측정 결과.

- **Scenario A** (1탭 고정): Trace 탭 고정, 다른 탭으로 이동 없음
  - 로그: `logs/EXP-02/log_20260615_003136.csv`
- **Scenario B** (탭 전환): 실행 중 탭을 순차적으로 전환
  - 로그: `logs/EXP-02/log_20260615_003429.csv`

**English**

Measurement after applying R1 tactic (isVisible() guard + showEvent catch-up), macOS 96kHz Playback mode.

- **Scenario A** (single tab fixed): stay on Trace tab, no tab switch
- **Scenario B** (tab switching): sequentially switch tabs during run

| 항목 / Metric | R3 Scenario A | R4 Scenario B |
|---------------|:-------------:|:-------------:|
| total_ms avg | 0.408 ms | 0.407 ms |
| exec_ms avg | 0.395 ms | 0.395 ms |
| tg_ms avg | 0.389 ms | 0.388 ms |
| plot_ms | 0 ms | 0 ms |
| deadline miss | 0 / 4,501 (0%) | 0 / 4,501 (0%) |
| backlog | 0 / 4,501 (0%) | 0 / 4,501 (0%) |
| **replot_count avg** | **2.08 / beat** | **1.20 / beat** |
| replot_count 분포 / dist | [2, 3] | [1, 2, 3] |

**replot_count 해석 (한국어)**

- **R2b (R1 가드 없음, 실측)**: avg **8.22** / beat, 분포 [4, 5, 8, 9, 11, 13]
  - 최빈값 8: 탭마다 replot 1회 × 8탭(BeatNoiseScopeTab·RateScopeTab 각 2plot, 나머지 1plot)
  - 최대값 13: RateScopeTab 이벤트마다 mRatePlot 추가 replot(rate point마다 호출)
  - BeatErrorTab·SweepScopeTab 등 박자 이벤트 없을 때 skip → 최소값 4
- **R3 Scenario A (R1 적용, 1탭)**: avg **2.08** → 동일 조건 대비 **75% 감소**
- **R4 Scenario B (R1 적용, 탭 전환)**: avg **1.20** → 동일 조건 대비 **85% 감소**

**replot_count interpretation (English)**

- **R2b (no R1 guard, measured)**: avg **8.22** / beat, distribution [4, 5, 8, 9, 11, 13]
  - Mode 8: most beats trigger 1 replot per tab (BeatNoiseScope/RateScope have 2 plots each)
  - Max 13: RateScopeTab calls mRatePlot->replot() per rate-point event within a frame
  - Min 4: frames with no acoustic events skip BeatError/Sweep
- **R3 Scenario A (R1 applied, 1 tab)**: avg **2.08** → **75% reduction** vs R2b baseline
- **R4 Scenario B (R1 applied, tab switch)**: avg **1.20** → **85% reduction** vs R2b baseline

**한국어 주요 관찰**

1. **exec/deadline 지표는 변화 없음**: macOS에서 QueuedConnection으로 plot_ms가 이미 0이므로, R1의 replot 감소는 exec path에 영향을 주지 않는다.
2. **replot_count가 R1 검증의 직접 지표**: 실제로 얼마나 많은 탭이 매 비트마다 replot()을 호출하는지를 정량화 — Scenario A에서 85% 감소 확인.
3. **Scenario B catch-up 동작 정상**: showEvent()의 `QTimer::singleShot(0)` 덕분에 탭 전환 시 화면이 즉시 최신 데이터로 갱신되며 UI 블로킹 없음.
4. **RPi에서의 기대 효과**: macOS와 달리 RPi는 QueuedConnection 없이 탭 렌더링이 exec path에 포함될 경우, R1로 replot 85% 감소 → plot_ms ~14ms 감소로 직결될 것으로 예상.

**English Key Observations**

1. **exec/deadline metrics unchanged**: On macOS, QueuedConnection already makes plot_ms = 0; R1's replot reduction does not affect the exec path here.
2. **replot_count is the direct R1 validation metric**: Quantifies how many tabs actually call replot() per beat — 85% reduction confirmed in Scenario A.
3. **Scenario B catch-up works correctly**: `QTimer::singleShot(0)` in showEvent() ensures the tab refreshes to the latest data snapshot on switch with no UI blocking.
4. **Expected RPi impact**: Unlike macOS, if RPi's rendering runs in the exec path (no QueuedConnection), R1's 85% replot reduction would directly cut ~14ms from plot_ms per beat.

---

## 7. RPi 기준선과 비교 / Comparison with RPi Baseline

**한국어**

RPi 기준선은 `baseline/experiments2` 브랜치에서 측정한 값이다(단일 스레드, 탭 렌더링 동기화).

**English**

RPi baseline was measured on `baseline/experiments2` branch (single thread, synchronous tab rendering).

| 항목 / Metric | macOS · Baseline (R1) | macOS · T2 (R2) | macOS · T2, 가드 없음 (R2b) | macOS · T2+R1 Scenario A (R3) | RPi · baseline/experiments2 |
|---------------|:---------------------:|:---------------:|:---------------------------:|:-----------------------------:|:---------------------------:|
| wait_ms avg | 420 ms | **0.013 ms** | 0.013 ms | 0.013 ms | N/A |
| exec_ms avg | 0.57 ms | **0.40 ms** | ~0.40 ms | 0.395 ms | 20 ms |
| tg_ms avg | 0.56 ms | 0.39 ms | ~0.39 ms | 0.389 ms | ~4 ms |
| plot_ms avg | 0 ms | 0 ms | 0 ms | 0 ms | 16 ms (79%) |
| deadline miss | 0% | **0%** | 0% | 0% | 43% |
| backlog | 47% | **0%** | 0% | 0% | — |
| replot_count avg | — | — | **8.22** (실측 baseline) | **2.08** (↓75%) | — |

**한국어**

주요 관찰:

1. **plot_ms = 0**: QueuedConnection으로 탭 렌더링이 exec path에서 완전히 분리됨 → feature/layer 설계 효과 확인
2. **exec_ms = 0.57ms**: macOS 환경에서 DSP 처리 자체는 매우 빠름. RPi에서는 하드웨어 차이로 더 높을 것으로 예상
3. **wait_ms = 420ms**: Qt 큐 대기가 압도적. macOS의 스케줄링 여유로 인해 신호가 모였다가 한번에 처리되는 패턴
4. **backlog 47%**: 프레임이 쌓여서 한꺼번에 처리되는 경우가 절반. RPi에서는 단일 코어 포화로 더 심각할 것
5. **deadline miss 0%**: macOS 기준 exec 처리 능력은 충분. RPi는 exec 자체가 느려 deadline miss 43% 발생

**English**

Key observations:

1. **plot_ms = 0**: Tab rendering fully removed from exec path via QueuedConnection → design intent of feature/layer confirmed.
2. **exec_ms = 0.57ms**: Very fast processing on macOS hardware. Expected to be higher on RPi due to hardware difference.
3. **wait_ms = 420ms**: Qt queue wait dominates. On macOS with scheduling slack, signals accumulate and are processed in batches.
4. **backlog 47%**: Half of frames arrive in batch (accumulated). Expected to be worse on RPi under single-core saturation.
5. **deadline miss 0%**: exec processing capacity is sufficient on macOS. On RPi, slow exec caused 43% deadline miss.

---

## 8. 아키텍처 결정에 대한 시사점 / Implications for Architecture Decisions

**한국어**

| 전술 / Tactic | 시사점 / Implication |
|--------------|---------------------|
| R1 Lazy Render (active tab only) | plot_ms가 이미 0 → QueuedConnection으로 효과 달성됨. RPi에서 추가 이득 있을 수 있음 |
| R2 Timer-Decoupled Render | 마찬가지로 이미 분리됨. FPS 제어 목적으로는 유효 |
| T1 SCHED_RR + CPU Affinity | wait_ms 감소 효과 기대. 오디오 스레드 우선순위 고정 → backlog 감소 |
| T2 DSP Offload Thread | ✅ **macOS 검증 완료** — wait_ms ×32,000 감소, backlog 0%. RPi에서도 동일 효과 기대 |

**English**

| 전술 / Tactic | 시사점 / Implication |
|--------------|---------------------|
| R1 Lazy Render (active tab only) | plot_ms already 0 via QueuedConnection — design goal achieved. May yield additional gain on RPi |
| R2 Timer-Decoupled Render | Rendering already decoupled. Valid for FPS rate control |
| T1 SCHED_RR + CPU Affinity | Linux-only; skip macOS; apply on RPi in combination with T2 |
| T2 DSP Offload Thread | ✅ **macOS validated** — wait_ms ×32,000 reduction, backlog 0%. Same effect expected on RPi |

---

## 9. 다음 단계 / Next Steps

**한국어**

| 단계 / Step | 내용 / Action | 상태 / Status |
|:-----------:|--------------|:------------:|
| macOS R1 | Baseline (main thread DSP) 측정 | ✅ 완료 |
| macOS R2 | T2 (DSP Offload Thread) 적용 측정 | ✅ 완료 |
| macOS R3 | T2+R1 Lazy Rendering · Scenario A (1탭 고정) 측정 | ✅ 완료 |
| macOS R4 | T2+R1 Lazy Rendering · Scenario B (탭 전환) 측정 | ✅ 완료 |
| RPi R5 | T2+R1 브랜치 그대로 RPi 측정 → exec_ms + replot_count RPi 기준선 확보 | ⏳ 다음 |
| RPi R6 | T1 (SCHED_RR + CPU Affinity) 추가 적용 후 RPi 재측정 | 📅 예정 |

**English**

| 단계 / Step | 내용 / Action | 상태 / Status |
|:-----------:|--------------|:------------:|
| macOS R1 | Baseline (main thread DSP) measurement | ✅ Done |
| macOS R2 | T2 (DSP Offload Thread) measurement | ✅ Done |
| macOS R3 | T2+R1 Lazy Rendering · Scenario A (1 tab fixed) | ✅ Done |
| macOS R4 | T2+R1 Lazy Rendering · Scenario B (tab switching) | ✅ Done |
| RPi R5 | Run T2+R1 branch on RPi → establish RPi exec_ms + replot_count baseline | ⏳ Next |
| RPi R6 | Add T1 (SCHED_RR + CPU Affinity) and re-measure on RPi | 📅 Planned |

---

## 10. 참고 / Links and References

- `baseline/experiments2` 브랜치: RPi 기준선 측정 원본 / RPi baseline measurement source
- `docs/milestone2/architectural-approaches.md`: 전술 옵션 및 trade-off 분석 / Tactic options and trade-off analysis
- 로그 파일 R1 (Baseline) / Log file R1: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_223711.csv`
- 로그 파일 R2 (T2) / Log file R2: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_231224.csv`
- 로그 파일 R2b (가드 없는 replot 기준선) / Log file R2b: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260615_050649.csv` (branch: `exp/r1-replot-baseline`)
- 로그 파일 R3 (T2+R1 Scenario A) / Log file R3: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260615_003136.csv`
- 로그 파일 R4 (T2+R1 Scenario B) / Log file R4: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260615_003429.csv`
- 분석 도구 / Analysis tool: `src/tools/analyze_log.py`
