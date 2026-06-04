# Risk Assessment

**Team**: Blue Sky (Team 3) | **Milestone**: M1 | **Due**: 2026-06-09

> **Prob**: H / M / L | **Impact**: H / M / L

---

## 1. Technical and Non-Technical Risks

Each risk is assessed on a High/Medium/Low scale for both Probability and Impact.

### Technical Risks

| ID | Risk | Prob | Impact | Related Issue |
|----|------|:----:|:------:|:-------------:|
| TR-01 | C-event placement not optimized — which of `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` minimizes error vs WeiShi 1000 is unverified | H | H | OI-01 |
| TR-02 | AGC re-enabled after reboot corrupts signal — measurement values become unreliable | M | H | OI-02 |
| TR-03 | RPi throughput at 96k sps with Qt GUI simultaneously unverified | H | H | OI-03 |
| TR-04 | Qt FPS drops with 11 active graph tabs rendering simultaneously | M | H | OI-03 |
| TR-05 | `MainWindow.cpp` God Object — constant merge conflicts when 3 developers modify simultaneously | H | H | OI-04 |
| TR-06 | No macOS build branch — audio broken for 2 macOS developers | H | M | OI-05 |
| TR-07 | FFT spectrogram CPU overload on RPi | M | M | OI-03 |
| TR-08 | Simultaneous WeiShi 1000 & RPi measurement setup uncertain — same input required for accuracy comparison; both devices must capture the same watch signal simultaneously | H | H | OI-09 |

### Non-Technical Risks

| ID | Risk | Prob | Impact | Related Issue |
|----|------|:----:|:------:|:-------------:|
| NR-01 | Coding/architecture team boundary unclear — design decisions not reflected in implementation | H | H | OI-06 |
| NR-02 | Scope overextension in 5-week schedule — implementing all 11 graphs risks degrading core feature quality | H | M | OI-07 |
| NR-03 | English communication overhead — risk of design decisions not reaching all team members | M | H | OI-08 |

---

## 2. Open Issues

Each open issue directly affects final demo outcome.

| ID | Open Issue | Related Risk | If Unresolved |
|----|-----------|:------------:|---------------|
| OI-01 | C-event placement undecided — onset/peak detection implemented (`tg_c_placement_t`) but optimal setting vs WeiShi 1000 unverified | TR-01 | Measurement values unreliable |
| OI-02 | AGC-off persistence after RPi reboot unverified | TR-02 | All measurements unreliable |
| OI-03 | RPi performance under load not measured (sps, FPS, FFT) | TR-03, TR-04, TR-07 | Real-time processing fails at demo — audio and rendering failure |
| OI-04 | `MainWindow.cpp` refactoring scope not confirmed | TR-05 | Every new graph triggers whole-file merge conflicts |
| OI-05 | macOS build strategy undecided | TR-06 | 2 macOS developers cannot test audio locally |
| OI-06 | Coding/architecture team sync process not defined | NR-01 | Design decisions not implemented — rework risk before M2 |
| OI-07 | Graph priority not classified | NR-02 | Core features incomplete at demo |
| OI-08 | Language mismatch — no standard for deliverable writing (professor is English-speaking; presentations and milestone submissions must be in English) | NR-03 | English-speaking member misses context + milestone quality degraded |
| OI-09 | Simultaneous WeiShi 1000 & RPi measurement setup not verified | TR-08 | Cannot compare accuracy on same input — ±5 s/d target unverifiable |

---

## 3. Actions and Planned Experiments

Technical risks with uncertainty are resolved through Planned Experiments; others are addressed by immediate actions.

| Issue | Action | Done When |
|:-----:|--------|-----------|
| OI-02 | Integrated into EX-01 pre-step — re-check AGC state in AlsaMixer after RPi reboot; if restored to on, apply permanent disable method (`alsactl store`, etc.) | AGC-off persistence confirmed after reboot |
| OI-01 | **EX-03**: Compare error by `tg_c_placement_t` setting — measure same watch with PEAK vs ONSET placement, compare Rate/Amplitude vs WeiShi 1000 (Beat Error excluded — computed from A events only, unaffected by placement; detection already implemented; goal is selecting optimal setting) | Placement setting decided + Rate/Amplitude error margin measured |
| OI-03 | **EX-01**: Measure processing time and FPS at 96k/48k sps with Qt GUI on RPi | Time and FPS measured; 48k fallback decision made |
| OI-04 | Confirm `AudioCapture` / `MeasurementEngine` split scope and document 4-layer module boundaries | Ownership and split scope confirmed in Project Plan; 4-layer boundaries documented |
| OI-05 | Add `Q_OS_MAC` branch and implement CoreAudio / PortAudio integration for real audio on macOS | Build succeeds on macOS and audio confirmed working (including Sim mode) |
| OI-06 | Fix daily afternoon sync meeting + communicate via Teams channel | Role boundaries confirmed and documented in Project Plan |
| OI-07 | Classify 11 graphs as Core / Required / Stretch | Graph priorities classified as Core / Required / Stretch and reflected in Project Plan |
| OI-08 | Write all deliverables in bilingual (KO/EN); milestone submissions and presentations in English | Writing standard agreed + all M1 deliverables verified to comply with bilingual rule |
| OI-09 | **EX-02**: Experiment simultaneous measurement setup — evaluate mic splitter or sequential method | Setup feasibility confirmed + measurement method decided |
