# Section 3 — Schedule & Remaining Risks

← [Architecture Views](slide-architecture-view.md) | [Presentation Index](README.md)

> **Time**: ~5 min | Goal: M2 sprint recap → open risks → M3 schedule

---

## 3-A. What We Did in Milestone 2

> 📢 **PRESENT** (~2 min)

| Sprint | Date | Focus | Key Outcome |
|--------|------|-------|-------------|
| W2 S1 | 6/9–6/10 | Architecture & Tabs | 4-layer structure established; all 11 graph tabs implemented; unit tests bootstrapped |
| W2 S2 | 6/11–6/12 | Deployability & Measurement | Structured logging + RPi metrics; full unit-test suite; EXP-02 baseline started |
| W3 S1 | 6/15–6/16 | Experiments & AI Step 1 | EXP-01 ✅ ADR-003 accepted; EXP-02 T2+R1 confirmed on RPi; FG latency 🔴 new bottleneck; AI Step 1 (rule-based diagnosis) |
| W3 S2 | 6/17–6/18 | Enhancements & AI Step 2 | AI Step 2 (LLM explainer on RPi); 4-layer refactor complete (IAudioSource, SessionController) |


---

## 3-B. Remaining Risks & Open Items

> 📢 **PRESENT** (~1 min)

| Risk | ID | Severity | Mitigation Plan |
|------|----|:--------:|-----------------|
| FG scheduling latency exceeds real-time budget | TR-10 | 🔴 Critical | Apply priority scheduling on RPi and measure (W4 S1) |
| Filter tuning not validated on real signal | TR-08 | 🟡 Medium | Filter tuning experiment — target 06/25 |
| Rendering impact under full tab load unknown | TR-09 | 🟡 Medium | Rendering performance benchmark on RPi — target 06/26 |
| End-to-end accuracy not yet validated | TR-05 | 🔴 Critical | Full accuracy validation with real watch (W5 S1) |

**Critical path**: FG fix (E2-8) → filter & rendering experiments → WeiShi accuracy → demo

---

## 3-C. M3 Schedule

> 📢 **PRESENT** (~2 min)

GitHub Project Board: [Board](https://github.com/users/Ji-Min-Lee/projects/3/views/2?filterQuery=milestone%3A%22Mileston3+-+Demo%22) · [Table](https://github.com/users/Ji-Min-Lee/projects/3/views/3?filterQuery=milestone%3A%22Mileston3+-+Demo%22) · [Roadmap](https://github.com/users/Ji-Min-Lee/projects/3/views/5?filterQuery=milestone%3A%22Mileston3+-+Demo%22)

| Sprint | Date | Tasks | Grading Area |
|--------|------|-------|--------------|
| W4 S1 | 6/22–6/23 | Fix FG scheduling latency on RPi · Microphone auto-recovery | Area 4, Area 6 |
| W4 S2 | 6/24–6/25 | Filter tuning experiment · ADR finalized | Area 6, Area 4 |
| W4 S3 | 6/25–6/26 | EXP-05: 14-tab FPS on RPi · UI layout review (dropdown for less-used controls) | Area 4, Area 6 |
| W4 S4 | 6/26–6/28 | Slides: QA tradeoff · Extensibility · AI-in-development · Bonus polish · WeiShi accuracy validation · Full RPi run | Area 3, 5, 7, Bonus, Area 4 |
| Buffer | 6/29–6/30 | Buffer & Presentation Prep (no new implementation) | — |
| **M3 Demo** | **7/1** | **Final Demo on Raspberry Pi** | All 8 areas |

