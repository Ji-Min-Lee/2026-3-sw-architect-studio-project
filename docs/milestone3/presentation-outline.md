# Team Presentation Outline

**Milestone**: M3 | **Date**: 2026-07-01 | **Duration**: 20 minutes

> 20 minutes is not enough for exhaustive coverage. Select 1-2 key points per section.

---

## Presentation Structure

| Section | Time | Key Points |
|---------|------|-----------|
| 1. QA Requirements | ~4 min | Top 2-3 drivers that shaped the architecture |
| 2. Architecture | ~6 min | Key views + approaches + design rationale |
| 3. Experiments & Evaluation | ~5 min | Most impactful experiment results |
| 4. Lessons Learned | ~5 min | What went right / wrong / differently |

---

## Section 1: Quality Attribute Requirements (~4 min)

> Select 2-3 highest-priority QARs that most influenced architectural decisions.

### Chosen QARs to Present

**QAR-1**: _______________ (e.g., Real-Time Performance)

- Why ranked HIGH priority:
- How it shaped architecture:
- How we verified it:

**QAR-2**: _______________ (e.g., Extensibility)

- Why ranked HIGH priority:
- How it shaped architecture:
- How we verified it:

**QAR-3** *(optional)*: _______________

- Why ranked HIGH priority:
- How it shaped architecture:
- How we verified it:

---

## Section 2: Architecture (~6 min)

### 2.1 Architecture Overview
> Brief system diagram — the pipeline from watch to display

*[Include simplified version of deployment/runtime diagram]*

### 2.2 Key Architectural Approaches

| Approach | What It Is | Why We Chose It | Trade-off |
|----------|-----------|----------------|-----------|
| | | | |
| | | | |

### 2.3 Architecture Views

> Reference the views from M2 — select the most illustrative one or two.

**Module View highlight**: 
- Key point: New graph tabs can be added by modifying Presentation layer only

**Runtime/C&C View highlight**:
- Key point: Audio thread / UI thread separation enables real-time performance

**Deployment View highlight**:
- Key point: RPi 5 as sole runtime target; AGC disabled requirement

### 2.4 Design Rationale for 1-2 Key Decisions

**Decision 1**: _______________ (e.g., Beat event onset vs peak detection)
- Options considered:
- Choice made:
- Why:
- Evidence from experiment:

**Decision 2**: _______________ (e.g., Threading model)
- Options considered:
- Choice made:
- Why:
- Evidence from experiment:

---

## Section 3: Experiments & Architecture Evaluation (~5 min)

### 3.1 Most Impactful Experiments

| Experiment | Question | Key Finding | Architecture Decision Made |
|------------|---------|-------------|---------------------------|
| EX-01: RPi Sample Rate | Can RPi sustain 96k sps? | ___k sps sustainable | Target sample rate set to ___ |
| EX-02: Beat Detection | Onset vs peak? | ___ method more stable | BeatDetector uses ___ |
| EX-___ | | | |

### 3.2 Architecture Evaluation

> Did we formally or informally evaluate the architecture? How?

| Method | Applied To | Finding |
|--------|-----------|---------|
| Scenario walkthrough | Extensibility (add new graph) | Required changes: ___ files |
| Performance measurement | End-to-end latency | Avg: ___ ms, Worst: ___ ms |
| Accuracy test vs WeiShi | Rate/Amplitude/Beat Error | Within tolerance: Yes / No |
| | | |

### 3.3 Unresolved Concerns
- [ ] *(list anything still open)*

---

## Section 4: Lessons Learned (~5 min)

### What Went Right

| Thing | Why It Worked |
|-------|---------------|
| | |
| | |

### What Went Wrong

| Thing | What Happened | Impact |
|-------|---------------|--------|
| | | |
| | | |

### What We Would Do Differently

| If We Did It Again | Why |
|-------------------|-----|
| | |
| | |

### Key Takeaways for Software Architecture

> 2-3 sentences connecting project experience to architecture principles

---

## Slide Outline (suggested)

| Slide # | Title | Content |
|---------|-------|---------|
| 1 | Title | Team, project name, date |
| 2 | System Overview | Watch → RPi → GUI diagram |
| 3 | Top QA Requirements | 2-3 QARs with measurability |
| 4 | Architecture: Module View | Layer diagram, extensibility highlight |
| 5 | Architecture: Runtime View | Thread model, pipeline, latency points |
| 6 | Key Design Decisions | 2 decisions with rationale |
| 7 | Experiment Results | Table: question → finding → decision |
| 8 | Architecture Evaluation | Latency numbers, accuracy, extensibility |
| 9 | Lessons Learned | What went right/wrong/differently |
| 10 | Demo Preview | Screenshot of GUI on RPi |

---

## Review Checklist

- [ ] QA requirements section covers high-priority drivers linked to architecture
- [ ] Architecture section includes at least one view with rationale
- [ ] Experiments section explains how results influenced architecture
- [ ] Lessons learned is honest and reflective
- [ ] Total content fits in 20 minutes (rehearse!)
