# Presentation Script — TimeGrapher M2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

---

## Slide Flow & Timing (target: ~20 min)

| Section | Slides | Est. Time |
|---------|--------|:---------:|
| Opening | 01 | 2 min |
| Project Plan Update (incl. QA Priority reframe) | 02 | 4 min |
| Experiment Results | 03 | 6 min |
| Architecture (incl. QA Tradeoff) | 04 | 6 min |
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

This brings us to QA priority.

The governing goal of this project is Measurement Accuracy. Rate, Amplitude, and Beat Error must match the WeiShi No.1000 reference device. That is the criterion we evaluate the entire architecture against.

Every other QA in the table is a prerequisite for reaching that goal — not a goal in itself.

Real-Time Performance is first because: if the pipeline misses the 21ms deadline, a beat event is dropped. One dropped event means Rate and BPH cannot be calculated correctly for that cycle. You cannot be accurate if you're missing data.

Low Latency is second because: if the time from capture to detection exceeds one beat period — about 20.8ms at 28,800 BPH — timestamps shift. Shifted timestamps corrupt Beat Error and Amplitude. These two measurements are derived entirely from timing.

Signal Quality is third. The LP/HP filter chain removes ambient noise before the detector runs. A false positive creates a phantom beat event. The math engine will compute a valid-looking number that is simply wrong.

Modifiability is the execution enabler. Without a clean layer structure, two teams building 11 graph tabs in parallel block each other. It also lets us swap filter parameters and detection thresholds without touching the DSP layer.

The trade-off we accepted: BPH coverage narrowed to 28,800 BPH. Full range is an accuracy stretch goal for M3, not a structural constraint.

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

We have four views. Each answers a different architectural question. I'll go through them in order.

### View 1 — TimeGrapher 4-Layer Allowed-to-Use View

[point to diagram] This is a layered module view. Four layers: Acquisition at the bottom, Signal Processing, Domain, Presentation at the top.

The key relation here is "allowed-to-use" — arrows only go downward. Presentation depends on Domain. Domain depends on Processing. No bypassing allowed. Presentation cannot reach into Processing or Acquisition directly.

The red "NOT ALLOWED" arrows on the left make this constraint explicit. These are the dependencies we are prohibiting — and this prohibition is what makes the architecture scalable.

The practical consequence: adding a new graph tab is a Presentation-only change. One class, zero changes to Domain or below. That's how we have two teams building 11 tabs in parallel without blocking each other.

Note the annotation on the right: ADD-2-02 (R1) — the isVisible() guard — is a Presentation-layer decision. It doesn't touch Domain.

### View 2 — Graph Tab Decomposition View

[point to diagram] This zooms into Presentation. We have GraphTabManager, which manages tabs through the IGraphTab interface. Each concrete tab — TraceDisplay, VarioDisplay, and so on — implements that interface independently.

The color coding maps directly to our build priority: green is HIGH (demo survival), yellow is MEDIUM, red is LOW or optional.

The key point here is the interface contract. IGraphTab defines updateData() and showEvent(). Those two methods are the only coupling between the manager and the tab. Each developer owns their tab class. There is no cross-tab dependency.

### View 3 — DSP Pipeline Thread Model

[point to diagram] This is the runtime view. Two threads. Audio Thread on the left, UI Thread on the right.

The two architectural decisions are labeled directly in the diagram where they operate.

ADD-2-01 (T2): DSPWorker runs on a separate thread. AudioCapture writes to the ring buffer, DSPWorker reads from it. These are decoupled. Before T2, the entire pipeline ran on one thread — wait_ms was 420ms, backlog was 47%. After T2: wait_ms dropped to 0.013ms, backlog zero.

ADD-2-02 (R1): On the UI Thread side, the active tab's updateData() has an isVisible() guard. Only the tab currently visible calls replot(). On tab switch, showEvent() triggers a catch-up via singleShot(0) so the user always sees a fresh frame.

The connectors between threads are shown with their types: ring buffer and Qt::QueuedConnection. Both are thread-safe by design. No manual locking at the handoff point.

### View 4 — Raspberry Pi 5 Hardware Deployment View

[point to diagram] The deployment view. Raspberry Pi 5 is the runtime target — ARM64, 8GB. The software stack runs inside Raspberry Pi OS.

From the left: dev machine compiles on macOS, deploys via SSH/SCP. From the right: USB sensor stand captures the watch microphone signal, feeds 96kHz PCM via ALSA to the executable. The rendered output goes HDMI to the 8-inch touchscreen. Touch events come back via USB HID.

The orange warning box is there deliberately: AGC must be disabled on every RPi boot. If auto gain control is on, the signal amplitude fluctuates and Beat Error calculations become unreliable. This is an operational constraint, not a code issue.

### QA Tradeoff Analysis

Before we walk through the design decisions, I want to explicitly connect the architecture back to our governing goal: Accuracy.

The four views we just showed are not independent choices. They all serve the same purpose.

The 4-layer structure ensures that the DSP pipeline is insulated from display code. This matters for accuracy because if rendering runs in the same execution path as beat detection — which is what the original code does — a slow replot can delay the next capture callback. You drop frames. You miss beats. Your Rate calculation is wrong.

The thread model — T2, DSP Offload — separates capture and DSP into their own thread. This ensures that the UI can never stall the measurement pipeline. Before T2, wait_ms was 420ms. After T2, it was 0.013ms. That 32,000x reduction is not a performance optimization. It is what makes accurate timestamping possible.

The isVisible() guard — R1 — reduces replot frequency by 75 to 85%. That exec budget goes back to DSP processing. The measurement engine runs with more headroom. Fewer deadline misses. More accurate data.

Two tradeoffs to name explicitly.

Modifiability vs. performance: the abstraction boundary introduces one extra function call per domain query. We measured it — less than 0.1ms, under 0.5% of the exec budget. Accepted.

Lazy Rendering vs. display freshness: a non-visible tab does not refresh. When you switch tabs, you see a stale frame for up to 21ms. We handle this with showEvent() and QTimer::singleShot(0), which delivers a catch-up frame on every tab switch. Accepted.

---

### Design Decisions and Trade-offs

Two decisions, both validated on macOS.

For rendering: R1 (Lazy Rendering) selected. R2 (Timer-Decoupled, 20FPS) kept as conditional fallback — we apply it if RPi R5 shows replot spikes above 20 per beat or exec leak after tab switch.

For threading: T2 (DSP Offload Thread) selected. T1 (SCHED_RR + CPU Affinity) will be layered on top during RPi R6.

### Architecture Evaluation

[walk through the table] Every experiment result maps to an architecture change. EXP-02 RPi baseline → structural root cause → two ADD decisions. Both validated on macOS.

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
