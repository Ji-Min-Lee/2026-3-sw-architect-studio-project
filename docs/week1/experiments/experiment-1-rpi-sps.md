# Experiment 1 — RPi5 처리 성능 측정 / RPi5 Processing Performance

> **작성일 / Date**: 2026-06-02  
> **목표 완료일 / Target**: 2026-06-11 (Week 2, M1 제출 직후)  
> **담당 / Owner**: TBD

---

## 실험 목표 / Goal

> RPi5가 각 샘플레이트(48k / 96k / 192k Hz)에서 4,096개 샘플 묶음을  
> 다음 묶음이 도착하기 전에 처리할 수 있는가?  
>
> Can the RPi5 process each 4,096-sample chunk before the next one arrives,  
> at each target sample rate (48k / 96k / 192k Hz)?

---

## 배경 / Background

마이크는 쉬지 않고 PCM 데이터를 보냅니다. 코드는 4,096개씩 묶어서 처리하므로,  
각 묶음의 처리 시간이 아래 허용값을 초과하면 데이터가 쌓이다가 유실됩니다.

The microphone continuously sends PCM data. Since the code processes 4,096 samples per chunk,  
if any chunk takes longer than the budget below, data accumulates and eventually drops.

| 샘플레이트 / Sample Rate | 허용 처리 시간 / Budget |
|------------------------|----------------------|
| 48,000 Hz | 4096 / 48000 = **85.3 ms** |
| 96,000 Hz | 4096 / 96000 = **42.7 ms** |
| 192,000 Hz | 4096 / 192000 = **21.3 ms** |

**프로젝트 QA 목표 / Project QA Targets**

- 최소 / Minimum: 48,000 Hz
- 목표 / Target: 96,000 Hz
- 도전 / Stretch: 192,000 Hz

---

## 측정 방법 / Measurement Method

### Step 1 — 타이머 코드 삽입 / Insert Timer Code

`src/MainWindow.cpp`의 `ProcessSamples()` 함수 안, `tg_process()` 앞뒤에 추가합니다.

```cpp
// [추가] 처리 시작 시간
auto t_start = std::chrono::high_resolution_clock::now();

tg_process(mCtx, mInputBlock, slice, &r);  // 기존 코드 (변경 없음)

// [추가] 처리 종료 시간 및 로그
auto t_end = std::chrono::high_resolution_clock::now();
double elapsed_ms = (t_end - t_start).count() / 1e6;
double budget_ms  = slice * 1000.0 / mCurrentSamplesPerSecond;

qInfo() << QString("tg_process: slice=%1  elapsed=%2ms  budget=%3ms  %4")
               .arg(slice)
               .arg(elapsed_ms, 0, 'f', 3)
               .arg(budget_ms,  0, 'f', 1)
               .arg(elapsed_ms < budget_ms ? "OK" : "OVER");
```

헤더 추가도 필요합니다 (`MainWindow.cpp` 상단):

```cpp
#include <chrono>
```

---

### Step 2 — RPi5에서 Sim 모드로 실행 / Run on RPi5 in Sim Mode

실제 시계 없이도 됩니다. Sim 모드가 가짜 신호를 생성합니다.

```
UI 설정:
  Mode        → Sim
  BPH         → 21600 (또는 28800)
  Amplitude   → 300°
  Beat Error  → 0.0 ms
  Error Rate  → 0 s/d
  Realistic   → 체크
```

각 샘플레이트별 측정 절차:

```
1. Sample Rate → 48000 Hz 선택
2. Start 클릭
3. 5분 이상 실행 (온도 안정화 대기)
4. 로그 수집 → 파일 저장
5. Stop → Sample Rate → 96000 Hz 변경
6. 반복
7. 192000 Hz 반복
```

> **5분 이상 실행하는 이유**: RPi5는 발열 시 CPU 클럭을 자동으로 낮춥니다 (thermal throttling).  
> 처음 1분은 괜찮아도 5분 후 느려질 수 있으므로, 안정화된 상태에서 측정해야 합니다.

---

### Step 3 — CPU 사용률 및 온도 동시 측정 / Measure CPU Usage and Temperature

별도 터미널에서 동시에 실행합니다.

```bash
# 1초마다 온도 + CPU 사용률 출력
watch -n 1 "echo '--- $(date) ---' && vcgencmd measure_temp && top -bn1 | grep 'Cpu(s)'"
```

또는 로그 파일로 저장:

```bash
while true; do
  echo "$(date +%H:%M:%S) $(vcgencmd measure_temp) $(top -bn1 | grep 'Cpu(s)' | awk '{print $2}')"
  sleep 1
done >> rpi_monitor.log
```

---

## 결과 판단 기준 / Pass / Fail Criteria

```
elapsed_ms < budget_ms × 0.5   → ✅ 여유 있음 (그래프 렌더링 추가해도 안전)
elapsed_ms < budget_ms × 0.8   → ⚠️  가능하나 빠듯 (렌더링 최적화 필요)
elapsed_ms > budget_ms          → ❌ 실시간 불가 (해당 샘플레이트 포기)
```

CPU 사용률:

```
CPU < 50%   → ✅ 렌더링 추가 여유 있음
CPU 50~80%  → ⚠️  렌더링 최적화 필요
CPU > 80%   → ❌ 렌더링 추가 시 실시간 불가 위험
```

---

## 결과 기록표 / Results Table

| 샘플레이트 | 허용 시간 | 평균 처리 시간 | 최악 처리 시간 | CPU 평균 | 온도 | 판정 |
|-----------|---------|-------------|-------------|---------|-----|-----|
| 48,000 Hz | 85.3 ms | - | - | - | - | - |
| 96,000 Hz | 42.7 ms | - | - | - | - | - |
| 192,000 Hz | 21.3 ms | - | - | - | - | - |

---

## 결과에 따른 아키텍처 결정 / Architecture Decision Based on Results

| 결과 시나리오 | 아키텍처 결정 |
|------------|------------|
| 48k ✅, 96k ✅, 192k ✅ | 192k stretch 목표 달성 가능 → 그대로 진행 |
| 48k ✅, 96k ✅, 192k ❌ | 목표(96k) 달성 → 192k stretch 포기 |
| 48k ✅, 96k ❌ | 목표 미달 → DSP 최적화 또는 처리 방식 변경 검토 |
| 48k ❌ | 최소 목표 미달 → 구조적 문제 → 즉시 팀 리뷰 |

이 결과는 **M1 문서의 QA 수치** 및 **Architectural Approaches**에 반영됩니다.

---

## 관련 문서 / Related Documents

- [프로젝트 QA 정의](../kickoff-workshop/qa-consensus.md)
- [신호 처리 기초 개념](../kickoff-workshop/graph-analysis-total.md) — 신호 처리 기초 개념 섹션
- [코드 구조 분석](../development-team/code-walkthrough.md)
