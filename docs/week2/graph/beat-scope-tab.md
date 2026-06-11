# Beat Scope 그래프 / Beat Scope Graph

> **작성일 / Date**: 2026-06-12  
> **출처 / Source**: Time Grapher Project Plan (Draft) p.17, Witschi Chronoscope X1 G3 Instruction Manual p.19

---

## 1. 목적 / Purpose

**한국어**

Beat Scope 그래프는 기계식 시계의 박동 소음(beat noise)을 음향 엔벨로프(|진폭|) 파형으로 시각화하는 오실로스코프 형태의 디스플레이다. 개별 tic/tac 파형의 형태를 관찰하여 팔레트 포크의 작동 이상, 탈진기 마모, 진폭 불균형 등 기계적 결함을 진단하는 데 사용한다.

- **Scope 1**: 가장 최근 beat 또는 선택한 과거 beat의 전체 파형을 확대 표시. A(녹색)·C(빨간색) 마커로 T1(팔레트 포크 발동 시점)과 T3(임펄스 피크 시점)를 표시한다.
- **Scope 2**: tic과 tac 파형을 20 ms 구간으로 나란히 표시. Σ 평균 기능으로 최대 50개 beat를 누적 평균하여 랜덤 노이즈를 제거한다.

**English**

The Beat Scope graph is an oscilloscope-style display that visualises the acoustic envelope (|amplitude|) of each mechanical watch beat. By inspecting the shape of individual tic/tac waveforms, it helps diagnose mechanical faults such as pallet-fork malfunctions, escapement wear, or amplitude imbalance.

- **Scope 1**: Enlarged view of the most recent beat (or a selected past beat), with A (green) and C (red) markers for T1 (pallet-fork onset) and T3 (impulse peak).
- **Scope 2**: Side-by-side 20 ms view of tic vs. tac. The Σ averaging mode accumulates up to 50 beats to suppress random noise.

---

## 2. 요구사항 / Requirements

**한국어**

Project Plan p.17 기준:

| 수준 / Level | 내용 / Requirement |
|-------------|-------------------|
| **shall** (필수) | 박동 소음을 음향 엔벨로프(|신호|) 파형으로 표시 |
| **shall** (필수) | A(T1) 이벤트에 녹색 마커, C(T3) 이벤트에 빨간색 마커 표시 |
| **shall** (필수) | 시간 범위 선택: 20 ms / 200 ms / 400 ms |
| **shall** (필수) | 최근 beat 소음이 현재 파형 하단에 작은 스트립으로 표시 |
| **shall** (필수) | Lift angle 정보 표시 |
| **should** (권고) | tic과 tac 파형을 분리된 두 축에 나란히 표시 (Scope 2) |
| **should** (권고) | 최대 50 tic + 50 tac 누적 평균으로 노이즈 감소 (Σ averaging) |

**English**

Based on Project Plan p.17:

| 수준 / Level | 내용 / Requirement |
|-------------|-------------------|
| **shall** (mandatory) | Display beat noise as acoustic envelope (|signal|) waveform |
| **shall** (mandatory) | Green marker for A (T1) event; red marker for C (T3) event |
| **shall** (mandatory) | Selectable time range: 20 ms / 200 ms / 400 ms |
| **shall** (mandatory) | Most recent beat noises appear as small strips beneath the current waveform |
| **shall** (mandatory) | Lift angle readout |
| **should** (recommended) | Separate tic/tac display on two stacked axes (Scope 2) |
| **should** (recommended) | Up to 50 tic + 50 tac accumulated average for noise reduction (Σ averaging) |

---

## 3. Witschi Ground Truth

**한국어**

Witschi Chronoscope X1 G3 매뉴얼 p.19 기준 레퍼런스 화면:

| 항목 / Item | 내용 / Description |
|------------|-------------------|
| X축 / X-axis | 시간 (ms) |
| Y축 / Y-axis | \|진폭\| (절댓값 엔벨로프) |
| A 마커 (녹색 점선) | T1: 팔레트 포크 발동 시점 |
| C 마커 (빨간색 점선) | T3: 임펄스 피크 시점 |
| 하단 strip bar | 최근 beat 파형 썸네일 (클릭으로 선택) |
| Scope 2 상단 | tic 파형 (20 ms 고정) |
| Scope 2 하단 | tac 파형 (20 ms 고정) |
| Σ averaging | 50 tic / 50 tac 누적 평균 |

Witschi 주요 특징:
- 파형은 raw PCM이 아닌 처리된 엔벨로프(|진폭|)를 표시
- Scope 1과 Scope 2가 동일 화면 내 전환 방식으로 표시됨
- Lift angle이 정보 레이블로 함께 표시됨

**English**

Reference screen from Witschi Chronoscope X1 G3 Manual p.19:

Key characteristics:
- Waveform shows processed envelope (|amplitude|), not raw PCM
- Scope 1 and Scope 2 are toggled within the same panel
- Lift angle is shown as part of the info label

---

## 4. 우리 구현 / Our Implementation

**한국어**

### 4.1 그래프 구조 / Graph Structure

**Scope 1**

| 요소 / Element | 내용 / Description |
|--------------|-------------------|
| X축 | Time (ms), 범위: 0 ~ 선택된 range (20/200/400 ms) |
| Y축 | \|Amplitude\| |
| A 마커 | darkGreen 점선, T1 위치 |
| C 마커 | Red 점선, T3 위치 |
| 하단 BeatStripBar | 최근 10개 beat 썸네일, 클릭 시 mBeatCombo 연동 |
| 정보 레이블 | Lift angle, A/C 마커 설명, beat 인덱스 및 tic/tac 표시 |

**Scope 2**

| 요소 / Element | 내용 / Description |
|--------------|-------------------|
| 상단 축 X | Time (ms), 0 ~ 20 ms 고정 |
| 상단 축 Y | \|Amplitude\| (tic) |
| 하단 축 X | Time (ms), 0 ~ 20 ms 고정 |
| 하단 축 Y | \|Amplitude\| (tac) |
| Σ averaging | 50 tic / 50 tac 누적 합산 후 평균 |

### 4.2 데이터 소스 / Data Source

| 소스 / Source | 설명 / Description |
|-------------|-------------------|
| `Measurement.pcm` | 엔벨로프 필터링된 PCM (절댓값 처리됨) |
| `AcousticEvent.isA` | true → A이벤트 (T1), false → C이벤트 (T3) |
| `AcousticEvent.samplePos` | 이벤트의 절대 샘플 인덱스 |
| `AcousticEvent.isTic` | tic(true) / tac(false) 구분 |
| `Measurement.samplesPerSecond` | 샘플레이트 (기본 48000 Hz) |

### 4.3 핵심 연산 / Key Computations

```
// 버퍼: 최근 1초 |pcm| 롤링 유지
mBuf ← abs(Measurement.pcm), max 48000 samples

// beat 캡처 (kPreEventMs=2ms 선행 + range samples)
startAbs = aPos − (2ms × sps)
ys[k]    = mBuf[startAbs + k]   (k = 0 .. need−1)

// 마커 위치
xMs(marker) = (markerAbsPos − startAbs) / sps × 1000.0

// Scope 2 평균 (len2 = 20ms × sps)
sum[k] += ys[k]    (누적)
avg[k]  = sum[k] / count
```

### 4.4 Witschi 대비 차이점 / Differences from Witschi

| 항목 / Item | Witschi | 우리 구현 / Our Implementation |
|------------|---------|-------------------------------|
| 뷰 전환 | 동일 화면 내 토글 | QStackedWidget (Scope 1 / Scope 2) ✅ |
| Beat strip bar | 하단 썸네일 스트립 | BeatStripBar 커스텀 위젯 ✅ |
| 선택 가능 beat 수 | 명시 없음 | 최근 10개 |
| Σ averaging 사이클 | 50 | 50 ✅ |
| Lift angle 표시 | 정보 레이블 | 정보 레이블 ✅ |

**English**

### 4.1 Graph Structure

See tables in Korean section above.

### 4.2 Data Source

| Source | Description |
|--------|-------------|
| `Measurement.pcm` | Envelope-filtered PCM (absolute value) |
| `AcousticEvent.isA` | true → A event (T1), false → C event (T3) |
| `AcousticEvent.samplePos` | Absolute sample index of the event |
| `AcousticEvent.isTic` | Distinguishes tic (true) from tac (false) |
| `Measurement.samplesPerSecond` | Sample rate (default 48000 Hz) |

### 4.3 Key Computations

See code block in Korean section above.

### 4.4 Differences from Witschi

See table in Korean section above.

---

## 5. TC 목록 및 실행 결과 / Test Cases and Results

**한국어**

테스트 파일: `src/tests/test_added_tabs.cpp`  
실행 타겟: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|--------------------|---------------------------|--------------|
| `beatNoise_capturesBeatWaveformWindow` | A이벤트 수신 후 beat 파형 윈도우가 캡처되고 ys 크기가 rangeSamples 이상 | ✅ PASS |
| `beatNoise_reset_clearsCaptures` | reset() 호출 시 beat 캡처 데이터가 모두 소거됨 | ✅ PASS |

**English**

Test file: `src/tests/test_added_tabs.cpp`  
Build target: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|--------------------|---------------------------|--------------|
| `beatNoise_capturesBeatWaveformWindow` | After A event received, beat waveform window is captured with ys.size() ≥ rangeSamples | ✅ PASS |
| `beatNoise_reset_clearsCaptures` | All captured beat data cleared after reset() | ✅ PASS |

### 실행 결과 / Execution Output

```
********* Start testing of TestAddedTabs *********
PASS   : TestAddedTabs::beatNoise_capturesBeatWaveformWindow()
PASS   : TestAddedTabs::beatNoise_reset_clearsCaptures()
Totals: 16 passed, 0 failed, 0 skipped, 0 blacklisted, 150ms
********* Finished testing of TestAddedTabs *********
```
