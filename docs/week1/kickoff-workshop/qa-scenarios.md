# QA Scenarios — TimeGrapher

> **Format**: Quality Attribute Scenario (CMU SEI)  
> **Source**: Time Grapher Project Plan (Draft).pdf — p.25-26  
> **작성일**: 2026-06-02

---

## QA 1: Real-Time Performance

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (음향 신호 발생) |
| **Stimulus** | USB 센서를 통해 연속적인 음향 샘플 스트림 입력 |
| **Artifact** | 신호 수집 → 처리 → 분석 → GUI 표시 파이프라인 |
| **Environment** | Raspberry Pi 5 (8GB RAM), 정상 운영 상태 |
| **Response** | 수집된 샘플을 끊김 없이 실시간으로 처리하고 GUI에 표시 |
| **Response Measure** | - Objective: 96,000 sps 처리 유지<br>- Minimum: 48,000 sps (이 이하면 실패)<br>- Stretch: 192,000 sps<br>- dropped audio block 발생 없음 |

### 도메인 배경

**sps(Samples Per Second)** = 마이크로 들어오는 음향 신호를 1초에 몇 번 디지털로 측정하는가.

```
샘플: 마이크가 찍는 사진 한 장
beat: 시계 탈진기가 한 번 동작하는 사건 (tic 또는 tac)

28,800 BPH 시계 기준:
- 1초에 beat 8개 발생 (28,800 ÷ 3,600)
- 96,000 sps라면 beat 하나 안에 샘플 12,000개
```

sps가 높을수록 A·C 이벤트의 타이밍을 더 정밀하게 특정할 수 있습니다.

```
48,000 sps → 1샘플 = 0.021ms 오차 가능
96,000 sps → 1샘플 = 0.010ms 오차 가능
192,000 sps → 1샘플 = 0.005ms 오차 가능
```

**beat 감지 원리**: 샘플 에너지(크기²)가 임계값을 넘는 순간을 beat 발생으로 판단. beat 하나 안에서 첫 번째 큰 피크 = A(T1), 두 번째 피크 = C(T3).

**RPi가 문제인 이유**: 샘플 수집 + 필터링 + A·C 감지 + Rate·Beat Error·Amplitude 계산 + GUI 11개 그래프 렌더링을 동시에 끊김 없이 처리해야 함. dropped audio block 발생 시 A·C 이벤트 누락 → 수치 계산 불가.

---

## QA 2: Low Latency

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (beat 이벤트 발생) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처 시작 |
| **Artifact** | 캡처 → beat detection → 계산 → GUI 렌더링 전 구간 |
| **Environment** | Raspberry Pi 5, 실시간 운영 상태 |
| **Response** | 캡처된 샘플에 대한 파형, 마커, 계산값이 GUI에 표시됨 |
| **Response Measure** | - ① capture→process 지연 (ms) — average + worst-case<br>- ② process→display 지연 (ms) — average + worst-case<br>- ③ end-to-end 지연 (ms) — average + worst-case<br>- dropped audio block 수<br>- missed beat detection 수<br>*(구체적 목표 수치는 Experiment 1·2 결과 후 확정)* |

### 도메인 배경

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

28,800 BPH 시계는 **125ms마다** beat가 발생합니다. end-to-end 지연이 125ms를 넘으면 이미 다음 beat가 들어온 상태 → backlog(처리 밀림) 발생 → dropped beat로 이어집니다.

**Real-Time Performance와의 충돌**:

```
sps 높일수록
    → 샘플 수 증가 → ① 처리 시간 증가 → Low Latency 달성 어려워짐
```

sps를 얼마나 높일지 = latency를 얼마나 허용할지의 트레이드오프. Experiment 1·2 결과 후 목표 수치 확정합니다.

| 구간 | 측정 내용 | 보고 항목 |
|------|---------|---------|
| ① capture→process | 샘플 수집 완료 ~ A·C 감지·계산 완료 | average + worst-case (ms) |
| ② process→display | 계산 완료 ~ GUI 화면 표시 | average + worst-case (ms) |
| ③ end-to-end | ①+② 전체 | average + worst-case (ms) |

---

## QA 3: Correctness

| 항목 | 내용 |
|------|------|
| **Source** | 사용자 (동일 시계, 동일 조건에서 측정) |
| **Stimulus** | 실시간 음향 신호 수신 중 다수의 GUI 뷰 동시 표시 |
| **Artifact** | Rate, Amplitude, Beat Error 계산 로직 및 GUI 전체 뷰 |
| **Environment** | 정상 운영 상태 / ambient noise가 존재하는 환경 |
| **Response** | 모든 GUI 뷰(Trace, Rate Stability, Beat Error, Sequence 등)에서 동일 데이터 기반으로 일관된 수치 표시 |
| **Response Measure** | - 동일 beat 데이터에서 파생된 모든 뷰의 수치 일치<br>- ambient noise 환경에서도 측정값 안정 유지<br>*(허용 편차 범위는 팀 합의 필요)* |

### 도메인 배경

Rate, Beat Error, Amplitude는 모두 **동일한 A·C 이벤트 타이밍**에서 계산됩니다.

```
같은 A 이벤트 데이터
    ├──▶ Rate 계산 ──▶ Rate 그래프
    ├──▶ Beat Error 계산 ──▶ Beat Error 뷰
    └──▶ (A+C) Amplitude 계산 ──▶ Amplitude 그래프
```

서로 다른 데이터로 계산하면 뷰마다 수치가 달라지는 inconsistency 발생. "단일 데이터 소스" 구조가 핵심 아키텍처 결정입니다.

---

## QA 4: Measurement Accuracy

| 항목 | 내용 |
|------|------|
| **Source** | 기계식 시계 (escapement 진동 발생) |
| **Stimulus** | T1 (impulse), T3 (lock+banking) 이벤트 포함 음향 신호 입력 |
| **Artifact** | 신호 처리 파이프라인의 beat event detection 모듈 |
| **Environment** | 정상 운영 상태 / 신호가 약하거나 노이즈가 있는 열화 환경 |
| **Response** | T1·T3 onset/peak를 정확히 감지하여 Rate, Amplitude, Beat Error 계산. 신호 열화 시 불안정한 값 대신 명확한 경고 표시 |
| **Response Measure** | - WeiShi No.1000 대비 Rate 오차: ___ s/d 이하<br>- WeiShi No.1000 대비 Beat Error 오차: ___ ms 이하<br>- 신호 열화 시 graceful degradation (불안정 출력 없음)<br>*(구체적 오차 범위는 Experiment 3 결과 후 확정)* |

### 도메인 배경

A(T1) 이벤트 타이밍 1샘플 오차가 미치는 영향:

```
96,000 sps 기준 1샘플 = 0.010ms

Rate:       tic/tac 주기 계산에 직접 영향 → s/day 오차 발생
Beat Error: t1, t2 계산에 직접 영향 → 정상 범위 0.6ms 대비 민감
Amplitude:  t_AC = C - A 계산에 직접 영향 → 분모라서 오차 증폭
```

특히 **C(T3) 이벤트**는 A보다 작고 불규칙해서 감지가 더 어렵고, Amplitude 계산의 분모(t_AC)에 들어가므로 오차가 증폭됩니다.

---

## QA 5: Extensibility

| 항목 | 내용 |
|------|------|
| **Source** | 개발자 (새 그래프 또는 분석 기능 추가) |
| **Stimulus** | 기존 코드베이스에 새로운 그래프/필터/디스플레이 모드 추가 요청 |
| **Artifact** | GUI 모듈 구조 전체 (신호 수집 / 처리 / 계산 / 표시 레이어) |
| **Environment** | 개발 환경 (PC 또는 RPi), 개발 기간 내 |
| **Response** | 기존 모듈의 major redesign 없이 새 기능을 독립적으로 추가·테스트 가능 |
| **Response Measure** | - 새 그래프 1개 추가 시 변경되는 파일 수: ___ 개 이하<br>- 기존 모듈에 대한 수정 없이 추가 가능한 구조 유지<br>*(상한 파일 수는 팀 합의 필요)* |

### 도메인 배경

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

---

## 팀 합의 필요 항목

| QA | 미확정 수치 | 확정 시점 |
|----|-----------|---------|
| Low Latency | 3구간 각각 목표 ms | Experiment 1·2 결과 후 (Week 2) |
| Correctness | 허용 편차 범위 | 팀 합의 |
| Measurement Accuracy | WeiShi 대비 오차 (s/d, ms) | Experiment 3 결과 후 (Week 2) |
| Extensibility | 변경 파일 수 상한 | 팀 합의 |
