# Project — Codex Instructions

## Submodule: time-grapher skill

The `time-grapher` skill is tracked as a Git submodule in **two locations**:

| Path | Purpose |
|------|---------|
| `.Codex/skills/time-grapher` | Active skill loaded by Codex |
| `.github/skills/time-grapher` | Skill registry pointer for GitHub |

Both must always point to the same commit. Whenever you make or pull changes to the `time-grapher` skill, update **both pointers** together:

```bash
# 1. Make changes inside the submodule and push
cd .Codex/skills/time-grapher
git add <files> && git commit -m "..." && git push

# 2. Update both submodule pointers in the parent repo
cd /Users/jimin/Desktop/2026_architect/project
git submodule update --remote .Codex/skills/time-grapher
git submodule update --remote .github/skills/time-grapher
git add .Codex/skills/time-grapher .github/skills/time-grapher
git commit -m "chore: sync time-grapher submodule pointers to latest"
git push
```

Never update only one pointer — a mismatch causes the registry and the active skill to diverge.

## Documentation Convention

`docs/` 하위 문서 작성 시 `/time-grapher` 스킬의 문서 컨벤션을 참조한다.
