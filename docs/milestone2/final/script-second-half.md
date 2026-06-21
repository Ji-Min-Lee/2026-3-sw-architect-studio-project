# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

"In the topics I'm covering today, the second is correctness and the third is extensibility."

"First, correctness. The question is simple: when a measurement is ready, does every tab get it? Every single time?"

"The naive approach is to have `MeasurementEngine` call each tab directly. But then it has to know about all 14 tabs. Add one more tab, and you're modifying the engine. That's tight coupling."

"Instead, we used Qt Signal-Slot as an Observer pattern. The engine emits one signal — `measurementReady`. Every tab subscribes. The engine doesn't know who's listening."

"One more thing — the Digital Signal Processing (DSP) pipeline runs on a separate background worker thread, but tabs render on the Qt main thread. Qt's `QueuedConnection` takes care of the cross-thread handoff automatically. No manual locking."

> **📋 Diagram — Graph Tab Observer Module View**
>
> The diagram has three vertical zones.
>
> - **Top — Subject**: `MeasurementEngine`. Stereotyped `<<Subject>>`, exposes one signal: `measurementReady(m Measurement)`. No dependency on any tab.
> - **Middle — Observer**: `BaseGraphTab`. Stereotyped `<<abstract, Observer>>`. Defines `onMeasurement()` as a pure virtual method, plus `replotAll()` and `mPaused`. Every tab must implement this contract.
> - **Bottom — Concrete Observers**: `TraceTab`, `VarioTab`, `BeatErrorTab`, and 11 more tabs — all inherit from `BaseGraphTab`.
> - **Right side — Value Object composition**: `Measurement` composes `WatchMetrics`, `SignalFrame`, and `AcousticEvent`. All three are immutable.
> - **`MainWindow`**: holds all tabs via `mAllTabs[*]`, registers each with `registerTab()`.
> - **Dashed arrow**: `MeasurementEngine →«notify»→ BaseGraphTab` — runtime notification. `SessionController` is intentionally omitted — it is a runtime connector, not a structural dependency.

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

> **📋 Diagram — 4-Layer Allowed-to-Use Module View**
>
> The diagram has five horizontal bands, top to bottom:
>
> - **UI Coordinator** (gray): `MainWindow`, `SessionController`, `DiagnosisDialog`. Wires layers together at startup. No domain logic.
> - **Presentation** (blue): `BaseGraphTab` interface + 14 concrete tabs + `GraphTabManager`.
> - **Domain** (green): `MeasurementEngine`, `WatchDiagnostics`, `WatchExplainer`, `Measurement` VOs. Pure domain computation.
> - **Signal Processing** (yellow): `DSPWorker` + external library `tg_process` (FilterChain, BeatDetector).
> - **Acquisition** (red): `IAudioSource` interface + `AudioWorker`, `PlaybackWorker`, `SimWorker`, `AudioRingBuffer`.
>
> All dashed `«use»` arrows point downward. The legend on the right explains arrow types and border styles: solid border = concrete class, `«interface»` tag = interface, dashed border = Value Object or external library.

> **📌 Q&A**
>
> **Q. Why is `MeasurementEngine` in Signal Processing, not Domain?**
> It's owned and driven by `DSPWorker` inside the background DSP worker thread pipeline. It calls `processBlock()` as part of the DSP loop — it's not a stand-alone domain service. Pure domain objects like `WatchMath` and `WatchDiagnostics` have no dependency on the DSP loop, so they stay in Domain.
>
> **Q. What does "3 files or less" actually mean?**
> It means the new tab's `.h` and `.cpp` (counted as 1 pair), plus one change to `MainWindow` to register it. That's the maximum for a standard tab. `RadarChartTab` needed 3 because it reads per-position data from `SequenceTab` rather than through `measurementReady` — that's the one exception.
>
> **Q. What's "UI Coordinator" — is it a 5th layer?**
> It's not a domain layer. `MainWindow` and `SessionController` are wiring code — they set up connections at startup but own no business logic. We called it out separately in the diagram to avoid confusion, but it doesn't violate the 4-layer rule.

### Interface — IAudioSource

"Second, `IAudioSource`. We have three audio sources — microphone, file, simulation. Before this interface, each had its own connection logic scattered around. After, they all go through one interface, one `connect()` block in `SessionController`. One place to change if anything needs to change."

> **📋 Diagram — Audio Source Module Uses View**
>
> The diagram is split into two sides.
>
> - **AS-IS (left)**: `AudioWorker`, `PlaybackWorker`, and `SimWorker` each have their own `connect()` block. Wiring logic is duplicated and scattered across the codebase.
> - **TO-BE (right)**: `SessionController` depends only on `IAudioSource`. All three sources implement that interface. One `connect()` block handles all of them.
> - Arrows show the direction of dependency inversion — `SessionController` depends on the abstraction, not the concrete implementations.

> **📌 Q&A**
>
> **Q. Are there plans to add more audio sources?**
> Not currently. The benefit of `IAudioSource` isn't extensibility for new sources — it's that the existing three sources are unified in one place, making the code easier to read and maintain.
>
> **Q. What does the `connect()` block actually do?**
> It wires the Qt signals from the active audio source to `DSPWorker`. When the session starts, `SessionController` casts the active source to `IAudioSource` and connects its audio data signal to the DSP input — regardless of which concrete source is in use.

### Entity / Value Object

"Third, the `Measurement` struct. It's made of three Value Objects — `WatchMetrics`, `SignalFrame`, and `AcousticEvent` — all immutable once created. Tabs get it as read-only. They physically can't mutate the result. So correctness is enforced by the data structure itself, not by trust."

> **📋 Diagram — Watch Measurement Domain Model**
>
> The diagram uses the same green color as the Domain band in the 4-layer diagram — visually confirming these objects belong to the Domain layer.
>
> - **Center — `Measurement`** (solid border): a snapshot struct created each DSP cycle. Includes a `noSignal` flag.
> - **Right — three Value Objects** (dashed borders, all immutable):
>   - `WatchMetrics`: `rate_spd`, `amplitude_deg`, `beatError_ms`, `bph` — computed watch metrics
>   - `SignalFrame`: `samples: PCMBlock`, `timestamp: uint64` — raw audio capture data
>   - `AcousticEvent`: `t1`, `t3: uint64` — individual tick event timestamps
> - A note at the bottom confirms color alignment with view1. Replacing or adding any Presentation or Signal Processing component has zero impact on this structure.

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
> 14 tabs, each with a set of structural tests. The same test template was applied across all tabs — baseline 11 in Week 2 Sprint 1, then the 3 additional tabs in Week 2 Sprint 2 and Week 3 Sprint 1.
>
> **Q. What's the plan for real domain logic tests?**
> That's part of the ongoing work. As the team builds domain knowledge — especially through accuracy validation in Week 4 Sprint 4 — we'll add tests for the actual watch metrics computation in `WatchMath` and `WatchDiagnostics`.

---

## Wrap-up

"Quick summary before we move to the schedule. Latency — solved with DSP thread separation. Correctness — Observer pattern. Extensibility — layers, interface, and immutable Value Objects. And the domain knowledge gap — covered with AI-generated tests."

---

## 3-A. What We Did in M2

"Milestone 2 had a clear arc. We built, we measured, we found a problem, and we responded."

"We started with the 4-layer structure and 11 baseline tabs. Then ran experiments — Experiment 1 confirmed our filter approach, Experiment 2 confirmed the background DSP worker thread works on Raspberry Pi."

"In parallel, we shipped AI Step 1 — rule-based diagnosis — and AI Step 2 — a Large Language Model (LLM) explainer running locally on Raspberry Pi. And completed the full architecture refactor."

---

## 3-B. Remaining Risks

"Going into the final sprint, we have three open risks."

"One critical. Technical Risk 5 — end-to-end accuracy hasn't been validated with a real watch yet, that's Week 4 Sprint 4."

"Two medium. Filter tuning and rendering performance under 14 tabs — both targeted this week."

"The critical path: filter and rendering experiments, then accuracy validation, then demo."

> **📌 Q&A**
>
> **Q. What happens if accuracy validation fails in Week 4 Sprint 4?**
> That would be a critical issue for the demo. The buffer week (6/29–6/30) is there partly for this — if Week 4 Sprint 4 reveals accuracy problems, we'd use the buffer to investigate and decide whether it's a filter tuning issue or a deeper calibration problem.

---

## 3-C. M3 Schedule

"Four sprints left. Sprint 1 — microphone auto-recovery. Sprint 2 — filter tuning. Sprint 3 — rendering benchmark. Sprint 4 — accuracy validation and full Raspberry Pi run. Then a buffer week, and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
