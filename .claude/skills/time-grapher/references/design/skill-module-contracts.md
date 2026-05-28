# TimeGrapher Skill — Module Contracts

## 모듈 목록

| 모듈 | 파일 | 역할 |
|------|------|------|
| index-hub | `SKILL.md` | 트리거·목적·참고 문서 인덱스 |
| project-context | `references/project/overview.md` | 프로젝트 배경·시스템 구성·QA 제공 |
| milestone-context | `references/project/milestones.md` | 마일스톤별 산출물·체크 포인트 제공 |
| todo-context | `references/project/todo.md` | 주차별 TODO·일정 제공 |

---

## index-hub (`SKILL.md`)

### Input
```
사용자 요청 (자연어)
트리거 키워드: timegrapher / time grapher / 과제 / 마일스톤 / milestone /
              아키텍처 과제 / watch / 시계 / acoustic / raspberry pi 과제 / Qt 과제
```

### Output
```
- 트리거 감지 시: 스킬 활성화 + 적절한 참고 모듈로 라우팅
- 직접 응답 불가: 상세 내용은 references/로 위임
```

### Failure
```
- 트리거 미감지: 스킬 미로드 (정상)
- 참고 파일 없음: references/ 하위 파일 경로 오류 → 사용자에게 안내
```

---

## project-context (`overview.md`)

### Input
```
요청 의도: 프로젝트 배경 / 시스템 구성 / 기술 스택 / 품질 속성 / 구현 대상 그래프
```

### Output
```
- 프로젝트 배경 및 목표 (1~2문단)
- 시스템 구성도 (텍스트 다이어그램)
- 하드웨어 스펙 표
- 품질 속성 5가지 (QA, 목표, 측정 방법)
- 구현 대상 그래프 11종 목록
```

### Failure
```
- 하드웨어 상세 스펙 요청 → assets/TimeGrapher GUI Set Up Instructions.pdf 안내
- 계산 공식 요청 → assets/TimeGrapher Equations_v0.docx.pdf 안내
```

---

## milestone-context (`milestones.md`)

### Input
```
요청 의도: 특정 마일스톤 산출물 / 제출 기한 / 심사 기준 / 발표 준비
```

### Output
```
- 요청한 마일스톤(M1/M2/M3)의 산출물 목록
- 제출 기한 (날짜 + 요일)
- 심사 체크 포인트 (Mentor 질문 기준)
- M3: 발표 항목 및 데모 요건
```

### Failure
```
- 채점 기준 세부 요청 → assets/LG SW Architect Final Demo Grading Score Sheet and Ruberic.pdf 안내
```

---

## todo-context (`todo.md`)

### Input
```
요청 의도: 이번 주 할 일 / 특정 주차 TODO / 전체 일정 / 완료 항목 확인
```

### Output
```
- 현재 날짜 기준 해당 주차의 TODO 체크리스트
- 완료([x]) / 미완료([ ]) 항목 구분
- Gantt 차트 (전체 일정)
```

### Failure
```
- 현재 날짜 불명확 시 → 사용자에게 날짜 확인 후 응답
```
