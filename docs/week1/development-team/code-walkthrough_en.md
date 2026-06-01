# TimeGrapher — Code Structure & Execution Flow Analysis

## 0. Feature-to-File Map (Quick Reference)

> **Quick reference table for team members new to the codebase**

### 🎯 Core Features → File Mapping

| Feature | File | Key Functions/Classes | One-line Description |
|---------|------|----------------------|---------------------|
| **App Entry Point** | `Main.cpp` | `main()` | Qt app initialization, MainWindow creation |
| **UI & Overall Control** | `MainWindow.cpp/h` | `MainWindow` | Hub for all features (God Object) |
| **Microphone Input** | `AudioWorker.cpp/h` | `TAudioWorker` | Real-time mic → ring buffer |
| **WAV Playback** | `PlaybackWorker.cpp/h` | `TPlaybackWorker` | WAV file → ring buffer |
| **Simulation** | `SimWorker.cpp/h` | `TSimWorker` | Synthesized signal → ring buffer |
| **Watch Sound Synthesis** | `WatchSynthStream.cpp/h` | `watch_synth_stream_*` | Tic-Toc signal generation for testing |
| **Signal Processing (DSP)** | `Dsp.cpp/h` | `tg_hpf_*`, `tg_envelope_*` | HPF + Envelope extraction |
| **A/C Event Detection** | `Detector.cpp/h` | `tg_detector_*` | Silence→Burst detection |
| **BPH Detection & Sync** | `Bph.cpp/h` | `tg_phase_score`, `tg_sync_*` | Rayleigh phase score + PLL |
| **DSP Unified API** | `Timegrapher.cpp/h` | `tg_process()` | Single entry point for above 3 |
| **Sound Image Visualization** | `SoundImageRenderer.cpp/h` | `SoundImageRenderer` | 2D folded image rendering |
| **WAV Save** | `WavStreamWriter.cpp/h` | `WavStreamWriter` | Streaming WAV file save |
| **Linear Regression (Rate)** | `RollingLeastSquares.cpp/h` | `RollingLeastSquares` | For rate error calculation |
| **Moving Average** | `RollingAverage.cpp/h` | `RollingAverage` | Beat error/amplitude smoothing |
| **Graph Library** | `qcustomplot.cpp/h` | `QCustomPlot` | Qt-based plotting library |

---

### 📁 Detailed File Roles

```
src/
├── Main.cpp                 # App entry point
├── MainWindow.cpp/h/ui      # UI + all logic (1,600+ lines, refactoring target)
│
├── [Audio Input Layer]
│   ├── AudioWorker.cpp/h    # Live: QAudioSource → ring buffer
│   ├── PlaybackWorker.cpp/h # Playback: WAV file → ring buffer
│   ├── SimWorker.cpp/h      # Sim: synthesizer → ring buffer
│   ├── WatchSynthStream.cpp/h # Watch sound synthesis engine
│   └── SharedAudio.h        # Ring buffer struct (TMasterAudioDataRaw)
│
├── [Signal Processing Layer]
│   ├── Timegrapher.cpp/h    # ★ DSP pipeline unified API
│   ├── Dsp.cpp/h            # HPF (DC removal) + Envelope
│   ├── Detector.cpp/h       # Silence/Burst based A/C event detection
│   └── Bph.cpp/h            # BPH auto-detection + PLL sync tracking
│
├── [Visualization Layer]
│   ├── SoundImageRenderer.cpp/h  # 2D Sound Image (timegrapher-specific)
│   ├── SoundImageWidget.cpp/h    # Sound Image Qt widget
│   └── qcustomplot.cpp/h         # Graph library (external)
│
├── [Utilities]
│   ├── RollingLeastSquares.cpp/h # Linear regression (rate calculation)
│   ├── RollingAverage.cpp/h      # Moving average
│   ├── WavStreamWriter.cpp/h     # WAV file save
│   └── WaveHeader.h              # WAV header struct
│
└── [Platform Audio]
    ├── LinuxAudio.cpp/h     # Linux ALSA volume control
    └── WindowsAudio.cpp/h   # Windows volume control
```

---

### 🔗 Data Flow at a Glance

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Data Flow                                       │
└─────────────────────────────────────────────────────────────────────────────┘

[Input Sources]              [Ring Buffer]           [Processing]        [Output]
                                  │
 AudioWorker ──┐                  │
 (Microphone)  │                  ▼
               ├──► TMasterAudioDataRaw ──► ProcessSamples()
 PlaybackWorker│     (30-sec float buffer)         │
 (WAV file)    │                                   │
               │                                   ├──► tg_process()
 SimWorker ────┘                                   │    (HPF→ENV→DET→BPH)
 (Synthesis)                                       │         │
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
                                                   └──► WavStreamWriter → WAV file
```

---

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

## 1.5. Core Features Detailed Explanation

> **Explains what problem each feature solves and how it works in code**

---

### 🎤 Feature 1: Audio Acquisition

**Problem**: Need to capture watch sounds in real-time.

**Solution**: Worker pattern supporting 3 input modes

| Mode | Class | Input Source | Use Case |
|------|-------|--------------|----------|
| **Live** | `TAudioWorker` | Microphone (QAudioSource) | Real watch measurement |
| **Playback** | `TPlaybackWorker` | WAV file | Re-analyze recorded sound |
| **Sim** | `TSimWorker` | `WatchSynthStream` | Test with known parameters |

**Key Code Location**:
```cpp
// AudioWorker.cpp - Microphone data reception
void TAudioWorker::ProcessAudioInput() {
    QByteArray data = mAudioInputDevice->readAll();  // Read from mic
    memcpy(&mRawAudio->RawData[WriteIndex], ...);    // Write to ring buffer
    emit AudioDataReady();                            // Notify main thread
}
```

**Ring Buffer Structure** (`SharedAudio.h`):
```cpp
typedef struct {
    float    *RawData;           // float PCM data (30 seconds worth)
    uint64_t  WriteIndex;        // Write position
    uint64_t  TotalSamplesWritten;
    QMutex   *Mutex;             // Thread synchronization
    int       SampleRate;        // 48000 Hz
    int       BufferSize;        // SampleRate × 30
} TMasterAudioDataRaw;
```

---

### 🔊 Feature 2: Signal Processing Pipeline (DSP)

**Problem**: Need to accurately detect Tic/Toc events from raw audio.

**Solution**: 4-stage pipeline encapsulated in single `tg_process()` call

```
┌─────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│   PCM   │ → │   HPF    │ → │ Envelope │ → │ Detector │ → A/C Events
│ (Raw)   │    │(DC Block)│    │          │    │          │
└─────────┘    └──────────┘    └──────────┘    └──────────┘
                                                    ↓
                                              ┌──────────┐
                                              │   BPH    │ → Sync Status
                                              │  (Sync)  │
                                              └──────────┘
```

| Stage | File | Function | Role |
|-------|------|----------|------|
| **1. HPF** | `Dsp.cpp` | `tg_hpf_process()` | Remove DC/hum below 200Hz |
| **2. Envelope** | `Dsp.cpp` | `tg_envelope_process()` | Rectify + 0.15ms LPF → envelope |
| **3. Detector** | `Detector.cpp` | `tg_detector_process()` | Silence/Burst analysis → A/C events |
| **4. BPH Sync** | `Bph.cpp` | `tg_sync_update()` | Rayleigh score + PLL tracking |

**Key Code Location**:
```cpp
// Timegrapher.cpp - Single entry point
tg_result_t r;
tg_process(mCtx, inputSamples, sampleCount, &r);

// Outputs:
// r.events[]        - A/C event array
// r.processed_pcm[] - Processed waveform (for oscilloscope)
// r.detected_bph    - Detected BPH (e.g., 21600)
// r.sync_status     - NOT_SYNCED / SYNCED / MISMATCH
```

**What are A/C Events?**
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

### 📊 Feature 3: Measurement Calculations

**Problem**: Need to calculate watch performance metrics from A/C events.

**Solution**: 3 core measurement calculations

| Measurement | Unit | Function | Meaning |
|-------------|------|----------|---------|
| **Rate Error** | s/day | `ComputeRateError()` | Gains/loses per day |
| **Beat Error** | ms | `ComputeBeatError()` | Tic:Toc balance |
| **Amplitude** | ° | `ComputeAmplitude()` | Balance wheel amplitude |

#### Rate Error

```cpp
// MainWindow.cpp - ComputeRateError()
// Principle: Linear regression slope of A event time vs ideal time

RollingLeastSquares.AddPoint(actual_time, ideal_time);
slope = regression_slope;
rate_error = (slope - 1.0) × 86400;  // Convert to s/day

// Example: slope=1.000116 → +10 s/day (gains 10 seconds per day)
```

#### Beat Error

```cpp
// MainWindow.cpp - ComputeBeatError()
// Principle: Compare intervals of 3 consecutive A events

t[0] = A event 1 time
t[1] = A event 2 time  
t[2] = A event 3 time

beat_error = ((t[2]-t[1]) - (t[1]-t[0])) / 2 × 1000;  // ms

// Example: 0.5ms → Tic is 0.5ms later than Toc
```

#### Amplitude

```cpp
// MainWindow.cpp - ComputeAmplitude()
// Principle: Reverse calculate from A→C time and BPH

T1 = C_event_time - A_event_time;  // seconds
amplitude = arcsin(π × T1 × BPH/3600) × (180/π) × 2 / LiftAngle;

// Example: 270° → Good balance wheel amplitude
```

---

### 🖼️ Feature 4: Sound Image Visualization

**Problem**: Need to generate timegrapher-specific 2D folded image.

**Solution**: `SoundImageRenderer` class

```
Sound Image Structure:
┌─────────────────────────────────────────┐
│ ← X-axis: Beat number (time elapsed) → │
│                                         │
│ ↑                                       │
│ Y-axis: Time position within beat       │
│ ↓                                       │
│                                         │
│  Green dot = A event (Tic)              │
│  Blue dot = C event (Toc)               │
│  Brightness = Signal magnitude          │
└─────────────────────────────────────────┘
```

**Key Code Location**:
```cpp
// SoundImageRenderer.cpp
void SoundImageRenderer::processSamples(const float* pcm, size_t count) {
    // 1. Calculate beat period based on BPH
    // 2. Fold samples by beat period
    // 3. Pixel brightness = normalized signal magnitude
    // 4. Move to next column at beat boundary
}

void SoundImageRenderer::markAEventAbsoluteSampleIndex(uint64_t idx) {
    // Mark green pixel at A event position
}

void SoundImageRenderer::markCEventAbsoluteSampleIndex(uint64_t idx) {
    // Mark blue pixel at C event position
}
```

---

### ⏱️ Feature 5: BPH Auto-Detection and Synchronization

**Problem**: Need to automatically determine watch's BPH (Beats Per Hour).

**Solution**: Rayleigh phase score + PLL tracking

```
Supported BPH (auto-detection):
  12000, 14400, 18000, 19800, 21600, 25200, 28800, 36000, 43200

Detection Algorithm:
  1. Collect events for ~1.5 seconds
  2. Calculate Rayleigh phase score for each candidate BPH
     score = |Σ e^(i × 2π × event_time / T)| / N
  3. Select highest-scoring BPH above threshold (0.7)
  4. Fine-track with PLL (drift correction)
```

**Key Code Location**:
```cpp
// Bph.cpp
double tg_phase_score(const double *event_times, size_t n, double period) {
    // Calculate phase alignment using Rayleigh statistics
    // 0.0 = random, 1.0 = perfect alignment
}

int tg_bph_pick_by_phase(event_times, n, candidate_list, ...) {
    // Select highest scoring BPH from all candidates
}

void tg_sync_update(tg_sync_t *s, tg_raw_event_t *ev) {
    // PLL-style fine adjustment of beat_period and ac_offset
}
```

---

### 💾 Feature 6: WAV File Save

**Problem**: Need to save audio being measured to file.

**Solution**: Streaming WAV save (`WavStreamWriter`)

```cpp
// WavStreamWriter.cpp
// Feature: Write header first, patch size on close

writer.open("output.wav", 48000, 1);  // Write header
while (recording) {
    writer.write(samples, count);      // Append data
}
writer.close();                        // Patch header sizes
```

---

### 🧪 Feature 7: Simulation Mode

**Problem**: Need to test with known parameters.

**Solution**: Generate synthetic signal with `WatchSynthStream`

```cpp
// WatchSynthStream.cpp
WatchSynthStreamConfig cfg;
cfg.bph = 21600;                    // BPH
cfg.rate_error_s_per_day = 10.0;    // +10 s/day
cfg.beat_error_ms = 0.5;            // 0.5ms beat error
cfg.watch_amplitude_degrees = 270;  // Amplitude

// Ground truth available for algorithm verification
```

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

---

## 7. Code Reading Guide (For New Team Members)

> **If you don't know where to start reading the code, follow this order**

---

### 📖 Recommended Reading Order

#### Level 1: Grasp Overall Structure (30 min)

| Order | File | What to Read | Understanding Goal |
|-------|------|--------------|-------------------|
| 1 | `MainWindow.h` | Entire class declaration | What members/methods exist |
| 2 | `SharedAudio.h` | Entire file (short) | Ring buffer structure |
| 3 | `Timegrapher.h` | Comments + `tg_event_t`, `tg_result_t` | DSP input/output structure |

#### Level 2: Trace Core Flow (1 hour)

| Order | File | What to Read | Understanding Goal |
|-------|------|--------------|-------------------|
| 4 | `MainWindow.cpp` | `on_StartPushButton_clicked()` | Entry point on start |
| 5 | `MainWindow.cpp` | `StartAudioThread()` | Thread creation method |
| 6 | `AudioWorker.cpp` | `ProcessAudioInput()` | Mic → ring buffer |
| 7 | `MainWindow.cpp` | `ProcessSamples()` | DSP call + event handling |

#### Level 3: Understand Measurement Logic (1 hour)

| Order | File | What to Read | Understanding Goal |
|-------|------|--------------|-------------------|
| 8 | `MainWindow.cpp` | `A_Event()`, `C_Event()` | Event handlers |
| 9 | `MainWindow.cpp` | `ComputeRateError()` | Rate calculation algorithm |
| 10 | `MainWindow.cpp` | `ComputeBeatError()` | Beat error calculation |
| 11 | `MainWindow.cpp` | `ComputeAmplitude()` | Amplitude calculation |

#### Level 4: DSP Internals (Optional, 2 hours)

| Order | File | What to Read | Understanding Goal |
|-------|------|--------------|-------------------|
| 12 | `Dsp.cpp` | `tg_hpf_process()`, `tg_envelope_process()` | Filter operation |
| 13 | `Detector.cpp` | `tg_detector_process()` | A/C detection algorithm |
| 14 | `Bph.cpp` | `tg_phase_score()`, `tg_sync_update()` | BPH detection/sync |

---

### 🔍 Where to Check When Debugging

| Symptom | File to Check | Function to Check |
|---------|---------------|-------------------|
| Mic input not working | `AudioWorker.cpp` | `ProcessAudioInput()` |
| A/C events not detected | `Detector.cpp` | `tg_detector_process()` |
| BPH not recognized | `Bph.cpp` | `tg_bph_pick_by_phase()` |
| Abnormal rate value | `MainWindow.cpp` | `ComputeRateError()` |
| Sound Image not rendering | `SoundImageRenderer.cpp` | `processSamples()` |
| ScopePlot not updating | `MainWindow.cpp` | ScopePlot code in `ProcessSamples()` |

---

### 📝 Key Constants/Settings Locations

| Setting | File | Location |
|---------|------|----------|
| Sample rate | `MainWindow.cpp` | `ConfigureSoundCard()` |
| Ring buffer size (30 sec) | `MainWindow.cpp` | `StartAudioThread()` |
| HPF cutoff (200Hz) | `Timegrapher.cpp` | `tg_config_default()` |
| Envelope smoothing (0.15ms) | `Timegrapher.cpp` | `tg_config_default()` |
| BPH candidate list | `Bph.cpp` | `TG_AUTO_BPH_LIST[]` |
| Phase score threshold (0.7) | `Bph.cpp` | `tg_bph_pick_by_phase()` |

---

### 🎯 Files to Modify When Adding Features

| Feature to Add | Files to Modify |
|----------------|-----------------|
| New graph tab | `MainWindow.cpp/h/ui` (current structure) |
| New input mode | New Worker class + `MainWindow.cpp` |
| New measurement | `MainWindow.cpp` + new Compute function |
| DSP parameter change | `Timegrapher.cpp` or `tg_config_t` |
| New file format | `WavStreamWriter.cpp` or new Writer |

---

### 💡 Frequently Asked Questions (FAQ)

**Q: Where is `tg_process()` called?**
> A: Inside `ProcessSamples()` function in `MainWindow.cpp`

**Q: What's the difference between A and C events?**
> A: A = Tic start (escapement opens), C = Toc peak (escapement closes)

**Q: How is BPH auto-detected?**
> A: `Bph.cpp` selects highest Rayleigh phase score among candidate BPHs

**Q: What happens when ring buffer is full?**
> A: Circular buffer overwrites oldest data (maintains 30-sec history)

**Q: Why separate threads?**
> A: Audio capture blocking would freeze UI, so processed in TimeCriticalPriority worker thread

**Q: What is QCustomPlot?**
> A: Qt-based open-source graphing library (https://www.qcustomplot.com)
