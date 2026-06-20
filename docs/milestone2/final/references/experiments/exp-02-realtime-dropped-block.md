# EXP-02: RPi Real-Time Performance — Dropped Block Measurement

**QA**: QAS-2 | **Status**: ✅ Done (2026-06-15)

---

## Objective

Verify that RPi 5 sustains **zero dropped audio blocks** at 96k sps under continuous operation, confirming QAS-2 is achievable without real-time scheduling extensions.

## Result

**Dropped Block = 0** across all 9 runs (3 sps × 3 scheduling policies). QAS-2 Pass.

**Confirmed operating point**: 96k sps, default scheduling — exec avg 9.6 ms (< 21.3 ms deadline).

## Key Findings

| Finding | Detail |
|---------|--------|
| Target sps | **96k** — 0 drops, exec avg 9.6 ms well within 21.3 ms deadline |
| SCHED_RR / FIFO | Not required — no improvement in drop count |
| Thermal throttling | ≥ 85 °C on rpi1; 30 s ring buffer absorbs exec spikes → Dropped stays 0 |

## Run Summary

Platform: RPi (host=lg1), 5 min/sps, 30 s ring buffer. Deadline: 48k=42.67 ms · **96k=21.33 ms** · 192k=10.67 ms.

| Run | Scheduling | sps | exec avg/max (ms) | exec > deadline | Dropped | Data |
|:---:|-----------|:---:|:-----------------:|:---------------:|:-------:|:----:|
| E2-01 | default | 48k | 5.8 / 36.6 | 4.9 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_203222_48000_default.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_203222_48000_default.png) |
| E2-02 | default | **96k** ★ | **9.6 / 39.2** | **8.1 %** | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_204746_96000_default.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_204746_96000_default.png) |
| E2-03 | default | 192k | 15.8 / 51.6 | 12.1 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_210310_192000_default.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_210310_192000_default.png) |
| E2-04 | SCHED_RR p50 | 48k | 6.9 / 37.5 | 6.6 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_203730_48000_rr.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_203730_48000_rr.png) |
| E2-05 | SCHED_RR p50 | **96k** ★ | **9.8 / 39.9** | **8.4 %** | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_205254_96000_rr.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_205254_96000_rr.png) |
| E2-06 | SCHED_RR p50 | 192k | 16.0 / 61.7 | 12.5 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_210818_192000_rr.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_210818_192000_rr.png) |
| E2-07 | SCHED_FIFO p50 | 48k | 7.2 / 35.2 | 6.9 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_204238_48000_fifo.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_204238_48000_fifo.png) |
| E2-08 | SCHED_FIFO p50 | **96k** ★ | **9.9 / 41.4** | **8.6 %** | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_205802_96000_fifo.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_205802_96000_fifo.png) |
| E2-09 | SCHED_FIFO p50 | 192k | 16.0 / 52.1 | 12.5 % | **0** | [csv](../../../../../src/logs/EXP-01/log_20260615_211326_192000_fifo.csv) · [plot](../../../../../src/logs/EXP-01/log_20260615_211326_192000_fifo.png) |

★ QAS-2 target sps

## Architecture Decisions

→ **96k sps accepted** — [ADR-003](../adr/ADR-003-sample-rate-selection.md). SCHED_RR / FIFO not applied.

## Links

- Risk resolved: [TR-01](../risks.md)
- Full run table: [experiment-results.md](../../../../milestone2/experiment-results.md#exp-02-rpi-real-time-performance----dropped-block-measurement)
- Log directory: `src/logs/EXP-01/` (legacy directory name)
