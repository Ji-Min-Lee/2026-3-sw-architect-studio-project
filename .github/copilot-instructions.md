# Project — Copilot Instructions

## Submodule: time-grapher skill

The `time-grapher` skill is tracked as a Git submodule in **two locations**:

| Path | Purpose |
|------|---------|
| `.claude/skills/time-grapher` | Active skill loaded by Claude Code |
| `.github/skills/time-grapher` | Skill registry pointer for GitHub |

Both must always point to the same commit. Whenever you make or pull changes to the `time-grapher` skill, update **both pointers** together:

```bash
# 1. Make changes inside the submodule and push
cd .claude/skills/time-grapher
git add <files> && git commit -m "..." && git push

# 2. Update both submodule pointers in the parent repo
cd /Users/jimin/Desktop/2026_architect/project
git submodule update --remote .claude/skills/time-grapher
git submodule update --remote .github/skills/time-grapher
git add .claude/skills/time-grapher .github/skills/time-grapher
git commit -m "chore: sync time-grapher submodule pointers to latest"
git push
```

Never update only one pointer — a mismatch causes the registry and the active skill to diverge.
