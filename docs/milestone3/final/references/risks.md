# Risk Register

**Team**: Blue Sky (Team 3) | **Milestone**: M3 | **Date**: 2026-06-25

---

## Technical Risks

| ID | Description | Prob | Impact | Experiment | Resolution | Status |
|----|-------------|:----:|:------:|------------|------------|:------:|
| TR-01 | RPi 5 cannot sustain 96kHz audio capture without block drops while Qt GUI runs | M | H | [EXP-01](experiments/exp-01-realtime-dropped-block.md) | 96kHz confirmed: Dropped=0 at all sps; SCHED_RR not required; ADR-003 Accepted | ✅ Resolved |
| TR-02 | Single-threaded pipeline saturates cpu2 (91%); 43% deadline miss on RPi | H | H | [EXP-02](experiments/exp-02-latency-e2e.md) | **T2 DSP Offload Thread** — wait_ms ×32,000 reduction, backlog 0% (macOS + RPi E2-5) | ✅ Resolved |
| TR-03 | Qt QueuedConnection accumulates 420ms backlog; frame queue grows unbounded | H | H | [EXP-02](experiments/exp-02-latency-e2e.md) | **T2** eliminates coupling between audio callback and Qt event loop (RPi E2-5/6 confirmed) | ✅ Resolved |
| TR-04 | `replot()` in exec path consumes 79% of exec budget (16ms / 21ms); scales with N tabs | H | H | [EXP-02](experiments/exp-02-latency-e2e.md) / [EXP-03](experiments/exp-03-extensibility-observer-pattern.md) | **R1 Lazy Rendering** — replot_count ↓75–85% (macOS); EXP-03: 14-tab load confirmed, exec avg 2.2ms / max 4.8ms (RPi E3-07) | ✅ Resolved |
| TR-05 | LP/HP filter defaults reject beat signal or pass ambient noise at edge BPH values | M | M | [EXP-04](experiments/exp-04-correctness-detector-optimization.md) | EXP-04 E5-03 (274 runs, 0–60 dB SNR): `onset_fraction=0.08`, `min_peak_fraction=0.10` confirmed; `Detector.cpp` defaults updated | ✅ Resolved |
| TR-06 | Layer refactoring introduces regression in existing DSP behavior | M | H | — | 4-layer structure enforced; 142 unit tests (10 binaries) all passing as of 2026-06-21 | ✅ Resolved |
| TR-07 | Residual cross-layer coupling survives refactoring | M | M | — | Allowed-to-use rule enforced; compiler catches upward dependency | ✅ Resolved |
| TR-08 | New graph tab requires data not in current MeasurementEngine output | L | M | [EXP-03](experiments/exp-03-extensibility-observer-pattern.md) | All 14 tabs implemented ≤ 3-file change each; zero Presentation→DSP layer violations; EXP-03 confirms Domain output covers all requirements | ✅ Resolved |
| TR-09 | Signal quality warning thresholds mismatched to actual watch signal | L | L | — | Heartbeat pattern implemented; threshold tunable via single parameter | ✅ Resolved |

---

## Non-Technical Risks

| ID | Description | Prob | Impact | Resolution | Status |
|----|-------------|:----:|:------:|------------|:------:|
| NTR-04 | English communication overhead slows documentation velocity | M | L | All deliverables in English. Internal discussion Korean; design summaries bilingual | ✅ Convention held |
| NTR-05 | Single RPi device creates experiment bottleneck | H | M | EXP-01 validated on each dev machine first; RPi-independent work ran in parallel; RPi slot reserved for target-hardware confirmation | ✅ Mitigated |
| NTR-07 | Equation-level derivations difficult (Rate / Beat Error / Amplitude formulas) | M | H | AI-assisted equation interpretation; [142 tests across 10 binaries](unit-test-results.md) verify correctness independently of individual formula fluency; EXP-06 WeiShi comparison confirmed within tolerance | ✅ Resolved |

---

## Open Risk Summary (as of M3)

> ✅ No open risks — all technical and non-technical risks resolved as of 2026-06-25.

---

## Risk Resolution History

| Date | ID | Event |
|------|----|-------|
| 2026-06-25 | NTR-07 | EXP-06 WeiShi comparison complete: Rate/Amplitude/Beat Error within tolerance; QAS-5 Pass |
| 2026-06-21 | TR-04/06/08 | EXP-03 complete: 14 tabs verified ≤ 3-file change each, zero layer violations; test suite expanded to 142 tests (10 binaries), all passing |
| 2026-06-17 | TR-05 | EXP-04 E5-03 complete (274 runs): `onset_fraction=0.08`, `min_peak_fraction=0.10` — stable beat detection at 0–60 dB SNR; `Detector.cpp` defaults updated |
| 2026-06-15 | TR-01 | EXP-01 RPi: Dropped=0 at 48k/96k/192k under all scheduling policies; ADR-003 Accepted |
| 2026-06-15 | TR-02/03 | EXP-02 RPi E2-5/6: T2+R1 on RPi — E2E avg 2.05ms, 0 deadline miss, 0 backlog |
| 2026-06-15 | TR-02/03 | EXP-02 macOS R2: T2 applied — wait_ms ×32,000 reduction, backlog 0% |
| 2026-06-15 | TR-04 | EXP-02 macOS R3/R4: R1 applied — replot ↓75–85% |
| 2026-06-13 | TR-06/07 | Layer refactoring complete; initial 116 unit tests passing |
| 2026-06-10 | TR-08 | Domain output review complete — all graph data requirements covered |
| 2026-06-10 | TR-09 | Adaptive threshold implementation complete |
