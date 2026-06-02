# 리스크 평가 수정안 / Revised Risk Assessment

> **작성일 / Date**: 2026-06-03  
> **마일스톤 / Milestone**: M1  
> **목적 / Purpose**: 기존 리스크 평가 문서의 중복 항목 통합, 분류 정합성 개선, 완화 액션 구체화

---

## 1. 평가 기준 / Assessment Criteria

**한국어**

본 수정안은 기존 리스크 목록에서 중복되는 항목을 통합하고, 기술 리스크와 비기술 리스크의 경계를 명확히 하며, 각 리스크에 대해 실행 가능한 대응 액션과 완료 기준을 추가하는 데 목적이 있다.

- 확률(Probability)
  - `H`: 현재 근거상 발생 가능성이 높고 아직 검증되지 않음
  - `M`: 발생 가능성은 있으나 일부 우회 또는 완화 가능
  - `L`: 발생 가능성이 낮거나 이미 일부 근거가 확보됨
- 영향도(Impact)
  - `H`: 최종 데모, 핵심 품질 속성, 또는 주요 일정에 직접적인 실패를 유발
  - `M`: 품질 저하 또는 일정 지연을 초래하나 대체 수단이 존재
  - `L`: 국소적인 수정이나 운영적 조치로 대응 가능
- 종합 수준(Level)
  - `H`: M2 이전에 즉시 완화 또는 실험이 필요
  - `M`: 구현 중 지속 모니터링 및 해결 필요
  - `L`: 추적 관찰하되 새로운 증거가 나오면 재평가

**English**

This revision consolidates overlapping items, clarifies the boundary between technical and non-technical risks, and adds concrete mitigation actions and completion criteria for each risk.

- Probability
  - `H`: likely to occur based on current evidence and not yet validated
  - `M`: plausible but partially avoidable or mitigable
  - `L`: unlikely or partially backed by existing evidence
- Impact
  - `H`: directly threatens the final demo, core quality attributes, or major schedule commitments
  - `M`: causes quality degradation or schedule delay but has fallback options
  - `L`: can be handled by local fixes or operational workarounds
- Level
  - `H`: requires immediate mitigation or experiment before M2
  - `M`: must be monitored and resolved during implementation
  - `L`: track and revisit when evidence changes

---

## 2. 통합 리스크 목록 / Consolidated Risk List

**한국어**

기존 문서의 중복 항목을 통합한 결과, 핵심 리스크는 아래와 같다.

### 기술 리스크 / Technical Risks

| ID | 리스크 / Risk | 확률 / Prob | 영향 / Impact | 수준 / Level |
|----|--------------|:-----------:|:-------------:|:------------:|
| TR-01 | Raspberry Pi 5에서 Qt GUI와 함께 `96,000 sps` 실시간 처리가 불가능할 수 있음 | H | H | H |
| TR-02 | `T1/T3` beat event 검출 정확도가 부족하여 Rate, Amplitude, Beat Error 계산 신뢰도가 떨어질 수 있음 | H | H | H |
| TR-03 | `AGC` 및 오디오 입력 설정이 실험 간 일관되게 유지되지 않아 측정 재현성이 저하될 수 있음 | M | H | H |
| TR-04 | `Equations_v0` 해석 오류 또는 구현 실수로 계산식이 잘못 적용될 수 있음 | M | H | H |
| TR-05 | 다중 그래프 또는 고주기 업데이트 시 Qt 렌더링이 병목이 되어 latency와 FPS가 악화될 수 있음 | M | H | M |
| TR-06 | USB 오디오 버퍼 언더런 또는 드롭으로 beat 데이터가 누락될 수 있음 | M | H | M |
| TR-07 | 기존 `TimeGrapher_v10.5` 코드 구조를 잘못 이해해 확장 설계가 잘못될 수 있음 | M | M | M |
| TR-08 | RPi와 `WeiShi 1000` 간 비교 검증 환경이 불완전하여 정확도 검증 신뢰성이 떨어질 수 있음 | M | H | H |
| TR-09 | 기존 코드의 강결합과 추상화 부족으로 새 그래프 추가 시 변경 범위가 과도하게 커질 수 있음 | H | H | H |
| TR-10 | Time-Frequency Spectrogram 등 고비용 연산이 RPi에서 실시간 처리 한계를 초과할 수 있음 | L | M | L |

### 비기술 리스크 / Non-Technical Risks

| ID | 리스크 / Risk | 확률 / Prob | 영향 / Impact | 수준 / Level |
|----|--------------|:-----------:|:-------------:|:------------:|
| NR-01 | 선택 기능 구현이 우선되어 핵심 기능 품질과 일정이 희생될 수 있음 | H | M | M |
| NR-02 | 역할 분담이 불명확하면 작업 중복, 누락, 문서 통합 품질 저하가 발생할 수 있음 | M | M | M |
| NR-03 | C++ 경험이 부족한 팀원이 핵심 코드 수정에 투입될 경우 생산성 저하와 버그 유입이 발생할 수 있음 | H | H | H |
| NR-04 | 시계 도메인 지식 부족으로 측정값과 정상 범위를 잘못 해석할 수 있음 | M | H | M |
| NR-05 | 전문가 응답 지연으로 도메인 불확실성이 구현 단계까지 남을 수 있음 | L | M | L |

**English**

After consolidating overlapping entries from the original document, the key risks are as follows.

### Technical Risks

| ID | Risk | Prob | Impact | Level |
|----|------|:----:|:------:|:-----:|
| TR-01 | Raspberry Pi 5 may fail to sustain real-time processing at `96,000 sps` while running the Qt GUI | H | H | H |
| TR-02 | `T1/T3` beat event detection may be too inaccurate to support reliable Rate, Amplitude, and Beat Error calculation | H | H | H |
| TR-03 | `AGC` and audio input settings may not remain stable across experiments, reducing measurement reproducibility | M | H | H |
| TR-04 | Misinterpretation or incorrect implementation of `Equations_v0` may lead to invalid calculations | M | H | H |
| TR-05 | Qt rendering may become a bottleneck under multiple graphs or high refresh rates, harming latency and FPS | M | H | M |
| TR-06 | USB audio buffer underruns or dropped blocks may cause beat data loss | M | H | M |
| TR-07 | Incorrect understanding of the `TimeGrapher_v10.5` baseline structure may lead to flawed extension design | M | M | M |
| TR-08 | An incomplete comparison setup between RPi and `WeiShi 1000` may weaken confidence in accuracy validation | M | H | H |
| TR-09 | Tight coupling and missing abstraction in the existing code may make new graph additions too expensive | H | H | H |
| TR-10 | High-cost computations such as spectrogram FFT may exceed real-time limits on RPi | L | M | L |

### Non-Technical Risks

| ID | Risk | Prob | Impact | Level |
|----|------|:----:|:------:|:-----:|
| NR-01 | Optional features may consume time and degrade core feature quality or schedule reliability | H | M | M |
| NR-02 | Unclear role ownership may cause duplicated work, missing work, and weak document integration | M | M | M |
| NR-03 | Team members with limited C++ experience may reduce productivity and introduce defects in core code | H | H | H |
| NR-04 | Lack of watch-domain knowledge may lead to incorrect interpretation of measurements and normal ranges | M | H | M |
| NR-05 | Delayed expert responses may leave domain uncertainties unresolved into implementation | L | M | L |

---

## 3. 핵심 오픈 이슈와 프로젝트 영향 / Key Open Issues and Project Impact

**한국어**

아래 오픈 이슈는 프로젝트 결과에 직접적인 영향을 주므로 M1 이후 우선적으로 해소되어야 한다.

| 오픈 이슈 / Open Issue | 연결 리스크 / Linked Risk | 연결 QA / QA | 미해소 시 영향 / Impact if Unresolved |
|------------------------|--------------------------|--------------|--------------------------------------|
| Pi 5에서 `96k sps`를 backlog 없이 유지할 수 있는가? | TR-01 | Real-Time Performance | 실시간 처리 목표를 달성하지 못해 최종 데모 신뢰성이 저하됨 |
| `T1/T3` 검출 정확도가 기준 기기와 비교해 충분한가? | TR-02, TR-04, TR-08 | Correctness / Measurement Accuracy | Rate, Amplitude, Beat Error 값의 신뢰성이 약화됨 |
| `AGC`와 오디오 입력 설정이 재부팅과 실험 반복 후에도 안정적으로 유지되는가? | TR-03 | Consistency | 측정 재현성이 떨어져 실험 결과를 믿기 어려워짐 |
| 기존 코드에 확장용 추상화 계층을 도입할 수 있는가? | TR-07, TR-09 | Extensibility | 새 그래프 추가마다 변경 범위가 커져 구현 일정과 구조 품질이 동시에 악화됨 |
| 팀이 핵심 C++ 모듈 작업을 안정적으로 수행할 수 있는가? | NR-02, NR-03 | Implementation Quality / Schedule | 버그 유입과 일정 지연으로 M2, M3 준비가 흔들릴 수 있음 |

**English**

The following open issues directly affect project outcomes and therefore should be resolved with priority after M1.

| Open Issue | Linked Risk | QA | Impact if Unresolved |
|------------|-------------|----|----------------------|
| Can Pi 5 sustain `96k sps` without backlog? | TR-01 | Real-Time Performance | The team may fail to achieve credible real-time behavior in the final demo |
| Is `T1/T3` detection accurate enough compared with the reference device? | TR-02, TR-04, TR-08 | Correctness / Measurement Accuracy | Confidence in Rate, Amplitude, and Beat Error values will be weakened |
| Do `AGC` and audio input settings remain stable across reboot and repeated experiments? | TR-03 | Consistency | Poor reproducibility will make experiment results difficult to trust |
| Can an extensibility layer be introduced into the existing code? | TR-07, TR-09 | Extensibility | Adding each new graph may expand code changes and hurt both schedule and structure quality |
| Can the team execute critical C++ module work reliably? | NR-02, NR-03 | Implementation Quality / Schedule | Defect inflow and schedule delay may jeopardize M2 and M3 preparation |

---

## 4. 대응 계획 / Mitigation Plan

**한국어**

상위 리스크는 단순한 아이디어 수준이 아니라, 실험 또는 검증 가능한 액션으로 관리해야 한다.

| 리스크 ID / Risk ID | 액션 / Action | 담당 / Owner | 목표일 / Target Date | 완료 기준 / Success Criteria |
|--------------------|--------------|-------------|---------------------|-----------------------------|
| TR-01 | Pi 5에서 `48k / 96k / 192k sps` 처리 성능 실험 수행 | Signal Processing Owner | 2026-06-12 | 각 샘플레이트별 CPU 사용률, 처리시간, backlog 유무 기록 완료 |
| TR-02 | Playback 및 실제 시계 데이터로 `T1/T3` 검출 알고리즘 비교 검증 | Algorithm Owner | 2026-06-12 | threshold/peak 방식 비교 결과와 오류 사례 정리 완료 |
| TR-03 | `AGC` 및 오디오 설정 고정 절차 수립, 재부팅 후 재현성 점검 | Platform Owner | 2026-06-10 | 설정 체크리스트 작성 및 3회 재부팅 후 동일 설정 유지 확인 |
| TR-04 | `Equations_v0` 기반 계산식 검증과 단위 테스트 작성 | Algorithm Owner | 2026-06-13 | 대표 입력 케이스에 대해 계산 결과와 기대값 비교 완료 |
| TR-05 | 다중 그래프 활성화 시 FPS 및 latency 측정 | UI Owner | 2026-06-13 | 그래프 수에 따른 FPS/latency 데이터 확보 및 병목 여부 판단 완료 |
| TR-06 | USB 버퍼 크기와 드롭 발생 여부 측정 | Platform Owner | 2026-06-13 | 버퍼 설정별 dropped block 카운트와 안정 동작 조건 기록 완료 |
| TR-07 | `v10.5` 구조 분석 및 모듈/의존 관계 정리 | Architecture Owner | 2026-06-09 | 확장 대상 모듈, 변경 hotspot, 의존 구조 요약 완료 |
| TR-08 | `WeiShi 1000` 비교 검증 방식 확정 | Validation Owner | 2026-06-10 | 동시 비교 가능 여부 확인 또는 순차 비교 대안 절차 정의 완료 |
| TR-09 | 새 그래프 1개 추가를 가정한 spike로 변경 범위 측정 | Architecture Owner | 2026-06-14 | 수정 파일 수, 직접 의존 위치, 추상화 필요 지점 식별 완료 |
| NR-01 | HIGH 우선순위 기능 중심으로 스코프 제한 | PM / Team Lead | 2026-06-09 | 구현 범위 목록에 must-have / optional 구분 반영 완료 |
| NR-02 | 역할 분담 및 문서 통합 책임자 확정 | PM / Team Lead | 2026-06-05 | 각 M1/M2 산출물 owner와 integrator 지정 완료 |
| NR-03 | 핵심 로직은 C++ 숙련자에게 배정하고 코드 리뷰 규칙 적용 | PM / Tech Lead | 2026-06-05 | 핵심 모듈 담당자 지정 및 리뷰 규칙 합의 완료 |
| NR-04 | Witschi 문서와 전문가 피드백 기반으로 도메인 이해 보강 | Domain Owner | 2026-06-10 | 정상 범위와 해석 기준 정리 문서 작성 완료 |
| NR-05 | 전문가 질의를 조기 발송하고 응답 전 진행 가능한 작업 분리 | Domain Owner | 2026-06-06 | 미해결 질의 목록과 대기 중 대체 작업 목록 정리 완료 |

**English**

Top risks should be managed through verifiable actions rather than vague mitigation ideas.

| Risk ID | Action | Owner | Target Date | Success Criteria |
|---------|--------|-------|-------------|------------------|
| TR-01 | Run Pi 5 performance experiments at `48k / 96k / 192k sps` | Signal Processing Owner | 2026-06-12 | CPU usage, processing time, and backlog status recorded for each sample rate |
| TR-02 | Compare `T1/T3` detection algorithms using playback and real watch data | Algorithm Owner | 2026-06-12 | threshold vs peak comparison results and error cases documented |
| TR-03 | Establish fixed `AGC` and audio setup procedure; verify reproducibility after reboot | Platform Owner | 2026-06-10 | setup checklist written and same settings confirmed after 3 reboot cycles |
| TR-04 | Validate formulas from `Equations_v0` and create unit tests | Algorithm Owner | 2026-06-13 | representative input cases compared with expected calculation results |
| TR-05 | Measure FPS and latency with multiple graphs active | UI Owner | 2026-06-13 | FPS/latency data collected and bottleneck conclusion documented |
| TR-06 | Measure USB buffer settings against dropped-block behavior | Platform Owner | 2026-06-13 | stable operating conditions and dropped block counts recorded |
| TR-07 | Analyze `v10.5` structure and summarize module/dependency relationships | Architecture Owner | 2026-06-09 | extension targets, hotspots, and dependency summary completed |
| TR-08 | Finalize comparison method against `WeiShi 1000` | Validation Owner | 2026-06-10 | simultaneous comparison feasibility confirmed or sequential fallback procedure defined |
| TR-09 | Use a one-graph spike to measure extension change scope | Architecture Owner | 2026-06-14 | touched files, direct dependency points, and abstraction needs identified |
| NR-01 | Limit scope to HIGH-priority features | PM / Team Lead | 2026-06-09 | implementation scope list separates must-have and optional features |
| NR-02 | Finalize role ownership and document integration ownership | PM / Team Lead | 2026-06-05 | owners and integrator assigned for each M1/M2 artifact |
| NR-03 | Assign core logic to C++-proficient members and enforce code review | PM / Tech Lead | 2026-06-05 | critical module ownership and review rules agreed |
| NR-04 | Strengthen domain understanding using Witschi material and expert input | Domain Owner | 2026-06-10 | summary of normal ranges and interpretation criteria completed |
| NR-05 | Send expert questions early and split work that can proceed without answers | Domain Owner | 2026-06-06 | unresolved question list and parallel fallback work list completed |

---

## 5. 우선 관리 대상 / Top Risks to Manage First

**한국어**

우선 관리 대상은 `TR-01`, `TR-02`, `TR-03`, `TR-08`, `TR-09`, `NR-03`이다. 이 리스크들은 각각 실시간 성능, 정확도, 재현성, 검증 신뢰성, 확장성, 구현 실행력에 직접 연결되며, 해소되지 않으면 M2 설계와 M3 데모 준비의 전제가 약해진다.

**English**

The first-priority risks are `TR-01`, `TR-02`, `TR-03`, `TR-08`, `TR-09`, and `NR-03`. These directly affect real-time performance, accuracy, reproducibility, validation credibility, extensibility, and implementation capability. If they remain unresolved, the foundation for M2 design and M3 demo readiness will be weak.

