# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

"In the topics I'm covering today, the second is correctness and the third is extensibility."

"First, correctness. The question is simple: when a measurement is ready, does every tab get it? Every single time?"

"The naive approach is to have `MeasurementEngine` call each tab directly. But then it has to know about all 14 tabs. Add one more tab, and you're modifying the engine. That's tight coupling."

"Instead, we used Qt Signal-Slot as an Observer pattern. The engine emits one signal — `measurementReady`. Every tab subscribes. The engine doesn't know who's listening."

"One more thing — DSP runs on a separate thread, T2, but tabs render on the Qt main thread. Qt's `QueuedConnection` takes care of the cross-thread handoff automatically. No manual locking."

> **📌 Q&A**
>
> **Q. Why Qt Signal-Slot instead of a custom Observer?**
> Qt Signal-Slot is built into the framework we're already using. It gives us thread-safe delivery via `QueuedConnection` for free. A custom Observer would need manual locking for cross-thread safety — more code, more risk.
>
> **Q. What if a tab is not visible — does it still receive the signal?**
> Yes, the signal is always delivered. But each tab has an `isVisible()` guard inside `onMeasurement()` — if the tab isn't visible, it skips the replot. When it becomes visible again, `showEvent()` triggers a catch-up render. So no data is lost, but rendering is lazy.
>
> **Q. How do you guarantee all 14 tabs are actually subscribed?**
> `MainWindow` calls `registerTab()` for every tab at startup, and `SessionController` wires the signal-slot connection in one block. If a tab isn't registered, it simply won't render — it won't cause a crash or a missed update elsewhere.

---

## Transition

"So that's correctness. Now — how did we keep the system extensible as we added more and more tabs?"

---

## 2-C. Extensibility

"We ended up with 14 tabs. We started with 11, added 2 more, then 1 bonus. And none of these additions broke anything below. Three design decisions made that possible."

### Layer

"First, a 4-layer architecture. Acquisition at the bottom, then Signal Processing, Domain, and Presentation at the top. The rule is simple — dependencies only go downward."

"Adding a new tab means touching only the Presentation layer. Three files or less. We verified this across all three rounds of additions. The Domain layer was never touched."

> **📌 Q&A**
>
> **Q. Why is `MeasurementEngine` in Signal Processing, not Domain?**
> It's owned and driven by `DSPWorker` inside the T2 thread pipeline. It calls `processBlock()` as part of the DSP loop — it's not a stand-alone domain service. Pure domain objects like `WatchMath` and `WatchDiagnostics` have no dependency on the DSP loop, so they stay in Domain.
>
> **Q. What does "3 files or less" actually mean?**
> It means the new tab's `.h` and `.cpp` (counted as 1 pair), plus one change to `MainWindow` to register it. That's the maximum for a standard tab. `RadarChartTab` needed 3 because it reads per-position data from `SequenceTab` rather than through `measurementReady` — that's the one exception.
>
> **Q. What's "UI Coordinator" — is it a 5th layer?**
> It's not a domain layer. `MainWindow` and `SessionController` are wiring code — they set up connections at startup but own no business logic. We called it out separately in the diagram to avoid confusion, but it doesn't violate the 4-layer rule.

### Interface — IAudioSource

"Second, `IAudioSource`. We have three audio sources — microphone, file, simulation. Before this interface, each had its own connection logic scattered around. After, they all go through one interface, one `connect()` block in `SessionController`. One place to change if anything needs to change."

> **📌 Q&A**
>
> **Q. Are there plans to add more audio sources?**
> Not currently. The benefit of `IAudioSource` isn't extensibility for new sources — it's that the existing three sources are unified in one place, making the code easier to read and maintain.
>
> **Q. What does the `connect()` block actually do?**
> It wires the Qt signals from the active audio source to `DSPWorker`. When the session starts, `SessionController` casts the active source to `IAudioSource` and connects its audio data signal to the DSP input — regardless of which concrete source is in use.

### Entity / Value Object

"Third, the `Measurement` struct. It's made of three Value Objects — `WatchMetrics`, `SignalFrame`, and `AcousticEvent` — all immutable once created. Tabs get it as read-only. They physically can't mutate the result. So correctness is enforced by the data structure itself, not by trust."

> **📌 Q&A**
>
> **Q. What if a tab needs to store or accumulate measurement data over time?**
> Each tab manages its own local state for that. For example, `SequenceTab` accumulates readings per position in its own member variables. `Measurement` itself is a snapshot per DSP cycle — tabs are responsible for what they do with it after `onMeasurement()` is called.
>
> **Q. Why three separate Value Objects instead of one flat struct?**
> Each VO has a different lifecycle and owner. `AcousticEvent` is produced by beat detection, `SignalFrame` by the audio capture, `WatchMetrics` by the DSP math. Keeping them separate makes the data flow clearer and lets each layer pass only what it needs.

---

## 2-D. AI-Assisted Unit Test

"Now, one risk we had going in — most of us don't have deep watch-measurement domain knowledge. Rate, amplitude, beat error — it's hard to write tests for things you don't fully understand yet."

"So we used AI to generate unit tests. They verify that each tab correctly receives measurements and doesn't mutate them. It's not a replacement for domain knowledge — that takes time to build. But it let us verify structural correctness right now, while we're still learning."

> **📌 Q&A**
>
> **Q. What AI tool did you use, and what exactly does it test?**
> We used Claude to generate the tests. They test that `onMeasurement()` is called with a valid `Measurement`, that the tab doesn't throw, and that the measurement data isn't mutated after the call. It's interface compliance testing, not domain logic testing.
>
> **Q. How many tests were generated?**
> 14 tabs, each with a set of structural tests. The same test template was applied across all tabs — baseline 11 in W2 S1, then the 3 additional tabs in W2 S2 and W3 S1.
>
> **Q. What's the plan for real domain logic tests?**
> That's part of the ongoing work. As the team builds domain knowledge — especially through accuracy validation in W4 S4 — we'll add tests for the actual watch metrics computation in `WatchMath` and `WatchDiagnostics`.

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

> **📌 Q&A**
>
> **Q. What exactly is FG scheduling latency?**
> FG stands for foreground — it's the latency between when the DSP thread finishes processing and when the Qt main thread actually starts rendering. We measured `fg_wait` avg at 60.1 ms, with 84% of frames exceeding the real-time deadline. The T2 offload fixed the DSP side, but the rendering side is still the bottleneck.
>
> **Q. What happens if accuracy validation fails in W4 S4?**
> That would be a critical issue for the demo. The buffer week (6/29–6/30) is there partly for this — if W4 S4 reveals accuracy problems, we'd use the buffer to investigate and decide whether it's a filter tuning issue or a deeper calibration problem.

---

## 3-C. M3 Schedule

"Four sprints left. Sprint 1 — FG fix. Sprint 2 — filter tuning. Sprint 3 — rendering benchmark. Sprint 4 — accuracy validation and full RPi run. Then a buffer week, and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
