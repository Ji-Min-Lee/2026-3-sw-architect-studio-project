# TimeGrapher — Milestone 2

**Team**: Blue Sky (Team 3) | **Date**: 2026-06-22

---

## Presentation

| # | Section | Document | 시간 |
|---|---------|----------|------|
| 1 | Wrap-up M1 & Intro | [slide-wrapup-intro.md](slide-wrapup-intro.md) | ~3분 |
| 2 | Architecture Views | [slide-architecture-view.md](slide-architecture-view.md) | ~12분 |
| 3 | Remaining Schedule | [slide-schedule.md](slide-schedule.md) | ~5분 |

---

## Traceability: QA → Risk → Experiment → Architecture Decision

> 발표에서 다루지 않는 내용 포함. 전체 설계 근거를 한눈에 확인.

### QAS-1 — Measurement Accuracy *(Governing Goal)*

| Risk | 내용 | Experiment | 결과 | Architecture Decision |
|------|------|-----------|------|-----------------------|
| [TR-05](references/risks.md) | LP/HP filter 기본값이 edge BPH에서 beat signal 차단 또는 노이즈 통과 | [EXP-03](references/experiments/exp-03-filter-tuning-noise-accuracy.md) | ⏳ 06/25 | — (결과 후 ADR 확정) |
| [NTR-07](references/risks.md) | 팀 내 시계 측정 수식(Rate/Amplitude/Beat Error) 도메인 지식 부족 | — | AI 활용 유닛 테스트로 구조 검증 (119 tests / 5 binaries) | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) Observer — 일관된 Measurement 전달 보장 |

### QAS-2 — Real-Time Performance

| Risk | 내용 | Experiment | 결과 | Architecture Decision |
|------|------|-----------|------|-----------------------|
| [TR-02](references/risks.md) | 단일 스레드에서 cpu2 91% 포화 → RPi 43% deadline miss | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) | wait_ms 420ms → **0.013ms** (×32,000) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread ✅ |
| [TR-03](references/risks.md) | Qt QueuedConnection에 420ms backlog 누적 → frame queue 무한 증가 | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) | Backlog 0% (macOS + RPi E2-5/6) | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 DSP Offload Thread ✅ |
| [TR-04](references/risks.md) | `replot()`이 exec budget의 79% 소비 (16ms / 21ms), 탭 수에 비례 악화 | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) / [EXP-05](references/experiments/exp-05-rendering-realtime-impact.md) | replot/beat 8.22 → **1.20** (↓85%) macOS | [ADR-002](references/adr/ADR-002-r1-lazy-rendering.md) R1 Lazy Rendering ✅ · [ADR-004](references/adr/ADR-004-r2-timer-decoupled-rendering.md) R2 조건부 ⏳ |
| [TR-10](references/risks.md) 🔴 | Qt FG 이벤트 루프가 frameLogged 시그널을 avg 60.1ms 지연 수신 (84% > 21ms deadline) | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) E2-7 | 🔴 미해결 | E2-8: SCHED_RR / QTimer polling — 측정 예정 06/22 |

### QAS-3 — Low Latency

| Risk | 내용 | Experiment | 결과 | Architecture Decision |
|------|------|-----------|------|-----------------------|
| [TR-01](references/risks.md) | RPi 5에서 96kHz 오디오 캡처 중 Qt GUI 동시 실행 시 block drop 발생 가능 | [EXP-01](references/experiments/exp-01-high-res-sampling-beat-error.md) | Dropped=0 at 48k/96k/192k · SCHED_RR 불필요 | [ADR-003](references/adr/ADR-003-sample-rate-selection.md) 96kHz Accepted ✅ |
| TR-02/03 | *(QAS-2와 공유)* 단일 스레드 캡처→처리 지연 | [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) | E2E avg **2.05ms** on RPi | [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md) T2 + AudioRingBuffer ✅ |

### QAS-4 — Extensibility / Modifiability

| Risk | 내용 | Experiment | 결과 | Architecture Decision |
|------|------|-----------|------|-----------------------|
| [TR-06](references/risks.md) | 레이어 리팩토링이 기존 DSP 동작에 regression 유발 | — | 116 unit tests (7 binaries) all passing ✅ | 4-Layer Allowed-to-Use 구조 강제 |
| [TR-07](references/risks.md) | 리팩토링 후에도 cross-layer 결합 잔존 | — | 컴파일러가 upward dependency를 catch ✅ | Allowed-to-use 규칙 + 레이어별 include 제한 |
| [TR-08](references/risks.md) | 새 그래프 탭이 현재 `MeasurementEngine` 출력에 없는 데이터 요구 | — | 11개 탭 데이터 요구사항 검토 — 전부 현행 Domain 출력으로 충족 ✅ | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) BaseGraphTab Observer |
| — | `MainWindow`가 concrete audio worker에 직접 의존 → 소스 추가 시 3+ 파일 수정 | — | `NetworkWorker` 추가 시 ≤ 2 파일로 축소 | [ADR-005](references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md) IAudioSource Dependency Inversion ✅ |

### QAS-5 — Correctness

| Risk | 내용 | Experiment | 결과 | Architecture Decision |
|------|------|-----------|------|-----------------------|
| [NTR-07](references/risks.md) | 도메인 수식 이해 부족으로 탭 구현 정합성 검증 어려움 | — | AI 활용 유닛 테스트 — 수식 이해 없이 구조 검증 가능 | [ADR-006](references/adr/ADR-006-basegraphtab-observer-pattern.md) — 모든 탭이 동일한 `Measurement` 수신 보장 |

---

## Architecture Views

| View | 유형 | Primary QA | 문서 |
|------|------|------------|------|
| C&C: DSP Pipeline Thread Model | C&C / Runtime | QAS-2, QAS-3 | [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) |
| Layered: 4-Layer Allowed-to-Use | Module — Layered | QAS-4 | [view-layered-4layer.md](references/views/view-layered-4layer.md) |
| Decomposition: Graph Tab | Module — Decomposition | QAS-4 | [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md) |
| Module: IAudioSource Dependency Inversion | Module — Decomposition | QAS-4 (Extensibility) | [view-iaudiosource.md](references/views/view-iaudiosource.md) |
| Deployment: Build-Deploy Pipeline | Deployment | Deployability | [view-deployment-build-pipeline.md](references/views/view-deployment-build-pipeline.md) |

---

## Document Structure

```
docs/milestone2/final/
├── README.md                         ← 이 파일 — 전체 트레이서빌리티 맵
├── slide-wrapup-intro.md             ← Section 1: M1 feedback + ARS overview
├── slide-architecture-view.md        ← Section 2: Latency / Correctness / Extensibility / Risk
├── slide-schedule.md                 ← Section 3: sprint 회고 + M2→Final 일정
├── assets/                           ← view 다이어그램 (.drawio + .png)
└── references/
    ├── qa/                           ← QA 시나리오 파일 (qas-1 ~ qas-5)
    ├── risks.md                      ← 전체 리스크 레지스터
    ├── views/                        ← Merson 7-section 템플릿 view 파일
    ├── experiments/                  ← 실험 파일 (EXP-01 ~ EXP-05)
    └── adr/                          ← ADR 파일 (ADR-001 ~ ADR-006)
```
