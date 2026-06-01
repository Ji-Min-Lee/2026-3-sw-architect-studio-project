# TimeGrapher 코드 구조 및 실행 흐름 분석

## 1. Static View 정확도 검증

`docs/static.pu`의 다이어그램과 실제 코드를 비교한 결과.

| 항목 | 다이어그램 | 실제 코드 | 판정 |
|------|-----------|-----------|------|
| AW/PW/SW → 링버퍼 | ✅ | memcpy로 TMasterAudioDataRaw에 기록 | 정확 |
| SA → MW → DSP 파이프라인 | ✅ | ProcessSamples() 내부에서 순차 처리 | 정확 |
| HPF → ENV → DET → BPH → TG 분리 | 개념적으로 정확 | 실제로는 `tg_process()` 단일 함수 호출로 캡슐화됨 | 개념 맞음, 코드 구조는 다름 |
| TG → MW (tg_event_t) | ✅ | `r.events[i]` 루프로 수신 | 정확 |
| MW → RLS / RA → STRUCTS → CP | ✅ | ComputeRateError(), ComputeBeatError() | 정확 |
| MW → SIR → SIW | ✅ | mSoundRenderer.processSamples() | 정확 |
| MW → WavStreamWriter | ✅ | mWavWriter->write() | 정확 |
| **ScopePlot (오실로스코프)** | ❌ 없음 | `ui->ScopePlot` — envelope 실시간 표시 | **누락** |

### 누락된 컴포넌트: ScopePlot

다이어그램에 없지만 실제로 존재한다. `tg_process()` 결과의 `processed_pcm`(처리된 오디오 파형)과 `onset_threshold`(검출 임계값)를 실시간으로 오실로스코프처럼 표시하는 QCustomPlot 그래프다. A/C 이벤트 발생 시 수직 마커와 시간 레이블도 여기에 추가된다.

---

## 2. 실행 흐름 — Start 버튼부터 계측값 출력까지

### 2-1. Start 버튼 클릭

```
on_StartPushButton_clicked()          MainWindow.cpp:1484
  ├─ Mode == "Live"     → LiveStart()
  ├─ Mode == "Playback" → PlaybackStart()
  └─ Mode == "Sim"      → SimStart()
```

세 모드 모두 이후 DSP 처리 경로는 동일하다. 차이는 링버퍼에 데이터를 채우는 주체뿐이다.

---

### 2-2. 스레드 생성 (LiveStart 기준)

```
LiveStart()                           MainWindow.cpp:1376
  └─ StartAudioThread()               MainWindow.cpp:627

StartAudioThread():
  1. TMasterAudioDataRaw 링버퍼 할당
       크기 = SampleRate(48000) × 30초 = 1,440,000 floats
  2. TAudioWorker 생성
  3. mAudioWorker.moveToThread()      ← 별도 스레드로 격리
  4. 시그널-슬롯 연결:
       AudioDataReady   →  HandleAudioInput   (데이터 도착 알림)
       LocalStartAudio  →  StartAudioRecording
       LocalStopAudio   →  StopAudioRecording
  5. mAudioWorkerThread->start(TimeCriticalPriority)
  6. emit LocalStartAudio(...)        ← 워커 스레드에서 마이크 열기
```

---

### 2-3. 워커 스레드: 마이크 → 링버퍼

```
[워커 스레드]

QAudioSource::readyRead 시그널 발생 (마이크에 PCM 데이터 도착)
  └─ TAudioWorker::ProcessAudioInput()    AudioWorker.cpp

      1. mAudioInputDevice->readAll()     ← float PCM 바이트 읽기
      2. 뮤텍스 잠금
      3. memcpy → 링버퍼[WriteIndex]     ← 원형 덮어쓰기
         WriteIndex = (WriteIndex + N) % BufferSize
      4. TotalSamplesWritten += N
      5. 뮤텍스 해제
      6. emit AudioDataReady()            ← 메인 스레드에 신호
```

---

### 2-4. 메인 스레드: DSP 파이프라인

```
[메인 스레드] AudioDataReady 시그널 수신

HandleAudioInput()                    MainWindow.cpp:892
  └─ HandleInputData(mRawAudio)
       └─ ProcessSamples(mRawAudio)   MainWindow.cpp:926
```

`ProcessSamples()` 내부는 아래 루프를 반복한다:

```
while (SamplesToAdd > 0):

  1. 링버퍼에서 최대 4096 샘플 읽기 → mInputBlock[]

  2. mWavWriter->write(mInputBlock, slice)
        ← WAV 파일 동시 저장 (사용자가 녹음 중일 때)

  3. mSoundRenderer.processSamples(mInputBlock, slice)
        ← 사운드이미지 픽셀 계산 (타임그래퍼 특유의 2D 시각화)

  4. tg_process(mCtx, mInputBlock, slice, &r)
        ← DSP 파이프라인 전체 실행 (단일 함수)
        내부 순서:
          HPF(DC 제거) → Envelope(포락선) → Detector(A/C 검출)
          → BPH 감지(Rayleigh 위상 점수) → PLL 동기 추적
        출력:
          r.processed_pcm[]   : 처리된 파형 (오실로스코프용)
          r.onset_threshold   : 현재 검출 임계값
          r.events[]          : A/C 이벤트 배열
          r.sync_status       : NOT_SYNCED / SYNCED / MISMATCH
          r.detected_bph      : 감지된 BPH

  5. r.processed_pcm → ui->ScopePlot 에 실시간 추가
        (오실로스코프 파형 + 임계값 선)

  6. for each r.events[i]:

       TG_EVENT_A (탈진기 언락, 'Tic' 소리 시작):
         → AddVerticalMarker(ScopePlot, 녹색)
         → A_Event(time, bph)
              ├─ ComputeRateError()   RollingLeastSquares로 선형회귀
              └─ ComputeBeatError()   A이벤트 간격 기록
         → mSoundRenderer.markAEventAbsoluteSampleIndex() (녹색 픽셀)

       TG_EVENT_C (탈진기 드롭, 'Toc' 소리 최대):
         → AddVerticalMarker(ScopePlot, 빨간색)
         → C_Event(time, bph)
              └─ ComputeAmplitude()   A→C 간격으로 진폭 계산
         → mSoundRenderer.markCEventAbsoluteSampleIndex() (파란색 픽셀)
```

---

### 2-5. 계측값 계산

#### 레이트 오차 (s/day) — ComputeRateError()

```
x = A이벤트 샘플 인덱스 (시간)
y = 이상적인 누적 보기 시간
RollingLeastSquares.Add(x, y) → 선형회귀 기울기 계산

레이트오차 = (실제기울기 / 이상기울기 - 1) × 86400  [s/day]
```

#### 비트 오차 (ms) — ComputeBeatError()

```
A이벤트 3개를 기록: t[0], t[1], t[2]
비트오차 = ((t[2]-t[1]) - (t[1]-t[0])) / 2 × 1000  [ms]
이상적인 시계 = 0ms (Tic:Toc = 50:50)
```

#### 진폭 (°) — ComputeAmplitude()

```
T1 = C이벤트 시간 - A이벤트 시간  [초]
진폭 = arcsin(π × T1 × BPH/3600) × (180/π) × 2 / LiftAngle
```

---

### 2-6. UI 업데이트

```
DisplayResults()
  └─ 레이트오차, 비트오차, 진폭, BPH를 Qt 레이블에 표시
```

---

## 3. 스레드 구조 요약

```
┌─────────────────────────────┐         ┌──────────────────────────────┐
│     메인 스레드 (Qt Event)   │         │   워커 스레드                 │
│                             │         │   (TimeCriticalPriority)     │
│  HandleAudioInput()         │ ◄─────  │  emit AudioDataReady()       │
│  ProcessSamples()           │ signal  │                              │
│    tg_process()             │         │  ProcessAudioInput()         │
│    A_Event() / C_Event()    │         │    memcpy → 링버퍼            │
│    ScopePlot 업데이트        │         │    QAudioSource (마이크)      │
│    SoundImage 업데이트       │         │                              │
│    DisplayResults()         │         │                              │
└─────────────────────────────┘         └──────────────────────────────┘
         ▲                                          │
         └──────────── TMasterAudioDataRaw ─────────┘
                       float 링버퍼 (30초)
                       뮤텍스로 WriteIndex 보호
                       워커: 쓰기만 / 메인: 읽기만
```

---

## 4. 구현 현황 — 베이스코드 vs 과제 요구사항

### 과제에서 요구하는 11가지 그래프 (overview.md 기준)

| # | 그래프 | 설명 | 구현 여부 |
|---|--------|------|-----------|
| 1 | **Trace Display** | 레이트 편차 + 진폭을 시간축으로 연속 기록 (평활화 포함) | ❌ 미구현 |
| 2 | **Vario Display** | 레이트/진폭의 Min/Max/Avg/σ 통계, 허용범위 구분 | ❌ 미구현 |
| 3 | **Multi-Position Sequence Display** | 최대 10개 자세 측정 결과 비교 (X·D 요약값 포함) | ❌ 미구현 |
| 4 | **Beat-Noise Scope 1** | 개별 비트 노이즈 파형 (스트립 뷰, 20/200/400ms 범위) | ❌ 미구현 |
| 5 | **Beat-Noise Scope 2** | Tic/Toc 이중 축, 평균화 | ❌ 미구현 |
| 6 | **Beat Error Display & Diagnostic Trace** | 비트오차 값 + 트레이스 그래프 | ❌ 미구현 |
| 7 | **Long-Term Performance Graph** | 레이트/진폭/비트오차 장기 변화 추이 | ❌ 미구현 |
| 8 | **Escapement Analyzer & Marker-Line Display** | A/C 이벤트 마커 + ms 레이블 표시 | 🔶 부분 구현 (ScopePlot의 마커만 존재) |
| 9 | **Time-Frequency Spectrogram** | 시간-주파수 에너지 분포 (색상 강도 = 신호 강도) | ❌ 미구현 |
| 10 | **Waveform Comparison Display** | 정렬된 연속 비트 파형 비교 + 타이밍 마커 | ❌ 미구현 |
| 11 | **Scope Mode (Synchronized Sweep)** | 오실로스코프 스타일 고정 스윕 윈도우 | 🔶 부분 구현 (ScopePlot이 유사하나 동기화 스윕 없음) |
| (+) | **Scope Function (F0/F1/F2/F3 Filter Views)** | 4가지 필터 처리 뷰 동시 표시 | ❌ 미구현 |

### 베이스코드에 이미 구현된 것

| 기능 | 파일 | 비고 |
|------|------|------|
| Rate / Amplitude / Beat Error / BPH 계산 | `MainWindow.cpp` | `ComputeRateError`, `ComputeAmplitude`, `ComputeBeatError` |
| Sound Image (타임그래퍼 특유의 2D 시각화) | `SoundImageRenderer.cpp` | Tic=녹색, Toc=파란색 픽셀 |
| 기본 Scope (오실로스코프) | `MainWindow.cpp` | `ui->ScopePlot` — envelope + 임계값 + A/C 마커 |
| WAV 파일 녹음 | `WavStreamWriter.cpp` | 스트리밍 저장 |
| Live / Playback / Sim 모드 | 각 Worker | 세 모드 모두 동일 DSP 파이프라인 사용 |
| 시계 소리 합성 | `WatchSynthStream.cpp` | Sim 모드용 |
| BPH 자동 감지 + PLL 동기 | `Bph.cpp`, `Timegrapher.cpp` | Rayleigh 위상 점수 + PLL |

### Construction Plan 우선순위 (milestone2/construction-plan.md)

```
Phase A (Must): 핵심 파이프라인 — RPi에서 Live 동작, 필터, 검출, 계측값 검증
Phase B (HIGH): Trace Display, Vario, Beat Error, Pause/Rewind
Phase C (MEDIUM): Multi-Position, Beat-Noise Scope 1&2, Long-Term, Escapement Analyzer, Scope Mode, Scope Function
Phase D (LOW/Optional): Spectrogram, Waveform Comparison, Watch Position GUI, AI 분류
Phase E: RPi 통합, 검증, 데모 준비
```

### 현재 베이스코드 Gap 요약

베이스코드는 **신호 획득 → DSP → 기초 계측값 계산**까지만 완성된 상태다.
과제의 11가지 그래프 중 **완전히 구현된 것은 0개**, 부분 구현 2개(ScopePlot, SoundImage).

추가로 구현해야 할 핵심 작업:
1. **탭 기반 UI 확장** — 각 그래프를 별도 탭으로 추가 (Extensibility QA 요구사항)
2. **데이터 버퍼링 레이어** — 장기 트레이스용 시계열 저장소
3. **그래프 렌더러 11종** — QCustomPlot 또는 커스텀 QWidget으로 구현
4. **Multi-Position 세션 관리** — 자세별 측정값 저장/비교
5. **Pause/Rewind 기능** — 링버퍼 기반 재생 제어
6. **필터 뷰 (F0~F3)** — 다양한 DSP 필터 파라미터 적용 및 동시 표시

---

## 5. 핵심 설계 포인트

| 포인트 | 설명 |
|--------|------|
| **Producer-Consumer** | 워커(쓰기)와 메인(읽기)이 링버퍼를 공유. 속도 차이를 30초 버퍼가 흡수 |
| **단일 DSP 진입점** | `tg_process()` 하나로 HPF~PLL 전체 처리. 외부에서 내부 구조 몰라도 됨 |
| **모드 무관 처리** | Live/Playback/Sim 모두 같은 `ProcessSamples()` 경로 사용. 소스만 다름 |
| **서브샘플 정밀도** | A이벤트: 선형보간, C이벤트: 포물선보간으로 샘플 간격보다 정밀한 타이밍 |
| **TimeCriticalPriority** | 워커 스레드를 최고 우선순위로 실행해 오디오 끊김 방지 |

---

## 6. 아키텍처 관점 설계 이슈 분석

> SW Architecture 과목 관점에서 **현재 코드의 구조적 문제**와 **팀이 계획한 목표 아키텍처와의 Gap**을 분석한다.

---

### 6-1. 핵심 문제: MainWindow가 God Object

현재 `MainWindow` 클래스는 혼자서 다음을 모두 담당한다.

| 책임 | 메서드 예시 |
|------|------------|
| 스레드 생명주기 관리 | `StartAudioThread()`, `StopPlaybackThread()` |
| DSP 파이프라인 실행 | `ProcessSamples()`, `tg_process()` 호출 |
| **도메인 계산** | `ComputeRateError()`, `ComputeBeatError()`, `ComputeAmplitude()` |
| **계측 상태 보관** | `mRateErrorEvents`, `mBeatErrorEvents`, `mAmplitudeEvents` (private 멤버) |
| UI 렌더링 | `AddVerticalMarker()`, `DisplayResults()` |
| 그래프 데이터 관리 | `xTic`, `yTic`, `xToc`, `yToc` 벡터 직접 관리 |
| 파일 I/O | `mWavWriter->write()` |
| 사운드카드 설정 | `ConfigureSoundCard()` |

**Private 메서드만 31개, 멤버 변수 30개 이상.** 전형적인 **God Class 안티패턴.**

---

### 6-2. 계획된 아키텍처 vs 현재 코드 Gap

팀의 `milestone2/architecture-module-view.md`에는 4-레이어 구조가 설계되어 있다.

```
[계획된 목표 아키텍처]                    [현재 코드 현실]

Presentation Layer                        MainWindow (전부 여기)
  └─ GraphTabManager                           ↕ 없음
  └─ TraceTab, VarioTab, ...                   ↕ 없음

Domain Layer                                   ↕ 없음
  └─ MeasurementEngine          ←────────  MainWindow 내부에 묻혀있음
  └─ MeasurementStore           ←────────  존재하지 않음

Signal Processing Layer                        ↕ 없음
  └─ FilterChain                ←────────  tg_process() 안에 캡슐화됨
  └─ SignalBuffer                ←────────  TMasterAudioDataRaw가 부분 담당

Acquisition Layer                              일부 존재
  └─ AudioCapture (추상화)       ←────────  존재하지 않음
  └─ LiveCapture / Playback / Sim ←───────  TAudioWorker 등 개별 클래스 존재
```

**결론: 계획된 4레이어 중 실제로 구현된 레이어는 Acquisition 레이어의 일부뿐.**

---

### 6-3. QAR-04 Extensibility 위반 — 새 그래프 추가 시 파급효과

현재 구조에서 **새 그래프 탭을 하나 추가**하려면:

1. `MainWindow.h` 수정 — 새 그래프 위젯 멤버 추가
2. `MainWindow.cpp` 수정 — `ComputeRateError()` 내부에 직접 그래프 데이터 추가
3. `MainWindow.cpp` 수정 — `DisplayResults()` 수정
4. `MainWindow.ui` 수정 — Qt Designer에서 탭 추가

→ **매번 MainWindow를 열어야 한다.** 기존 코드 수정 없이 새 기능 추가 불가.

`architectural-drivers.md`의 QAR-04는 이렇게 명시한다:
> *"Adding a new graph tab requires changes to ≤ N files; zero changes to signal acquisition/processing pipeline"*

현재 코드는 이 요구사항을 **충족하지 못한다.**

---

### 6-4. 구체적 아키텍처 개선 방향

#### 개선 1: MeasurementEngine 추출 (Domain Layer 분리)

```
현재:
  MainWindow::ComputeRateError()  → 계산 + UI 직접 접근
  MainWindow::ComputeAmplitude()  → 계산 + 멤버 상태 관리

개선 후:
  MeasurementEngine::onBeatEvent(BeatEvent)
    → Measurement { rate_sd, amplitude_deg, beat_error_ms, bph }
    → emit measurementUpdated(Measurement)   ← Qt Signal
```

MainWindow는 `MeasurementEngine`을 생성하고 시그널만 연결. 계산 로직은 모른다.

#### 개선 2: MeasurementStore 신설 (히스토리 버퍼)

장기 트레이스/Vario/Long-Term 그래프는 과거 데이터가 필요하다. 현재는 존재하지 않는다.

```cpp
class MeasurementStore {
public:
    void append(const Measurement &m);
    QVector<Measurement> getHistory(int seconds) const;
    Measurement getLast() const;
    Statistics getStats() const;  // min/max/avg/σ for Vario
};
```

#### 개선 3: AudioCapture 추상화 (Acquisition Layer 통일)

현재 Live/Playback/Sim의 연결 코드가 세 곳에 거의 중복된다.

```cpp
class AudioCapture : public QObject {  // 인터페이스
signals:
    void dataReady();
    void done();
public slots:
    virtual void start(AudioConfig cfg) = 0;
    virtual void stop() = 0;
};
// LiveCapture / PlaybackCapture / SimCapture 가 각각 구현
```

`MainWindow`는 `AudioCapture *mCapture` 하나만 보유. 모드 전환은 인스턴스 교체.

#### 개선 4: Observer 패턴으로 그래프 디커플링

```
현재:
  ComputeRateError() → ui->RatePlot->graph(0)->setData() 직접 호출
  (MainWindow가 계산도 하고 그래프도 그린다)

개선 후:
  MeasurementEngine::measurementUpdated(Measurement) ─Signal─►
    TraceTab::onMeasurement(Measurement)
    VarioTab::onMeasurement(Measurement)
    BeatErrorTab::onMeasurement(Measurement)
    ...새 탭은 connect() 한 줄만 추가
```

새 그래프 탭을 추가할 때 **MainWindow.cpp를 수정하지 않아도 된다.**

---

### 6-5. 아키텍처 패턴 및 전술 매핑

| 이슈 | 적용할 패턴/전술 | 해결하는 QAR |
|------|----------------|-------------|
| God Object (MainWindow) | **Layered Architecture** — 4레이어 엄격 분리 | QAR-04 Extensibility |
| 새 그래프마다 MainWindow 수정 | **Observer / Publish-Subscribe** | QAR-04 Extensibility |
| Live/Playback/Sim 중복 구조 | **Strategy Pattern** (AudioCapture 추상화) | QAR-04, QAR-01 |
| 장기 히스토리 없음 | **Repository 패턴** (MeasurementStore) | FR-07 Long-Term Graph |
| 계산과 렌더링 결합 | **Separation of Concerns** | QAR-04, QAR-03 |
| 단위 테스트 불가 | **Dependency Injection** — 의존성 외부에서 주입 | QAR-03 Correctness |

---

### 6-6. 리팩토링 영향 분석

```
변경 범위가 큰 것 (먼저 결정해야 함):
  ├─ MainWindow.h / .cpp — 계산 로직 전부 추출
  ├─ 신규: MeasurementEngine.h / .cpp
  ├─ 신규: MeasurementStore.h / .cpp
  └─ 신규: AudioCapture.h (인터페이스)

변경 범위가 작은 것 (기존 코드 거의 유지):
  ├─ TAudioWorker.cpp    — LiveCapture로 래핑
  ├─ TPlaybackWorker.cpp — PlaybackCapture로 래핑
  └─ TSimWorker.cpp      — SimCapture로 래핑

건드리지 않아도 되는 것:
  └─ Timegrapher.cpp, Detector.cpp, Bph.cpp, Dsp.cpp
     (tg_process C 라이브러리는 이미 잘 캡슐화되어 있음)
```

---

### 6-7. 아키텍처 과제 관점 핵심 메시지

> **현재 베이스코드는 "동작하는 코드"이지 "설계된 코드"가 아니다.**
> 팀이 milestone2 문서에 설계한 4레이어 아키텍처를 실제로 코드에 구현하는 것이
> 이 과제의 핵심 architectural challenge다.

특히 **QAR-04 Extensibility**가 핵심 평가 기준이다:
- Demo에서 *"새 그래프 추가 시 몇 개 파일을 수정했는가?"* 를 설명해야 함
- 현재 구조로는 답이 "MainWindow 하나에 전부" → 아키텍처 실패
- 목표 구조로는 "새 Tab 클래스 파일 1개 추가 + `connect()` 1줄" → 아키텍처 성공
