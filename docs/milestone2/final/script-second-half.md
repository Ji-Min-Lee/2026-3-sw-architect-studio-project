# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~13 min

---

## 2-B. Correctness: Observer Pattern

"The second quality goal we're covering today is correctness. Specifically — when a measurement result is ready, every tab must receive it. No tab gets skipped, no tab gets a different result."

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

"First, layers. The diagram shows five horizontal bands. At the bottom — **Acquisition**: microphone input, file playback, simulation, all behind a single interface. Above that — **Signal Processing**: the DSP worker and beat detector, running on a dedicated T2 thread. Then **Domain**: watch physics math and AI diagnosis. At the top — **Presentation**: the 14 display tabs. There's also a thin **UI Coordinator** band — `MainWindow` and `SessionController` — that wires the layers together at startup but owns no business logic."

"The rule is: dependencies only go **downward**. That's what the dashed arrows in the diagram show — every arrow points down, never up."

"Adding a new tab means touching **only** the Presentation layer. Three components or less. Nothing below changes. We added tabs in three rounds — first 11, then 2 more, then 1 bonus. Each time, the Domain layer stayed untouched."

---

### Interface — IAudioSource

"Second, the audio source. The diagram shows two sides — AS-IS on the left, TO-BE on the right. On the left, three separate `connect()` blocks, one per concrete worker, scattered across the code. On the right, one `SessionController` calling one interface: `IAudioSource`. All three sources plug into that single block."

"If we need to change how audio wiring works, we touch **one place**. That's the real benefit — not adding new sources, but making the existing code easier to read and modify."

---

### Entity / Value Object

"Third, the `Measurement` struct. The diagram shows `Measurement` at the center — solid border, meaning it's a concrete struct. Composed of three Value Objects with dashed borders: `WatchMetrics` holds the computed watch numbers — rate, amplitude, beat error, BPH. `SignalFrame` holds the raw PCM samples and timestamp. `AcousticEvent` holds the individual tick event timestamps."

"All three are immutable after creation. Tabs receive `Measurement` as read-only — they **cannot** modify measurement results. Correctness is guaranteed by structure."

"And because these VOs live in the Domain layer — same green color as the Domain band in the layer diagram — replacing or adding any Presentation component has zero impact on the domain logic."

---

## 2-D. Risk — AI-Assisted Unit Test

"One real risk we had: most team members don't have deep watch-measurement domain knowledge. Rate, Amplitude, Beat Error — hard to verify by reading code alone."

"So we used AI-generated unit tests. They validate that each tab correctly implements the interface — that `onMeasurement()` receives the right data and doesn't mutate it."

"No domain expert needed. The 11 baseline tabs passed in W2 Sprint 1, and the same approach covered the 3 additional tabs added later."

"This was our mitigation for NTR-07 — using automation to cover a knowledge gap."

---

## Wrap-up (handoff to Schedule section)

"So to summarize this section: latency solved by threads, correctness by Observer, extensibility by layers plus interfaces plus immutable VOs. And AI-assisted tests closed the domain knowledge gap."

"Now let's look at where we are on the schedule, and what's left before the final demo."

---

## 3-A. What We Did in Milestone 2

"Here's a quick recap of what we delivered."

"Week 2 Sprint 1 — 4-layer decomposition and all 11 baseline tabs. Sprint 2 — structured logging, RPi system metrics, full unit-test suite, and EXP-02 baseline runs started."

"Week 3 Sprint 1 — EXP-01 done, ADR-003 accepted. EXP-02 confirmed T2 works on RPi. But we found a new problem: FG scheduling latency is way over budget — that's the red item we'll talk about in a moment. We also shipped AI Step 1, rule-based watch diagnosis."

"Week 3 Sprint 2 — AI Step 2, an LLM explainer running on RPi via Ollama. And we completed the 4-layer refactor — IAudioSource and SessionController fully in place."

---

## 3-B. Remaining Risks & Open Items

"We have four open risks going into the final sprint."

"Two are critical. TR-10 — FG scheduling latency on RPi is still over budget. We'll apply priority scheduling and measure next week. TR-05 — end-to-end accuracy against a real watch hasn't been validated yet. That's W4 Sprint 4."

"Two are medium. TR-08 — filter tuning needs to be validated on a real noisy signal, targeting June 25. TR-09 — we haven't measured rendering performance under all 14 tabs on RPi yet, targeting June 26."

"The critical path is: fix FG scheduling → run filter and rendering experiments → validate accuracy → demo."

---

## 3-C. M3 Schedule

"Four sprints left. Sprint 1 — fix FG scheduling on RPi. Sprint 2 — filter tuning. Sprint 3 — 14-tab rendering benchmark. Sprint 4 — accuracy validation with a real watch and full RPi run. Then a buffer for presentation prep."

---

## Closing — Milestone 2 Wrap-up

"That's everything for Milestone 2. Thank you."
