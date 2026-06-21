# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

> 📂 **[화면] slide-architecture-view.md › `## 2-B. Correctness: Observer Pattern`**
> → 클래스 다이어그램 보여주기

"In the topics I'm covering today, the second is correctness and the third is extensibility."

"First, correctness. The question is simple: when a measurement is ready, does every tab get it? Every single time?"

"The naive approach is to have `MeasurementEngine` call each tab directly. But then the engine has to know about all 14 tabs by name. Add one more tab, and you're modifying the engine. That's tight coupling — every change to the tab set ripples back into the engine."

"So instead, we applied the GoF Observer pattern. `MeasurementEngine` is the Subject. It emits one signal — `measurementReady`. Every tab is a Concrete Observer, subscribed through `BaseGraphTab::onMeasurement()`. The engine emits and forgets — it has zero compile-time knowledge of who's listening."

---

> 📂 **[화면 이동] view-decomposition-graph-tab.md › `## Element Catalog`**
> → `#### BaseGraphTab`, `#### SessionController` 항목 보여주기

"This is the static structure. Three zones in the diagram."

"Top — `MeasurementEngine`, the Subject. One signal out, nothing else. No arrow to any tab at compile time."

"Middle — `BaseGraphTab`, the abstract Observer. Every tab must implement `onMeasurement()`. That's the contract."

"Bottom — the 14 concrete tabs. All inherit from `BaseGraphTab`. Adding one more requires zero changes to `MeasurementEngine`."

"The wiring happens at runtime. `SessionController` is the coordinator — it calls `Qt::connect()` at session start, linking the Subject's signal to every tab's slot via `QueuedConnection`."

---

> 📂 **[화면 이동] view-decomposition-graph-tab.md › `## Behavior`**
> → 런타임 시퀀스 이미지 + 3단계 설명 보여주기

*(시퀀스 다이어그램 가리키며)*

"Three phases. First — register: `MainWindow` passes all tabs to `SessionController`. Second — wire: when the user hits Start, `SessionController` applies `Qt::connect()` for all 14 slots. Third — deliver: every DSP block fires the signal, all 14 `onMeasurement()` slots receive it on the Qt main thread."

---

> 📂 **[화면 이동] ADR-006-basegraphtab-observer-pattern.md › `## Decision`**

"This decision is recorded in ADR-006. The driving quality attribute was QAS-4 — Modifiability. The risk we were managing was sprint-parallel development: Week 2 Sprint 1 required 11 tabs to be built simultaneously by multiple developers. Without a stable interface contract, every developer would be touching the same wiring code and creating merge conflicts."

---

> 📂 **[화면 유지] ADR-006 › `## Rationale` — Options Considered 표**

"We considered a separate `GraphTabManager` class to own the tab registry. We rejected it — `MainWindow` already owns the `QTabWidget`, so adding a manager in between adds an extra indirection without actually reducing coupling. The Observer pattern with `registerTab()` achieves the same single-point-of-registration goal with less complexity."

---

> 📂 **[화면 유지] ADR-006 › `## Consequences`**

"The trade-off we accepted: all 14 `onMeasurement()` slots fire on the Qt main thread per beat event. When all tabs are visible simultaneously at high beats-per-hour, the `isVisible()` guard gives no benefit. That's why we have ADR-004 — a timer-decoupled rendering tactic — as a backup plan if the full 14-tab rendering benchmark shows it's needed."

> **📌 Q&A**
>
> **Q. Why Qt Signal-Slot instead of a custom Observer implementation?**
> Qt Signal-Slot is the Observer pattern built into the framework we're already using. The key benefit over a hand-rolled observer is `QueuedConnection`: it makes cross-thread delivery safe without manual locking or mutexes. A custom observer would need us to write that synchronization ourselves — more code, more risk of race conditions.
>
> **Q. What if a tab is not visible — does it still receive the signal?**
> Yes, the signal is always delivered. But each tab has an `isVisible()` guard inside `onMeasurement()` — if the tab isn't visible, it skips the replot. When it becomes visible again, `showEvent()` triggers a catch-up render via `replotAll()`. No data is lost, but rendering work is deferred. This is the R1 Lazy Rendering tactic — it dropped replot count from 8.22 to 1.20 per beat, a 85% reduction.
>
> **Q. How do you guarantee all 14 tabs are actually subscribed?**
> `MainWindow` calls `connectObservers()` once at startup, passing the full list. `SessionController` stores it as `mObserverTabs` and re-applies `Qt::connect()` at the start of every session. If a tab isn't in the list, it simply won't render — it won't cause a crash or a missed update elsewhere. The failure mode is visible and localized.

---

## Transition

> 📂 **[화면 이동] slide-architecture-view.md › `## 2-C. Extensibility`**

"So that's correctness through Observer. Now — how did we keep the system extensible as we added more and more tabs?"

"We ended up with 14 tabs. We started with 11, added 2 more, then 1 bonus. Three separate additions, and none of them broke anything below. Three design decisions made that possible — layering, dependency inversion, and immutable Value Objects."

---

## 2-C. Extensibility — Layer

> 📂 **[화면 이동] view-layered-4layer.md › 상단 다이어그램**
> → `![4-Layer Allowed-to-Use View]` 이미지 보여주기

"First, a 4-layer architecture. This is a Module View using the Layered style — specifically the Allowed-to-Use variant, where each layer may only depend on layers below it. Acquisition at the bottom, then Signal Processing, Domain, and Presentation at the top. Dependencies only flow downward. No upward references allowed."

---

> 📂 **[화면 유지] view-layered-4layer.md › `## Element Catalog`**
> → `#### Presentation Layer` 항목 보여주기

"The practical effect: adding a new tab means touching only the Presentation layer. Three files or less. We tracked this across all three rounds of additions, and the Domain layer was never touched once. That's the Restrict Dependencies tactic for modifiability."

---

> 📂 **[화면 유지] view-layered-4layer.md › `## Behavior`**
> → Tab addition history 표 + Dependency Structure Matrix 보여주기

"The Dependency Structure Matrix here is the actual `#include` trace from the code. Every 1 is in the lower triangle — no layer violations. And the tab addition history confirms the ≤ 3-file rule held across all three sprint rounds."

> **📌 Q&A**
>
> **Q. Why is `MeasurementEngine` in Signal Processing, not Domain?**
> Because of how it's owned and driven at runtime. `MeasurementEngine` lives on the T2 DSP thread — it's structurally owned by `DSPWorker` and is part of the DSP pipeline, not a stand-alone domain service. Pure domain objects — `WatchMath`, `WatchDiagnostics`, `Measurement` — have no dependency on the DSP loop. That's the boundary.
>
> **Q. What does "3 files or less" actually mean?**
> A new tab's `.h` and `.cpp` pair, plus one change in `MainWindow` to register it — that's two to three units. `RadarChartTab` is the one exception at 3, because it reads per-position data directly from `SequenceTab` rather than through `measurementReady`. Every other tab stays at 2.
>
> **Q. What's "UI Coordinator" — is it a 5th layer?**
> It's not a domain layer. `MainWindow` and `SessionController` are wiring code — they set up connections at startup and own no business logic. We surfaced them explicitly in the diagram because they cross all four layers, and hiding them would make the view misleading. But they don't violate the 4-layer rule.

---

## 2-C. Extensibility — Interface

> 📂 **[화면 이동] view-iaudiosource.md › 상단 다이어그램**
> → `![IAudioSource Dependency Inversion]` 이미지 보여주기

"Second, `IAudioSource`. This is the Dependency Inversion Principle applied to audio sources. We have three modes: Live — microphone input, Playback — WAV file input, and Sim — synthesized watch signal. Before this interface, `SessionController` held three separate concrete pointers and duplicated the `connect()` block for each mode."

---

> 📂 **[화면 유지] view-iaudiosource.md › `## Element Catalog`**
> → `#### AS-IS` → `#### TO-BE` 순서로 보여주기

"After introducing `IAudioSource`, `SessionController` depends only on the abstraction. All three sources implement the same interface. There's one `connect()` block, one `mActiveSource` pointer, one place to change if anything needs to change."

---

> 📂 **[화면 이동] ADR-005-p1-iaudiosource-dependency-inversion.md › `## Decision`**

"This is ADR-005. Same QAS-4 — Modifiability. The risk: the AS-IS design had three separate concrete pointers in `MainWindow`, and each mode had its own duplicated `connect()` block. Adding a fourth source — say, a network audio stream — would mean modifying `MainWindow` in three places unrelated to the new source's logic."

---

> 📂 **[화면 유지] ADR-005 › `## Rationale`**

"We considered keeping the concrete pointers and adding runtime type checks with `qobject_cast`. We rejected that because it doesn't reduce the number of `connect()` blocks, and it ties `SessionController` to all concrete types anyway — the problem isn't solved, just moved."

---

> 📂 **[화면 유지] ADR-005 › `## Consequences`**

"The trade-off: the `sourceComplete()` signal has different semantics per source type. Live mic never emits it — there's no end-of-file for a microphone. Playback and Sim emit it at natural termination. Any future implementer of `IAudioSource` must follow this contract or session teardown will break. That's an implicit obligation the interface itself can't enforce."

> **📌 Q&A**
>
> **Q. Are there plans to add more audio sources?**
> Not currently. The primary benefit here isn't future extensibility — it's eliminating duplicated wiring logic now. The three sources were already there; `IAudioSource` unifies them. That said, if a network audio source were needed, it would slot in with zero changes to `SessionController`.
>
> **Q. What does the `connect()` block actually do?**
> It wires the `dataReady` signal from the active audio source to `DSPWorker::onDataReady` via `QueuedConnection`. When a session starts, `startSourceThread()` takes any `IAudioSource` and connects it identically regardless of which concrete type it is. That's the inversion in action.

---

## 2-C. Extensibility — Entity / Value Object

> 📂 **[화면 이동] view-domain-entity-vo.md › 상단 다이어그램**
> → `![Domain Entity / Value Object Module View]` 이미지 보여주기

"Third, the `Measurement` struct. In Domain-Driven Design terms, this is an Entity composed of three Value Objects — `WatchMetrics`, `SignalFrame`, and `AcousticEvent`. All three are explicitly marked immutable in the code: once produced, they don't change."

---

> 📂 **[화면 유지] view-domain-entity-vo.md › `## Element Catalog`**
> → `#### Relations` 표 보여주기

"Tabs receive `Measurement` by const reference. They physically cannot mutate it. So correctness is enforced by the type system, not by convention. The `Measurement` object in the Domain layer also has zero upward dependencies — no reference to any tab or rendering concept. That's what keeps the Domain layer clean."

> **📌 Q&A**
>
> **Q. What if a tab needs to store or accumulate measurement data over time?**
> Each tab manages its own local state for that. `SequenceTab`, for example, accumulates readings per position in its own member variables. `Measurement` is a per-cycle snapshot — what a tab does with it after `onMeasurement()` is the tab's own responsibility. The domain object stays immutable.
>
> **Q. Why three separate Value Objects instead of one flat struct?**
> Each VO has a different lifecycle and producer. `AcousticEvent` is produced by the beat detector, `SignalFrame` by the audio capture, `WatchMetrics` by the DSP math. Keeping them separate clarifies data flow and lets each layer pass only what it owns.

---

## 2-D. AI-Assisted Unit Test

> 📂 **[화면 이동] slide-architecture-view.md › `## 2-D. Risk: AI-Assisted Unit Test`**

"Now, one architectural risk we had going in — most of us don't have deep watch-measurement domain knowledge. Rate, amplitude, beat error — these are domain concepts that take time to fully understand. When developers don't understand the domain, two things can go wrong: they implement the wrong logic, or they can't tell whether their implementation is correct."

"We addressed this with two axes. First axis — AI. We used AI to help developers understand the domain during implementation: interpreting equations, clarifying what Rate or Beat Error actually means in code. That reduces the chance of introducing a mistake in the first place."

"Second axis — unit tests. Even with AI assistance, we can't be sure every formula is right yet. So we used AI to generate unit tests that verify structural correctness — every tab receives the same `Measurement`, doesn't mutate it, and honors the `BaseGraphTab` interface contract. If a developer misunderstands the domain and wires something wrong, the test catches it before it reaches integration."

"Together, AI lowers the risk of writing wrong code, and unit tests catch what slips through. In architecture terms, both are compensating for the same domain knowledge gap — just at different stages of development."

"The domain logic tests — the actual physics of `WatchMath` and `WatchDiagnostics` — are the next layer of validation, planned for the accuracy experiment in Week 4 Sprint 4."

> **📌 Q&A**
>
> **Q. What AI tool did you use, and what exactly does it test?**
> We used Claude to generate the test suite. Each test constructs a mock `Measurement`, calls `onMeasurement()`, and verifies: the tab doesn't throw, the tab doesn't mutate the struct, and the call completes. It's interface compliance testing — verifying the Observer contract, not domain logic.
>
> **Q. How many tests were generated?**
> 14 tabs, each with a set of structural tests. The same template applied across all tabs — baseline 11 in Week 2 Sprint 1, then 3 more in Week 2 Sprint 2 and Week 3 Sprint 1.
>
> **Q. What's the plan for real domain logic tests?**
> As we build domain knowledge — especially through the WeiShi accuracy comparison in Week 4 Sprint 4 — we'll add tests for the actual watch metrics computation in `WatchMath` and `WatchDiagnostics`. The structural tests give us a safety net now; domain tests will validate correctness later.

---

## 3-B. Remaining Risks

> 📂 **[화면 이동] slide-schedule.md › `## 3-B. Remaining Risks & Open Items`**

"Going into the final week, there are two things still open. Filter tuning and rendering performance haven't been validated on real hardware yet — those are the experiments we're running this week. And the big one: we haven't compared our measurements against a reference device yet. That's the WeiShi accuracy validation, scheduled for the end of the week."

"So the remaining focus is experiments first, accuracy validation last, then demo."

"All remaining tasks and issues are tracked on our GitHub Project Board — sprint by sprint through the demo on July 1st."

---

## 3-C. Milestone 3 Schedule

> 📂 **[화면 이동] slide-schedule.md › `## 3-C. M3 Schedule`**

"Four sprints left. Sprint 1 — microphone auto-recovery. Sprint 2 — filter tuning. Sprint 3 — 14-tab rendering benchmark on Raspberry Pi. Sprint 4 — accuracy validation and full Raspberry Pi run. Then a buffer week, and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
