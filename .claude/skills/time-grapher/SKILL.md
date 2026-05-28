---
name: time-grapher
description: >-
  Context skill for the LG SW Architect Training Program TimeGrapher project.
  Provides project summary, milestone deliverables, TODO list, and technical specs.
  Trigger when: "timegrapher", "time grapher", "assignment", "milestone",
  "architecture assignment", "watch", "acoustic", "raspberry pi assignment", "Qt assignment".
metadata:
  version: 0.1.0
  last-updated-at: 2026-05-28
  dependency-skills: []
---

# TimeGrapher Project Skill

> **LG Software Architecture Training Program × CMU MSE**  
> "From Tick to Trace: Real-Time Acoustic Analysis"

## Purpose

Provides the TimeGrapher project context to Claude for the LG SW Architect Training Program.
Loads the reference documents below when answering questions, planning, or reviewing to ensure accurate project-based responses.

---

## References

### Project

| Document | Contents |
|----------|----------|
| [Project Overview](references/project/overview.md) | Background, system architecture, quality attributes, tech stack |
| [Milestone Deliverables](references/project/milestones.md) | M1/M2/M3 deliverables and checkpoints |
| [TODO List](references/project/todo.md) | Weekly tasks + full schedule |

### Design

| Document | Contents |
|----------|----------|
| [Architecture](references/design/skill-architecture.md) | Skill structure and module relationship diagram |
| [Module Contracts](references/design/skill-module-contracts.md) | Input/output/failure contracts |
| [Data Model](references/design/skill-data-model.md) | Project information data structure |
| [Dependency Map](references/design/skill-dependency-map.md) | Skill dependency and association relationships |

---

## Assets (assets/)

| File | Purpose |
|------|---------|
| `Time Grapher Project Plan (Draft).pdf` | Full requirements and deliverable spec (primary reference) |
| `LG 2026 Arch Kick Off Brief.pdf` | Kickoff slides |
| `TimeGrapher Equations_v0.docx.pdf` | Measurement calculation formulas |
| `TimeGrapher GUI Set Up Instructions.pdf` | Qt environment setup |
| `Witschi-Training-Course.pdf` | Watch measurement concepts (pp.14-19 required reading) |
| `Witschi Chronoscope X1 G3 Instruction Manual.pdf` | GUI display mode reference |
| `LG SW Architect Final Demo Grading Score Sheet and Ruberic.pdf` | Grading criteria |
| `TimeGrapher_v10.5_Student.zip` | Existing codebase |
