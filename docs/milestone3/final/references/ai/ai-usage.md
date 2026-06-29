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

## Implementation Phase

Code with repetitive structure and formula-based algorithm implementations were drafted by AI and then reviewed and adjusted by engineers. When translating domain knowledge — such as measurement formulas from Witschi documentation — into working code, AI assisted with interpreting the math and producing an initial implementation, which the team then validated and refined. The experiment log analysis tooling (`analyze_log.py`) was built the same way, turning a multi-step task into a single session.

## Documentation Phase

Experiment result documents, design rationale, and architecture notes were written with AI assistance in the same session as the implementation. A fixed technical experiment template was used: AI filled in the first draft, and the team refined the content — keeping documentation current across all three milestones rather than accumulating a backlog at the end.

## Outcome Assessment

| Area | AI Role | Outcome |
|------|---------|---------|
| Layer decomposition & module contracts | Surfaced options, flagged coupling issues | ✅ Effective — compressed multi-session design work into one session |
| Witschi formula → C++ translation | Drafted initial implementation | ✅ Effective — team focused on validation and refinement |
| Experiment log analysis tooling (`analyze_log.py`) | Generated initial script | ✅ Effective |
| ADR / Experiment document drafts | First-draft author using fixed templates | ✅ Effective — documentation stayed current across all 3 milestones |
| Risk assessment and experiment result judgment | Draft evaluation | ⚠️ Limited — AI drafts required mandatory team review and correction before being accepted |

The ⚠️ row is important: the team identified where AI cannot substitute for engineering judgment. Risk assessment and experiment interpretation require domain knowledge and accountability that remain with the engineers. AI drafts in these areas were treated as starting points, not conclusions.
