# Section 3 — Remaining Schedule

← [Architecture Views](slide-architecture-view.md) | [Presentation Index](README.md)

> **Time**: ~5 min | Goal: Brief W2–W3 recap → M2→Final schedule in detail

---

## 3-A. Sprint Summary (W2–W3) — Brief

> 📢 **PRESENT** (~1 min)

| Sprint | Focus | Key Outcome |
|--------|-------|-------------|
| W2 S1 (6/9–6/10) | Modifiability | 4-Layer structure established + all 14 tabs ✅ |
| W2 S2 (6/11–6/12) | Deployability | `run_exp.sh` + CSV logger + RPi workflow confirmed |
| W3 S1 (6/16–6/17) | Real-Time / Latency | EXP-02 complete · ADR-001/002 accepted |
| W3 S2 (6/18–6/19) | Correctness under Noise | EXP-03/05 in progress |

---

## 3-B. Experiment Status

> 📢 **PRESENT** (~1 min)

| ID | Experiment | Status | Key Result |
|----|------------|:------:|------------|
| [EXP-01](references/experiments/exp-01-high-res-sampling-beat-error.md) | RPi sample rate sustainability | ✅ 06/15 | Dropped=0 at 96kHz → [ADR-003](references/adr/ADR-003-sample-rate-selection.md) Accepted |
| [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) E2-5/6 | T2+R1 on RPi | ✅ 06/15 | E2E avg **2.05 ms**, 0 deadline miss |
| [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) E2-7 | FG scheduling latency | ✅ 06/16 | fg_wait avg **60.1 ms**, 84% > deadline 🔴 |
| [EXP-03](references/experiments/exp-03-filter-tuning-noise-accuracy.md) | LP/HP filter sweep | ⏳ | Target 06/25 |
| [EXP-05](references/experiments/exp-05-rendering-realtime-impact.md) | Qt 11-tab FPS on RPi | ⏳ | Target 06/26 |

**Critical open risk**: FG scheduling (TR-10) — E2-8 scheduled (W4 S1)

---

## 3-C. M2 → Final Schedule

> 📢 **PRESENT** (~3 min)

| Sprint | Date | Focus | Grading Item |
|--------|------|-------|--------------|
| W4 S1 | 6/22–6/23 | RPi E2-8 (FG scheduling fix) + M2 feedback applied | Experiments (TR-10 resolve) |
| W4 S2 | 6/24–6/25 | EXP-03 filter sweep + ADR-003 finalized | Experiments, ADRs |
| W4 S3 | 6/25–6/26 | EXP-05 rendering FPS + Usability + AI feature | Experiments, Usability QA |
| W4 S4 | 6/26–6/28 | Radar Chart + Diagnosis / Classification | Demo feature completeness |
| W5 S1 | 6/29–6/30 | RPi integration + WeiShi accuracy validation + Demo rehearsal | QAS-1 Accuracy verified, Demo |
| **M3 Demo** | **7/1** | **Final Demo on Raspberry Pi** | — |

**Critical path**: RPi experiments → WeiShi accuracy validation → demo

### Quality Gates *(must pass before M3 demo)*

| Gate | Criteria | Status |
|------|----------|:------:|
| Core pipeline accuracy | Rate / Amplitude / Beat Error match WeiShi within tolerance | ⏳ W5 S1 |
| Real-time | Dropped blocks = 0 at 96kHz on RPi | ✅ EXP-01 |
| DSP latency | E2E avg < 10ms on RPi | ✅ E2-6: 2.05ms |
| FG latency | fg_wait avg < 21ms on RPi | ⏳ E2-8 |
| Rendering | 0% deadline miss under 14-tab load | ⏳ EXP-05 |
| Extensibility | New tab ≤ 3 files, 0 Domain changes | ✅ |

---

*Reference only — not presented:*

- Full risk register: [references/risks.md](references/risks.md)

<details>
<summary>Team Structure (reference)</summary>

| Role | Name |
|------|------|
| Product Owner | Jimin Lee |
| Scrum Master (Experiment Team) | Dong Ho Shin |
| Scrum Master (Development Team) | Sungho Shin |
| **Experiment Team** | Dong Ho Shin, Gyeongjin Shin, Kyudae Bahn, Taejoon Song |
| **Development Team** | Hung Son Tong, Jimin Lee, Sungho Shin |

</details>

<details>
<summary>Explicitly Deferred to M3 (reference)</summary>

| Item | Reason |
|------|--------|
| Timeline scrub (Pause + rewind) | Pause freeze implemented; full scrub exceeds M2 scope |
| `⚠ Noisy signal` warning | Requires SNR computation; deferred pending EXP-03 results |
| BPH range beyond 28,800 BPH | Confirm current operating point first; stretch goal |

</details>
