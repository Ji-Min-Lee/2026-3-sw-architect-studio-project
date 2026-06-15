# TimeGrapher — Milestone 2 Presentation

**Team**: Blue Sky (Team 3) | **Milestone**: M2 | **Date**: 2026-06-22

---

## Agenda

| # | Section | Focus |
|---|---------|-------|
| 1 | Project Plan Update | What changed since M1, and why |
| 2 | Experiment Results | What we measured, what it proved |
| 3 | Architecture | Module / Runtime / Deployment Views + Design Decisions |
| 4 | Construction Plan | What we build next, and how we'll get there |

---

## Our M2 Thesis

> **M1** established what we needed to build.  
> **M2** proves we know *how* to build it — on the target hardware, within 5 weeks.

In M1, we identified the system goals and planned our experiments.  
In M2, we executed those experiments and let the data drive our architecture.

**The key finding**: The original single-threaded design fails real-time constraints on Raspberry Pi 5 (43% deadline miss). We identified the root cause, defined two independent architectural decisions, validated both on macOS, and now have a clear path to RPi.

---

## What "Done" Looks Like for M2

| Deliverable | Status |
|-------------|--------|
| Updated Project Plan | ✅ |
| Experiment Results (EXP-02 complete) | ✅ |
| Architecture — Module View | ✅ |
| Architecture — Runtime / C&C View | ✅ |
| Architecture — Deployment View | ✅ |
| Construction Plan (M2 → M3) | ✅ |
