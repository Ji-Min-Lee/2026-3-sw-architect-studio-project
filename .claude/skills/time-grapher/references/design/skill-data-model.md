# TimeGrapher Skill — Data Model

## Static Data Structure

This skill uses markdown files under `references/` as static data sources without external DB or API.

```
references/
├── project/
│   ├── overview.md        ← ProjectContext
│   ├── milestones.md      ← MilestoneContext
│   └── todo.md            ← TodoContext
└── design/                ← SkillDesign (meta)
    ├── skill-architecture.md
    ├── skill-module-contracts.md
    ├── skill-data-model.md
    └── skill-dependency-map.md

assets/                    ← RawAssets (PDF/ZIP, direct access minimized)
evals/
└── evals.json             ← EvalCases
```

## Core Data Types

### ProjectContext

```yaml
project:
  name: TimeGrapher
  program: "LG SW Architect Training Program × CMU MSE"
  tagline: "From Tick to Trace: Real-Time Acoustic Analysis"
  duration_weeks: 5
  start_date: 2026-05-27
  end_date: 2026-07-01
  platform: Raspberry Pi 5
  language: C++
  framework: Qt
  codebase: TimeGrapher_v10.5_Student.zip

measurements:
  - name: Rate
    unit: s/d
    normal_range: "±5 s/d"
  - name: Amplitude
    unit: degrees
    normal_range: "270~310° (strong), 220~250° (acceptable)"
  - name: BeatError
    unit: ms
    normal_range: "≤ 0.6 ms"
  - name: BPH
    unit: bph
    common_values: [21600, 28800, 36000]

quality_attributes:
  - name: RealTimePerformance
    target_sps: 96000
    min_sps: 48000
    stretch_sps: 192000
  - name: LowLatency
    measurement: capture_to_process + process_to_display + end_to_end
  - name: Correctness
    reference: "WeiShi No.1000"
  - name: MeasurementAccuracy
    events: [T1, T3]
  - name: Extensibility
    criteria: "New graphs addable without major changes to existing code"
```

### MilestoneContext

```yaml
milestones:
  - id: M1
    due: 2026-06-09
    deliverables:
      - ProjectPlan
      - ArchitecturalDrivers
      - RiskAssessment
      - PlannedExperiments
      - ArchitecturalApproaches
  - id: M2
    due: 2026-06-22
    deliverables:
      - UpdatedProjectPlan
      - ExperimentResults
      - Architecture_ModuleView
      - Architecture_CnCView
      - Architecture_DeploymentView
      - ConstructionPlan
  - id: M3
    due: 2026-07-01
    deliverables:
      - TeamPresentation_20min
      - FinalDemonstration_RaspberryPi
```

### TodoContext

```yaml
weeks:
  - week: 0
    period: "05/25 ~ 05/29"
    theme: Kickoff
    milestone: null
  - week: 1
    period: "06/01 ~ 06/05"
    theme: Analysis & Planning
    milestone: null
  - week: 2
    period: "06/08 ~ 06/12"
    theme: Milestone 1 Submission
    milestone: M1  # due 06/09
  - week: 3
    period: "06/15 ~ 06/19"
    theme: Experiments & Design & Implementation
    milestone: null
  - week: 4
    period: "06/22 ~ 06/26"
    theme: Milestone 2 Submission
    milestone: M2  # due 06/22
  - week: 5
    period: "06/29 ~ 07/01"
    theme: Final Demo
    milestone: M3  # due 07/01
```

## Data Flow

```
RawAssets (PDF/ZIP)
    ↓ read and organize (manual, one-time)
references/project/*.md   ← directly referenced by Claude
    ↓ loaded on skill trigger
Claude Context
    ↓ generate response
User
```
