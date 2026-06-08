# Presentation Script — TimeGrapher M1
> **Speaker A (Gyeongjin)**: Project Plan + Architecture Approaches + Experiments  
> **Speaker B**: Quality Attributes (QA Drivers)

---

## [Speaker A] Opening

Hello, we are Team 3. I'm Gyeongjin, and I'll be presenting today along with one other teammate.

To give you a quick overview of how we'll divide the presentation — my teammate will cover the quality attribute requirements, and I will cover everything else: the project plan, architectural approaches, and experiments.

Also, due to time constraints, we will skip some details and focus on what we think is most important. Please bear with us.

Let's get started with the project plan.

---

## [Speaker A] 1. Project Plan

### What is TimeGrapher?

TimeGrapher analyzes acoustic signals (beat noise) from the escapement vibration of a mechanical watch to diagnose watch performance in real time. It captures tick-tock sounds via microphone, detects A(T1)·C(T3) events, and computes Rate, Amplitude, and Beat Error for GUI display.

Our team's core objectives, in priority order, are:

| Rank | Objective | Description |
|:---:|-----------|-------------|
| **1st** | **Accurate Measurement** | Priority is to measure Rate / Amplitude / Beat Error accurately for at least some BPH range. Sacrificing accuracy to cover more BPH is not an acceptable trade-off |
| **2nd** | **Real-Time Performance** | Process every audio block within the block period to prevent Ring Buffer overflow — prerequisite for measurement accuracy |
| **3rd** | **Extensible Architecture** | Enable parallel development of 11 graphs within 5 weeks |
| **4th** | **Architecture Principles** | Apply CMU MSE software architecture design principles |

### Why Agile + ADD?

We have a 5-week timeline with high requirement uncertainty — we didn't know from the start exactly how many graphs we needed or what performance numbers were achievable on Raspberry Pi.

A BDUF (Big Design Up Front) approach would be too risky here: if we plan everything upfront and find out in week 4 that a key assumption was wrong, we have no time to recover.

So we adopted **Agile** to iterate quickly, and **ADD (Attribute-Driven Design)** to make sure our sprints are architecture-driven, not just feature-driven.

The key insight from ADD is this: we cannot satisfy all QAs simultaneously in 5 weeks. So we must prioritize — implement the most critical QA first, then build on top of it. ADD gives us a systematic way to do that.

### Development Process (Agile)

| Event | Cadence | Participants | Duration | ADD Step |
|-------|---------|-------------|---------|---------|
| Sprint Planning | Every sprint start (every 2 days) | Architecture Committee (both SMs + PO) | 1 hour | **Step 2–4**: Select QA driver → decomposition target → tactic/pattern |
| Sprint (Development) | 2 days | Each team independently | 2 days | **Step 5**: Element instantiation + responsibility allocation (impl + experiment) |
| Sprint Review & Retrospective | Every sprint end | Full team | 1 hour | **Step 6**: Views sketch + record design decisions (ADR) |

Two development teams run in parallel within the same sprint period, each focusing on a different QA driver.

We will skip the detailed Agile rules here.

### Sprint Schedule

Our target is **June 26** for full implementation and RPi validation. Everything after that is presentation prep only.

```
M1 Submission (06/09) → Construction start (06/10) → M2 Submission (06/22) → RPi Final Validation (06/26) → M3 Final Demo (07/01)
```

| Period | Team 1 | Team 2 |
|--------|--------|--------|
| P1 (06/10~12) | QAS-1 Real-Time (AP-1+AP-2+EXP-01) | QAS-5 Extensibility base (AP-3) |
| P2 (06/15~16) | QAS-1 complete (AP-6 fallback) | QAS-3 Correctness start (AP-3+AP-4+EXP-03) |
| P3 (06/17~19) | QAS-2 Low Latency (EXP-02+AP-7a) | QAS-3 Correctness complete (EXP-03+AP-5) |
| P4 (06/22~23) | QAS-4 Usability + Core graphs | QAS-5 All graphs |
| P5 (06/24~26) | RPi integration + stabilization | RPi integration + stabilization |

---

## [Speaker B] 2. Quality Attribute Requirements

*(Speaker B takes over here)*

---

## [Speaker A] 3. Architectural Approaches

*(Resume after Speaker B)*

---

## [Speaker A] 4. Experiments

---

## [Speaker A] Closing

That's our M1 presentation. Thank you.
