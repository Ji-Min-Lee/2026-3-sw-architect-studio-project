# 품질 속성 시나리오 — 낮은 지연 / Quality Attribute Scenario — Low Latency

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09

---

## L-3: 낮은 지연 / Low Latency

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **Source** | 기계식 시계 (beat 이벤트 발생원) / Mechanical watch (beat event source) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처가 시작됨 / Audio sample block capture begins from microphone |
| **Artifact** | TimeGrapher 애플리케이션 전체 — 오디오 캡처 → DSP 파이프라인 → Qt GUI 렌더링 / Full TimeGrapher application — audio capture → DSP pipeline → Qt GUI rendering |
| **Environment** | Raspberry Pi 5 (8GB RAM), 정상 운영 상태 — Qt GUI 실행 중, 라이브 오디오 캡처 중 / Normal operation — Qt GUI active, live audio capture running |
| **Response** | 캡처된 샘플 블록에 대응하는 파형, beat 마커, 계산값(Rate·Amplitude·Beat Error)이 GUI에 표시됨 / Waveform, beat markers, and computed values (Rate·Amplitude·Beat Error) for the captured block are displayed in the GUI |
| **Response Measure** | ① capture→process: **< 70 ms** *(잠정 — EX-L1 실측으로 확정)* <br>② process→display: **< 30 ms** *(잠정 — EX-L1 실측으로 확정)* <br>③ end-to-end (①+②): **< 100 ms** *(28,800 BPH 기준, 잠정)* <br>Missed beat: **0** (end-to-end가 beat 주기를 초과하는 순간 즉시 위반) <br>*(Dropped audio block은 QAS-P1 Real-Time Performance 소관)* |

---

## 수치 근거 / Latency Target Derivation

**핵심 제약**: end-to-end 지연이 beat 주기를 초과하면, 이전 beat의 표시가 완료되기 전에 다음 beat 처리가 시작되어 기능이 붕괴됨.

**안전 마진 80% 적용**: OS 스케줄러 지터, Qt 렌더링 변동, RPi 열 스로틀링 흡수 목적.

| BPH | Beat 주기 | 80% → end-to-end 목표 | ① capture→process (70%) | ② process→display (30%) |
|-----|---------|----------------------|------------------------|------------------------|
| 21,600 | 167 ms | 133 ms | 93 ms | 40 ms |
| **28,800** | **125 ms** | **100 ms** | **70 ms** | **30 ms** |
| 36,000 | 100 ms | 80 ms | 56 ms | 24 ms |
| 43,200 | 83 ms | 66 ms | 46 ms | 20 ms |

> 현재 목표는 28,800 BPH 기준 **Option A** (end-to-end < 100 ms). EX-L1 결과 및 실제 보유 시계 BPH 확인 후 상향 검토.
>
> ⚠️ **Team decision required — 보유 시계의 BPH 확인 후 지원 범위 최종 결정 필요**

---

## ③ 구간 분리 근거 / Rationale for Splitting Three Segments

| 구간 | 시간 유형 | 측정 경계 | 병목 원인 | 제어 방법 |
|------|---------|---------|---------|---------|
| ① capture→process | **Wait** (OS 콜백 주기 ~20ms) + **Execute** (DSP 처리) | ALSA 콜백 수신 타임스탬프 → T1/T3 이벤트 타임스탬프 | OS 콜백 주기(~20ms), Ring Buffer 대기, DSP 처리 시간 | 스레드 우선순위, Lock-Free Ring Buffer |
| ② process→display | **Execute** (Qt 렌더링) | T1/T3 이벤트 타임스탬프 → GUI `paintEvent()` 완료 | Qt 렌더링 시간, GUI 업데이트 빈도(FPS), 렌더링 스레드 경합 | Qt 렌더링 최적화, 필요 시 렌더링 스로틀링 |
| ③ end-to-end | Wait + Execute 합산 | ALSA 콜백 수신 → GUI 화면 갱신 완료 | ①+② 합산 | ①, ② 각각 제어 |

> 구간을 분리하는 이유: 병목이 ①에서 발생하는지 ②에서 발생하는지를 구별해야 올바른 전술을 선택할 수 있음. 합산 수치만 보면 원인을 알 수 없음.

---

## OS 콜백 주기와 SPS의 관계 / OS Callback Period vs SPS

capture→process 지연의 **하한은 SPS가 아니라 OS 오디오 콜백 주기**로 결정된다.

| 모드 | 콜백 주기 | 콜백당 샘플 수 (SPS ÷ 50) | CPU 부하 경향 |
|------|---------|--------------------------|------------|
| WAV/Sim | **~20 ms** (코드 확인: `PLAYBACK_SAMPLE_PERIOD_MSEC = 20`) | 48k: ~960 / 96k: ~1,920 / 192k: ~3,840 | SPS↑ → 부하↑ |
| Live | **미확정** (`QAudioSource` 기본값 적용 중, EX-L1 실측 필요) | 동일 공식 적용 예상 | 동일 경향 |

**결론**: SPS를 높여도 capture→process 하한(~20ms)은 변하지 않는다. SPS는 CPU 부하를 통해 **간접적으로** 지연에 영향을 준다 — 높은 SPS에서 DSP가 콜백 주기를 초과하면 Ring Buffer가 쌓여 지연이 누적됨.

---

## Priority

| 항목 / Item | 수준 / Level | 근거 / Rationale |
|-------------|:------------:|-----------------|
| **Importance** | High | end-to-end 지연이 beat 주기를 초과하면 실시간 표시 자체가 불가능해짐. Rate·Amplitude 계산 정확도도 latency에 의존하며, 사용자가 시계 위치를 조정하는 피드백 루프가 지연에 직접 영향받음 |
| **Difficulty** | High | ① RPi 5에서 Qt GUI 렌더링과 DSP가 같은 프로세스 내에서 경쟁하는 구조. ② QAudioSource 라이브 캡처 콜백 주기가 미확인 상태. ③ 11개 그래프 탭 동시 렌더링 시 ② process→display 지연 증가 가능성. 세 요인이 복합적으로 작용하여 달성 난이도가 높음 |

---

## Risk

| ID | 리스크 / Risk | 설명 / Description | 확률 / Prob | 영향 / Impact | 대응 / Action |
|----|--------------|-------------------|:-----------:|:-------------:|--------------|
| LR-01 | QAudioSource 콜백 주기 미확인 | 라이브 캡처에서 기본 버퍼 주기가 ~20ms보다 클 경우 capture→process 하한이 상승하여 QAS-3 목표 위협 | M | H | EX-L1에서 실측하여 확인. 필요 시 `QAudioSource::setBufferSize()` 또는 ALSA 직접 제어로 주기 단축 |
| LR-02 | Qt GUI 렌더링 경합 → process→display 지연 | 11개 그래프 탭 동시 렌더링 시 Qt 메인 스레드가 DSP 결과 수신 전에 렌더링 중 → ② 구간 지연 증가 | M | M | EX-L1에서 탭 수별 FPS 및 ② 구간 측정. 필요 시 lazy rendering (현재 탭만 업데이트) 적용 |
| LR-03 | RPi 열 스로틀링 → 간헐적 지연 급증 | 장시간 실행(10분+) 시 RPi CPU 클럭 다운 → DSP 처리 시간 급증 → end-to-end > 100ms 간헐 발생 | L | H | EX-L1에서 10분 이상 연속 실행하며 최악값(worst-case) 측정. 쿨링 확보 방안 검토 |
| LR-04 | 고BPH 시계 미지원 | 보유 시계가 36,000 BPH 이상일 경우 현재 목표(100ms)로는 beat 주기(83ms) 이하 달성 불가 | L | M | 보유 시계 BPH 확인 후 지원 BPH 범위 결정. 필요 시 Option B (66ms)로 목표 상향 |

---

## 실험 계획 / Experiment Plan

### EX-L1: End-to-End 지연 실측 / End-to-End Latency Measurement

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **목적 / Goal** | 세 구간(capture→process, process→display, end-to-end)의 실제 지연을 SPS별·탭 구성별로 측정하고, QAS-L3 목표(< 100ms) 달성 가능 여부를 확인한다 |
| **방법 / Method** | **타임스탬프 3점 삽입**: TS1 = ALSA 콜백 진입 직후, TS2 = T1/T3 이벤트 계산 완료 직후, TS3 = Qt `paintEvent()` 완료 직후. ① = TS2−TS1, ② = TS3−TS2, ③ = TS3−TS1 로그 기록. SPS 3단계(48k/96k/192k) × 탭 구성 2가지(Rate/Scope 탭만 / 전체 11탭) × 10분 연속 실행 |
| **측정 항목 / Metrics** | SPS \| 탭 구성 \| ① avg·max \| ② avg·max \| ③ avg·max \| QAudioSource 콜백 실제 주기 \| Dropped block 수 |
| **완료 기준 / Done when** | 48k/96k sps 각각에서 ③ max < 100ms (28,800 BPH 기준) 확인. QAudioSource 라이브 콜백 주기 수치 확정. *(Dropped block = 0 달성 여부는 EX-P1의 완료 기준)* |
| **EX-P1(성능)와 관계** | EX-P1이 SPS별 Dropped block을 측정하므로, EX-L1은 EX-P1과 동일 실행 세션에서 타임스탬프를 추가 수집하여 효율화 가능 |

---

## Tactic / Pattern

수업 자료 [Bass13 Performance Tactics, Lecture 08] 기준으로 선택.

| ID | 전술·패턴 / Tactic · Pattern | 출처 / Source | 적용 근거 / Rationale | 트레이드오프 / Trade-off |
|----|---------------------------|:-------------:|----------------------|------------------------|
| **LT-01** | **Introduce Concurrency** — 오디오 스레드·DSP 스레드·GUI 스레드 분리 | Lecture 08, Performance Tactic #4 | 오디오 캡처 콜백이 DSP나 GUI 렌더링에 의해 블로킹되지 않도록 한다. 캡처 스레드는 항상 콜백 주기(~20ms) 이내에 복귀해야 하므로, DSP와 GUI는 별도 스레드에서 실행되어야 ① 구간 하한을 보호할 수 있음 | Design complexity ↑. 스레드 간 데이터 전달을 위한 동기화 메커니즘 필요 |
| **LT-02** | **Lock-Free Ring Buffer** (Producer-Consumer) | Lecture 08, Performance Tactic #4 (Concurrency 구현 패턴) | 오디오 스레드(producer)와 DSP 스레드(consumer) 사이에서 뮤텍스를 제거하여 락 경합에 의한 ① 구간 지연 급증을 방지. 뮤텍스 대기가 발생하면 콜백 주기(~20ms)를 초과할 수 있음 | Lock-free 자료구조의 구현 복잡도 ↑. 정확한 memory ordering 보장 필요 |
| **LT-03** | **Reduce Computational Overhead** — DSP 연산 최적화 | Lecture 08, Performance Tactic #1 | T1/T3 이벤트 감지에 필요하지 않은 연산(예: 불필요한 데이터 복사, 반복 계산)을 제거하여 ① 구간의 DSP 처리 시간을 단축. "필요한 일만 한다" — 실시간 경로에서 불필요한 작업을 배제 | Modifiability ↓ (최적화된 코드는 변경하기 어려워짐). 가독성 ↓ |
| **LT-04** | **Manage Work Requests** — 렌더링 스로틀링 (Lazy Rendering) | Lecture 08, Performance Tactic #3 | 11개 탭 중 현재 사용자가 보고 있는 탭만 업데이트하여 Qt 메인 스레드의 렌더링 부하를 줄이고 ② 구간 지연을 감소시킴. 모든 탭을 매 프레임 업데이트하는 것은 필요 이상의 작업 | 비활성 탭의 데이터가 탭 전환 시 순간적으로 오래된 값을 보여줄 수 있음 (사용자가 인지할 정도인지 EX-L1으로 확인 필요) |
*(LT-05 SPS 폴백은 Dropped block = 0을 지키기 위한 전술이므로 QAS-P1 Real-Time Performance 문서 소관)*

---

## 아키텍처 ↔ Latency 구간 매핑 / Architecture — Latency Segment Mapping

```
[ALSA Driver / QAudioSource]
         │  ← TS1 (콜백 진입)
         ▼
[Audio Thread]  ── Lock-Free Ring Buffer ──▶ [DSP Thread]
    (LT-01, LT-02)                                │  ← TS2 (T1/T3 계산 완료)
    ←── ① capture→process (~20ms 하한) ────────────┘
                                                   │
                                              Qt Signal/Slot
                                                   │
                                                   ▼
                                          [GUI Rendering Thread]
                                             (LT-03, LT-04)
                                                   │  ← TS3 (paintEvent 완료)
                                                   ▼
                                            [화면 / Screen]
    ←──────────── ③ end-to-end (< 100ms) ─────────────────────┘
```

---

## 다른 QA와의 의존 관계 / Dependency on Other QAs

| 관계 | 내용 |
|------|------|
| **QAS-P1(실시간 성능) → L-3** | Dropped block이 발생하면 ① 구간에 Ring Buffer 대기 지연이 누적되어 end-to-end가 beat 주기를 초과할 수 있음. QAS-P1이 선행 조건 |
| **L-3 → QAS-C(정확성)** | end-to-end 지연이 beat 주기를 초과하면 T1/T3 타임스탬프 정보가 실제 발생 시각과 어긋나 Rate·Amplitude 계산 오차가 발생할 수 있음 |
| **LT-04(렌더링 스로틀링) → QAS-U(사용성)** | 비활성 탭 업데이트를 억제하면 탭 전환 시 순간적으로 오래된 데이터가 보일 수 있음. 허용 가능한 수준인지 사용성 관점에서 검토 필요 |

---

## 미결 사항 / Open Issues

| ID | 질문 / Question | 해결 방법 / Resolution |
|----|-----------------|----------------------|
| OI-L1 | QAudioSource 라이브 캡처의 실제 콜백 주기는 얼마인가? | EX-L1 실측 |
| OI-L2 | 11탭 동시 렌더링 시 ② process→display 구간이 30ms 이내인가? | EX-L1 실측 (탭 구성 비교) |
| OI-L3 | 지원할 최대 BPH는 얼마인가? (28,800 vs 36,000 vs 43,200) | 보유 시계 BPH 확인 후 팀 합의 |
| OI-L4 | LT-05 SPS 폴백 발생 시 사용자에게 어떤 방식으로 알릴 것인가? | QAS-U(사용성) 담당자와 협의 |
