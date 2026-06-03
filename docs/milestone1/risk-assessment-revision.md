# 리스크 평가 / Risk Assessment

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09

> **확률 / Prob**: H / M / L | **영향 / Impact**: H / M / L

---

## 1. 기술 및 비기술 리스크 / Technical and Non-Technical Risks

**한국어**  
각 리스크를 발생 가능성(Prob)과 영향도(Impact) 기준으로 H/M/L로 평가한다.

**English**  
Each risk is assessed on a High/Medium/Low scale for both Probability and Impact.

### 기술 리스크 / Technical Risks

| ID | 리스크 / Risk | Prob | Impact | 연관 이슈 / Issue |
|----|-------------|:----:|:------:|:----------------:|
| TR-01 | C 이벤트 placement 설정 미최적화 — `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` 중 어느 설정이 WeiShi 1000 기준 오차를 최소화하는지 미검증 / C-event placement not optimized — which of `TG_C_PLACEMENT_PEAK` vs `TG_C_PLACEMENT_ONSET` minimizes error vs WeiShi 1000 is unverified | H | H | OI-01 |
| TR-02 | AGC 재활성화로 신호 왜곡 — 재부팅 후 측정값 신뢰 불가 / AGC re-enabled after reboot corrupts signal | M | H | OI-02 |
| TR-03 | RPi에서 96k sps + Qt GUI 동시 처리 성능 미검증 / RPi throughput at 96k sps with Qt GUI unverified | H | H | OI-03 |
| TR-04 | Qt 11개 그래프 탭 동시 렌더링 시 FPS 저하 / Qt FPS drops with 11 active graph tabs | M | H | OI-03 |
| TR-05 | `MainWindow.cpp` God Object — 3명 동시 수정 시 merge conflict 상시 / God Object causes constant merge conflicts | H | H | OI-04 |
| TR-06 | macOS 빌드 분기 없음 — macOS 팀원 2명 오디오 동작 불가 / No macOS branch — audio broken for 2 devs | H | M | OI-05 |
| TR-07 | FFT 스펙트로그램 RPi CPU 과부하 / FFT spectrogram CPU overload on RPi | M | M | OI-03 |
| TR-08 | WeiShi 1000 & RPi 동시 측정 환경 구축 불확실 — 동일 입력으로 accuracy 비교하려면 두 장비가 같은 시계 신호를 동시에 수집해야 함 / Simultaneous WeiShi 1000 & RPi measurement setup uncertain — same input required for accuracy comparison | H | H | OI-09 |

### 비기술 리스크 / Non-Technical Risks

| ID | 리스크 / Risk | Prob | Impact | 연관 이슈 / Issue |
|----|-------------|:----:|:------:|:----------------:|
| NR-01 | 코딩팀-아키텍처팀 경계 불명확 — 설계가 구현에 미반영 / Coding/architecture boundary unclear | H | H | OI-06 |
| NR-02 | 11개 그래프 전부 구현 시 핵심 기능 품질 저하 / Scope overextension in 5-week schedule | H | M | OI-07 |
| NR-03 | 팀 전반의 영어 커뮤니케이션 부담 — 설계 결정사항 전달 누락 위험 / English communication overhead — risk of design decisions not reaching all members | M | H | OI-08 |

---

## 2. 미해결 이슈 / Open Issues

**한국어**  
아래 이슈들은 최종 데모 품질에 직접 영향을 미친다.

**English**  
Each open issue directly affects final demo outcome.

| ID | 미해결 이슈 / Open Issue | 연관 리스크 / Risk | 미해결 시 결과 / If Unresolved |
|----|------------------------|:-----------------:|-------------------------------|
| OI-01 | C 이벤트 placement 설정 미결정 — onset/peak 감지는 코드에 구현되어 있으나(`tg_c_placement_t`), 어느 설정이 WeiShi 1000 기준 오차를 최소화하는지 미검증 / C-event placement undecided — onset/peak detection implemented (`tg_c_placement_t`) but optimal setting vs WeiShi 1000 unverified | TR-01 | Rate/Amplitude 측정값 신뢰 불가 / Measurement values unreliable |
| OI-02 | RPi 재부팅 후 AGC 비활성화 지속 여부 미확인 / AGC-off persistence after reboot unverified | TR-02 | 모든 측정값 신뢰 불가 / All measurements unreliable |
| OI-03 | GUI 실행 중 RPi 최대 성능 미측정 (sps, FPS, FFT) / RPi performance under load unverified | TR-03, TR-04, TR-07 | 실시간 처리 불가 — 데모에서 오디오·렌더링 실패 / Real-time processing fails at demo |
| OI-04 | `MainWindow.cpp` 리팩토링 범위 미확정 / Refactoring scope not confirmed | TR-05 | 신규 그래프 추가마다 전체 파일 충돌 / Every new graph triggers whole-file conflicts |
| OI-05 | macOS 빌드 분기 처리 방안 미결정 / macOS build strategy undecided | TR-06 | macOS 팀원 2명 로컬 오디오 테스트 불가 / 2 macOS devs cannot test audio locally |
| OI-06 | 코딩팀-아키텍처팀 동기화 프로세스 미확정 / Coding/arch sync process not defined | NR-01 | 설계 결정이 구현에 미반영 — M2 전 전면 재작업 위험 / Design decisions not implemented — rework risk before M2 |
| OI-07 | 11개 그래프 우선순위 미분류 / Graph priority not classified | NR-02 | 핵심 기능 미완성 상태로 데모 진행 위험 / Core features incomplete at demo |
| OI-08 | 팀 내 소통 언어 불일치 — 산출물 작성 기준 미정 / Language mismatch — no standard for deliverable writing (교수 외국인, 발표·마일스톤 제출 영어 필수) | NR-03 | 영어권 팀원 이해 누락 + 마일스톤 제출물 품질 저하 / English-speaking member misses context + milestone quality degraded |
| OI-09 | WeiShi 1000 & RPi 동시 측정 환경 구축 가능 여부 미검증 / Simultaneous WeiShi 1000 & RPi measurement setup not verified | TR-08 | 동일 입력 기반 accuracy 비교 불가 — ±5 s/d 기준 충족 여부 확인 불가 / Cannot compare accuracy on same input — ±5 s/d target unverifiable |

---

## 3. 대응 액션 및 계획 실험 / Actions and Planned Experiments

**한국어**  
기술 리스크 중 실험이 필요한 항목은 Planned Experiment로, 나머지는 즉시 액션으로 해소한다.

**English**  
Technical risks with uncertainty are resolved through Planned Experiments; others are addressed by immediate actions.

| 이슈 / Issue | 대응 / Action | 완료 기준 / Done When |
|:-----------:|--------------|---------------------|
| OI-02 | **EX-01** 전처리 단계로 통합 — RPi 재부팅 후 AlsaMixer에서 AGC 상태 재확인; off 유지 시 계속 진행, on으로 복원 시 영구 비활성화 방법(`alsactl store` 등) 적용 / Integrated into EX-01 pre-step — re-check AGC state in AlsaMixer after RPi reboot; if restored to on, apply permanent disable method (`alsactl store`, etc.) | 재부팅 후 AGC off 지속 확인 / AGC-off persistence confirmed after reboot |
| OI-01 | **EX-03**: `tg_c_placement_t` 설정별 오차 비교 실험 — `TG_C_PLACEMENT_PEAK` / `TG_C_PLACEMENT_ONSET` 각각으로 동일 시계 측정 후 WeiShi 1000 기준 Rate/Beat Error 오차 비교 (onset/peak 감지 자체는 코드에 구현 완료, 최적 설정 선택이 목적) / Compare error by `tg_c_placement_t` setting — measure same watch with PEAK vs ONSET placement, compare Rate/Beat Error vs WeiShi 1000 (detection already implemented; goal is selecting optimal setting) | placement 설정 결정 + 오차 margin 수치 확보 / Placement setting decided + error margin measured |
| OI-03 | **EX-01**: RPi에서 96k/48k sps × Qt GUI 처리 시간 및 FPS 실측 / Measure processing time and FPS at 96k/48k sps with Qt GUI | sps별 처리 시간 + FPS 확보; 48k 폴백 여부 결정 / Time and FPS measured; fallback decided |
| OI-04 | `AudioCapture` / `MeasurementEngine` 분리 범위 확정 후 4-layer 모듈 경계 문서화 / Confirm split scope and document 4-layer boundaries | 모듈 경계 확정 / Module boundaries locked |
| OI-05 | `Q_OS_MAC` 분기 추가 또는 `MacAudio` 스텁 생성 / Add `Q_OS_MAC` branch or `MacAudio` stub | macOS에서 빌드 성공 / Build succeeds on macOS |
| OI-06 | 매일 오후 동기화 회의 고정 + Teams 채널 소통 / Fix daily afternoon sync meeting + communicate via Teams channel | Project Plan 작성 시 코딩팀-아키텍처팀 역할 경계 확정 및 문서화 완료 / Role boundaries confirmed and documented in Project Plan |
| OI-07 | 11개 그래프를 Core / Required / Stretch로 분류 / Classify graphs as Core / Required / Stretch | 11개 그래프 우선순위가 Project Plan에 Core / Required / Stretch로 분류 반영됨 / Graph priorities classified as Core / Required / Stretch and reflected in Project Plan |
| OI-08 | 모든 산출물을 한영 병기로 작성; 마일스톤 제출·발표는 영어 기준 / Write all deliverables in bilingual (KO/EN); milestone submissions and presentations in English | 작성 기준 합의 완료 + M1 제출 산출물 전체가 한/영 병기 규칙 준수 확인됨 / Writing standard agreed + all M1 deliverables verified to comply with bilingual rule |
| OI-09 | **EX-02**: WeiShi 1000 & RPi 동시 측정 환경 구축 실험 — 마이크 분기 또는 순차 측정 방식 검토 / Experiment simultaneous measurement setup — evaluate mic splitter or sequential method | 동시 측정 가능 여부 확인 + 방식 결정 / Setup feasibility confirmed + method decided |
