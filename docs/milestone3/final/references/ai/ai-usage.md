# How We Used AI

## Overview

AI (Claude Code) was used as a development assistant throughout the project. It supported design review, code drafting, and documentation — helping the team move faster through repetitive tasks and explore design options more quickly, especially during the architecture phase.

Beyond using AI as a tool, the team designed **how** to use AI: a shared Claude Code skill was built and maintained across all three milestones to give every team member a consistent, context-aware assistant.

## Claude Code Skill — Designed Collaboration Process

The team built and maintained a shared Claude Code skill that loaded project context automatically for all seven team members. This skill provided three structural benefits:

1. **Templates embedded in the skill** — Architecture view, ADR, and experiment document templates were provided through the skill. Seven team members could write independently and still produce structurally consistent documentation across all three milestones.

2. **Domain knowledge loaded into the skill** — Requirements documents, the Witschi manual, course materials, and grading rubrics were included in the skill context. This allowed AI to assist with domain-specific tasks (e.g., translating Witschi measurement formulas into working C++ code) without team members having to re-paste reference material in every session.

3. **Milestone feedback accumulated in the skill** — M1 and M2 instructor feedback was retained in the skill so that past corrections were automatically reflected in subsequent work, rather than being repeated or forgotten across sessions.

This reframes the narrative from "we used AI as a tool" to "we designed how to use AI" — a structured collaboration process, not just ad-hoc prompting.

## Design Phase

When working through layer decomposition, defining module contracts, and identifying thread-safety requirements, the team used AI to rapidly surface options and flag potential issues before committing to a direction. Comparing design alternatives in conversation with AI compressed what would otherwise be multi-session discussions into a single working session.

AI was particularly effective at **maintaining internal consistency across design artifacts**. When a module contract changed, AI could immediately identify which views, ADRs, and experiment documents needed corresponding updates — work that would otherwise slip through when done manually across seven team members.

## Implementation Phase

Code with repetitive structure and formula-based algorithm implementations were drafted by AI and then reviewed and adjusted by engineers. When translating domain knowledge — such as measurement formulas from Witschi documentation — into working code, AI assisted with interpreting the math and producing an initial implementation, which the team then validated and refined. The experiment log analysis tooling (`analyze_log.py`) was built the same way, turning a multi-step task into a single session.

**Template-driven code was where AI performed most reliably.** Given a clear pattern — a new graph tab following `BaseGraphTab`, a new ADR following the standard structure, a new experiment document following the fixed template — AI produced correct, conformant output on the first attempt with minimal correction. The more constrained the structure, the less rework was needed.

## Documentation Phase

Experiment result documents, design rationale, and architecture notes were written with AI assistance in the same session as the implementation. A fixed technical experiment template was used: AI filled in the first draft, and the team refined the content — keeping documentation current across all three milestones rather than accumulating a backlog at the end.

**The skill-based history mechanism proved effective for maintaining consistency across sessions.** Ground rules, instructor feedback, and team conventions accumulated in the skill over time. Once recorded, they were automatically applied in every subsequent session without team members needing to re-state them — reducing the cost of onboarding and catching recurring mistakes before they reached review.

## Diagram Generation

AI-generated diagrams were the area of highest friction. Left unconstrained, the outputs consistently suffered from overlapping edges, text overflowing bounding boxes, and uneven layout balance — requiring manual post-editing in every case.

Two practices reduced (but did not eliminate) rework:

1. **Providing a best-practice reference** — supplying an example diagram with a comparable structure gave AI a layout model to follow and improved structural coherence.
2. **Specifying components explicitly** — when the exact element names and relationship types were given upfront, the output was cleaner and closer to usable.

Even with both practices, minor layout defects (overlapping lines, label overflow) persisted and required direct correction by the team. Diagram generation remains an area where AI accelerates a first draft but cannot replace human refinement.

## Outcome Assessment

The grading rubric (Area 7) identifies six categories where AI can contribute to software development: **design, coding, debugging, testing, documentation, and analysis**. The table below maps our actual experience against these categories.

| Category (rubric) | Task | AI Role | Result |
|-------------------|------|---------|--------|
| Design | Layer decomposition & module contracts | Surfaced options, flagged coupling issues | ✅ Can help — compressed multi-session design work into one session |
| Design | Cross-artifact consistency checks | Identified which ADRs/views needed updates when contracts changed | ✅ Can help — effective across 7-member team |
| Coding | Witschi formula → C++ translation | Drafted initial implementation from domain doc | ✅ Can help — team focused effort on validation, not transcription |
| Coding | Template-conformant code (graph tabs, ADRs) | Generated conformant first draft from pattern | ✅ Can help — the more constrained the structure, the less rework needed |
| Analysis | Experiment log analysis (`analyze_log.py`) | Generated initial analysis script | ✅ Can help — turned a multi-step task into a single session |
| Documentation | ADR / experiment / architecture view drafts | First-draft author using embedded templates | ✅ Can help — documentation stayed current across all 3 milestones |
| Documentation | Diagram generation | First-draft layout from reference + component spec | ⚠️ Limited — structural output improved with guidance, but layout defects (overlapping edges, label overflow) required manual correction every time |
| Testing | Unit test case generation | Generated test cases from a domain knowledge checklist | ✅ Can help — AI translated domain requirements (beat detection rules, measurement formulas) into concrete test cases that the team then validated |

**On the ⚠️ row:** diagram generation is a layout problem. AI lacks spatial awareness and does not reliably enforce alignment, spacing, or non-overlapping edges. The quality gap is mechanical and predictable — it can be partially mitigated with tighter prompts (supplying a reference example, specifying components explicitly), but not eliminated.
