# EXP-02: End-to-End Latency — 2-Segment Timestamp Measurement

**QA**: QAS-2 | **Status**: ✅ Done (2026-06-11 ~ 2026-06-16)

---

## Objective

Measure end-to-end latency from audio block ready to beat result delivered, and identify the bottleneck causing deadline misses — guiding the T2 and R1 tactic decisions ([ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md), [ADR-002](../adr/ADR-002-r1-lazy-rendering.md)).

## Result

DSP E2E avg **2.2 ms** / max **4.8 ms** — within QAS-2 target (< 100 ms).

## Measurement Segments

- **TS1**: `emit PlaybackDataReady(TG_NOW())` — audio block ready
- **TS2**: `TG_NOW()` at entry of `MainWindow::HandleInputData()`
- `wait_ms` = TS2 − TS1 (Qt queue wait) · `exec_ms` = HandleInputData exit − TS2 (processing)

## Run History

> Log files in `src/logs/EXP-02/`.

| Run | Date | Configuration | E2E avg/max (ms) | Note | Data |
|:---:|------|--------------|:----------------:|------|:----:|
| E3-01 | 2026-06-12 | Windows reference (dev PC) | 2.8 / 363.9 | Healthy baseline | [csv](../../../../../src/logs/EXP-02/log_20260612_132536.csv) · [plot](../../../../../src/logs/EXP-02/log_20260612_132536.png) |
| E3-02 | 2026-06-11 | rpi1 — unoptimized | 255.4 / 900.9 | ❌ exec overrun 43 % | [csv](../../../../../src/logs/EXP-02/log_20260611_145543.csv) · [plot](../../../../../src/logs/EXP-02/log_20260611_145543.png) |
| E3-03 | 2026-06-15 | rpi2 baseline | 57.2 / 208.9 | ❌ `plot` on exec path | [csv](../../../../../src/logs/EXP-02/log_20260615_152751.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_152751.png) |
| E3-04 | 2026-06-15 | rpi2 + multi-tab | 80.1 / 258.7 | FG queue lag | [csv](../../../../../src/logs/EXP-02/log_20260615_162055.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_162055.png) |
| E3-05 | 2026-06-15 | +T2 (DSP offload thread) | 2.1 / 11.1 | ✅ **−97 %** | [csv](../../../../../src/logs/EXP-02/log_20260615_163106.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_163106.png) |
| E3-06 | 2026-06-15 | +R1 (Lazy Rendering) | 2.1 / 5.7 | ✅ tighter max | [csv](../../../../../src/logs/EXP-02/log_20260615_165612.csv) · [plot](../../../../../src/logs/EXP-02/log_20260615_165612.png) |
| E3-07 | 2026-06-16 | +Thread monitoring log | 2.2 / 4.8 | ✅ DSP healthy; per-thread timing confirmed | [csv](../../../../../src/logs/EXP-02/log_20260616_140850.csv) · [plot](../../../../../src/logs/EXP-02/log_20260616_140850.png) · [timeline](../../../../../src/logs/EXP-02/log_20260616_140850_timeline_dark_all.png) |

## Architecture Decisions

| Tactic | Outcome | ADR |
|--------|---------|:---:|
| T2 — DSP Offload Thread | wait_ms 420 ms → 0.013 ms (×32,000); E2E 80 ms → 2.1 ms (−97%) | [ADR-001](../adr/ADR-001-t2-dsp-offload-thread.md) |
| R1 — Lazy Rendering | replot_count 8.22 → 1.20/beat (↓85%); max tail 11.1 → 5.7 ms | [ADR-002](../adr/ADR-002-r1-lazy-rendering.md) |

## Links

- Analysis tool: `src/tools/analyze_log.py`
