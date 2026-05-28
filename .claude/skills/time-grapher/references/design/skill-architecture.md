# TimeGrapher Skill — Architecture

## Skill Type

`artifact_type: skill` / `skill_scope: single-skill` / `target_agent: claude`

This skill is a **Context Provider** type that injects project context into Claude without external API calls or code execution to guide accurate responses.

## Module Relationship Diagram

```mermaid
graph TD
    subgraph time-grapher-skill
        SKILL[SKILL.md\nIndex Hub]
        subgraph references/project
            OV[overview.md\nProject Overview]
            MS[milestones.md\nMilestone Deliverables]
            TD[todo.md\nTODO + Schedule]
        end
        subgraph references/design
            SA[skill-architecture.md]
            MC[skill-module-contracts.md]
            DM[skill-data-model.md]
            DP[skill-dependency-map.md]
        end
        subgraph evals
            EV[evals.json]
        end
        EVAL_S[evaluation-summary.md]
    end

    subgraph assets
        PDF1[Project Plan.pdf]
        PDF2[Equations.pdf]
        PDF3[KickOff.pdf]
        ZIP[TimeGrapher_v10.5.zip]
    end

    SKILL --> OV
    SKILL --> MS
    SKILL --> TD
    SKILL --> SA
    SKILL --> MC
    SKILL --> DM
    SKILL --> DP

    OV -.reference.-> PDF1
    MS -.reference.-> PDF1
    TD -.reference.-> PDF1
    OV -.reference.-> PDF2
```

## Processing Flow

```mermaid
flowchart LR
    U([User Request]) --> T{Trigger\nDetection}
    T -- "assignment/milestone/\ntimegrapher/watch" --> L[Load Skill]
    T -- not detected --> X([Normal Response])
    L --> R{Request Type\nClassification}
    R -- project overview --> OV[Load\noverview.md]
    R -- milestone/deliverables --> MS[Load\nmilestones.md]
    R -- tasks/schedule --> TD[Load\ntodo.md]
    R -- complex question --> ALL[Load All References]
    OV & MS & TD & ALL --> A([Generate Context-Based\nResponse])
```

## Design Principles

| Principle | Application |
|-----------|-------------|
| **SRP** | SKILL.md serves only as index; detailed content is separated into references/ |
| **OCP** | New milestones/deliverables only require modifying files under references/project/ |
| **DIP** | No direct dependency on assets/*.pdf; accessed through organized references/ |
