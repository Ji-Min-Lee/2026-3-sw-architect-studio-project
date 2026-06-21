# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

> 📂 **[화면] slide-architecture-view.md › `## 2-B. Correctness: Observer Pattern`**
> → 클래스 다이어그램 보여주기

"Our governing quality attribute is QAS-1 — measurement accuracy. One of its core requirements is that every graph tab must show the same value for the same metric. VarioTab and TraceTab both display Rate — they must match. Every single beat."

"The risk is consistency at the delivery point. If `MeasurementEngine` calls each tab directly, even a small sequencing difference could cause Tab A and Tab B to show slightly different values for the same beat. And with 14 tabs, that risk compounds."

"So we applied the GoF Observer pattern. `MeasurementEngine` is the Subject — it emits one signal, `measurementReady`, carrying a single `Measurement` struct. Every tab is a Concrete Observer, receiving that same struct through `BaseGraphTab::onMeasurement()`. One struct, one broadcast — all 14 tabs see identical data."

---

> 📂 **[화면 이동] view-decomposition-graph-tab.md › `## Element Catalog`**
> → `#### SessionController` 항목 보여주기

"The key structural point: there is no compile-time link from `MeasurementEngine` to any tab. `SessionController` wires the signal-slot connections at session start. After that, `MeasurementEngine` just emits — it has zero knowledge of who's listening."

---

> 📂 **[화면 이동] view-decomposition-graph-tab.md › `## Behavior`**
> → 런타임 시퀀스 이미지 보여주기

"Three phases at runtime. Register once at startup. Wire at each session start. Then deliver — every DSP block fires the signal, all 14 `onMeasurement()` slots receive the same struct on the Qt main thread."

---

> 📂 **[화면 이동] ADR-006-basegraphtab-observer-pattern.md › `## Decision`**

"This is ADR-006. The primary QA driving it was QAS-4 — Modifiability, not just correctness. The risk: Week 2 Sprint 1 required 11 tabs to be built simultaneously. Without a stable `onMeasurement()` contract, every developer would be touching the same wiring code and causing merge conflicts."

---

> 📂 **[화면 유지] ADR-006 › `## Rationale` — Options Considered 표**

"We looked at a separate `GraphTabManager` to own the registry. Rejected — `MainWindow` already owns the `QTabWidget`, so it adds indirection without reducing coupling. `registerTab()` on `MainWindow` achieves the same registration guarantee with less moving parts."

---

> 📂 **[화면 유지] ADR-006 › `## Consequences`**

"The trade-off: all 14 `onMeasurement()` slots fire on the Qt main thread per beat. When all tabs are visible at high BPH, the `isVisible()` guard gives no benefit. That's why ADR-004 exists as a backup plan — a timer-decoupled rendering tactic — if the 14-tab benchmark shows it's needed."

> **📌 Q&A**
>
> **Q. Why Qt Signal-Slot instead of a custom Observer implementation?**
> Qt Signal-Slot is the Observer pattern built into the framework we're already using. The key benefit is `QueuedConnection`: cross-thread delivery without manual locking. A custom observer would need us to write that synchronization ourselves — more code, more risk of race conditions.
>
> **Q. What if a tab is not visible — does it still receive the signal?**
> Yes, always delivered. But each tab has an `isVisible()` guard inside `onMeasurement()` — non-visible tabs skip the replot. When it becomes visible, `showEvent()` triggers a catch-up render via `replotAll()`. No data is lost. This is the R1 Lazy Rendering tactic — it dropped replot count from 8.22 to 1.20 per beat, an 85% reduction.
>
> **Q. How do you guarantee all 14 tabs are actually subscribed?**
> `MainWindow` calls `connectObservers()` once at startup, passing the full list. `SessionController` re-applies `Qt::connect()` at the start of every session. If a tab isn't in the list, it won't render — it won't crash or affect other tabs. The failure mode is visible and localized.

---

## Transition

> 📂 **[화면 이동] slide-architecture-view.md › `## 2-C. Extensibility`**

"Observer solves the correctness side — consistent delivery to all tabs. But the same decision also makes the system easier to extend. And that's what we'll look at now, through three design decisions: layering, dependency inversion, and immutable Value Objects."

---

## 2-C. Extensibility — Layer

> 📂 **[화면 이동] view-layered-4layer.md › 상단 다이어그램**
> → `![4-Layer Allowed-to-Use View]` 이미지 보여주기

"QAS-4 scenario: add a new graph tab with ≤ 3 file changes, zero references to Signal Processing or Acquisition. That's the concrete measurable target."

"The 4-layer structure enforces this. Acquisition at the bottom, Signal Processing, Domain, Presentation at the top. Dependencies flow downward only. The Presentation layer is free to grow without touching anything below."

---

> 📂 **[화면 유지] view-layered-4layer.md › `## Behavior` — "Tab addition history" 표 → "Dependency Structure Matrix" 표 순서로 스크롤**

"We ran this three times. 11 tabs in Sprint 1, plus 2 in Sprint 2, plus 1 bonus — each time ≤ 3 files, Domain layer untouched. The DSM is the actual `#include` trace from the code. Every 1 is in the lower triangle. No violations."

> **📌 Q&A**
>
> **Q. Why is `MeasurementEngine` in Signal Processing, not Domain?**
> It's owned and driven by `DSPWorker` inside the T2 thread pipeline. It's not a stand-alone domain service — it calls `processBlock()` as part of the DSP loop. Pure domain objects — `WatchMath`, `WatchDiagnostics`, `Measurement` — have zero dependency on that loop.
>
> **Q. What does "3 files or less" actually mean?**
> A new tab's `.h` + `.cpp`, plus one line in `MainWindow` to register it. `RadarChartTab` is the one exception at 3 — it reads directly from `SequenceTab` rather than through `measurementReady`. Every other tab stays at 2.
>
> **Q. What's "UI Coordinator" — is it a 5th layer?**
> No. `MainWindow` and `SessionController` are wiring code — startup connections, no business logic. We surfaced them explicitly because they cross all four layers, and hiding them would make the diagram misleading.

---

## 2-C. Extensibility — Interface

> 📂 **[화면 이동] view-iaudiosource.md › 상단 다이어그램**
> → `![IAudioSource Dependency Inversion]` 이미지 보여주기

"Same QAS-4 principle, applied to audio sources. Three modes: Live, Playback, Sim. In the AS-IS design, `SessionController` held three concrete pointers and duplicated the `connect()` block for each. Adding a fourth mode meant touching `MainWindow` in three unrelated places."

---

> 📂 **[화면 유지] view-iaudiosource.md › `## Element Catalog` — `#### TO-BE`**

"After introducing `IAudioSource`, there's one pointer, one `connect()` block. The high-level coordinator depends only on the abstraction."

---

> 📂 **[화면 이동] ADR-005-p1-iaudiosource-dependency-inversion.md › `## Rationale` — TO-BE 항목 보여주기**

"With `IAudioSource`, adding a new source comes down to two things: implement the interface in 1–2 files, and add a factory method in `SessionController`. Zero changes to `MainWindow`, `DSPWorker`, or `MeasurementEngine`. The change stays inside the Acquisition layer and doesn't propagate upward — which is exactly what QAS-4 requires."

---

> 📂 **[화면 유지] ADR-005 › `## Consequences`**

"The trade-off: `sourceComplete()` has different semantics per type. Live mic never emits it — no end-of-file for a microphone. Playback and Sim emit at natural termination. Any future implementer must follow this contract or session teardown breaks. It's an obligation the interface can't enforce at compile time."

> **📌 Q&A**
>
> **Q. Are there plans to add more audio sources?**
> Not currently. The primary benefit is eliminating duplicated wiring that already existed. That said, a network audio source would slot in with zero changes to `SessionController`.
>
> **Q. What does the `connect()` block actually do?**
> It wires `dataReady` from the active source to `DSPWorker::onDataReady` via `QueuedConnection`. `startSourceThread()` takes any `IAudioSource` and connects it identically, regardless of concrete type.

---

## 2-C. Extensibility — Entity / Value Object

> 📂 **[화면 이동] view-domain-entity-vo.md › 상단 다이어그램**
> → `![Domain Entity / Value Object Module View]` 이미지 보여주기

"Third — the `Measurement` struct itself. It's composed of three Value Objects: `WatchMetrics`, `SignalFrame`, `AcousticEvent`. All immutable once produced."

---

> 📂 **[화면 유지] view-domain-entity-vo.md › `## Element Catalog` — `#### Relations` 표**

"Tabs receive `Measurement` as read-only. They cannot mutate it. This is what closes the loop on QAS-1 — all 14 tabs receive the exact same immutable snapshot. VarioTab and TraceTab displaying Rate are reading from the same struct. Deviation across tabs is structurally impossible."

"And because `Measurement` lives entirely in the Domain layer with zero upward dependencies, adding or replacing a Presentation tab has no impact on it."

> **📌 Q&A**
>
> **Q. What if a tab needs to accumulate data over time?**
> Each tab manages its own local state. `SequenceTab` accumulates readings per position in its own member variables. `Measurement` is a per-cycle snapshot — what a tab does with it afterwards is its own responsibility.
>
> **Q. Why three separate Value Objects instead of one flat struct?**
> Each has a different lifecycle and producer. `AcousticEvent` from the beat detector, `SignalFrame` from audio capture, `WatchMetrics` from DSP math. Keeping them separate lets each layer pass only what it owns.

---

## 2-D. AI-Assisted Unit Test

> 📂 **[화면 이동] slide-architecture-view.md › `## 2-D. Risk: AI-Assisted Unit Test`**

"One risk we had going in — most of us don't have deep watch-measurement domain knowledge. Rate, amplitude, beat error take time to fully understand. When developers don't understand the domain, two things can go wrong: they implement the wrong logic, or they can't tell whether their implementation is correct."

"We addressed this with two axes. First — AI during implementation. We used AI to help interpret equations and clarify what Rate or Beat Error actually means in code. That reduces mistakes at the source."

"Second — AI-generated unit tests. Even with AI assistance, we can't fully verify domain logic yet. So we used it to generate tests that verify structural correctness: every tab receives the same `Measurement`, doesn't mutate it, and honors the `BaseGraphTab` contract. If a developer misunderstands the domain and wires something wrong, the test catches it before integration."

"Both axes are compensating for the same domain knowledge gap — at different stages. AI prevents mistakes during development; tests catch what slips through. The domain logic tests — actual physics of `WatchMath` and `WatchDiagnostics` — are the next layer, planned for the WeiShi accuracy validation in Week 4."

> **📌 Q&A**
>
> **Q. What exactly do the tests verify?**
> Each test constructs a mock `Measurement`, calls `onMeasurement()`, and checks: the tab doesn't throw, doesn't mutate the struct, and the call completes. Interface compliance — Observer contract, not domain logic.
>
> **Q. How many tests?**
> 14 tabs, same structural template each. 11 in Week 2 Sprint 1, 3 more added across Week 2 Sprint 2 and Week 3 Sprint 1.

---

## 3-B. Remaining Risks

> 📂 **[화면 이동] slide-schedule.md › `## 3-B. Remaining Risks & Open Items`**

"Going into the final week, two things are still open. Filter cutoff parameters haven't been tuned on a real watch signal yet — ambient noise in the lab can throw off beat detection, and we need to confirm the right thresholds on actual hardware. And the critical one: accuracy hasn't been compared against a reference device yet. That's the WeiShi validation at the end of the week."

"Experiments first, accuracy validation last, then demo. All tasks and issues are tracked on our GitHub Project Board."

---

## 3-C. Milestone 3 Schedule

> 📂 **[화면 이동] slide-schedule.md › `## 3-C. M3 Schedule`**

"Four sprints left. Microphone auto-recovery, then filter tuning on real hardware to validate noise thresholds, then accuracy validation against the WeiShi reference device and full Raspberry Pi run. Buffer week, and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
