# TimeGrapher Skill — Architecture

## 스킬 유형

`artifact_type: skill` / `skill_scope: single-skill` / `target_agent: claude`

이 스킬은 **Context Provider** 유형으로, 외부 API 호출이나 코드 실행 없이
프로젝트 컨텍스트를 Claude에게 주입해 정확한 응답을 유도한다.

## 모듈 관계도

```mermaid
graph TD
    subgraph time-grapher-skill
        SKILL[SKILL.md\nIndex Hub]
        subgraph references/project
            OV[overview.md\n프로젝트 개요]
            MS[milestones.md\n마일스톤 산출물]
            TD[todo.md\nTODO + 일정]
        end
        subgraph references/design
            SA[skill-architecture.md]
            MC[skill-module-contracts.md]
            DM[skill-data-model.md]
            DP[skill-dependency-map.md]
        end
        subgraph evals
            EV[evals.json]
        end
        EVAL_S[evaluation-summary.md]
    end

    subgraph assets
        PDF1[Project Plan.pdf]
        PDF2[Equations.pdf]
        PDF3[KickOff.pdf]
        ZIP[TimeGrapher_v10.5.zip]
    end

    SKILL --> OV
    SKILL --> MS
    SKILL --> TD
    SKILL --> SA
    SKILL --> MC
    SKILL --> DM
    SKILL --> DP

    OV -.참고.-> PDF1
    MS -.참고.-> PDF1
    TD -.참고.-> PDF1
    OV -.참고.-> PDF2
```

## 처리 흐름

```mermaid
flowchart LR
    U([사용자 요청]) --> T{트리거\n감지}
    T -- "과제/milestone/\ntimegrapher/시계" --> L[스킬 로드]
    T -- 미감지 --> X([일반 응답])
    L --> R{요청 유형\n분류}
    R -- 프로젝트 개요 --> OV[overview.md\n로드]
    R -- 마일스톤/산출물 --> MS[milestones.md\n로드]
    R -- 할 일/일정 --> TD[todo.md\n로드]
    R -- 복합 질문 --> ALL[전체 참고 로드]
    OV & MS & TD & ALL --> A([컨텍스트 기반\n응답 생성])
```

## 설계 원칙

| 원칙 | 적용 내용 |
|------|-----------|
| **SRP** | SKILL.md는 인덱스 역할만, 상세 내용은 references/로 분리 |
| **OCP** | 새 마일스톤·산출물 추가 시 references/project/ 파일만 수정 |
| **DIP** | assets/*.pdf에 직접 의존하지 않고, 정리된 references/를 통해 접근 |
