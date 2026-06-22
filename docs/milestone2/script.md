# Presentation Script — My Part (2-B → 2-C → Section 3)

> **Speaker**: covers from **2-B Correctness** through **Section 3 Schedule**  
> **Target**: ~7 min core delivery (buffer for mid-presentation questions)  
> **Screen**: scroll through markdown files live

---

## Receive Handoff

*[앞 발표자로부터 받기 — 화면은 2-slide-architecture-view.md에 그대로 있음]*

Now let's look at what that separation enables: making sure all 14 tabs show exactly the same data."

---

## 2-B. Correctness: Observer Pattern

> **Key message**: One signal, one struct — 14 tabs are guaranteed to see identical data. Correctness is enforced by structure, not by discipline.

**[SCREEN → `2-slide-architecture-view.md` | scroll to `## 2-B. Correctness: Observer Pattern` | show `view2b-observer-module.png`]**

"Our key quality goal is measurement accuracy — the computed Rate and Beat Error need to match the reference device. But being accurate at the computation level isn't enough on its own. If two tabs both show Rate for the same beat but display different numbers, the system is inaccurate from the user's perspective — even if the math is correct."

"So when we say correctness here, we mean structural consistency in how data gets delivered. The risk point is delivery. If `Measurement Engine` called each tab directly, even a tiny sequencing difference could cause two tabs to show different values for the same beat. With 14 tabs, that risk only gets worse."

"So we applied the Observer pattern. `Measurement Engine` emits one signal — `measurement Ready` — carrying a single `Measurement` struct. All 14 tabs receive that same struct through `Base Graph Tab :: on Measurement`. One broadcast — identical data to everyone."

**[SCREEN → `references/adr/ADR-006-basegraphtab-observer-pattern.md` | scroll to `## Decision`]**

"In `ADR-006` — the key point is that `Measurement Engine` has zero compile-time knowledge of any tab. `Session Controller` wires up all the signal-slot connections at session start, and after that, `Measurement Engine` just emits."

---

> **📌 QnA only — not covered in presentation**
>
> **Q. Why Qt Signal-Slot instead of a custom observer implementation?**  
> Qt Signal-Slot is the Observer pattern built into the framework. The key benefit is `Queued Connection` — cross-thread delivery without manual locking. A custom observer would need us to write that synchronization from scratch.
>
> **Q. What if a tab is not visible — does it still receive the signal?**  
> Yes, always delivered. But each tab has an `is Visible` guard inside `on Measurement` — non-visible tabs skip the replot. When the tab becomes visible, `show Event` triggers a catch-up render. No data is lost. This is the Lazy Rendering tactic — it dropped replot count from 8.22 to 1.20 per beat, an 85% reduction.
>
> **Q. How do you guarantee all 14 tabs are actually subscribed?**  
> `Main Window` calls `connect Observers` once at startup. `Session Controller` applies `connect` at the start of every session. If a tab is not in the list, it won't render — it won't crash or affect other tabs. The failure mode is visible and localized.

---

## AI-Assisted Tests — Correctness Side

**[SCREEN → `references/unit-test-results.md` | show summary table]**

"One more thing on correctness. Watch domain knowledge takes time to build up — and when developers don't fully have that domain understanding yet, they can implement wrong logic without even realizing it. That was our biggest non-technical risk going in."

"We tackled it with AI-generated unit tests: 142 test cases across 10 binaries, all passing — covering domain math, engine integration, and Observer contract compliance. This was only possible because our architecture was testable by design. Clean boundaries meant we could verify each component in isolation. The architecture itself reduced the risk."

---

## Transition to Extensibility

**[SCREEN → `2-slide-architecture-view.md` | scroll to `## 2-C. Extensibility`]**

"Observer handles correctness — consistent delivery to all tabs. But that same decision also makes the system easier to extend. We've got three design decisions to walk through: layering, dependency inversion, and immutable Value Objects."

---

## 2-C. Extensibility: Layer

> **Key message**: The 4-layer allowed-to-use structure turns "add a new tab" into a 3-file operation — the architecture enforces the constraint, not the developer.

**[SCREEN → show `view1-layered-module.png`]**

"Our target was: add a new graph tab in 3 files or fewer, with no knowledge of DSP or audio capture needed. The 4-layer structure enforces this — Acquisition, Signal Processing, Domain, Presentation. Dependencies only flow downward. So a developer adding a new tab just needs to know what `Measurement` contains. The Presentation layer can grow freely without touching anything below."

**[SCREEN → `references/views/view-layered-4layer.md` | scroll to `## Behavior` | show "Tab addition history" table, then "Dependency Structure Matrix"]**

"We did this across three rounds of tab additions — 11 tabs in Sprint 1, 2 more in Sprint 2, and 1 bonus tab. Every single time, 3 files or fewer, Domain layer untouched. The Dependency Structure Matrix here is the actual include trace from the code. Every dependency is in the lower triangle. No violations."

---

> **📌 QnA only — not covered in presentation**
>
> **Q. Why is `Measurement Engine` in Signal Processing, not Domain?**  
> It is owned and driven by `DSP Worker` inside the T2 thread pipeline. It calls `process Block` as part of the DSP loop — it is not a stand-alone domain service. Pure domain objects — `Watch Math`, `Watch Diagnostics`, `Measurement` — have zero dependency on that loop.
>
> **Q. What does "3 files or fewer" actually mean in practice?**  
> A new tab's header and implementation file, plus one line in `Main Window` to register it. `Radar Chart Tab` is the one exception at 3 — it reads directly from `Sequence Tab` rather than through `measurement Ready`. Every other tab stays at 2.
>
> **Q. What is "UI Coordinator" — is it a 5th layer?**  
> No. `Main Window` and `Session Controller` are wiring code — startup connections, no business logic. We surfaced them explicitly because they cross all four layers, and hiding them would make the diagram misleading.

---

## 2-C. Extensibility: Interface — IAudioSource

> **Key message**: Dependency inversion on the audio source reduces adding a new source to 2 files, with zero changes propagating upward.

**[SCREEN → `2-slide-architecture-view.md` | scroll to `### Interface` | show `view5-iaudiosource.png`]**

"Same principle, but applied to audio input. Before the refactor, `Session Controller` held three concrete pointers — live microphone, file playback, and simulation — each with duplicated wiring code. Adding a new source meant reading through nearly identical branches and touching `Main Window` in multiple places."

"After we introduced `I Audio Source`: one pointer, one connect block. Adding a new source means implementing the interface and one factory method — nothing changes above it. Less code to read, less code to change."

"The trade-off is that any future implementer has to follow this contract."

---

> **📌 QnA only — not covered in presentation**
>
> **Q. Are there plans to add more audio sources?**  
> Not currently. The primary benefit is eliminating duplicated wiring that already existed. That said, a network audio source would slot in with zero changes to `Session Controller`.
>
> **Q. What does the `connect` block actually do?**  
> It wires `data Ready` from the active source to `DSP Worker :: on Data Ready` via `Queued Connection`. `start Source Thread` takes any `I Audio Source` and connects it identically, regardless of concrete type.

---

## 2-C. Extensibility: Domain Entity and Value Object

> **Key message**: The immutable `Measurement` struct closes the correctness loop — deviation across tabs is structurally impossible.

**[SCREEN → `2-slide-architecture-view.md` | scroll to `### Entity / Value Object` | show `view6-domain-entity-vo.png`]**

"Originally, `Measurement` was a god object — one flat struct that every tab dug through. We decomposed it into three Value Objects grouped by domain: DSP math, audio capture, and beat detection. Each tab now only depends on what it actually needs."

"And all three are immutable once produced. Tabs receive `Measurement` read-only — they can't change it. This closes the loop on correctness: two tabs reading the same field are always reading the same value."

---

> **📌 QnA only — not covered in presentation**
>
> **Q. What if a tab needs to accumulate data over time?**  
> Each tab manages its own local state. `Sequence Tab` accumulates readings per position in its own member variables. `Measurement` is a per-cycle snapshot — what a tab does with it afterwards is its own responsibility.
>
> **Q. Why three separate Value Objects instead of one flat struct?**  
> Each has a different lifecycle and producer. `Acoustic Event` from the beat detector, `Signal Frame` from audio capture, `Watch Metrics` from DSP math. Keeping them separate lets each layer pass only what it owns.

---

## Section 3 — Schedule and Risks

## 3-A. What We Did in Milestone 2

**[SCREEN → `3-slide-risk-and-schedule.md` | scroll to `## 3-A`]**

"Four sprints in Milestone 2 — we got the architecture established, all 14 tabs implemented, every scheduled experiment completed, and the full refactor done."

---

## 3-B. Remaining Risks

**[SCREEN → scroll to `## 3-B. Remaining Risks & Open Items` | show risk table]**

"We've got one critical risk and one medium risk heading into the final week."

"The critical one: we haven't validated our measurement output against a reference device yet. That validation — against the WeiShi watch — is scheduled for Week 5 Sprint 1."

"On the medium side: filter cutoff values haven't been tuned on a real watch signal yet. Ambient noise in the lab can trigger false beats, and we need to confirm the right thresholds on actual hardware. That experiment is happening this week."

---

## 3-C. M3 Schedule

**[SCREEN → scroll to `## 3-C. M3 Schedule` | show sprint table + GitHub board links]**

"The critical path is clear — filter tuning this week, WeiShi accuracy validation in Week 5, full Raspberry Pi run, then a buffer week before the demo on July 1st. Every remaining task has an owner, a date, and it's all tracked as GitHub issues on our project board."

**[SCREEN → show GitHub Project Board link in `## 3-C. M3 Schedule`]**

"Each sprint builds on work that's already done — no blockers, no dependencies waiting to land. We're on track to hit all demo criteria by July 1st."

---

## Closing

"That's Milestone 2 from our side. Happy to take questions."
