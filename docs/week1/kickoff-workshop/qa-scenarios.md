# QA Scenarios — TimeGrapher

> **Format**: Quality Attribute Scenario (CMU SEI)  
> **출처 / Source**: Time Grapher Project Plan (Draft).pdf — p.25-26  
> **작성일 / Date**: 2026-06-02  
> **업데이트 / Updated**: 2026-06-02 (잠정 수치 확정, 우선순위 결정, BPH 스코프 논의 반영)

---

## QA 우선순위 / QA Priority

**한국어**

| 순위 | QA | 근거 요약 |
|------|-----|---------|
| **1** | Real-Time Performance | 나머지 모든 QA의 선행 조건. 이것이 무너지면 데이터 자체가 없음 |
| **2** | Measurement Accuracy | 프로젝트의 존재 이유. WeiShi 비교가 최종 데모 핵심. 아키텍처 결정에 가장 큰 영향 |
| **3** | Low Latency | Hard threshold — 초과 시 기능 붕괴. 실험으로만 검증 가능 |
| **4** | Correctness | Accuracy + 단일 데이터 소스 구조로 자동 확보. 독립적 아키텍처 목표가 아님 |
| **5** | Extensibility | 개발자 편의성이지만 11개 그래프 일정 리스크를 직접 통제 |

```
Real-Time Performance   → 시스템이 살아있는가?           (존재 조건)
Measurement Accuracy    → 시스템이 올바른가?              (목적 달성)
Low Latency             → 시스템이 쓸 만한가?             (사용자 체감)
Correctness             → 시스템이 내부적으로 일관적인가?  (신뢰성)
Extensibility           → 시스템을 계속 만들 수 있는가?   (개발 지속성)
```

**English**

| Priority | QA | Rationale |
|----------|-----|-----------|
| **1** | Real-Time Performance | Precondition for all other QAs. Without it, no data exists to measure anything else |
| **2** | Measurement Accuracy | The reason the project exists. WeiShi comparison is the core final demo requirement. Greatest influence on architectural decisions |
| **3** | Low Latency | Hard threshold — exceeding it causes functional failure. Requires empirical validation via experiments |
| **4** | Correctness | Automatically secured by Accuracy + single data source structure. Not an independent architectural target |
| **5** | Extensibility | Developer-facing, but directly controls schedule risk across 11 graph implementations |

```
Real-Time Performance   → Is the system alive?               (existence condition)
Measurement Accuracy    → Is the system correct?             (purpose achieved)
Low Latency             → Is the system usable?              (user experience)
Correctness             → Is the system internally coherent? (trustworthiness)
Extensibility           → Can development continue?          (development sustainability)
```

---

## BPH 지원 범위 결정 필요 / BPH Support Scope — Decision Required

**한국어**

기계식 시계는 최대 43,200 BPH까지 존재합니다. BPH에 따라 beat 주기가 달라지며, 이는 Low Latency 목표 수치에 직접 영향을 줍니다.

```
BPH별 beat 주기:
  21,600 BPH → 167ms
  28,800 BPH → 125ms
  36,000 BPH → 100ms
  43,200 BPH →  83ms  ← 가장 엄격한 조건
```

| 옵션 | 지원 범위 | Low Latency end-to-end 목표 |
|------|---------|--------------------------|
| **A** | 28,800 BPH까지 | < 100ms (125ms의 80%) |
| **B** | 43,200 BPH까지 | < 66ms (83ms의 80%) |

> ⚠️ **팀 결정 필요**: 보유 시계의 BPH 및 WeiShi No.1000 측정 범위 확인 후 결정.  
> 현재 잠정 수치는 **옵션 B (43,200 BPH 기준, < 66ms)** 로 설정. 팀 합의 후 확정.

**참고**: BPH가 높을수록 beat 처리가 더 자주 실행됨 (43,200 BPH = 초당 12회 vs 28,800 BPH = 초당 8회). 따라서 43,200 BPH 기준으로 실험하는 것이 worst-case CPU 부하 검증에 해당함.

**English**

Mechanical watches exist up to 43,200 BPH. BPH determines the beat interval, which directly affects the Low Latency target value.

```
Beat interval by BPH:
  21,600 BPH → 167 ms
  28,800 BPH → 125 ms
  36,000 BPH → 100 ms
  43,200 BPH →  83 ms  ← most demanding condition
```

| Option | Supported range | Low Latency end-to-end target |
|--------|----------------|------------------------------|
| **A** | Up to 28,800 BPH | < 100 ms (80% of 125 ms) |
| **B** | Up to 43,200 BPH | < 66 ms (80% of 83 ms) |

> ⚠️ **Team decision required**: Confirm BPH of available watches and WeiShi No.1000 measurement range.  
> Current provisional value set to **Option B (43,200 BPH basis, < 66 ms)**. To be finalized by team consensus.

**Note**: Higher BPH means beat processing runs more frequently (43,200 BPH = 12 times/sec vs 28,800 BPH = 8 times/sec). Testing under 43,200 BPH conditions therefore represents worst-case CPU load validation.

---

## QA 1: Real-Time Performance

> **우선순위 / Priority**: 1st — 존재 조건 / Existence condition

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (음향 신호 발생) |
| **Stimulus** | USB 센서를 통해 연속적인 음향 샘플 스트림 입력 |
| **Artifact** | 신호 수집 → 처리 → 분석 → GUI 표시 파이프라인 |
| **Environment** | Raspberry Pi 5 (8GB RAM), 정상 운영 상태 |
| **Response** | 수집된 샘플을 끊김 없이 실시간으로 처리하고 GUI에 표시 |
| **Response Measure** | - **Objective**: 96,000 sps 처리 유지<br>- **Minimum**: 48,000 sps (이 이하면 실패)<br>- **Stretch**: 192,000 sps<br>- dropped audio block 발생 없음 |

**English**

| Field | Content |
|-------|---------|
| **Source** | Mechanical watch (acoustic signal source) |
| **Stimulus** | Continuous stream of acoustic samples from USB sensor |
| **Artifact** | Signal acquisition → processing → analysis → GUI display pipeline |
| **Environment** | Raspberry Pi 5 (8GB RAM), normal operating conditions |
| **Response** | Captured samples processed in real time without interruption and displayed on GUI |
| **Response Measure** | - **Objective**: sustain 96,000 sps processing<br>- **Minimum**: 48,000 sps (below this = failure)<br>- **Stretch**: 192,000 sps<br>- Zero dropped audio blocks |

### 도메인 배경 / Domain Background

**한국어**

**sps(Samples Per Second)** = 마이크로 들어오는 음향 신호를 1초에 몇 번 디지털로 측정하는가.

```
샘플: 마이크가 찍는 사진 한 장
beat: 시계 탈진기가 한 번 동작하는 사건 (tic 또는 tac)

28,800 BPH 시계 기준:
- 1초에 beat 8개 발생 (28,800 ÷ 3,600)
- 96,000 sps라면 beat 하나 안에 샘플 12,000개

43,200 BPH 시계 기준:
- 1초에 beat 12개 발생 (43,200 ÷ 3,600)
- 96,000 sps라면 beat 하나 안에 샘플 8,000개
```

sps가 높을수록 A·C 이벤트의 타이밍을 더 정밀하게 특정할 수 있습니다.

```
48,000 sps → 1샘플 = 0.021ms 오차 가능
96,000 sps → 1샘플 = 0.010ms 오차 가능
192,000 sps → 1샘플 = 0.005ms 오차 가능
```

**beat 감지 원리**: 샘플 에너지(크기²)가 임계값을 넘는 순간을 beat 발생으로 판단. beat 하나 안에서 첫 번째 큰 피크 = A(T1), 두 번째 피크 = C(T3).

**RPi가 문제인 이유**: 샘플 수집 + 필터링 + A·C 감지 + Rate·Beat Error·Amplitude 계산 + GUI 11개 그래프 렌더링을 동시에 끊김 없이 처리해야 함. dropped audio block 발생 시 A·C 이벤트 누락 → 수치 계산 불가.

**English**

**sps (Samples Per Second)** = how many times per second the acoustic signal from the microphone is digitally measured.

```
Sample: one snapshot taken by the microphone
Beat:   one operation of the watch escapement (tic or tac)

For a 28,800 BPH watch:
- 8 beats per second (28,800 ÷ 3,600)
- At 96,000 sps: 12,000 samples per beat

For a 43,200 BPH watch:
- 12 beats per second (43,200 ÷ 3,600)
- At 96,000 sps: 8,000 samples per beat
```

Higher sps allows more precise localization of A·C event timing.

```
48,000 sps  → 1 sample = up to 0.021 ms error
96,000 sps  → 1 sample = up to 0.010 ms error
192,000 sps → 1 sample = up to 0.005 ms error
```

**Beat detection principle**: the system marks a beat whenever sample energy (amplitude²) exceeds a threshold. Within each beat, the first large peak = A(T1), the second peak = C(T3).

**Why RPi is the challenge**: sample acquisition + filtering + A·C detection + Rate·Beat Error·Amplitude calculation + rendering 11 GUI graphs must all run simultaneously without interruption. Dropped audio blocks → missed A·C events → metrics cannot be computed.

---

## QA 2: Low Latency

> **우선순위 / Priority**: 3rd — 사용자 체감 / User experience

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (beat 이벤트 발생) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처 시작 |
| **Artifact** | 캡처 → beat detection → 계산 → GUI 렌더링 전 구간 |
| **Environment** | Raspberry Pi 5, 실시간 운영 상태 |
| **Response** | 캡처된 샘플에 대한 파형, 마커, 계산값이 GUI에 표시됨 |
| **Response Measure** | - ① capture→process: **< 46ms** (잠정)<br>- ② process→display: **< 20ms** (잠정)<br>- ③ end-to-end: **< 66ms** (잠정, 43,200 BPH 기준 83ms의 80%)<br>- dropped audio block 수: 0<br>- missed beat detection 수: 0<br>*(Experiment 1·2 결과 후 확정. BPH 스코프 팀 합의 필요)* |

**English**

| Field | Content |
|-------|---------|
| **Source** | Mechanical watch (beat event source) |
| **Stimulus** | Acoustic sample block captured from microphone |
| **Artifact** | Entire pipeline: capture → beat detection → calculation → GUI rendering |
| **Environment** | Raspberry Pi 5, real-time operating conditions |
| **Response** | Waveform, markers, and computed values for the captured samples displayed on GUI |
| **Response Measure** | - ① capture→process: **< 46 ms** (provisional)<br>- ② process→display: **< 20 ms** (provisional)<br>- ③ end-to-end: **< 66 ms** (provisional, 80% of 83 ms beat interval at 43,200 BPH)<br>- Dropped audio block count: 0<br>- Missed beat detection count: 0<br>*(To be confirmed after Experiment 1·2. BPH scope requires team consensus)* |

### 수치 도출 근거 / Value Derivation

**한국어**

```
43,200 BPH 기준 beat 주기: 83ms
80% 안전 마진 적용: 83ms × 0.8 = 66ms  → end-to-end 목표

구간 배분:
  ① capture→process: 전체의 ~70% → 46ms
     (A·C 감지 + 계산이 대부분의 처리 시간 차지)
  ② process→display: 전체의 ~30% → 20ms
     (Qt 렌더링은 상대적으로 빠름)

왜 80%인가:
  OS 스케줄링 지연, Qt 렌더링 타이밍 변동,
  worst-case spike를 흡수하기 위한 안전 마진
```

**English**

```
Beat interval at 43,200 BPH: 83 ms
80% safety margin: 83 ms × 0.8 = 66 ms  → end-to-end target

Segment allocation:
  ① capture→process: ~70% of total → 46 ms
     (A·C detection + calculation takes the majority of processing time)
  ② process→display: ~30% of total → 20 ms
     (Qt rendering is relatively fast)

Why 80%:
  Safety margin to absorb OS scheduling jitter,
  Qt rendering timing variance, and worst-case spikes
```

### 도메인 배경 / Domain Background

**한국어**

```
시계가 tic 소리 냄
    ↓
마이크가 샘플 수집 (캡처)
    ↓  ① capture→process
A·C 이벤트 감지 + Rate·Beat Error·Amplitude 계산
    ↓  ② process→display
GUI 화면에 수치·파형 표시
```

```
[마이크 캡처] ──①──▶ [A·C 감지 + 계산] ──②──▶ [GUI 표시]
               ①: capture→process             ②: process→display
[마이크 캡처] ──────────────③ end-to-end ───────────▶ [GUI 표시]
```

end-to-end 지연이 beat 주기를 넘으면 이미 다음 beat가 들어온 상태 → backlog(처리 밀림) 발생 → dropped beat로 이어집니다.

| 구간 | 측정 내용 | 보고 항목 | 잠정 목표 |
|------|---------|---------|---------|
| ① capture→process | 샘플 수집 완료 ~ A·C 감지·계산 완료 | average + worst-case (ms) | < 46ms |
| ② process→display | 계산 완료 ~ GUI 화면 표시 | average + worst-case (ms) | < 20ms |
| ③ end-to-end | ①+② 전체 | average + worst-case (ms) | < 66ms |

**English**

```
Watch produces tic sound
    ↓
Microphone captures samples
    ↓  ① capture→process
A·C event detection + Rate·Beat Error·Amplitude calculation
    ↓  ② process→display
Metrics and waveform displayed on GUI
```

```
[mic capture] ──①──▶ [A·C detection + calc] ──②──▶ [GUI display]
               ①: capture→process              ②: process→display
[mic capture] ─────────────③ end-to-end ─────────────▶ [GUI display]
```

If end-to-end latency exceeds the beat interval, the next beat has already arrived → backlog accumulates → dropped beats.

| Segment | What is measured | Reported metrics | Provisional target |
|---------|-----------------|-----------------|-------------------|
| ① capture→process | End of sample capture to completion of A·C detection and calculation | average + worst-case (ms) | < 46 ms |
| ② process→display | Calculation complete to display on GUI | average + worst-case (ms) | < 20 ms |
| ③ end-to-end | Total of ①+② | average + worst-case (ms) | < 66 ms |

---

## QA 3: Correctness

> **우선순위 / Priority**: 4th — 내부 신뢰성 / Internal trustworthiness

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 사용자 (동일 시계, 동일 조건에서 측정) |
| **Stimulus** | 실시간 음향 신호 수신 중 다수의 GUI 뷰 동시 표시 |
| **Artifact** | Rate, Amplitude, Beat Error 계산 로직 및 GUI 전체 뷰 |
| **Environment** | 정상 운영 상태 / ambient noise가 존재하는 환경 |
| **Response** | 모든 GUI 뷰(Trace, Rate Stability, Beat Error, Sequence 등)에서 동일 데이터 기반으로 일관된 수치 표시 |
| **Response Measure** | - 동일 beat 데이터에서 파생된 모든 뷰의 수치 편차: **0** (허용 없음)<br>- 단일 데이터 소스 구조로 구조적 보장<br>- ambient noise 환경에서도 측정값 안정 유지 |

**English**

| Field | Content |
|-------|---------|
| **Source** | User (measuring with the same watch under the same conditions) |
| **Stimulus** | Multiple GUI views displayed simultaneously while receiving real-time acoustic signal |
| **Artifact** | Rate, Amplitude, Beat Error calculation logic and all GUI views |
| **Environment** | Normal operating conditions / environment with ambient noise |
| **Response** | All GUI views (Trace, Rate Stability, Beat Error, Sequence, etc.) display consistent values derived from the same data |
| **Response Measure** | - Value deviation across all views derived from the same beat data: **0** (none permitted)<br>- Structurally guaranteed by single data source architecture<br>- Measurements remain stable under ambient noise |

### 수치 도출 근거 / Value Derivation

**한국어**

편차 = 0으로 정한 이유: Rate·Beat Error·Amplitude는 모두 동일한 A·C 이벤트 타이밍에서 계산됩니다. 단일 데이터 소스 구조를 쓰면 같은 입력에서 같은 출력이 나오는 것이 수학적으로 보장됩니다. 따라서 허용 편차를 "0 초과"로 설정할 근거가 없습니다.

Correctness는 별도 실험이 아니라 **아키텍처 설계(단일 데이터 소스)로 보장**합니다.

**English**

Reason for deviation = 0: Rate, Beat Error, and Amplitude are all computed from the same A·C event timing. A single data source architecture mathematically guarantees identical output from identical input. There is no justification for permitting any non-zero deviation.

Correctness is guaranteed by **architectural design (single data source)**, not by a separate experiment.

### 도메인 배경 / Domain Background

**한국어**

Rate, Beat Error, Amplitude는 모두 **동일한 A·C 이벤트 타이밍**에서 계산됩니다.

```
같은 A 이벤트 데이터
    ├──▶ Rate 계산 ──▶ Rate 그래프
    ├──▶ Beat Error 계산 ──▶ Beat Error 뷰
    └──▶ (A+C) Amplitude 계산 ──▶ Amplitude 그래프
```

서로 다른 데이터로 계산하면 뷰마다 수치가 달라지는 inconsistency 발생. "단일 데이터 소스" 구조가 핵심 아키텍처 결정입니다.

**Correctness vs. Measurement Accuracy 구분**:
- Correctness: 뷰들이 **서로** 일치하는가 (내부 일관성)
- Measurement Accuracy: 뷰의 값이 **WeiShi**와 일치하는가 (외부 정확성)
- Correctness ✓ + Accuracy ✗ 가능: 모든 뷰가 일관되게 틀린 값을 보여줄 수 있음
- 따라서 Accuracy가 더 높은 우선순위

**English**

Rate, Beat Error, and Amplitude are all computed from the **same A·C event timing**.

```
Same A event data
    ├──▶ Rate calculation    ──▶ Rate graph
    ├──▶ Beat Error calc     ──▶ Beat Error view
    └──▶ (A+C) Amplitude calc ──▶ Amplitude graph
```

Computing from different data sources produces inconsistencies where each view shows a different value. "Single data source" is the key architectural decision.

**Correctness vs. Measurement Accuracy distinction**:
- Correctness: do the views agree **with each other**? (internal consistency)
- Measurement Accuracy: do the values agree **with WeiShi**? (external correctness)
- Correctness ✓ + Accuracy ✗ is possible: all views can consistently show the wrong value
- Therefore Accuracy holds higher priority

---

## QA 4: Measurement Accuracy

> **우선순위 / Priority**: 2nd — 프로젝트의 존재 이유 / The reason the project exists

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (escapement 진동 발생) |
| **Stimulus** | T1 (impulse), T3 (lock+banking) 이벤트 포함 음향 신호 입력 |
| **Artifact** | 신호 처리 파이프라인의 beat event detection 모듈 |
| **Environment** | 정상 운영 상태 / 신호가 약하거나 노이즈가 있는 열화 환경 |
| **Response** | T1·T3 onset/peak를 정확히 감지하여 Rate, Amplitude, Beat Error 계산. 신호 열화 시 불안정한 값 대신 명확한 경고 표시 |
| **Response Measure** | - WeiShi No.1000 대비 Rate 오차: **< 5 s/d** (잠정, Experiment 3 후 확정)<br>- WeiShi No.1000 대비 Beat Error 오차: **< 0.1ms** (잠정, Experiment 3 후 확정)<br>- 신호 열화 시 graceful degradation (불안정 출력 없음) |

**English**

| Field | Content |
|-------|---------|
| **Source** | Mechanical watch (escapement vibration source) |
| **Stimulus** | Acoustic signal containing T1 (impulse) and T3 (lock+banking) events |
| **Artifact** | Beat event detection module in the signal processing pipeline |
| **Environment** | Normal operating conditions / degraded environment with weak signal or noise |
| **Response** | Accurately detect T1·T3 onset/peak and compute Rate, Amplitude, Beat Error. On signal degradation, display a clear warning instead of unstable values. |
| **Response Measure** | - Rate error vs. WeiShi No.1000: **< 5 s/d** (provisional, confirmed after Experiment 3)<br>- Beat Error error vs. WeiShi No.1000: **< 0.1 ms** (provisional, confirmed after Experiment 3)<br>- Graceful degradation on signal degradation (no unstable output) |

### 수치 도출 근거 / Value Derivation

**한국어**

**Rate < 5 s/d**

도메인 기준에서 도출: Witschi 트레이닝 문서 기준 기계식 시계 정상 운영 범위는 ±5 s/d. 이것이 "좋은 시계"와 "조정이 필요한 시계"를 구분하는 경계값입니다. 시스템 오차가 5 s/d이면 정상 시계를 비정상으로 오진단할 수 있으므로, 5 s/d는 허용 상한이 아닌 출발점입니다. Experiment 3 결과에 따라 더 엄격하게 조정 예정(예: < 2 s/d).

**Beat Error < 0.1ms**

도메인 정상 범위(0.6ms 이하)의 약 1/6 수준. 시스템 오차가 측정 대상 값의 허용 범위보다 충분히 작아야 신뢰할 수 있는 진단이 가능합니다.

**English**

**Rate < 5 s/d**

Derived from domain standards: per Witschi training material, the normal operating range for a mechanical watch is ±5 s/d — the boundary between "good" and "needs adjustment." A system error of 5 s/d could misdiagnose a normal watch as defective, making 5 s/d a starting point, not an acceptable ceiling. Expected to tighten after Experiment 3 (e.g., < 2 s/d).

**Beat Error < 0.1 ms**

Approximately 1/6 of the domain normal range (≤ 0.6 ms). System measurement error must be well below the acceptable range of the measured quantity to enable reliable diagnosis.

### 도메인 배경 / Domain Background

**한국어**

A(T1) 이벤트 타이밍 1샘플 오차가 미치는 영향:

```
96,000 sps 기준 1샘플 = 0.010ms

Rate:       tic/tac 주기 계산에 직접 영향 → s/day 오차 발생
Beat Error: t1, t2 계산에 직접 영향 → 정상 범위 0.6ms 대비 민감
Amplitude:  t_AC = C - A 계산에 직접 영향 → 분모라서 오차 증폭
```

특히 **C(T3) 이벤트**는 A보다 작고 불규칙해서 감지가 더 어렵고, Amplitude 계산의 분모(t_AC)에 들어가므로 오차가 증폭됩니다.

**English**

Impact of a 1-sample A(T1) event timing error:

```
At 96,000 sps, 1 sample = 0.010 ms

Rate:       directly affects tic/tac period calculation → s/day error
Beat Error: directly affects t1, t2 calculation → sensitive relative to 0.6 ms threshold
Amplitude:  directly affects t_AC = C − A → error amplified because it is in the denominator
```

**C(T3) events** are smaller and more irregular than A, making them harder to detect; because they appear in the denominator of the Amplitude formula (t_AC), their errors are amplified.

---

## QA 5: Extensibility

> **우선순위 / Priority**: 5th — 개발 지속성 / Development sustainability

**한국어**

| 항목 | 내용 |
|------|------|
| **Source** | 개발자 (새 그래프 또는 분석 기능 추가) |
| **Stimulus** | 기존 코드베이스에 새로운 그래프/필터/디스플레이 모드 추가 요청 |
| **Artifact** | GUI 모듈 구조 전체 (신호 수집 / 처리 / 계산 / 표시 레이어) |
| **Environment** | 개발 환경 (PC 또는 RPi), 개발 기간 내 |
| **Response** | 기존 모듈의 major redesign 없이 새 기능을 독립적으로 추가·테스트 가능 |
| **Response Measure** | - 새 그래프 1개 추가 시 변경되는 파일 수: **≤ 3개** (잠정, 코드 분석 후 팀 합의)<br>- 기존 계산 로직 모듈에 대한 수정 없이 추가 가능한 구조 유지 |

**English**

| Field | Content |
|-------|---------|
| **Source** | Developer (adding new graph or analysis feature) |
| **Stimulus** | Request to add new graph / filter / display mode to the existing codebase |
| **Artifact** | Entire GUI module structure (signal acquisition / processing / calculation / display layers) |
| **Environment** | Development environment (PC or RPi), during the development period |
| **Response** | New features can be added and tested independently without major redesign of existing modules |
| **Response Measure** | - Files changed when adding 1 new graph: **≤ 3** (provisional, to be confirmed after codebase analysis + team consensus)<br>- Structure allows addition without modifying existing calculation logic modules |

### 수치 도출 근거 / Value Derivation

**한국어**

≤ 3개 파일로 정한 근거는 세 가지입니다.

**① 이상적인 플러그인 구조에서 불가피한 최솟값**

```
새 그래프 추가 시 반드시 필요한 파일:
  ① 새 그래프 위젯 파일 (새로 생성)        → 1개
  ② 탭 패널에 새 탭 등록 (기존 파일 수정)   → 1개
  ③ 데이터 구독 연결 (기존 파일 수정)       → 1개
                                            합계: 3개
```

이보다 많으면 구조가 충분히 모듈화되지 않았다는 신호입니다.

**② 11개 그래프 일정 리스크 통제**

파일 수 상한이 없으면 후반 그래프를 추가할수록 기존 코드 충돌 → 디버깅 시간 증가 → 일정 초과. ≤ 3개는 이 리스크를 통제하기 위한 설계 목표입니다.

**③ 최종 데모 설명 가능성**

"새 그래프 추가 시 3개 파일만 변경됩니다"는 데모에서 Extensibility를 구체적으로 증명할 수 있는 수치입니다.

**English**

Three reasons for the ≤ 3 file limit:

**① Unavoidable minimum in an ideal plugin structure**

```
Files necessarily touched when adding a new graph:
  ① New graph widget file (created new)          → 1 file
  ② Tab panel registration (existing modified)   → 1 file
  ③ Data subscription wiring (existing modified) → 1 file
                                            Total: 3 files
```

More than 3 signals insufficient modularity.

**② Schedule risk control across 11 graphs**

Without a file count ceiling, later graphs create increasing code conflicts → debugging time grows → schedule slips. ≤ 3 is a design target to control this risk.

**③ Demonstrability at the final demo**

"Adding a new graph touches only 3 files" is a concrete, verifiable claim that proves Extensibility at the demo.

### 도메인 배경 / Domain Background

**한국어**

구현해야 할 그래프가 11개 + Enhanced 기능입니다. A·C 이벤트 → 계산 → 표시의 파이프라인 구조에서 **표시 레이어만 독립적으로 추가**할 수 있어야 합니다.

```
[A·C 감지] → [Rate·BE·Amp 계산] → [표시 레이어]
                                        ├── Trace Display
                                        ├── Rate Stability
                                        ├── Beat Error View
                                        ├── Scope 1 & 2
                                        └── (새 그래프 추가)  ← 이것만 건드려야 함
```

계산 로직을 건드리지 않고 표시 레이어만 추가할 수 있는 구조 = Extensibility 확보.

**English**

There are 11 graphs to implement plus Enhanced features. Within the A·C event → calculation → display pipeline, **only the display layer should need to be added independently**.

```
[A·C detection] → [Rate·BE·Amp calculation] → [display layer]
                                                   ├── Trace Display
                                                   ├── Rate Stability
                                                   ├── Beat Error View
                                                   ├── Scope 1 & 2
                                                   └── (new graph)  ← only this should be touched
```

A structure where only the display layer can be added without touching calculation logic = Extensibility achieved.

---

## 확정 수치 요약 / Confirmed Values Summary

**한국어**

| QA | 수치 | 상태 | 확정 시점 |
|----|------|------|---------|
| Real-Time Performance | 96k sps / 48k sps min / 0 dropped blocks | ✅ 확정 | — |
| Low Latency ① | capture→process < 46ms | ⚠️ 잠정 | Experiment 1·2 후 |
| Low Latency ② | process→display < 20ms | ⚠️ 잠정 | Experiment 1·2 후 |
| Low Latency ③ | end-to-end < 66ms | ⚠️ 잠정 (BPH 스코프 팀 합의 필요) | Experiment 1·2 + 팀 합의 후 |
| Correctness | 편차 = 0 | ✅ 확정 | 단일 데이터 소스로 구조적 보장 |
| Measurement Accuracy (Rate) | < 5 s/d | ⚠️ 잠정 | Experiment 3 후 |
| Measurement Accuracy (Beat Error) | < 0.1ms | ⚠️ 잠정 | Experiment 3 후 |
| Extensibility | ≤ 3개 파일 | ⚠️ 잠정 | 코드 분석 + 팀 합의 후 |

**English**

| QA | Value | Status | When to confirm |
|----|-------|--------|----------------|
| Real-Time Performance | 96k sps / 48k sps min / 0 dropped blocks | ✅ Confirmed | — |
| Low Latency ① | capture→process < 46 ms | ⚠️ Provisional | After Experiment 1·2 |
| Low Latency ② | process→display < 20 ms | ⚠️ Provisional | After Experiment 1·2 |
| Low Latency ③ | end-to-end < 66 ms | ⚠️ Provisional (BPH scope requires team consensus) | After Experiment 1·2 + team consensus |
| Correctness | deviation = 0 | ✅ Confirmed | Structurally guaranteed by single data source |
| Measurement Accuracy (Rate) | < 5 s/d | ⚠️ Provisional | After Experiment 3 |
| Measurement Accuracy (Beat Error) | < 0.1 ms | ⚠️ Provisional | After Experiment 3 |
| Extensibility | ≤ 3 files | ⚠️ Provisional | After codebase analysis + team consensus |

---

## 멘토 리뷰 체크리스트 / Mentor Review Checklist

**한국어**

| 질문 | 답변 | 근거 |
|------|------|------|
| QA 요건이 actionable한가? | ✅ Yes (잠정 수치 포함) | 모든 QA에 측정 가능한 수치 존재. 잠정 항목은 확정 시점 명시 |
| 드라이버가 프로젝트 목표와 연결되어 있는가? | ✅ Yes | 5개 QA가 "동작/정확성/확장성" 목표를 완전히 분담 |
| 수치가 프로젝트 목표에서 명확히 도출되었는가? | ✅ Yes | 각 QA의 수치 도출 근거 명시 (beat 주기, 도메인 정상 범위, 플러그인 구조 최솟값) |
| 기능 요건이 이해되어 있는가? | ✅ Yes | T1/T3 구조, BPH-sps 관계, 11개 그래프 목록이 시나리오에 반영 |
| 요건에 우선순위가 설정되어 있는가? | ✅ Yes | 1~5순위 + 근거 명시 |

**English**

| Question | Answer | Evidence |
|----------|--------|---------|
| Are QA requirements actionable? | ✅ Yes (with provisional values) | All QAs have measurable values. Provisional items have explicit confirmation timelines |
| Do drivers relate to overall project objectives? | ✅ Yes | 5 QAs fully cover "operation / correctness / extensibility" objectives |
| Are measures clearly derived from project goals? | ✅ Yes | Derivation basis stated for each QA (beat interval, domain normal range, plugin structure minimum) |
| Are functional requirements understood? | ✅ Yes | T1/T3 structure, BPH-sps relationship, 11-graph list all reflected in scenarios |
| Are requirements prioritized? | ✅ Yes | Priority 1–5 with explicit rationale |
