# Project — Copilot Instructions

## Submodule: time-grapher skill

The `time-grapher` skill is tracked as a Git submodule in **three locations**:

| Path | Purpose |
|------|---------|
| `.claude/skills/time-grapher` | Active skill loaded by Claude Code |
| `.agents/skills/time-grapher` | Active skill loaded by Codex |
| `.github/skills/time-grapher` | Skill registry pointer for GitHub |

**Rule: If any one of the three submodule pointers is updated, all three must be updated together before committing.**

All three must always point to the same commit. Whenever you make or pull changes to the `time-grapher` skill, update **all three pointers** together:

```bash
# 1. Make changes inside the submodule and push
cd .claude/skills/time-grapher
git add <files> && git commit -m "..." && git push

# 2. Update all three submodule pointers in the parent repo
cd /Users/jimin/Desktop/2026_architect/project
git submodule update --remote .claude/skills/time-grapher
git submodule update --remote .agents/skills/time-grapher
git submodule update --remote .github/skills/time-grapher
git add .claude/skills/time-grapher .agents/skills/time-grapher .github/skills/time-grapher
git commit -m "chore: sync time-grapher submodule pointers to latest"
git push
```

Never update only one pointer — a mismatch causes the registry and the active skill to diverge.

## Instruction File Sync

`CLAUDE.md`, `AGENTS.md`, and `.github/copilot-instructions.md` are the canonical instruction files for Claude Code, Codex, and GitHub Copilot respectively. They must always stay in sync.

**Rule: If any one of these three files is modified, update all three files with the same changes before committing.**

| File | Agent |
|------|-------|
| `CLAUDE.md` | Claude Code |
| `AGENTS.md` | Codex / OpenAI Agents |
| `.github/copilot-instructions.md` | GitHub Copilot |

Each file may have agent-specific wording (e.g. tool names, skill paths) but the structure and rules must be identical across all three.

## Documentation Convention

When writing files under `docs/`, refer to the documentation convention defined in the `/time-grapher` skill.
