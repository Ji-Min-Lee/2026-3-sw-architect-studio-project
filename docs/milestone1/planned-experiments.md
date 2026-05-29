# Planned Experiments

**Milestone**: M1 | **Due**: 2026-06-09 | **Status**: [ ] Draft / [ ] Final

> Each experiment resolves a specific open question or risk. Results feed into M2 architecture views.

---

## Experiment Template

```
ID:           EX-XX
Title:        [Short descriptive title]
Question:     [What specific question does this answer?]
Risk Addressed: [TR-XX / OI-XX]
Approach:     [How will the experiment be run?]
Setup:        [Tools, hardware, test data needed]
Completion Criteria: [How will we know the experiment is done and the question answered?]
Expected Result: [What do we expect to find?]
Status:       [ ] Planned / [ ] In Progress / [ ] Complete
Results:      [Fill in at M2]
```

---

## EX-01: Sample Rate Performance on Raspberry Pi 5

| Field | Value |
|-------|-------|
| **Question** | What is the maximum sustained audio sample rate the RPi 5 can process while running the Qt GUI without dropping blocks? |
| **Risk Addressed** | TR-01 (RPi cannot sustain 96k sps) |
| **Approach** | Run TimeGrapher in Sim mode at 48k, 96k, 192k sps. Measure: FPS, dropped audio blocks, CPU %, memory usage. |
| **Setup** | Raspberry Pi 5 + 8" touchscreen, TimeGrapher_v10.5, AlsaMixer AGC disabled, Sim mode |
| **Completion Criteria** | Data collected at all three sample rates; clear recommendation for target sps |
| **Expected Result** | 96k sps achievable; 192k sps may require optimization |
| **Status** | [ ] Planned |
| **Results** | *(to be filled at M2)* |

---

## EX-02: Beat Event Detection Accuracy

| Field | Value |
|-------|-------|
| **Question** | Which T1 detection reference point (onset vs peak) produces the most stable and accurate rate/beat-error measurements? |
| **Risk Addressed** | TR-02 (beat detection inaccurate) |
| **Approach** | Use a known-rate watch. Record signal. Detect T1 using (a) onset, (b) peak methods. Compare rate and beat error against WeiShi 1000 reference. |
| **Setup** | Mechanical watch, WeiShi No.1000 reference, recorded PCM signal (Playback mode), analysis script |
| **Completion Criteria** | Quantitative comparison of onset vs peak detection accuracy (mean error, std dev vs reference) |
| **Expected Result** | One approach measurably more stable; informed choice for architecture |
| **Status** | [ ] Planned |
| **Results** | *(to be filled at M2)* |

---

## EX-03: Filter Parameter Sweep

| Field | Value |
|-------|-------|
| **Question** | What low-pass and high-pass cutoff values best preserve watch beat events while rejecting ambient noise? |
| **Risk Addressed** | TR-05 (ambient noise causes erratic measurements) |
| **Approach** | Apply LP/HP filters at different cutoff combinations to recorded watch signal + noise. Evaluate T1/T3 visibility and detection SNR. |
| **Setup** | Recorded watch PCM with and without ambient noise, parameter sweep script, visualization |
| **Completion Criteria** | Recommended LP/HP default values with justification; documented effect on detection quality |
| **Expected Result** | Optimal range identified (e.g., HP ~200Hz, LP ~8000Hz) |
| **Status** | [ ] Planned |
| **Results** | *(to be filled at M2)* |

---

## EX-04: Cross-Compilation & RPi Deploy

| Field | Value |
|-------|-------|
| **Question** | Can the Qt project be built on macOS/Windows and deployed to RPi, or must we build natively on RPi? |
| **Risk Addressed** | TR-03 (cross-compilation fails) |
| **Approach** | Attempt Qt cross-compile with RPi sysroot on macOS. If failed, verify native build on RPi and assess build time. |
| **Setup** | macOS dev machine, Qt Creator, RPi 5, SSH access |
| **Completion Criteria** | Working binary running on RPi 5 with confirmed build path documented |
| **Expected Result** | Native build on RPi is simpler; cross-compile may be possible but complex |
| **Status** | [ ] Planned |
| **Results** | *(to be filled at M2)* |

---

## EX-05: Qt Multi-Tab Rendering Performance

| Field | Value |
|-------|-------|
| **Question** | Can Qt sustain target rendering FPS with 3+ active graph tabs simultaneously on RPi? |
| **Risk Addressed** | TR-04 (Qt FPS degrades with multiple tabs) |
| **Approach** | Implement 2-3 stub graph tabs (placeholder paint). Measure rendering FPS and CPU usage. Test with active signal processing running in background thread. |
| **Setup** | RPi 5, Qt prototype with multiple QWidget tabs, FPS counter |
| **Completion Criteria** | FPS data at 1, 2, 3, 4 active tabs; go/no-go on architecture decision (separate render timers, lazy rendering) |
| **Expected Result** | FPS acceptable with off-UI-thread signal processing; will inform threading model |
| **Status** | [ ] Planned |
| **Results** | *(to be filled at M2)* |

---

## Experiment Status Summary

| ID | Title | Status | Resolves | M2 Result Available |
|----|-------|--------|----------|---------------------|
| EX-01 | Sample Rate Performance on RPi 5 | Planned | TR-01 | [ ] |
| EX-02 | Beat Event Detection Accuracy | Planned | TR-02 | [ ] |
| EX-03 | Filter Parameter Sweep | Planned | TR-05 | [ ] |
| EX-04 | Cross-Compilation & RPi Deploy | Planned | TR-03 | [ ] |
| EX-05 | Qt Multi-Tab Rendering Performance | Planned | TR-04 | [ ] |

---

## Review Checklist

- [ ] Experiments are specific and follow the template
- [ ] Each experiment clearly addresses a question/risk (linked to risk assessment)
- [ ] Completion criteria clearly defined for each experiment
- [ ] Experiments are feasible within M1→M2 timeline
