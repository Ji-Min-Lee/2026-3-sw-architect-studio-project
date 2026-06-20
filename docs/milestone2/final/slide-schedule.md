# Section 3 — Schedule & Remaining Risks

← [Architecture Views](slide-architecture-view.md) | [Presentation Index](README.md)

> **Time**: ~5 min | Goal: M2 sprint recap → open risks → M3 schedule

---

## 3-A. What We Did in Milestone 2

> 📢 **PRESENT** (~2 min)

| Sprint | Date | Focus | Key Outcome |
|--------|------|-------|-------------|
| W2 S1 | 6/9–6/10 | Architecture & Tabs | God Object → 4-layer decomposition; 11-tab impl (KissFFT, VarioTab, QAS-4 alarm); RPi build script; unit tests (TraceTab/BeatErrorTab/VarioTab) |
| W2 S2 | 6/11–6/12 | Deployability & Measurement | Per-frame logging facility (`--log`); RPi system metrics (CPU/mem/temp); full unit-test suite; EXP-02 baseline runs started (macOS + RPi); tab UX fixes (EscapementTab, LongTermTab, BeatErrorTab, VarioTab) |
| W3 S1 | 6/15–6/16 | Experiments & AI Step 1 | EXP-01 ✅ (Dropped=0 at 96kHz → ADR-003); EXP-02 E2-3~E2-7 ✅ (T2+R1 on RPi, E2-7 FG latency 🔴); rule-based watch diagnosis (AI Step 1); Radar/Polar tab; QAS-4 live detach alarm |
| W3 S2 | 6/17–6/18 | Enhancements & AI Step 2 | AI Step 2 — LLM explainer via Ollama (WatchExplainer, streaming, RPi-tuned); Sound Print SP-1/SP-2/SP-3; RS-1/RS-2 trend+stats overlay; RS-4 zoom slider; RS-5 click-to-sync; 4-layer refactor (SessionController, VOs, IAudioSource) |


---

## 3-B. Remaining Risks & Open Items

> 📢 **PRESENT** (~1 min)

| Risk | ID | Severity | Mitigation Plan |
|------|----|:--------:|-----------------|
| FG scheduling latency exceeds 21 ms budget | TR-10 | 🔴 Critical | E2-8: measure `SCHED_FIFO` priority on RPi (W4 S1) |
| Filter tuning not validated on real signal | TR-08 | 🟡 Medium | EXP-03 LP/HP sweep — target 06/25 |
| Qt rendering impact under 14-tab load unknown | TR-09 | 🟡 Medium | EXP-05 FPS benchmark on RPi — target 06/26 |
| WeiShi accuracy not yet validated end-to-end | TR-05 | 🔴 Critical | W5 S1 integration + comparison run |

**Critical path**: FG fix (E2-8) → filter & rendering experiments → WeiShi accuracy → demo

---

## 3-C. M3 Schedule

> 📢 **PRESENT** (~2 min)

| Sprint | Date | Tasks | Grading Area |
|--------|------|-------|--------------|
| W4 S1 | 6/22–6/23 | E2-8: FG scheduling fix (`SCHED_FIFO` on RPi) · Microphone replug auto-recovery | Area 4, Area 6 |
| W4 S2 | 6/24–6/25 | EXP-03: LP/HP filter sweep (real noise WAV) · ADR-003 finalized | Area 6, Area 4 evidence |
| W4 S3 | 6/25–6/26 | EXP-05: 14-tab FPS on RPi · UI layout review (dropdown for less-used controls) | Area 4, Area 6 |
| W4 S4 | 6/26–6/28 | Slides: QA tradeoff · Extensibility · AI-in-development · Bonus polish (Radar + Diagnosis) | Area 3, 5, 7, Bonus |
| W5 S1 | 6/29–6/30 | Full RPi run with real watch · WeiShi accuracy validation · Demo rehearsal | Area 1, Area 4 |
| **M3 Demo** | **7/1** | **Final Demo on Raspberry Pi** | All 8 areas |

