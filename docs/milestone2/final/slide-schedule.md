# Section 3 — Remaining Schedule

← [Architecture Views](slide-architecture-view.md) | [Presentation Index](README.md)

> **발표 시간**: ~5분 | 목표: W2–W3 간략 회고 → M2 이후 Final까지 상세 일정

---

## 3-A. Sprint Summary (W2–W3) — 간략히

> 📢 **PRESENT** (~1분)

| Sprint | Focus | Key Outcome |
|--------|-------|-------------|
| W2 S1 (6/9–6/10) | Modifiability | 4-Layer 구조 확립 + 11개 탭 ✅ |
| W2 S2 (6/11–6/12) | Deployability | `run_exp.sh` + CSV logger + RPi 워크플로 확인 |
| W3 S1 (6/16–6/17) | Real-Time / Latency | EXP-02 완료 · ADR-001/002 확정 |
| W3 S2 (6/18–6/19) | Correctness under Noise | EXP-03/05 진행 중 |

---

## 3-B. Experiment Status

> 📢 **PRESENT** (~1분)

| ID | Experiment | Status | Key Result |
|----|------------|:------:|------------|
| [EXP-01](references/experiments/exp-01-high-res-sampling-beat-error.md) | RPi sample rate sustainability | ✅ 06/15 | Dropped=0 at 96kHz → [ADR-003](references/adr/ADR-003-sample-rate-selection.md) Accepted |
| [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) E2-5/6 | T2+R1 on RPi | ✅ 06/15 | E2E avg **2.05 ms**, 0 deadline miss |
| [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) E2-7 | FG scheduling latency | ✅ 06/16 | fg_wait avg **60.1 ms**, 84% > deadline 🔴 |
| [EXP-03](references/experiments/exp-03-filter-tuning-noise-accuracy.md) | LP/HP filter sweep | ⏳ | Target 06/25 |
| [EXP-05](references/experiments/exp-05-rendering-realtime-impact.md) | Qt 11-tab FPS on RPi | ⏳ | Target 06/26 |

**Critical open risk**: FG scheduling (TR-10) — E2-8 측정 예정 (W4 S1)

---

## 3-C. M2 → Final 일정

> 📢 **PRESENT** (~3분)

| Sprint | Date | Focus | 채점표 항목 |
|--------|------|-------|------------|
| W4 S1 | 6/22–6/23 | RPi E2-8 (FG scheduling fix) + M2 feedback 반영 | Experiments (TR-10 resolve) |
| W4 S2 | 6/24–6/25 | EXP-03 filter sweep + ADR-003 finalized | Experiments, ADRs |
| W4 S3 | 6/25–6/26 | EXP-05 rendering FPS + Usability + AI feature | Experiments, Usability QA |
| W4 S4 | 6/26–6/28 | Radar Chart + Diagnosis / Classification 보강 | Demo feature completeness |
| W5 S1 | 6/29–6/30 | RPi integration + WeiShi accuracy validation + Demo rehearsal | QAS-1 Accuracy 검증, Demo |
| **M3 Demo** | **7/1** | **Final Demo on Raspberry Pi** | — |

**Critical path**: RPi 실험 → WeiShi 정확도 검증 → 데모

### Quality Gates *(M3 데모 전 통과 필요)*

| Gate | Criteria | Status |
|------|----------|:------:|
| Core pipeline accuracy | Rate/Amplitude/Beat Error ≈ WeiShi | ⏳ W5 S1 |
| Real-time | Dropped blocks = 0 at 96kHz on RPi | ✅ EXP-01 |
| DSP latency | E2E avg < 10ms on RPi | ✅ E2-6: 2.05ms |
| FG latency | fg_wait avg < 21ms on RPi | ⏳ E2-8 |
| Rendering | 0% deadline miss under 14-tab load | ⏳ EXP-05 |
| Extensibility | New tab ≤ 3 files, 0 Domain changes | ✅ |

---

*Reference only — not presented:*

- Full risk register: [references/risks.md](references/risks.md)
- Team structure and role assignments: 아래 참고

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
| Timeline scrub (Pause + rewind) | Pause freeze 구현 완료; full scrub은 M2 scope 초과 |
| `⚠ Noisy signal` warning | SNR 연산 필요; EXP-03 결과 후 결정 |
| BPH range beyond 28,800 BPH | 현 operating point 확인 후 stretch goal |

</details>
