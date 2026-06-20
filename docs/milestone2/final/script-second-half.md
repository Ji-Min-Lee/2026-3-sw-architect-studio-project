# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

"Today I'm covering the second and third quality goals — correctness and extensibility."

"First, correctness. The question is simple: when a measurement is ready, does every tab get it? Every single time?"

"The naive approach is to have `MeasurementEngine` call each tab directly. But then it has to know about all 14 tabs. Add one more tab, and you're modifying the engine. That's tight coupling."

"Instead, we used Qt Signal-Slot as an Observer pattern. The engine emits one signal — `measurementReady`. Every tab subscribes. The engine doesn't know who's listening."

"One more thing — DSP runs on a separate thread, T2, but tabs render on the Qt main thread. Qt's `QueuedConnection` takes care of the cross-thread handoff automatically. No manual locking."

---

## Transition

"So that's correctness. Now — how did we keep the system extensible as we added more and more tabs?"

---

## 2-C. Extensibility

"We ended up with 14 tabs. We started with 11, added 2 more, then 1 bonus. And none of these additions broke anything below. Three design decisions made that possible."

### Layer

"First, a 4-layer architecture. Acquisition at the bottom, then Signal Processing, Domain, and Presentation at the top. The rule is simple — dependencies only go downward."

"Adding a new tab means touching only the Presentation layer. Three files or less. We verified this across all three rounds of additions. The Domain layer was never touched."

### Interface — IAudioSource

"Second, `IAudioSource`. We have three audio sources — microphone, file, simulation. Before this interface, each had its own connection logic scattered around. After, they all go through one interface, one `connect()` block in `SessionController`. One place to change if anything needs to change."

### Entity / Value Object

"Third, the `Measurement` struct. It's made of three Value Objects — `WatchMetrics`, `SignalFrame`, and `AcousticEvent` — all immutable once created. Tabs get it as read-only. They physically can't mutate the result. So correctness is enforced by the data structure itself, not by trust."

---

## 2-D. AI-Assisted Unit Test

"Now, one risk we had going in — most of us don't have deep watch-measurement domain knowledge. Rate, amplitude, beat error — it's hard to write tests for things you don't fully understand yet."

"So we used AI to generate unit tests. They verify that each tab correctly receives measurements and doesn't mutate them. It's not a replacement for domain knowledge — that takes time to build. But it let us verify structural correctness right now, while we're still learning."

---

## Wrap-up

"Quick summary before we move to the schedule. Latency — solved with thread separation. Correctness — Observer pattern. Extensibility — layers, interface, and immutable value objects. And the domain knowledge gap — covered with AI-generated tests."

---

## 3-A. What We Did in M2

"M2 had a clear arc. We built, we measured, we found a problem, and we responded."

"We started with the 4-layer structure and 11 baseline tabs. Then ran experiments — EXP-01 confirmed our filter approach, EXP-02 confirmed T2 works on RPi. But EXP-02 also revealed something unexpected: FG scheduling latency is still over budget. That became our biggest open risk."

"In parallel, we shipped AI Step 1 — rule-based diagnosis — and AI Step 2 — an LLM explainer running locally on RPi. And completed the full architecture refactor."

---

## 3-B. Remaining Risks

"Going into the final sprint, we have four open risks."

"Two critical. TR-10 — FG scheduling latency, we're applying priority scheduling in W4 Sprint 1. TR-05 — end-to-end accuracy hasn't been validated with a real watch yet, that's W4 Sprint 4."

"Two medium. Filter tuning and rendering performance under 14 tabs — both targeted this week."

"The critical path: fix FG first, then experiments, then accuracy validation, then demo."

---

## 3-C. M3 Schedule

"Four sprints left. Sprint 1 — FG fix. Sprint 2 — filter tuning. Sprint 3 — rendering benchmark. Sprint 4 — accuracy validation and full RPi run. Then a buffer week, and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
