# 리스크 평가 / Risk Assessment

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09 | **상태 / Status**: [ ] 초안 Draft / [ ] 최종 Final

> **발생 가능성 / Probability**: H=높음 High / M=보통 Medium / L=낮음 Low  
> **영향도 / Impact**: H=높음 High / M=보통 Medium / L=낮음 Low

---

## 1. 기술 및 비기술 리스크 / Technical and Non-Technical Risks

**한국어**  
각 리스크를 발생 가능성(Probability)과 영향도(Impact) 기준으로 H/M/L 척도로 평가한다.

**English**  
Each risk is assessed on a High/Medium/Low scale for both Probability and Impact.

### 1-1. 기술 리스크 / Technical Risks

| ID | 카테고리 / Category | 리스크 / Risk | Prob | Impact | 근거 / Rationale |
|----|--------------------|--------------|:----:|:------:|-----------------|
| TR-01 | [신호 처리 / Signal Processing] | T1/T3 이벤트 감지 부정확 또는 불안정 — 시계 기종·환경마다 패턴 상이 / Beat event detection inaccurate — pattern varies per watch & environment | H | H | threshold 일반화 어려움; 정확도 검증 전 Rate/Amplitude 오차 margin 미확인 / Hard to generalize threshold; error margin unverified |
| TR-02 | [신호 처리 / Signal Processing] | 시스템 재시작 후 AGC 재활성화로 신호 진폭 왜곡 — 측정값 재현성 손실 / AGC re-enabled after reboot distorts signal amplitude | M | H | AGC 활성 상태면 Beat event 타이밍 변형 → 모든 측정값 신뢰 불가 / Active AGC corrupts beat timing → all measurements unreliable |
| TR-03 | [신호 처리 / Signal Processing] | USB 오디오 버퍼 언더런으로 오디오 블록 누락 및 비트 미감지 / USB audio buffer underrun causes block loss and missed beats | M | H | ALSA 버퍼 설정 미튜닝 시 저지연 요구사항(QAR-02) 달성 불가 / Untuned ALSA buffer prevents meeting low-latency requirement (QAR-02) |
| TR-04 | [성능 / Performance] | 보유 RPi에서 Qt GUI + 오디오 캡처 + DSP 동시 실행 시 96k sps 지속 처리 불가 — 실제 성능 미측정 상태 / RPi in hand has not been benchmarked — 96k sps under Qt GUI + DSP load unverified | H | H | 터치스크린 포함 실 환경에서 실험 전까지 달성 가능한 sps 미확인; 48k sps 폴백 설계 여부도 미결정 / Achievable sps with touchscreen attached unknown before EX-01; fallback design undecided |
| TR-05 | [성능 / Performance] | 여러 그래프 탭 동시 활성화 시 Qt 렌더링 FPS 저하 — 11개 탭 + QCustomPlot이 메인 스레드에서 실행 / Qt rendering FPS drops with multiple active graph tabs | M | H | RPi GPU 가속 제한 환경에서 QCustomPlot 11개 동시 업데이트 시 GUI 스레드 블로킹 가능 / QCustomPlot updating 11 graphs may block GUI thread on RPi |
| TR-06 | [성능 / Performance] | FFT 스펙트로그램 실시간 렌더링이 RPi에서 CPU 과부하 — 베이스코드에서 FFTW3 주석 처리 상태 / FFT spectrogram overloads RPi CPU — FFTW3 commented out in base code | M | M | 11개 그래프 중 구현 난이도 최상위; 일정 압박 시 완성 불가 위험 / Highest implementation difficulty among 11 graphs; may not complete under schedule pressure |
| TR-07 | [코드베이스 / 개발 환경 / Codebase & Dev Environment] | `MainWindow.cpp` (1,540줄, 61 메서드, 57 멤버변수) God Object + 스레드 보일러플레이트 3중 중복 — 코딩팀 3명 동시 수정으로 merge conflict 상시 발생 / God Object + duplicated thread boilerplate — 3 coding members editing same file causes constant merge conflicts | H | H | 새 기능 추가마다 변경 범위 폭발; 병렬 개발 구조 해소 전까지 충돌 불가피 / Changes compound on every extension; conflicts inevitable until God Object is resolved |
| TR-08 | [코드베이스 / 개발 환경 / Codebase & Dev Environment] | `MainWindow.cpp:6~8`에 `Q_OS_MAC` 분기 없음 — macOS 팀원 2명의 오디오 기능 동작 불가 / No `Q_OS_MAC` branch — audio broken for 2 macOS devs | H | M | 볼륨 제어·오디오 설정이 Linux/Windows 분기만 존재 / Volume control and audio settings only exist for Linux/Windows |
| TR-09 | [코드베이스 / 개발 환경 / Codebase & Dev Environment] | Rate / Amplitude / Beat Error 계산 오류 — `MainWindow` 안에 묻혀 단위 테스트 불가 / Measurement calculation errors — buried in `MainWindow`, untestable | M | H | Equations_v0 수식 적용 실수를 통합 전에 발견할 방법 없음 / No way to catch Equations_v0 formula mistakes before integration |
| TR-10 | [코드베이스 / 개발 환경 / Codebase & Dev Environment] | PC-RPi 빌드 환경 불일치 — Qt 버전·ALSA 드라이버 차이로 PC 빌드가 RPi에서 실패 가능 / PC-RPi environment mismatch — Qt version/ALSA driver differences may cause PC build to fail on RPi | M | M | 현재 전략은 SSH → RPi 내부 네이티브 빌드이나 환경 구성 미검증 상태 / Current strategy is SSH native build on RPi but environment unverified |
| TR-11 | [구현 범위 / Implementation Scope] | per-beat PCM 링 버퍼 미구현 — Waveform Comparison / Beat-Noise Scope에 멀티 beat PCM 히스토리 버퍼 신규 설계 필요 / Per-beat PCM ring buffer missing — Waveform Comparison and Beat-Noise Scope need new design | L | M | 베이스코드에 beat 슬라이싱 없음; 두 기능이 동일 버퍼 구조를 요구 / Base code has no beat slicing; both features require same buffer structure |
| TR-12 | [구현 범위 / Implementation Scope] | 장시간 운용 시 Rate/Amplitude/PCM 데이터 누적으로 메모리 증가 — OOM 위험 / Long-term data accumulation risks OOM even on RPi 8GB RAM | M | M | 히스토리 버퍼에 cap 미설정 시 RPi에서도 장시간 운용 불가 / Uncapped history buffer makes long-term operation infeasible |

---

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
| NR-09 | 하드웨어 장애 (RPi, 센서 스탠드, 시계) — 대체 장비 없어 실물 검증 전면 중단 / Hardware failure — no spare equipment, physical validation fully blocked | L | H | RPi 고장 시 Sim 모드로 일부 개발 가능하나 실물 검증 불가 / RPi failure allows Sim-mode dev only; no physical validation |
| NR-10 | 시계 공학 도메인 지식 부족 — Escapement 원리 미이해 시 측정값 해석 오류 가능 / Insufficient watch engineering knowledge — risk of misinterpreting measurement values | M | L | 문서(Witschi Training Course)로 보완 가능한 수준 / Recoverable via documentation (Witschi Training Course) |

---

## 2. 미해결 이슈 및 프로젝트 결과 연관성 / Open Issues and Their Impact on Project Outcome

**한국어**  
아래 이슈들은 최종 데모 품질(QA 달성 여부)에 직접 영향을 미친다. 각 이슈가 해결되지 않으면 어떤 결과가 발생하는지 명시한다.

**English**  
All issues below directly affect final demo quality (QA achievement). The consequence of each unresolved issue is explicitly stated.

| ID | 미해결 이슈 / Open Issue | 연관 리스크 / Risk | 미해결 시 결과 / If Unresolved |
|----|------------------------|:-----------------:|-------------------------------|
| OI-01 | T1 감지 기준점 결정 (onset vs peak) / Determine T1 detection reference point | TR-01 | **QAR-03 측정 정확도 미달** — Rate/Amplitude 값 신뢰 불가 / Measurement Accuracy QA fails — Rate/Amplitude values unreliable |
| OI-02 | RPi 재부팅 후 AGC 비활성화 설정 지속 여부 확인 / Verify AGC off persists after reboot | TR-02 | **모든 측정값 신뢰 불가** — AGC 활성화 상태면 Beat 타이밍 왜곡 / All measurements unreliable — AGC corrupts beat timing |
| OI-03 | GUI 실행 중 RPi 최대 지속 샘플레이트 확인 / Max sustained sps on RPi with GUI running | TR-04 | **QAR-01 실시간 성능 미달** — 데모에서 오디오 처리 불가 / Real-Time Performance QA fails — audio processing not viable at demo |
| OI-04 | `MainWindow.cpp` God Object 리팩토링 범위 확정 (`MeasurementEngine`, `AudioCapture` 분리) / Confirm refactoring scope | TR-07 | **QAR-04 확장성 미달** — 신규 그래프 추가마다 전체 파일 충돌 / Extensibility QA fails — every new graph triggers whole-file conflicts |
| OI-05 | macOS `Q_OS_MAC` 빌드 분기 처리 방안 결정 / Decide macOS branch strategy | TR-08 | macOS 팀원 2명이 로컬에서 오디오 기능 테스트 불가 — 개발 속도 저하 / 2 macOS devs cannot test audio locally — development slowdown |
| OI-06 | 코딩팀-아키텍처팀 협업 프로세스 및 Teams 채널 구조 확정 / Finalize collaboration process and Teams channel structure | NR-01 | 설계 결정이 구현에 반영되지 않아 M2 전 전면 재작업 위험 / Design decisions not reaching implementation — full rework risk before M2 |
| OI-07 | 11개 그래프 우선순위 분류 확정 (Core / Required / Stretch) / Finalize graph priority classification | NR-06 | 우선순위 없이 개발 시 핵심 기능 미완성 상태로 데모 진행 위험 / Without priority, core features may be incomplete at demo |

---

## 3. 미해결 이슈 대응 액션 / Actions to Address Open Issues

**한국어**  
각 미해결 이슈에 대해 구체적인 완료 기준이 설정된 대응 액션을 정의한다.

**English**  
Each open issue has a concrete action with a defined completion criterion.

| 이슈 / Issue | 대응 액션 / Action | 완료 기준 / Done When |
|-------------|------------------|-----------------------|
| OI-01 (비트 감지 / Beat detection) | **EX-02 수행**: 동일 시계에서 onset vs peak 방식 비교, WeiShi 1000 기준 오차 확인 / Run EX-02: compare onset vs peak on same watch against WeiShi 1000 | 오차 margin 수치 확보; 감지 방식 결정 / Error margin measured; detection method decided |
| OI-02 (AGC) | AlsaMixer에서 AGC OFF 설정 후 스크린샷 기록; 부팅 시 자동 적용 스크립트 작성; 앱 시작 시 AGC 활성화 여부 경고 진단 기능 추가 / Set AGC OFF in AlsaMixer and screenshot; write auto-apply boot script; add AGC warning diagnostic at app start | 재부팅 후 AGC 비활성화 상태 유지 확인 / AGC remains off after reboot |
| OI-03 (RPi sps) | **EX-01 수행**: RPi에서 96k / 48k / 192k sps 처리 시간 실측 / Run EX-01: measure processing time at 96k/48k/192k sps on RPi | sps별 처리 시간 수치 확보; 48k sps 폴백 여부 결정 / Processing time measured per sps; 48k sps fallback decision made |
| OI-04 (God Object 리팩토링) | 코딩팀이 `MainWindow.cpp:602~704` 중복 구간 다이어그램 작성 → 아키텍처팀과 공동 리뷰 → `AudioCapture` 인터페이스 및 `MeasurementEngine` 분리 범위 확정 / Coding team diagrams duplication → joint review → confirm split scope | 4레이어 모듈 경계 문서화 완료; 새 기능 추가 전 경계 확정 / 4-layer module boundaries documented and locked before new features |
| OI-05 (macOS 빌드) | `MacAudio.cpp/h` 스텁 추가 또는 `Q_OS_MAC` 분기 처리; macOS 팀원은 오디오 비활성화 상태로 DSP·UI 개발 / Add `MacAudio.cpp/h` stub or `Q_OS_MAC` branch; macOS devs develop DSP/UI with audio disabled | macOS에서 빌드 성공 확인 / Build succeeds on macOS |
| OI-06 (협업 프로세스) | 주 1회 동기화 회의 일정 고정; Teams 설계 결정 전용 채널 생성; 구현 시작 전 아키텍처 리뷰 게이트 프로세스 문서화 / Fix weekly sync; create Teams design-decision channel; document architecture review gate process | Teams 채널 구조 확정 + 1회 동기화 회의 완료 / Teams channel structure confirmed + first sync meeting held |
| OI-07 (그래프 우선순위) | 11개 그래프를 Core(1~4) / Required(5~8) / Stretch(9~11)로 분류; Stretch 탭은 CMake 피처 플래그로 빌드에서 제외 / Classify 11 graphs as Core/Required/Stretch; exclude Stretch tabs via CMake feature flag | 우선순위 목록 팀 합의 완료 / Priority list agreed by team |
