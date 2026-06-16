# Presentation Script — TimeGrapher M2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

---

## Slide Flow & Timing (target: ~20 min)

| # | Section | Slides | Est. Time |
|---|---------|--------|:---------:|
| 1 | M1 Feedback & Improvements | Feedback table + QA Priority | 4 min |
| 2 | Architectural Approach Overview | 4 architecture views | 5 min |
| 3 | Risk → Experiment → Architecture Decision | 3 cases (ADR-001, ADR-002, pending) | 7 min |
| 4 | Remaining Issues | Open experiments + risks | 2 min |
| 5 | Schedule | Construction plan | 2 min |

---

## Opening (30 sec)

Hello, we are Team 3 — Blue Sky.

Our M2 story is about evidence driving architecture.
In M1, we set up experiments and proposed architectural approaches.
In M2, we ran the key experiment, measured the failure on actual hardware, traced the root cause structurally, made two architecture decisions with data, and validated both.

Today we'll walk you through: what we fixed from M1 feedback, how the architecture is structured, and most importantly — the causal chain from risk to experiment to architecture decision.

---

## Section 1 — M1 Feedback & Improvements (4 min)

### M1 Feedback Response

Let's start with the M1 feedback directly.

[point to feedback table]

The critical points were: no owner or date per task, experiments not tracked as tasks, no README, architecture diagrams too detailed or unlabeled, and tactics mixed into the QA document.

We've addressed all five.

The most important structural fix: in M1, our QA document described solutions, not problems. The mentor was right to flag this. A QA requirement should express what the system must achieve, not how. We moved tactics to the Architectural Approaches document and rewrote QAs in problem-only language. Numbers that were "provisional" are now confirmed by EXP-02.

### QA Priority Reframe

This brings us to a reframe that changed how we think about the whole project.

[point to QA priority diagram]

Measurement Accuracy is not one QA among equals — it is the criterion the entire architecture is evaluated against. Rate, Amplitude, and Beat Error must match the WeiShi No.1000 reference.

Every other QA is a structural prerequisite. Real-Time Performance means: if the pipeline misses the 21ms deadline, a beat event is dropped — you cannot compute Rate for that cycle. Low Latency means: if the capture-to-detect path takes longer than one beat period, the timestamp shifts — Beat Error and Amplitude become wrong. Signal Quality means: if a false trigger fires, the math computes valid-looking numbers that are simply incorrect.

Modifiability is the execution enabler. Without a clean layer structure, two teams can't build 11 graph tabs in parallel without blocking each other.

One trade-off we accepted explicitly: BPH coverage is narrowed to 28,800 BPH for M3. Full range from 18,000 to 36,000 BPH is an accuracy stretch goal, not a structural constraint.

---

## Section 2 — Architectural Approach Overview (5 min)

### The Core Approach

The architecture has two structural pillars.

First: a 4-layer allowed-to-use structure — Acquisition, Signal Processing, Domain, Presentation — with a strict downward dependency rule. Presentation talks to Domain only. Domain does not depend on Presentation. This is how we enable 11 graph tabs to be built in parallel.

Second: explicit concurrency — two threads, two responsibilities. The audio pipeline runs on its own thread. The UI thread handles only rendering. The two are connected by a thread-safe ring buffer and Qt QueuedConnection.

Both pillars were forced by evidence. EXP-02 showed the original single-threaded design fails real-time constraints on Raspberry Pi — 43% deadline miss. We could not fix this with algorithm tuning. It required structural change.

### View 1 — 4-Layer Allowed-to-Use

[show view1-layered-module.png]

Four layers, one rule: dependencies flow downward only.

Acquisition captures PCM. Signal Processing filters and detects beats. Domain computes Rate, Amplitude, Beat Error, BPH. Presentation renders 11 graph tabs.

If you add a new graph tab, you add one class to Presentation. You touch nothing below Domain. This is our Modifiability QA enforced at the structure level.

### View 2 — Graph Tab Decomposition

[show view2-decomposition.png]

This zooms into Presentation. `GraphTabManager` manages all tabs through the `IGraphTab` interface. Each tab is independently developed and tested. No cross-tab dependencies.

Extension rule: one class, zero Domain changes. This is also how parallel development is structured — each team member owns specific tabs without conflict.

### View 3 — DSP Pipeline Thread Model

[show view3-thread-model.png]

This is the runtime view — two threads and how they connect.

The Audio Thread runs AudioCapture and DSPWorker. The ring buffer sits between them. Qt QueuedConnection emits measurement results to the UI thread asynchronously.

You'll notice two decisions are labeled directly in the diagram — ADD-2-01 (T2) where the DSP offload happens, and ADD-2-02 (R1) where the lazy rendering guard operates. I'll explain the data behind each in Section 3.

### View 4 — RPi 5 Deployment View

[show view4-deployment.png]

The deployment view shows which software runs on which hardware and the deploy path from macOS dev machines to the Raspberry Pi 5.

One operational constraint worth highlighting: AGC — Auto Gain Control — must be disabled on every RPi boot via alsamixer. If it's on, the signal gain fluctuates, and Amplitude and Beat Error measurements become unreliable. This is not a code fix; it's a boot-time configuration requirement.

---

## Section 3 — Risk → Experiment → Architecture Decision (7 min)

Now the core of our M2 story. Three cases, each following the same structure: here was the risk, here is what we measured, here is the architecture decision that evidence produced.

---

### Case 1: TR-02/03 — Single-Core Saturation → ADR-001

**The Risk**

Our highest-rated risk going into M2: the RPi 5 pipeline, running single-threaded, would saturate one core and fail to meet the 21ms exec deadline at scale.

**The Experiment**

EXP-02. We instrumented the pipeline with per-frame CSV logging — wait_ms (queue drain time), exec_ms (DSP computation time), deadline miss rate, and backlog percentage.

[point to data table]

RPi baseline: cpu2 at 91%, other cores idle. 43% of frames exceeded the deadline. Temperature at 85°C with active thermal throttling.

macOS baseline first: wait_ms at 420ms. Backlog at 47% — the queue accumulates faster than it drains. exec_ms was only 0.57ms — the DSP computation itself is fast. The bottleneck is the Qt event loop coupling, not the algorithm.

We applied T2 — moved DSP to a dedicated worker thread. wait_ms dropped from 420ms to 0.013ms. That's a ×32,000 reduction. Backlog went to zero.

**The Architecture Decision: ADR-001**

[reference references/adr/ADR-001-t2-dsp-offload-thread.md]

We will separate AudioCapture from DSP. AudioCapture writes PCM into a ring buffer and returns immediately. DSPWorker reads from that buffer on a separate thread. Measurement results are emitted via QueuedConnection to the UI thread.

The rejected alternative was T1 — SCHED_RR scheduling priority only. That improves jitter but doesn't reduce exec_ms or address the single-core saturation. T1 may supplement T2 on RPi, but it cannot replace it.

Status: Accepted. macOS validated. RPi R5 scheduled 06/23.

---

### Case 2: TR-04 — Rendering Bottleneck in Exec Path → ADR-002

**The Risk**

Even with T2 in place, `replot()` is still called on every beat event for every tab. On RPi baseline, plot alone consumed 16ms — 79% of the 21ms exec budget. N tabs means N × plot_ms.

**The Experiment**

After applying T2, we measured replot_count per beat with no guard: 8.22 per beat. Then we applied the R1 `isVisible()` guard.

[point to R3/R4 results]

Scenario A — one active tab: 8.22 drops to 2.08. That's 75% reduction.
Scenario B — tab switch: drops to 1.20. That's 85% reduction.

The tab switch experience: when you switch to a tab that has been non-visible, we fire a `QTimer::singleShot(0)` in `showEvent()` to deliver a catch-up frame immediately. The stale display duration is less than one beat period — 21ms at 28,800 BPH — which is imperceptible.

**The Architecture Decision: ADR-002**

[reference references/adr/ADR-002-r1-lazy-rendering.md]

One line per tab: `if (!isVisible()) return;` in `updateData()`. One `showEvent()` override per tab. Zero changes to Domain or below.

The expected RPi impact: if `plot` consumed 16ms at 8.22 replot/beat, at 1.20/beat the proportional saving is approximately 14ms.

The trade-off we accepted: non-visible tabs show a stale frame on switch. The catch-up mechanism makes this imperceptible in practice.

Status: Accepted. macOS validated. RPi confirmation via EXP-02 R5 on 06/23.

---

### Case 3: TR-01 — RPi Sample Rate (Decision Pending)

**The Risk**

RPi 5 may not sustain 96kHz audio capture without dropping blocks while the Qt GUI runs concurrently. This sets the sample rate ceiling — which directly affects Beat Error resolution (10.4µs per sample at 96kHz).

**Experiment Status**

EXP-01. macOS result: 96kHz sustained, zero dropped blocks. RPi measurement scheduled 06/23.

We are not making this architecture decision before we have RPi data. ADR-003 will be issued after EXP-01 RPi results are confirmed.

If 96kHz is not achievable: fallback to 48kHz, which degrades Beat Error resolution to 20.8µs per sample but remains within WeiShi comparison tolerance.

---

## Section 4 — Remaining Issues (2 min)

[point to open experiments table]

Three experiments are still running on RPi. EXP-01 and EXP-02 R5 are both on 06/23 — tomorrow. EXP-02 R5 is our go/no-go gate for Phase A. If T2+R1 resolves deadline miss on RPi, we proceed to graph implementation. If not, EXP-02 R6 applies T1 SCHED_RR on top on 06/24.

EXP-03, the filter parameter sweep, runs 06/25 and feeds directly into Phase A task A-02 — the filter cutoffs aren't finalized until that result is in.

EXP-05, the 11-tab rendering FPS on RPi, is our validation that R1 is sufficient under full load. If it's not, we fall back to R2 (timer-decoupled rendering), and ADR-002 would be superseded.

[point to unresolved concerns]

Three critical concerns: T2+R1 not yet on RPi, thermal throttle mitigation not yet applied, and filter cutoffs not determined. All three resolve by 06/25 if experiments run on schedule.

---

## Section 5 — Schedule (2 min)

[point to timeline]

06/23: RPi experiments + M2 feedback review.
06/24: Address M2 feedback + Phase A finalized — core pipeline complete.
06/25 onward: Phase B HIGH-priority graphs — Trace Display, Vario Display, Beat Error Display, Pause/Rewind.
06/29: Phase C MEDIUM graphs + RPi integration.
06/30: Demo rehearsal.
07/01: Final demo.

The demo survives on Phase A plus Phase B. Phase C strengthens it. Phase D is time-permitting.

One quality gate worth emphasizing: extensibility. New graph tab requires ≤ 3 file changes, zero Domain changes. That gate is already satisfied by design — confirmed by the 4-layer structure and IGraphTab interface. Every new tab we add between now and M3 is a live demonstration of that architectural decision working in practice.

---

## Closing (30 sec)

To summarize M2 in one sentence: we ran the experiment that confirmed a structural failure, traced the root cause, made two architecture decisions with data to back them, and validated both on macOS. The remaining work is hardware confirmation and graph implementation.

The architecture is ready. The pipeline is ready. The experiments running this week will tell us whether the target hardware agrees.

Thank you.

---

## Q&A Prep

| Expected Question | Key Points |
|-------------------|-----------|
| Why macOS validation only? | RPi has one device — EXP sequenced by dependency. macOS establishes structural correctness; RPi confirms quantitative targets. R5 is tomorrow. |
| What if T2+R1 doesn't fix RPi? | T1 (SCHED_RR) is the next lever (R6 on 06/24). If thermal throttle persists, active cooling or clock pinning. We have a fallback ladder. |
| How do you know the layer boundary doesn't add latency? | EXP-02 R2 measured exec_ms before and after the layer boundary. Overhead: < 0.1ms. Less than 0.5% of the exec budget. |
| Why not R3 (Double-Buffer Async Rendering)? | QPixmap is UI-thread-only in Qt. R3 requires off-screen render workers — significant redesign risk inside M2 deadline. Deferred to M3 review. |
| BPH coverage only 28,800? | Time-boxed decision. EXP-03 extends filter parameters to other BPH. Full range is M3 stretch goal. Accuracy at 28,800 takes priority over coverage breadth. |
