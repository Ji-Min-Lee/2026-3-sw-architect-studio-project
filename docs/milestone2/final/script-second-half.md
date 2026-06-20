# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~13 min

---

## 2-B. Correctness: Observer Pattern

"The second quality goal is correctness. Specifically — when a measurement result is ready, every tab must receive it. No tab gets skipped, no tab gets a different result."

"The problem with a naive approach is direct coupling. If `MeasurementEngine` calls each tab directly, it has to know about all 14 tabs. And if you add a new tab, you have to go back and modify `MeasurementEngine`."

"Our decision: Qt Signal-Slot as an Observer pattern. `MeasurementEngine` emits one signal — `measurementReady` — and every tab subscribes. `MeasurementEngine` has zero knowledge of any tab."

"There's also a thread safety concern. DSP runs on T2, tabs render on the Qt main thread. Qt's `QueuedConnection` handles the cross-thread delivery safely — no manual locking needed."

"The result: measurement logic and display logic are fully decoupled. Adding a new tab requires zero changes to `MeasurementEngine`. Correctness is guaranteed by the structure."

---

## Transition from 2-B

"So far we've seen how we solved **latency** with thread separation, and **correctness** with the Observer pattern. Now the third quality goal: **extensibility**."

---

## 2-C. Extensibility

"We now have **14** display tabs — 11 from the core requirements, 2 more added when we aligned with the project-plan screens, and 1 bonus Radar chart. Each one shows different watch metrics. The question is — how did we keep adding tabs **without breaking anything** that already works?"

"Three design decisions made that possible."

---

### Layer — 4-Layer Architecture

"First, layers. We have four layers: Acquisition, Signal Processing, Domain, and Presentation. The design rule is: dependencies only go **downward**."

"Adding a new tab means touching **only** the Presentation layer. Three files or less. Nothing below changes. We verified this across all three batches — 11, then 13, then 14. Each time, the Domain layer stayed untouched."

"But we also did an honest check using a Dependency Structure Matrix — tracing actual `#include` statements in the code. And we found one violation: `DSPWorker` in Signal Processing holds a reference to `MeasurementEngine` in Domain. That's an upward dependency."

"We've identified the fix — `SessionController` should own `MeasurementEngine` and pass it down, so `DSPWorker` never touches Domain directly. This is scheduled for W4."

---

### Interface — IAudioSource

"Second, the audio source. We have three sources — microphone, file playback, and simulation. Before `IAudioSource`, each one had its own `connect()` block scattered across multiple files."

"After — we introduced `IAudioSource` as a common interface. All three sources plug into one single `connect()` block in `SessionController`. No duplication."

"If we need to change how audio wiring works, we touch **one place**. That's the real benefit — not adding new sources, but making the existing code easier to read and modify."

---

### Entity / Value Object

"Third, the `Measurement` struct. This is what `MeasurementEngine` emits via `measurementReady` signal after each DSP cycle."

"It's made of three Value Objects — `WatchMetrics`, `SignalFrame`, and `AcousticEvent`. All immutable after creation."

"Tabs receive it as read-only. They **cannot** modify measurement results. So correctness is guaranteed by structure, not by trust."

"And because these VOs live in the Domain layer, replacing or adding any Presentation component has zero impact on the domain logic."

---

## 2-D. Risk — AI-Assisted Unit Test

"One real risk we had: most team members don't have deep watch-measurement domain knowledge. Rate, Amplitude, Beat Error — hard to verify by reading code alone."

"So we used AI-generated unit tests. They validate that each tab correctly implements the interface — that `updateData()` receives the right data and doesn't mutate it."

"No domain expert needed. The 11 baseline tabs passed in W2 Sprint 1, and the same approach covered the 3 additional tabs added later."

"This was our mitigation for NTR-07 — using automation to cover a knowledge gap."

---

## Wrap-up (handoff to Schedule section)

"So to summarize this section: latency solved by threads, correctness by Observer, extensibility by layers plus interfaces plus immutable VOs. And AI-assisted tests closed the domain knowledge gap."

"Now let's look at where we are on the schedule, and what's left before the final demo."

---

## 3-A. Sprint Summary (W2–W3)

"Here's a quick recap of the last two weeks."

"Week 2 Sprint 1 — we built the 4-layer structure and completed the first 11 tabs. Sprint 2 — we set up the RPi deployment workflow."

"Week 3 Sprint 1 — EXP-02 completed, ADR-001 and 002 accepted. Sprint 2 — we ran noise experiments, still in progress."

---

## 3-B. Experiment Status

"Five experiments total. Three are done."

"EXP-01 confirmed 96kHz is sustainable on RPi — zero dropped blocks. EXP-02 E2-5/6 confirmed DSP latency is 2.05ms on RPi — well under the 10ms target."

"But E2-7 showed a problem. Foreground scheduling latency averages 60ms — 84% of cycles miss the deadline. This is our critical open risk, TR-10. We have E2-8 scheduled for next week to fix it."

"EXP-03 and EXP-05 are still running — targets are June 25 and 26."

---

## 3-C. M2 → Final Schedule

"From here to the final demo on July 1st — about ten days, five sprints."

"Next week, W4: fix the FG scheduling issue, run filter sweep and rendering FPS experiments, and build the Radar Chart and Diagnosis features."

"W5 Sprint 1 — RPi integration, accuracy validation against WeiShi, and demo rehearsal."

"The critical path is: fix RPi scheduling → validate accuracy → demo."

"Before M3 we have six quality gates. Three are already green — real-time block drops, DSP latency, and extensibility. Three are still pending — FG latency, rendering under 14-tab load, and core accuracy against WeiShi. Those drive the W4–W5 work."

"That's our plan. Thank you."
