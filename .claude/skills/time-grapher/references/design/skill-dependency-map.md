# TimeGrapher Skill — Dependency Map

## Dependency Summary

| Item | Value |
|------|-------|
| `dependency-skills` | `[]` (none) |
| `related-skills` | none |
| Circular dependencies | 0 |
| External MCP/API | none |

## Relationship Diagram

```mermaid
graph LR
    TG[time-grapher\n(this skill)]

    subgraph "dependency-skills (required)"
        NONE1[none]
    end

    subgraph "related-skills (optional)"
        NONE2[none]
    end

    TG -.->|no dependency| NONE1
    TG -.->|no association| NONE2
```

## Rationale

This skill is a **Context Provider** type:
- No external API calls
- No prerequisite skill loading required
- Fully self-contained using only its own `references/` files

## Future Extension Considerations

If the following skills are added in the future, consider registering them under `related-skills`:

| Potential Related Skill | Reason |
|------------------------|--------|
| `ai-tools-skill` | If Jira (ALM) issue or GitLab MR integration is needed |
| (TBD) `qt-dev-skill` | If a Qt C++ development guide skill is created |
