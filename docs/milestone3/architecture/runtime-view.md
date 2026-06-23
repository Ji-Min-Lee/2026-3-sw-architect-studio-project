# Runtime View (Component & Connector View)

This view shows the runtime structure of the TimeGrapher system: which components run at execution time, in which threads, and how they communicate. The key architectural concern is thread-safe data flow from audio capture through the DSP pipeline to the UI.

```mermaid
graph TD
    subgraph AudioThread["Audio Thread"]
        AC["AudioCapture"]
    end

    subgraph DSPThread["DSP Thread - ADR 001"]
        DW["DSPWorker\nFilterChain + BeatDetector\n+ MeasurementEngine"]
    end

    subgraph UIThread["UI Thread"]
        CP["ControlPanel"]
        GTM["GraphTabManager"]
        GTs["Graph Tabs"]
    end

    subgraph Shared["Thread-Safe Shared State"]
        SB["SignalBuffer\nring buffer - ADR 005"]
        MS["MeasurementStore\nhistory store"]
        MQ["MeasurementQueue\nQt queued signal"]
    end

    AC -->|"PCM blocks"| SB
    SB -->|"PCM samples"| DW
    DW -->|"Measurement"| MQ
    DW -->|"append"| MS

    MQ -->|"thread-safe dispatch"| GTM
    GTM -->|"data update"| GTs
    MS -->|"history query"| GTs

    CP -->|"config change"| AC
    CP -->|"start / stop"| AC
```

## Element Catalog

#### AudioCapture (Audio Thread)
- Runs the ALSA / Qt Multimedia capture loop; writes fixed-size PCM blocks into the SignalBuffer.
- Does not perform any DSP; its only job is to keep the ring buffer filled without interruption.
- Receives configuration changes (sample rate, mode) from ControlPanel.

#### DSPWorker (DSP Thread)
- Introduced by [ADR 001](../ADRs/ADR001-dsp-offload-thread.md) to eliminate Qt event-loop blocking from the audio path.
- Owns the full DSP pipeline: FilterChain → BeatDetector → MeasurementEngine.
- Reads PCM blocks from SignalBuffer; publishes Measurement objects via MeasurementQueue to the UI thread.

#### GraphTabManager (UI Thread)
- Receives Measurement objects via Qt's queued signal mechanism (thread-safe cross-thread dispatch).
- Routes each measurement to the currently visible tab only — see [ADR 002](../ADRs/ADR002-lazy-rendering.md).
- On tab switch, triggers a catch-up repaint so the newly visible tab immediately shows the latest data.

#### Graph Tabs (UI Thread)
- Each tab visualizes a specific aspect of measurement data.
- Short-horizon tabs (Trace, Vario, BeatError) consume live Measurement objects from GraphTabManager.
- Long-horizon tabs (LongTerm, Spectrogram, WaveformComparison) query MeasurementStore for historical data on demand.

## Connector Types

| Connector | Type | Between | Properties |
|-----------|------|---------|------------|
| SignalBuffer | Ring buffer ([ADR 005](../ADRs/ADR005-ring-buffer-connector.md)) | AudioCapture → DSPWorker | Non-blocking write; backpressure absorbed by buffer depth |
| MeasurementQueue | Qt queued signal | DSPWorker → GraphTabManager | Thread-safe cross-thread dispatch; no manual locking needed |
| MeasurementStore | RW-locked shared store | DSPWorker → Graph Tabs | DSP thread appends; UI thread reads on demand |
| Direct call | Synchronous within DSP thread | FilterChain → BeatDetector → MeasurementEngine | No synchronization needed |
| Qt Signal (direct) | Synchronous Qt signal | ControlPanel → AudioCapture | UI-to-audio configuration updates |

## Behavior — Live Beat Processing Sequence

```mermaid
sequenceDiagram
    participant HW as USB Microphone
    participant AC as AudioCapture
    participant SB as SignalBuffer
    participant DW as DSPWorker
    participant MQ as MeasurementQueue
    participant GTM as GraphTabManager
    participant Tab as Active Graph Tab

    HW->>AC: new audio block
    AC->>SB: write PCM block
    Note over DW: waiting for data
    SB-->>DW: PCM block available
    DW->>DW: apply LP/HP filter
    DW->>DW: detect T1/T3 beat events
    DW->>DW: compute Rate, Amplitude, Beat Error
    DW->>MQ: enqueue Measurement
    MQ-->>GTM: deliver to UI thread
    GTM->>Tab: update with Measurement
    Tab->>Tab: render (visible tab only)
```

## Related ADRs
- [ADR 001 — DSP Offload Thread](../ADRs/ADR001-dsp-offload-thread.md)
- [ADR 002 — Lazy Rendering](../ADRs/ADR002-lazy-rendering.md)
- [ADR 004 — Qt as Application Framework](../ADRs/ADR004-qt-framework.md)
- [ADR 005 — Ring Buffer as Thread Boundary Connector](../ADRs/ADR005-ring-buffer-connector.md)

## Related Views
- [Module View](module-view.md)
- [Deployment View](deployment-view.md)
