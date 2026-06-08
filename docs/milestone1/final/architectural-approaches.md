# 아키텍처 어프로치 / Architectural Approaches — TimeGrapher

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **작성일 / Date**: 2026-06-07

---

## 1. 아키텍처 개요 / Architecture Overview

### 1.1 Pipe-and-Filter + Publish-Subscribe View of TimeGrapher

**한국어**

Pipe-and-Filter + Pub-Sub 스타일의 C&C View로 런타임 구조를 표현한다. 스레드 경계·커넥터 종류별로 적용된 아키텍처 어프로치(AP)를 주석으로 표시하며, 입력 소스는 Live(USB Mic) / Playback / Sim 세 가지 모드를 지원한다.

**English**

A C&C View in the Pipe-and-Filter + Publish-Subscribe style, describing the runtime structure. Each thread boundary and connector is annotated with the architectural approach (AP) applied at that point. Three input source modes are supported: Live (USB Mic), Playback, and Sim.

> 소스 파일 / Source file: [`assets/cc-view.puml`](assets/cc-view.puml)

![Pipe-and-Filter + Publish-Subscribe View of TimeGrapher](assets/cc-view.png)

---

### 1.2 Layered View of TimeGrapher

**한국어**

| 계층 / Layer | 책임 / Responsibility | 참조 허용 / May Reference |
|:------------:|----------------------|--------------------------|
| **Acquisition** | USB 오디오 입력 → PCM 샘플 → Ring Buffer 공급 | 없음 (최하위) |
| **Signal Processing** | HPF → Envelope → Detector → T1/T3 타임스탬프 추출 | Acquisition (Ring Buffer만) |
| **Domain** | T1/T3 타임스탬프 → Rate·Amplitude·Beat Error 계산, Measurement 발행 | Signal Processing (T1/T3만) |
| **Presentation** | GUI 렌더링, Observer 구독, 경고 표시 | **Domain Layer만** (MeasurementEngine 인터페이스) |

> **핵심 규칙**: Presentation Layer는 Signal Processing / Acquisition 레이어를 **직접 참조할 수 없다.** 이 규칙을 위반하면 QAS-5 Extensibility 목표(≤ 3파일 변경) 달성 불가.

**English**

| Layer | Responsibility | May Reference |
|:-----:|---------------|:-------------:|
| **Acquisition** | USB audio input → PCM samples → Ring Buffer supply | None (lowest layer) |
| **Signal Processing** | HPF → Envelope → Detector → T1/T3 timestamp extraction | Acquisition (Ring Buffer only) |
| **Domain** | T1/T3 timestamps → Rate·Amplitude·Beat Error computation, Measurement publication | Signal Processing (T1/T3 only) |
| **Presentation** | GUI rendering, Observer subscription, warning display | **Domain Layer only** (MeasurementEngine interface) |

> **Core rule**: Presentation Layer **must not directly reference** Signal Processing / Acquisition layers. Violating this rule makes QAS-5 Extensibility target (≤ 3-file change) unachievable.

**한국어**

아래 Module View(Layered Style)는 위 규칙을 시각화한다. 금지된 의존(❌ Forbidden)이 실선으로 표시되며, 이 의존이 존재할 경우 QAS-5 목표 달성 불가가 구조적으로 증명된다.

**English**

The Module View (Layered Style) below visualises the rule above. Forbidden dependencies (❌) are shown explicitly; their presence structurally prevents the QAS-5 ≤ 3-file target.

> 소스 파일 / Source file: [`assets/module-view.puml`](assets/module-view.puml)

![Layered View of TimeGrapher](assets/module-view.png)

---

## 2. 주요 아키텍처 어프로치 / Main Architectural Approaches

**한국어**

아키텍처 어프로치는 총 8개이며, 각각 하나 이상의 QA 드라이버에 직접 연결된다.

**English**

There are 8 architectural approaches in total; each directly addresses one or more QA drivers.

---

### AP-1: 3-스레드 파이프라인 (Concurrent Pipeline) / 3-Thread Pipeline

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **패턴 / Pattern** | Producer-Consumer Pipeline (Bass13 Performance Tactic #4 — Introduce Concurrency) |
| **구성 / Structure** | Audio Thread (생산자) → Lock-Free Ring Buffer → DSP Thread (소비자) → Qt Signal-Slot → GUI Thread |
| **적용 근거** | RPi 5의 단일 프로세스에서 오디오 캡처·DSP·GUI 렌더링이 동일 스레드에서 실행되면 캡처 콜백이 블로킹되어 Dropped Block 발생. 각 관심사를 독립 스레드로 분리하여 콜백 주기(~20ms) 보호 |
| **연결 드라이버** | QAS-1 (Real-Time Performance), QAS-2 (Low Latency) |

**English**

| Item | Detail |
|------|--------|
| **Pattern** | Producer-Consumer Pipeline (Bass13 Performance Tactic #4 — Introduce Concurrency) |
| **Structure** | Audio Thread (producer) → Lock-Free Ring Buffer → DSP Thread (consumer) → Qt Signal-Slot → GUI Thread |
| **Rationale** | Running audio capture, DSP, and GUI rendering in the same thread on RPi 5 causes callback blocking → Dropped Blocks. Separating each concern into an independent thread protects the callback period (~20 ms) |
| **Linked drivers** | QAS-1 (Real-Time Performance), QAS-2 (Low Latency) |

---

### AP-2: Lock-Free Ring Buffer

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **전술 / Tactic** | Reduce Resource Contention (Bass13 Performance Tactic #4) |
| **설명** | Audio Thread(생산자)와 DSP Thread(소비자) 사이의 뮤텍스를 제거하여 락 경합에 의한 DSP 처리 지연 방지. 원형 버퍼를 atomic 연산으로 구현 |
| **적용 근거** | 뮤텍스 대기가 발생하면 콜백 주기(~20ms)를 초과하여 Ring Buffer 오버플로(Dropped Block) 발생. Lock-Free 구조가 이 경로를 원천 차단 |
| **트레이드오프** | 구현 복잡도 ↑ (정확한 memory ordering 보장 필요). AP-1의 구현 패턴으로 AP-1과 함께 적용 |
| **연결 드라이버** | QAS-1 (Real-Time Performance — Dropped Block 방지), QAS-2 (Low Latency — ① 구간 보호) |

**English**

| Item | Detail |
|------|--------|
| **Tactic** | Reduce Resource Contention (Bass13 Performance Tactic #4) |
| **Description** | Eliminates mutex between Audio Thread (producer) and DSP Thread (consumer) to prevent DSP processing delays from lock contention; implements circular buffer via atomic operations |
| **Rationale** | Mutex waits can cause block period violations (~20 ms), leading to Ring Buffer overflow (Dropped Block). Lock-Free structure eliminates this failure path entirely |
| **Trade-off** | Higher implementation complexity (correct memory ordering required). Applied together with AP-1 as its implementation pattern |
| **Linked drivers** | QAS-1 (Real-Time Performance — prevents Dropped Block), QAS-2 (Low Latency — protects segment ①) |

---

### AP-6: Graceful Degradation (SPS 폴백) / Graceful Degradation

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **전술 / Tactic** | Graceful Degradation (Bass13 Performance Tactic) |
| **설명** | EXP-01 결과 96k sps에서 Dropped Block > 0이 확인되면 48k sps로 자동 전환. 블록 주기를 ~10ms → ~20ms로 확장하여 DSP에 허용되는 처리 시간을 두 배로 늘림 |
| **트레이드오프** | T1 감지 분해능: 96k 시 10.4 µs/sample → 48k 시 20.8 µs/sample. 분해능 저하 대신 Dropped Block = 0 보장 |
| **잠정 상태** | ⚠️ 폴백 기준(96k 달성 가능 여부)은 **EXP-01** 결과로 확정 |
| **연결 드라이버** | QAS-1 (Real-Time Performance — Dropped Block = 0 보장) |

**English**

| Item | Detail |
|------|--------|
| **Tactic** | Graceful Degradation (Bass13 Performance Tactic) |
| **Description** | If EXP-01 confirms Dropped Block > 0 at 96k sps, auto-switch to 48k sps; block period expands from ~10 ms to ~20 ms, doubling the DSP time budget |
| **Trade-off** | T1 detection resolution degrades: 10.4 µs/sample at 96k → 20.8 µs/sample at 48k; resolution sacrificed to guarantee Dropped Block = 0 |
| **Provisional** | ⚠️ Fallback threshold (whether 96k is achievable) confirmed by **EXP-01** |
| **Linked drivers** | QAS-1 (Real-Time Performance — guarantees Dropped Block = 0) |

**한국어**

아래 상태 다이어그램은 폴백 결정 흐름을 보여준다. 96k sps → Dropped Block 감지 → 48k sps 전환은 단방향(비가역)이며, 세션 내 재전환 없이 안정성을 우선한다.

**English**

The state diagram below shows the fallback decision flow. The transition 96k sps → Dropped Block detected → 48k sps is one-way (non-reversible); stability is prioritised over peak resolution within a session.

> 소스 파일 / Source file: [`assets/ap6-state.puml`](assets/ap6-state.puml)

![AP-6: Graceful Degradation — Sample Rate Fallback Decision](assets/ap6-state.png)

---

### AP-8: Increase Resources (애플리케이션 Ring Buffer 크기 증설) / Increase Resources

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **전술 / Tactic** | Increase Resources (Bass13 Performance Tactic) |
| **설명** | 애플리케이션 Ring Buffer(`SECONDS_OF_BUFFER`)를 늘려 포그라운드 처리 지연 허용 시간을 확보. 현재 30초(96k sps 기준 약 11.5 MB). 파라미터 한 줄 변경만으로 크기 조정 가능 |
| **적용 근거** | Ring Buffer가 가득 차면 오래된 미처리 샘플이 덮어쓰여 Dropped Block 발생. 버퍼를 충분히 크게 유지하면 포그라운드가 일시적으로 지연되어도 샘플 손실 없이 따라잡을 수 있는 시간적 여유 제공 |
| **트레이드오프** | 메모리 증가 (RPi 5 8 GB 기준 부담 없음). 단, 버퍼가 과도하게 크면 시스템이 지속적으로 과부하 상태일 때 감지가 늦어지고 AP-6 폴백 결정이 지연됨 |
| **잠정 상태** | ⚠️ 최적 크기는 **EXP-01** 결과에서 실제 처리 지연 패턴을 확인한 후 결정. 현재 30초 기본값 유지 |
| **연결 드라이버** | QAS-1 (Real-Time Performance — 일시적 처리 지연 흡수) |

**English**

| Item | Detail |
|------|--------|
| **Tactic** | Increase Resources (Bass13 Performance Tactic) |
| **Description** | Increases the application Ring Buffer (`SECONDS_OF_BUFFER`) to provide more headroom for transient foreground processing delays. Currently 30 s (~11.5 MB at 96k sps). Size is adjustable by changing a single parameter |
| **Rationale** | When the Ring Buffer fills up, unprocessed samples are overwritten, causing Dropped Blocks. A sufficiently large buffer gives the foreground time to catch up after a temporary stall without sample loss |
| **Trade-off** | Memory increase (negligible on RPi 5 8 GB). However, an excessively large buffer delays detection of sustained overload and postpones the AP-6 fallback decision |
| **Provisional** | ⚠️ Optimal size determined after **EXP-01** reveals actual processing delay patterns. Default of 30 s retained until then |
| **Linked drivers** | QAS-1 (Real-Time Performance — absorbs transient processing delays) |

---

### AP-7a: Lazy Rendering

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **전술 / Tactic** | Manage Work Requests — 렌더링 스로틀링 (Bass13 Performance Tactic #3) |
| **설명** | 11개 탭 중 현재 활성 탭만 `paintEvent()`를 실행. 비활성 탭은 데이터만 업데이트하고 렌더링 보류 |
| **적용 근거** | 11탭 동시 렌더링 시 Qt 메인 스레드 부하로 ② process→display 구간이 30ms를 초과할 가능성 (TR-04). 비활성 탭을 스킵하면 렌더링 부하를 1탭 수준으로 감소 |
| **트레이드오프** | 비활성 탭 전환 시 순간적으로 오래된 값이 보일 수 있음 → EXP-02로 허용 수준 확인 |
| **잠정 상태** | ⚠️ Lazy Rendering 필수 여부는 **EXP-02** OI-L2 결과로 결정 |
| **연결 드라이버** | QAS-2 (Low Latency — ② process→display < 30ms) |

**English**

| Item | Detail |
|------|--------|
| **Tactic** | Manage Work Requests — rendering throttling (Bass13 Performance Tactic #3) |
| **Description** | Of 11 tabs, only the active tab executes `paintEvent()`; inactive tabs update data but defer rendering |
| **Rationale** | Simultaneous rendering of 11 tabs may overload the Qt main thread, pushing segment ② process→display beyond 30 ms (TR-04); skipping inactive tabs reduces rendering load to single-tab level |
| **Trade-off** | Momentarily stale values may appear on tab switch → EXP-02 confirms acceptable level |
| **Provisional** | ⚠️ Whether Lazy Rendering is mandatory decided by **EXP-02** OI-L2 result |
| **Linked drivers** | QAS-2 (Low Latency — process→display < 30 ms) |

---

### AP-4: Observer 패턴 / Qt Signal-Slot (단일 데이터 소스) / Observer Pattern

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **패턴 / Pattern** | Observer (GoF) / Qt Signal-Slot |
| **설명** | MeasurementEngine이 단일 `Measurement` 구조체를 `measurementReady()` 시그널로 발행. 모든 11개 탭이 동일 시그널을 독립적으로 구독 |
| **적용 근거 (Correctness)** | 뷰가 독립적으로 값을 계산하면 계산 경로 차이로 뷰 간 값이 달라질 수 있음. Observer 패턴으로 단일 소스에서 발행된 동일 구조체를 모든 뷰가 구독하므로 일관성이 구조적으로 보장됨 |
| **적용 근거 (Extensibility)** | 새 그래프 추가 시 기존 로직을 수정할 필요 없이 구독만 추가하면 됨 (AP-3과 상보적) |
| **연결 드라이버** | QAS-3 QA-C1 (Correctness — 동일 데이터 소스), QAS-5 (Extensibility — 구독 추가만으로 확장) |

**English**

| Item | Detail |
|------|--------|
| **Pattern** | Observer (GoF) / Qt Signal-Slot |
| **Description** | MeasurementEngine publishes a single `Measurement` struct via `measurementReady()` signal; all 11 tabs independently subscribe to the same signal |
| **Rationale (Correctness)** | If views compute values independently, differing computation paths can cause inter-view divergence. Observer pattern ensures all views receive the same struct from a single source — consistency is structurally guaranteed |
| **Rationale (Extensibility)** | Adding a new graph only requires adding a subscription — no modification of existing logic (complementary to AP-3) |
| **Linked drivers** | QAS-3 QA-C1 (Correctness — same data source), QAS-5 (Extensibility — extend by subscription only) |

---

### AP-5: Adaptive Threshold DSP 파이프라인 / Adaptive Threshold DSP Pipeline

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **패턴 / Pattern** | Pipes and Filters (POSA) + Adaptive Threshold |
| **고정 결정** | DSP 파이프라인: Raw PCM → HPF (DC blocker ≥200 Hz) → Envelope (one-pole LPF) → Detector. Adaptive threshold 전략 채택 (이미 구현됨): `noise_floor` = 최근 256 ms 무음 구간 75th percentile, `reference_peak` = 최근 16 beat peak median |
| **미결 결정** | Detector 파라미터(`onset_fraction`=0.03, `min_peak_fraction`=0.20) 기본값이 소음 3조건에서 최적인지 검증 필요 → **EXP-03** |
| **트레이드오프** | `onset_fraction` 높이면 노이즈 차단↑ but 실제 beat onset missed. 낮추면 민감도↑ but false detection |
| **연결 드라이버** | QAS-3 QA-C2 (Correctness — 소음 환경 beat 감지 품질) |

**English**

| Item | Detail |
|------|--------|
| **Pattern** | Pipes and Filters (POSA) + Adaptive Threshold |
| **Fixed decisions** | DSP pipeline: Raw PCM → HPF (DC blocker ≥200 Hz) → Envelope (one-pole LPF) → Detector. Adaptive threshold strategy adopted (already implemented): `noise_floor` = 75th percentile of last 256 ms silence; `reference_peak` = median of last 16 beat peaks |
| **Open decision** | Whether default Detector parameters (`onset_fraction`=0.03, `min_peak_fraction`=0.20) are optimal under 3 noise conditions — confirmed by **EXP-03** |
| **Trade-off** | Higher `onset_fraction` → better noise rejection but may miss real beat onset. Lower → higher sensitivity but false detections |
| **Linked drivers** | QAS-3 QA-C2 (Correctness — beat detection quality under ambient noise) |

---

### AP-7b: Heartbeat 패턴 / Heartbeat Pattern

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **패턴 / Pattern** | Heartbeat (재사용) |
| **설명** | 기존 A(T1)·C(T3) 이벤트를 heartbeat로 재사용. N초 동안 이벤트 미수신 → `⚠ No signal` 경고. noise/signal 비율 임계값 초과 → `⚠ Noisy signal` 경고. 정상 복귀 시 M초 후 자동 해제 |
| **적용 근거** | 별도 감지 로직을 추가하지 않고 기존 Detector 출력을 재활용 → 구현 비용 최소화 |
| **잠정 상태** | ⚠️ N·M 수치 및 noise/signal 임계값은 **EXP-04** 결과로 확정 |
| **연결 드라이버** | QAS-4 (Usability — 신호 품질 경고) |

**English**

| Item | Detail |
|------|--------|
| **Pattern** | Heartbeat (reuse) |
| **Description** | Reuses existing A(T1)·C(T3) events as heartbeat. No beat event for N seconds → `⚠ No signal`. noise/signal ratio exceeds threshold → `⚠ Noisy signal`. Auto-cleared M seconds after signal recovers |
| **Rationale** | Reuses existing Detector output without additional detection logic → minimal implementation cost |
| **Provisional** | ⚠️ N·M values and noise/signal threshold confirmed by **EXP-04** |
| **Linked drivers** | QAS-4 (Usability — signal quality warning) |

---

### AP-3: Layered Architecture + Restrict Dependencies

**한국어**

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **패턴 / Pattern** | Layered Architecture + Restrict Dependencies (Bass13 Modifiability Tactic) |
| **설명** | 기존 God Object 구조를 4-계층 (Acquisition → Signal Processing → Domain → Presentation)으로 분리. Presentation Layer는 Domain Layer(MeasurementEngine 인터페이스)만 참조 가능 |
| **적용 근거** | God Object 구조에서는 그래프 1개 추가 시 여러 파일 수정 필요 → 병렬 개발 충돌. 레이어 분리 후 새 그래프는 Presentation Layer 내 파일 3개(위젯 신규 생성 + 탭 등록 + 구독 연결)만 변경 |
| **연결 드라이버** | QAS-5 (Extensibility — ≤ 3파일 목표) |

**English**

| Item | Detail |
|------|--------|
| **Pattern** | Layered Architecture + Restrict Dependencies (Bass13 Modifiability Tactic) |
| **Description** | Splits the existing God Object structure into 4 layers (Acquisition → Signal Processing → Domain → Presentation); Presentation Layer may only reference the Domain Layer (MeasurementEngine interface) |
| **Rationale** | In the God Object structure, adding each graph requires modifying multiple files → parallel development conflicts. After layer separation, adding a new graph only touches 3 files in the Presentation Layer (new widget + tab registration + subscription wiring) |
| **Linked drivers** | QAS-5 (Extensibility — ≤ 3-file target) |

---

### 2.9 드라이버 지원 요약 및 평가 / Driver Support Summary & Assessment

**한국어**

8개 어프로치가 5개 QA 드라이버를 어떻게, 얼마나 잘 지원하는지 아래 표와 평가로 정리한다. 각 어프로치의 구조적 결정은 설계 단계에서 완료되었으며, 미결 사항은 실험(EXP-01~04)으로 확정한다.

**English**

The table and assessments below summarise how and how well the 8 approaches support the 5 QA drivers. Structural decisions are complete at design time; open items are resolved by experiments (EXP-01–04).

| QA | 우선순위 / Priority | 지원 어프로치 / Supporting Approaches | 지원 방식 / How | 지원 수준 / Level | 미결 실험 / Open Exp. |
|----|:-------------------:|--------------------------------------|----------------|:-----------------:|:---------------------:|
| **QAS-1** Real-Time Performance | 1 | AP-1, AP-2, AP-6, AP-8 | 스레드 분리 → Lock-Free → Ring Buffer 증설 → 폴백: 3중 방어선으로 Dropped Block = 0 보장 / Thread separation → Lock-Free → Buffer increase → fallback: 3-layer defense guarantees Dropped Block = 0 | 구조적 충분 / Structurally sufficient | EXP-01 |
| **QAS-2** Low Latency | 2 | AP-1, AP-2, AP-7a | 3구간 측정 구조 + ① 구간 하한 보호 + ② 구간 렌더링 부하 감소 / 3-segment measurement + segment ① lower-bound protection + segment ② render-load reduction | 구조적 충분 / Structurally sufficient | EXP-02 |
| **QAS-3** Correctness | 3 | AP-4 (QA-C1), AP-5 (QA-C2) | Observer로 단일 소스 일관성 구조 보장 + Adaptive Threshold로 소음 beat 감지 / Observer structurally guarantees single-source consistency + Adaptive Threshold handles noisy beat detection | QA-C1 구조 완성 / C1 done; QA-C2 실험 후 완성 / C2 post-EXP | EXP-03 |
| **QAS-4** Usability | 4 | AP-7b | Heartbeat 패턴으로 신호 소실·노이즈 즉시 감지 / Heartbeat immediately detects signal loss and noise | 구조 확정, 임계값 미결 / Structure confirmed; thresholds open | EXP-04 |
| **QAS-5** Extensibility | 5 | AP-3, AP-4 | Layered Architecture로 변경 파일 ≤ 3 + Observer로 구독 추가만으로 확장 / Layered Architecture limits changes to ≤ 3 files + Observer extends by subscription only | 구조적 달성 가능, 리팩터링 후 검증 / Achievable; verify post-refactor | — |

---

**QAS-1 / Real-Time Performance**

**한국어**

- AP-1(스레드 분리): 캡처 콜백을 DSP·GUI로부터 격리 → Dropped Block 1차 방어선.
- AP-2(Lock-Free Ring Buffer): 뮤텍스 경합 제거 → DSP 처리 지연 2차 방어선.
- AP-8(Ring Buffer 증설): 포그라운드 일시 지연 시 샘플 손실 없이 따라잡을 여유 제공. 최적 크기는 EXP-01 후 결정.
- AP-6(Graceful Degradation): 96k sps 달성 불가 시 48k sps 폴백으로 Dropped Block = 0 최후 보장.
- **미결**: EXP-01 없이는 96k sps 달성 가능 여부와 Ring Buffer 최적 크기 미확인. Conservative 기본값(48k 폴백, 30초 버퍼)으로 최소 목표 보장 가능.

**English**

- AP-1 (thread separation): isolates capture callback from DSP/GUI — first line of defense against Dropped Blocks.
- AP-2 (Lock-Free Ring Buffer): eliminates mutex contention — second line of defense against DSP delays.
- AP-8 (Ring Buffer increase): provides headroom to catch up after transient foreground stalls without sample loss. Optimal size determined after EXP-01.
- AP-6 (Graceful Degradation): guarantees Dropped Block = 0 via 48k sps fallback if 96k sps is unachievable — last line of defense.
- **Open**: Whether 96k sps is achievable and the optimal Ring Buffer size require EXP-01. Conservative defaults (48k fallback, 30 s buffer) guarantee minimum target.

---

**QAS-2 / Low Latency**

**한국어**

- AP-1(3-스레드 파이프라인): TS1/TS2/TS3 3구간 분리 측정으로 병목 구간 특정 가능.
- AP-2(Lock-Free Ring Buffer): ① 구간(capture→process) 하한을 OS 콜백 주기(~20ms)로 유지.
- AP-7a(Lazy Rendering): ② 구간(process→display) 렌더링 부하를 1탭 수준으로 감소.
- **미결**: ① OI-L1(QAudioSource 실제 콜백 주기), ② OI-L2(11탭 렌더링 시 ② 구간 < 30ms) — 둘 다 EXP-02 해소. 28,800 / 36,000 / 43,200 BPH 시계 모두 보유하므로 Primary 달성 확인 후 Stretch도 동일 세션에서 검증 가능.

**English**

- AP-1 (3-thread pipeline): enables TS1/TS2/TS3 3-segment measurement to identify the bottleneck.
- AP-2 (Lock-Free Ring Buffer): preserves segment ① (capture→process) lower bound at OS callback period (~20 ms).
- AP-7a (Lazy Rendering): reduces segment ② (process→display) rendering load to single-tab equivalent.
- **Open**: ① OI-L1 (actual QAudioSource callback period), ② OI-L2 (segment ② < 30 ms at 11 tabs) — both resolved by EXP-02. All three watches (28,800 / 36,000 / 43,200 BPH) are available; Stretch BPH can be verified in the same EXP-02 session after Primary is confirmed.

---

**QAS-3 / Correctness**

**한국어**

- **QA-C1 (동일 데이터 소스)**: AP-4(Observer/Signal-Slot) 구현 완료 시 모든 뷰가 동일 `Measurement` 구조체를 구독 → 뷰 간 일관성이 구조적으로 보장. 실험 불필요.
- **QA-C2 (소음 beat 감지)**: AP-5(Adaptive Threshold DSP) 파이프라인 구조·알고리즘 확정. `onset_fraction`/`min_peak_fraction` 최적값은 EXP-03 후 확정.

**English**

- **QA-C1 (same data source)**: AP-4 (Observer/Signal-Slot) implementation makes all views subscribe to the same `Measurement` struct → inter-view consistency structurally guaranteed. No experiment needed.
- **QA-C2 (noisy beat detection)**: AP-5 pipeline structure and algorithm confirmed. Optimal `onset_fraction`/`min_peak_fraction` values confirmed by EXP-03.

---

**QAS-4 / Usability**

**한국어**

- AP-7b(Heartbeat): 기존 A/C 이벤트 재활용 → 별도 로직 없이 신호 소실·노이즈 감지, 구현 비용 최소.
- N·M 수치 및 noise/signal 임계값은 EXP-04 후 확정. 임계값 미확정의 오경보·미경보는 사용자 경험에만 영향 (QAS-3과 독립).

**English**

- AP-7b (Heartbeat): reuses existing A/C events for signal loss and noise detection — minimal implementation cost.
- N·M values and noise/signal threshold confirmed by EXP-04. Unconfirmed thresholds affect only user experience (independent of QAS-3).

---

**QAS-5 / Extensibility**

**한국어**

- AP-3(Layered Architecture): God Object를 4-계층으로 분리하면 신규 그래프 추가 시 변경 파일이 Presentation Layer 3개로 제한.
- AP-4(Observer): 구독 추가만으로 기존 로직 수정 없이 확장 가능.
- **리스크 TR-07**: God Object 분리 중 기존 그래프 회귀 위험 → 증분 리팩터링 + 단위 테스트 기준값으로 완화. 리팩터링 완료 후 `git diff --stat`으로 ≤ 3파일 검증.

**English**

- AP-3 (Layered Architecture): decomposing God Object into 4 layers limits new-graph changes to 3 Presentation Layer files.
- AP-4 (Observer): extension by subscription only — no modification of existing logic.
- **Risk TR-07**: regression in existing graphs during God Object decomposition → mitigated by incremental refactoring + unit test baseline. Verified via `git diff --stat` ≤ 3 files after refactoring.

