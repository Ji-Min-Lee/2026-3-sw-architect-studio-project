# Presentation Script — TimeGrapher M2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

---

## Slide Flow & Timing (target: ~20 min)

| Section | Slides | Est. Time |
|---------|--------|:---------:|
| Opening | 01 | 2 min |
| Project Plan Update | 02 | 4 min |
| Experiment Results | 03 | 7 min |
| Architecture | 04 | 5 min |
| Construction Plan | 05 | 2 min |

---

## 01. Opening

Hello, we are Team 3 — Blue Sky.

I'll start with where we are: M2 is our proof-of-concept checkpoint. In M1, we said — "here is the problem, here is our plan, and here are the experiments we will run." In M2, we executed those experiments and let the results drive our architecture.

Our thesis for today is this: the original design fails real-time constraints on Raspberry Pi 5. We identified the root cause structurally, defined two architectural decisions, validated both on macOS, and now have a clear path to the target hardware.

We have four sections: Plan Update → Experiment Results → Architecture → Construction Plan.

---

## 02. Project Plan Update

### M1 Feedback Response

Let's start by acknowledging the M1 feedback directly.

The critical points were: tasks had no owner or date, experiments were not tracked as tasks, there was no README, and the architecture diagrams were too detailed. We've addressed all of these.

[point to the table] Specifically — tasks now have owners and target dates, experiments are tracked on the Kanban board, a README links all documents together, and we've simplified diagrams to one per view with a legend.

One structural fix worth calling out: our QA document in M1 had tactics mixed in. The mentor was right to flag this — QAs should describe the problem, not the solution. We've separated those out. Tactics now live in the Architectural Approaches document.

### Revised QA Priority

This brings us to QA priority. We revised it based on both the M1 feedback and our EXP-02 results.

Modifiability is now first — not because it's the most exciting, but because it's the prerequisite for everything else. Without a clean layer structure, 11 graph tabs built in parallel by two teams causes developer blocking. You can't swap tactics cleanly either.

Real-Time and Low Latency are second — EXP-02 gave us hard numbers showing 43% deadline miss on RPi. That required structural change.

The trade-off we accepted: we narrowed BPH coverage to 28,800 BPH and de-prioritized accuracy maximization. The rationale is straightforward — a working system within 5 weeks beats a theoretically accurate system that isn't finished.

### Risk Status

[walk through the risk table] TR-02 — the real-time deadline risk — is now resolved on macOS through our experiments. RPi confirmation is next. The remaining open risks all point to RPi measurements in the next 48 hours.

---

## 03. Experiment Results

This is the core of M2.

### EXP-02: What We Found on RPi

Let me show you the RPi baseline first. Single-threaded design, 96kHz. The exec budget per frame is 21ms. We were consuming 20ms — 95% of the budget. And of that 20ms, 16ms — 79% — was plot rendering alone.

Result: 43% deadline miss. 441 out of 1,015 frames failed. Three other CPU cores were idle while cpu2 ran at 91% load. Temperature hit 85°C — thermal throttle kicked in.

The root cause is structural. This cannot be fixed by tuning DSP algorithms.

### What We Did About It

We ran five measurement runs on macOS with progressive tactics applied.

[walk through the run history table]

R1 — baseline. wait_ms averaging 420ms, backlog 47%.

R2 — we applied T2: DSP Offload Thread. wait_ms dropped from 420ms to 0.013ms — a factor of 32,000. Backlog went to zero. The DSP thread tracked the worker thread at identical FPS: 95.6.

R2b — we measured the replot baseline without any rendering guard. 8.22 replots per beat across all tabs.

R3 and R4 — we applied R1: Lazy Rendering. isVisible() guard in each tab's updateData(). Result: 75% replot reduction in Scenario A (single tab), 85% in Scenario B (tab switching). Tab switch UX is preserved through a showEvent() catch-up using QTimer::singleShot(0).

### The Causal Chain

[point to the causal chain diagram]

This is the key takeaway. EXP-02 gave us two root causes. Each root cause maps to one architectural decision. T2 addresses single-core saturation. R1 addresses rendering in the exec path. Both are validated on macOS. Next step is RPi R5.

### What's Still Open

EXP-01, 03, 04, 05 are pending — all targeting RPi measurements starting tomorrow. EXP-01 confirms our sample rate target. EXP-03 gives us the LP/HP filter cutoffs. EXP-05 measures 11-tab rendering load on RPi.

---

## 04. Architecture

### Module View

[point to diagram] Four layers: Acquisition, Signal Processing, Domain, Presentation.

The dependency rule is strict and one-directional — downward only. Presentation may not bypass Domain to touch Processing or Acquisition. This is enforced by code review and will be enforced by the compiler through header include rules.

The benefit: adding a new graph tab is a Presentation-only change. Zero changes to Domain or below. That's how we support 11 graphs being built in parallel by two teams without blocking each other.

### Runtime / C&C View

[point to diagram] Two threads. Audio Thread handles AudioCapture and — after ADD-2-01 — the DSPWorker. UI Thread handles GraphTabManager and all graph tabs.

The connector between them is Qt::QueuedConnection — cross-thread safe by Qt's design. No manual locking needed at the handoff point.

ADD-2-02 (R1) is shown here: each tab's updateData() has an isVisible() guard. Only the active tab calls replot(). On tab switch, showEvent() triggers a catch-up replot via singleShot(0).

### Deployment View

[point to diagram] Raspberry Pi 5 is the runtime target. USB sensor stand connects the watch microphone. 8-inch touchscreen on HDMI. Development happens on macOS, deployed via SSH/SCP.

One operational note: AGC must be disabled on every RPi boot. If it's on, amplitude measurements drift — the signal gain fluctuates and Beat Error calculations become unreliable.

### Design Decisions and Trade-offs

We made two independent decisions.

For rendering: R1 (Lazy Rendering) was selected. R2 (Timer-Decoupled, 20FPS) is kept as a conditional fallback — we will apply it if RPi R5 shows replot spikes above 20 per beat or exec leak after tab switch.

For threading: T2 (DSP Offload Thread) was selected. T1 (SCHED_RR + CPU Affinity) will be layered on top during RPi R6. T3 (Full Pipeline Split) is out of scope for MVP.

### Architecture Evaluation

[walk through the table] Every experiment result drove an architecture change. EXP-02 RPi baseline → structural root cause analysis → two ADD decisions. Both decisions are now validated on macOS.

The unresolved critical concern is RPi. We don't yet have T2+R1 measured on the target hardware. That's the first thing we do tomorrow.

---

## 05. Construction Plan

[walk through the schedule] Ten days from now to M3 demo.

Phase A — core pipeline — is the prerequisite for everything. Beat detection, filter chain, Rate/Amplitude/Beat Error calculations, latency instrumentation. Target: complete by 06/24.

Phase B — HIGH priority graphs — Trace, Vario, Beat Error Display. These are the demo survival set. Target: 06/27.

Phase C — MEDIUM priority graphs — the remaining seven displays. Target: 06/29.

Phase D is optional. Time permitting.

The quality gates are measurable. Core pipeline must match WeiShi within tolerance. Real-time must show no dropped blocks at 96kHz on RPi. Latency must be documented with average and worst-case numbers. Extensibility: new tab ≤ 3 file changes, zero Domain changes.

---

## Anticipated Q&A

**Q: Why didn't you run EXP-01 through EXP-05 earlier?**

A: EXP-02 on macOS was the highest-risk experiment — it confirmed the structural failure and validated the fix. We prioritized depth on that experiment over breadth across all five. RPi experiments start tomorrow.

**Q: How do you know T2+R1 will work on RPi?**

A: We don't yet — that's an open item we've named explicitly. The macOS validation gives us confidence in the approach. The RPi measurement is the next gate. If T2+R1 alone is not enough, T1 (SCHED_RR) is the next layer.

**Q: What if the architecture evaluation shows the decisions aren't sufficient?**

A: We've defined trigger criteria for R2 (Timer-Decoupled rendering) and T1 escalation. If RPi R5 shows exec leak or replot spikes above threshold, we apply R2. If thermal throttle persists, T1 pins the capture thread. The decisions are additive, not mutually exclusive.

**Q: What is the extensibility evidence?**

A: The layer structure enforces it at design level. Any new graph tab class lives in Presentation and depends only on Domain interfaces. We'll demonstrate this concretely in M3 by showing the diff for adding one graph.
