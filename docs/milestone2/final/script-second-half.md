# Presentation Script — Second Half (2-B · 2-C · 2-D · Section 3)

> Speaker: covers from **2-B. Correctness** through **Section 3 Schedule**
> Estimated time: ~10 min

---

## 2-B. Correctness: Observer Pattern

"In the topics I'm covering today, the second is correctness and the third is extensibility."

"First, correctness. The question is simple: when a measurement is ready, does every tab get it? Every single time?"

"The naive approach is to have `MeasurementEngine` call each tab directly. But then it has to know about all 14 tabs. Add one more tab, and you're modifying the engine. That's tight coupling."

"Instead, we used Qt Signal-Slot as an Observer pattern. The engine emits one signal — `measurementReady`. Every tab subscribes. The engine doesn't know who's listening."

"One more thing — DSP runs on a separate thread, T2, but tabs render on the Qt main thread. Qt's `QueuedConnection` takes care of the cross-thread handoff automatically. No manual locking."

> **📋 Diagram — Graph Tab Observer Module View**
>
> 다이어그램은 세 구역으로 나뉩니다.
>
> - **왼쪽 상단 — Subject**: `MeasurementEngine`. `<<Subject>>` 스테레오타입, `measurementReady(m Measurement)` 시그널 하나만 노출. 탭에 대한 의존성 없음.
> - **왼쪽 중단 — Observer**: `BaseGraphTab`. `<<abstract, Observer>>` 스테레오타입. `onMeasurement()` 순수 가상 함수와 `replotAll()`, `mPaused` 멤버 정의. 모든 탭이 이 계약을 따름.
> - **왼쪽 하단 — Concrete Observers**: `TraceTab`, `VarioTab`, `BeatErrorTab`, `... + 11 more tabs`. 모두 `BaseGraphTab`을 상속.
> - **오른쪽 — Value Object 구성**: `Measurement`가 `WatchMetrics`, `SignalFrame`, `AcousticEvent` 세 VO를 포함(composition). 각 VO는 불변(immutable).
> - **`MainWindow`**: `mAllTabs[*]`로 모든 탭을 보유. `registerTab()`으로 등록.
> - **점선 화살표**: `MeasurementEngine →«notify»→ BaseGraphTab` — 런타임 알림 관계. `SessionController`는 런타임 배선자로 이 다이어그램에 의도적으로 생략.

> **📌 Q&A**
>
> **Q. Why Qt Signal-Slot instead of a custom Observer?**
> Qt Signal-Slot is built into the framework we're already using. It gives us thread-safe delivery via `QueuedConnection` for free. A custom Observer would need manual locking for cross-thread safety — more code, more risk.
>
> **Q. What if a tab is not visible — does it still receive the signal?**
> Yes, the signal is always delivered. But each tab has an `isVisible()` guard inside `onMeasurement()` — if the tab isn't visible, it skips the replot. When it becomes visible again, `showEvent()` triggers a catch-up render. So no data is lost, but rendering is lazy.
>
> **Q. How do you guarantee all 14 tabs are actually subscribed?**
> `MainWindow` calls `registerTab()` for every tab at startup, and `SessionController` wires the signal-slot connection in one block. If a tab isn't registered, it simply won't render — it won't cause a crash or a missed update elsewhere.

---

## Transition

"So that's correctness. Now — how did we keep the system extensible as we added more and more tabs?"

---

## 2-C. Extensibility

"We ended up with 14 tabs. We started with 11, added 2 more, then 1 bonus. And none of these additions broke anything below. Three design decisions made that possible."

### Layer

"First, a 4-layer architecture. Acquisition at the bottom, then Signal Processing, Domain, and Presentation at the top. The rule is simple — dependencies only go downward."

"Adding a new tab means touching only the Presentation layer. Three files or less. We verified this across all three rounds of additions. The Domain layer was never touched."

> **📋 Diagram — 4-Layer Allowed-to-Use Module View**
>
> 다이어그램은 다섯 개의 가로 밴드로 구성됩니다 (위에서 아래로).
>
> - **UI Coordinator** (회색): `MainWindow`, `SessionController`, `DiagnosisDialog`. 레이어 간 배선 담당. 도메인 로직 없음.
> - **Presentation** (파란색): `BaseGraphTab` 인터페이스 + 14개 구체 탭 + `GraphTabManager`.
> - **Domain** (초록색): `MeasurementEngine`, `WatchDiagnostics`, `WatchExplainer`, `Measurement` VO. 순수 도메인 계산.
> - **Signal Processing** (노란색): `DSPWorker` + 외부 라이브러리 `tg_process` (FilterChain, BeatDetector).
> - **Acquisition** (빨간색): `IAudioSource` 인터페이스 + `AudioWorker`, `PlaybackWorker`, `SimWorker`, `AudioRingBuffer`.
>
> 점선 화살표(`«use»`)는 모두 아래 방향. 우측 범례(Legend)에서 화살표 종류와 레이어 색상 코딩 확인 가능.
> 실선 테두리 = 구체 클래스, `«interface»` 태그 = 인터페이스, 점선 테두리 = Value Object 또는 외부 라이브러리.

> **📌 Q&A**
>
> **Q. Why is `MeasurementEngine` in Signal Processing, not Domain?**
> It's owned and driven by `DSPWorker` inside the T2 thread pipeline. It calls `processBlock()` as part of the DSP loop — it's not a stand-alone domain service. Pure domain objects like `WatchMath` and `WatchDiagnostics` have no dependency on the DSP loop, so they stay in Domain.
>
> **Q. What does "3 files or less" actually mean?**
> It means the new tab's `.h` and `.cpp` (counted as 1 pair), plus one change to `MainWindow` to register it. That's the maximum for a standard tab. `RadarChartTab` needed 3 because it reads per-position data from `SequenceTab` rather than through `measurementReady` — that's the one exception.
>
> **Q. What's "UI Coordinator" — is it a 5th layer?**
> It's not a domain layer. `MainWindow` and `SessionController` are wiring code — they set up connections at startup but own no business logic. We called it out separately in the diagram to avoid confusion, but it doesn't violate the 4-layer rule.

### Interface — IAudioSource

"Second, `IAudioSource`. We have three audio sources — microphone, file, simulation. Before this interface, each had its own connection logic scattered around. After, they all go through one interface, one `connect()` block in `SessionController`. One place to change if anything needs to change."

> **📋 Diagram — Audio Source Module Uses View**
>
> 다이어그램은 좌우 두 파트로 구성됩니다.
>
> - **AS-IS (왼쪽)**: `AudioWorker`, `PlaybackWorker`, `SimWorker` 각각이 별도의 `connect()` 블록을 가짐. 소스마다 배선 로직이 분산되어 중복 발생.
> - **TO-BE (오른쪽)**: `SessionController`가 `IAudioSource` 인터페이스 하나만 바라봄. 세 소스 모두 이 인터페이스를 구현(`implements`). `connect()` 블록은 `SessionController` 안에 단 하나.
> - 화살표는 의존성 역전 방향을 표시 — `SessionController`가 구체 구현이 아닌 추상 인터페이스에 의존.

> **📌 Q&A**
>
> **Q. Are there plans to add more audio sources?**
> Not currently. The benefit of `IAudioSource` isn't extensibility for new sources — it's that the existing three sources are unified in one place, making the code easier to read and maintain.
>
> **Q. What does the `connect()` block actually do?**
> It wires the Qt signals from the active audio source to `DSPWorker`. When the session starts, `SessionController` casts the active source to `IAudioSource` and connects its audio data signal to the DSP input — regardless of which concrete source is in use.

### Entity / Value Object

"Third, the `Measurement` struct. It's made of three Value Objects — `WatchMetrics`, `SignalFrame`, and `AcousticEvent` — all immutable once created. Tabs get it as read-only. They physically can't mutate the result. So correctness is enforced by the data structure itself, not by trust."

> **📋 Diagram — Watch Measurement Domain Model**
>
> 다이어그램은 Domain 레이어 색상(초록)으로 통일되어 있습니다.
>
> - **중앙 — `Measurement`** (실선 테두리): DSP 사이클마다 생성되는 스냅샷 struct. `noSignal` 플래그 포함.
> - **오른쪽 — 세 Value Object** (점선 테두리, 불변):
>   - `WatchMetrics`: `rate_spd`, `amplitude_deg`, `beatError_ms`, `bph` — 계산된 시계 수치
>   - `SignalFrame`: `samples: PCMBlock`, `timestamp: uint64` — 원시 오디오 캡처 데이터
>   - `AcousticEvent`: `t1`, `t3: uint64` — 개별 틱 이벤트 타임스탬프
> - **하단 노트**: 색상이 view1(4-Layer 다이어그램)의 Domain 밴드와 동일 → 이 객체들이 Domain 레이어 소속임을 시각적으로 확인 가능.
> - Presentation 또는 Signal Processing을 교체해도 이 구조는 전혀 영향을 받지 않음.

> **📌 Q&A**
>
> **Q. What if a tab needs to store or accumulate measurement data over time?**
> Each tab manages its own local state for that. For example, `SequenceTab` accumulates readings per position in its own member variables. `Measurement` itself is a snapshot per DSP cycle — tabs are responsible for what they do with it after `onMeasurement()` is called.
>
> **Q. Why three separate Value Objects instead of one flat struct?**
> Each VO has a different lifecycle and owner. `AcousticEvent` is produced by beat detection, `SignalFrame` by the audio capture, `WatchMetrics` by the DSP math. Keeping them separate makes the data flow clearer and lets each layer pass only what it needs.

---

## 2-D. AI-Assisted Unit Test

"Now, one risk we had going in — most of us don't have deep watch-measurement domain knowledge. Rate, amplitude, beat error — it's hard to write tests for things you don't fully understand yet."

"So we used AI to generate unit tests. They verify that each tab correctly receives measurements and doesn't mutate them. It's not a replacement for domain knowledge — that takes time to build. But it let us verify structural correctness right now, while we're still learning."

> **📌 Q&A**
>
> **Q. What AI tool did you use, and what exactly does it test?**
> We used Claude to generate the tests. They test that `onMeasurement()` is called with a valid `Measurement`, that the tab doesn't throw, and that the measurement data isn't mutated after the call. It's interface compliance testing, not domain logic testing.
>
> **Q. How many tests were generated?**
> 14 tabs, each with a set of structural tests. The same test template was applied across all tabs — baseline 11 in W2 S1, then the 3 additional tabs in W2 S2 and W3 S1.
>
> **Q. What's the plan for real domain logic tests?**
> That's part of the ongoing work. As the team builds domain knowledge — especially through accuracy validation in W4 S4 — we'll add tests for the actual watch metrics computation in `WatchMath` and `WatchDiagnostics`.

---

## Wrap-up

"Quick summary before we move to the schedule. Latency — solved with thread separation. Correctness — Observer pattern. Extensibility — layers, interface, and immutable value objects. And the domain knowledge gap — covered with AI-generated tests."

---

## 3-A. What We Did in M2

"M2 had a clear arc. We built, we measured, we found a problem, and we responded."

"We started with the 4-layer structure and 11 baseline tabs. Then ran experiments — EXP-01 confirmed our filter approach, EXP-02 confirmed T2 works on RPi. But EXP-02 also revealed something unexpected: FG scheduling latency is still over budget. That became our biggest open risk."

"In parallel, we shipped AI Step 1 — rule-based diagnosis — and AI Step 2 — an LLM explainer running locally on RPi. And completed the full architecture refactor."

---

## 3-B. Remaining Risks

"Going into the final sprint, we have four open risks."

"Two critical. TR-10 — FG scheduling latency, we're applying priority scheduling in W4 Sprint 1. TR-05 — end-to-end accuracy hasn't been validated with a real watch yet, that's W4 Sprint 4."

"Two medium. Filter tuning and rendering performance under 14 tabs — both targeted this week."

"The critical path: fix FG first, then experiments, then accuracy validation, then demo."

> **📌 Q&A**
>
> **Q. What exactly is FG scheduling latency?**
> FG stands for foreground — it's the latency between when the DSP thread finishes processing and when the Qt main thread actually starts rendering. We measured `fg_wait` avg at 60.1 ms, with 84% of frames exceeding the real-time deadline. The T2 offload fixed the DSP side, but the rendering side is still the bottleneck.
>
> **Q. What happens if accuracy validation fails in W4 S4?**
> That would be a critical issue for the demo. The buffer week (6/29–6/30) is there partly for this — if W4 S4 reveals accuracy problems, we'd use the buffer to investigate and decide whether it's a filter tuning issue or a deeper calibration problem.

---

## 3-C. M3 Schedule

"Four sprints left. Sprint 1 — FG fix. Sprint 2 — filter tuning. Sprint 3 — rendering benchmark. Sprint 4 — accuracy validation and full RPi run. Then a buffer week, and demo on July 1st."

---

## Closing

"That's everything for Milestone 2. Thank you."
