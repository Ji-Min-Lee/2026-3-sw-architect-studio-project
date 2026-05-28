# TimeGrapher Skill — Dependency Map

## 의존 관계 요약

| 항목 | 값 |
|------|-----|
| `dependency-skills` | `[]` (없음) |
| `related-skills` | 없음 |
| 순환 의존 | 0건 |
| 외부 MCP/API | 없음 |

## 관계도

```mermaid
graph LR
    TG[time-grapher\n(this skill)]

    subgraph "dependency-skills (required)"
        NONE1[없음]
    end

    subgraph "related-skills (optional)"
        NONE2[없음]
    end

    TG -.->|의존 없음| NONE1
    TG -.->|연관 없음| NONE2
```

## 근거

이 스킬은 **Context Provider** 유형으로:
- 외부 API 호출 없음
- 다른 스킬 선행 로드 불필요
- 자체 `references/` 파일만으로 동작 완결

## 향후 확장 고려

향후 다음 스킬이 추가될 경우 `related-skills`에 등록을 검토한다:

| 잠재 연관 스킬 | 사유 |
|----------------|------|
| `ai-tools-skill` | Jira(ALM) 이슈·GitLab MR 연동 필요 시 |
| (미정) `qt-dev-skill` | Qt C++ 개발 가이드 스킬 생성 시 |
