# Planned Technical Experiments — TimeGrapher

**Team**: Blue Sky (Team 3) | **Milestone**: M1 | **Date**: 2026-06-07

---

## 1. Experiment Overview

| ID | Experiment Name | Linked QA | Linked Risk | Timing (Reference) |
|----|----------------|:---------:|:-----------:|:------------------|
| **EXP-01** | RPi Real-Time Performance — Dropped Block Measurement | QAS-1 | TR-01, TR-02 | Sprint 1 (Week 1) |
| **EXP-02** | End-to-End Latency — 3-Segment Timestamp Measurement | QAS-2 | TR-03, TR-04 | Sprint 2–3 (Week 1–2) |
| **EXP-03** | Detector Parameter Optimization Under Noise Conditions | QAS-3 | TR-05 | Sprint 3–4 (Week 2) |
| **EXP-04** | Signal Quality Warning Threshold Search | QAS-4 | TR-09 | Sprint 3–4 (Week 2) |
| **EXP-05** | BPH Escalation Verification — 36k/43k BPH Latency Measurement | QAS-2 (Stretch) | — | After all 28,800 BPH QA targets confirmed (Week 3+) |

> **Note**: Timing references the draft project plan and is not final. Experiments with prerequisites (EXP-02, EXP-03, EXP-04) proceed after EXP-01 results are confirmed and Observer pattern refactoring is complete. **EXP-05** is conducted only after EXP-02 confirms that all 28,800 BPH QA targets are met.

> **Observer pattern refactoring ≤ 3-file verification** is managed as an implementation checklist item and is not classified as a separate experiment.

---

## Experiment EXP-01: RPi Real-Time Performance — Dropped Block Measurement

---

### Results and recommendations

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective

Verify that zero Dropped Blocks (Ring Buffer overflow) occur on Raspberry Pi 5 when running Qt GUI + DSP pipeline concurrently at 48,000 / 96,000 / 192,000 sps.

**Technical question**: "Can RPi 5 achieve Dropped Block = 0 at 96,000 sps while running Qt GUI + DSP concurrently? If not, what is the maximum sps that can be processed stably?"

The results of this experiment directly inform:
- Finalizing QAS-1 Response Measure (current Objective: 96,000 sps is provisional)
- Confirming the Graceful Degradation fallback threshold (whether 48k sps fallback is triggered)
- Prerequisites for EXP-02 and EXP-03 — latency measurement and parameter tuning are only meaningful after SPS direction is confirmed

---

### Status

**Planned**

---

### Expected outcomes

- Table of Dropped Block counts for each condition (48k / 96k / 192k sps, continuous run)
- Conclusion on maximum achievable sps and finalized QAS-1 Response Measure
- Graceful Degradation fallback decision (whether 96k is achievable → whether 48k fallback is needed)
- Before/after comparison of Priority Scheduling (`SCHED_RR` / `SCHED_FIFO`) application

---

### Resources required

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5 (16GB RAM), USB audio sensor (microphone), 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, ALSA, `TimeGrapher_v10.5` codebase |
| Mechanical watch | One 28,800 BPH watch (live capture audio source) |
| Measurement tools | Ring Buffer overflow counter injection, `chrono` timestamps, Linux `perf` |
| Effort | ~1–2 person-days (setup + measurement + analysis) |

---

### Experiment description

1. **Environment setup**: Configure Ubuntu 24.04 + Qt6 build environment on RPi 5. Position 28,800 BPH watch in front of USB microphone.
2. **Inject Ring Buffer overflow counter**: Add `dropped_block_count` variable to `AudioCapture` module; increment on overflow event.
3. **Configure sps conditions**: Use `QAudioFormat::setSampleRate()` to set 48,000 / 96,000 / 192,000 sps in sequence.
4. **Continuous run**: For each sps condition, maintain Qt GUI + DSP (filtering + T1/T3 detection + Rate·Amplitude·Beat Error computation) running concurrently until Dropped Block counter stabilizes.
5. **Priority Scheduling comparison**: Repeat same conditions before and after applying `SCHED_RR` to measure Linux scheduler jitter effect (verifies TR-02).
6. **Record results**: Tabulate Dropped Block counts per sps. Confirm whether 96k is achievable → record finalized QAS-1 Response Measure.
7. **Fallback decision**: If 96k is not achievable, report Graceful Degradation fallback (automatic switch to 48k sps) decision to Architecture Committee.

---

### Duration

- **Target completion**: Within Sprint 1 (reference: 2026-06-09)
- **Prerequisites**: RPi 5 hardware setup + mechanical watch connected
- **Work blocked until this completes**: EXP-02 (latency measurement), EXP-03 (parameter tuning) — measurement conditions for both are undefined until SPS direction is decided

---

## Experiment EXP-02: End-to-End Latency — 3-Segment Timestamp Measurement

---

### Results and recommendations

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective

Measure the end-to-end latency of the full TimeGrapher pipeline (audio capture → DSP → Qt GUI rendering) by breaking it into 3 segments. Identify the bottleneck in each segment and verify whether end-to-end < 100 ms at 28,800 BPH is achievable.

**Technical questions this experiment must answer**:
- "What is the actual QAudioSource live capture callback period?"
- "Does ② process→display exceed 30 ms when 11 tabs render simultaneously?"
- "After achieving 28,800 BPH, can the target be raised to 36,000 / 43,200 BPH?"

The results of this experiment directly inform:
- Finalizing QAS-2 Response Measure (BPH-based latency targets)
- Decision on whether Lazy Rendering tactic is mandatory
- Preliminary data for BPH escalation feasibility

---

### Status

**Planned**

---

### Expected outcomes

- Latency measurement table: 3 segments × 3 sps tiers × 2 tab configurations (1-tab / 11-tab), mean + worst-case
- QAudioSource callback period measured value
- Lazy Rendering necessity decision (whether ② segment exceeds 30 ms at 11 tabs)
- Figures for team report on BPH escalation feasibility
- Finalized QAS-2 Response Measure

---

### Resources required

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5 (16GB RAM), USB audio sensor, 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` codebase |
| Mechanical watch | 28,800 BPH watch (primary target); additional BPH watches (if BPH escalation is explored) |
| Measurement tools | `std::chrono::high_resolution_clock` timestamps at 3 points (TS1/TS2/TS3), Qt `QElapsedTimer` |
| Tab configuration | 1-tab build vs. 11-tab complete build — two binaries or runtime tab count control |
| Prerequisites | EXP-01 complete (SPS direction decided), Observer pattern refactoring complete |
| Effort | ~2–3 person-days |

---

### Experiment description

**Timestamp injection points (3 points)**:

| Point | Location | Measured segment |
|-------|----------|-----------------|
| **TS1** | Immediately after ALSA callback received (entry of `audioDataAvailable()`) | ① start |
| **TS2** | Immediately after T1/T3 event timestamp is finalized (DSP processing complete) | ① end / ② start |
| **TS3** | Immediately after Qt `paintEvent()` completes (GUI rendering done) | ② end = ③ end |

> **Wait / Execute time mapping**: The interval before TS1 (beat occurrence → callback fires) is **Wait time** (OS callback period ~20ms). TS2−TS1 is **Execute time ①** (DSP processing); TS3−TS2 is **Execute time ②** (Qt rendering). Wait time is measured indirectly via consecutive TS1 interval histogram (TS1[n+1]−TS1[n]).

**Experiment procedure**:

1. **Inject timestamp code**: Insert `std::chrono::high_resolution_clock::now()` at TS1·TS2·TS3 positions. Log all three values per beat event to a log file.
2. **1-tab configuration measurement**: Build with only one tab active; run continuously at each sps tier (48k/96k/192k) × 28,800 BPH watch. Collect mean and worst-case per segment.
3. **11-tab configuration measurement**: Repeat identical conditions with all 11 tabs active. Compare ② segment (process→display) against 1-tab baseline.
4. **Callback period measurement**: Plot histogram of TS1 intervals to characterize actual QAudioSource callback period and jitter distribution.
5. **Lazy Rendering decision**: Calculate frequency of ② segment > 30 ms at 11 tabs. If threshold exceeded, trigger Lazy Rendering tactic application decision.
6. **BPH escalation review**: If 28,800 BPH end-to-end < 100 ms is achieved, repeat identical measurement with 36,000 / 43,200 BPH watches and report feasibility to team.
7. **Finalize results**: Populate QAS-2 BPH-based latency target table with measured values; confirm Response Measure.

---

### Duration

- **Target completion**: Within Sprint 2–3 (reference: ~2026-06-12)
- **Prerequisites**: ① EXP-01 complete (SPS direction confirmed) ② Observer pattern refactoring complete (11-tab build needed for tab comparison)
- **Completion criteria**: 3-segment × 2-tab-configuration measurement table complete + QAS-2 Response Measure finalized

---

## Experiment EXP-03: Detector Parameter Optimization Under Noise Conditions

---

### Results and recommendations

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective

Systematically search combinations of `onset_fraction` and `min_peak_fraction` parameters in `Detector.cpp` across three noise conditions (low / medium / high ambient noise) to identify the optimal combination that minimizes Δ Rate / Δ Amplitude / Δ Beat Error.

**Technical question**: "Across low / medium / high noise environments, which combination of `onset_fraction` and `min_peak_fraction` minimizes measurement deviation (Δ) for Rate / Amplitude / Beat Error?"

The results of this experiment directly inform:
- Finalizing QAS-3 QA-C2 Response Measure (currently provisional, noted as "confirmed after EXP-03")
- Updating default parameters in `Detector.cpp` (resolves TR-05)
- Validating adaptive threshold algorithm effectiveness under real-world noise conditions

---

### Status

**Planned**

---

### Expected outcomes

- Grid search result table: `onset_fraction` × `min_peak_fraction` combinations × 3 noise conditions, with Δ Rate / Δ Amplitude / Δ Beat Error per cell
- Optimal parameter combination per noise condition + recommended default values
- Validity assessment of adaptive threshold (noise floor = 75th percentile of last 256 ms, reference_peak = median of last 16 beats) under each noise condition
- Finalized QAS-3 QA-C2 Response Measure (including acceptable Δ thresholds)

---

### Resources required

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5, USB audio sensor, 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` codebase |
| Mechanical watch | One 28,800 BPH watch (reference watch for parameter tuning) |
| Noise environment | Three ambient noise levels (e.g., quiet lab / office / added noise source) |
| Measurement tools | Rate·Amplitude·Beat Error log across N repetitions per condition; reference values from Witschi or equivalent |
| Prerequisites | EXP-01 complete (SPS to be used for measurement confirmed) |
| Effort | ~2–3 person-days |

---

### Experiment description

**Parameter search range (grid search)**:

| Parameter | Current default | Search range | Step |
|-----------|:--------------:|-------------|:----:|
| `onset_fraction` | 0.03 | 0.01 ~ 0.10 | 0.01 |
| `min_peak_fraction` | 0.20 | 0.10 ~ 0.40 | 0.05 |

**Noise condition definitions**:

| Condition | Environment description | Expected noise floor |
|-----------|------------------------|:-------------------:|
| Low noise | Quiet closed lab, surrounding equipment off | ~30 dB SPL |
| Medium noise | Typical office, HVAC and conversation noise | ~50 dB SPL |
| High noise | Added noise source (e.g., music playback) | ~65 dB SPL |

**Experiment procedure**:

1. **Collect baseline**: Measure Rate·Amplitude·Beat Error 30 times in low-noise environment with default parameters (`onset_fraction=0.03`, `min_peak_fraction=0.20`) to establish baseline.
2. **Fix noise condition order**: Low → Medium → High. Allow 2-minute stabilization after each noise condition change.
3. **Grid search**: For each noise condition, apply all `onset_fraction` × `min_peak_fraction` combinations sequentially. Measure Rate·Amplitude·Beat Error 10 times per combination.
4. **Compute Δ**: For each combination, calculate Δ Rate / Δ Amplitude / Δ Beat Error against the baseline (low noise + default parameters).
5. **Select optimal combination**: Choose the parameter combination with the minimum sum of Δ across all three noise conditions.
6. **Validate adaptive threshold**: With adaptive threshold active, verify that the optimal parameter combination also holds under abrupt noise transitions (low→high switch).
7. **Record results**: Reflect optimal parameter combination + QAS-3 QA-C2 acceptable Δ values into Architectural Drivers.

---

### Duration

- **Target completion**: Within Sprint 3–4 (reference: ~2026-06-18)
- **Prerequisites**: EXP-01 complete (SPS for measurement confirmed)
- **Completion criteria**: 3-condition × full parameter combination measurement table complete + optimal parameters confirmed + QAS-3 QA-C2 Response Measure recorded

---

## Experiment EXP-04: Signal Quality Warning Threshold Search

---

### Results and recommendations

(Not yet concluded — to be recorded after experiment execution.)

---

### Objective

Measure the optimal onset/clear thresholds (N seconds, M seconds, noise/signal ratio) for `⚠ No signal` / `⚠ Noisy signal` warnings in real environments to minimize false alarms and missed warnings.

**Technical questions**:
- "After removing the watch from the microphone, within how many seconds should `⚠ No signal` appear for the user to notice immediately?"
- "After restoring the watch, within how many seconds should the warning clear to confirm stable signal recovery?"
- "What noise/signal ratio threshold triggers `⚠ Noisy signal` without false alarms in real environments?"

The results of this experiment directly inform:
- Finalizing QAS-4 Response Measure (N·M values currently unresolved)
- Setting Heartbeat pattern parameter (N-second timeout value)
- Hardening `⚠ Noisy signal` threshold as a code constant

---

### Status

**Planned**

---

### Expected outcomes

- Distribution of time from watch removal to `⚠ No signal` warning appearance (measured N seconds)
- Distribution of time from watch restore to warning clearance (measured M seconds)
- `⚠ Noisy signal` threshold candidates and false-alarm rates across multiple noise environments (low/medium/high)
- Finalized QAS-4 Response Measure (N·M values + noise/signal threshold)
- Recommended Heartbeat parameter value

---

### Resources required

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5, USB audio sensor, 8" touchscreen |
| Software | Ubuntu 24.04, Qt6, `TimeGrapher_v10.5` codebase with warning display implementation complete |
| Mechanical watch | One 28,800 BPH watch (subject of removal/restore operations) |
| Noise environment | Same as EXP-03 — three ambient noise levels (low/medium/high) |
| Measurement tools | Warning display timestamp log; configurable Heartbeat N-second parameter for sweep |
| Prerequisites | Observer pattern refactoring complete + `⚠ No signal` / `⚠ Noisy signal` warning UI implemented |
| Effort | ~1–2 person-days |

---

### Experiment description

**Part A — `⚠ No signal` N·M value search**:

1. Prepare builds with Heartbeat N parameter set to 1 / 2 / 3 / 5 seconds.
2. For each N setting, remove the 28,800 BPH watch from the microphone and measure time until warning appears (10 repetitions).
3. Restore the watch and measure time M until warning clears (10 repetitions).
4. Decide optimal N·M balancing "too slow (delayed user awareness)" vs. "too fast (false alarms from signal fluctuation)."

**Part B — `⚠ Noisy signal` threshold search**:

1. Collect real-time noise/signal ratio logs under 3 noise conditions.
2. Plot ratio distribution histogram per condition; check gap between 99th percentile of low-noise normal range and 1st percentile of high-noise range.
3. Apply 3–5 candidate thresholds and measure false-alarm rate and miss rate per condition.
4. Select final threshold with minimum weighted sum of false-alarm rate + miss rate.
5. Record N·M values + noise/signal threshold in QAS-4 Response Measure.

---

### Duration

- **Target completion**: Within Sprint 3–4 (reference: ~2026-06-18)
- **Prerequisites**: Observer pattern refactoring complete + `⚠ No signal` / `⚠ Noisy signal` warning UI implemented
- **Completion criteria**: N·M values confirmed + noise/signal threshold confirmed + QAS-4 Response Measure populated with confirmed values

---

## Experiment EXP-05: BPH Escalation Verification — 36k/43k BPH Latency Measurement

---

### Results and recommendations

(Not yet conducted — to be performed after all 28,800 BPH QA targets are confirmed.)

---

### Objective

Assuming all 28,800 BPH QA targets (QAS-1~4) are confirmed, verify that end-to-end latency completes within 80% of each beat period for 36,000 / 43,200 BPH mechanical watches. Confirms whether the team's 2nd goal (wider BPH coverage) is achievable.

**Technical question**: "Under the same 3-segment measurement conditions as EXP-02, can end-to-end latency targets be met for 36,000 / 43,200 BPH watches?"

Results inform:
- Confirming whether QAS-2 Stretch target is achieved
- Decision to declare or abandon the team's 2nd goal (BPH range expansion)

---

### Status

**Deferred** — Blocked until all 28,800 BPH QA targets are confirmed

---

### Expected outcomes

- Latency measurement table: 36,000 / 43,200 BPH watches × 3 segments (①②③) × confirmed sps
- End-to-end latency as a fraction of beat period (whether < 80% is met)
- QAS-2 Stretch target pass/fail judgment + team 2nd goal declaration decision

---

### Resources required

| Resource | Detail |
|----------|--------|
| Hardware | Raspberry Pi 5 (16GB RAM), USB audio sensor, 8" touchscreen |
| Software | EXP-02-complete build (reuse timestamp injection code) |
| Mechanical watches | 36,000 BPH watch, 43,200 BPH watch |
| Prerequisites | EXP-02 complete + QAS-1~4 all confirmed at 28,800 BPH |
| Effort | ~1 person-day (reuses EXP-02 setup) |

---

### Duration

- **Target completion**: As soon as possible after 28,800 BPH QA confirmation (Week 3+)
- **Prerequisites**: EXP-02 complete + all QAS-1~4 passed at 28,800 BPH
- **Completion criteria**: 36k/43k BPH measurement table complete + QAS-2 Stretch confirmed
