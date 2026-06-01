# QA Scenarios — TimeGrapher

> **Format**: Quality Attribute Scenario (CMU SEI)  
> **Source**: Time Grapher Project Plan (Draft).pdf — p.25-26  
> **Date**: 2026-06-02

---

## QA 1: Real-Time Performance

| Field | Content |
|-------|---------|
| **Source** | Mechanical watch (acoustic signal source) |
| **Stimulus** | Continuous stream of acoustic samples from USB sensor |
| **Artifact** | Signal acquisition → processing → analysis → GUI display pipeline |
| **Environment** | Raspberry Pi 5 (8GB RAM), normal operating conditions |
| **Response** | Captured samples processed in real time without interruption and displayed on GUI |
| **Response Measure** | - Objective: sustain 96,000 sps processing<br>- Minimum: 48,000 sps (below this = failure)<br>- Stretch: 192,000 sps<br>- Zero dropped audio blocks |

### Domain Background

**sps (Samples Per Second)** = how many times per second the acoustic signal from the microphone is digitally measured.

```
Sample: one snapshot taken by the microphone
Beat:   one operation of the watch escapement (tic or tac)

For a 28,800 BPH watch:
- 8 beats per second (28,800 ÷ 3,600)
- At 96,000 sps: 12,000 samples per beat
```

Higher sps allows more precise localization of A·C event timing.

```
48,000 sps  → 1 sample = up to 0.021 ms error
96,000 sps  → 1 sample = up to 0.010 ms error
192,000 sps → 1 sample = up to 0.005 ms error
```

**Beat detection principle**: the system marks a beat whenever sample energy (amplitude²) exceeds a threshold. Within each beat, the first large peak = A(T1), the second peak = C(T3).

**Why RPi is the challenge**: sample acquisition + filtering + A·C detection + Rate·Beat Error·Amplitude calculation + rendering 11 GUI graphs must all run simultaneously without interruption. Dropped audio blocks → missed A·C events → metrics cannot be computed.

---

## QA 2: Low Latency

| Field | Content |
|-------|---------|
| **Source** | Mechanical watch (beat event source) |
| **Stimulus** | Acoustic sample block captured from microphone |
| **Artifact** | Entire pipeline: capture → beat detection → calculation → GUI rendering |
| **Environment** | Raspberry Pi 5, real-time operating conditions |
| **Response** | Waveform, markers, and computed values for the captured samples displayed on GUI |
| **Response Measure** | - ① capture→process latency (ms) — average + worst-case<br>- ② process→display latency (ms) — average + worst-case<br>- ③ end-to-end latency (ms) — average + worst-case<br>- Dropped audio block count<br>- Missed beat detection count<br>*(Specific target values finalized after Experiment 1·2 results)* |

### Domain Background

```
Watch produces tic sound
    ↓
Microphone captures samples
    ↓  ① capture→process
A·C event detection + Rate·Beat Error·Amplitude calculation
    ↓  ② process→display
Metrics and waveform displayed on GUI
```

```
[mic capture] ──①──▶ [A·C detection + calc] ──②──▶ [GUI display]
               ①: capture→process              ②: process→display
[mic capture] ─────────────③ end-to-end ─────────────▶ [GUI display]
```

A 28,800 BPH watch produces a beat every **125 ms**. If end-to-end latency exceeds 125 ms, the next beat has already arrived → backlog accumulates → dropped beats.

**Conflict with Real-Time Performance**:

```
Higher sps
    → more samples → longer ① processing time → harder to achieve Low Latency
```

How high to set sps = how much latency to tolerate. Target values are finalized after Experiment 1 and 2 results.

| Segment | What is measured | Reported metrics |
|---------|-----------------|-----------------|
| ① capture→process | End of sample capture to completion of A·C detection and calculation | average + worst-case (ms) |
| ② process→display | Calculation complete to display on GUI | average + worst-case (ms) |
| ③ end-to-end | Total of ①+② | average + worst-case (ms) |

---

## QA 3: Correctness

| Field | Content |
|-------|---------|
| **Source** | User (measuring with the same watch under the same conditions) |
| **Stimulus** | Multiple GUI views displayed simultaneously while receiving real-time acoustic signal |
| **Artifact** | Rate, Amplitude, Beat Error calculation logic and all GUI views |
| **Environment** | Normal operating conditions / environment with ambient noise |
| **Response** | All GUI views (Trace, Rate Stability, Beat Error, Sequence, etc.) display consistent values derived from the same data |
| **Response Measure** | - All views derived from the same beat data show matching values<br>- Measurements remain stable under ambient noise<br>*(Acceptable deviation range requires team consensus)* |

### Domain Background

Rate, Beat Error, and Amplitude are all computed from the **same A·C event timing**.

```
Same A event data
    ├──▶ Rate calculation    ──▶ Rate graph
    ├──▶ Beat Error calc     ──▶ Beat Error view
    └──▶ (A+C) Amplitude calc ──▶ Amplitude graph
```

Computing from different data sources produces inconsistencies where each view shows a different value. "Single data source" is the key architectural decision.

---

## QA 4: Measurement Accuracy

| Field | Content |
|-------|---------|
| **Source** | Mechanical watch (escapement vibration source) |
| **Stimulus** | Acoustic signal containing T1 (impulse) and T3 (lock+banking) events |
| **Artifact** | Beat event detection module in the signal processing pipeline |
| **Environment** | Normal operating conditions / degraded environment with weak signal or noise |
| **Response** | Accurately detect T1·T3 onset/peak and compute Rate, Amplitude, Beat Error. On signal degradation, display a clear warning instead of unstable values. |
| **Response Measure** | - Rate error vs. WeiShi No.1000: ___ s/d or less<br>- Beat Error error vs. WeiShi No.1000: ___ ms or less<br>- Graceful degradation on signal degradation (no unstable output)<br>*(Specific error thresholds finalized after Experiment 3 results)* |

### Domain Background

Impact of a 1-sample A(T1) event timing error:

```
At 96,000 sps, 1 sample = 0.010 ms

Rate:       directly affects tic/tac period calculation → s/day error
Beat Error: directly affects t1, t2 calculation → sensitive relative to 0.6 ms threshold
Amplitude:  directly affects t_AC = C − A → error amplified because it is in the denominator
```

**C(T3) events** are smaller and more irregular than A, making them harder to detect; because they appear in the denominator of the Amplitude formula (t_AC), their errors are amplified.

---

## QA 5: Extensibility

| Field | Content |
|-------|---------|
| **Source** | Developer (adding new graph or analysis feature) |
| **Stimulus** | Request to add new graph / filter / display mode to the existing codebase |
| **Artifact** | Entire GUI module structure (signal acquisition / processing / calculation / display layers) |
| **Environment** | Development environment (PC or RPi), during the development period |
| **Response** | New features can be added and tested independently without major redesign of existing modules |
| **Response Measure** | - Files changed when adding 1 new graph: ___ or fewer<br>- Structure allows addition without modifying existing modules<br>*(Upper file count limit requires team consensus)* |

### Domain Background

There are 11 graphs to implement plus Enhanced features. Within the A·C event → calculation → display pipeline, **only the display layer should need to be added independently**.

```
[A·C detection] → [Rate·BE·Amp calculation] → [display layer]
                                                   ├── Trace Display
                                                   ├── Rate Stability
                                                   ├── Beat Error View
                                                   ├── Scope 1 & 2
                                                   └── (new graph)  ← only this should be touched
```

A structure where only the display layer can be added without touching calculation logic = Extensibility achieved.

---

## Items Requiring Team Consensus

| QA | Undecided value | When to finalize |
|----|----------------|-----------------|
| Low Latency | Target ms for each of the 3 segments | After Experiment 1·2 results (Week 2) |
| Correctness | Acceptable deviation range | Team consensus |
| Measurement Accuracy | Error vs. WeiShi (s/d, ms) | After Experiment 3 results (Week 2) |
| Extensibility | Upper limit on changed files | Team consensus |
