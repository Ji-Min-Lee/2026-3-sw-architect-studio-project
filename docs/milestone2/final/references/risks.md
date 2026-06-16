# Risk Register

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Technical Risks

| ID | Description | Prob | Impact | Experiment | Resolution | Status |
|----|-------------|:----:|:------:|------------|------------|:------:|
| TR-01 | RPi 5 cannot sustain 96kHz audio capture without block drops while Qt GUI runs | M | H | [EXP-01](experiments/exp-01-sample-rate.md) | Fallback to 48kHz if needed; ADR-003 pending result | ⏳ In Progress |
| TR-02 | Single-threaded pipeline saturates cpu2 (91%); 43% deadline miss on RPi | H | H | [EXP-02](experiments/exp-02-pipeline-latency.md) | **T2 DSP Offload Thread** — wait_ms ×32,000 reduction, backlog 0% (macOS) | ✅ macOS — RPi R5 06/23 |
| TR-03 | Qt QueuedConnection accumulates 420ms backlog; frame queue grows unbounded | H | H | [EXP-02](experiments/exp-02-pipeline-latency.md) | **T2** eliminates coupling between audio callback and Qt event loop | ✅ macOS — RPi R5 06/23 |
| TR-04 | `replot()` in exec path consumes 79% of exec budget (16ms / 21ms); scales with N tabs | H | H | [EXP-02](experiments/exp-02-pipeline-latency.md) / [EXP-05](experiments/exp-05-rendering-fps.md) | **R1 Lazy Rendering** — replot_count ↓75–85% (macOS); 11-tab load pending EXP-05 | ✅ macOS — RPi EXP-05 06/26 |
| TR-05 | LP/HP filter defaults reject beat signal or pass ambient noise at edge BPH values | M | M | [EXP-03](experiments/exp-03-filter-sweep.md) | Parameter sweep to determine optimal cutoffs; adaptive threshold fallback | ⏳ Scheduled 06/25 |
| TR-06 | Layer refactoring introduces regression in existing DSP behavior | M | H | — | 4-layer structure enforced; 116 unit tests (7 binaries) all passing | ✅ Resolved |
| TR-07 | Residual cross-layer coupling survives refactoring | M | M | — | Allowed-to-use rule enforced; compiler catches upward dependency | ✅ Resolved |
| TR-08 | New graph tab requires data not in current MeasurementEngine output | L | M | — | 11-graph data requirements reviewed; all covered by current Domain output | ✅ Resolved |
| TR-09 | Signal quality warning thresholds mismatched to actual watch signal | L | L | — | Heartbeat pattern implemented; threshold tunable via single parameter | ✅ Resolved |

---

## Non-Technical Risks

| ID | Description | Prob | Impact | Resolution | Status |
|----|-------------|:----:|:------:|------------|:------:|
| NTR-04 | English communication overhead slows documentation velocity | M | L | All deliverables in English. Internal discussion Korean; design summaries bilingual | ✅ Convention held |
| NTR-05 | Single RPi device creates experiment bottleneck | H | M | EXP-02 validated on each dev machine first; RPi-independent work ran in parallel; RPi slot reserved for target-hardware confirmation | ✅ Mitigated |
| NTR-07 | Equation-level derivations difficult (Rate / Beat Error / Amplitude formulas) | M | H | AI-assisted equation interpretation; 119 tests across 5 binaries verify correctness independently of individual formula fluency | ✅ Mitigated — test suite is safety net |

---

## Open Risk Summary (as of M2)

| Priority | ID | Concern | Blocks | Resolution Target |
|:--------:|----|---------|--------|-------------------|
| 🔴 Critical | TR-02/03 (RPi) | T2+R1 not yet measured on target hardware | Phase A go/no-go | EXP-02 R5 — 06/23 |
| 🔴 Critical | TR-04 (Thermal) | 85°C thermal throttle not yet mitigated | Exec deadline on RPi | EXP-02 R6 (SCHED_RR) — 06/24 |
| 🟡 Medium | TR-05 | Filter cutoffs undetermined | Phase A task A-02 | EXP-03 — 06/25 |
| 🟡 Medium | TR-01 | 96kHz RPi capability unconfirmed | ADR-003 (sample rate) | EXP-01 RPi — 06/23 |
| 🟢 Low | NTR-07 | WeiShi accuracy comparison not yet performed | M3 demo criterion | WeiShi session — 06/29 |

---

## Risk Resolution History

| Date | ID | Event |
|------|----|-------|
| 2026-06-15 | TR-02/03 | EXP-02 macOS R2: T2 applied — wait_ms ×32,000 reduction, backlog 0% |
| 2026-06-15 | TR-04 | EXP-02 macOS R3/R4: R1 applied — replot ↓75–85% |
| 2026-06-13 | TR-06/07 | Layer refactoring complete; 116 unit tests passing |
| 2026-06-10 | TR-08 | Domain output review complete — all 11-graph data requirements covered |
| 2026-06-10 | TR-09 | Adaptive threshold implementation complete |
