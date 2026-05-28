# TimeGrapher Skill — Module Contracts

## Module List

| Module | File | Role |
|--------|------|------|
| index-hub | `SKILL.md` | Trigger, purpose, and reference document index |
| project-context | `references/project/overview.md` | Provides project background, system architecture, and QA |
| milestone-context | `references/project/milestones.md` | Provides milestone deliverables and checkpoints |
| todo-context | `references/project/todo.md` | Provides weekly TODOs and schedule |

---

## index-hub (`SKILL.md`)

### Input
```
User request (natural language)
Trigger keywords: timegrapher / time grapher / assignment / milestone /
                  architecture assignment / watch / acoustic / raspberry pi assignment / Qt assignment
```

### Output
```
- On trigger detected: skill activated + routed to appropriate reference module
- Cannot respond directly: detailed content delegated to references/
```

### Failure
```
- Trigger not detected: skill not loaded (normal)
- Reference file missing: references/ sub-file path error → inform user
```

---

## project-context (`overview.md`)

### Input
```
Request intent: project background / system architecture / tech stack / quality attributes / required graphs
```

### Output
```
- Project background and goals (1-2 paragraphs)
- System architecture diagram (text diagram)
- Hardware spec table
- 5 quality attributes (QA, target, measurement method)
- List of 11 required graphs
```

### Failure
```
- Request for detailed hardware specs → refer to assets/TimeGrapher GUI Set Up Instructions.pdf
- Request for calculation formulas → refer to assets/TimeGrapher Equations_v0.docx.pdf
```

---

## milestone-context (`milestones.md`)

### Input
```
Request intent: specific milestone deliverables / submission deadline / review criteria / presentation preparation
```

### Output
```
- Deliverable list for requested milestone (M1/M2/M3)
- Submission deadline (date + day of week)
- Review checkpoints (based on mentor questions)
- M3: presentation items and demo requirements
```

### Failure
```
- Request for detailed grading criteria → refer to assets/LG SW Architect Final Demo Grading Score Sheet and Ruberic.pdf
```

---

## todo-context (`todo.md`)

### Input
```
Request intent: this week's tasks / specific week's TODO / full schedule / completed items check
```

### Output
```
- TODO checklist for the relevant week based on current date
- Completed ([x]) / incomplete ([ ]) items distinguished
- Gantt chart (full schedule)
```

### Failure
```
- Current date unclear → ask user to confirm date before responding
```
