# 아키텍처 접근법 / Architectural Approaches

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09 | **상태 / Status**: [ ] 초안 Draft / [ ] 최종 Final

---

## 1. 아키텍처 개요 / Architecture Overview

**한국어**

시스템은 5단계 파이프라인 구조로 구성된다. 음향 신호 수집부터 GUI 표시까지 데이터가 단방향으로 흐르며, 신호 처리는 백그라운드 스레드에서, GUI 렌더링은 Qt 메인 스레드에서 분리 실행된다. 계산 레이어와 표시 레이어 사이는 Qt Signal-Slot(Observer)으로 연결되어 둘 사이의 결합을 끊는다.

**English**

The system is structured as a 5-stage pipeline. Data flows unidirectionally from audio capture to GUI display. Signal processing runs on a background thread; GUI rendering runs on the Qt main thread. The calculation layer and visualization layer are decoupled via Qt Signal-Slot (Observer).

```
                        ┌─────────────── Background Thread (Audio/DSP) ────────────────┐
[Watch + Microphone]
        ↓ USB audio (PCM)
[Signal Acquisition]  ←→  AlsaMixer (AGC disabled)          ← AudioCapture module
        ↓ raw PCM blocks
[Signal Processing]   ←  LP/HP Filter chain                  ← SignalProcessor module
        ↓ filtered signal + envelope
[Beat Event Detection]  →  T1 (A), T3 (C) timestamps         ← MeasurementEngine module
        ↓ beat events
[Measurement Calculation]  →  Rate, Amplitude, Beat Error, BPH  ← MeasurementEngine module
        └─────────────────────────────────────────────────────────────────────────────────┘
                        Qt Signal-Slot QueuedConnection (thread-safe crossing)
                        ↓ MeasurementResult struct (emitted signal)
                        ┌─────────────── Main Thread (Qt GUI) ──────────────────────────┐
[Visualization Layer]  →  GraphView 인터페이스 구현체 (탭별 독립 위젯)
        ↓
[Qt GUI / Main Window]  →  터치스크린 (1280×800)
                        └───────────────────────────────────────────────────────────────┘

운영 모드 / Operating Modes: Live | Playback | Sim
```

---

## 2. 주요 아키텍처 접근법 / Key Architectural Approaches

### 2.1 패턴: 파이프-필터 / Pattern: Pipe-and-Filter

**한국어**

신호 수집 → 필터링 → 감지 → 계산의 4단계를 독립 스테이지로 구성한다. 각 스테이지는 단일 책임을 가지며, 특정 스테이지를 교체하거나 재순서화해도 인접 스테이지에 영향이 없다. 이 패턴은 데이터 흐름의 처리 순서를 규정하는 **런타임 구조**에 해당한다.

**English**

The four stages — acquisition → filtering → detection → calculation — are structured as independent pipe stages. Each stage has a single responsibility; replacing or reordering a stage does not affect adjacent ones. This pattern governs the **runtime processing sequence**.

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **적용 범위 / Applied To** | Signal Acquisition → Processing → Detection → Calculation |
| **근거 / Rationale** | 단계별 교체·테스트 가능; EX-03(`tg_c_placement_t` 비교) 시 Detection 스테이지만 변경 / Each stage independently replaceable; EX-03 only modifies the Detection stage |
| **트레이드오프 / Trade-off** | 단계 버퍼링으로 레이턴시 증가 → 버퍼 크기 최소화로 보완 / Stage buffering adds latency — mitigated by minimizing buffer sizes |
| **연결 드라이버 / Linked Driver** | QAS-2 (Low Latency), QAS-4 (Extensibility) |

---

### 2.2 패턴: 옵저버 / Pattern: Observer (Qt Signal-Slot)

**한국어**

계산 레이어가 `MeasurementResult`를 Qt Signal로 발행하면, 각 그래프 탭(GraphView 구현체)이 슬롯을 통해 독립적으로 구독한다. 새 탭 추가 시 계산 코드는 변경하지 않는다.

**English**

The calculation layer emits `MeasurementResult` as a Qt Signal. Each graph tab (a `GraphView` implementation) subscribes independently via its slot. Adding a new tab requires no changes to the calculation code.

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **적용 범위 / Applied To** | Measurement Calculation → Visualization Layer (모든 그래프 탭) |
| **근거 / Rationale** | 11개 탭을 개별 파일로 독립 개발 가능; 머지 충돌 최소화 / All 11 tabs developed in separate files; minimizes merge conflicts (TR-05) |
| **트레이드오프 / Trade-off** | 간접 참조 증가; 이 규모에서는 허용 가능 / Slightly more indirection; acceptable at this scale |
| **연결 드라이버 / Linked Driver** | QAS-4 (Extensibility), QAS-5 (Correctness) |

---

### 2.3 전술: 동시성 — 스레드 분리 / Tactic: Concurrency — Thread Separation

**한국어**

신호 수집·처리·계산은 백그라운드 스레드에서, Qt GUI 렌더링은 메인 스레드에서 실행한다. 두 스레드 간 데이터 전달은 Qt Signal-Slot의 `QueuedConnection`을 사용해 thread-safe하게 처리한다. 이로써 GUI 지연이 오디오 처리를 블로킹하지 않는다.

**English**

Audio capture, processing, and calculation run on a background thread. Qt GUI rendering runs on the main thread. Cross-thread data passing uses Qt Signal-Slot `QueuedConnection` for thread safety. This prevents GUI lag from blocking audio processing.

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **적용 범위 / Applied To** | AudioCapture, MeasurementEngine (백그라운드); GraphView, MainWindow (메인) |
| **근거 / Rationale** | GUI 프레임 드롭이 beat 미감지로 이어지지 않도록 격리 / Isolates GUI frame drops from missed beats |
| **트레이드오프 / Trade-off** | 스레드 동기화 복잡도 증가; QueuedConnection으로 관리 / Thread synchronization complexity — managed via QueuedConnection |
| **연결 드라이버 / Linked Driver** | QAS-1 (Real-Time Performance), QAS-2 (Low Latency) |

---

### 2.4 전술: 레이어드 아키텍처 — God Object 분해 / Tactic: Layered Architecture — God Object Decomposition

**한국어**

현재 `MainWindow.cpp`(1,540줄, 61 메서드)가 수집·처리·계산·표시를 모두 담당하는 God Object다(TR-05). 이를 4개 레이어로 분해하여 각 레이어가 하나의 책임만 갖게 한다. 이 전술은 Pipe-and-Filter가 규정하는 런타임 데이터 흐름과 달리, **모듈 분류 및 의존성 방향**을 규정하는 정적 구조에 해당한다.

**English**

The current `MainWindow.cpp` (1,540 lines, 61 methods) is a God Object responsible for acquisition, processing, calculation, and display (TR-05). Decomposing it into 4 layers gives each layer a single responsibility. Unlike Pipe-and-Filter which governs runtime data flow, this tactic governs **static module structure and dependency direction**.

```
┌─────────────────────────────────────────────────────┐
│  Presentation Layer — GraphView (인터페이스)           │  각 탭이 독립 구현 / Each tab is independent impl
│    TraceDisplayView, BeatErrorView, SpectrogramView  │  MainWindow는 탭 컨테이너만 담당
├─────────────────────────────────────────────────────┤
│  Domain Layer — MeasurementEngine                    │  Rate, Amplitude, Beat Error, BPH 계산
│    Detector, RateCalculator, AmplitudeCalculator     │  단위 테스트 가능
├─────────────────────────────────────────────────────┤
│  Processing Layer — SignalProcessor                  │  LP/HP 필터, envelope 추출
│    LowPassFilter, HighPassFilter, EnvelopeExtractor  │  파라미터만 교체하면 독립 테스트 가능
├─────────────────────────────────────────────────────┤
│  Acquisition Layer — AudioCapture                    │  ALSA 수집, AGC 비활성화 확인 (TR-02)
│    AlsaCapture, PlaybackReader, SimGenerator         │  운영 모드 전환 책임
└─────────────────────────────────────────────────────┘
```

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **적용 범위 / Applied To** | 전체 모듈 구조 / Overall module structure |
| **근거 / Rationale** | 새 그래프 추가 시 Presentation Layer 파일만 추가; 하위 레이어 수정 없음 / New graph = add Presentation file only; no changes below |
| **확장성 목표 / Extensibility Target** | 새 그래프 1개 추가 시 변경 파일 ≤ 3개 (QAS-4) / ≤ 3 files changed per new graph |
| **트레이드오프 / Trade-off** | 초기 설계 규율 필요; 레이어 경계 확정 전 구현 시작 시 효과 없음 / Requires up-front discipline; ineffective if implementation starts before boundaries are locked |
| **연결 드라이버 / Linked Driver** | QAS-4 (Extensibility), QAS-3 (Correctness — 단일 계산 소스) |

---

### 2.5 전술: 우아한 성능 저하 / Tactic: Graceful Degradation

**한국어**

RPi 5의 실제 처리 능력이 확인되기 전까지, 샘플레이트를 3단계로 계획한다. 각 단계에서 시스템은 정상 동작을 유지하되 측정 해상도만 낮아진다. EX-01 결과에 따라 실제 목표 sps가 확정된다.

**English**

Until RPi 5 real-world capacity is verified via EX-01, three sample-rate tiers are planned. The system maintains normal operation at each tier; only measurement resolution decreases. EX-01 results confirm the actual target sps.

| 단계 / Tier | 샘플레이트 / Sample Rate | 상태 / Status |
|------------|----------------------|--------------|
| Stretch | 192,000 sps | RPi 부하 실측 후 결정 / Decided after EX-01 |
| **Objective** | **96,000 sps** | **목표 / Target** |
| Minimum | 48,000 sps | 이 이하면 프로젝트 실패 / Below this = project failure |

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **적용 범위 / Applied To** | AudioCapture — ALSA 버퍼 및 샘플레이트 설정 |
| **연결 드라이버 / Linked Driver** | QAS-1 (Real-Time Performance) |
| **연결 실험 / Linked Experiment** | EX-01 |

---

## 3. 아키텍처 ↔ 드라이버 매핑 / Architecture ↔ Driver Mapping

**한국어**

각 QA 드라이버가 어떤 아키텍처 접근법으로 지원되는지, 그리고 현재 지원이 충분한지를 함께 명시한다.

**English**

For each QA driver, the table lists which architectural approach supports it and whether the current support is sufficient.

| 드라이버 / Driver | 아키텍처 접근법 / Approach | 지원 강도 / Coverage | 비고 / Note |
|-----------------|--------------------------|:-------------------:|------------|
| QAS-1 Real-Time Performance | 스레드 분리 (2.3), Graceful Degradation (2.5) | 부분 / Partial | EX-01 실측 전까지 미확정 |
| QAS-2 Low Latency | Pipe-and-Filter 최소 버퍼 (2.1), 스레드 분리 (2.3) | 부분 / Partial | 레이턴시 예산 §4 참조 |
| QAS-3 Measurement Accuracy | 레이어드 구조 단일 계산 소스 (2.4) + **T1 timing 기준점 선택 (EX-03)** + **WeiShi 측정 환경 (EX-02)** | 약함 / Weak | 정확도는 구조보다 알고리즘 선택에 의존 — EX-02/EX-03 결과 후 재평가 |
| QAS-4 Extensibility | Observer/Signal-Slot (2.2), 레이어드 구조 (2.4) | 충분 / Strong | GraphView 인터페이스로 탭 독립 추가 보장 |
| QAS-5 Correctness | Observer 단일 신호 발행 (2.2), MeasurementEngine 단일 소스 (2.4) | 충분 / Strong | 구조적으로 보장 — 모든 뷰가 동일 Signal 구독 |

---

## 4. 레이턴시 예산 분석 / Latency Budget Analysis

**한국어**

QAS-2 요구사항(end-to-end < 100 ms)을 스테이지별로 분배한다. 아래 수치는 EX-01 실측 전 잠정값이며, 실험 결과에 따라 조정된다.

**English**

QAS-2 (end-to-end < 100 ms) is broken down per pipeline stage. Values below are tentative pending EX-01 measurements and will be revised after results.

```
capture → process  (백그라운드 스레드 / Background thread)
─────────────────────────────────────────────────────
[ALSA 버퍼 수집 / ALSA buffer capture]       ~20 ms  ← 버퍼 크기 설정으로 제어 가능
[LP/HP 필터 처리 / Filter processing]         ~10 ms
[Beat event 감지 / Beat detection]            ~10 ms
[Rate/Amplitude/BE 계산 / Calculation]         ~5 ms
─────────────────────────────────────────────────────
소계 / Subtotal                               ~45 ms  (목표 / target < 70 ms)

process → display  (메인 스레드 / Main thread)
─────────────────────────────────────────────────────
[GUI 이벤트 루프 픽업 대기 / GUI event-loop pickup]  ~25 ms  ← GUI 렌더 부하 의존
[Qt Signal-Slot 디스패치 / dispatch]                  ~5 ms   ← QueuedConnection 자체는 μs 단위
[GraphView 렌더링 / rendering]                        ~25 ms
─────────────────────────────────────────────────────
소계 / Subtotal                               ~55 ms  (목표 / target < 30 ms → EX-01로 실측 검증)

End-to-End 합계                              ~100 ms  (28,800 BPH 기준 한 beat 주기 이내)
```

**한국어**

28,800 BPH 시계의 beat 주기는 208 ms다. End-to-end 100 ms는 한 beat 주기의 절반 이내로, 다음 beat 처리 전에 현재 beat 표시가 완료됨을 의미한다. process→display 소계(~55 ms)가 목표(< 30 ms)를 초과하므로, EX-01에서 실측 후 렌더링 전략(lazy rendering, render timer 주기 조정 등)을 결정한다.

**English**

A 28,800 BPH watch has a beat period of 208 ms. End-to-end 100 ms is within half a beat period. The process→display subtotal (~55 ms) exceeds the target (< 30 ms); EX-01 empirical measurement will inform the rendering strategy (lazy rendering, render timer interval tuning, etc.).

---

## 5. 핵심 인터페이스 계약 / Key Interface Contracts

**한국어**

Observer 패턴과 레이어드 구조가 실제로 동작하려면 두 인터페이스의 계약이 확정되어야 한다. 아래 정의를 기준으로 코딩팀이 탭을 독립 구현한다. `t1_ms` / `t3_ms` 필드는 EX-03 결과(`tg_c_placement_t` 설정 확정)에 따라 조정될 수 있다.

**English**

For the Observer pattern and layered structure to function, two interface contracts must be fixed. Coding team implements each tab independently against these definitions. The `t1_ms` / `t3_ms` fields may be adjusted after EX-03 results confirm the optimal `tg_c_placement_t` setting.

### 5.1 MeasurementResult 구조체 / MeasurementResult Struct

```cpp
struct MeasurementResult {
    // 측정값 / Measurements
    double  rate_spd;          // Rate (s/d) — WeiShi 오차 < 5 s/d 목표
    double  amplitude_deg;     // Amplitude (°)
    double  beatError_ms;      // Beat Error (ms)
    int     bph;               // Beats Per Hour

    // 이벤트 타이밍 / Event timestamps (백그라운드 스레드 기준 ms)
    double  t1_ms;             // T1 (A 이벤트) — onset 또는 peak, EX-03으로 확정
    double  t3_ms;             // T3 (C 이벤트) — onset 또는 peak, EX-03으로 확정

    // 파형 데이터 / Waveform data (Scope 계열 탭용)
    QVector<float> processedPcm;   // HPF + envelope 처리 완료 샘플
    float          onsetThreshold; // 현재 감지 임계값

    // 메타 / Meta
    int     beatIndex;         // 0부터 순환하는 beat 번호
    qint64  captureTimestamp;  // ALSA 캡처 시작 시각 (ms, 모노토닉)
};
```

### 5.2 GraphView 인터페이스 / GraphView Interface

```cpp
class GraphView : public QWidget {
    Q_OBJECT
public:
    explicit GraphView(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~GraphView() = default;

    // 매 beat마다 MeasurementEngine이 emit → 각 탭이 구현
    // Called each beat by MeasurementEngine; each tab implements this
    virtual void onMeasurementUpdate(const MeasurementResult& result) = 0;

    // 운영 모드 전환 시 호출 (Live / Playback / Sim)
    // Called on operating mode change
    virtual void onModeChanged(OperatingMode mode) {}  // 기본 no-op
};
```

**한국어**

`MainWindow`는 `GraphView*` 목록을 관리하고 `MeasurementEngine`의 Signal을 각 탭의 `onMeasurementUpdate` 슬롯에 연결한다. 새 탭 추가 시 `GraphView`를 상속한 클래스 파일 1개 + `MainWindow` 탭 등록 1줄 + `CMakeLists.txt` 1줄, 총 3파일 이내 변경으로 완료된다.

**English**

`MainWindow` manages a list of `GraphView*` and connects `MeasurementEngine`'s signal to each tab's `onMeasurementUpdate` slot. Adding a new tab requires: 1 new file subclassing `GraphView` + 1-line tab registration in `MainWindow` + 1-line `CMakeLists.txt` update — ≤ 3 files total.

---

## 6. 미결 설계 결정 / Open Design Decisions

**한국어**

아래 결정들은 실험 결과 후 확정된다. **구현 블로커**로 표시된 항목은 해당 결정이 확정되기 전까지 코딩팀이 관련 영역 구현을 시작하지 않는다.

**English**

The decisions below are confirmed after experiment results. Items marked as **implementation blockers** must not be implemented until the decision is locked.

| 결정 / Decision | 옵션 / Options | 연결 실험 / Linked Experiment | 상태 / Status |
|----------------|--------------|------------------------------|--------------|
| 목표 샘플레이트 / Target sample rate | 96k sps 유지 vs 48k sps 폴백 | EX-01 | Open — **구현 블로커** |
| QAS-2 지연 수치 확정 / Latency finalization | capture→process < 70 ms, process→display < 30 ms (잠정) | EX-01 | Open |
| WeiShi 비교 측정 방식 / Measurement method | 동시 측정 vs 순차 측정 | EX-02 | Open |
| T1·T3 timing 기준점 / `tg_c_placement_t` | TG_C_PLACEMENT_PEAK vs TG_C_PLACEMENT_ONSET | EX-03 | Open — **구현 블로커** |
| 스레딩 경계 세분화 / Threading boundary | 2-thread 확정, 렌더링 전략 미결 | EX-01 | Open |

---

## 7. 아키텍처 범위 외 항목 / Out of Scope

**한국어**

- 클라우드 연결 또는 원격 데이터 로깅 없음 (온디바이스 전용)
- 커스텀 DSP 하드웨어 없음 (USB 오디오 입력 그대로 사용)
- Witschi Chronoscope UI 정확한 복제 없음 (기능적 참고만)
- AI 기능(FR-09)은 선택적 — 아키텍처가 이에 의존하지 않음

**English**

- No cloud connectivity or remote data logging (on-device only)
- No custom DSP hardware (uses USB audio input as-is)
- No exact copy of Witschi Chronoscope UI (functional reference only)
- AI feature (FR-09) is optional — architecture must not depend on it

---

## 8. 검토 체크리스트 / Review Checklist

- [ ] 아키텍처 개요 (스레드 구분 포함) 제공됨 / Architecture overview with thread boundaries provided
- [ ] 주요 접근법 (전술·패턴·전략) 정의됨 / Key approaches (tactics, patterns, strategies) defined
- [ ] Pipe-and-Filter(런타임)와 Layered(정적 구조)의 역할 구분 명시됨 / Distinction between Pipe-and-Filter (runtime) and Layered (static) clarified
- [ ] God Object 분해 모듈 구조 명시됨 / God Object decomposition module structure specified
- [ ] 각 접근법이 특정 QA 드라이버와 연결됨 / Each approach linked to specific QA drivers
- [ ] QAS-3 정확도의 실험 의존성 명시됨 / QAS-3 accuracy dependency on experiments stated
- [ ] 레이턴시 예산 스테이지별 분해됨 / Latency budget broken down per stage
- [ ] 미결 설계 결정 및 구현 블로커 명시됨 / Open decisions and implementation blockers listed
- [ ] MeasurementResult 구조체 및 GraphView 인터페이스 정의됨 / MeasurementResult and GraphView interface defined
- [ ] 실험 참조가 EX-01/EX-02/EX-03과 정합됨 / Experiment references consistent with EX-01/EX-02/EX-03
- [ ] M2 구현을 가이드하기에 충분한 설계 / Design sufficient to guide M2 construction
