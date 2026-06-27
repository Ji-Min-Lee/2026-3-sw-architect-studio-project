# TimeGrapher sprint and team structure view

**Style**: Work Assignment (할당 뷰 — 작업할당 스타일)  
**Mapping**: Software architecture elements → Organizational units (teams, milestones, sprints)  
**Purpose**: Shows which architectural modules and QAS concerns are owned by which team, in which sprint, and gated by which milestone. Answers: "Who builds what, when, and how do we know it's done?"

---

## Diagram

```
Architecture Elements (Modules & QAS)
        │
        │  allocated-to
        ▼
┌─────────────────────────────────────────────────────────┐
│                  GitHub Project Board                   │
│         Ji-Min-Lee/2026-3-sw-architect-studio-project   │
│                                                         │
│  ┌──────────────┐          ┌──────────────┐             │
│  │    team1     │          │    team2     │             │
│  │  (Arch/Core) │          │  (UI/Scope)  │             │
│  └──────┬───────┘          └──────┬───────┘             │
│         │                        │                      │
│    QAS-1,2,4,5              QAS-3, UI tabs              │
│         │                        │                      │
│         └──────────┬─────────────┘                      │
│                    ▼                                     │
│         Sprint Labels: w2-1 → w3-2 → w4-1 ...          │
│                    │                                     │
│         ┌──────────┴─────────────┐                      │
│         ▼                        ▼                      │
│   Milestone 2 (06-22)    Milestone 3 Demo (07-01)       │
│   Architecture + Exp      Final demo                    │
└─────────────────────────────────────────────────────────┘
```

**KEY**: Each GitHub Issue = one architecture work unit. Sprint label = iteration gate. Milestone = scope gate.

---

## Element Catalog

### Software Elements (Architecture Work Units)

Each issue represents an architecture element that must be built, validated, or documented.

| Work Unit Type | Examples | QAS Traceability |
|---------------|----------|-----------------|
| Experiment (EXP-xx) | EXP-06 Witschi comparison, EXP-01 latency | QAS-5 Accuracy, QAS-1 Performance |
| ADR | ADR-001 T2 thread, ADR-003 sample rate | All QAS |
| Architecture View | Module view, C&C view, Allocation view | QAS-3 Modifiability |
| Implementation task | God Object → 4-layer refactor | QAS-4 Correctness |
| Risk | Risk assessment, mitigation plans | All QAS |

### Organizational Elements

| Org Unit | Scope | Sprint Cadence |
|----------|-------|---------------|
| **team1** | Domain layer: DSP, measurement engine, experiments | Every 2 days (w2-1, w3-1, w3-2, …) |
| **team2** | UI layer: tab data models, scope views, sound print | Every 2 days |
| **Architecture Committee** | Sprint planning, ADR decisions, milestone gate review | Each sprint boundary |
| **all-teams** | Integration, documentation, milestone submission | At milestone close |

### Environmental Elements (Tracking Infrastructure)

| Tool / View | Purpose | URL |
|-------------|---------|-----|
| Kanban Board | Sprint WIP visibility (Todo / In Progress / Sprint Backlog / Done) | GitHub Projects view 2 |
| Roadmap View | Schedule vs. milestone calendar; drift detection | GitHub Projects view 1 |
| Table View | Assignee audit, milestone assignment, QAS label audit | GitHub Projects default |
| Milestones | Scope gate: M2 (06-22) and M3-Demo (07-01) | GitHub Milestones page |

---

## Work Assignment Mapping

### By Team × QAS

| Architecture Element | Team | Sprint | Milestone | Status |
|---------------------|------|--------|-----------|--------|
| EXP-06 Witschi accuracy comparison | team1 | w2-1 | M2 | ✅ Done |
| ADR-003 Sample rate selection | team1 | w2-1 | M2 | ✅ Done |
| 4-layer God Object refactor | team1+2 | w3-1 | M2 | ✅ Done |
| Unit test suite (142 tests) | team2 | w3-2 | M2 | ✅ Done |
| Architecture views (Module/C&C/Allocation) | all-teams | w3-2 | M2 | ✅ Done |
| Risk assessment & mitigation | team1 | w3-1 | M2 | ✅ Done |
| Final demo preparation | all-teams | w4-1 | M3-Demo | In Progress |

### Sprint Board Column Rules

| Column | Entry Criteria | Exit Criteria |
|--------|---------------|--------------|
| **Todo** | Pulled from Backlog at sprint planning; assignee set | Work started |
| **In Progress** | Actively worked; max 2–3 items per team | Pass criteria met, output doc written |
| **Sprint Backlog** | Planned for future sprint; not yet pulled | Sprint planning pulls it to Todo |
| **Done** | Pass criteria met; result linked in issue; issue closed | — |

### Milestone Scope Gate

| Milestone | Deadline | Scope Rule |
|-----------|----------|-----------|
| **Milestone 2** | 2026-06-22 | Architecture + experiments complete. M2 bar = 100% = scope done. |
| **Milestone 3 Demo** | 2026-07-01 | Final demo ready. Issues surfacing mid-sprint and post-M2 go here — M2 scope never expands. |

---

## What This View Reveals

- **No orphaned tasks**: Table view enforces that every issue has an assignee and a milestone before sprint close.
- **Scope protection**: The milestone field (not a label, not a column) is the authoritative scope gate. Mid-sprint issues are immediately assigned to M3-Demo to prevent M2 scope creep.
- **QAS traceability**: Sprint and QAS labels on every issue mean any filter immediately shows "what is team X working on, for which quality attribute, in which sprint" — the architecture committee can audit QA coverage at a glance.
- **WIP discipline**: In Progress cap of 2–3 items per team prevents the common failure mode of starting everything and finishing nothing before the milestone deadline.

---

## Related Views

- [Raspberry Pi Deployment View](view-deployment-build-pipeline.md) — shows the hardware targets these teams build for
- [Pre-commit Correctness Gate View](view-allocation-implementation.md) — shows the commit-time correctness gate that protects formula and calculation changes

## Related References

- [Planned Experiments](../experiments/planned-experiments.md) — experiment issues that flow through this sprint/milestone structure
