# Architectural Approaches — TimeGrapher

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09 | **상태 / Status**: [ ] 초안 Draft / [ ] 최종 Final

---

## 1. 아키텍처 개요 / Architecture Overview

### 한국어

TimeGrapher는 **4개의 수직 레이어**로 구성된 실시간 음향 분석 시스템이다.

```
┌──────────────────────────────────────────────────────────┐
│  Layer 4: Presentation                                   │
│  11개 그래프 탭 (QWidget 기반, Qt Signal-Slot 구독)        │
├──────────────────────────────────────────────────────────┤
│  Layer 3: Domain (Measurement Engine)                    │
│  Rate / Amplitude / Beat Error / BPH 계산                │
│  단일 데이터 소스 — 모든 뷰가 동일 계산 결과를 사용        │
├──────────────────────────────────────────────────────────┤
│  Layer 2: Signal Processing                              │
│  HPF → Envelope → Beat Event Detection (T1/T3)           │
├──────────────────────────────────────────────────────────┤
│  Layer 1: Signal Acquisition                             │
│  USB 오디오 캡처 (ALSA) / Playback / Sim                  │
│  AGC 비활성화 필수                                        │
└──────────────────────────────────────────────────────────┘
         ↕ 스레드 경계 / Thread boundary
┌──────────────────────────────────────────────────────────┐
│  Background Thread: Layer 1 + Layer 2 + Layer 3          │
│  Main (UI) Thread:  Layer 4                              │
└──────────────────────────────────────────────────────────┘
```

**핵심 설계 원칙 두 가지:**

1. **레이어 방향성**: 데이터는 항상 아래 → 위로 흐른다. 상위 레이어가 하위 레이어를 의존하지만, 역방향 의존은 없다.
2. **스레드 분리**: 신호 처리(Layer 1~3)는 백그라운드 스레드, GUI 렌더링(Layer 4)은 메인 스레드. Qt Signal-Slot이 스레드 경계를 연결한다.

### English

TimeGrapher is a real-time acoustic analysis system structured as **four vertical layers**.

```
┌──────────────────────────────────────────────────────────┐
│  Layer 4: Presentation                                   │
│  11 graph tabs (QWidget-based, Qt Signal-Slot subscriber)│
├──────────────────────────────────────────────────────────┤
│  Layer 3: Domain (Measurement Engine)                    │
│  Rate / Amplitude / Beat Error / BPH calculation         │
│  Single data source — all views consume identical output │
├──────────────────────────────────────────────────────────┤
│  Layer 2: Signal Processing                              │
│  HPF → Envelope → Beat Event Detection (T1/T3)           │
├──────────────────────────────────────────────────────────┤
│  Layer 1: Signal Acquisition                             │
│  USB audio capture (ALSA) / Playback / Sim               │
│  AGC must be disabled                                    │
└──────────────────────────────────────────────────────────┘
         ↕ Thread boundary
┌──────────────────────────────────────────────────────────┐
│  Background Thread: Layer 1 + Layer 2 + Layer 3          │
│  Main (UI) Thread:  Layer 4                              │
└──────────────────────────────────────────────────────────┘
```

**Two core design principles:**

1. **Layer directionality**: Data always flows bottom → top. Upper layers depend on lower layers; no reverse dependency.
2. **Thread separation**: Signal processing (Layers 1–3) runs on a background thread; GUI rendering (Layer 4) on the main thread. Qt Signal-Slot bridges the thread boundary.

---

## 2. 주요 아키텍처 접근법 / Key Architectural Approaches

### AP-1: Pipe-and-Filter — 신호 처리 파이프라인

**한국어**

| 항목 | 내용 |
|------|------|
| **패턴** | Pipe-and-Filter |
| **적용 범위** | Layer 1 → Layer 2 내부: Acquisition → HPF → Envelope → Detector |
| **설계 의도** | 각 단계가 단일 책임을 가지며, 단계 간 인터페이스(float32 PCM 버퍼)만 유지하면 개별 단계를 독립적으로 교체·튜닝 가능 |
| **구체적 효과** | HPF 컷오프 변경, Envelope 스무딩 파라미터 조정, 감지 알고리즘 교체가 다른 단계에 영향 없이 가능 |
| **트레이드오프** | 단계 간 버퍼링으로 미세 지연 발생 → 버퍼 크기를 최소화(4,096 샘플)하여 보상 |
| **연결 드라이버** | QA-1 Real-Time Performance, QA-3 Low Latency |
| **완화 리스크** | TR-01 (T1/T3 감지 알고리즘 교체 용이), TR-03 (버퍼 크기 튜닝 용이) |

```
USB PCM (float32)
    │
    ▼  tg_hpf_process()        ← 단계 교체 시 이 함수만 변경
    │  HPF 출력 (F1)
    ▼  tg_envelope_process()   ← 독립 교체 가능
    │  Envelope 출력 (F2)
    ▼  tg_detect()             ← 감지 알고리즘 독립 교체 가능
    │  A/C 이벤트 타임스탬프 (F3)
    ▼
 tg_result_t
```

**English**

| Field | Content |
|-------|---------|
| **Pattern** | Pipe-and-Filter |
| **Applied To** | Layer 1 → Layer 2: Acquisition → HPF → Envelope → Detector |
| **Intent** | Each stage has a single responsibility; stages share only a float32 PCM buffer interface, so individual stages can be replaced or tuned independently |
| **Concrete Effect** | HPF cutoff changes, envelope smoothing adjustments, and detection algorithm swaps can be made without affecting other stages |
| **Trade-off** | Inter-stage buffering introduces minor latency → compensated by keeping buffer sizes minimal (4,096 samples) |
| **Linked Driver** | QA-1 Real-Time Performance, QA-3 Low Latency |
| **Risk Mitigated** | TR-01 (easy algorithm swap), TR-03 (buffer size tuning) |

---

### AP-2: Observer / Qt Signal-Slot — 측정값 배포

**한국어**

| 항목 | 내용 |
|------|------|
| **패턴** | Observer (Qt Signal-Slot) |
| **적용 범위** | Layer 3 (Measurement Engine) → Layer 4 (11개 그래프 탭) |
| **설계 의도** | 계산 결과를 Signal로 emit하면, 구독하는 그래프 탭이 몇 개든 계산 코드를 수정하지 않고 데이터를 수신 |
| **구체적 효과** | 새 그래프 추가 시 `connect(engine, &MeasurementEngine::beatReady, newTab, &NewGraphTab::onBeatReady)` 한 줄로 데이터 연결 완료 |
| **트레이드오프** | Signal-Slot 연결이 많아질수록 디버깅 복잡도 증가 → 연결 목록을 명시적으로 문서화하여 관리 |
| **연결 드라이버** | QA-5 Extensibility, QA-4 Correctness |
| **완화 리스크** | TR-07 (God Object 분리 후 계산 코드 재사용), TR-09 (계산 로직 단일화) |

```
MeasurementEngine (Layer 3)
    │
    │ emit beatReady(BeatResult)   ← 계산 결과 단일 발행
    │
    ├──→ TraceTab::onBeatReady()   ← 기존 탭
    ├──→ VarioTab::onBeatReady()   ← 기존 탭
    ├──→ BeatErrorTab::onBeatReady() ← 기존 탭
    └──→ NewGraphTab::onBeatReady() ← 새 탭: connect 한 줄 추가만으로 완성
```

**English**

| Field | Content |
|-------|---------|
| **Pattern** | Observer (Qt Signal-Slot) |
| **Applied To** | Layer 3 (Measurement Engine) → Layer 4 (11 graph tabs) |
| **Intent** | Measurement results are emitted as Signals; any number of subscribing graph tabs receive data without modifying the calculation code |
| **Concrete Effect** | Adding a new graph requires only one line: `connect(engine, &MeasurementEngine::beatReady, newTab, &NewGraphTab::onBeatReady)` |
| **Trade-off** | More Signal-Slot connections increase debugging complexity → mitigated by explicitly documenting all connections |
| **Linked Driver** | QA-5 Extensibility, QA-4 Correctness |
| **Risk Mitigated** | TR-07 (calculation code reused after God Object split), TR-09 (calculation logic centralized) |

---

### AP-3: Layered Architecture — 확장 격리

**한국어**

| 항목 | 내용 |
|------|------|
| **전술** | 엄격한 레이어 분리 (Acquisition / Processing / Domain / Presentation) |
| **적용 범위** | 전체 모듈 구조 |
| **설계 의도** | 새 그래프(Presentation 레이어)를 추가할 때 하위 3개 레이어를 수정하지 않음. Extensibility QA의 "≤ 3 파일 변경" 목표를 구조적으로 보장 |
| **구체적 효과** | 새 그래프 추가 시 변경 파일: ① 새 위젯 파일 신규 생성 ② 탭 패널 등록 1곳 수정 ③ Signal-Slot connect 1곳 추가 → 합계 3개 |
| **트레이드오프** | 레이어 경계를 초기에 명확히 설계해야 하는 선행 비용 발생 |
| **연결 드라이버** | QA-5 Extensibility, QA-4 Correctness (단일 데이터 소스 보장) |
| **완화 리스크** | TR-07 (God Object를 레이어별로 분리), TR-09 (계산 로직을 Layer 3에 격리) |

```
새 그래프 추가 시 변경 범위 / Scope of change when adding a new graph:

  Layer 1 (Acquisition)   ← 수정 없음 / No change
  Layer 2 (Processing)    ← 수정 없음 / No change
  Layer 3 (Domain)        ← 수정 없음 / No change
  Layer 4 (Presentation)
    ├── NewGraphTab.h/.cpp  ← ① 신규 생성 / New file
    ├── TabPanel.cpp        ← ② 탭 등록 1줄 / Register tab
    └── MainWindow.cpp      ← ③ connect 1줄 / Wire signal
                               합계 3개 파일 / Total: 3 files
```

**English**

| Field | Content |
|-------|---------|
| **Tactic** | Strict layer separation (Acquisition / Processing / Domain / Presentation) |
| **Applied To** | Overall module structure |
| **Intent** | New graphs (Presentation layer) can be added without modifying the lower three layers. Structurally guarantees the Extensibility QA target of "≤ 3 files changed" |
| **Concrete Effect** | Files changed when adding a new graph: ① new widget file created ② tab panel registration modified ③ Signal-Slot connect added → total 3 files |
| **Trade-off** | Up-front cost of clearly defining layer boundaries before implementation begins |
| **Linked Driver** | QA-5 Extensibility, QA-4 Correctness (single data source guaranteed) |
| **Risk Mitigated** | TR-07 (God Object split by layer), TR-09 (calculation logic isolated in Layer 3) |

---

### AP-4: Concurrency — 백그라운드 처리 스레드

**한국어**

| 항목 | 내용 |
|------|------|
| **전술** | UI 스레드와 신호 처리 스레드 분리 |
| **적용 범위** | Layer 1 + Layer 2 + Layer 3 → 백그라운드 스레드, Layer 4 → Qt 메인 스레드 |
| **설계 의도** | 96k sps 오디오 처리가 GUI 렌더링을 블로킹하지 않도록 분리. dropped block = 0 목표 달성의 선행 조건 |
| **구체적 효과** | GUI가 느려지거나 탭 전환이 발생해도 오디오 처리 스레드는 영향을 받지 않음 |
| **트레이드오프** | 스레드 간 데이터 공유 시 동기화 필요 → Qt Signal-Slot (queued connection)으로 스레드 안전 보장 |
| **연결 드라이버** | QA-1 Real-Time Performance, QA-3 Low Latency |
| **완화 리스크** | TR-04 (RPi 96k sps 처리 불가 위험), TR-05 (Qt 렌더링 FPS 저하 위험) |

```
Background Thread                    Main (UI) Thread
─────────────────                    ────────────────
AudioWorker                          Qt Event Loop
  → tg_process()                         ↑
  → MeasurementEngine                    │ Qt::QueuedConnection
  → emit beatReady(result) ──────────────┤
                                         ↓
                                    GraphTab::onBeatReady()
                                    QCustomPlot update
```

**English**

| Field | Content |
|-------|---------|
| **Tactic** | Separate UI thread from signal processing thread |
| **Applied To** | Layers 1–3 → background thread; Layer 4 → Qt main thread |
| **Intent** | 96k sps audio processing must not block GUI rendering. Prerequisite for achieving dropped block = 0 |
| **Concrete Effect** | GUI slowdowns or tab switches do not affect the audio processing thread |
| **Trade-off** | Cross-thread data sharing requires synchronization → guaranteed thread-safe via Qt Signal-Slot (queued connection) |
| **Linked Driver** | QA-1 Real-Time Performance, QA-3 Low Latency |
| **Risk Mitigated** | TR-04 (RPi 96k sps under load), TR-05 (Qt rendering FPS drop) |

---

### AP-5: Single Data Source — Correctness 구조적 보장

**한국어**

| 항목 | 내용 |
|------|------|
| **전술** | 단일 데이터 소스 (Single Source of Truth) |
| **적용 범위** | MeasurementEngine (Layer 3) — Rate / Amplitude / Beat Error 계산이 한 곳에만 존재 |
| **설계 의도** | 모든 그래프 탭이 동일한 `BeatResult` 객체를 수신. 뷰마다 계산을 다시 하지 않으므로 수치 불일치 구조적 불가 |
| **구체적 효과** | Correctness QA (편차 = 0)를 코드 규율이 아니라 **구조**로 보장 |
| **트레이드오프** | MeasurementEngine이 단일 장애점(SPOF)이 될 수 있음 → 단위 테스트로 보완 |
| **연결 드라이버** | QA-4 Correctness, QA-2 Measurement Accuracy |
| **완화 리스크** | TR-09 (계산 오류가 MainWindow에 분산되는 문제 해소) |

**English**

| Field | Content |
|-------|---------|
| **Tactic** | Single Source of Truth |
| **Applied To** | MeasurementEngine (Layer 3) — Rate / Amplitude / Beat Error calculated in one place only |
| **Intent** | All graph tabs receive the same `BeatResult` object. No per-view recalculation means numerical disagreement is structurally impossible |
| **Concrete Effect** | Correctness QA (deviation = 0) is guaranteed by **structure**, not by code discipline |
| **Trade-off** | MeasurementEngine becomes a single point of failure → compensated by unit tests |
| **Linked Driver** | QA-4 Correctness, QA-2 Measurement Accuracy |
| **Risk Mitigated** | TR-09 (calculation logic scattered across MainWindow) |

---

### AP-6: Graceful Degradation — sps 폴백

**한국어**

| 항목 | 내용 |
|------|------|
| **전술** | 단계적 성능 강등 (Graceful Degradation) |
| **적용 범위** | Signal Acquisition 레이어 — 샘플레이트 설정 |
| **설계 의도** | RPi 5가 96k sps를 지속하지 못할 경우 48k sps로 자동 폴백. 시스템이 죽는 대신 측정 정밀도만 낮아짐 |
| **구체적 효과** | 192k sps (stretch) → 96k sps (objective) → 48k sps (minimum) 순서로 단계적 강등. 48k 이하는 프로젝트 실패 기준 |
| **트레이드오프** | sps 낮아질수록 T1/T3 타이밍 정밀도 저하 → Measurement Accuracy에 영향 |
| **연결 드라이버** | QA-1 Real-Time Performance, QA-2 Measurement Accuracy |
| **완화 리스크** | TR-04 (RPi 96k sps 달성 불가 위험) |

**English**

| Field | Content |
|-------|---------|
| **Tactic** | Graceful Degradation |
| **Applied To** | Signal Acquisition layer — sample rate configuration |
| **Intent** | If RPi 5 cannot sustain 96k sps, fall back automatically to 48k sps. System degrades in precision rather than failing |
| **Concrete Effect** | Staged fallback: 192k sps (stretch) → 96k sps (objective) → 48k sps (minimum). Below 48k = project failure |
| **Trade-off** | Lower sps reduces T1/T3 timing precision → impacts Measurement Accuracy |
| **Linked Driver** | QA-1 Real-Time Performance, QA-2 Measurement Accuracy |
| **Risk Mitigated** | TR-04 (RPi unable to sustain 96k sps) |

---

### AP-7: Per-Beat PCM Ring Buffer — 파형 그래프 공통 기반

**한국어**

| 항목 | 내용 |
|------|------|
| **전술** | 공유 링 버퍼 (Shared Ring Buffer) |
| **적용 범위** | Layer 2 출력 → ④ Beat-Noise Scope / ⑦ Escapement Analyzer / ⑨ Waveform Comparison / ⑩ Scope Mode |
| **설계 의도** | A 이벤트 타임스탬프를 기준으로 beat 단위로 PCM을 슬라이싱하는 링 버퍼를 단 하나만 구현. 파형을 필요로 하는 4개 그래프가 이 버퍼를 공유 |
| **구체적 효과** | 파형 그래프마다 별도 버퍼를 구현하지 않아도 됨 → 중복 구현 제거, 메모리 효율 확보 |
| **트레이드오프** | 링 버퍼가 단일 공유 자원 → 접근 동기화 필요 (read-only 구독 방식으로 경합 최소화) |
| **연결 드라이버** | QA-5 Extensibility, QA-1 Real-Time Performance |
| **완화 리스크** | TR-11 (per-beat PCM 링 버퍼 미구현), TR-12 (메모리 누적 방지 — cap 설정) |

**English**

| Field | Content |
|-------|---------|
| **Tactic** | Shared Ring Buffer |
| **Applied To** | Layer 2 output → ④ Beat-Noise Scope / ⑦ Escapement Analyzer / ⑨ Waveform Comparison / ⑩ Scope Mode |
| **Intent** | A single ring buffer slices PCM into beat windows anchored on A event timestamps. Four waveform-based graphs share this buffer |
| **Concrete Effect** | No need to implement a separate buffer per waveform graph → eliminates duplication, improves memory efficiency |
| **Trade-off** | Shared resource requires access synchronization → minimized by read-only subscription model |
| **Linked Driver** | QA-5 Extensibility, QA-1 Real-Time Performance |
| **Risk Mitigated** | TR-11 (per-beat PCM ring buffer missing), TR-12 (memory accumulation — cap enforced) |

---

## 3. 아키텍처 ↔ 드라이버 매핑 / Architecture ↔ Driver Mapping

### 한국어

| QA | 접근법 | 지원 방식 |
|----|--------|---------|
| **QA-1 Real-Time Performance** | AP-4 (스레드 분리), AP-6 (sps 폴백), AP-1 (최소 버퍼) | 오디오 처리가 GUI에 블로킹되지 않음. 96k 불가 시 48k 폴백으로 dropped block 0 유지 |
| **QA-2 Measurement Accuracy** | AP-1 (파이프라인 단계 교체 용이), AP-5 (단일 계산 소스), AP-6 (sps 최대화) | T1/T3 감지 알고리즘을 독립 교체 가능. 계산 오류 격리. 높은 sps로 타이밍 정밀도 확보 |
| **QA-3 Low Latency** | AP-4 (스레드 분리), AP-1 (4096 샘플 버퍼) | 42.7ms 예산 내에서 처리. GUI 렌더링 지연이 오디오 처리에 전파되지 않음 |
| **QA-4 Correctness** | AP-5 (단일 데이터 소스), AP-2 (모든 탭이 동일 Signal 수신) | 구조적으로 뷰 간 수치 불일치 불가 |
| **QA-5 Extensibility** | AP-3 (레이어 분리), AP-2 (Observer), AP-7 (공유 링 버퍼) | 새 그래프 추가 시 ≤ 3 파일 변경. 하위 레이어 수정 없음 |

### English

| QA | Approach | How it supports |
|----|----------|----------------|
| **QA-1 Real-Time Performance** | AP-4 (thread separation), AP-6 (sps fallback), AP-1 (minimal buffer) | Audio processing never blocked by GUI. 48k fallback maintains dropped block = 0 if 96k unachievable |
| **QA-2 Measurement Accuracy** | AP-1 (pipeline stage swap), AP-5 (single calculation source), AP-6 (maximize sps) | T1/T3 detection algorithm swappable independently. Calculation errors isolated. High sps for timing precision |
| **QA-3 Low Latency** | AP-4 (thread separation), AP-1 (4096-sample buffer) | Processing fits within 42.7ms budget. GUI rendering delays cannot propagate to audio processing |
| **QA-4 Correctness** | AP-5 (single data source), AP-2 (all tabs receive same Signal) | Numerical disagreement between views structurally impossible |
| **QA-5 Extensibility** | AP-3 (layer separation), AP-2 (Observer), AP-7 (shared ring buffer) | ≤ 3 files changed to add a new graph. No lower-layer modification needed |

---

## 4. 리스크 ↔ 접근법 매핑 / Risk ↔ Approach Mapping

### 한국어

| 리스크 / Risk | 완화 접근법 / Mitigating Approach |
|--------------|----------------------------------|
| TR-01 T1/T3 감지 부정확 | AP-1 (파이프라인 — 감지 알고리즘 독립 교체) |
| TR-03 USB 오디오 버퍼 언더런 | AP-1 (버퍼 크기 최소화 + 튜닝 가능), AP-4 (스레드 분리) |
| TR-04 RPi 96k sps 처리 불가 | AP-4 (스레드 분리), AP-6 (sps 폴백) |
| TR-05 Qt 렌더링 FPS 저하 | AP-4 (UI/처리 스레드 분리로 렌더링이 처리에 영향 없음) |
| TR-06 FFT 스펙트로그램 CPU 과부하 | AP-4 (FFT를 별도 스레드 또는 낮은 주기로 실행), EX-01로 검증 |
| TR-07 MainWindow God Object | AP-3 (레이어 분리로 해소), AP-5 (계산 로직 Layer 3 이전) |
| TR-09 계산 오류 (단위 테스트 불가) | AP-5 (MeasurementEngine 분리 → 단위 테스트 가능) |
| TR-11 per-beat PCM 링 버퍼 미구현 | AP-7 (공유 링 버퍼로 설계) |
| TR-12 메모리 누적 OOM | AP-7 (링 버퍼 cap 설정으로 상한 제어) |

### English

| Risk | Mitigating Approach |
|------|---------------------|
| TR-01 T1/T3 detection inaccuracy | AP-1 (pipeline — detection algorithm swappable independently) |
| TR-03 USB audio buffer underrun | AP-1 (minimal tunable buffer), AP-4 (thread separation) |
| TR-04 RPi unable to sustain 96k sps | AP-4 (thread separation), AP-6 (sps fallback) |
| TR-05 Qt rendering FPS drop | AP-4 (UI/processing thread separation — rendering cannot impact processing) |
| TR-06 FFT spectrogram CPU overload | AP-4 (FFT on separate thread or lower update rate), validated by EX-01 |
| TR-07 MainWindow God Object | AP-3 (resolved by layer separation), AP-5 (calculation logic moved to Layer 3) |
| TR-09 Calculation errors (untestable) | AP-5 (MeasurementEngine extracted → unit-testable) |
| TR-11 Per-beat PCM ring buffer missing | AP-7 (designed as shared ring buffer) |
| TR-12 Memory accumulation OOM | AP-7 (ring buffer cap enforces memory ceiling) |

---

## 5. 미결 설계 결정 / Open Design Decisions

**한국어**  
아래 결정들은 실험 결과가 나온 후 M2에서 확정된다.

**English**  
The decisions below are pending experiment results and will be confirmed at M2.

| 결정 / Decision | 옵션 / Options | 근거 실험 / Linked Experiment | 상태 / Status |
|----------------|---------------|------------------------------|--------------|
| T1/T3 감지 기준점 / Detection reference | `TG_C_PLACEMENT_ONSET` vs `TG_C_PLACEMENT_PEAK` | EX-03 | Open |
| 목표 sps / Target sps | 96k 유지 / 48k 폴백 | EX-01 | Open |
| 스레드 모델 / Threading model | 단일 스레드 / 오디오-UI 분리 스레드 | EX-01, EX-05 | Open |
| FFT 라이브러리 / FFT library | FFTW3 재활성화 / kissfft / 자체 DFT | EX-01 (FFT 조건) | Open |
| FFT 윈도우 크기 / FFT window size | 1024 / 2048 / 4096 샘플 | EX-01 (FFT 조건) | Open |
| Low Latency 수치 확정 / QA-3 targets | 3구간 잠정값 대체 | EX-01 | Open |

---

## 6. 설계의 구성 충분성 / Design Soundness Assessment

**한국어**

현재 아키텍처 접근법이 구성(construction)을 안내하기에 충분한지를 QA별로 평가한다.

| QA | 판단 | 근거 |
|----|------|------|
| QA-1 Real-Time Performance | ✅ 충분 | 스레드 분리(AP-4) + sps 폴백(AP-6) 조합이 설계 수준에서 dropped block 방어. EX-01로 수치 검증 예정 |
| QA-2 Measurement Accuracy | ⚠️ 부분 충분 | 파이프라인(AP-1)과 단일 계산 소스(AP-5)는 명확. 단, T1/T3 감지 알고리즘의 최적 파라미터는 EX-03 결과 후 확정 |
| QA-3 Low Latency | ⚠️ 부분 충분 | 스레드 분리(AP-4)와 최소 버퍼(AP-1) 전략은 수립됨. 3구간 수치는 EX-01 결과 전까지 잠정값 |
| QA-4 Correctness | ✅ 충분 | 단일 데이터 소스(AP-5) + Observer(AP-2)로 구조적 보장. 추가 설계 결정 불필요 |
| QA-5 Extensibility | ✅ 충분 | 레이어 분리(AP-3) + Observer(AP-2) + 공유 링 버퍼(AP-7)로 ≤ 3 파일 조건 달성 경로 명확 |

**English**

Assessment of whether the current architectural approaches are sound enough to guide construction.

| QA | Assessment | Rationale |
|----|------------|-----------|
| QA-1 Real-Time Performance | ✅ Sufficient | Thread separation (AP-4) + sps fallback (AP-6) defensively covers dropped blocks at design level. EX-01 validates numerically |
| QA-2 Measurement Accuracy | ⚠️ Partially sufficient | Pipeline (AP-1) and single calculation source (AP-5) are clear. Optimal T1/T3 detection parameters require EX-03 results |
| QA-3 Low Latency | ⚠️ Partially sufficient | Thread separation (AP-4) and minimal buffering (AP-1) strategy established. Three-segment targets remain provisional until EX-01 |
| QA-4 Correctness | ✅ Sufficient | Single data source (AP-5) + Observer (AP-2) provide structural guarantee. No further design decisions needed |
| QA-5 Extensibility | ✅ Sufficient | Layer separation (AP-3) + Observer (AP-2) + shared ring buffer (AP-7) provide a clear path to ≤ 3 files |

---

## 7. 설계 범위 외 항목 / Out of Scope

- 클라우드 연결 또는 원격 데이터 로깅 없음 (온디바이스 전용) / No cloud connectivity or remote logging (on-device only)
- 커스텀 DSP 하드웨어 없음 (USB 오디오 입력 그대로 사용) / No custom DSP hardware (USB audio input as-is)
- Witschi Chronoscope UI 복제 아님 (기능적 참고만) / Not a copy of Witschi Chronoscope UI (functional reference only)
- AI 기능(FR-19)은 옵션 — 아키텍처가 이에 의존하지 않음 / AI feature (FR-19) is optional — architecture must not depend on it

---

## 8. 리뷰 체크리스트 / Review Checklist

- [ ] 아키텍처 개요 설명 제공됨 / Architecture overview-level description provided
- [ ] 주요 아키텍처 접근법(전술, 패턴, 설계 전략) 정의됨 / Key architectural approaches (tactics, patterns, design strategies) defined
- [ ] 각 접근법이 특정 아키텍처 드라이버와 연결됨 / Each approach linked to specific architectural drivers
- [ ] 각 접근법이 특정 기술 리스크와 연결됨 / Each approach linked to specific technical risks
- [ ] 미결 설계 결정이 실험과 연결됨 / Open design decisions linked to experiments
- [ ] 설계가 M2 구성을 안내하기에 충분함 / Design is sufficient to guide construction at M2
