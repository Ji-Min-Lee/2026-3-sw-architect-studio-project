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

"Our governing quality goal is measurement accuracy — the computed Rate and Beat Error must match the reference device. But accuracy at the computation level is not enough. If two tabs both display Rate and show different numbers for the same beat, the system is inaccurate from the user's perspective — even if the math is correct."

"That's what correctness means here: structural consistency in data delivery. The risk is at the delivery point. If `Measurement Engine` called each tab directly, even a tiny sequencing difference could cause two tabs to show different values for the same beat. With 14 tabs, that risk compounds."

"So we applied the Observer pattern. `Measurement Engine` emits one signal — `measurement Ready` — carrying a single `Measurement` struct. All 14 tabs receive that same struct through `Base Graph Tab :: on Measurement`. One broadcast — identical data to everyone."

**[SCREEN → `references/adr/ADR-006-basegraphtab-observer-pattern.md` | scroll to `## Decision`]**

"In `ADR-006` — the key point is that `Measurement Engine` has zero compile-time knowledge of any tab. `Session Controller` wires the signal-slot connections at session start. After that, `Measurement Engine` just emits."

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

**[SCREEN → `references/views/view-decomposition-graph-tab.md` | scroll to `## Behavior` | show Observer contract validation table]**

"One more thing on correctness. Watch domain knowledge takes time to build — and when developers don't fully understand the domain, they may implement wrong logic without knowing it. That was our biggest non-technical risk."

"We addressed it with AI-generated unit tests: 37 test cases verifying Observer contract compliance — every tab receives the same `Measurement`, doesn't mutate it, and honors the interface. This was only possible because our architecture was testable by design. Clean boundaries meant each tab could be verified in isolation. The architecture reduced the risk."

---

## Transition to Extensibility

**[SCREEN → `2-slide-architecture-view.md` | scroll to `## 2-C. Extensibility`]**

"Observer solves correctness — consistent delivery to all tabs. The same decision also makes the system easier to extend. Three design decisions: layering, dependency inversion, and immutable Value Objects."

---

## 2-C. Extensibility: Layer

> **Key message**: The 4-layer allowed-to-use structure turns "add a new tab" into a 3-file operation — the architecture enforces the constraint, not the developer.

**[SCREEN → show `view1-layered-module.png`]**

"Our target: add a new graph tab in 3 files or fewer, with zero knowledge of DSP or audio capture required. The 4-layer structure enforces this — Acquisition, Signal Processing, Domain, Presentation. Dependencies flow downward only. A developer adding a new tab only needs to know what `Measurement` contains. The Presentation layer grows freely without touching anything below."

**[SCREEN → `references/views/view-layered-4layer.md` | scroll to `## Behavior` | show "Tab addition history" table, then "Dependency Structure Matrix"]**

"We ran this three times — 11 tabs in Sprint 1, plus 2 in Sprint 2, plus 1 bonus tab. Every time, 3 files or fewer, Domain layer untouched. The Dependency Structure Matrix is the actual include trace from the code. Every dependency is in the lower triangle. No violations."

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

"Same principle, applied to audio input. Before the refactor, `Session Controller` held three concrete pointers — live microphone, file playback, and simulation — each with duplicated wiring code. Adding a new source meant reading through nearly identical branches and touching `Main Window` in multiple places."

"After introducing `I Audio Source`: one pointer, one connect block. Adding a new source means implementing the interface and one factory method — zero changes above. Less code to read, less code to change."

**[SCREEN → `references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md` | scroll to `## Consequences`]**

"Trade-off: Any future implementer must follow this contract — the interface cannot enforce it at compile time."

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

"Originally, `Measurement` was a god object — one flat struct that every tab dug through. We decomposed it into three Value Objects grouped by domain: DSP math, audio capture, and beat detection. Each tab now depends only on what it actually needs. The intent becomes self-documenting."

"And all three are immutable once produced. Tabs receive `Measurement` read-only — they cannot change it. This closes the loop on correctness: two tabs reading the same field are reading the same value, always."

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

"Four sprints in Milestone 2 — architecture established, all 14 tabs implemented, all scheduled experiments completed, full refactor done."

---

## 3-B. Remaining Risks

**[SCREEN → scroll to `## 3-B. Remaining Risks & Open Items` | show risk table]**

"One critical and one medium risk going into the final week."

"The critical one: we haven't compared our measurement output against a reference device yet. That validation — against the WeiShi watch — is planned for Week 5 Sprint 1."

"On the medium side: filter cutoff values haven't been tuned on a real watch signal yet. Ambient noise in the lab can trigger false beats, and we need to confirm the right thresholds on actual hardware — that experiment is scheduled for this week."

---

## 3-C. M3 Schedule

**[SCREEN → scroll to `## 3-C. M3 Schedule` | show sprint table + GitHub board links]**

"The critical path is clear — filter tuning this week, then WeiShi accuracy validation in Week 5, full Raspberry Pi run, buffer week, and demo on July 1st. Every remaining task has an owner, a date, and is tracked as a GitHub issue on our project board."

**[SCREEN → show GitHub Project Board link in `## 3-C. M3 Schedule`]**

"Each sprint is scoped so that nothing depends on a task that hasn't landed yet. We're on track to hit all demo criteria by July 1st."

---

## Closing

"That's Milestone 2 from our side. Happy to take questions."
