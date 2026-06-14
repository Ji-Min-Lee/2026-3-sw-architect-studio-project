# EXP-02: E2E 오디오 파이프라인 레이턴시 기준선 측정 / EXP-02: E2E Audio Pipeline Latency Baseline Measurement

> **작성일 / Date**: 2026-06-14  
> **브랜치 / Branch**: `feature/layer-ex-baseline`  
> **상태 / Status**: Concluded

---

## 1. 결과 요약 / Results and Recommendations

**한국어**

macOS 환경(Playback 모드, 96kHz)에서 feature/layer 코드베이스의 E2E 레이턴시 기준선을 측정하였다.
exec_ms 평균 0.57ms로 처리 자체는 매우 빠르며, deadline miss 0%를 달성하였다.
대부분의 지연(wait_ms 평균 420ms)은 오디오 스레드 → 메인 스레드 간 Qt 큐 대기에서 발생하였다.
RPi 기준선(EXP-02, baseline/experiments2)과 비교하여 feature/layer의 QueuedConnection 분리가 plot 부하(16ms)를 exec path에서 제거함을 확인하였다.

**English**

Measured the E2E latency baseline of the feature/layer codebase on macOS (Playback mode, 96kHz).
Average exec_ms of 0.57ms confirms very fast processing; deadline miss rate is 0%.
The dominant delay (avg wait_ms = 420ms) originates from Qt queue latency between the audio thread and the main thread.
Compared to the RPi baseline (EXP-02, baseline/experiments2), the QueuedConnection separation in feature/layer confirms that the 16ms plot burden has been removed from the exec path.

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

## 7. RPi 기준선과 비교 / Comparison with RPi Baseline

**한국어**

RPi 기준선은 `baseline/experiments2` 브랜치에서 측정한 값이다(단일 스레드, 탭 렌더링 동기화).

**English**

RPi baseline was measured on `baseline/experiments2` branch (single thread, synchronous tab rendering).

| 항목 / Metric | macOS · feature/layer | RPi · baseline |
|---------------|----------------------|----------------|
| exec_ms avg | **0.57 ms** | 20 ms |
| tg_ms avg | 0.56 ms | ~4 ms |
| plot_ms avg | **0 ms** | 16 ms (79%) |
| deadline miss | **0%** | 43% |
| wait_ms avg | 420 ms | 측정 안 됨 / N/A |
| backlog | 47% | - |

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
| T2 DSP Offload Thread | tg_ms(0.56ms)가 exec의 98%. 별도 스레드로 분리 시 메인 스레드 응답성 개선 |

**English**

| 전술 / Tactic | 시사점 / Implication |
|--------------|---------------------|
| R1 Lazy Render (active tab only) | plot_ms already 0 via QueuedConnection — design goal achieved. May yield additional gain on RPi |
| R2 Timer-Decoupled Render | Rendering already decoupled. Valid for FPS rate control |
| T1 SCHED_RR + CPU Affinity | Expected to reduce wait_ms by pinning audio thread priority → reduce backlog |
| T2 DSP Offload Thread | tg_ms (0.56ms) is 98% of exec. Moving to dedicated thread improves main-thread responsiveness |

---

## 9. 다음 단계 / Next Steps

**한국어**

1. RPi에서 feature/layer-ex-baseline 동일 측정 실행 → RPi exec_ms 기준선 확보
2. T1(SCHED_RR + CPU Affinity) 적용 후 재측정 → wait_ms / backlog 감소 확인
3. T2(DSP Offload Thread) 적용 후 재측정 → exec 분리 효과 확인
4. R1 또는 R2 적용 후 RPi 재측정 → rendering 전술 선택 결정

**English**

1. Run same measurement on RPi with feature/layer-ex-baseline → establish RPi exec_ms baseline
2. Apply T1 (SCHED_RR + CPU Affinity) and re-measure → verify wait_ms / backlog reduction
3. Apply T2 (DSP Offload Thread) and re-measure → verify exec separation effect
4. Apply R1 or R2 on RPi and re-measure → decide rendering tactic

---

## 10. 참고 / Links and References

- `baseline/experiments2` 브랜치: RPi 기준선 측정 원본 / RPi baseline measurement source
- `docs/milestone2/architectural-approaches.md`: 전술 옵션 및 trade-off 분석 / Tactic options and trade-off analysis
- 로그 파일 / Log file: `build/TimeGrapher.app/Contents/MacOS/logs/EXP-02/log_20260614_223711.csv`
- 분석 도구 / Analysis tool: `src/tools/analyze_log.py`
