---
name: time-grapher
description: >-
  LG SW Architect 과정 TimeGrapher 프로젝트 컨텍스트 스킬.
  프로젝트 요약·마일스톤 산출물·TODO 리스트·기술 스펙을 제공한다.
  Trigger when: "timegrapher", "time grapher", "과제", "마일스톤", "milestone",
  "아키텍처 과제", "watch", "시계", "acoustic", "raspberry pi 과제", "Qt 과제".
metadata:
  version: 0.1.0
  last-updated-at: 2026-05-28
  dependency-skills: []
---

# TimeGrapher Project Skill

> **LG Software Architecture Training Program × CMU MSE**  
> "From Tick to Trace: Real-Time Acoustic Analysis"

## 목적

LG SW Architect 과정 TimeGrapher 프로젝트의 컨텍스트를 Claude에게 제공한다.
질문·계획·리뷰 시 아래 참고 문서를 로드해 정확한 프로젝트 정보를 기반으로 응답한다.

---

## 참고 문서 (References)

### 프로젝트

| 문서 | 내용 |
|------|------|
| [프로젝트 개요](references/project/overview.md) | 배경, 시스템 구성, 품질 속성, 기술 스택 |
| [마일스톤 산출물](references/project/milestones.md) | M1/M2/M3 산출물 및 체크 포인트 |
| [TODO 리스트](references/project/todo.md) | 주차별 할 일 + 전체 일정 |

### 설계

| 문서 | 내용 |
|------|------|
| [아키텍처](references/design/skill-architecture.md) | 스킬 구조 및 모듈 관계도 |
| [모듈 계약](references/design/skill-module-contracts.md) | 입력/출력/실패 계약 |
| [데이터 모델](references/design/skill-data-model.md) | 프로젝트 정보 데이터 구조 |
| [의존 관계](references/design/skill-dependency-map.md) | 스킬 의존·연관 관계 |

---

## 자산 (assets/)

| 파일 | 용도 |
|------|------|
| `Time Grapher Project Plan (Draft).pdf` | 전체 요구사항·산출물 명세 (주 참고) |
| `LG 2026 Arch Kick Off Brief.pdf` | 킥오프 슬라이드 |
| `TimeGrapher Equations_v0.docx.pdf` | 측정값 계산 공식 |
| `TimeGrapher GUI Set Up Instructions.pdf` | Qt 환경 셋업 |
| `Witschi-Training-Course.pdf` | 시계 측정 개념 교육 (pp.14-19 필독) |
| `Witschi Chronoscope X1 G3 Instruction Manual.pdf` | GUI 디스플레이 모드 참고 |
| `LG SW Architect Final Demo Grading Score Sheet and Ruberic.pdf` | 채점 기준 |
| `TimeGrapher_v10.5_Student.zip` | 기존 코드베이스 |
