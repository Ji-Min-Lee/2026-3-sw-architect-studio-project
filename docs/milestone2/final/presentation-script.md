# Presentation Script — TimeGrapher M2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

---

## Slide Flow & Timing (target: ~20 min)

| # | Section | Est. Time |
|---|---------|:---------:|
| 1 | M1 Feedback & Improvements | 3 min |
| 2 | Our Goals | 3 min |
| 3 | Architectural Approach Overview | 7 min |
| 4 | Milestone 2 Sprint Progress | 5 min |
| 5 | Remaining Schedule | 2 min |

---

## Opening (30 sec)

Hello, we are Team 3 — Blue Sky.

Our M2 story is structured around goals. We started from what we need to achieve — delivery, accuracy, usability — and built the architecture to serve each of those goals with evidence.

Today we'll cover: what we fixed from M1, the goals driving our architecture, the views we chose and why, the sprint progress over the past two weeks, and the remaining schedule.

---

## Section 1 — M1 Feedback & Improvements (3 min)

Let's start with M1 feedback directly.

[point to feedback table]

The critical points were: no owner or date per task, experiments not tracked as tasks, no README, architecture diagrams too detailed or unlabeled, and tactics mixed into the QA document.

We've addressed all five.

The most important structural fix: in M1, our QA document described solutions, not problems. A QA requirement should express what the system must achieve — not how. We moved tactics to the Architectural Approaches document and rewrote QAs in problem-only language. Numbers that were "provisional" are now confirmed by EXP-02.

Every experiment now maps to a Risk ID, and every task has an owner and date on our GitHub board.

---

## Section 2 — Our Goals (3 min)

[point to goal map table]

We organized our goals into three categories — on-schedule delivery, accuracy, and usability.

For on-schedule delivery: we need to shorten the dev machine to RPi deploy cycle, and we need to apply architecture decisions quickly enough to stay on schedule. These are Deployability and Modifiability concerns.

For accuracy: we have four QAs, in priority order. First, Accuracy itself — computed Rate, Amplitude, and Beat Error must match the Witschi reference instrument within tolerance. This is the governing criterion we validate against. Second, Real-Time Performance — the pipeline must process each beat within the 21ms window or we drop a beat and lose Rate. Third, Latency — capture-to-detect latency must be low enough that timestamps are correct, because Beat Error is computed from timestamp deltas. Fourth, Reliability — the system must produce correct results even under noise.

For usability: inputs the system cannot handle — signal too weak, device not connected — must be clearly communicated to the user.

[point to QA priority table]

Our QA priority order is: Accuracy, Real-Time, Latency, Reliability, Modifiability, Usability.

[point to QA dependency tree]

Now here is the key reframe from M1. Measurement Accuracy is not one QA among equals — and in M2 we made it explicit as its own measurable QA. Rate, Amplitude, Beat Error must match the Witschi reference. Real-Time Performance and Low Latency are mathematical prerequisites to getting that number right — miss the deadline or shift the timestamp, and the computed value is simply wrong. Reliability handles the false trigger case. Modifiability is the execution enabler — without a clean layer structure, architecture changes cannot be applied fast enough to stay on schedule.

This priority order is what drove every architecture decision we made in M2.

---

## Section 3 — Architectural Approach Overview (7 min)

[point to view selection table]

We chose four views, each tied to a specific goal and a specific reader. The deployment view addresses deployability. The two module views address modifiability. The C&C thread model addresses accuracy.

Let me walk through each.

---

### View 1 — Deployment View (Allocation)

[show view4-deployment.png]

This is the allocation view — which software runs on which hardware and how we deploy.

The dev machine is where all code work happens: write, build, unit test, run. Once validated, push to GitHub. The RPi pulls from GitHub, builds, and runs experiments.

This separation is what shortens the cycle. We don't push to RPi repeatedly during development. RPi is used only for hardware experiments and the final demo. No re-imaging, no file transfers, no environment drift.

One constraint worth calling out: AGC — Auto Gain Control — must be disabled on every RPi boot via alsamixer. If AGC is on, the signal gain fluctuates, and Amplitude and Beat Error become unreliable. This is not a code fix. It's a boot-time configuration requirement, and it's documented here as an operational constraint.

---

### View 2 — 4-Layer Allowed-to-Use (Module View)

[show view1-layered-module.png]

Four layers. One rule: dependencies flow downward only.

Acquisition captures PCM from audio hardware. Signal Processing filters and detects beat events. Domain computes Rate, Amplitude, Beat Error, BPH. Presentation renders the 11 graph tabs.

The key property: Presentation depends on Domain. Domain does not depend on Presentation. If you add a new graph tab, you add one class to Presentation. You touch nothing in Domain, Signal Processing, or Acquisition.

This is our modifiability guarantee enforced structurally, not by convention. It's also how two people can build different tabs in parallel without blocking each other.

---

### View 3 — Graph Tab Decomposition (Module — Zoom)

[show view2-decomposition.png]

This zooms into Presentation. `GraphTabManager` manages all tabs through the `IGraphTab` interface. Each tab is an independent class — developed and tested in isolation.

The extension rule is: implement `IGraphTab`, register in `GraphTabManager`, done. No other files are touched.

Evidence that this works: all 11 graph tabs are implemented as of today. Each team member owned specific tabs without conflict.

---

### View 4 — DSP Pipeline Thread Model (C&C View)

[show view3-thread-model.png]

This is the runtime view, and this is where the accuracy story lives.

Two threads. The Audio Thread runs AudioCapture and DSPWorker. The UI Thread handles only rendering. Between them: a thread-safe ring buffer and Qt QueuedConnection for asynchronous result delivery.

You'll see two decisions labeled directly on this diagram — ADR-001 and ADR-002. Both were forced by experiment evidence.

[point to evidence table]

ADR-001, T2: AudioCapture writes PCM into the ring buffer and returns immediately. DSPWorker reads from the buffer on a separate thread and does all signal processing there. The experiment result: wait_ms dropped from 420ms to 0.013ms — that's a factor of 32,000. Backlog went from 47% to zero.

Why this matters for accuracy: with the original single-threaded design, the Qt event loop queuing caused a 420ms backlog. Frames were not processed in time. 43% of frames on RPi exceeded the 21ms deadline. This is a structural failure — not an algorithm problem. You cannot fix it by tuning DSP parameters. It requires structural separation.

ADR-002, R1: In the original design, `replot()` is called on every beat event for every tab, even tabs not currently visible. On RPi, rendering one plot consumed 16ms — 79% of the 21ms exec budget. With 11 tabs, this does not scale.

The fix: one line per tab — `if (!isVisible()) return;` in `updateData()`. Non-visible tabs skip `replot()`. When a tab becomes visible, `showEvent()` fires a catch-up frame via `QTimer::singleShot(0)`.

Experiment result: replot count per beat dropped from 8.22 to 1.20 — an 85% reduction. Expected RPi saving: approximately 14ms.

Trade-off we explicitly accepted: a non-visible tab shows a stale frame on switch. The catch-up mechanism makes this imperceptible — less than one beat period, which is 21ms at 28,800 BPH.

One case pending: TR-01, the RPi sample rate ceiling. EXP-01 confirmed 96kHz on macOS — zero dropped blocks. We have not yet run EXP-01 on RPi. ADR-003 — sample rate decision — is deferred until we have RPi data on 06/23. The fallback is 48kHz, which degrades Beat Error resolution from 10.4µs to 20.8µs per sample but remains within WeiShi comparison tolerance.

---

## Section 4 — Milestone 2 Sprint Progress (5 min)

[point to team structure diagram]

We have two parallel teams under one Product Owner.

Jimin Lee is the Product Owner — responsible for requirements prioritization and sprint goal approval.

The Experiment Team — Dong Ho Shin, Gyeongjin Shin, Kyudae Bahn, and Taejoon Song — owns all experiments: EXP-01 through EXP-05, risk validation, and ADR evidence collection. Scrum Master is Dong Ho Shin.

The Development Team — Hung Son Tong, Jimin Lee, and Sungho Shin — owns architecture implementation, graph tab development, and the DSP pipeline. Scrum Master is Sungho Shin.

The Architecture Committee — PO and both Scrum Masters — meets at each Sprint Planning to apply ADD Steps 2–4 and align on architecture decisions before development begins.

[point to sprint timeline]

We ran four sprints across weeks 2 and 3. Let me walk through each.

---

### W2 Sprint 1 — Modifiability (6/9–6/10)

The goal of this sprint was to make parallel tab development possible.

We defined the 4-layer structure, created the `IGraphTab` interface and `GraphTabManager`, and started parallel implementation of all 11 graph tabs.

One risk we addressed here: domain knowledge gap. Understanding watch measurement mathematics — Beat Error, Amplitude, Rate — requires specialized knowledge the team didn't have on day one.

We used AI-assisted unit test generation to bootstrap structural validation. The AI could generate tests for the interface contracts and computation logic without the team needing deep domain expertise first. This let us validate the graph tab structure early without slowing down implementation.

By the end of sprint 1, all 11 graph tabs were implemented. [point to GitHub board] You can see the issue tickets and status here.

---

### W2 Sprint 2 — Deployability (6/11–6/12)

With the tabs built, the next bottleneck was the experiment feedback cycle. Manually copying files to RPi, rebuilding, running, and collecting results by hand was slow.

This sprint delivered the experiment runner scripts — `run_exp.sh` and `analyze_log.py` — and a structured CSV logger embedded in the DSP pipeline.

The result: the deploy flow from the deployment view is now operational. `git pull` on RPi, build, run experiment, collect CSV, analyze. The cycle is short enough that we can run multiple experiment rounds in a single day.

---

### W3 Sprint 1 — Performance: Real-Time / Latency (6/16–6/17)

This is the sprint that produced our core architecture decisions.

[point to EXP-02 results in the C&C view table]

We ran EXP-02, measured the baseline failure — 43% deadline miss on RPi, 420ms backlog on macOS — traced the root cause structurally, applied T2 and R1, and validated both on macOS.

ADR-001 and ADR-002 both required trade-off documentation. For ADR-001, the rejected alternative was SCHED_RR scheduling priority alone — T1 improves jitter but cannot address single-core saturation. For ADR-002, the rejected alternative was double-buffered async rendering — QPixmap is UI-thread-only in Qt, requiring a significant redesign that we could not deliver within the sprint.

[point to open experiments table]

Three things remain from this sprint: EXP-01 and EXP-02 R5 on RPi, scheduled 06/23. These are our go/no-go gates for Phase A.

---

### W3 Sprint 2 — Reliability: Noise (6/18–6/19, Planned)

The next sprint focuses on signal quality under noise.

EXP-03 will sweep LP/HP filter parameters and feed directly into ADR-003. EXP-05 will confirm that R1 is sufficient under 11-tab full load on RPi, or trigger the fallback to R2 timer-decoupled rendering.

The ADR count for this sprint depends on results — we expect at least one ADR from EXP-03, and one from EXP-05 if R1 proves insufficient.

---

## Section 5 — Remaining Schedule (2 min)

[point to upcoming sprint table]

06/23: RPi experiments — EXP-01 sample rate, EXP-02 R5 T2+R1 confirmation. This is also M2 feedback day.
06/24–06/25: EXP-02 R6 thermal mitigation + EXP-03 filter sweep + ADR-003 finalized.
06/26: EXP-05 11-tab FPS + Usability sprint begins.
06/26–06/28: Radar Chart + Diagnosis/Classification + buffer.
06/29–06/30: RPi integration + WeiShi accuracy validation + demo rehearsal.
07/01: Final demo.

[point to phase priority table]

All 11 graph tabs are done. The critical path is: RPi experiments → WeiShi accuracy validation → demo. Phase A — core capture-to-DSP pipeline on RPi — is the gate for everything else.

One extensibility note worth emphasizing: adding a new graph tab requires at most 3 file changes and zero Domain changes. This gate is already satisfied by design. Every tab we add between now and M3 is a live demonstration of that architectural decision working in practice.

---

## Closing (30 sec)

To summarize M2 in two sentences:

We built the architecture to serve measurable goals, ran the experiment that confirmed a structural failure, made two decisions with data behind them, and validated both.

The remaining work is hardware confirmation on RPi and the WeiShi accuracy validation. If T2+R1 holds on RPi — which we expect — the demo path is clear.

Thank you.

---

## Q&A Prep

| Expected Question | Key Points |
|-------------------|-----------|
| Why macOS validation only for T2+R1? | RPi has one device — EXP sequenced by dependency. macOS establishes structural correctness; RPi confirms quantitative targets. EXP-02 R5 is 06/23. |
| What if T2+R1 doesn't fix RPi? | T1 (SCHED_RR) is the next lever (EXP-02 R6, 06/24). If thermal throttle persists, active cooling or clock pinning. We have a fallback ladder. |
| How do you know the layer boundary doesn't add latency? | EXP-02 R2 measured exec_ms before and after the layer boundary. Overhead < 0.1ms — less than 0.5% of the exec budget. |
| Why not R3 (double-buffer async rendering)? | QPixmap is UI-thread-only in Qt. R3 requires off-screen render workers — significant redesign risk inside M2 deadline. Deferred to M3 review if EXP-05 shows R1 insufficient. |
| BPH coverage only 28,800? | Time-boxed decision. EXP-03 extends filter parameters to other BPH. Full range is M3 stretch goal. Accuracy at 28,800 takes priority over coverage breadth. |
| Why AI-generated unit tests? | Domain knowledge gap was a tracked risk. AI generated structural tests without requiring deep watch-measurement expertise — allowed parallel tab validation on schedule. |
