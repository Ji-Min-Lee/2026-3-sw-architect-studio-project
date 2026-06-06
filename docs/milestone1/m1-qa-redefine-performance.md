# 품질 속성 시나리오 — 실시간 성능 / Quality Attribute Scenario — Real-Time Performance

**팀 / Team**: Blue Sky (3팀) | **마일스톤 / Milestone**: M1 | **마감 / Due**: 2026-06-09

---

## P-1: 실시간 오디오 처리 성능 (Real-Time Performance)

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **Source** | 기계식 시계 (음향 신호 발생) / Mechanical watch (acoustic signal source) |
| **Stimulus** | USB 센서를 통해 연속적인 음향 샘플 스트림 입력 / Continuous audio sample stream via USB sensor |
| **Artifact** | 오디오 캡처 파이프라인 — ALSA 드라이버 → Audio Thread → Ring Buffer (Dropped Block 직접 측정 경계). DSP 파이프라인(필터링 → T1/T3 감지 → 측정값 계산) 처리 속도가 Ring Buffer 점유율을 결정 / Audio capture pipeline — ALSA driver → Audio Thread → Ring Buffer (direct Dropped Block measurement boundary). DSP pipeline processing speed determines Ring Buffer occupancy |
| **Environment** | - 하드웨어: Raspberry Pi 5 (8GB RAM), Ubuntu 24.04 / Hardware: Raspberry Pi 5 (8GB RAM), Ubuntu 24.04<br>- 입력: USB PnP 마이크 연결, Live 모드 실행 중 / Input: USB PnP microphone connected, running in Live mode<br>- 측정 조건: 28,800 BPH 기계식 시계, 연속 10분 이상 / Measurement: 28,800 BPH mechanical watch, continuous ≥ 10 min<br>- 부하: Qt GUI 렌더링·오디오 처리·DSP 파이프라인 동시 실행 / Load: Qt GUI rendering, audio processing, and DSP pipeline running concurrently |
| **Response** | ALSA 드라이버가 생성하는 각 오디오 블록을 Ring Buffer 오버플로 없이 수신·저장한다. DSP 파이프라인(필터링 → T1/T3 감지 → 측정값 계산)이 블록 주기(1/Background FPS)를 초과하면 Ring Buffer가 차올라 Dropped Block이 발생하므로, 블록 주기 내 DSP 완료가 전제 조건이다. GUI 표시까지의 지연은 QAS-3에서 관리한다 / Store each audio block from the ALSA driver into the Ring Buffer without overflow. If the DSP pipeline (filtering → T1/T3 detection → measurement computation) exceeds the block period (1/Background FPS), the Ring Buffer fills and causes Dropped Blocks; completing DSP within the block period is therefore a prerequisite. GUI display latency is managed by QAS-3 |
| **Response Measure** | - **Objective**: 96,000 sps 처리 유지 / sustain 96,000 sps<br>- **Minimum**: 48,000 sps (이 이하면 프로젝트 실패 / below this = project failure)<br>- **Stretch**: 192,000 sps<br>- Dropped audio block: **0** (Ring Buffer 오버플로 없음 / no Ring Buffer overflow)<br>- *(잠정값 — EX-01 결과로 확정 / Provisional — confirmed by EX-01)* |

> **한국어**
>
> Dropped Block은 ALSA 드라이버 → Audio Thread → Ring Buffer 구간에서 발생하는 오버플로를 직접 측정한다. Ring Buffer가 오버플로되는 근본 원인은 DSP 파이프라인(필터링 → T1/T3 이벤트 감지 → Rate·Amplitude·Beat Error 계산)이 블록 주기보다 오래 걸리는 것이다. 즉, DSP 처리 속도는 Dropped Block의 **원인**이고, Ring Buffer 오버플로 여부가 **측정 지점**이다. SPS가 높을수록 블록 주기가 짧아져 DSP에 허용되는 시간이 줄어든다. RPi가 96k sps를 감당하지 못하면 48k sps로 폴백하여 블록 주기를 2배 늘려 안정성을 보장한다. GUI 표시 지연은 QAS-3 Low Latency에서 별도 관리한다.
>
> **English**
>
> Dropped Block directly measures overflow at the ALSA driver → Audio Thread → Ring Buffer boundary. The root cause of Ring Buffer overflow is the DSP pipeline (filtering → T1/T3 event detection → Rate·Amplitude·Beat Error computation) taking longer than the block period. In other words, DSP processing speed is the **cause** and Ring Buffer overflow is the **measurement point**. Higher SPS shortens the block period, reducing the time budget for DSP. If the RPi cannot sustain 96k sps, the system falls back to 48k sps, doubling the block period to guarantee stability. GUI display latency is managed separately under QAS-3 Low Latency.

---

## Priority

| 항목 / Item | 수준 / Level | 근거 / Rationale |
|-------------|:------------:|-----------------|
| **Importance** | High | Dropped Block이 발생하면 T1/T3 이벤트 시간 정보 자체가 소실되어 Rate·Amplitude·Beat Error 계산이 불가능하다. 다른 모든 QA의 선행 조건 / If Dropped Blocks occur, T1/T3 event timing data is lost and Rate·Amplitude·Beat Error computation becomes impossible. Prerequisite for all other QAs |
| **Difficulty** | High | RPi 5에서 96k sps 블록 주기(~10ms) 내에 DSP 파이프라인 완료가 가능한지 미확인이며, GUI 렌더링과 DSP가 동일 프로세스에서 경쟁하는 구조이므로 성능 달성이 어려움 / Unverified whether RPi 5 can complete the DSP pipeline within the 96k sps block period (~10ms); GUI rendering and DSP compete within the same process, making performance targets challenging |

---

## Risk

| 리스크 / Risk | 설명 / Description | 영향 / Impact |
|--------------|-------------------|--------------|
| **DSP 처리 지연 → Ring Buffer 오버플로** | DSP 파이프라인(필터링·FFT·RLS)이 블록 주기(96k: ~10ms)를 초과 → Ring Buffer 차오름 → Dropped Block / DSP pipeline exceeds block period (~10ms at 96k) → Ring Buffer fills → Dropped Block | T1/T3 이벤트 시간 소실 → Rate·Amplitude 측정 불가 / T1/T3 timing data lost → Rate·Amplitude measurement failure |
| **스케줄러 지터 → 간헐적 블록 주기 초과** | Linux 범용 스케줄러에서 오디오 스레드 우선순위가 낮아 간헐적으로 DSP 처리가 블록 주기를 초과 → Ring Buffer 일시적 오버플로 / Low audio thread scheduling priority causes intermittent DSP to exceed block period → transient Ring Buffer overflow | 산발적 Dropped Block → 장기 Rate 측정 오차 누적 / Sporadic Dropped Blocks → accumulated long-term Rate error |

---

## 실험 계획 / Experiment Plan

### EX-01: RPi 성능 벤치마크 / RPi Performance Benchmark

| 항목 / Item | 내용 / Detail |
|------------|--------------|
| **목적 / Goal** | SPS별로 Ring Buffer 오버플로(Dropped Block) 발생 여부를 측정하여 96k sps 목표 달성 가능 여부 결정 / Measure Ring Buffer overflow (Dropped Block) per SPS tier to determine whether 96k sps target is achievable |
| **방법 / Method** | 48k / 96k / 192k sps 각각 10분 연속 실행. Dropped Block 수·Background FPS 기록. GUI 표시 지연(end-to-end latency)은 QAS-3 실험에서 별도 측정 / Run each SPS tier for 10 min. Record Dropped Block count and Background FPS. GUI display latency (end-to-end) is measured separately in QAS-3 experiment |
| **측정 항목 / Metrics** | SPS \| 블록 주기 / Block period \| Dropped Blocks \| Background FPS |
| **완료 기준 / Done** | 96k sps에서 Dropped Block = 0 달성 확인. 미달 시 48k 폴백 결정 / Confirm Dropped Block = 0 at 96k sps; decide 48k fallback if not met |

---

## Tactic / Pattern

| 전술 · 패턴 / Tactic · Pattern | 적용 이유 / Rationale |
|-------------------------------|----------------------|
| **Lock-Free Ring Buffer** | Audio Thread(생산자)와 Processing Thread(소비자) 사이 Ring Buffer의 뮤텍스 제거 → 락 경합으로 인한 DSP 지연 방지 → Ring Buffer 오버플로(Dropped Block) 직접 예방 / Eliminating mutex on the Ring Buffer between Audio Thread (producer) and Processing Thread (consumer) prevents DSP delays from lock contention → directly prevents Ring Buffer overflow (Dropped Block) |
| **Graceful Degradation** | DSP가 96k sps 블록 주기(~10ms) 내에 완료되지 못할 경우 48k sps로 자동 전환하여 블록 주기를 ~20ms로 확보. 정밀도는 낮아지지만 Dropped Block = 0 보장 / Auto-switch to 48k sps when DSP cannot complete within the 96k block period (~10ms), securing ~20ms block period. Lower precision but Dropped Block = 0 guaranteed |
| **Priority Scheduling** | 오디오 처리 스레드에 높은 스케줄링 우선순위 부여 → Linux 스케줄러 지터로 인한 간헐적 블록 주기 초과 예방 → Ring Buffer 오버플로 방지 / Elevating audio thread scheduling priority prevents intermittent block period violations caused by Linux scheduler jitter → prevents Ring Buffer overflow |
