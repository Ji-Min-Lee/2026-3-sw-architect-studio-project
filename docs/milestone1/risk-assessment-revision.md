# 리스크 평가 / Risk Assessment

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09 | **상태 / Status**: [ ] 초안 Draft / [ ] 최종 Final

> **발생 가능성 / Probability**: H=높음 High / M=보통 Medium / L=낮음 Low  
> **영향도 / Impact**: H=높음 High / M=보통 Medium / L=낮음 Low

---

## 팀 컨텍스트 / Team Context

| 항목 / Item | 내용 / Details |
|-------------|---------------|
| 팀 구성 / Members | 7명: 이지민, 신경진, 반규대, 송태준, 신성호, 신동호, Hung Son Tong |
| 팀 구조 / Structure | 코딩팀 Coding: 신성호, Hung Son Tong, 신경진 / 아키텍처팀 Architecture: 이지민, 반규대, 송태준, 신동호 |
| 개발 OS / Dev OS | macOS 2명 / Windows 4명 / Linux 1명 |
| 주요 도구 / Tools | Qt Creator (최신 버전 / latest), VSCode, GitHub |
| 빌드 방식 / Build | 현재 로컬 빌드 → 추후 SSH로 RPi 내부 네이티브 빌드 예정 / Local builds now → RPi native build via SSH later |
| 하드웨어 / Hardware | Raspberry Pi 보유 — 터치스크린 (DC 12V=2A), 아직 미가동 / RPi in hand — touchscreen (DC 12V=2A), not yet booted |
| 소통 채널 / Comms | 오프라인 위주 + KakaoTalk + Microsoft Teams |
| 멘토 미팅 / Mentor | M1 이전 미진행 / No meeting held before M1 |

---

## 1. 기술 및 비기술 리스크 / Technical and Non-Technical Risks

**한국어**  
각 리스크를 발생 가능성(Probability)과 영향도(Impact) 기준으로 H/M/L 척도로 평가한다.

**English**  
Each risk is assessed on a High/Medium/Low scale for both Probability and Impact.

### 1-1. 기술 리스크 / Technical Risks

| ID | 리스크 / Risk | Prob | Impact | 근거 / Rationale |
|----|--------------|:----:|:------:|-----------------|
| TR-01 | RPi 5에서 96k sps 실시간 처리 불가 — CPU 병목 / RPi 5 cannot sustain 96k sps realtime | H | H | 오디오 캡처 + DSP + Qt GUI를 동시 수행 시 CPU 과부하 가능성이 실험 전까지 미확인 / CPU overload risk unverified before benchmark |
| TR-02 | T1/T3 이벤트 감지 부정확 또는 불안정 — 시계 기종·환경마다 패턴 상이 / Beat event detection inaccurate — pattern varies per watch & environment | H | H | threshold 일반화 어려움; 정확도 검증 전 Rate/Amplitude 오차 margin 미확인 / Hard to generalize threshold; error margin unverified |
| TR-03 | 시스템 재시작 후 AGC 재활성화로 신호 진폭 왜곡 — 측정값 재현성 손실 / AGC re-enabled after reboot distorts signal amplitude | M | H | AGC 활성 상태면 Beat event 타이밍 변형 → 모든 측정값 신뢰 불가 / Active AGC corrupts beat timing → all measurements unreliable |
| TR-04 | `MainWindow.cpp` (1,540줄, 61 메서드, 57 멤버변수) God Object + 스레드 보일러플레이트 3중 중복 — 구조 붕괴 위험 / God Object + duplicated thread boilerplate — architecture collapse risk | H | H | 코딩팀 3명 모두 동일 파일 수정; 새 기능 추가마다 변경 범위 폭발 / All 3 coding members edit same file; changes compound on extension |
| TR-05 | `MainWindow.cpp:6~8`에 `Q_OS_MAC` 분기 없음 — macOS 팀원 2명의 오디오 기능 동작 불가 / No `Q_OS_MAC` branch — audio broken for 2 macOS devs | H | M | 볼륨 제어·오디오 설정이 Linux/Windows 분기만 존재 / Volume control and audio settings only exist for Linux/Windows |
| TR-06 | 코딩팀 3명 모두 `MainWindow.cpp` 동시 수정 — merge conflict 빈번 / All 3 coding members edit `MainWindow.cpp` — frequent merge conflicts | H | M | God Object 구조 해소 전까지 병렬 개발 시 충돌 불가피 / Parallel development causes conflicts until God Object is resolved |
| TR-07 | 여러 그래프 탭 동시 활성화 시 Qt 렌더링 FPS 저하 — 11개 탭 + QCustomPlot이 메인 스레드에서 실행 / Qt rendering FPS drops with multiple active graph tabs | M | H | RPi GPU 가속 제한 환경에서 QCustomPlot 11개 동시 업데이트 시 GUI 스레드 블로킹 가능 / QCustomPlot updating 11 graphs may block GUI thread on RPi |
| TR-08 | USB 오디오 버퍼 언더런으로 오디오 블록 누락 및 비트 미감지 / USB audio buffer underrun causes block loss and missed beats | M | H | ALSA 버퍼 설정 미튜닝 시 저지연 요구사항(QAR-02) 달성 불가 / Untuned ALSA buffer prevents meeting low-latency requirement (QAR-02) |
| TR-09 | Rate / Amplitude / Beat Error 계산 오류 — `MainWindow` 안에 묻혀 단위 테스트 불가 / Measurement calculation errors — buried in `MainWindow`, untestable | M | H | Equations_v0 수식 적용 실수를 통합 전에 발견할 방법 없음 / No way to catch Equations_v0 formula mistakes before integration |
| TR-10 | RPi 터치스크린 (DC 12V=2A) 아직 미가동 — 해상도·Qt 렌더링 호환성 미확인 / RPi touchscreen not yet booted — resolution and Qt rendering compatibility unknown | M | H | 실제 화면 크기에 따른 UI 레이아웃 조정 범위 미파악 / Unknown layout adjustment scope for actual screen size |
| TR-11 | 크로스 컴파일 환경 셋업 실패 / PC-RPi 빌드 환경 불일치 / Cross-compilation failure / PC-RPi environment mismatch | M | M | Qt 버전·ALSA 드라이버 차이로 PC 빌드가 RPi에서 실패 가능 / Qt version/ALSA driver differences may cause PC build to fail on RPi |
| TR-12 | FFT 스펙트로그램 실시간 렌더링이 RPi에서 CPU 과부하 — 베이스코드에서 FFTW3 주석 처리 상태 / FFT spectrogram overloads RPi CPU — FFTW3 commented out in base code | M | M | 11개 그래프 중 구현 난이도 최상위; 일정 압박 시 완성 불가 위험 / Highest implementation difficulty among 11 graphs; may not complete under schedule pressure |
| TR-13 | per-beat PCM 링 버퍼 미구현 — Waveform Comparison / Beat-Noise Scope에 멀티 beat PCM 히스토리 버퍼 신규 설계 필요 / Per-beat PCM ring buffer missing — Waveform Comparison and Beat-Noise Scope need new design | L | M | 베이스코드에 beat 슬라이싱 없음; 두 기능이 동일 버퍼 구조를 요구 / Base code has no beat slicing; both features require same buffer structure |
| TR-14 | 장시간 운용 시 Rate/Amplitude/PCM 데이터 누적으로 메모리 증가 — OOM 위험 / Long-term data accumulation risks OOM even on RPi 8GB RAM | M | M | 히스토리 버퍼에 cap 미설정 시 RPi에서도 장시간 운용 불가 / Uncapped history buffer makes long-term operation infeasible |
| TR-15 | Pause → Resume 시 BPH PLL 재동기화까지 측정값 일시적 왜곡 / Pause→Resume data discontinuity — measurements distorted until BPH PLL re-syncs | M | M | 재동기화 대기 구간을 UI에 표시하지 않으면 오염된 측정값이 그래프에 그대로 반영 / Contaminated measurements appear in graph if grace period is not marked |

### 1-2. 비기술 리스크 / Non-Technical Risks

| ID | 리스크 / Risk | Prob | Impact | 근거 / Rationale |
|----|--------------|:----:|:------:|-----------------|
| NR-01 | 코딩팀-아키텍처팀 역할 경계 불명확 — 설계 결정이 구현에 미반영 또는 중복 작업 / Coding/architecture boundary unclear — design decisions not reaching implementation | H | H | 현재 동기화 회의 미고정; 구현 시작 전 아키텍처 리뷰 게이트 없음 / No fixed sync meeting; no architecture review gate before implementation |
| NR-02 | 일부 팀원이 OOD 설계에 익숙하지 않아 설계 의도와 구현 불일치 / C++/Qt 숙련도 차이로 코드 리뷰·통합 단계 병목 / OOD unfamiliarity causes implementation deviation; skill gap creates integration bottleneck | H | H | DSP 파이프라인은 도메인 지식 필요; 숙련도 차이가 통합 단계에서 병목 / DSP pipeline requires domain knowledge; skill gap compounds at integration |
| NR-03 | 소통 채널 분산 (오프라인 + KakaoTalk + Teams) — 설계 결정사항 추적 어렵고 부재 팀원 전달 누락 / Fragmented comms — design decisions hard to track, absent members miss updates | H | M | 현재 설계 결정의 단일 기록 채널 없음 / No single channel for design decision records |
| NR-04 | M1 이전 멘토 미팅 미진행 — 아키텍처 방향 사전 검증 없이 M1 제출 / No mentor meeting before M1 — architecture direction unvalidated | H | H | 근본 결함 발견 시 M2 전 전면 재작업 필요 / Fundamental flaws require full rework before M2 |
| NR-05 | Hung Son Tong에게 설계 의도 전달 누락 위험 / 팀 전반의 영어 커뮤니케이션 부담 / Design intent lost in translation for Hung Son; team-wide English communication burden | M | H | 한국어 위주 오프라인 회의 결정사항이 영어로 전달 안 되면 잘못된 구현 발생 / Korean-only offline decisions cause wrong implementation if not translated |
| NR-06 | 범위 과다 확장 — 11개 그래프 전부 구현 시 핵심 기능 품질 저하 / 5주 일정 내 완성 불가 / Scope overextension — full 11-graph implementation degrades core quality in 5-week schedule | H | M | 7명 × 2h/day = 70h/week 대비 추정 작업량 143h 이상 / Estimated 143h+ workload vs 70h/week capacity |
| NR-07 | Equations_v0 미숙지로 측정 공식 잘못된 구현 / Measurement formula misimplementation — insufficient Equations_v0 familiarity | M | H | Rate/Amplitude/Beat Error 공식 오해 시 QAR-03 달성 불가 / Misunderstood formulas make QAR-03 (Measurement Accuracy) unachievable |
| NR-08 | WeiShi 1000 레퍼런스 디바이스 접근 지연 — 정확도 비교 기준 없이 Correctness 검증 불가 / Delayed WeiShi 1000 access — accuracy validation impossible without reference | M | H | 레퍼런스 없이는 ±5 s/d 오차 기준 충족 여부 확인 불가 / Cannot verify ±5 s/d error margin without reference device |
| NR-09 | Grading Rubric 미수령 — 구현 우선순위 판단 기준 불명확 / Grading rubric not yet distributed — implementation priority ambiguous | M | M | Week 2~3 배포 예정; 그 전까지 잘못된 우선순위로 개발 가능 / Distributed Week 2~3; may develop wrong priorities until then |
| NR-10 | 하드웨어 장애 (RPi, 센서 스탠드, 시계) — 대체 장비 없어 실물 검증 전면 중단 / Hardware failure — no spare equipment, physical validation fully blocked | L | H | RPi 고장 시 Sim 모드로 일부 개발 가능하나 실물 검증 불가 / RPi failure allows Sim-mode dev only; no physical validation |
| NR-11 | 시계 공학 도메인 지식 부족 — Escapement 원리 미이해 시 측정값 해석 오류 가능 / Insufficient watch engineering knowledge — risk of misinterpreting measurement values | M | L | 문서(Witschi Training Course)로 보완 가능한 수준 / Recoverable via documentation (Witschi Training Course) |

---

## 2. 미해결 이슈 및 프로젝트 결과 연관성 / Open Issues and Their Impact on Project Outcome

**한국어**  
아래 이슈들은 모두 최종 데모 품질(QA 달성 여부)에 직접 영향을 미치는 사항이다. 각 이슈가 해결되지 않으면 어떤 결과가 발생하는지 명시한다.

**English**  
All issues below directly affect final demo quality (QA achievement). The consequence of each unresolved issue is explicitly stated.

| ID | 미해결 이슈 / Open Issue | 연관 리스크 / Risk | 미해결 시 결과 / If Unresolved | 담당자 / Owner | 목표 시점 / Target |
|----|------------------------|:-----------------:|-------------------------------|---------------|-------------------|
| OI-01 | GUI 실행 중 RPi 최대 지속 샘플레이트 확인 / Max sustained sps on RPi with GUI running | TR-01 | **QAR-01 실시간 성능 미달** — 데모에서 오디오 처리 불가 / Real-Time Performance QA fails — audio processing not viable at demo | 미정 TBD | EX-01 → 2026-06-11 |
| OI-02 | T1 감지 기준점 결정 (onset vs peak) / Determine T1 detection reference point | TR-02 | **QAR-03 측정 정확도 미달** — Rate/Amplitude 값 신뢰 불가 / Measurement Accuracy QA fails — Rate/Amplitude values unreliable | 미정 TBD | EX-02 → 2026-06-12 |
| OI-03 | `MainWindow.cpp` God Object 리팩토링 범위 확정 (`MeasurementEngine`, `AudioCapture` 분리) / Confirm refactoring scope | TR-04 | **QAR-04 확장성 미달** — 신규 그래프 추가마다 전체 파일 충돌 / Extensibility QA fails — every new graph triggers whole-file conflicts | 미정 TBD | Week 1 → 2026-06-09 |
| OI-04 | macOS `Q_OS_MAC` 빌드 분기 처리 방안 결정 / Decide macOS branch strategy | TR-05 | macOS 팀원 2명이 로컬에서 오디오 기능 테스트 불가 — 개발 속도 저하 / 2 macOS devs cannot test audio locally — development slowdown | 미정 TBD | Week 1 → 2026-06-09 |
| OI-05 | RPi 터치스크린 부팅 및 Qt 앱 구동 확인 / Boot RPi touchscreen and run Qt app | TR-10 | **데모 환경 미확인** — 실제 화면에서 UI가 깨지거나 렌더링 실패 가능 / Demo environment unverified — UI may break or fail to render on actual screen | 미정 TBD | Week 1 → 2026-06-09 |
| OI-06 | RPi 재부팅 후 AGC 비활성화 설정 지속 여부 확인 / Verify AGC off persists after reboot | TR-03 | **모든 측정값 신뢰 불가** — AGC 활성화 상태면 Beat 타이밍 왜곡 / All measurements unreliable — AGC corrupts beat timing | 미정 TBD | Week 1 → 2026-06-09 |
| OI-07 | 코딩팀-아키텍처팀 협업 프로세스 및 Teams 채널 구조 확정 / Finalize collaboration process and Teams channel structure | NR-01 | 설계 결정이 구현에 반영되지 않아 M2 전 전면 재작업 위험 / Design decisions not reaching implementation — full rework risk before M2 | 미정 TBD | Week 1 → 2026-06-09 |
| OI-08 | 11개 그래프 우선순위 분류 확정 (Core / Required / Stretch) / Finalize graph priority classification | NR-06 | 우선순위 없이 개발 시 핵심 기능 미완성 상태로 데모 진행 위험 / Without priority, core features may be incomplete at demo | 미정 TBD | M1 → 2026-06-09 |
| OI-09 | Grading Rubric 수령 후 구현 우선순위 재검토 / Re-review priorities after rubric arrives | NR-09 | Rubric 기준 미달 항목을 뒤늦게 발견 — 수정 시간 부족 / Late discovery of rubric gaps — insufficient time to fix | 미정 TBD | 수령 즉시 Upon receipt |
| OI-10 | Pause/Resume 시 오디오 캡처 스레드 처리 정책 명확화 / Define audio capture thread policy on Pause/Resume | TR-15 | Resume 후 오염된 측정값이 그래프에 반영 — QAR-03 측정 정확도 저하 / Contaminated post-Resume measurements degrade Measurement Accuracy QA | 미정 TBD | M2 → 2026-06-22 |

---

## 3. 미해결 이슈 대응 액션 / Actions to Address Open Issues

**한국어**  
각 미해결 이슈에 대해 구체적인 완료 기준과 기한이 설정된 대응 액션을 정의한다.

**English**  
Each open issue has a concrete action with a defined completion criterion and deadline.

| 이슈 / Issue | 대응 액션 / Action | 완료 기준 / Done When | 기한 / By |
|-------------|------------------|-----------------------|-----------|
| OI-01 (RPi sps) | **EX-01 수행**: RPi에서 96k / 48k / 192k sps 처리 시간 실측 / Run EX-01: measure processing time at 96k/48k/192k sps on RPi | sps별 처리 시간 수치 확보; 48k sps 폴백 여부 결정 / Processing time measured per sps; 48k sps fallback decision made | 2026-06-11 |
| OI-02 (비트 감지 / Beat detection) | **EX-02 수행**: 동일 시계에서 onset vs peak 방식 비교, WeiShi 1000 기준 오차 확인 / Run EX-02: compare onset vs peak on same watch against WeiShi 1000 | 오차 margin 수치 확보; 감지 방식 결정 / Error margin measured; detection method decided | 2026-06-12 |
| OI-03 (God Object 리팩토링) | 코딩팀이 `MainWindow.cpp:602~704` 중복 구간 다이어그램 작성 → 아키텍처팀과 공동 리뷰 → `AudioCapture` 인터페이스 및 `MeasurementEngine` 분리 범위 확정 / Coding team diagrams duplication → joint review → confirm split scope | 4레이어 모듈 경계 문서화 완료; 새 기능 추가 전 경계 확정 / 4-layer module boundaries documented and locked before new features | 2026-06-09 |
| OI-04 (macOS 빌드) | `MacAudio.cpp/h` 스텁 추가 또는 `Q_OS_MAC` 분기 처리; macOS 팀원은 오디오 비활성화 상태로 DSP·UI 개발 / Add `MacAudio.cpp/h` stub or `Q_OS_MAC` branch; macOS devs develop DSP/UI with audio disabled | macOS에서 빌드 성공 확인 / Build succeeds on macOS | 2026-06-09 |
| OI-05 (RPi 터치스크린) | Week 1 내 RPi + 터치스크린 부팅 및 Qt 앱 구동 테스트; SSH 원격 디버깅 환경 구성 / Boot RPi + touchscreen and run Qt app in Week 1; set up SSH remote debug | Qt 앱이 터치스크린 해상도에서 정상 렌더링 확인 / Qt app renders correctly at touchscreen resolution | 2026-06-09 |
| OI-06 (AGC) | AlsaMixer에서 AGC OFF 설정 후 스크린샷 기록; 부팅 시 자동 적용 스크립트 작성; 앱 시작 시 AGC 활성화 여부 경고 진단 기능 추가 / Set AGC OFF in AlsaMixer and screenshot; write auto-apply boot script; add AGC warning diagnostic at app start | 재부팅 후 AGC 비활성화 상태 유지 확인 / AGC remains off after reboot | 2026-06-09 |
| OI-07 (협업 프로세스) | 주 1회 동기화 회의 일정 고정; Teams 설계 결정 전용 채널 생성; 구현 시작 전 아키텍처 리뷰 게이트 프로세스 문서화 / Fix weekly sync; create Teams design-decision channel; document architecture review gate process | Teams 채널 구조 확정 + 1회 동기화 회의 완료 / Teams channel structure confirmed + first sync meeting held | 2026-06-09 |
| OI-08 (그래프 우선순위) | 11개 그래프를 Core(1~4) / Required(5~8) / Stretch(9~11)로 분류; Stretch 탭은 CMake 피처 플래그로 빌드에서 제외 / Classify 11 graphs as Core/Required/Stretch; exclude Stretch tabs via CMake feature flag | 우선순위 목록 팀 합의 완료 / Priority list agreed by team | 2026-06-09 |
| OI-09 (Grading Rubric) | 수령 전까지 Project Plan p.25~26 QA 정의 기준으로 우선순위 유지; 수령 즉시 1일 내 계획 재검토 / Use Project Plan pp.25-26 QA definitions until rubric arrives; review plan within 1 day of receipt | Rubric 기반 우선순위 재조정 완료 / Priority re-adjusted based on rubric | 수령 즉시 Upon receipt |
| OI-10 (Pause/Resume) | Pause 시 오디오 캡처 스레드 처리 정책 결정(지속 or 중단); Resume 직후 3 beat 구간을 신뢰 불가 상태로 마킹하여 그래프에 반영 / Decide capture thread policy on Pause; mark 3-beat grace period as unreliable after Resume | Pause/Resume 시나리오 테스트 통과 + 불연속 구간 마킹 UI 확인 / Pause/Resume scenario test passes + discontinuity marking visible in UI | 2026-06-22 |

---

## 리스크 히트맵 / Risk Heatmap

```
영향도 / Impact
  H  │ TR-03 TR-07 TR-08 │ TR-01 TR-02 TR-04 │
     │ TR-09 TR-10 NR-07 │ NR-01 NR-02 NR-04 │
     │ NR-08 NR-10       │                   │
  M  │ TR-11 TR-12 TR-13 │ TR-05 TR-06 NR-03 │
     │ TR-14 TR-15 NR-09 │ NR-06             │
  L  │ NR-11             │                   │
     └───────────────────┴───────────────────┘
           M (Medium)          H (High)
                                    ↑ 발생 가능성 / Probability
```

> 🔴 **즉시 조치 / Immediate**: TR-01, TR-02, TR-04, NR-01, NR-04  
> 🟠 **Week 1 내 해결 / Resolve by Week 1**: TR-03, TR-05, TR-10, NR-03, OI-06, OI-07  
> 🟡 **아키텍처 설계 시 반영 / Address in architecture design**: TR-06~09, TR-11~15, NR-02, NR-06
