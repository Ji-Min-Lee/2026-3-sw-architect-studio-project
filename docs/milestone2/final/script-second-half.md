# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

"The second quality goal we're covering today is correctness — every tab must receive each measurement result, no exceptions."

"Our decision: Qt Signal-Slot as Observer. `MeasurementEngine` emits `measurementReady`, every tab subscribes via `BaseGraphTab::onMeasurement()`. `MeasurementEngine` has zero knowledge of any tab. And Qt's `QueuedConnection` handles the T2 (DSP) to Qt main thread delivery safely — no manual locking needed."

---

## Transition from 2-B

"Latency — threads. Correctness — Observer. Now: **extensibility**."

---

## 2-C. Extensibility

"We ended up with 14 tabs. The question is how we kept adding them without breaking what already works. Three decisions made that possible."

### Layer

"First, layers. Four layers — Acquisition, Signal Processing, Domain, Presentation — dependencies go downward only. Adding a new tab touches only the Presentation layer, three files or less. We did this three times: 11 baseline, +2, +1 bonus. Domain layer untouched each time."

### Interface — IAudioSource

"Second, `IAudioSource`. AS-IS: three separate `connect()` blocks, one per audio source. TO-BE: one interface, one block in `SessionController`. Changing audio wiring means touching one place."

### Entity / Value Object

"Third, `Measurement`. Composed of three immutable Value Objects — `WatchMetrics`, `SignalFrame`, `AcousticEvent`. Tabs receive it read-only. They cannot mutate results. Correctness by structure."

---

## 2-D. Risk — AI-Assisted Unit Test

"NTR-07: team lacks deep watch-measurement domain knowledge. Mitigation: AI-generated unit tests that verify `onMeasurement()` interface compliance. Deep expertise takes time — AI helps us verify structural correctness in the short term while the team continues building that knowledge."

---

## Wrap-up

"Latency — threads. Correctness — Observer. Extensibility — layers, interface, immutable VOs. Domain knowledge gap — AI-assisted tests."

---

## 3-A. What We Did in Milestone 2

"M2 in one line: build → measure → discover → respond."

"We built the 4-layer architecture and 11 baseline tabs. Ran experiments — EXP-01 and EXP-02 confirmed our thread design on RPi. But found a new problem: FG scheduling latency is over budget. We responded with AI capabilities and completed the full 4-layer refactor."

---

## 3-B. Remaining Risks

"Four open risks. Two critical — TR-10: FG scheduling latency, fix targeted W4 S1. TR-05: end-to-end accuracy not yet validated, targeted W4 S4. Two medium — filter tuning and rendering benchmark on RPi, both targeting this week."

"Critical path: FG fix → experiments → accuracy validation → demo."

---

## 3-C. M3 Schedule

"Four sprints. S1 — FG fix. S2 — filter tuning. S3 — rendering benchmark. S4 — accuracy validation and full RPi run. Then buffer and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
