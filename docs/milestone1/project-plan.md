# Project Plan

**Milestone**: M1 | **Due**: 2026-06-09 | **Status**: [ ] Draft / [ ] Final

---

## 1. Team & Role Assignments

| Name | Role | Responsibilities |
|------|------|-----------------|
|      |      |                 |
|      |      |                 |

---

## 2. Milestone Definitions

| Milestone | Due Date | Key Deliverables | Success Criteria |
|-----------|----------|-----------------|-----------------|
| M1 | 2026-06-09 | Project Plan, Arch Drivers, Risk, Experiments, Arch Approaches | Mentor review pass |
| M2 | 2026-06-22 | Experiment Results, Architecture Views, Construction Plan | All critical risks addressed |
| M3 | 2026-07-01 | Final Demo on RPi, Team Presentation | GUI runs on Raspberry Pi 5 |

---

## 3. Architecture-Based Task Breakdown

> Tasks should reflect the chosen architectural approaches, not just feature implementation.

| Task | Owner | Milestone | Depends On | Est. Effort |
|------|-------|-----------|------------|-------------|
| Setup Qt dev environment on macOS/Windows | | M1 | — | |
| Build & run baseline TimeGrapher_v10.5 | | M1 | Setup | |
| Cross-compile / deploy to Raspberry Pi 5 | | M1-M2 | Baseline build | |
| Define module decomposition | | M1 | Arch Drivers | |
| Signal acquisition layer refactor | | M2 | Module design | |
| Signal processing pipeline (filter chain) | | M2 | Signal acquisition | |
| Beat event detection (T1/T3) | | M2 | Signal processing | |
| Rate / Amplitude / Beat Error calculation | | M2 | Beat detection | |
| Implement Trace Display tab | | M2 | Calculation | |
| Implement Vario Display tab | | M2 | Calculation | |
| Implement Sequence Display tab | | M2-M3 | Calculation | |
| Implement Beat-Noise Scope (Scope 1 & 2) | | M2-M3 | Beat detection | |
| Implement Beat Error Display & Diagnostic Trace | | M2-M3 | Calculation | |
| Implement Long-Term Performance Graph | | M3 | Calculation | |
| Implement Escapement Analyzer & Marker-Line | | M3 | Beat detection | |
| Implement Time-Frequency Spectrogram | | M3 | Signal processing | |
| Implement Waveform Comparison Display | | M3 | Beat detection | |
| Implement Scope Mode (Synchronized Sweep) | | M3 | Signal acquisition | |
| Implement Scope Function (F0/F1/F2/F3) | | M3 | Signal processing | |
| AI Feature (on-device signal quality) | | M3 | Signal processing | |
| Latency measurement & reporting | | M2-M3 | Full pipeline | |
| Performance tuning for RPi (96k sps target) | | M3 | Full pipeline | |

---

## 4. Technical Experiment Plans

> Experiments to resolve architectural unknowns. See `planned-experiments.md` for detail.

| Experiment | Question Addressed | Target Milestone |
|------------|--------------------|-----------------|
| Sample rate benchmark on RPi 5 | Can RPi achieve 96k sps with real-time Qt GUI? | M1→M2 |
| Beat event detection accuracy | Which detection approach (onset vs peak) gives stable T1? | M1→M2 |
| Filter parameter sweep | Optimal low-pass/high-pass cutoffs for watch signal? | M1→M2 |
| Cross-compilation setup | Can we build Qt app on macOS targeting RPi ARM64? | M1 |
| Qt rendering FPS on RPi | Can Qt sustain target FPS with multiple graph tabs? | M2 |

---

## 5. Schedule Overview

```
Week 1 (05/27-05/30): Project setup, baseline code study, env setup
Week 2 (06/02-06/09): [M1 DUE] Arch drivers, risk, experiments, arch approaches
Week 3 (06/10-06/16): Experiments, module design, begin implementation
Week 4 (06/17-06/22): [M2 DUE] Experiment results, arch views, construction plan
Week 5 (06/23-07/01): [M3 DUE] Implementation complete, demo, presentation
```

---

## 6. Review Checklist

- [ ] Role assignments and tasks clearly defined
- [ ] Architecture-based implementation tasks reflected
- [ ] Technical experiment plans included
- [ ] Milestone dates and success criteria defined
