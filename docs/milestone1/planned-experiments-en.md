# Planned Experiments

**Team**: Blue Sky (Team 3) | **Milestone**: M1 | **Due**: 2026-06-09

---

## Summary

Each experiment resolves a specific open issue (OI) or technical risk (TR) and confirms QA target values.

| ID | Experiment | Resolves | Validates QA | Prerequisite |
|----|-----------|---------|:------------:|:------------:|
| EX-01 | RPi Performance Benchmark | OI-02, OI-03, TR-02, TR-03, TR-04, TR-07 | QAS-1, QAS-3 | — |
| EX-02 | Simultaneous WeiShi & RPi Measurement Setup | OI-09, TR-08 | QAS-2 | — |
| EX-03 | `tg_c_placement_t` Placement Setting Comparison | OI-01, OI-02, TR-01, TR-02 | QAS-2 | EX-02 |

> **Recommended Order**: EX-01 & EX-02 (parallel) → EX-03

---

## EX-01: RPi Performance Benchmark

### Results and Recommendations

*(To be recorded at M2)*

---

### Objective

What is the maximum sps the RPi 5 can sustain without dropping audio blocks while running the Qt GUI? What are the three latency segments of QAS-3?

This experiment provides data for:
- QAS-1: Confirm target sps (sustain 96k or fall back to 48k)
- QAS-3: Finalize all three latency segments
  - ① capture→process: audio callback start → beat event timestamp generated (provisional < 70 ms)
  - ② process→display: beat event timestamp → GUI screen update complete (provisional < 30 ms)
  - ③ end-to-end: ① + ② total (provisional < 100 ms)

---

### Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### Expected Outcomes

- Table of CPU usage, dropped block count, and GUI FPS at 48k / 96k / 192k sps
- QAS-3 latency measurements: ① capture→process, ② process→display, ③ end-to-end
- Target sps decision (sustain 96k / fall back to 48k) with rationale
- Finalized QAS-1 / QAS-3 values (replacing provisional figures)

---

### Resources Required

| Item | Detail |
|------|--------|
| Hardware | RPi 5 (16GB), 8" touchscreen, USB audio sensor |
| Software | TimeGrapher_v10.5 Sim mode, AlsaMixer (AGC off), `htop`, `QElapsedTimer` |
| Effort | 0.5 person-day |

---

### Experiment Procedure

1. Disable AGC and verify persistence (OI-02)
   - 1-a. Disable AGC in AlsaMixer
   - 1-b. Reboot RPi and re-enter AlsaMixer to verify AGC state — if still off, proceed; if restored to on, apply permanent disable method and retry
   - 1-c. Confirm AGC off, then launch TimeGrapher in Sim mode
2. Run at 48k → 96k → 192k sps, 5 minutes each
3. At each sps, measure:
   - CPU usage (`htop`), dropped blocks, GUI FPS
   - ① capture→process: audio callback start → beat event timestamp generated (`QElapsedTimer`)
   - ② process→display: beat event timestamp → GUI `paintEvent` complete (`QElapsedTimer`)
   - ③ end-to-end: ① + ② combined
4. Enable FFT spectrogram tab and repeat (TR-07)
5. Compile results table and decide on 48k fallback

---

### Completion Criteria

- Measurement data collected at all three sps including FFT-active condition
- Team agreement on target sps
- QAS-1 / QAS-3 provisional values replaced with empirical data

---

### Duration

Target completion: **2026-06-07**

---

### Links and References

- [Risk Assessment — OI-03, TR-03, TR-04, TR-07](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-1, QAS-3](./architectural-drivers.md)

---

## EX-02: Simultaneous WeiShi & RPi Measurement Setup

### Results and Recommendations

*(To be recorded at M2)*

---

### Objective

Is it feasible to simultaneously collect acoustic signals from the same watch using both WeiShi No.1000 and RPi?

QAS-2 uses WeiShi No.1000 as the reference. Without the same input signal, watch movement variation makes comparison meaningless (OI-09, TR-08). This experiment confirms the measurement method for EX-03.

---

### Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### Expected Outcomes

- Go/no-go verdict on simultaneous measurement (including mic splitter / sequential fallback)
- Confirmed measurement method and setup procedure
- If sequential: estimated error margin from watch drift

---

### Resources Required

| Item | Detail |
|------|--------|
| Hardware | WeiShi No.1000, RPi 5, USB microphone, (candidate) mic splitter |
| Software | TimeGrapher_v10.5 |
| Effort | 0.5 person-day |

---

### Experiment Procedure

1. Attempt simultaneous signal feed to WeiShi No.1000 and RPi mic via mic splitter
2. If feasible: compare measurements on same signal, verify repeatability (3 trials)
3. If not feasible: perform sequential measurement — switch from WeiShi No.1000 to RPi within **1 minute**, repeat 3 times, and estimate drift error margin from measurement variance (std dev)
4. Document confirmed method → hand off to EX-03

---

### Completion Criteria

- One measurement method (simultaneous or sequential) decided and documented
- Repeatability confirmed over 3 trials with the confirmed method

---

### Duration

Target completion: **2026-06-07** *(before EX-03)*

---

### Links and References

- [Risk Assessment — OI-09, TR-08](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-2](./architectural-drivers.md)

---

## EX-03: `tg_c_placement_t` Placement Setting Comparison

### Results and Recommendations

*(To be recorded at M2)*

---

### Objective

Which setting — `TG_C_PLACEMENT_PEAK` (C event at maximum amplitude) or `TG_C_PLACEMENT_ONSET` (half-height crossing found by backward walk from peak) — minimizes Rate and Amplitude error vs WeiShi No.1000?

Onset/peak detection is already implemented in `Detector.cpp` / `Timegrapher.h`. Since `tg_c_placement_t` switches the primary timing reference, this experiment selects the **optimal setting**. Beat Error is computed solely from A events and is unaffected by placement. Results confirm QAS-2 Rate threshold (< 5 s/d) and establish the Amplitude error margin.

> **Prerequisite**: Run after EX-02 is concluded.

---

### Status

`[ ] Planned` | `[ ] In Progress` | `[ ] Concluded`

---

### Expected Outcomes

- Comparison table of Rate / Amplitude error (mean, std dev) for PEAK vs ONSET setting across 2+ watch models
- Confirmed optimal `tg_c_placement_t` setting
- Finalized QAS-2 Rate threshold and established Amplitude error margin (replacing provisional values)

---

### Resources Required

| Item | Detail |
|------|--------|
| Hardware | Mechanical watches (2+ models), WeiShi No.1000, USB microphone |
| Software | TimeGrapher_v10.5 Playback mode (`tg_c_placement_t` parameter switch) |
| Code | `src/Detector.cpp`, `src/Detector.h`, `src/Timegrapher.h` |
| Effort | 1 person-day |

---

### Experiment Procedure

> Use the measurement method confirmed in EX-02.

1. Record reference Rate and Beat Error from WeiShi No.1000
2. Record PCM audio of the same watch via USB mic (30+ seconds)
3. Run Playback mode with (a) `TG_C_PLACEMENT_PEAK`, (b) `TG_C_PLACEMENT_ONSET`
4. Compare Rate and Amplitude for each setting against WeiShi reference (Beat Error excluded — computed from A events only, unaffected by placement)
5. Compute error statistics (mean, std dev); confirm lower-error setting
6. Repeat with 2+ watch models to verify generalizability

---

### Completion Criteria

- Rate / Amplitude error data collected for both settings across 2+ watch models
- Optimal `tg_c_placement_t` setting confirmed by team
- QAS-2 Rate provisional value and Amplitude error margin replaced with empirical data

---

### Duration

Target completion: **2026-06-08** *(after EX-02 concluded)*

---

### Links and References

- [Risk Assessment — OI-01, OI-02, TR-01, TR-02](./risk-assessment-revision.md)
- [Architectural Drivers — QAS-2](./architectural-drivers.md)
