# Presentation Script — Section 2-C & 2-D

> Speaker: covers from **2-C. Extensibility** through **2-D. Risk** and **Section 3 Schedule**
> Estimated time: ~10 min

---

## Transition from 2-B

"So far we've seen how we solved **latency** with thread separation, and **correctness** with the Observer pattern. Now the third quality goal: **extensibility**."

---

## 2-C. Extensibility

"We have **14** display tabs. Each one shows different watch metrics. The question is — how do we add a new tab **without breaking anything** that already works?"

"We made three design decisions together to answer that."

---

### Layer — 4-Layer Architecture

"First, layers. We have four layers: Acquisition, Signal Processing, Domain, and Presentation. The design rule is: dependencies only go **downward**."

"Adding a new tab means touching **only** the Presentation layer. Three files or less. Nothing below changes. We verified this — all 14 tabs were added this way."

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

"No domain expert needed. All 14 tabs passed within Sprint 1 of Week 2."

"This was our mitigation for NTR-07 — using automation to cover a knowledge gap."

---

## Wrap-up (handoff to Schedule section)

"So to summarize this section: latency solved by threads, correctness by Observer, extensibility by layers plus interfaces plus immutable VOs. And AI-assisted tests closed the domain knowledge gap."

"Now let's look at where we are on the schedule, and what's left before the final demo."

---

## 3-A. Sprint Summary (W2–W3)

"Here's a quick recap of the last two weeks."

"Week 2 Sprint 1 — we built the 4-layer structure and completed all 14 tabs. Sprint 2 — we set up the RPi deployment workflow."

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
