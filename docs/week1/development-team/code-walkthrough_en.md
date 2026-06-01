# TimeGrapher — Code Structure & Execution Flow Analysis

## 1. Static View Accuracy Check

Comparison between `docs/static.pu` and the actual codebase.

| Item | Diagram | Actual Code | Verdict |
|------|---------|-------------|---------|
| AW/PW/SW → Ring Buffer | ✅ | memcpy into TMasterAudioDataRaw | Accurate |
| SA → MW → DSP Pipeline | ✅ | Sequential execution inside ProcessSamples() | Accurate |
| HPF → ENV → DET → BPH → TG separation | Conceptually correct | Actually encapsulated in a single `tg_process()` call | Concept OK, code structure differs |
| TG → MW (tg_event_t) | ✅ | Consumed via `r.events[i]` loop | Accurate |
| MW → RLS / RA → STRUCTS → CP | ✅ | ComputeRateError(), ComputeBeatError() | Accurate |
| MW → SIR → SIW | ✅ | mSoundRenderer.processSamples() | Accurate |
| MW → WavStreamWriter | ✅ | mWavWriter->write() | Accurate |
| **ScopePlot (oscilloscope)** | ❌ Missing | `ui->ScopePlot` — real-time envelope display | **Not in diagram** |

### Missing Component: ScopePlot

Exists in code but absent from the diagram. It is a QCustomPlot graph that displays `processed_pcm` (processed audio waveform) and `onset_threshold` (detection threshold) from `tg_process()` in real time, like an oscilloscope. Vertical markers and time labels are also added here whenever A/C events are detected.

---

## 2. Execution Flow — From Start Button to Measurement Output

### 2-1. Start Button Click

```
on_StartPushButton_clicked()          MainWindow.cpp:1484
  ├─ Mode == "Live"     → LiveStart()
  ├─ Mode == "Playback" → PlaybackStart()
  └─ Mode == "Sim"      → SimStart()
```

All three modes share the same DSP processing path afterward. The only difference is which entity fills the ring buffer.

---

### 2-2. Thread Creation (LiveStart)

```
LiveStart()                           MainWindow.cpp:1376
  └─ StartAudioThread()               MainWindow.cpp:627

StartAudioThread():
  1. Allocate TMasterAudioDataRaw ring buffer
       Size = SampleRate(48000) × 30 sec = 1,440,000 floats
  2. Create TAudioWorker
  3. mAudioWorker.moveToThread()      ← isolate to a dedicated thread
  4. Signal-slot connections:
       AudioDataReady   →  HandleAudioInput   (new data notification)
       LocalStartAudio  →  StartAudioRecording
       LocalStopAudio   →  StopAudioRecording
  5. mAudioWorkerThread->start(TimeCriticalPriority)
  6. emit LocalStartAudio(...)        ← open microphone from worker thread
```

---

### 2-3. Worker Thread: Microphone → Ring Buffer

```
[Worker Thread]

QAudioSource::readyRead signal fires (PCM data arrives from mic)
  └─ TAudioWorker::ProcessAudioInput()    AudioWorker.cpp

      1. mAudioInputDevice->readAll()     ← read float PCM bytes
      2. Lock mutex
      3. memcpy → ring buffer[WriteIndex] ← circular overwrite
         WriteIndex = (WriteIndex + N) % BufferSize
      4. TotalSamplesWritten += N
      5. Unlock mutex
      6. emit AudioDataReady()            ← signal main thread
```

---

### 2-4. Main Thread: DSP Pipeline

```
[Main Thread] Receives AudioDataReady signal

HandleAudioInput()                    MainWindow.cpp:892
  └─ HandleInputData(mRawAudio)
       └─ ProcessSamples(mRawAudio)   MainWindow.cpp:926
```

`ProcessSamples()` runs the following loop:

```
while (SamplesToAdd > 0):

  1. Read up to 4096 samples from ring buffer → mInputBlock[]

  2. mWavWriter->write(mInputBlock, slice)
        ← concurrent WAV file save (when user is recording)

  3. mSoundRenderer.processSamples(mInputBlock, slice)
        ← sound image pixel computation (timegrapher-specific 2D visualization)

  4. tg_process(mCtx, mInputBlock, slice, &r)
        ← full DSP pipeline in a single call
        Internal order:
          HPF (DC removal) → Envelope → Detector (A/C detection)
          → BPH detection (Rayleigh phase score) → PLL sync tracking
        Outputs:
          r.processed_pcm[]   : processed waveform (for oscilloscope)
          r.onset_threshold   : current detection threshold
          r.events[]          : array of A/C events
          r.sync_status       : NOT_SYNCED / SYNCED / MISMATCH
          r.detected_bph      : detected BPH

  5. r.processed_pcm → added to ui->ScopePlot in real time
        (oscilloscope waveform + threshold line)

  6. for each r.events[i]:

       TG_EVENT_A (escapement unlock, start of 'Tic' sound):
         → AddVerticalMarker(ScopePlot, green)
         → A_Event(time, bph)
              ├─ ComputeRateError()   linear regression via RollingLeastSquares
              └─ ComputeBeatError()   records A event interval
         → mSoundRenderer.markAEventAbsoluteSampleIndex() (green pixel)

       TG_EVENT_C (escapement drop, 'Toc' sound peak):
         → AddVerticalMarker(ScopePlot, red)
         → C_Event(time, bph)
              └─ ComputeAmplitude()   calculates amplitude from A→C interval
         → mSoundRenderer.markCEventAbsoluteSampleIndex() (blue pixel)
```

---

### 2-5. Measurement Calculations

#### Rate Error (s/day) — ComputeRateError()

```
x = A event sample index (time)
y = ideal cumulative watch time
RollingLeastSquares.Add(x, y) → compute linear regression slope

Rate error = (actual slope / ideal slope - 1) × 86400  [s/day]
```

#### Beat Error (ms) — ComputeBeatError()

```
Record 3 A events: t[0], t[1], t[2]
Beat error = |((t[2]-t[1]) - (t[1]-t[0])) / 2| × 1000  [ms]
Ideal watch = 0 ms (Tic:Toc = 50:50)
```

#### Amplitude (°) — ComputeAmplitude()

```
T1 = C event time - A event time  [seconds]
Amplitude = LiftAngle / sin(2π × T1 / (7200 / BPH))  [degrees]
```

---

### 2-6. UI Update

```
DisplayResults()
  └─ Displays Rate Error, Beat Error, Amplitude, BPH in Qt labels
```

---

## 3. Thread Structure Summary

```
┌─────────────────────────────┐         ┌──────────────────────────────┐
│     Main Thread (Qt Event)  │         │   Worker Thread               │
│                             │         │   (TimeCriticalPriority)     │
│  HandleAudioInput()         │ ◄─────  │  emit AudioDataReady()       │
│  ProcessSamples()           │ signal  │                              │
│    tg_process()             │         │  ProcessAudioInput()         │
│    A_Event() / C_Event()    │         │    memcpy → ring buffer      │
│    ScopePlot update         │         │    QAudioSource (microphone) │
│    SoundImage update        │         │                              │
│    DisplayResults()         │         │                              │
└─────────────────────────────┘         └──────────────────────────────┘
         ▲                                          │
         └──────────── TMasterAudioDataRaw ─────────┘
                       float ring buffer (30 sec)
                       Mutex protects WriteIndex
                       Worker: write only / Main: read only
```

---

## 4. Implementation Status — Base Code vs. Assignment Requirements

### 11 Required Graphs (per overview.md)

| # | Graph | Description | Status |
|---|-------|-------------|--------|
| 1 | **Trace Display** | Continuous recording of rate deviation + amplitude over time (with smoothing) | ❌ Not implemented |
| 2 | **Vario Display** | Min/Max/Avg/σ statistics for rate/amplitude with acceptable range bands | ❌ Not implemented |
| 3 | **Multi-Position Sequence Display** | Compare up to 10 position measurements (including X·D summary values) | ❌ Not implemented |
| 4 | **Beat-Noise Scope 1** | Individual beat noise waveform (strip view, 20/200/400 ms ranges) | ❌ Not implemented |
| 5 | **Beat-Noise Scope 2** | Tic/Toc dual axis with averaging | ❌ Not implemented |
| 6 | **Beat Error Display & Diagnostic Trace** | Beat error values + trace graph | ❌ Not implemented |
| 7 | **Long-Term Performance Graph** | Long-term trends of rate/amplitude/beat error | ❌ Not implemented |
| 8 | **Escapement Analyzer & Marker-Line Display** | A/C event markers + ms labels | 🔶 Partial (ScopePlot markers only) |
| 9 | **Time-Frequency Spectrogram** | Time-frequency energy distribution (color intensity = signal strength) | ❌ Not implemented |
| 10 | **Waveform Comparison Display** | Aligned continuous beat waveform comparison + timing markers | ❌ Not implemented |
| 11 | **Scope Mode (Synchronized Sweep)** | Oscilloscope-style fixed sweep window | 🔶 Partial (ScopePlot exists but no synchronized sweep) |
| (+) | **Scope Function (F0/F1/F2/F3 Filter Views)** | 4 filter processing views displayed simultaneously | ❌ Not implemented |

### Already Implemented in Base Code

| Feature | File | Notes |
|---------|------|-------|
| Rate / Amplitude / Beat Error / BPH calculation | `MainWindow.cpp` | `ComputeRateError`, `ComputeAmplitude`, `ComputeBeatError` |
| Sound Image (timegrapher-specific 2D visualization) | `SoundImageRenderer.cpp` | Tic=green, Toc=blue pixels |
| Basic Scope (oscilloscope) | `MainWindow.cpp` | `ui->ScopePlot` — envelope + threshold + A/C markers |
| WAV file recording | `WavStreamWriter.cpp` | Streaming save |
| Live / Playback / Sim modes | Each Worker | All modes share the same DSP pipeline |
| Watch sound synthesis | `WatchSynthStream.cpp` | For Sim mode |
| BPH auto-detection + PLL sync | `Bph.cpp`, `Timegrapher.cpp` | Rayleigh phase score + PLL |

### Construction Plan Priority (milestone2/construction-plan.md)

```
Phase A (Must):   Core pipeline — Live mode on RPi, filters, detection, measurement validation
Phase B (HIGH):   Trace Display, Vario, Beat Error Display, Pause/Rewind
Phase C (MEDIUM): Multi-Position, Beat-Noise Scope 1&2, Long-Term, Escapement Analyzer, Scope Mode, Scope Function
Phase D (LOW):    Spectrogram, Waveform Comparison, Watch Position GUI, AI classification
Phase E:          RPi integration, validation, demo prep
```

### Current Base Code Gap Summary

The base code is complete through **signal acquisition → DSP → basic measurement calculation**.
Of the 11 required graphs, **0 are fully implemented**, 2 are partially implemented (ScopePlot, SoundImage).

Key tasks remaining:
1. **Tab-based UI expansion** — add each graph as a separate tab (Extensibility QA requirement)
2. **Data buffering layer** — time-series store for long-term trace graphs
3. **11 graph renderers** — implemented with QCustomPlot or custom QWidget
4. **Multi-Position session management** — per-position measurement storage and comparison
5. **Pause/Rewind** — ring-buffer-based playback control
6. **Filter views (F0–F3)** — multiple DSP filter configurations displayed simultaneously

---

## 5. Key Design Points

| Point | Description |
|-------|-------------|
| **Producer-Consumer** | Worker (write) and main thread (read) share a ring buffer. 30-sec buffer absorbs speed differences |
| **Single DSP Entry Point** | `tg_process()` encapsulates HPF through PLL. Caller doesn't need to know internal stages |
| **Mode-agnostic Processing** | Live/Playback/Sim all use the same `ProcessSamples()` path — only the data source differs |
| **Sub-sample Precision** | A event: linear interpolation; C event: parabolic interpolation for timing beyond sample resolution |
| **TimeCriticalPriority** | Worker thread runs at highest OS priority to prevent audio dropout |

---

## 6. Architectural Design Issue Analysis

> Analysis of **structural problems in the current code** and the **gap between the planned target architecture and the actual implementation**, from a Software Architecture course perspective.

---

### 6-1. Core Problem: MainWindow is a God Object

The `MainWindow` class currently handles all of the following on its own:

| Responsibility | Example Methods |
|----------------|-----------------|
| Thread lifecycle management | `StartAudioThread()`, `StopPlaybackThread()` |
| DSP pipeline execution | `ProcessSamples()`, `tg_process()` call |
| **Domain calculations** | `ComputeRateError()`, `ComputeBeatError()`, `ComputeAmplitude()` |
| **Measurement state** | `mRateErrorEvents`, `mBeatErrorEvents`, `mAmplitudeEvents` (private members) |
| UI rendering | `AddVerticalMarker()`, `DisplayResults()` |
| Graph data management | Directly manages `xTic`, `yTic`, `xToc`, `yToc` vectors |
| File I/O | `mWavWriter->write()` |
| Sound card configuration | `ConfigureSoundCard()` |

**31+ private methods, 30+ member variables.** A textbook **God Class anti-pattern.**

---

### 6-2. Planned Architecture vs. Actual Code Gap

The team's `milestone2/architecture-module-view.md` specifies a 4-layer structure:

```
[Planned Target Architecture]             [Current Code Reality]

Presentation Layer                        MainWindow (everything here)
  └─ GraphTabManager                           ↕ does not exist
  └─ TraceTab, VarioTab, ...                   ↕ does not exist

Domain Layer                                   ↕ does not exist
  └─ MeasurementEngine          ←────────  buried inside MainWindow
  └─ MeasurementStore           ←────────  does not exist

Signal Processing Layer                        ↕ does not exist
  └─ FilterChain                ←────────  encapsulated inside tg_process()
  └─ SignalBuffer                ←────────  partially covered by TMasterAudioDataRaw

Acquisition Layer                              partially exists
  └─ AudioCapture (abstraction) ←────────  does not exist
  └─ LiveCapture / Playback / Sim ←───────  TAudioWorker etc. exist as separate classes
```

**Conclusion: Of the planned 4 layers, only part of the Acquisition layer is actually implemented.**

---

### 6-3. QAR-04 Extensibility Violation — Impact of Adding a New Graph

In the current structure, adding **one new graph tab** requires:

1. Modify `MainWindow.h` — add new graph widget member
2. Modify `MainWindow.cpp` — inject graph data directly inside `ComputeRateError()`
3. Modify `MainWindow.cpp` — update `DisplayResults()`
4. Modify `MainWindow.ui` — add tab in Qt Designer

→ **MainWindow must be touched every time.** Zero-change extension is impossible.

`architectural-drivers.md` QAR-04 states:
> *"Adding a new graph tab requires changes to ≤ N files; zero changes to signal acquisition/processing pipeline"*

The current code **does not satisfy this requirement.**

---

### 6-4. Concrete Architectural Improvements

#### Improvement 1: Extract MeasurementEngine (Domain Layer Separation)

```
Current:
  MainWindow::ComputeRateError()  → calculation + direct UI access
  MainWindow::ComputeAmplitude()  → calculation + member state management

After:
  MeasurementEngine::onBeatEvent(BeatEvent)
    → Measurement { rate_sd, amplitude_deg, beat_error_ms, bph }
    → emit measurementUpdated(Measurement)   ← Qt Signal
```

MainWindow only creates `MeasurementEngine` and connects signals. It has no knowledge of calculation logic.

#### Improvement 2: Introduce MeasurementStore (History Buffer)

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

#### Improvement 3: AudioCapture Abstraction (Unified Acquisition Layer)

The connection boilerplate for Live/Playback/Sim is nearly duplicated in three places.

```cpp
class AudioCapture : public QObject {  // interface
signals:
    void dataReady();
    void done();
public slots:
    virtual void start(AudioConfig cfg) = 0;
    virtual void stop() = 0;
};
// LiveCapture / PlaybackCapture / SimCapture each implement this
```

`MainWindow` holds a single `AudioCapture *mCapture`. Mode switching = swapping instances.

#### Improvement 4: Observer Pattern for Graph Decoupling

```
Current:
  ComputeRateError() → ui->RatePlot->graph(0)->setData()  (direct call)
  (MainWindow both calculates and renders)

After:
  MeasurementEngine::measurementUpdated(Measurement) ─Signal─►
    TraceTab::onMeasurement(Measurement)
    VarioTab::onMeasurement(Measurement)
    BeatErrorTab::onMeasurement(Measurement)
    ... new tabs only need one connect() call
```

Adding a new graph tab **no longer requires modifying MainWindow.cpp.**

---

### 6-5. Architecture Pattern and Tactic Mapping

| Issue | Pattern / Tactic | QAR Addressed |
|-------|-----------------|---------------|
| God Object (MainWindow) | **Layered Architecture** — strict 4-layer separation | QAR-04 Extensibility |
| MainWindow modified for every new graph | **Observer / Publish-Subscribe** | QAR-04 Extensibility |
| Duplicated Live/Playback/Sim structure | **Strategy Pattern** (AudioCapture abstraction) | QAR-04, QAR-01 |
| No long-term history buffer | **Repository Pattern** (MeasurementStore) | FR-07 Long-Term Graph |
| Calculation and rendering coupled | **Separation of Concerns** | QAR-04, QAR-03 |
| Unit testing not possible | **Dependency Injection** — inject dependencies externally | QAR-03 Correctness |

---

### 6-6. Refactoring Impact Analysis

```
Large change scope (decide first):
  ├─ MainWindow.h / .cpp — extract all calculation logic
  ├─ New: MeasurementEngine.h / .cpp
  ├─ New: MeasurementStore.h / .cpp
  └─ New: AudioCapture.h (interface)

Small change scope (existing code mostly preserved):
  ├─ TAudioWorker.cpp    — wrap as LiveCapture
  ├─ TPlaybackWorker.cpp — wrap as PlaybackCapture
  └─ TSimWorker.cpp      — wrap as SimCapture

No changes needed:
  └─ Timegrapher.cpp, Detector.cpp, Bph.cpp, Dsp.cpp
     (the tg_process C library is already well-encapsulated)
```

---

### 6-7. Core Message from an Architecture Course Perspective

> **The current base code is "working code", not "designed code".**
> Implementing the 4-layer architecture planned in the milestone2 documents into actual code
> is the core architectural challenge of this assignment.

**QAR-04 Extensibility** is the key evaluation criterion:
- The demo must answer: *"How many files did you change to add a new graph?"*
- Current structure → "Everything in MainWindow" → **architecture failure**
- Target structure → "1 new Tab class file + 1 `connect()` call" → **architecture success**
