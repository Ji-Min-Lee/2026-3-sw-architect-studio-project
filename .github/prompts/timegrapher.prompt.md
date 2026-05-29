---
mode: ask
description: TimeGrapher 프로젝트 컨텍스트 로드 — 코드 분석, 아키텍처, 마일스톤 질문 시 사용
---

다음 프로젝트 참조 문서들을 읽고 질문에 답하거나 작업을 수행하세요.

## 읽어야 할 참조 문서

### 항상 읽기
- [프로젝트 개요](.claude/skills/time-grapher/references/project/overview.md)
- [마일스톤 및 산출물](.claude/skills/time-grapher/references/project/milestones.md)
- [주간 TODO 및 일정](.claude/skills/time-grapher/references/project/todo.md)

### 아키텍처/설계 질문 시 추가로 읽기
- [아키텍처 구조](.claude/skills/time-grapher/references/design/skill-architecture.md)
- [모듈 컨트랙트](.claude/skills/time-grapher/references/design/skill-module-contracts.md)
- [데이터 모델](.claude/skills/time-grapher/references/design/skill-data-model.md)
- [의존성 맵](.claude/skills/time-grapher/references/design/skill-dependency-map.md)

## 프로젝트 핵심 정보 (Quick Reference)

| 항목 | 값 |
|------|-----|
| 과제 | LG SW Architect Training × CMU MSE |
| 대상 플랫폼 | Raspberry Pi 5 + 8" Touchscreen (1280×800) |
| 개발 환경 | Windows/macOS + Qt Creator |
| Qt 버전 | Qt 6.11.1 MinGW 64-bit |
| 빌드 시스템 | CMake + MinGW Makefiles |
| 언어 | C++17 |
| 베이스 코드 | TimeGrapher_v10.5_Student |
| 팀 규모 | 7명 / 하루 2시간 |

## 마일스톤 요약

| 마일스톤 | 제출일 | 핵심 산출물 |
|----------|--------|------------|
| M1 | 2026-06-09 | Project Plan, Architectural Drivers, Risk Assessment, Planned Experiments, Architectural Approaches |
| M2 | 2026-06-22 | Experiment Results, Architecture Views (Module/C&C/Deployment), Construction Plan |
| M3 | 2026-07-01 | Final Demo (RPi), Team Presentation |

## 핵심 제약사항

- 모든 아키텍처 결정은 Quality Attribute Scenarios (QAS)로 정당화
- 새 그래프/분석은 **기존 코드 수정 없이** 추가 가능해야 함 (OCP)
- 오디오 처리와 GUI 렌더링은 서로 블록하지 않아야 함 (실시간 성능)

## 음향 이벤트 구조

```
T1 (A): Impulse pin → Pallet fork    ← Rate / Beat Error 계산에 사용
T2 (B): Escape wheel → Pallet stone  ← 불규칙, 미사용
T3 (C): Escape wheel locks + Fork → Banking pin  ← T1과 함께 Amplitude 계산
```

---

위 문서들을 읽은 후 사용자의 요청을 처리하세요.
