# Latency QA Revision

**Branch**: docs/m1-qa-redefine-bhan-tj | **작성**: 2026-06-04  
**근거**: 멘토 피드백 #4 (Latency + 실험 연결), #5 (SPS와 Latency 함께 논의)

> 아래 4개 섹션을 각 대상 파일에 반영한다. 각 섹션은 삽입/교체 위치와 최종 텍스트를 명시한다.

---

## 0. 코드 분석 결과 (전제 수정)

이전 버전에서 "블록 크기 4,096 샘플 고정 → capture→process 하한 = 4096 ÷ sps × 1000ms"로 기술했으나, 코드 확인 결과 이 전제가 틀렸다.

| 경로 | 실제 청크 크기 | 근거 |
|------|-------------|------|
| WAV 재생 (`PlaybackWorker.cpp:23`) | `sps / 50` (Linux RPi 기준 20ms 고정) | `PLAYBACK_SAMPLE_PERIOD_MSEC = 20` |
| 라이브 캡처 (`AudioWorker.cpp:71`) | `readAll()` — Qt/OS가 결정 | `QAudioSource` 버퍼 주기, 미설정 |
| `DETECTOR_NUMBER_OF_SAMPLES = 4096` | `tg_process()` max slice 상한 | 실제 전달 단위 아님 |

따라서 capture→process 지연 하한은 **sps와 무관하게 OS 오디오 콜백 주기**로 결정된다.
- WAV 모드: ~20ms (코드에서 확정)
- 라이브 모드: QAudioSource 기본 버퍼 주기 (미설정, EX-01 실측 필요)

sps가 latency에 미치는 영향은 콜백 주기가 아니라 **콜백당 처리해야 할 샘플 수 → CPU 부하**를 통해 간접적으로 나타난다.

---

## 1. architectural-drivers.md — QAS-3 교체

**위치**: `### QAS-3: 낮은 지연 / Low Latency ★ Priority 3` 섹션 전체를 아래로 교체

---

### QAS-3: 낮은 지연 / Low Latency ★ Priority 3

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **Source** | 기계식 시계 (beat 이벤트 발생) / Mechanical watch (beat event source) |
| **Stimulus** | 마이크에서 음향 샘플 블록 캡처 시작 / Audio sample block capture starts from microphone |
| **Artifact** | capture → beat detection → 계산 → GUI 렌더링 전 구간 / Full pipeline: capture → beat detection → computation → GUI rendering |
| **Environment** | Raspberry Pi 5, 실시간 운영 상태 / real-time operating state |
| **Response** | 캡처된 샘플에 대한 파형, 마커, 계산값이 GUI에 표시됨 / Waveform, markers, and computed values for captured samples displayed in GUI |
| **Response Measure** | ① capture→process: **< 70 ms** *(잠정 — EX-01 실측으로 확정)* <br>② process→display: **< 30 ms** *(잠정 — EX-01 실측으로 확정)* <br>③ end-to-end (①+②): **< 100 ms** (28,800 BPH 기준) *(잠정 — EX-01 실측으로 확정)* <br>Missed beat: **0** (end-to-end가 beat 주기 초과 시 즉시 위반 — hard constraint) <br>Dropped audio block: **0** |

#### 수치 근거 / Latency Target Derivation

**핵심 제약**: end-to-end 지연이 beat 주기를 초과하면 다음 beat 처리 전에 이전 beat 표시가 완료되지 않아 기능 붕괴.  
**안전 마진 80%** 적용 (OS 지터, Qt 렌더링 변동 흡수 목적).

| BPH | Beat 주기 | 80% 마진 → end-to-end 목표 | ① capture→process (70%) | ② process→display (30%) |
|-----|---------|--------------------------|------------------------|------------------------|
| 28,800 | 125 ms | 100 ms | 70 ms | 30 ms |
| 36,000 | 100 ms | 80 ms | 56 ms | 24 ms |
| **43,200** | **83 ms** | **66 ms** | **46 ms** | **20 ms** |

| 옵션 | 지원 범위 | end-to-end | ① | ② | 비고 |
|:---:|---------|-----------|---|---|-----|
| **Option A** *(현재 목표)* | ~28,800 BPH | **< 100 ms** | **< 70 ms** | **< 30 ms** | 기본 목표 |
| **Option B** *(stretch)* | ~43,200 BPH | < 66 ms | < 46 ms | < 20 ms | EX-01 결과 후 상향 검토 |

*현재 잠정값은 Option A (28,800 BPH 기준). EX-01 결과 후 Option B 상향 가능 여부 팀 합의로 결정.*

> ⚠️ **Team decision required — supported BPH range not yet confirmed (to be decided after verifying available watches)**

#### SPS와 Latency의 관계 / Relationship between SPS and Latency

capture→process 지연의 하한은 sps가 아니라 **OS 오디오 콜백 주기**로 결정된다.  
현재 코드에서 오디오 데이터는 OS가 콜백을 호출할 때마다 일괄 전달되며, 콜백 주기는 sps와 무관하게 ~20ms 단위로 동작한다 (WAV 재생 기준 확인; 라이브 캡처는 `QAudioSource` 기본값 적용 중).

sps는 latency를 직접 결정하지 않지만, **콜백당 처리 샘플 수를 통해 CPU 부하에 간접 영향**을 준다:

| sps | 콜백당 샘플 수 (20ms 기준) | CPU 부하 경향 | capture→process 하한 |
|-----|------------------------|------------|-------------------|
| 48,000 | 960 샘플 | 낮음 | ~20 ms *(EX-01 실측)* |
| **96,000** | **1,920 샘플** | **중** *(기준)* | **~20 ms** *(EX-01 실측)* |
| 192,000 | 3,840 샘플 | 높음 | ~20 ms *(EX-01 실측)* |

> **결정 원칙**: sps 선택의 핵심 기준은 latency 하한이 아니라 **CPU 부하 (Dropped block = 0 유지 가능 여부)**다.  
> EX-01에서 48k / 96k / 192k sps 각각의 CPU 점유율, Dropped block 수, 실제 콜백 주기를 측정하여 최종 sps를 결정한다.
>
> *→ [architectural-approaches.md — SPS 선택과 레이턴시](./architectural-approaches.md) 및 [EX-01](./planned-experiments.md#ex-01) 참조*

With the current implementation, the capture→process latency floor is determined by the **OS audio callback period** (~20ms), not by sps. sps affects CPU load per callback — higher sps means more samples to process per 20ms window — which can indirectly threaten latency if the CPU cannot keep up (causing dropped blocks).

**Common constraints**: Dropped audio block: **0**, Missed beat: **0** — violated immediately upon latency overrun *(see EX-01)*

---

## 2. architectural-approaches.md — 신규 섹션 삽입

**위치**: `### AT-05: 버퍼 크기 상한 ...` 섹션 직후, `## 4. 아키텍처 ↔ 드라이버 매핑` 직전에 삽입

---

### [분석] SPS 선택과 레이턴시 예산 / SPS Selection and Latency Budget

AT-04(sps 폴백)와 AT-05(블록 크기 상한)의 관계를 이해하려면 실제 오디오 데이터 전달 구조를 먼저 파악해야 한다.

**오디오 전달 구조**:
- 라이브 캡처: `QAudioSource.readAll()` — OS가 콜백 주기마다 일괄 전달 (주기 미설정, EX-01 실측)
- WAV 재생: 20ms 주기로 `sps/50` 샘플씩 전달 (코드 확정: `PLAYBACK_SAMPLE_PERIOD_MSEC = 20`)
- `DETECTOR_NUMBER_OF_SAMPLES = 4096`: `tg_process()` max slice 상한 — 실제 전달 단위 아님

**capture→process 하한 = OS 콜백 주기 (~20ms), sps와 무관**  
*sps별 콜백 샘플 수 및 CPU 부하 수치 → [QAS-3 SPS와 Latency의 관계](./architectural-drivers.md#qas-3) 참조*

**트레이드오프**:
- sps ↑ → 신호 해상도 ↑, beat 감지 정확도 잠재적 향상 / 콜백당 처리 샘플 수 ↑ → CPU 부하 ↑ (QAS-1 위협)
- sps ↓ → CPU 부하 ↓ / 신호 해상도 ↓
- AT-05의 4,096 샘플 상한은 tg_process() 호출 단위 제한 — 모든 sps에서 실제 콜백 샘플 수(≤ 3,840)가 상한 이하이므로 현재 구현에서 활성화되지 않음

**결정**: 최종 sps는 EX-01에서 CPU 점유율·Dropped block 수를 실측하여 QAS-1(Dropped block = 0)을 만족하는 최고 sps로 결정한다. capture→process latency는 sps 선택과 독립적으로 ~20ms 수준으로 예상되며, 실측으로 확정한다.

AT-04 (sps fallback) and AT-05 (block size cap) are independent tactics. The actual audio delivery unit is determined by the OS callback period (~20ms), not by the block size constant. sps affects CPU load per callback, not the capture→process latency floor directly.

**Decision**: Final sps is determined by EX-01 measurement of CPU usage and dropped block count. capture→process latency is expected at ~20ms regardless of sps, to be confirmed empirically.

*→ [EX-01](./planned-experiments.md#ex-01) 참조*

---

## 3. planned-experiments.md — EX-01 목적 교체

**위치**: `## EX-01` 아래 `### 목적 / Objective` 섹션 전체를 아래로 교체

---

### 목적 / Objective

이 실험은 **두 가지를 동등한 목표로** 측정한다:

1. **sps 확정**: Qt GUI가 실행 중인 RPi 5에서 오디오 블록 드롭 없이 지속할 수 있는 최대 sps는 얼마인가? (QAS-1)
2. **레이턴시 확정**: 선택된 sps에서 end-to-end 지연(capture→display) 3구간이 QAS-3 목표(< 100 ms) 이내인가? (QAS-3)

capture→process 지연 하한은 sps가 아니라 OS 오디오 콜백 주기로 결정된다 (WAV 재생 기준 ~20ms 확인; 라이브 캡처는 미설정). 따라서 sps 선택의 핵심 기준은 **CPU 부하** — 높은 sps일수록 콜백당 처리 샘플 수가 늘어 Dropped block 위험이 증가한다. 이 실험은 48k / 96k / 192k sps **각각에서** CPU 성능과 실제 3구간 레이턴시를 **동시에** 측정하여 최종 sps를 결정한다.

| 측정 항목 / Metric | 잠정 목표치 / Provisional Target | 측정 방법 / How to Measure | 관련 QAS |
|-----------------|-------------------------------|--------------------------|---------|
| 최대 sps | 96k 유지 (불가 시 48k 폴백) | Dropped block = 0 유지 가능한 최고 sps | QAS-1 |
| OS 콜백 주기 | ~20 ms (예상) | 콜백 간격 실측 (라이브 캡처) | QAS-1/3 |
| ① capture→process | **< 70 ms** *(잠정)* | 오디오 콜백 시작 → beat event 타임스탬프 | QAS-3 |
| ② process→display | **< 30 ms** *(잠정)* | beat event 타임스탬프 → GUI 화면 갱신 완료 | QAS-3 |
| ③ end-to-end | **< 100 ms** *(잠정)* | ① + ② 전체 | QAS-3 |

---

## 4. risk-assessment-revision.md — OI-03 행 교체

**위치**: `## 3. 대응 액션 및 계획 실험` 테이블에서 `OI-03` 행 전체를 아래로 교체

---

| OI-03 | **EX-01**: RPi에서 48k / 96k / 192k sps × Qt GUI 조건별 **CPU 점유율 · Dropped block · GUI FPS · OS 오디오 콜백 주기 · end-to-end 레이턴시 3구간(capture→process / process→display / end-to-end) 동시 실측**. sps 선택 기준은 CPU 부하(Dropped block = 0 유지); capture→process 하한은 sps 무관하게 OS 콜백 주기로 결정됨을 실측으로 확인 / EX-01: Simultaneously measure CPU usage, dropped blocks, GUI FPS, OS audio callback period, and **3-segment end-to-end latency** at 48k / 96k / 192k sps with Qt GUI. sps selection criterion is CPU load (Dropped block = 0); capture→process floor is determined by OS callback period independently of sps. | sps별 CPU 성능 + **3구간 레이턴시 실측 수치 확보**; OS 콜백 주기 확인; QAS-1 목표 sps 및 QAS-3 레이턴시 잠정값 확정; 48k 폴백 여부 결정 완료 / Per-sps CPU performance + **3-segment latency figures collected**; OS callback period confirmed; QAS-1 target sps and QAS-3 latency confirmed; 48k fallback decision finalized |
