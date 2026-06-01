# Milestones & Grading

> **Date**: 2026-06-02  
> **Source**: Time Grapher Project Plan (Draft).pdf — p.30-33 / LG SW Architect Final Demo Grading Score Sheet p.3

---

## Grading Scale

> Point values differ per item, but the grade criteria apply equally to all items.

| Grade | Criteria |
|-------|----------|
| **Outstanding** | All elements completed + original analysis + design rationale presented with clear logic |
| **Satisfactory** | All elements completed + comprehensive analysis + consistent logic |
| **Marginal** | Functionality works without changing existing code |
| **Unsatisfactory** | Performance or quality requirements not met |
| **Not Acceptable** | Functionality does not work |

> **Satisfactory → Outstanding gap**: Not whether something was implemented, but **how logically the design rationale is explained**

---

## Milestone 1 — `2026-06-09 (Tue)`

### Deliverables

| Document | Key Content |
|----------|------------|
| Project Plan | Role assignment + tasks + milestones / architecture-driven implementation tasks / experiment plan included |
| Architectural Drivers | 5 QAs defined in measurable form + prioritization |
| Risk Assessment | Technical and non-technical risks rated H/M/L + mitigation actions defined |
| Planned Experiments | Experiment purpose + questions to resolve + completion criteria |
| Architectural Approaches | Architecture overview + patterns/tactics + linkage to QA drivers |

### To Achieve Outstanding

- QAs must be expressed as **measurable, verifiable numbers**, not plain descriptions
- Each Architectural Approach must **clearly map to which QA it supports and how**
- Experiment **completion criteria** must be concretely defined

### Mentor Check Questions

- Is each QA "actionable"? (will be flagged immediately if no numbers)
- Are Architectural Approaches linked to QA drivers?
- How can you tell when an experiment is complete?
- Are there concrete mitigation actions for each risk?

---

## Milestone 2 — `2026-06-22 (Mon)`

### Deliverables

| Document | Key Content |
|----------|------------|
| Updated Project Plan | Risk-based plan update + realistic implementation schedule |
| Experiment Results | Results of Experiments 1·2·3 + list of unresolved items |
| Architecture — Module View | Code-level structure + dependencies (at least 1 required) |
| Architecture — C&C View | Component-connector runtime perspective (at least 1 required) |
| Architecture — Deployment View | RPi-based hardware placement + communication channels |
| Construction Plan | Detailed implementation tasks + remaining schedule |

### To Achieve Outstanding

- Clearly articulate **how experiment results changed architectural decisions**
- All three Architecture Views must be consistent and cross-referenced
- Must be able to **recognize and explain the trade-offs** of the chosen architectural approaches

### Mentor Check Questions

- Did the experiment results actually resolve the open questions?
- Are all three Architecture Views present? (Module / C&C / Deployment)
- Did the experiments lead to architectural improvements?
- Are there any critical concerns still unresolved?

---

## Milestone 3 — `2026-07-01 (Wed)`

### Team Presentation (20 minutes)

| Section | Content |
|---------|---------|
| QA Requirements | High-priority QAs + their influence on the architecture |
| Architecture | Views + key approaches + design rationale |
| Experiments & Evaluation | Experiment results + architecture evaluation activities |
| Lessons Learned | What went well / what didn't / what we would do differently |

> 20 minutes is not enough to cover everything in depth → select 1–2 key points per section and go deep

### Final Demo (runs on RPi)

| QA | How to demonstrate |
|----|-------------------|
| Low Latency | Present 3-segment latency numbers (ms) — average + worst-case |
| Real-Time Performance | Confirm real-time operation on RPi |
| Correctness | Verify measurement stability under identical watch and conditions |
| Measurement Accuracy | Compare values against WeiShi No.1000 |
| Extensibility | Explain the scope of existing code changes when adding a new graph |

### To Achieve Outstanding

- Explain not just that things work, but **how the architectural and implementation choices support each QA**
- New features must be **integrated into the existing app**, not separate prototypes
- Clearly explain **what each additional feature shows the user**

### Grading Rubric Note

> The TimeGrapher-specific rubric is **scheduled for release in Week 2 or 3** (p.33)  
> The current Grading Score Sheet in assets is for the ADS-B project — do not apply it  
> Share with the full team as soon as the rubric is received

---

## Outstanding Summary per Milestone

```
M1: "QA numbers + Approach-Driver linkage"
M2: "Experiments → architectural improvements flow + 3-View consistency"
M3: "Functional demo + QA evidence numbers + design rationale explanation"
```
