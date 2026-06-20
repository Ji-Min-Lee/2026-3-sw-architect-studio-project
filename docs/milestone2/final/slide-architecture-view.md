# Section 2 — Architecture Views

← [Wrap-up & Intro](slide-wrapup-intro.md) | [Presentation Index](README.md) | Next: [Schedule →](slide-schedule.md)

> **발표 시간**: ~12분 | 목표: Latency → Correctness → Extensibility 순으로 architecture decision 설명

모든 view는 **Merson 7-section 템플릿** 사용. 각 view는 특정 독자 + 특정 QA를 위해서만 작성.  
→ 상세 view 문서: [references/views/](references/views/)

---

## 2-A. Latency: Thread Separation

> 📢 **PRESENT** (~4분) · 근거: [EXP-02](references/experiments/exp-02-realtime-deadline-compliance.md) · 결정: [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md)

**문제**: 단일 스레드에서 GUI replot이 DSP 처리를 블로킹 → RPi 43% deadline miss

![DSP Pipeline Thread Model](assets/view3-thread-model.png)

**결정**: T1(오디오 캡처) · T2(DSP) · Qt main(렌더링)을 세 스레드로 분리, lock-free ring buffer 연결

| 측정 항목 | Before (단일 스레드) | After (T2 분리) |
|-----------|:-------------------:|:---------------:|
| wait_ms avg | 420 ms | **0.013 ms** (×32,000) |
| Deadline miss | 43% | **0%** (macOS) · RPi: E2-8 예정 |
| Backlog 발생 | 있음 | **없음** |

> 🔴 **미해결**: FG scheduling — fg_wait avg 60.1ms, 84% > deadline → E2-8 (6/22)

→ Full view: [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) · ADR: [ADR-001](references/adr/ADR-001-t2-dsp-offload-thread.md)

---

## 2-B. Correctness: Observer Pattern

> 📢 **PRESENT** (~3분) · 근거: QAS-1 Measurement Accuracy (governing goal)

**문제**: 측정 계산 결과가 모든 탭에 일관성 있게 전달되어야 함. 탭별 직접 호출은 결합도를 높이고 누락 위험이 있음.

**결정**: Qt Signal-Slot을 Observer로 사용 — `MeasurementEngine`이 `Measurement` 구조체를 발행, 모든 탭이 구독

```
MeasurementEngine  ── Qt Signal: measurement(Measurement) ──▶  GraphTabManager
                                                                     ├── TraceTab::updateData(m)
                                                                     ├── VarioTab::updateData(m)
                                                                     └── … (14 tabs, 동일 Measurement 수신)
```

**효과**:
- `MeasurementEngine`은 탭의 존재를 모름 → 측정 로직과 표시 로직 완전 분리
- 새 탭 추가 시 `MeasurementEngine` 코드 무변경 → correctness 유지
- Qt `QueuedConnection` — T2(DSP) → Qt main thread 안전한 cross-thread 전달

→ Full view: [view-cc-dsp-pipeline.md](references/views/view-cc-dsp-pipeline.md) · [view-decomposition-graph-tab.md](references/views/view-decomposition-graph-tab.md)

---

## 2-C. Extensibility: Layer + Interface + Entity/VO

> 📢 **PRESENT** (~4분) · 근거: [QAS-4 Extensibility/Modifiability](references/qa/qas-4-extensibility-modifiability.md)

세 가지 설계 결정이 함께 extensibility를 보장합니다.

### Layer — 4-Layer Allowed-to-Use

![4-Layer Allowed-to-Use View](assets/view1-layered-module.png)

Acquisition → Signal Processing → Domain → Presentation. 의존성은 아래 방향으로만.  
새 탭 추가 = Presentation 레이어 ≤ 3 파일. Domain 이하 zero 변경. ✅ 14개 탭 구현 완료.

→ Full view: [view-layered-4layer.md](references/views/view-layered-4layer.md)

### Interface — IAudioSource Dependency Inversion

![IAudioSource Dependency Inversion](assets/view5-iaudiosource.png)

| | AS-IS | TO-BE |
|---|-------|-------|
| 확장 포인트 | `MainWindow` + `AudioManager` + `DSPWorker` 수정 | `IAudioSource` 구현만 |
| `connect()` blocks | 3 (concrete worker별) | 1 (인터페이스 경유) |
| `NetworkWorker` 추가 시 | 3+ 파일 변경 | **≤ 2 파일** |

→ Full view: [view-iaudiosource.md](references/views/view-iaudiosource.md)

### Entity / Value Object — Domain Layer

`MeasurementEngine`이 발행하는 `Measurement` 구조체는 세 개의 VO로 구성:

| Value Object | 내용 | 불변성 |
|---|---|---|
| `WatchMetrics` | Rate (s/d), Amplitude (°), Beat Error (ms), BPH | 계산 완료 후 불변 |
| `SignalFrame` | PCM 샘플 블록 + 타임스탬프 | 캡처 후 불변 |
| `AcousticEvent` | 개별 비트 이벤트 (T1/T3 타임스탬프) | 탐지 후 불변 |

탭은 `Measurement`를 읽기 전용으로 수신 → 측정 결과를 변조할 수 없음 → correctness 보장.  
VO가 Domain 계층에 머무르기 때문에 Presentation 교체/추가 시 도메인 로직 무영향.

---

## 2-D. Risk: AI-Assisted Unit Test

> 📢 **PRESENT** (~1분) · 근거: [TR-NTR-01](references/risks.md) — 도메인 지식 부족 리스크

**리스크**: 팀원 대부분이 시계 측정 도메인(Rate, Amplitude, Beat Error) 전문 지식이 없음 → 탭 구현의 정합성 검증 어려움

**대응**: AI를 활용한 유닛 테스트 자동 생성 — 도메인 지식 없이도 `BaseGraphTab::updateData()` 동작 검증 가능

- 구조적 정합성(인터페이스 준수, 레이어 경계) 검증 → macOS에서 즉시 실행 가능
- 도메인 전문가 없이 11개 탭 W2 Sprint 1 내 완료

---

*Reference only — not presented:*

- [Deployment: Build-Deploy Pipeline](references/views/view-deployment-build-pipeline.md) — RPi 배포 워크플로 (Deployability)
- [ADR-002: R1 Lazy Rendering](references/adr/ADR-002-r1-lazy-rendering.md) — isVisible() guard, replot/beat 8.22 → 1.20 (↓85%)
- [Decomposition: Graph Tab](references/views/view-decomposition-graph-tab.md) — ≤3 파일 확장 레시피 상세
