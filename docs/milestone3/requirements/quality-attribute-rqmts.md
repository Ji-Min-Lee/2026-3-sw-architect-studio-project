# Quality Attribute Requirements — TimeGrapher System

Quality attributes are prioritized by the degree of constraint they impose on architectural decisions.
Scenarios follow the Bass / Clements / Kazman format (source, stimulus, artifact, environment, response, response measure).

---

## Priority Ranking

| Rank | QA | Rationale |
|:----:|----|-----------|
| **1** | Real-Time Performance | Prerequisite for all other QAs — without continuous capture, there is no data |
| **2** | Measurement Accuracy | The reason the system exists; most directly shapes sps choice and detection algorithm |
| **3** | Low Latency | Hard threshold defined by beat period; exceeding it causes functional failure |
| **4** | Correctness | Structurally guaranteed by Accuracy + single data source; not a separate architectural concern |
| **5** | Extensibility | Controls schedule risk for implementing all 11 graph tabs |

---

## QAS-1: Real-Time Performance ★ Priority 1

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (acoustic signal source) |
| **Stimulus** | Continuous audio sample stream via USB sensor |
| **Artifact** | Full pipeline: capture → signal processing → domain computation → GUI display |
| **Environment** | Raspberry Pi 5, normal operating state |
| **Response** | Process all captured samples continuously without dropping audio blocks |
| **Response Measure** | Sustain 96,000 sps; dropped audio blocks = **0**; minimum viable = 48,000 sps |

**Architecture drivers**: [ADR 001](../ADRs/ADR001-dsp-offload-thread.md) · [ADR 002](../ADRs/ADR002-lazy-rendering.md) · [ADR 004](../ADRs/ADR004-qt-framework.md) · [ADR 005](../ADRs/ADR005-ring-buffer-connector.md)

---

## QAS-2: Measurement Accuracy ★ Priority 2

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (escapement vibration) |
| **Stimulus** | Acoustic signal containing T1(impulse) and T3(lock+banking) events |
| **Artifact** | Beat event detection module in the signal processing pipeline |
| **Environment** | Normal operating state; also degraded environment with weak or noisy signal |
| **Response** | Accurately detect T1·T3 onset/peak; compute Rate, Amplitude, and Beat Error; show a clear warning instead of unstable values under signal degradation |
| **Response Measure** | Rate error vs WeiShi No.1000: **< 5 s/d** (provisional); Beat Error deviation: **< 0.1 ms**; graceful degradation under signal noise |

---

## QAS-3: Low Latency ★ Priority 3

| Item | Detail |
|------|--------|
| **Source** | Mechanical watch (beat event source) |
| **Stimulus** | Audio sample block captured from microphone |
| **Artifact** | Full pipeline: capture → beat detection → computation → GUI rendering |
| **Environment** | Raspberry Pi 5, real-time operating state |
| **Response** | Waveform, markers, and computed values displayed in GUI before the next beat arrives |
| **Response Measure** | End-to-end latency **< 100 ms** (baseline for 28,800 BPH); capture → process **< 70 ms**; process → display **< 30 ms**; missed beats = 0 |

#### Latency Budget Derivation

The end-to-end latency budget is derived from the watch beat period with an 80% safety margin to absorb OS jitter and Qt rendering variation.

| BPH | Beat Period | 80% Budget | Capture → Process (70%) | Process → Display (30%) |
|-----|------------|-----------|------------------------|------------------------|
| 28,800 | 125 ms | **100 ms** | **70 ms** | **30 ms** |
| 36,000 | 100 ms | 80 ms | 56 ms | 24 ms |
| 43,200 | 83 ms | 66 ms | 46 ms | 20 ms |

**Architecture drivers**: [ADR 001](../ADRs/ADR001-dsp-offload-thread.md) · [ADR 005](../ADRs/ADR005-ring-buffer-connector.md)

---

## QAS-4: Correctness ★ Priority 4

| Item | Detail |
|------|--------|
| **Source** | User (same watch, same conditions) |
| **Stimulus** | Repeated measurement sessions on the same watch |
| **Artifact** | Domain computation layer (Rate, Amplitude, Beat Error calculation) |
| **Environment** | Normal operating state |
| **Response** | Each display tab shows the same derived values for the same underlying measurement data |
| **Response Measure** | Rate σ **≤ 1.0 s/d** across repeated sessions; no tab shows a different value from another for the same beat |

**Architecture note**: Correctness is structurally guaranteed by the single `MeasurementStore` — all tabs read from one source of truth. No additional architectural mechanism is required.

---

## QAS-5: Extensibility ★ Priority 5

| Item | Detail |
|------|--------|
| **Source** | Developer |
| **Stimulus** | Add a new graph tab that visualizes existing measurement data |
| **Artifact** | Presentation layer (`GraphTabManager` + new tab widget) |
| **Environment** | Development time |
| **Response** | New tab is fully functional with no changes to Domain, Signal Processing, or Acquisition layers |
| **Response Measure** | Zero modifications to files outside the Presentation layer; new tab registered in `GraphTabManager` only |

**Architecture drivers**: [ADR 003](../ADRs/ADR003-layered-architecture.md)
