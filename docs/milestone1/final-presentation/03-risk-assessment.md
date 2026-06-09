# Risk Assessment — TimeGrapher

---

This document answers three questions in order:

1. What are the technical and non-technical risks, and how are Probability and Impact rated?
2. Are open questions/issues clearly linked to things that will affect the project outcome?
3. Have actions been identified to address the open questions/issues?

---

## 1. Risk Assessment Criteria

Each risk is rated on Probability and Impact on a three-point scale: H / M / L.

| Level | Probability Criteria | Impact Criteria |
|:-----:|---------------------|----------------|
| **H (High)** | Cannot be guaranteed without experiment / already expected to occur from prior knowledge | QA target unachievable → directly threatens team primary goal (accurate measurement) or delivery |
| **M (Medium)** | Can be mitigated by design decisions but may worsen depending on experiment results | Some QA target degraded / development velocity reduced |
| **L (Low)** | Known mitigation exists or probability is low | Partial feature shortfall or only affects specific environments |

**Overall Risk = max(Probability, Impact)**. If either axis is H, the risk is rated H overall.

---

## 2. Technical Risks

### 2.1 Technical Risk Summary

| ID | Risk | Prob | Impact | Overall | Linked QA | Open Issue |
|----|------|:----:|:------:|:-------:|:---------:|:----------:|
| TR-01 | RPi 5 cannot achieve Dropped Block = 0 at 96k sps | H | H | **H** | QAS-1 | OI-P1 |
| TR-02 | Linux scheduler jitter causes intermittent Dropped Blocks | M | H | **H** | QAS-1 | — |
| TR-03 | End-to-end latency exceeds beat period (RPi + Qt unverified) | H | H | **H** | QAS-2 | OI-L1, OI-L2 |
| TR-04 | 11-tab simultaneous rendering pushes ② process→display > 30 ms | M | M | **M** | QAS-2 | OI-L2 |
| TR-05 | Detector default parameters fail under ambient noise | M | M | **M** | QAS-3 | OI-C1 |
| TR-06 | Residual coupling after Observer refactoring violates ≤ 3-file constraint | M | M | **M** | QAS-5 | — |
| TR-07 | Regression in existing graphs during God Object decomposition | M | H | **H** | QAS-5 | — |
| TR-08 | New graph requires data not currently provided by MeasurementEngine | L | M | **M** | QAS-5 | — |
| TR-09 | Signal quality warning thresholds mismatched to real environment | M | L | **M** | QAS-4 | OI-U1, OI-U2 |

---

### TR-01 — RPi 5 Cannot Achieve Dropped Block = 0 at 96k sps

| Item | Detail |
|------|--------|
| **Description** | If the DSP pipeline exceeds the 96k sps block period (~10 ms), Ring Buffer overflows, causing Dropped Blocks |
| **Probability: H** | Whether RPi 5 can sustain 96k sps while running Qt GUI + DSP concurrently is unverified before experiment |
| **Impact: H** | Dropped Block → T1/T3 timestamp lost → Rate·Amplitude·Beat Error computation impossible → **team primary goal collapses**. Prerequisite for all QAs |
| **Mitigation tactics** | ① Graceful Degradation: auto-fallback to 48k sps if 96k is unachievable (guarantees Dropped Block = 0, lower resolution) ② Lock-Free Ring Buffer: eliminates mutex to prevent DSP delays |
| **Residual risk** | At 48k sps fallback, T1 detection resolution degrades to 20.8 µs/sample — 36,000 / 43,200 BPH support becomes infeasible |
| **Linked issue** | OI-P1 → resolved by **EXP-01** |

---

### TR-02 — Linux Scheduler Jitter Causes Intermittent Dropped Blocks

| Item | Detail |
|------|--------|
| **Description** | Linux general-purpose scheduler treats audio thread priority low, causing intermittent block period violations → sporadic Dropped Blocks → accumulated long-term Rate measurement error |
| **Probability: M** | Known characteristic of Linux general-purpose scheduler; occurs if audio thread priority is left at default |
| **Impact: H** | Sporadic block loss accumulates Rate measurement error over time → accuracy degradation; difficult to reproduce, increasing debugging cost |
| **Mitigation tactic** | Priority Scheduling: elevate audio thread priority (`SCHED_RR` or `SCHED_FIFO`) to absorb Linux scheduler jitter |
| **Residual risk** | Effectiveness on RPi must be co-verified through EXP-01 |
| **Linked issue** | Indirectly confirmed from sporadic Dropped Block patterns in EXP-01 results |

---

### TR-03 — End-to-End Latency Exceeds Beat Period (RPi + Qt Unverified)

| Item | Detail |
|------|--------|
| **Description** | If the full pipeline (audio capture → DSP → Qt GUI rendering) cannot complete within 100 ms (80% of 125 ms beat period at 28,800 BPH), real-time display fails |
| **Probability: H** | Whether ① capture→process (< 70 ms) + ② process→display (< 30 ms) can both be met on RPi 5 + Qt6 is unverified before EXP-02; QAudioSource live callback period itself is unconfirmed |
| **Impact: H** | Latency violation → next beat processing starts before previous beat display completes → real-time display breakdown → indirect effect on T1/T3 timestamp accuracy |
| **Mitigation tactics** | ① Introduce Concurrency (Audio/DSP/GUI thread separation) ② Lock-Free Ring Buffer ③ Lazy Rendering (render active tab only) ④ Reduce Computational Overhead |
| **Linked issues** | OI-L1 (callback period unknown), OI-L2 (11-tab rendering overhead) → resolved by **EXP-02** |

---

### TR-04 — 11-Tab Simultaneous Rendering Pushes ② process→display > 30 ms

| Item | Detail |
|------|--------|
| **Description** | Once 11 graph tabs are complete, Qt main thread rendering load may cause ② process→display to exceed 30 ms |
| **Probability: M** | 1-2 tabs may be fine; accumulated load after 11 tabs is plausible but unverified |
| **Impact: M** | If it causes end-to-end violation → same outcome as TR-03; however, Lazy Rendering can mitigate it |
| **Mitigation tactic** | Lazy Rendering: execute `paintEvent()` only for the active tab; update data for inactive tabs but defer rendering |
| **Linked issue** | OI-L2 → confirmed in EXP-02 by comparing 1-tab vs. 11-tab configurations |

---

### TR-05 — Detector Default Parameters Fail Under Ambient Noise

| Item | Detail |
|------|--------|
| **Description** | Default `onset_fraction` (0.03) / `min_peak_fraction` (0.20) in `Detector.cpp` may degrade beat detection quality under ambient noise |
| **Probability: M** | Adaptive threshold algorithm is implemented, but whether default parameters hold under 3 noise conditions (low/medium/high) is experimentally unverified |
| **Impact: M** | Δ Rate / Δ Amplitude / Δ Beat Error increases → partial degradation of team primary goal; reliability decreases but measurement does not fail completely |
| **Mitigation tactic** | Adaptive threshold (noise floor = 75th percentile of last 256 ms, reference_peak = median of last 16 beats) + confirm optimal parameters via EXP-03 |
| **Linked issue** | OI-C1 → resolved by **EXP-03** (3 noise conditions × Detector parameter combinations) |

---

### TR-06 — Residual Coupling After Observer Refactoring Violates ≤ 3-File Constraint

| Item | Detail |
|------|--------|
| **Description** | Residual coupling between Presentation Layer and Signal Processing / Acquisition layers persists after Observer pattern refactoring, causing new graph additions to touch more than 3 files |
| **Probability: M** | Full layer separation requires deep understanding of existing code; partial coupling likely to remain during refactoring |
| **Impact: M** | ≤ 3-file constraint violated → graph addition cost grows in later development weeks, parallel development schedule risk materializes |
| **Mitigation tactic** | Restrict Dependencies (Layered Architecture): enforce rule that Presentation Layer may only reference Domain Layer (MeasurementEngine interface) |
| **Linked issue** | Directly verified by ≤ 3-file experiment after Observer pattern refactoring is complete |

---

### TR-07 — Regression in Existing Graphs During God Object Decomposition

| Item | Detail |
|------|--------|
| **Description** | Side effects in existing graphs (Trace, Beat Error, etc.) during the structural transition from God Object to layered Signal-Slot subscription model |
| **Probability: M** | Common risk in large-scale structural refactoring; existing codebase (`TimeGrapher_v10.5`) lacks automated regression tests, making detection difficult |
| **Impact: H** | Regression → rollback → revert to God Object structure → QAS-5 (Extensibility) abandoned → parallel development of 11 graphs collapses entirely |
| **Mitigation tactics** | ① Incremental refactoring (decompose by feature, verify at each step) ② Manually compare Rate·Amplitude·Beat Error values on the same watch under the same conditions before and after refactoring ③ Write unit tests for core computation logic (Rate·Amplitude·Beat Error) — establish baseline before refactoring, run same tests after to automatically detect regressions |
| **Linked issue** | No separate open issue — risk is intrinsic to the refactoring execution phase |

---

### TR-08 — New Graph Requires Data Not Currently Provided by MeasurementEngine

| Item | Detail |
|------|--------|
| **Description** | A new graph (e.g., Spectrogram) requires raw audio data not currently published by MeasurementEngine, expanding the modification scope into the preprocessing layer |
| **Probability: L** | Most of the 11 graphs need only Rate·Amplitude·Beat Error derivatives; excluding raw-audio-dependent graphs from scope keeps probability low |
| **Impact: M** | ≤ 3-file constraint violated + TR-07 regression risk co-occurs due to preprocessing layer modification |
| **Mitigation tactic** | Review the 11-graph list in advance to confirm whether MeasurementEngine's current output covers all data needs; expand Domain Layer interface first if not |
| **Linked issue** | Addressed by pre-implementation review when graph implementation plan is finalized |

---

### TR-09 — Signal Quality Warning Thresholds Mismatched to Real Environment

| Item | Detail |
|------|--------|
| **Description** | Warning onset/clear thresholds (N·M seconds, noise/signal ratio) for `⚠ No signal` / `⚠ Noisy signal` may not match real-environment ambient noise levels, causing false alarms or missed warnings |
| **Probability: M** | Threshold is environment-dependent; noise floor may differ significantly between lab and field environments |
| **Impact: L** | User experience degradation — false alarms undermine trust in measurements or cultivate habit of ignoring warnings; no direct effect on measurement accuracy (independent of QAS-3 Correctness) |
| **Mitigation tactic** | Heartbeat pattern (reuse existing A/C events): display `⚠ No signal` if no beat event for N seconds without additional detection logic — threshold tuning simplified to a single parameter |
| **Linked issues** | OI-U1 (N·M values), OI-U2 (noise/signal thresholds) → resolved by watch removal/restore experiment + multi-environment threshold search |

---

## 3. Non-Technical Risks

### 3.1 Non-Technical Risk Summary

| ID | Risk | Prob | Impact | Overall | Linked QA | Open Issue |
|----|------|:----:|:------:|:-------:|:---------:|:----------:|
| NTR-01 | Schedule overload for parallel implementation of 11 graphs in 5 weeks | H | H | **H** | QAS-5 | — |
| NTR-02 | Coding/architecture team boundary unclear — design decisions not reflected in implementation | H | H | **H** | All QAs | OI-06 |
| NTR-03 | Scope overextension — implementing all 11 graphs degrades core feature quality | H | M | **H** | QAS-5 | OI-07 |
| NTR-04 | English communication overhead — risk of design decisions not reaching all team members | M | H | **H** | All QAs | OI-08 |
| NTR-05 | Single RPi 5 device creates experiment bottleneck | M | M | **M** | QAS-1, QAS-2 | — |
| NTR-06 | Delayed experiments prevent finalizing provisional values | M | M | **M** | QAS-1, QAS-2, QAS-3 | — |

---

### NTR-01 — Schedule Overload for Parallel Implementation of 11 Graphs in 5 Weeks

| Item | Detail |
|------|--------|
| **Description** | Schedule requires 11 graphs to be divided among team members and developed in parallel during Weeks 3–4; under God Object structure, code conflicts and debugging costs escalate as development progresses |
| **Probability: H** | 11 graphs × complex dependencies = conflicts inevitable in parallel work; occurs if QAS-5 Extensibility is not met |
| **Impact: H** | Schedule overrun → incomplete graph demo → delivery threat; affects all subsequent milestones |
| **Mitigation** | Achieving QAS-5 Extensibility (≤ 3-file structure) is the direct mitigation for this risk; independent graph addition eliminates parallel work conflicts |
| **Linked issues** | Directly tied to Architectural Drivers team objective 3rd (extensible architecture); linked to TR-06 and TR-07 |

---

### NTR-02 — Coding/Architecture Team Boundary Unclear

| Item | Detail |
|------|--------|
| **Description** | Role boundary between coding team and architecture team is undefined; architecture decisions (module separation direction, QA tactic selection) may not be reflected in implementation |
| **Probability: H** | Synchronization process is currently undefined; working independently causes immediate design–implementation divergence |
| **Impact: H** | Unimplemented architecture decisions → full rework risk before M2; QA tactics (Lock-Free Ring Buffer, Observer pattern, etc.) not applied in code → all QA goals missed |
| **Mitigation** | Fix daily afternoon sync meeting + communicate via Teams channel; confirm and document coding/architecture role boundaries in Project Plan |
| **Linked issue** | OI-06 → resolved by establishing role boundaries and sync process |

---

### NTR-03 — Scope Overextension — Implementing All 11 Graphs Degrades Core Quality

| Item | Detail |
|------|--------|
| **Description** | Attempting to implement all 11 graphs within 5 weeks degrades quality of core measurement features (Rate·Amplitude·Beat Error accuracy) or makes QA targets unachievable — scope overextension risk |
| **Probability: H** | Implementing all 11 graphs is high load relative to team size and timeline; without priority classification, quality degradation near deadline is inevitable |
| **Impact: M** | Lower completeness of core features (QAS-1~3) directly affects demo quality; partially mitigated if QAS-5 (Extensibility) enables parallel development |
| **Mitigation** | Classify 11 graphs into **Core / Required / Stretch**; only Core graphs (directly tied to team's primary goal) are mandatory for M2 demo; Stretch added if time permits |
| **Linked issue** | OI-07 → resolved by reflecting graph priorities as Core / Required / Stretch in Project Plan |

---

### NTR-04 — English Communication Overhead

| Item | Detail |
|------|--------|
| **Description** | Instructors and reviewers are English-speaking and milestone submissions/presentations are in English; Korean-dominant team communication risks design decisions not being accurately conveyed to English-speaking members or evaluators |
| **Probability: M** | Korean-dominant internal communication is natural, but Korean-only deliverables fall short of evaluation criteria |
| **Impact: H** | English-speaking team member misses context → design–implementation mismatch; lower deliverable quality → direct impact on evaluation score |
| **Mitigation** | Write all deliverables in **bilingual (KO/EN)** format; milestone submissions and presentations in English; internal team communication in Korean is acceptable, but design decision summaries must be recorded bilingually in Teams |
| **Linked issue** | OI-08 → resolved by agreeing on writing standard and verifying all M1 deliverables comply with bilingual rule |

---

### NTR-05 — Single RPi 5 Device Creates Experiment Bottleneck

| Item | Detail |
|------|--------|
| **Description** | EXP-01 (real-time performance), EXP-02 (latency), EXP-03 (Detector parameters) all require RPi 5; if the team owns only one device, experiments must run sequentially, creating a schedule bottleneck |
| **Probability: M** | Sequential dependencies between experiments (e.g., EXP-01 → Observer refactoring → EXP-02) make full parallelization difficult; partially mitigated through shared device scheduling |
| **Impact: M** | Experiment delays → design decisions deferred with provisional values → Architectural Drivers update delayed → subsequent milestone start delayed |
| **Mitigation** | ① Prioritize experiments: EXP-01 (resolves TR-01) first, then EXP-02, then EXP-03 ② Develop RPi-independent work (GUI layout, Observer refactoring) on Windows PC in parallel |
| **Linked issues** | Affects OI-P1, OI-L1, OI-L2, OI-C1 broadly |

---

### NTR-06 — Delayed Experiments Prevent Finalizing Provisional Values

| Item | Detail |
|------|--------|
| **Description** | Provisional values (⚠️) in Architectural Drivers — 96k sps target, 100 ms latency target, optimal Detector parameters — are not confirmed before M2 due to experiment delays |
| **Probability: M** | Experiments depend on prerequisites (Observer refactoring, hardware setup) that can delay execution |
| **Impact: M** | Designs based on provisional values → rework required if experiments contradict assumptions → M2 schedule pressure |
| **Mitigation** | Execute EXP-01 as early as possible within M1; design conservatively (48k sps fallback, 100 ms upper bound) so minimum behavior is guaranteed regardless of experiment outcome |
| **Linked issues** | OI-P1, OI-L1, OI-L2, OI-L3, OI-C1 — directly linked to Open Issues section of Architectural Drivers |

---

## 4. Open Issues — Project Outcome Linkage

The following table maps all Architectural Drivers open issues (OI-*) to risks and actions. All OIs are directly or indirectly linked to the team's primary goal (accurate measurement).

| OI ID | Open Question | Project Impact | Linked Risk | Action |
|:-----:|--------------|:-------------:|:-----------:|:------:|
| **OI-P1** | Can RPi 5 achieve Dropped Block = 0 at 96k sps? | If not → 48k sps fallback → 36k/43k BPH unsupported → team 2nd goal abandoned | TR-01 | Run **EXP-01** |
| **OI-L1** | What is QAudioSource live callback period? | Longer than ~20 ms → capture→process lower bound rises → latency target may be exceeded | TR-03 | **EXP-02** TS1 measurement |
| **OI-L2** | Is ② process→display < 30 ms with 11 tabs? | If not → Lazy Rendering enforced → inactive tab data freshness limited | TR-03, TR-04 | **EXP-02** 1-tab vs 11-tab comparison |
| **OI-L3** | After 28,800 BPH, can target be raised to 36k/43k? | If not achieved → team 2nd goal abandoned → QAS-2 Stretch unconfirmed | — | Deferred to **EXP-05** after all 28,800 BPH QA targets are met |
| **OI-C1** | What Detector parameters minimize Δ across 3 noise conditions? | Unresolved → accuracy degrades under noise → QAS-3 Response Measure unconfirmed | TR-05 | Run **EXP-03** |
| **OI-U1** | What are the N·M second values for warning onset/clear? | Unresolved → QAS-4 Response Measure unconfirmed → Usability goal unverifiable | TR-09 | Watch removal/restore experiment |
| **OI-U2** | What are No signal / Noisy signal thresholds? | Unresolved → false alarms or missed warnings → user trust degraded | TR-09 | Multi-environment threshold search |
| **OI-06** | Team structure undefined | Design decisions not implemented → rework risk before M2 | NTR-02 | ✅ Resolved by forming Team 1 + Team 2 parallel Agile structure (ADD-based 2-day Scrum) per project-plan.md |
| **OI-07** | Graph priority not classified | Core features incomplete at demo | NTR-03 | ✅ Classified in project-plan.md Section 6: Core (3) / Required (3) / Stretch (2+) |
| **OI-08** | No standard for deliverable writing language | English-speaking member misses context + milestone quality degraded | NTR-04 | ✅ Agreed: team communication and deliverables in bilingual (KO/EN); presentations and submissions in English |

---

## 5. Action Plan

Execution order of experiments and design actions to resolve risks.

| Order | Action | Target Risk | Prerequisite | Resolved OI |
|:-----:|--------|:-----------:|-------------|:-----------:|
| **1** | **EXP-01**: 48k/96k/192k sps × 10 min Dropped Block measurement | TR-01, TR-02 | RPi 5 setup, mechanical watch connected | OI-P1 |
| **2** | **Observer pattern refactoring** (incremental, with unit tests + regression verification) | TR-06, TR-07, NTR-01 | EXP-01 result (SPS direction decided) | — |
| **3** | **≤ 3-file structure verification** (add a new graph for real) | TR-06 | Observer refactoring complete | — |
| **4** | **EXP-02**: 3-segment timestamps × 1-tab/11-tab × 3 SPS tiers | TR-03, TR-04 | Observer refactoring complete | OI-L1, OI-L2, OI-L3 |
| **5** | **EXP-03**: 3 noise conditions × Detector parameter combinations | TR-05 | EXP-01 complete (SPS decided) | OI-C1 |
| **6** | **Warning threshold experiment**: watch removal/restore + multi-environment search | TR-09 | Observer refactoring complete | OI-U1, OI-U2 |
| ~~**Immediate**~~ **✅ Done** | **Team structure definition**: Team 1 + Team 2 parallel Agile structure per project-plan.md | NTR-02 | — | OI-06 |
| ~~**Immediate**~~ **✅ Done** | **Graph priority classification**: Core (3) / Required (3) / Stretch (2+) finalized in project-plan.md Section 6 | NTR-03 | — | OI-07 |
| ~~**Immediate**~~ **✅ Done** | **Writing standard agreement**: bilingual (KO/EN) for all deliverables; English for presentations and submissions | NTR-04 | — | OI-08 |

---

## 6. Risk–QA Consolidated Mapping

| QA (per Architectural Drivers) | Priority | Linked Technical Risk | Linked Non-Technical Risk | Highest Overall |
|:------------------------------|:-------:|:--------------------:|:-------------------------:|:---------------:|
| Real-Time Performance (QAS-1) | 1 | TR-01 (**H**), TR-02 (**H**) | NTR-05, NTR-06, NTR-02 (**H**) | **H** |
| Low Latency (QAS-2) | 2 | TR-03 (**H**), TR-04 (M) | NTR-05, NTR-06 | **H** |
| Correctness (QAS-3) | 3 | TR-05 (M) | NTR-06 | **M** |
| Usability (QAS-4) | 4 | TR-09 (M) | — | **M** |
| Extensibility (QAS-5) | 5 | TR-06 (M), TR-07 (**H**), TR-08 (M) | NTR-01 (**H**), NTR-02 (**H**), NTR-03 (**H**) | **H** |
| All QAs | — | — | NTR-04 (**H**) | **H** |
