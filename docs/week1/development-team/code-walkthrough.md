# TimeGrapher 코드 구조 및 실행 흐름 분석 / TimeGrapher — Code Structure & Execution Flow Analysis

> **작성일 / Date**: 2026-06-02

---

## 0. 기능별 소스 파일 맵 (빠른 참조) / Feature-to-File Map (Quick Reference)

**한국어**

코드를 처음 보는 팀원을 위한 빠른 참조표입니다.

**English**

Quick reference table for team members new to the codebase.

---

### 🎯 핵심 기능 → 파일 매핑 / Core Features → File Mapping

| 기능 / Feature | 파일 / File | 핵심 함수/클래스 / Key Functions/Classes | 설명 / Description |
|----------------|-------------|----------------------------------------|-------------------|
| **앱 진입점 / App Entry Point** | `Main.cpp` | `main()` | Qt 앱 초기화, MainWindow 생성 / Qt app initialization, MainWindow creation |
| **UI 및 전체 제어 / UI & Overall Control** | `MainWindow.cpp/h` | `MainWindow` | 모든 기능의 허브 (God Object) / Hub for all features (God Object) |
| **마이크 입력 / Microphone Input** | `AudioWorker.cpp/h` | `TAudioWorker` | 실시간 마이크 → 링버퍼 / Real-time mic → ring buffer |
| **WAV 재생 / WAV Playback** | `PlaybackWorker.cpp/h` | `TPlaybackWorker` | WAV 파일 → 링버퍼 / WAV file → ring buffer |
| **시뮬레이션 / Simulation** | `SimWorker.cpp/h` | `TSimWorker` | 합성 신호 → 링버퍼 / Synthesized signal → ring buffer |
| **시계 소리 합성 / Watch Sound Synthesis** | `WatchSynthStream.cpp/h` | `watch_synth_stream_*` | 테스트용 Tic-Toc 신호 생성 / Tic-Toc signal generation for testing |
| **신호 처리 (DSP) / Signal Processing (DSP)** | `Dsp.cpp/h` | `tg_hpf_*`, `tg_envelope_*` | HPF + Envelope 추출 / HPF + Envelope extraction |
| **A/C 이벤트 검출 / A/C Event Detection** | `Detector.cpp/h` | `tg_detector_*` | Silence→Burst 탐지 / Silence→Burst detection |
| **BPH 감지 & 동기화 / BPH Detection & Sync** | `Bph.cpp/h` | `tg_phase_score`, `tg_sync_*` | Rayleigh 위상 점수 + PLL / Rayleigh phase score + PLL |
| **DSP 통합 API / DSP Unified API** | `Timegrapher.cpp/h` | `tg_process()` | 위 3개를 묶은 단일 진입점 / Single entry point for above 3 |
| **Sound Image 시각화 / Sound Image Visualization** | `SoundImageRenderer.cpp/h` | `SoundImageRenderer` | 2D 폴딩 이미지 렌더링 / 2D folded image rendering |
| **WAV 저장 / WAV Save** | `WavStreamWriter.cpp/h` | `WavStreamWriter` | 스트리밍 WAV 파일 저장 / Streaming WAV file save |
| **선형회귀 (Rate) / Linear Regression (Rate)** | `RollingLeastSquares.cpp/h` | `RollingLeastSquares` | 레이트 오차 계산용 / For rate error calculation |
| **이동 평균 / Moving Average** | `RollingAverage.cpp/h` | `RollingAverage` | 비트 오차/진폭 평활화 / Beat error/amplitude smoothing |
| **그래프 라이브러리 / Graph Library** | `qcustomplot.cpp/h` | `QCustomPlot` | Qt 기반 플로팅 라이브러리 / Qt-based plotting library |

---

### 📁 파일별 역할 상세 / Detailed File Roles

```
src/
├── Main.cpp                 # 앱 진입점 / App entry point
├── MainWindow.cpp/h/ui      # UI + 전체 로직 (1,600줄+, 리팩토링 대상)
│                            # UI + all logic (1,600+ lines, refactoring target)
│
├── [오디오 입력 레이어 / Audio Input Layer]
│   ├── AudioWorker.cpp/h    # Live: QAudioSource → 링버퍼 / ring buffer
│   ├── PlaybackWorker.cpp/h # Playback: WAV 파일 → 링버퍼 / WAV file → ring buffer
│   ├── SimWorker.cpp/h      # Sim: 합성기 → 링버퍼 / synthesizer → ring buffer
│   ├── WatchSynthStream.cpp/h # 시계 소리 합성 엔진 / Watch sound synthesis engine
│   └── SharedAudio.h        # 링버퍼 구조체 / Ring buffer struct (TMasterAudioDataRaw)
│
├── [신호 처리 레이어 / Signal Processing Layer]
│   ├── Timegrapher.cpp/h    # ★ DSP 파이프라인 통합 API / DSP pipeline unified API
│   ├── Dsp.cpp/h            # HPF (DC 제거) + Envelope (포락선) / HPF (DC removal) + Envelope
│   ├── Detector.cpp/h       # Silence/Burst 기반 A/C 이벤트 검출 / A/C event detection
│   └── Bph.cpp/h            # BPH 자동 감지 + PLL 동기 추적 / BPH auto-detection + PLL sync
│
├── [시각화 레이어 / Visualization Layer]
│   ├── SoundImageRenderer.cpp/h  # 2D Sound Image (타임그래퍼 특유) / timegrapher-specific
│   ├── SoundImageWidget.cpp/h    # Sound Image Qt 위젯 / Qt widget
│   └── qcustomplot.cpp/h         # 그래프 라이브러리 (외부) / Graph library (external)
│
├── [유틸리티 / Utilities]
│   ├── RollingLeastSquares.cpp/h # 선형회귀 (레이트 계산) / Linear regression (rate)
│   ├── RollingAverage.cpp/h      # 이동 평균 / Moving average
│   ├── WavStreamWriter.cpp/h     # WAV 파일 저장 / WAV file save
│   └── WaveHeader.h              # WAV 헤더 구조체 / WAV header struct
│
└── [플랫폼 오디오 / Platform Audio]
    ├── LinuxAudio.cpp/h     # Linux ALSA 볼륨 제어 / Linux ALSA volume control
    └── WindowsAudio.cpp/h   # Windows 볼륨 제어 / Windows volume control
```

---

### 🔗 데이터 흐름 한눈에 보기 / Data Flow at a Glance

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                   데이터 흐름 / Data Flow                                    │
└─────────────────────────────────────────────────────────────────────────────┘

[입력 소스 / Input Sources]      [링버퍼 / Ring Buffer]    [처리 / Processing]    [출력 / Output]
                                       │
 AudioWorker ──┐                       │
 (마이크/Mic)   │                       ▼
               ├──► TMasterAudioDataRaw ──► ProcessSamples()
 PlaybackWorker│     (30초 float 버퍼)            │
 (WAV 파일)    │     (30-sec float buffer)       │
               │                                  ├──► tg_process()
 SimWorker ────┘                                  │    (HPF→ENV→DET→BPH)
 (합성/Synth)                                     │         │
                                                  │         ├──► A/C Events
                                                  │         │       │
                                                  │         │       ├──► ComputeRateError()  → Rate (s/day)
                                                  │         │       ├──► ComputeBeatError()  → Beat Error (ms)
                                                  │         │       └──► ComputeAmplitude()  → Amplitude (°)
                                                  │         │
                                                  │         └──► processed_pcm → ScopePlot
                                                  │
                                                  ├──► SoundImageRenderer → Sound Image
                                                  │
                                                  └──► WavStreamWriter → WAV 파일/file
```

---

## 1. Static View 정확도 검증 / Static View Accuracy Check

**한국어**

`docs/static.pu`의 다이어그램과 실제 코드를 비교한 결과입니다.

**English**

Comparison between `docs/static.pu` and the actual codebase.

| 항목 / Item | 다이어그램 / Diagram | 실제 코드 / Actual Code | 판정 / Verdict |
|-------------|---------------------|------------------------|----------------|
| AW/PW/SW → 링버퍼 / Ring Buffer | ✅ | memcpy로 TMasterAudioDataRaw에 기록 / memcpy into TMasterAudioDataRaw | 정확 / Accurate |
| SA → MW → DSP 파이프라인 / Pipeline | ✅ | ProcessSamples() 내부에서 순차 처리 / Sequential execution inside ProcessSamples() | 정확 / Accurate |
| HPF → ENV → DET → BPH → TG 분리 | 개념적으로 정확 / Conceptually correct | 실제로는 `tg_process()` 단일 함수 호출로 캡슐화됨 / Encapsulated in single `tg_process()` call | 개념 맞음, 코드 구조는 다름 / Concept OK, code structure differs |
| TG → MW (tg_event_t) | ✅ | `r.events[i]` 루프로 수신 / Consumed via `r.events[i]` loop | 정확 / Accurate |
| MW → RLS / RA → STRUCTS → CP | ✅ | ComputeRateError(), ComputeBeatError() | 정확 / Accurate |
| MW → SIR → SIW | ✅ | mSoundRenderer.processSamples() | 정확 / Accurate |
| MW → WavStreamWriter | ✅ | mWavWriter->write() | 정확 / Accurate |
| **ScopePlot (오실로스코프 / oscilloscope)** | ❌ 없음 / Missing | `ui->ScopePlot` — envelope 실시간 표시 / real-time envelope display | **누락 / Not in diagram** |

### 누락된 컴포넌트: ScopePlot / Missing Component: ScopePlot

**한국어**

다이어그램에 없지만 실제로 존재한다. `tg_process()` 결과의 `processed_pcm`(처리된 오디오 파형)과 `onset_threshold`(검출 임계값)를 실시간으로 오실로스코프처럼 표시하는 QCustomPlot 그래프다. A/C 이벤트 발생 시 수직 마커와 시간 레이블도 여기에 추가된다.

**English**

Exists in code but absent from the diagram. It is a QCustomPlot graph that displays `processed_pcm` (processed audio waveform) and `onset_threshold` (detection threshold) from `tg_process()` in real time, like an oscilloscope. Vertical markers and time labels are also added here whenever A/C events are detected.

---

## 1.5. 핵심 기능별 상세 설명 / Core Features Detailed Explanation

**한국어**

각 기능이 어떤 문제를 해결하고, 코드에서 어떻게 동작하는지 설명합니다.

**English**

Explains what problem each feature solves and how it works in code.

---

### 🎤 기능 1: 오디오 입력 / Feature 1: Audio Acquisition

**한국어**

**문제**: 시계 소리를 실시간으로 캡처해야 한다.

**해결**: 3가지 입력 모드를 지원하는 Worker 패턴

**English**

**Problem**: Need to capture watch sounds in real-time.

**Solution**: Worker pattern supporting 3 input modes

| 모드 / Mode | 클래스 / Class | 입력 소스 / Input Source | 사용 시나리오 / Use Case |
|-------------|---------------|-------------------------|------------------------|
| **Live** | `TAudioWorker` | 마이크 (QAudioSource) / Microphone | 실제 시계 측정 / Real watch measurement |
| **Playback** | `TPlaybackWorker` | WAV 파일 / WAV file | 녹음된 소리 재분석 / Re-analyze recorded sound |
| **Sim** | `TSimWorker` | `WatchSynthStream` | 알려진 파라미터로 테스트 / Test with known parameters |

**핵심 코드 위치 / Key Code Location**:
```cpp
// AudioWorker.cpp - 마이크 데이터 수신 / Microphone data reception
void TAudioWorker::ProcessAudioInput() {
    QByteArray data = mAudioInputDevice->readAll();  // 마이크에서 읽기 / Read from mic
    memcpy(&mRawAudio->RawData[WriteIndex], ...);    // 링버퍼에 쓰기 / Write to ring buffer
    emit AudioDataReady();                            // 메인 스레드에 알림 / Notify main thread
}
```

**링버퍼 구조 / Ring Buffer Structure** (`SharedAudio.h`):
```cpp
typedef struct {
    float    *RawData;           // float PCM 데이터 (30초분) / float PCM data (30 seconds worth)
    uint64_t  WriteIndex;        // 쓰기 위치 / Write position
    uint64_t  TotalSamplesWritten;
    QMutex   *Mutex;             // 스레드 동기화 / Thread synchronization
    int       SampleRate;        // 48000 Hz
    int       BufferSize;        // SampleRate × 30
} TMasterAudioDataRaw;
```

---

### 🔊 기능 2: 신호 처리 파이프라인 (DSP) / Feature 2: Signal Processing Pipeline (DSP)

**한국어**

**문제**: 원시 오디오에서 Tic/Toc 이벤트를 정확히 검출해야 한다.

**해결**: 4단계 파이프라인을 `tg_process()` 하나로 캡슐화

**English**

**Problem**: Need to accurately detect Tic/Toc events from raw audio.

**Solution**: 4-stage pipeline encapsulated in single `tg_process()` call

```
┌─────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│   PCM   │ → │   HPF    │ → │ Envelope │ → │ Detector │ → A/C Events
│ (Raw)   │    │(DC제거)  │    │ (포락선) │    │ (검출)   │
│         │    │(DC Block)│    │          │    │          │
└─────────┘    └──────────┘    └──────────┘    └──────────┘
                                                    ↓
                                              ┌──────────┐
                                              │   BPH    │ → Sync Status
                                              │ (동기화) │
                                              │  (Sync)  │
                                              └──────────┘
```

| 단계 / Stage | 파일 / File | 함수 / Function | 역할 / Role |
|--------------|-------------|-----------------|-------------|
| **1. HPF** | `Dsp.cpp` | `tg_hpf_process()` | 200Hz 이하 DC/험 제거 / Remove DC/hum below 200Hz |
| **2. Envelope** | `Dsp.cpp` | `tg_envelope_process()` | 정류 + 0.15ms LPF → 포락선 / Rectify + 0.15ms LPF → envelope |
| **3. Detector** | `Detector.cpp` | `tg_detector_process()` | Silence/Burst 분석 → A/C 이벤트 / Silence/Burst analysis → A/C events |
| **4. BPH Sync** | `Bph.cpp` | `tg_sync_update()` | Rayleigh 점수 + PLL 추적 / Rayleigh score + PLL tracking |

**핵심 코드 위치 / Key Code Location**:
```cpp
// Timegrapher.cpp - 단일 진입점 / Single entry point
tg_result_t r;
tg_process(mCtx, inputSamples, sampleCount, &r);

// 결과물 / Outputs:
// r.events[]        - A/C 이벤트 배열 / A/C event array
// r.processed_pcm[] - 처리된 파형 (오실로스코프용) / Processed waveform (for oscilloscope)
// r.detected_bph    - 감지된 BPH (예: 21600) / Detected BPH (e.g., 21600)
// r.sync_status     - NOT_SYNCED / SYNCED / MISMATCH
```

**A/C 이벤트란? / What are A/C Events?**

**한국어**
```
A 이벤트 (Unlock/Tic 시작):
  - 탈진기가 열리는 순간
  - 포락선이 노이즈 플로어를 처음 넘는 지점
  - 선형 보간으로 서브샘플 정밀도 확보

C 이벤트 (Drop/Toc 피크):
  - 탈진 휠이 잠금면에 떨어지는 순간
  - 포락선의 최대값 지점
  - 포물선 보간으로 서브샘플 정밀도 확보
```

**English**
```
A Event (Unlock/Tic start):
  - Moment when escapement opens
  - Point where envelope first crosses noise floor
  - Linear interpolation for sub-sample precision

C Event (Drop/Toc peak):
  - Moment when escape wheel drops onto locking face
  - Maximum point of envelope
  - Parabolic interpolation for sub-sample precision
```

---

### 📊 기능 3: 계측값 계산 / Feature 3: Measurement Calculations

**한국어**

**문제**: A/C 이벤트로부터 시계 성능 지표를 계산해야 한다.

**해결**: 3가지 핵심 계측값 계산

**English**

**Problem**: Need to calculate watch performance metrics from A/C events.

**Solution**: 3 core measurement calculations

| 계측값 / Measurement | 단위 / Unit | 함수 / Function | 의미 / Meaning |
|---------------------|-------------|-----------------|----------------|
| **Rate Error / 레이트 오차** | s/day | `ComputeRateError()` | 하루 빠르기/느리기 / Gains/loses per day |
| **Beat Error / 비트 오차** | ms | `ComputeBeatError()` | Tic:Toc 균형 / Tic:Toc balance |
| **Amplitude / 진폭** | ° | `ComputeAmplitude()` | 밸런스 휠 진폭 / Balance wheel amplitude |

#### Rate Error / 레이트 오차 (s/day)

```cpp
// MainWindow.cpp - ComputeRateError()
// 원리: A이벤트 시간 vs 이상적 시간의 선형회귀 기울기
// Principle: Linear regression slope of A event time vs ideal time

RollingLeastSquares.AddPoint(실제시간/actual_time, 이상시간/ideal_time);
slope = 회귀기울기/regression_slope;
rate_error = (slope - 1.0) × 86400;  // s/day로 변환 / Convert to s/day

// 예 / Example: slope=1.000116 → +10 s/day (하루에 10초 빠름 / gains 10 seconds per day)
```

#### Beat Error / 비트 오차 (ms)

```cpp
// MainWindow.cpp - ComputeBeatError()
// 원리: 연속 3개 A이벤트 간격 비교
// Principle: Compare intervals of 3 consecutive A events

t[0] = A이벤트1 시간 / A event 1 time
t[1] = A이벤트2 시간 / A event 2 time
t[2] = A이벤트3 시간 / A event 3 time

beat_error = ((t[2]-t[1]) - (t[1]-t[0])) / 2 × 1000;  // ms

// 예 / Example: 0.5ms → Tic이 Toc보다 0.5ms 늦음 / Tic is 0.5ms later than Toc
```

#### Amplitude / 진폭 (°)

```cpp
// MainWindow.cpp - ComputeAmplitude()
// 원리: A→C 시간과 BPH로 역산
// Principle: Reverse calculate from A→C time and BPH

T1 = C이벤트시간 - A이벤트시간;  // 초 / seconds
amplitude = arcsin(π × T1 × BPH/3600) × (180/π) × 2 / LiftAngle;

// 예 / Example: 270° → 양호한 밸런스 휠 진폭 / Good balance wheel amplitude
```

---

### 🖼️ 기능 4: Sound Image 시각화 / Feature 4: Sound Image Visualization

**한국어**

**문제**: 타임그래퍼 특유의 2D 폴딩 이미지를 생성해야 한다.

**해결**: `SoundImageRenderer` 클래스

**English**

**Problem**: Need to generate timegrapher-specific 2D folded image.

**Solution**: `SoundImageRenderer` class

```
Sound Image 구조 / Structure:
┌─────────────────────────────────────────┐
│ ← X축: 비트(Beat) 번호 (시간 경과) →     │
│ ← X-axis: Beat number (time elapsed) → │
│                                         │
│ ↑                                       │
│ Y축: 비트 내 시간 위치                   │
│ Y-axis: Time position within beat       │
│ ↓                                       │
│                                         │
│  녹색 점 = A이벤트 (Tic)                 │
│  Green dot = A event (Tic)              │
│  파란 점 = C이벤트 (Toc)                 │
│  Blue dot = C event (Toc)               │
│  밝기 = 신호 크기                        │
│  Brightness = Signal magnitude          │
└─────────────────────────────────────────┘
```

---

### ⏱️ 기능 5: BPH 자동 감지 및 동기화 / Feature 5: BPH Auto-Detection and Synchronization

**한국어**

**문제**: 시계의 BPH(Beats Per Hour)를 자동으로 알아내야 한다.

**해결**: Rayleigh 위상 점수 + PLL 추적

**English**

**Problem**: Need to automatically determine watch's BPH (Beats Per Hour).

**Solution**: Rayleigh phase score + PLL tracking

```
지원 BPH (자동 감지) / Supported BPH (auto-detection):
  12000, 14400, 18000, 19800, 21600, 25200, 28800, 36000, 43200

감지 알고리즘 / Detection Algorithm:
  1. 약 1.5초간 이벤트 수집 / Collect events for ~1.5 seconds
  2. 각 후보 BPH에 대해 Rayleigh 위상 점수 계산
     Calculate Rayleigh phase score for each candidate BPH
     score = |Σ e^(i × 2π × event_time / T)| / N
  3. 점수가 임계값(0.7) 이상인 최고 점수 BPH 선택
     Select highest-scoring BPH above threshold (0.7)
  4. PLL로 미세 추적 (drift 보정) / Fine-track with PLL (drift correction)
```

---

### 💾 기능 6: WAV 파일 저장 / Feature 6: WAV File Save

**한국어**

**문제**: 측정 중인 오디오를 파일로 저장해야 한다.

**해결**: 스트리밍 WAV 저장 (`WavStreamWriter`)

**English**

**Problem**: Need to save audio being measured to file.

**Solution**: Streaming WAV save (`WavStreamWriter`)

```cpp
// WavStreamWriter.cpp
// 특징: 헤더를 먼저 쓰고, 닫을 때 크기 패치
// Feature: Write header first, patch size on close

writer.open("output.wav", 48000, 1);  // 헤더 쓰기 / Write header
while (recording) {
    writer.write(samples, count);      // 데이터 추가 / Append data
}
writer.close();                        // 헤더 크기 패치 / Patch header sizes
```

---

### 🧪 기능 7: 시뮬레이션 모드 / Feature 7: Simulation Mode

**한국어**

**문제**: 알려진 파라미터로 테스트해야 한다.

**해결**: `WatchSynthStream`으로 합성 신호 생성

**English**

**Problem**: Need to test with known parameters.

**Solution**: Generate synthetic signal with `WatchSynthStream`

```cpp
// WatchSynthStream.cpp
WatchSynthStreamConfig cfg;
cfg.bph = 21600;                    // BPH
cfg.rate_error_s_per_day = 10.0;    // +10 s/day
cfg.beat_error_ms = 0.5;            // 0.5ms 비트 오차 / beat error
cfg.watch_amplitude_degrees = 270;  // 진폭 / Amplitude

// Ground Truth가 있어서 알고리즘 검증 가능
// Ground truth available for algorithm verification
```

---

## 2. 실행 흐름 — Start 버튼부터 계측값 출력까지 / Execution Flow — From Start Button to Measurement Output

### 2-1. Start 버튼 클릭 / Start Button Click

```
on_StartPushButton_clicked()          MainWindow.cpp:1484
  ├─ Mode == "Live"     → LiveStart()
  ├─ Mode == "Playback" → PlaybackStart()
  └─ Mode == "Sim"      → SimStart()
```

**한국어**

세 모드 모두 이후 DSP 처리 경로는 동일하다. 차이는 링버퍼에 데이터를 채우는 주체뿐이다.

**English**

All three modes share the same DSP processing path afterward. The only difference is which entity fills the ring buffer.

---

### 2-2. 스레드 생성 (LiveStart 기준) / Thread Creation (LiveStart)

```
LiveStart()                           MainWindow.cpp:1376
  └─ StartAudioThread()               MainWindow.cpp:627

StartAudioThread():
  1. TMasterAudioDataRaw 링버퍼 할당 / Allocate ring buffer
       크기/Size = SampleRate(48000) × 30초/sec = 1,440,000 floats
  2. TAudioWorker 생성 / Create TAudioWorker
  3. mAudioWorker.moveToThread()      ← 별도 스레드로 격리 / isolate to dedicated thread
  4. 시그널-슬롯 연결 / Signal-slot connections:
       AudioDataReady   →  HandleAudioInput   (데이터 도착 알림 / new data notification)
       LocalStartAudio  →  StartAudioRecording
       LocalStopAudio   →  StopAudioRecording
  5. mAudioWorkerThread->start(TimeCriticalPriority)
  6. emit LocalStartAudio(...)        ← 워커 스레드에서 마이크 열기 / open mic from worker thread
```

---

### 2-3. 워커 스레드: 마이크 → 링버퍼 / Worker Thread: Microphone → Ring Buffer

```
[워커 스레드 / Worker Thread]

QAudioSource::readyRead 시그널 발생 / signal fires (PCM 데이터 도착 / data arrives from mic)
  └─ TAudioWorker::ProcessAudioInput()    AudioWorker.cpp

      1. mAudioInputDevice->readAll()     ← float PCM 바이트 읽기 / read float PCM bytes
      2. 뮤텍스 잠금 / Lock mutex
      3. memcpy → 링버퍼[WriteIndex]     ← 원형 덮어쓰기 / circular overwrite
         WriteIndex = (WriteIndex + N) % BufferSize
      4. TotalSamplesWritten += N
      5. 뮤텍스 해제 / Unlock mutex
      6. emit AudioDataReady()            ← 메인 스레드에 신호 / signal main thread
```

---

### 2-4. 메인 스레드: DSP 파이프라인 / Main Thread: DSP Pipeline

```
[메인 스레드 / Main Thread] AudioDataReady 시그널 수신 / Receives signal

HandleAudioInput()                    MainWindow.cpp:892
  └─ HandleInputData(mRawAudio)
       └─ ProcessSamples(mRawAudio)   MainWindow.cpp:926
```

**한국어**

`ProcessSamples()` 내부는 아래 루프를 반복한다:

**English**

`ProcessSamples()` runs the following loop:

```
while (SamplesToAdd > 0):

  1. 링버퍼에서 최대 4096 샘플 읽기 → mInputBlock[]
     Read up to 4096 samples from ring buffer → mInputBlock[]

  2. mWavWriter->write(mInputBlock, slice)
        ← WAV 파일 동시 저장 (사용자가 녹음 중일 때)
        ← concurrent WAV file save (when user is recording)

  3. mSoundRenderer.processSamples(mInputBlock, slice)
        ← 사운드이미지 픽셀 계산 (타임그래퍼 특유의 2D 시각화)
        ← sound image pixel computation (timegrapher-specific 2D visualization)

  4. tg_process(mCtx, mInputBlock, slice, &r)
        ← DSP 파이프라인 전체 실행 (단일 함수)
        ← full DSP pipeline in a single call
        내부 순서 / Internal order:
          HPF(DC 제거) → Envelope(포락선) → Detector(A/C 검출)
          HPF (DC removal) → Envelope → Detector (A/C detection)
          → BPH 감지(Rayleigh 위상 점수) → PLL 동기 추적
          → BPH detection (Rayleigh phase score) → PLL sync tracking
        출력 / Outputs:
          r.processed_pcm[]   : 처리된 파형 (오실로스코프용) / processed waveform
          r.onset_threshold   : 현재 검출 임계값 / current detection threshold
          r.events[]          : A/C 이벤트 배열 / array of A/C events
          r.sync_status       : NOT_SYNCED / SYNCED / MISMATCH
          r.detected_bph      : 감지된 BPH / detected BPH

  5. r.processed_pcm → ui->ScopePlot 에 실시간 추가
     r.processed_pcm → added to ui->ScopePlot in real time
        (오실로스코프 파형 + 임계값 선 / oscilloscope waveform + threshold line)

  6. for each r.events[i]:

       TG_EVENT_A (탈진기 언락, 'Tic' 소리 시작 / escapement unlock, start of 'Tic' sound):
         → AddVerticalMarker(ScopePlot, 녹색/green)
         → A_Event(time, bph)
              ├─ ComputeRateError()   RollingLeastSquares로 선형회귀 / linear regression
              └─ ComputeBeatError()   A이벤트 간격 기록 / records A event interval
         → mSoundRenderer.markAEventAbsoluteSampleIndex() (녹색 픽셀 / green pixel)

       TG_EVENT_C (탈진기 드롭, 'Toc' 소리 최대 / escapement drop, 'Toc' sound peak):
         → AddVerticalMarker(ScopePlot, 빨간색/red)
         → C_Event(time, bph)
              └─ ComputeAmplitude()   A→C 간격으로 진폭 계산 / calculates amplitude from A→C interval
         → mSoundRenderer.markCEventAbsoluteSampleIndex() (파란색 픽셀 / blue pixel)
```

---

### 2-5. 계측값 계산 / Measurement Calculations

#### 레이트 오차 / Rate Error (s/day) — ComputeRateError()

```
x = A이벤트 샘플 인덱스 (시간) / A event sample index (time)
y = 이상적인 누적 보기 시간 / ideal cumulative watch time
RollingLeastSquares.Add(x, y) → 선형회귀 기울기 계산 / compute linear regression slope

레이트오차/Rate error = (실제기울기/actual slope / 이상기울기/ideal slope - 1) × 86400  [s/day]
```

#### 비트 오차 / Beat Error (ms) — ComputeBeatError()

```
A이벤트 3개를 기록 / Record 3 A events: t[0], t[1], t[2]
비트오차/Beat error = ((t[2]-t[1]) - (t[1]-t[0])) / 2 × 1000  [ms]
이상적인 시계/Ideal watch = 0ms (Tic:Toc = 50:50)
```

#### 진폭 / Amplitude (°) — ComputeAmplitude()

```
T1 = C이벤트 시간 - A이벤트 시간  [초/seconds]
진폭/Amplitude = arcsin(π × T1 × BPH/3600) × (180/π) × 2 / LiftAngle
```

---

### 2-6. UI 업데이트 / UI Update

```
DisplayResults()
  └─ 레이트오차, 비트오차, 진폭, BPH를 Qt 레이블에 표시
     Displays Rate Error, Beat Error, Amplitude, BPH in Qt labels
```

---

## 3. 스레드 구조 요약 / Thread Structure Summary

```
┌─────────────────────────────┐         ┌──────────────────────────────┐
│  메인 스레드 (Qt Event)      │         │   워커 스레드                 │
│  Main Thread (Qt Event)     │         │   Worker Thread               │
│                             │         │   (TimeCriticalPriority)     │
│  HandleAudioInput()         │ ◄─────  │  emit AudioDataReady()       │
│  ProcessSamples()           │ signal  │                              │
│    tg_process()             │         │  ProcessAudioInput()         │
│    A_Event() / C_Event()    │         │    memcpy → 링버퍼/ring buffer │
│    ScopePlot 업데이트/update │         │    QAudioSource (마이크/mic)  │
│    SoundImage 업데이트/update│         │                              │
│    DisplayResults()         │         │                              │
└─────────────────────────────┘         └──────────────────────────────┘
         ▲                                          │
         └──────────── TMasterAudioDataRaw ─────────┘
                       float 링버퍼 (30초) / ring buffer (30 sec)
                       뮤텍스로 WriteIndex 보호 / Mutex protects WriteIndex
                       워커: 쓰기만 / 메인: 읽기만
                       Worker: write only / Main: read only
```

---

## 4. 구현 현황 — 베이스코드 vs 과제 요구사항 / Implementation Status — Base Code vs. Assignment Requirements

### 과제에서 요구하는 11가지 그래프 / 11 Required Graphs (per overview.md)

| # | 그래프 / Graph | 설명 / Description | 구현 여부 / Status |
|---|----------------|-------------------|-------------------|
| 1 | **Trace Display** | 레이트 편차 + 진폭을 시간축으로 연속 기록 (평활화 포함) / Continuous recording of rate deviation + amplitude over time (with smoothing) | ❌ 미구현 / Not implemented |
| 2 | **Vario Display** | 레이트/진폭의 Min/Max/Avg/σ 통계, 허용범위 구분 / Min/Max/Avg/σ statistics with acceptable range bands | ❌ 미구현 / Not implemented |
| 3 | **Multi-Position Sequence Display** | 최대 10개 자세 측정 결과 비교 (X·D 요약값 포함) / Compare up to 10 position measurements | ❌ 미구현 / Not implemented |
| 4 | **Beat-Noise Scope 1** | 개별 비트 노이즈 파형 (스트립 뷰, 20/200/400ms 범위) / Individual beat noise waveform (strip view) | ❌ 미구현 / Not implemented |
| 5 | **Beat-Noise Scope 2** | Tic/Toc 이중 축, 평균화 / Tic/Toc dual axis with averaging | ❌ 미구현 / Not implemented |
| 6 | **Beat Error Display & Diagnostic Trace** | 비트오차 값 + 트레이스 그래프 / Beat error values + trace graph | ❌ 미구현 / Not implemented |
| 7 | **Long-Term Performance Graph** | 레이트/진폭/비트오차 장기 변화 추이 / Long-term trends | ❌ 미구현 / Not implemented |
| 8 | **Escapement Analyzer & Marker-Line Display** | A/C 이벤트 마커 + ms 레이블 표시 / A/C event markers + ms labels | 🔶 부분 구현 / Partial (ScopePlot markers only) |
| 9 | **Time-Frequency Spectrogram** | 시간-주파수 에너지 분포 (색상 강도 = 신호 강도) / Time-frequency energy distribution | ❌ 미구현 / Not implemented |
| 10 | **Waveform Comparison Display** | 정렬된 연속 비트 파형 비교 + 타이밍 마커 / Aligned beat waveform comparison | ❌ 미구현 / Not implemented |
| 11 | **Scope Mode (Synchronized Sweep)** | 오실로스코프 스타일 고정 스윕 윈도우 / Oscilloscope-style fixed sweep window | 🔶 부분 구현 / Partial (ScopePlot exists but no synchronized sweep) |
| (+) | **Scope Function (F0/F1/F2/F3 Filter Views)** | 4가지 필터 처리 뷰 동시 표시 / 4 filter views simultaneously | ❌ 미구현 / Not implemented |

---

### 📋 그래프별 구현 가이드 — 수정/생성 파일 명세 / Implementation Guide — Files to Modify/Create

**한국어**

각 미구현 그래프를 구현할 때 **수정해야 할 기존 파일**과 **새로 생성해야 할 파일**을 명시한다.
목표 아키텍처(Observer 패턴 + 탭 분리)를 따를 경우의 가이드이다.

**English**

Specifies **existing files to modify** and **new files to create** for implementing each graph.
This guide assumes the target architecture (Observer pattern + tab separation).

---

#### 1. Trace Display

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/TraceTab.h` | 트레이스 탭 위젯 클래스 선언 / Trace tab widget class declaration |
| 🆕 신규 생성 / New | `src/TraceTab.cpp` | QCustomPlot 기반 Rate+Amplitude 시계열 그래프 / QCustomPlot-based Rate+Amplitude time series |
| 🆕 신규 생성 / New | `src/MeasurementStore.h/cpp` | 과거 측정값 저장소 (시계열 버퍼) / Historical measurement store (time series buffer) |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | `connect(MeasurementEngine, &measurementUpdated, TraceTab, &onMeasurement)` 추가 / Add signal connection |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | 탭 위젯에 Trace 탭 추가 / Add Trace tab to tab widget |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `ComputeRateError()`, `ComputeAmplitude()` 결과값 → `MeasurementStore` → `TraceTab`

---

#### 2. Vario Display

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/VarioTab.h/cpp` | Min/Max/Avg/σ 통계 막대 그래프 / Statistics bar chart with min/max/avg/σ |
| ✏️ 수정 / Modify | `src/MeasurementStore.h/cpp` | `getStatistics()` 메서드 추가 / Add `getStatistics()` method |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | VarioTab 시그널 연결 / Connect VarioTab signals |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Vario 탭 추가 / Add Vario tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `MeasurementStore::getStatistics()` → `VarioTab`

---

#### 3. Multi-Position Sequence Display

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/MultiPositionTab.h/cpp` | 최대 10개 자세 비교 테이블/그래프 / Up to 10 position comparison table/graph |
| 🆕 신규 생성 / New | `src/PositionSession.h/cpp` | 자세별 측정 세션 관리 클래스 / Per-position measurement session manager |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | 자세 저장 버튼 핸들러 + 세션 관리 로직 / Position save button handler + session management |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | MultiPosition 탭 + 자세 저장 버튼 / MultiPosition tab + position save button |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `PositionSession` 배열 (최대 10개) / Array of `PositionSession` (up to 10)

---

#### 4. Beat-Noise Scope 1

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/BeatNoiseScope1Tab.h/cpp` | 개별 비트 파형 스트립 뷰 (20/200/400ms) / Individual beat waveform strip view |
| 🆕 신규 생성 / New | `src/BeatWaveformBuffer.h/cpp` | 비트 단위 PCM 파형 버퍼 / Per-beat PCM waveform buffer |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | A이벤트 발생 시 파형 버퍼에 저장 + 탭 연결 / Store waveform on A event + connect tab |
| ✏️ 수정 / Modify | `src/Timegrapher.h` | `tg_result_t`에 비트 경계 정보 추가 (선택적) / Add beat boundary info to `tg_result_t` (optional) |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Beat-Noise Scope 1 탭 추가 / Add Beat-Noise Scope 1 tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `r.processed_pcm[]` (A이벤트 전후 윈도우) / (window around A event)

---

#### 5. Beat-Noise Scope 2

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/BeatNoiseScope2Tab.h/cpp` | Tic/Toc 이중 축 + 평균화 그래프 / Dual-axis Tic/Toc with averaging |
| ✏️ 수정 / Modify | `src/BeatWaveformBuffer.h/cpp` | Tic/Toc 분리 저장 + Σ 평균 계산 / Separate Tic/Toc storage + Σ average calculation |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | BeatNoiseScope2Tab 시그널 연결 / Connect BeatNoiseScope2Tab signals |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Beat-Noise Scope 2 탭 추가 / Add Beat-Noise Scope 2 tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `BeatWaveformBuffer`의 Tic/Toc 분리 평균 / Tic/Toc separated averages from `BeatWaveformBuffer`

---

#### 6. Beat Error Display & Diagnostic Trace

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/BeatErrorTab.h/cpp` | 비트오차 값 표시 + 트레이스 그래프 / Beat error value display + trace graph |
| ✏️ 수정 / Modify | `src/MeasurementStore.h/cpp` | Beat Error 시계열 저장 추가 / Add Beat Error time series storage |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | BeatErrorTab 시그널 연결 / Connect BeatErrorTab signals |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Beat Error 탭 추가 / Add Beat Error tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `ComputeBeatError()` 결과값 → `MeasurementStore` → `BeatErrorTab`

---

#### 7. Long-Term Performance Graph

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/LongTermTab.h/cpp` | 장기 트렌드 그래프 (Rate/Amplitude/BeatError) / Long-term trend graph |
| ✏️ 수정 / Modify | `src/MeasurementStore.h/cpp` | 확장된 히스토리 버퍼 (분/시간 단위) / Extended history buffer (minutes/hours) |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | LongTermTab 시그널 연결 / Connect LongTermTab signals |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Long-Term 탭 추가 / Add Long-Term tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `MeasurementStore`의 장기 히스토리 / Long-term history from `MeasurementStore`

---

#### 8. Escapement Analyzer & Marker-Line Display (확장 / Enhancement)

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/EscapementAnalyzerTab.h/cpp` | 전용 A/C 마커 분석 뷰 / Dedicated A/C marker analysis view |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | 기존 `AddVerticalMarker()` 로직 분리 + ms 레이블 강화 / Separate existing marker logic + enhance ms labels |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Escapement Analyzer 탭 추가 / Add Escapement Analyzer tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `r.events[]`의 A/C 이벤트 타이밍 / A/C event timing from `r.events[]`

---

#### 9. Time-Frequency Spectrogram

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/SpectrogramTab.h/cpp` | 2D 스펙트로그램 히트맵 렌더링 / 2D spectrogram heatmap rendering |
| 🆕 신규 생성 / New | `src/FFTProcessor.h/cpp` | STFT (Short-Time FFT) 처리기 / STFT processor |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | FFT 처리 + SpectrogramTab 연결 / FFT processing + SpectrogramTab connection |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Spectrogram 탭 추가 / Add Spectrogram tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 + FFT 라이브러리 링크 / Add new source files + FFT library link |

**데이터 소스 / Data Source**: `mInputBlock[]` 원시 PCM → `FFTProcessor` → 스펙트로그램 / Raw PCM → FFT → Spectrogram

**의존성 / Dependencies**: FFTW 또는 KissFFT 라이브러리 / FFTW or KissFFT library

---

#### 10. Waveform Comparison Display

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/WaveformCompareTab.h/cpp` | 정렬된 비트 파형 오버레이 비교 / Aligned beat waveform overlay comparison |
| ✏️ 수정 / Modify | `src/BeatWaveformBuffer.h/cpp` | 비트 정렬 알고리즘 추가 / Add beat alignment algorithm |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | WaveformCompareTab 시그널 연결 / Connect WaveformCompareTab signals |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Waveform Comparison 탭 추가 / Add Waveform Comparison tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `BeatWaveformBuffer`의 정렬된 연속 비트 / Aligned consecutive beats from `BeatWaveformBuffer`

---

#### 11. Scope Mode (Synchronized Sweep) (확장 / Enhancement)

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/SyncScopeTab.h/cpp` | BPH 동기화 스윕 오실로스코프 / BPH-synchronized sweep oscilloscope |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | 기존 ScopePlot 로직 참조 + 동기화 트리거 추가 / Reference existing ScopePlot + add sync trigger |
| ✏️ 수정 / Modify | `src/Bph.h/cpp` | 동기화 트리거 타이밍 정보 노출 / Expose sync trigger timing info |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Sync Scope 탭 추가 / Add Sync Scope tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `r.processed_pcm[]` + `r.sync_status` + BPH 기반 트리거 / + BPH-based trigger

---

#### (+) Scope Function (F0/F1/F2/F3 Filter Views)

| 구분 / Type | 파일 / File | 작업 내용 / Task |
|------------|-------------|-----------------|
| 🆕 신규 생성 / New | `src/FilterViewTab.h/cpp` | 4분할 필터 뷰 (F0~F3) / 4-way split filter view (F0~F3) |
| ✏️ 수정 / Modify | `src/Dsp.h/cpp` | 다양한 필터 프리셋 (Lowpass/Highpass/Bandpass/Raw) / Multiple filter presets |
| ✏️ 수정 / Modify | `src/Timegrapher.h/cpp` | 멀티 필터 출력 지원 (`tg_result_t` 확장) / Multi-filter output support (extend `tg_result_t`) |
| ✏️ 수정 / Modify | `src/MainWindow.cpp` | FilterViewTab 시그널 연결 / Connect FilterViewTab signals |
| ✏️ 수정 / Modify | `src/MainWindow.ui` | Filter View 탭 추가 / Add Filter View tab |
| ✏️ 수정 / Modify | `src/CMakeLists.txt` | 새 소스 파일 추가 / Add new source files |

**데이터 소스 / Data Source**: `tg_process()` 확장 — 4종 필터 처리 결과 동시 출력 / Extended `tg_process()` — 4 filter outputs simultaneously

---

### 📊 신규 파일 생성 요약 / New Files Summary

| 카테고리 / Category | 신규 파일 / New Files | 총 개수 / Count |
|--------------------|----------------------|-----------------|
| **탭 위젯 / Tab Widgets** | `TraceTab`, `VarioTab`, `MultiPositionTab`, `BeatNoiseScope1Tab`, `BeatNoiseScope2Tab`, `BeatErrorTab`, `LongTermTab`, `EscapementAnalyzerTab`, `SpectrogramTab`, `WaveformCompareTab`, `SyncScopeTab`, `FilterViewTab` | 12 × 2 = **24 파일** |
| **데이터 레이어 / Data Layer** | `MeasurementStore`, `PositionSession`, `BeatWaveformBuffer`, `FFTProcessor` | 4 × 2 = **8 파일** |
| **합계 / Total** | — | **32 신규 파일** |

### 📊 기존 파일 수정 요약 / Existing Files Modification Summary

| 파일 / File | 수정 빈도 / Frequency | 주요 변경 / Key Changes |
|-------------|----------------------|------------------------|
| `MainWindow.cpp` | 🔴 높음 / High | 모든 탭 시그널 연결 / All tab signal connections |
| `MainWindow.ui` | 🔴 높음 / High | 12개 탭 추가 / Add 12 tabs |
| `CMakeLists.txt` | 🔴 높음 / High | 32개 신규 파일 등록 / Register 32 new files |
| `MeasurementStore.h/cpp` | 🟡 중간 / Medium | 통계/히스토리 기능 확장 / Extend statistics/history |
| `Dsp.h/cpp` | 🟡 중간 / Medium | 필터 프리셋 추가 / Add filter presets |
| `Timegrapher.h/cpp` | 🟢 낮음 / Low | `tg_result_t` 확장 (선택적) / Extend `tg_result_t` (optional) |
| `Bph.h/cpp` | 🟢 낮음 / Low | 동기화 트리거 정보 노출 / Expose sync trigger info |

---

### 베이스코드에 이미 구현된 것 / Already Implemented in Base Code

| 기능 / Feature | 파일 / File | 비고 / Notes |
|---------------|-------------|--------------|
| Rate / Amplitude / Beat Error / BPH 계산 / calculation | `MainWindow.cpp` | `ComputeRateError`, `ComputeAmplitude`, `ComputeBeatError` |
| Sound Image (타임그래퍼 특유의 2D 시각화 / timegrapher-specific 2D visualization) | `SoundImageRenderer.cpp` | Tic=녹색/green, Toc=파란색/blue 픽셀/pixels |
| 기본 Scope (오실로스코프 / Basic oscilloscope) | `MainWindow.cpp` | `ui->ScopePlot` — envelope + 임계값/threshold + A/C 마커/markers |
| WAV 파일 녹음 / WAV file recording | `WavStreamWriter.cpp` | 스트리밍 저장 / Streaming save |
| Live / Playback / Sim 모드 / modes | 각 Worker / Each Worker | 세 모드 모두 동일 DSP 파이프라인 사용 / All modes share same DSP pipeline |
| 시계 소리 합성 / Watch sound synthesis | `WatchSynthStream.cpp` | Sim 모드용 / For Sim mode |
| BPH 자동 감지 + PLL 동기 / BPH auto-detection + PLL sync | `Bph.cpp`, `Timegrapher.cpp` | Rayleigh 위상 점수 + PLL / Rayleigh phase score + PLL |

### Construction Plan 우선순위 / Priority (milestone2/construction-plan.md)

```
Phase A (Must):   핵심 파이프라인 — RPi에서 Live 동작, 필터, 검출, 계측값 검증
                  Core pipeline — Live mode on RPi, filters, detection, measurement validation
Phase B (HIGH):   Trace Display, Vario, Beat Error Display, Pause/Rewind
Phase C (MEDIUM): Multi-Position, Beat-Noise Scope 1&2, Long-Term, Escapement Analyzer, Scope Mode, Scope Function
Phase D (LOW):    Spectrogram, Waveform Comparison, Watch Position GUI, AI classification
Phase E:          RPi 통합, 검증, 데모 준비 / RPi integration, validation, demo prep
```

### 현재 베이스코드 Gap 요약 / Current Base Code Gap Summary

**한국어**

베이스코드는 **신호 획득 → DSP → 기초 계측값 계산**까지만 완성된 상태다.
과제의 11가지 그래프 중 **완전히 구현된 것은 0개**, 부분 구현 2개(ScopePlot, SoundImage).

**English**

The base code is complete through **signal acquisition → DSP → basic measurement calculation**.
Of the 11 required graphs, **0 are fully implemented**, 2 are partially implemented (ScopePlot, SoundImage).

추가로 구현해야 할 핵심 작업 / Key tasks remaining:
1. **탭 기반 UI 확장 / Tab-based UI expansion** — 각 그래프를 별도 탭으로 추가 / add each graph as a separate tab (Extensibility QA)
2. **데이터 버퍼링 레이어 / Data buffering layer** — 장기 트레이스용 시계열 저장소 / time-series store for long-term trace
3. **그래프 렌더러 11종 / 11 graph renderers** — QCustomPlot 또는 커스텀 QWidget으로 구현
4. **Multi-Position 세션 관리 / session management** — 자세별 측정값 저장/비교 / per-position measurement storage
5. **Pause/Rewind 기능** — 링버퍼 기반 재생 제어 / ring-buffer-based playback control
6. **필터 뷰 (F0~F3) / Filter views** — 다양한 DSP 필터 파라미터 적용 및 동시 표시

---

## 5. 핵심 설계 포인트 / Key Design Points

| 포인트 / Point | 설명 / Description |
|---------------|-------------------|
| **Producer-Consumer** | 워커(쓰기)와 메인(읽기)이 링버퍼를 공유. 속도 차이를 30초 버퍼가 흡수 / Worker (write) and main thread (read) share ring buffer. 30-sec buffer absorbs speed differences |
| **단일 DSP 진입점 / Single DSP Entry Point** | `tg_process()` 하나로 HPF~PLL 전체 처리. 외부에서 내부 구조 몰라도 됨 / `tg_process()` encapsulates HPF through PLL. Caller doesn't need to know internal stages |
| **모드 무관 처리 / Mode-agnostic Processing** | Live/Playback/Sim 모두 같은 `ProcessSamples()` 경로 사용. 소스만 다름 / All modes use same `ProcessSamples()` path — only data source differs |
| **서브샘플 정밀도 / Sub-sample Precision** | A이벤트: 선형보간, C이벤트: 포물선보간으로 샘플 간격보다 정밀한 타이밍 / A event: linear interpolation; C event: parabolic interpolation for timing beyond sample resolution |
| **TimeCriticalPriority** | 워커 스레드를 최고 우선순위로 실행해 오디오 끊김 방지 / Worker thread runs at highest OS priority to prevent audio dropout |

---

## 6. 아키텍처 관점 설계 이슈 분석 / Architectural Design Issue Analysis

**한국어**

SW Architecture 과목 관점에서 **현재 코드의 구조적 문제**와 **팀이 계획한 목표 아키텍처와의 Gap**을 분석한다.

**English**

Analysis of **structural problems in the current code** and the **gap between the planned target architecture and the actual implementation**, from a Software Architecture course perspective.

---

### 6-1. 핵심 문제: MainWindow가 God Object / Core Problem: MainWindow is a God Object

**한국어**

현재 `MainWindow` 클래스는 혼자서 다음을 모두 담당한다.

**English**

The `MainWindow` class currently handles all of the following on its own:

| 책임 / Responsibility | 메서드 예시 / Example Methods |
|----------------------|------------------------------|
| 스레드 생명주기 관리 / Thread lifecycle management | `StartAudioThread()`, `StopPlaybackThread()` |
| DSP 파이프라인 실행 / DSP pipeline execution | `ProcessSamples()`, `tg_process()` 호출/call |
| **도메인 계산 / Domain calculations** | `ComputeRateError()`, `ComputeBeatError()`, `ComputeAmplitude()` |
| **계측 상태 보관 / Measurement state** | `mRateErrorEvents`, `mBeatErrorEvents`, `mAmplitudeEvents` (private 멤버/members) |
| UI 렌더링 / UI rendering | `AddVerticalMarker()`, `DisplayResults()` |
| 그래프 데이터 관리 / Graph data management | `xTic`, `yTic`, `xToc`, `yToc` 벡터 직접 관리 / Directly manages vectors |
| 파일 I/O / File I/O | `mWavWriter->write()` |
| 사운드카드 설정 / Sound card configuration | `ConfigureSoundCard()` |

**한국어**

**Private 메서드만 31개, 멤버 변수 30개 이상.** 전형적인 **God Class 안티패턴.**

**English**

**31+ private methods, 30+ member variables.** A textbook **God Class anti-pattern.**

---

### 6-2. 계획된 아키텍처 vs 현재 코드 Gap / Planned Architecture vs. Actual Code Gap

**한국어**

팀의 `milestone2/architecture-module-view.md`에는 4-레이어 구조가 설계되어 있다.

**English**

The team's `milestone2/architecture-module-view.md` specifies a 4-layer structure:

```
[계획된 목표 아키텍처 / Planned Target Architecture]      [현재 코드 현실 / Current Code Reality]

Presentation Layer                                      MainWindow (전부 여기 / everything here)
  └─ GraphTabManager                                         ↕ 없음 / does not exist
  └─ TraceTab, VarioTab, ...                                 ↕ 없음 / does not exist

Domain Layer                                                 ↕ 없음 / does not exist
  └─ MeasurementEngine          ←────────  MainWindow 내부에 묻혀있음 / buried inside MainWindow
  └─ MeasurementStore           ←────────  존재하지 않음 / does not exist

Signal Processing Layer                                      ↕ 없음 / does not exist
  └─ FilterChain                ←────────  tg_process() 안에 캡슐화됨 / encapsulated inside tg_process()
  └─ SignalBuffer                ←────────  TMasterAudioDataRaw가 부분 담당 / partially covered

Acquisition Layer                                            일부 존재 / partially exists
  └─ AudioCapture (추상화/abstraction) ←────────  존재하지 않음 / does not exist
  └─ LiveCapture / Playback / Sim ←───────  TAudioWorker 등 개별 클래스 존재 / exist as separate classes
```

**결론 / Conclusion**: 계획된 4레이어 중 실제로 구현된 레이어는 Acquisition 레이어의 일부뿐. / Of the planned 4 layers, only part of the Acquisition layer is actually implemented.

---

### 6-3. QAR-04 Extensibility 위반 — 새 그래프 추가 시 파급효과 / QAR-04 Extensibility Violation — Impact of Adding a New Graph

**한국어**

현재 구조에서 **새 그래프 탭을 하나 추가**하려면:

1. `MainWindow.h` 수정 — 새 그래프 위젯 멤버 추가
2. `MainWindow.cpp` 수정 — `ComputeRateError()` 내부에 직접 그래프 데이터 추가
3. `MainWindow.cpp` 수정 — `DisplayResults()` 수정
4. `MainWindow.ui` 수정 — Qt Designer에서 탭 추가

→ **매번 MainWindow를 열어야 한다.** 기존 코드 수정 없이 새 기능 추가 불가.

**English**

In the current structure, adding **one new graph tab** requires:

1. Modify `MainWindow.h` — add new graph widget member
2. Modify `MainWindow.cpp` — inject graph data directly inside `ComputeRateError()`
3. Modify `MainWindow.cpp` — update `DisplayResults()`
4. Modify `MainWindow.ui` — add tab in Qt Designer

→ **MainWindow must be touched every time.** Zero-change extension is impossible.

**한국어**

`architectural-drivers.md`의 QAR-04는 이렇게 명시한다:
> *"Adding a new graph tab requires changes to ≤ N files; zero changes to signal acquisition/processing pipeline"*

현재 코드는 이 요구사항을 **충족하지 못한다.**

**English**

`architectural-drivers.md` QAR-04 states:
> *"Adding a new graph tab requires changes to ≤ N files; zero changes to signal acquisition/processing pipeline"*

The current code **does not satisfy this requirement.**

---

### 6-4. 구체적 아키텍처 개선 방향 / Concrete Architectural Improvements

#### 개선 1: MeasurementEngine 추출 (Domain Layer 분리) / Improvement 1: Extract MeasurementEngine (Domain Layer Separation)

```
현재 / Current:
  MainWindow::ComputeRateError()  → 계산 + UI 직접 접근 / calculation + direct UI access
  MainWindow::ComputeAmplitude()  → 계산 + 멤버 상태 관리 / calculation + member state management

개선 후 / After:
  MeasurementEngine::onBeatEvent(BeatEvent)
    → Measurement { rate_sd, amplitude_deg, beat_error_ms, bph }
    → emit measurementUpdated(Measurement)   ← Qt Signal
```

**한국어**

MainWindow는 `MeasurementEngine`을 생성하고 시그널만 연결. 계산 로직은 모른다.

**English**

MainWindow only creates `MeasurementEngine` and connects signals. It has no knowledge of calculation logic.

#### 개선 2: MeasurementStore 신설 (히스토리 버퍼) / Improvement 2: Introduce MeasurementStore (History Buffer)

**한국어**

장기 트레이스/Vario/Long-Term 그래프는 과거 데이터가 필요하다. 현재는 존재하지 않는다.

**English**

Long-term trace, Vario, and Long-Term graphs require historical data. This does not currently exist.

```cpp
class MeasurementStore {
public:
    void append(const Measurement &m);
    QVector<Measurement> getHistory(int seconds) const;
    Measurement getLast() const;
    Statistics getStats() const;  // min/max/avg/σ for Vario
};
```

#### 개선 3: AudioCapture 추상화 (Acquisition Layer 통일) / Improvement 3: AudioCapture Abstraction (Unified Acquisition Layer)

**한국어**

현재 Live/Playback/Sim의 연결 코드가 세 곳에 거의 중복된다.

**English**

The connection boilerplate for Live/Playback/Sim is nearly duplicated in three places.

```cpp
class AudioCapture : public QObject {  // 인터페이스 / interface
signals:
    void dataReady();
    void done();
public slots:
    virtual void start(AudioConfig cfg) = 0;
    virtual void stop() = 0;
};
// LiveCapture / PlaybackCapture / SimCapture 가 각각 구현 / each implement this
```

**한국어**

`MainWindow`는 `AudioCapture *mCapture` 하나만 보유. 모드 전환은 인스턴스 교체.

**English**

`MainWindow` holds a single `AudioCapture *mCapture`. Mode switching = swapping instances.

#### 개선 4: Observer 패턴으로 그래프 디커플링 / Improvement 4: Observer Pattern for Graph Decoupling

```
현재 / Current:
  ComputeRateError() → ui->RatePlot->graph(0)->setData()  (직접 호출 / direct call)
  (MainWindow가 계산도 하고 그래프도 그린다 / MainWindow both calculates and renders)

개선 후 / After:
  MeasurementEngine::measurementUpdated(Measurement) ─Signal─►
    TraceTab::onMeasurement(Measurement)
    VarioTab::onMeasurement(Measurement)
    BeatErrorTab::onMeasurement(Measurement)
    ... 새 탭은 connect() 한 줄만 추가 / new tabs only need one connect() call
```

**한국어**

새 그래프 탭을 추가할 때 **MainWindow.cpp를 수정하지 않아도 된다.**

**English**

Adding a new graph tab **no longer requires modifying MainWindow.cpp.**

---

### 6-5. 아키텍처 패턴 및 전술 매핑 / Architecture Pattern and Tactic Mapping

| 이슈 / Issue | 적용할 패턴/전술 / Pattern/Tactic | 해결하는 QAR / QAR Addressed |
|-------------|--------------------------------|------------------------------|
| God Object (MainWindow) | **Layered Architecture** — 4레이어 엄격 분리 / strict 4-layer separation | QAR-04 Extensibility |
| 새 그래프마다 MainWindow 수정 / MainWindow modified for every new graph | **Observer / Publish-Subscribe** | QAR-04 Extensibility |
| Live/Playback/Sim 중복 구조 / Duplicated structure | **Strategy Pattern** (AudioCapture 추상화 / abstraction) | QAR-04, QAR-01 |
| 장기 히스토리 없음 / No long-term history | **Repository 패턴 / Pattern** (MeasurementStore) | FR-07 Long-Term Graph |
| 계산과 렌더링 결합 / Calculation and rendering coupled | **Separation of Concerns** | QAR-04, QAR-03 |
| 단위 테스트 불가 / Unit testing not possible | **Dependency Injection** — 의존성 외부에서 주입 / inject dependencies externally | QAR-03 Correctness |

---

### 6-6. 리팩토링 영향 분석 / Refactoring Impact Analysis

```
변경 범위가 큰 것 (먼저 결정해야 함) / Large change scope (decide first):
  ├─ MainWindow.h / .cpp — 계산 로직 전부 추출 / extract all calculation logic
  ├─ 신규 / New: MeasurementEngine.h / .cpp
  ├─ 신규 / New: MeasurementStore.h / .cpp
  └─ 신규 / New: AudioCapture.h (인터페이스 / interface)

변경 범위가 작은 것 (기존 코드 거의 유지) / Small change scope (existing code mostly preserved):
  ├─ TAudioWorker.cpp    — LiveCapture로 래핑 / wrap as LiveCapture
  ├─ TPlaybackWorker.cpp — PlaybackCapture로 래핑 / wrap as PlaybackCapture
  └─ TSimWorker.cpp      — SimCapture로 래핑 / wrap as SimCapture

건드리지 않아도 되는 것 / No changes needed:
  └─ Timegrapher.cpp, Detector.cpp, Bph.cpp, Dsp.cpp
     (tg_process C 라이브러리는 이미 잘 캡슐화되어 있음)
     (the tg_process C library is already well-encapsulated)
```

---

### 6-7. 아키텍처 과제 관점 핵심 메시지 / Core Message from an Architecture Course Perspective

**한국어**

> **현재 베이스코드는 "동작하는 코드"이지 "설계된 코드"가 아니다.**
> 팀이 milestone2 문서에 설계한 4레이어 아키텍처를 실제로 코드에 구현하는 것이
> 이 과제의 핵심 architectural challenge다.

특히 **QAR-04 Extensibility**가 핵심 평가 기준이다:
- Demo에서 *"새 그래프 추가 시 몇 개 파일을 수정했는가?"* 를 설명해야 함
- 현재 구조로는 답이 "MainWindow 하나에 전부" → 아키텍처 실패
- 목표 구조로는 "새 Tab 클래스 파일 1개 추가 + `connect()` 1줄" → 아키텍처 성공

**English**

> **The current base code is "working code", not "designed code".**
> Implementing the 4-layer architecture planned in the milestone2 documents into actual code
> is the core architectural challenge of this assignment.

**QAR-04 Extensibility** is the key evaluation criterion:
- The demo must answer: *"How many files did you change to add a new graph?"*
- Current structure → "Everything in MainWindow" → **architecture failure**
- Target structure → "1 new Tab class file + 1 `connect()` call" → **architecture success**

---

## 7. 코드 읽기 가이드 (신규 팀원용) / Code Reading Guide (For New Team Members)

**한국어**

어디서부터 코드를 읽어야 할지 모르겠다면 이 순서를 따르세요.

**English**

If you don't know where to start reading the code, follow this order.

---

### 📖 추천 읽기 순서 / Recommended Reading Order

#### Level 1: 전체 구조 파악 (30분) / Grasp Overall Structure (30 min)

| 순서 / Order | 파일 / File | 읽을 부분 / What to Read | 이해 목표 / Understanding Goal |
|-------------|-------------|-------------------------|-------------------------------|
| 1 | `MainWindow.h` | 클래스 선언부 전체 / Entire class declaration | 어떤 멤버/메서드가 있는지 / What members/methods exist |
| 2 | `SharedAudio.h` | 전체 (짧음) / Entire file (short) | 링버퍼 구조체 이해 / Ring buffer structure |
| 3 | `Timegrapher.h` | 주석 + `tg_event_t`, `tg_result_t` / Comments + structs | DSP 입출력 구조 / DSP input/output structure |

#### Level 2: 핵심 흐름 추적 (1시간) / Trace Core Flow (1 hour)

| 순서 / Order | 파일 / File | 읽을 부분 / What to Read | 이해 목표 / Understanding Goal |
|-------------|-------------|-------------------------|-------------------------------|
| 4 | `MainWindow.cpp` | `on_StartPushButton_clicked()` | 시작 진입점 / Entry point on start |
| 5 | `MainWindow.cpp` | `StartAudioThread()` | 스레드 생성 방법 / Thread creation method |
| 6 | `AudioWorker.cpp` | `ProcessAudioInput()` | 마이크 → 링버퍼 / Mic → ring buffer |
| 7 | `MainWindow.cpp` | `ProcessSamples()` | DSP 호출 + 이벤트 처리 / DSP call + event handling |

#### Level 3: 계측 로직 이해 (1시간) / Understand Measurement Logic (1 hour)

| 순서 / Order | 파일 / File | 읽을 부분 / What to Read | 이해 목표 / Understanding Goal |
|-------------|-------------|-------------------------|-------------------------------|
| 8 | `MainWindow.cpp` | `A_Event()`, `C_Event()` | 이벤트 핸들러 / Event handlers |
| 9 | `MainWindow.cpp` | `ComputeRateError()` | 레이트 계산 알고리즘 / Rate calculation algorithm |
| 10 | `MainWindow.cpp` | `ComputeBeatError()` | 비트 오차 계산 / Beat error calculation |
| 11 | `MainWindow.cpp` | `ComputeAmplitude()` | 진폭 계산 / Amplitude calculation |

#### Level 4: DSP 내부 (선택, 2시간) / DSP Internals (Optional, 2 hours)

| 순서 / Order | 파일 / File | 읽을 부분 / What to Read | 이해 목표 / Understanding Goal |
|-------------|-------------|-------------------------|-------------------------------|
| 12 | `Dsp.cpp` | `tg_hpf_process()`, `tg_envelope_process()` | 필터 동작 / Filter operation |
| 13 | `Detector.cpp` | `tg_detector_process()` | A/C 검출 알고리즘 / A/C detection algorithm |
| 14 | `Bph.cpp` | `tg_phase_score()`, `tg_sync_update()` | BPH 감지/동기화 / BPH detection/sync |

---

### 🔍 디버깅 시 확인할 위치 / Where to Check When Debugging

| 증상 / Symptom | 확인할 파일 / File to Check | 확인할 함수 / Function to Check |
|---------------|---------------------------|-------------------------------|
| 마이크 입력 안 됨 / Mic input not working | `AudioWorker.cpp` | `ProcessAudioInput()` |
| A/C 이벤트 검출 안 됨 / A/C events not detected | `Detector.cpp` | `tg_detector_process()` |
| BPH 인식 안 됨 / BPH not recognized | `Bph.cpp` | `tg_bph_pick_by_phase()` |
| 레이트 값 이상 / Abnormal rate value | `MainWindow.cpp` | `ComputeRateError()` |
| Sound Image 안 그려짐 / Sound Image not rendering | `SoundImageRenderer.cpp` | `processSamples()` |
| ScopePlot 업데이트 안 됨 / ScopePlot not updating | `MainWindow.cpp` | `ProcessSamples()` 내 ScopePlot 코드 |

---

### 📝 주요 상수/설정값 위치 / Key Constants/Settings Locations

| 설정 / Setting | 파일 / File | 위치 / Location |
|---------------|-------------|-----------------|
| 샘플레이트 / Sample rate | `MainWindow.cpp` | `ConfigureSoundCard()` |
| 링버퍼 크기 (30초) / Ring buffer size (30 sec) | `MainWindow.cpp` | `StartAudioThread()` |
| HPF 컷오프 (200Hz) / HPF cutoff | `Timegrapher.cpp` | `tg_config_default()` |
| Envelope 스무딩 (0.15ms) / smoothing | `Timegrapher.cpp` | `tg_config_default()` |
| BPH 후보 목록 / BPH candidate list | `Bph.cpp` | `TG_AUTO_BPH_LIST[]` |
| 위상 점수 임계값 (0.7) / Phase score threshold | `Bph.cpp` | `tg_bph_pick_by_phase()` |

---

### 🎯 기능 추가 시 수정할 파일 예측 / Files to Modify When Adding Features

| 추가할 기능 / Feature to Add | 수정할 파일 / Files to Modify |
|-----------------------------|------------------------------|
| 새 그래프 탭 / New graph tab | `MainWindow.cpp/h/ui` (현재 구조 / current structure) |
| 새 입력 모드 / New input mode | 새 Worker 클래스 / New Worker class + `MainWindow.cpp` |
| 새 계측값 / New measurement | `MainWindow.cpp` + 새 Compute 함수 / new Compute function |
| DSP 파라미터 변경 / DSP parameter change | `Timegrapher.cpp` 또는 / or `tg_config_t` |
| 파일 포맷 추가 / New file format | `WavStreamWriter.cpp` 또는 / or 새 Writer / new Writer |

---

### 💡 자주 묻는 질문 (FAQ) / Frequently Asked Questions

**Q: `tg_process()`는 어디서 호출되나요? / Where is `tg_process()` called?**
> A: `MainWindow.cpp`의 `ProcessSamples()` 함수 내부 / Inside `ProcessSamples()` function in `MainWindow.cpp`

**Q: A이벤트와 C이벤트의 차이는? / What's the difference between A and C events?**
> A: A = Tic 시작 (탈진기 열림), C = Toc 피크 (탈진기 닫힘) / A = Tic start (escapement opens), C = Toc peak (escapement closes)

**Q: BPH는 어떻게 자동 감지되나요? / How is BPH auto-detected?**
> A: `Bph.cpp`에서 Rayleigh 위상 점수로 후보 BPH 중 최고 점수 선택 / `Bph.cpp` selects highest Rayleigh phase score among candidate BPHs

**Q: 링버퍼가 가득 차면? / What happens when ring buffer is full?**
> A: 원형 버퍼라서 오래된 데이터를 덮어씀 (30초 히스토리 유지) / Circular buffer overwrites oldest data (maintains 30-sec history)

**Q: 왜 스레드를 분리했나요? / Why separate threads?**
> A: 오디오 캡처가 블로킹되면 UI가 멈추므로, TimeCriticalPriority 워커 스레드에서 처리 / Audio capture blocking would freeze UI, so processed in TimeCriticalPriority worker thread

**Q: QCustomPlot은 뭔가요? / What is QCustomPlot?**
> A: Qt 기반 오픈소스 그래프 라이브러리 / Qt-based open-source graphing library (https://www.qcustomplot.com)
