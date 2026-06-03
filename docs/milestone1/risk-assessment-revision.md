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
| TR-01 | Beat 감지 부정확 — 시계 기종마다 패턴 상이 / Beat detection inaccurate — pattern varies per watch | H | H | OI-01 |
| TR-02 | AGC 재활성화로 신호 왜곡 — 재부팅 후 측정값 신뢰 불가 / AGC re-enabled after reboot corrupts signal | M | H | OI-02 |
| TR-03 | RPi에서 96k sps + Qt GUI 동시 처리 성능 미검증 / RPi throughput at 96k sps with Qt GUI unverified | H | H | OI-03 |
| TR-04 | Qt 11개 그래프 탭 동시 렌더링 시 FPS 저하 / Qt FPS drops with 11 active graph tabs | M | H | OI-03 |
| TR-05 | `MainWindow.cpp` God Object — 3명 동시 수정 시 merge conflict 상시 / God Object causes constant merge conflicts | H | H | OI-04 |
| TR-06 | macOS 빌드 분기 없음 — macOS 팀원 2명 오디오 동작 불가 / No macOS branch — audio broken for 2 devs | H | M | OI-05 |
| TR-07 | FFT 스펙트로그램 RPi CPU 과부하 / FFT spectrogram CPU overload on RPi | M | M | OI-03 |

### 비기술 리스크 / Non-Technical Risks

| ID | 리스크 / Risk | Prob | Impact | 연관 이슈 / Issue |
|----|-------------|:----:|:------:|:----------------:|
| NR-01 | 코딩팀-아키텍처팀 경계 불명확 — 설계가 구현에 미반영 / Coding/architecture boundary unclear | H | H | OI-06 |
| NR-02 | 11개 그래프 전부 구현 시 핵심 기능 품질 저하 / Scope overextension in 5-week schedule | H | M | OI-07 |
| NR-03 | WeiShi 1000 접근 지연 — 정확도 기준 없이 검증 불가 / Delayed reference device — accuracy unverifiable | M | H | OI-08 |
| NR-04 | 팀 전반의 영어 커뮤니케이션 부담 — 설계 결정사항 전달 누락 위험 / English communication overhead — risk of design decisions not reaching all members | M | H | OI-09 |

---

## 2. 미해결 이슈 / Open Issues

**한국어**  
아래 이슈들은 최종 데모 품질에 직접 영향을 미친다.

**English**  
Each open issue directly affects final demo outcome.

| ID | 미해결 이슈 / Open Issue | 연관 리스크 / Risk | 미해결 시 결과 / If Unresolved |
|----|------------------------|:-----------------:|-------------------------------|
| OI-01 | T1 감지 기준점 미결정 (onset vs peak) / T1 detection reference point undecided | TR-01 | Rate/Amplitude 측정값 신뢰 불가 / Measurement values unreliable |
| OI-02 | RPi 재부팅 후 AGC 비활성화 지속 여부 미확인 / AGC-off persistence after reboot unverified | TR-02 | 모든 측정값 신뢰 불가 / All measurements unreliable |
| OI-03 | GUI 실행 중 RPi 최대 성능 미측정 (sps, FPS, FFT) / RPi performance under load unverified | TR-03, TR-04, TR-07 | 실시간 처리 불가 — 데모에서 오디오·렌더링 실패 / Real-time processing fails at demo |
| OI-04 | `MainWindow.cpp` 리팩토링 범위 미확정 / Refactoring scope not confirmed | TR-05 | 신규 그래프 추가마다 전체 파일 충돌 / Every new graph triggers whole-file conflicts |
| OI-05 | macOS 빌드 분기 처리 방안 미결정 / macOS build strategy undecided | TR-06 | macOS 팀원 2명 로컬 오디오 테스트 불가 / 2 macOS devs cannot test audio locally |
| OI-06 | 코딩팀-아키텍처팀 동기화 프로세스 미확정 / Coding/arch sync process not defined | NR-01 | 설계 결정이 구현에 미반영 — M2 전 전면 재작업 위험 / Design decisions not implemented — rework risk before M2 |
| OI-07 | 11개 그래프 우선순위 미분류 / Graph priority not classified | NR-02 | 핵심 기능 미완성 상태로 데모 진행 위험 / Core features incomplete at demo |
| OI-08 | WeiShi 1000 대여 일정 미확보 / WeiShi 1000 loan schedule not secured | NR-03 | ±5 s/d 오차 기준 충족 여부 확인 불가 / Cannot verify ±5 s/d accuracy target |
| OI-09 | 오프라인 결정사항 영어 공유 프로세스 없음 / No process to share offline decisions in English | NR-04 | 영어권 팀원의 잘못된 구현 위험 / Risk of wrong implementation by English-speaking member |

---

## 3. 대응 액션 및 계획 실험 / Actions and Planned Experiments

**한국어**  
기술 리스크 중 실험이 필요한 항목은 Planned Experiment로, 나머지는 즉시 액션으로 해소한다.

**English**  
Technical risks with uncertainty are resolved through Planned Experiments; others are addressed by immediate actions.

| 이슈 / Issue | 대응 / Action | 완료 기준 / Done When |
|:-----------:|--------------|---------------------|
| OI-01, OI-02 | **EX-01**: onset vs peak 비교 실험, WeiShi 1000 기준 오차 측정 / Compare onset vs peak, measure error vs WeiShi 1000 | 감지 방식 결정 + 오차 margin 수치 확보 / Method decided + error margin measured |
| OI-03 | **EX-02**: RPi에서 96k/48k sps × Qt GUI 처리 시간 및 FPS 실측 / Measure processing time and FPS at 96k/48k sps with Qt GUI | sps별 처리 시간 + FPS 확보; 48k 폴백 여부 결정 / Time and FPS measured; fallback decided |
| OI-04 | `AudioCapture` / `MeasurementEngine` 분리 범위 확정 후 4-layer 모듈 경계 문서화 / Confirm split scope and document 4-layer boundaries | 모듈 경계 확정 / Module boundaries locked |
| OI-05 | `Q_OS_MAC` 분기 추가 또는 `MacAudio` 스텁 생성 / Add `Q_OS_MAC` branch or `MacAudio` stub | macOS에서 빌드 성공 / Build succeeds on macOS |
| OI-06 | 매일 오후 동기화 회의 고정 + Teams 채널 소통 / Fix daily afternoon sync meeting + communicate via Teams channel | 첫 동기화 회의 완료 / First sync meeting held |
| OI-07 | 11개 그래프를 Core / Required / Stretch로 분류 / Classify graphs as Core / Required / Stretch | 팀 합의 완료 / Team agreement reached |
| OI-08 | WeiShi 1000 대여 일정 조기 확보 / Secure WeiShi 1000 loan schedule early | 대여 일정 확정 / Loan date confirmed |
| OI-09 | 오프라인 회의 결정사항 영문 요약 Teams 공유 / Share English summary of offline decisions on Teams | 매 회의 후 영문 요약 업로드 / English summary posted after each meeting |
