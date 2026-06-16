# 아키텍처 접근 방안 / Architectural Approaches

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [x] Draft  [ ] Final  
**Last Updated**: 2026-06-13

> **한국어**  
> EXP-02 실험 결과(RPi 실시간 실패)를 기반으로, QAS-1(실시간 성능) · QAS-2(저지연) 달성을 위한 아키텍처 대안을 정의하고 트레이드오프를 비교한다.  
> 두 개의 독립적 설계 결정(Decision)으로 나누어 각각 3개 옵션을 제시한다.  
> 각 Decision은 독립적으로 선택 가능하며, 최종 선택 조합이 Construction Plan에 반영된다.
>
> **English**  
> Based on EXP-02 results (RPi real-time FAIL), this document defines and compares architectural alternatives to meet QAS-1 (Real-Time Performance) and QAS-2 (Low Latency).  
> Two independent architectural decisions are separated, each with three options.  
> Each decision can be selected independently; the chosen combination is reflected in the Construction Plan.

---

## 배경 / Background

### 확인된 문제 / Confirmed Problem (EXP-02 R2 — RPi Baseline)

**한국어**

| 지표 / Metric | 값 / Value | 의미 / Implication |
|--------------|:---------:|-------------------|
| exec 평균 | 20 ms | deadline(21 ms) 대비 95% 소모 |
| `plot` 비중 | 16 ms | exec의 **79%** — 렌더링이 병목 |
| deadline 초과 | 43 % | 1015 프레임 중 441회 실패 |
| cpu2 점유율 | 91 % | 오디오 파이프라인이 단일 코어에 집중 |
| 온도 | 85 °C | 열 제한(throttle) 지속 → 클럭 저하 |
| 나머지 코어 | idle | 멀티코어 미활용 |

**English**

The bottleneck is **structural**, not algorithmic. `plot` rendering alone consumes 79% of the exec budget; the audio pipeline is pinned to a single core while others idle; thermal throttling compounds the problem. Algorithm tuning alone cannot fix this.

### 루트 원인 구조 / Root Cause Structure

```
[Root Cause 1] 렌더링이 오디오 경로에 결합
  AudioThread exec 안에서 모든 탭의 plot 수행
  → plot 16ms × N_tabs = exec 초과

[Root Cause 2] 단일 코어 집중
  AudioCapture + DSP + Qt Event Loop → cpu2 91%
  → 나머지 3 코어 미활용

[Root Cause 3] 열 제한
  85°C → 클럭 저하 → 실질 성능 추가 하락
```

---

## Decision 1: 렌더링 전략 / Rendering Strategy

> **한국어** `plot`이 exec deadline을 초과하는 문제 해결.  
> **English** Addresses the `plot` overrun of exec deadline.

### 옵션 비교 / Option Comparison

| 항목 / Item | Option R1: Lazy Rendering | Option R2: Timer-Decoupled Rendering | Option R3: Double-Buffer Async Rendering |
|------------|:-------------------------:|:------------------------------------:|:----------------------------------------:|
| 핵심 전술 | 보이는 탭만 repaint | 고정 FPS 타이머로 repaint 분리 | 렌더 결과를 QPixmap에 저장 후 UI에서 합성 |
| 구현 복잡도 | **낮음** | 중간 | **높음** |
| exec 절감 효과 | 높음 (탭 1개 기준) | 중간 (전체 FPS 제한) | **매우 높음** (오디오 경로와 완전 분리) |
| 11탭 전부 열렸을 때 | 1탭만 렌더 | N_active × (1/FPS) | 비동기 → 오디오 경로 영향 없음 |
| 데이터 일관성 위험 | 없음 | 없음 | QPixmap 공유 시 락 필요 |
| M2 deadline 적합성 | ✅ 즉시 적용 가능 | ✅ 적용 가능 | ⚠️ 설계 변경 크고 시간 부족 |

---

### Option R1: Lazy Rendering (Active Tab Only)

**한국어**  
현재 활성(visible) 탭만 매 beat에 repaint. 비활성 탭은 data만 업데이트하고 그리지 않는다.

**English**  
Only the currently visible tab repaints each beat. Inactive tabs update their data model but skip rendering.

```
BeatEvent
  └→ GraphTabManager
       ├→ ActiveTab.updateData(m)
       │    └→ update() → paintEvent()  ← 렌더 발생
       └→ InactiveTabs.updateData(m)    ← 렌더 없음 (isVisible() guard)
```

**구현 변경 범위:**

```cpp
// 각 탭 updateData() 진입부에 추가
void TraceDisplay::updateData(const Measurement& m) {
    store(m);
    if (!isVisible()) return;   // ← 이 한 줄
    update();
}
```

| 장점 / Pros | 단점 / Cons |
|-------------|-------------|
| 변경 범위 최소 (탭당 1줄 guard) | 탭 전환 시 stale 데이터 → catch-up 렌더 필요 |
| exec에서 plot 완전 제거(비활성 탭) | 11탭 모두 active 상태면 효과 없음 |
| QAS-5 구조 변경 없음 | 백그라운드 탭 "멈춘 것처럼" 보임 |

**QAS 연관:**  
- QAS-2: ② exec 16ms → ~1ms (비활성 10탭 plot 제거)  
- QAS-5: 탭 추가 시 guard 패턴 동일 적용 → ≤ 3 파일 유지

---

### Option R2: Timer-Decoupled Rendering

**한국어**  
렌더링 주기를 beat 이벤트에서 분리. Qt 타이머(20 FPS)가 `update()`를 주기적으로 발생시키고, beat 이벤트는 데이터 모델만 업데이트한다.

**English**  
Decouple rendering from beat events. A Qt timer (20 FPS) drives `update()`; beat events only update the data model.

```
BeatEvent → updateData(m) → store in tab's data model (no repaint)
QTimer(50ms) → all visible tabs → update() → paintEvent()
```

| 장점 / Pros | 단점 / Cons |
|-------------|-------------|
| 오디오 경로에서 렌더 타이밍 완전 분리 | 타이머 관리 필요 (start/stop with pipeline) |
| FPS 상한이 명확하게 제어됨 | BPH 480(8 BPS)에서 20 FPS 타이머가 과잉 렌더 가능 |
| 모든 탭이 일정 간격으로 업데이트됨 | R1보다 코드 변경 범위 넓음 (TabManager + 탭) |

**QAS 연관:**  
- QAS-2: exec 내 plot 제거 → deadline 초과율 대폭 감소  
- 장기적으로 R1보다 구조 명확

---

### Option R3: Double-Buffer Async Rendering

**한국어**  
각 탭이 DSP 결과를 QPixmap에 비동기로 렌더링하고, UI 스레드는 완성된 Pixmap만 합성(blit). 오디오 경로는 렌더링 시간 영향을 전혀 받지 않는다.

**English**  
Each tab renders into a QPixmap asynchronously; the UI thread only blits the finished pixmap. The audio path is completely isolated from render timing.

```
Audio Thread: BeatEvent → DataModel (lock-free write)
Render Worker: DataModel (read) → QPixmap (off-screen)
UI Thread: QPixmap blit → paintEvent (microseconds)
```

| 장점 / Pros | 단점 / Cons |
|-------------|-------------|
| 오디오 경로 ↔ 렌더링 완전 분리 | QPixmap은 UI 스레드에서만 생성 가능 → 설계 재검토 필요 |
| 최대 exec 절감 가능 | 3개 스레드 동기화, 데이터 경쟁 위험 |
| 고 BPH에서도 렌더 부하 무관 | M2 deadline(06/22) 내 구현 위험 높음 |

---

## Decision 2: 스레딩 전략 / Threading Strategy

> **한국어** 단일 코어(cpu2) 포화 및 멀티코어 미활용 문제 해결.  
> **English** Addresses single-core (cpu2) saturation and idle multi-core problem.

### 현재 구조 / Current Structure

```
cpu2 (Audio Thread):
  AudioCapture → SignalBuffer → FilterChain → BeatDetector → MeasurementEngine
                                                                     ↓ Qt queued signal
Qt Main Thread (any core):
  GraphTabManager → 11 Graph Tabs → paintEvent()
```

### 옵션 비교 / Option Comparison

| 항목 / Item | Option T1: SCHED_RR + CPU Affinity | Option T2: DSP Offload Thread | Option T3: Pipeline Thread Split |
|------------|:----------------------------------:|:-----------------------------:|:---------------------------------:|
| 핵심 전술 | OS 스케줄링 우선순위 + 코어 고정 | AudioCapture ↔ DSP 분리 | Capture / Filter / Detect 각각 분리 |
| 코어 분산 | 부분적 (캡처 코어 분리) | 중간 (캡처 1코어 + DSP 1코어) | **높음** (단계별 코어 분산) |
| 구현 복잡도 | **낮음** (OS API만) | 중간 | **높음** |
| exec 절감 직접 효과 | 없음 (스케줄링 개선만) | 중간 | 높음 |
| 모듈 구조 변경 | 없음 | SignalBuffer 추가 | 파이프라인 재설계 |
| M2 deadline 적합성 | ✅ 즉시 | ✅ 적용 가능 | ⚠️ 위험 |

---

### Option T1: SCHED_RR + CPU Affinity

**한국어**  
오디오 스레드에 SCHED_RR(실시간 스케줄링)을 적용하고, cpu_set을 사용해 특정 코어(예: cpu0)에 고정. 다른 코어는 Qt 이벤트 루프와 렌더링이 사용하도록 분리.

**English**  
Apply SCHED_RR to the audio thread and pin it to a dedicated core (e.g., cpu0) using `cpu_set`. Other cores handle the Qt event loop and rendering.

```cpp
// 오디오 스레드 시작 시
struct sched_param param;
param.sched_priority = 80;
pthread_setschedparam(pthread_self(), SCHED_RR, &param);

cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(0, &cpuset);  // cpu0에 고정
pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
```

| 장점 / Pros | 단점 / Cons |
|-------------|-------------|
| 코드 변경 최소 (스레드 초기화부만) | exec 절감 없음 — plot 병목 그대로 |
| QAS-1(Dropped Block) 개선 가능 | cpu2 91% 문제의 근본 해결 아님 |
| EXP-01에서 즉시 검증 가능 | thermal throttle 해결 못함 |

**QAS 연관:**  
- QAS-1: OS jitter 흡수 → Dropped Block 감소 기대  
- QAS-2: exec 병목은 그대로 → R1/R2 렌더링 전략과 반드시 병행 필요

---

### Option T2: DSP Offload Thread (Audio ↔ DSP Separation)

**한국어**  
AudioCapture와 DSP 파이프라인(FilterChain → BeatDetector → MeasurementEngine)을 별도 스레드로 분리. 두 스레드 사이에 PCM 링 버퍼를 추가.

**English**  
Separate `AudioCapture` (audio callback) from the DSP pipeline (FilterChain → BeatDetector → MeasurementEngine) into two threads connected by a PCM ring buffer.

```
cpu0 (Capture Thread, SCHED_RR):
  AudioCapture → [PCM Ring Buffer write]

cpu1 (DSP Thread):
  [PCM Ring Buffer read] → FilterChain → BeatDetector → MeasurementEngine
                                                               ↓ Qt queued signal

Qt Main Thread (any core):
  GraphTabManager → Tabs
```

| 장점 / Pros | 단점 / Cons |
|-------------|-------------|
| Capture 스레드를 경량 유지 (write only) | Ring Buffer 추가 → 설계 변경 |
| DSP 코어 분산 → cpu2 포화 해소 | PCM Ring Buffer와 기존 SignalBuffer 역할 중복 위험 |
| SCHED_RR을 Capture에만 좁게 적용 가능 | 스레드 간 동기화 설계 필요 |

**구조 변경 범위:**  
- `SignalBuffer` 역할 분화 (capture→DSP용 PCM buffer 추가)  
- `AudioCapture`: write만 수행하도록 callback 경량화  
- `DSPThread`: 새 워커 스레드로 FilterChain~ME 포함

**QAS 연관:**  
- QAS-1: Capture 스레드 경량화 → Dropped Block 위험 감소  
- QAS-2: DSP 코어 분산 → exec 시간 개선 + 렌더링 전략과 시너지

---

### Option T3: Full Pipeline Thread Split

**한국어**  
각 파이프라인 단계(Capture / Filter / Detect / Measure)를 별도 스레드로 분리, 코어별 고정 배분. 완전한 파이프라인 병렬성.

**English**  
Separate each pipeline stage (Capture / Filter / Detect / Measure) into individual threads pinned to separate cores. Full pipeline parallelism.

```
cpu0: AudioCapture (SCHED_RR)
cpu1: FilterChain
cpu2: BeatDetector + MeasurementEngine
cpu3: Qt Event Loop + Rendering
```

| 장점 / Pros | 단점 / Cons |
|-------------|-------------|
| 이론적 최대 코어 활용 | 스테이지 간 큐 × 3 추가 → 지연 증가 가능 |
| thermal 부하 분산 → throttle 완화 기대 | 설계 복잡도 대폭 증가 |
| QAS-1 / QAS-2 동시 달성 가능성 | M2(06/22) 내 구현 위험 매우 높음 |
|  | 단계별 latency가 pipeline depth만큼 증가 |

---

## 통합 트레이드오프 매트릭스 / Combined Trade-off Matrix

**한국어** 각 조합의 핵심 트레이드오프를 요약한다.  
**English** Summary of key trade-offs for each combination.

| 조합 / Combo | exec 절감 | 코어 분산 | 구현 복잡도 | M2 위험 | 권장 대상 |
|:------------:|:---------:|:---------:|:-----------:|:-------:|----------|
| **R1 + T1** | ★★★★☆ | ★★☆☆☆ | ★☆☆☆☆ | **낮음** | 빠른 검증, MVP |
| **R1 + T2** | ★★★★☆ | ★★★★☆ | ★★★☆☆ | 중간 | 균형 잡힌 접근 |
| **R2 + T1** | ★★★☆☆ | ★★☆☆☆ | ★★☆☆☆ | **낮음** | 렌더 타이밍 제어 우선 |
| **R2 + T2** | ★★★★★ | ★★★★☆ | ★★★★☆ | 중간 | 구조 완성도 우선 |
| R3 + T3 | ★★★★★ | ★★★★★ | ★★★★★ | **높음** | M3 이후 검토 |

### 핵심 트레이드오프 정리 / Key Trade-off Summary

```
빠른 MVP vs 구조 완성도
  R1+T1 → 변경 최소, EXP-01/02 즉시 재실험 가능
  R2+T2 → 더 나은 설계 기반, 하지만 구현 시간 더 필요

렌더링 분리 방식
  R1(Lazy) → 비활성 탭만 이득, 11탭 동시 오픈 시 한계
  R2(Timer) → FPS 상한 명확, 모든 탭 일정하게 업데이트

DSP 분리 여부
  T1만으로는 exec 병목 미해결 → 반드시 R1 또는 R2와 병행 필요
  T2는 Capture 스레드를 경량화해 QAS-1 Dropped Block 동시 개선
```

---

## 미결 실험과의 연관 / Link to Pending Experiments

**한국어**  
아래 실험 결과가 최종 옵션 선택에 영향을 준다.

**English**  
The following experiment results will influence the final option selection.

| 실험 / Experiment | 영향 / Impact on Decision |
|-------------------|--------------------------|
| **EXP-01** (SPS 확정) | T1/T2의 SCHED_RR 효과 검증; QAS-1 baseline 확보 |
| **EXP-02** (11탭 RPi 재실험) | R1 vs R2 선택 근거 — 11탭 동시 오픈 시 plot 비용 확인 |
| **EXP-03** (파라미터 최적화) | T2 DSP 분리 후 BeatDetector exec 시간 변화 확인 |

---

## 다음 단계 / Next Steps

**한국어**

1. 이 문서를 기반으로 팀 내 옵션 검토 및 Decision 확정
2. 확정 조합을 `Architecture Decisions Log` (experiment-results.md)에 기록
3. 선택된 전술을 Module View / Runtime View에 반영
4. Construction Plan Phase A에 구현 태스크 추가

**English**

1. Review options as a team and finalize Decision 1 + Decision 2 choices
2. Record chosen combination in `Architecture Decisions Log` (experiment-results.md)
3. Reflect selected tactics in Module View / Runtime View
4. Add implementation tasks to Construction Plan Phase A

---

## 리뷰 체크리스트 / Review Checklist

- [ ] 두 Decision이 독립적으로 선택 가능한가?
- [ ] 각 옵션의 트레이드오프가 명확히 기술되었는가?
- [ ] EXP-01/02 실험 결과와 옵션 선택 연결이 명확한가?
- [ ] M2 deadline(06/22) 내 구현 가능성이 반영되었는가?
- [ ] QAS-1(Real-Time), QAS-2(Low Latency)와의 연관이 추적 가능한가?
