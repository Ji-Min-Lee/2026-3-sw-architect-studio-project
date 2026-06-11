# Escapement Analyzer & Marker-Line Display 그래프 / Escapement Analyzer & Marker-Line Display Graph

> **작성일 / Date**: 2026-06-12  
> **출처 / Source**: Time Grapher Project Plan (Draft) p.20 Fig 15, etimer.net reference

---

## 1. 목적 / Purpose

**한국어**

Escapement Analyzer는 각 beat(박동) 내부에서 발생하는 두 음향 이벤트, A(T1: impulse pin이 pallet fork를 타격)와 C(T3: escape wheel tooth가 pallet에 잠김)의 위치를 파형 위에 수직 마커 라인으로 표시하고, 그 간격(t_AC, ms)을 계산하여 시각화하는 디스플레이다.

핵심 질문: **"A 이벤트와 C 이벤트 사이의 시간 간격이 정확하고 beat간 안정적인가?"**

Rate/Amplitude 같은 상위 요약 값만으로는 볼 수 없는 **beat 내부의 fine-grained 타이밍 구조**를 진단하며, 특히 amplitude 계산의 근거가 되는 t_AC 값이 얼마나 안정적으로 측정되는지, C 이벤트의 onset 기준과 peak 기준 중 어느 쪽이 더 반복적인지 비교하여 더 신뢰할 수 있는 측정 기준을 결정하는 데 사용한다.

**English**

The Escapement Analyzer displays vertical timing markers for the A (T1: impulse pin strikes pallet fork) and C (T3: escape wheel locks on pallet) acoustic events overlaid on the raw waveform, and calculates the elapsed interval (t_AC, ms) between them.

Core question: **"Is the A-to-C timing interval accurate and stable beat-to-beat?"**

This display exposes fine-grained beat-internal timing structure that is invisible in higher-level summary values (Rate, Amplitude). It is used to assess t_AC measurement stability and to determine whether the C-event onset or peak reference point produces more repeatable amplitude estimates.

---

## 2. 데이터 소스 / Data Sources

**한국어**

| 소스 / Source | 내용 / Description |
|---|---|
| 마이크 PCM 오디오 | Raw acoustic waveform (샘플 스트림) |
| A 이벤트 타임스탬프 | T1: impulse pin이 pallet fork를 타격하는 시점 (가장 정밀하고 반복적) |
| C 이벤트 타임스탬프 | T3: escape wheel tooth가 pallet에 잠기고 fork가 banking pin을 타격하는 시점 |
| 샘플 클락 | 샘플 인덱스 → 시간 변환 (e.g. 48,000 Hz) |
| BPH 설정값 | 이상적 beat 간격 계산용 (I_target = 3600 / BPH seconds) |
| Lift Angle 설정값 | amplitude 계산 시 t_AC와 함께 사용 |

**English**

| 소스 / Source | 내용 / Description |
|---|---|
| Microphone PCM audio | Raw acoustic waveform sample stream |
| A event timestamp | T1: impulse pin strikes pallet fork (most precise, most repeatable) |
| C event timestamp | T3: escape wheel tooth locks on pallet; fork strikes banking pin |
| Sample clock | Converts sample index to time (e.g. 48,000 Hz) |
| BPH setting | Nominal beat interval I_target = 3600 / BPH seconds |
| Lift Angle setting | Used with t_AC to compute amplitude |

---

## 3. 연산 / Calculations

**한국어**

### A-C 간격 측정

```
t_AC = C_timestamp − A_timestamp    (seconds)
t_AC_ms = (c_index − a_index) / f_s × 1000
```

### Amplitude 계산 (t_AC 사용)

```
Amp = (3600 × λ) / (π × n × t_AC)

  λ = lift angle (degrees)
  n = BPH
  t_AC = A-to-C interval (seconds)
```

### Beat-to-beat Stability (σ)

```
mPeakHistory, mOnsetHistory: 최근 20개 beat의 t_AC rolling window
σ = sqrt( Σ(xi − mean)² / (N−1) )   (sample standard deviation)
```

**English**

### A-to-C interval

```
t_AC_ms = (c_index − a_index) / f_s × 1000
```

### Amplitude (uses t_AC)

```
Amp = (3600 × λ) / (π × n × t_AC)
```

### Beat-to-beat stability

Rolling window of N=20 beats; reports sample standard deviation σ for both peak and onset references, and identifies which reference is more stable.

---

## 4. 그래프 구성 / Graph Layout

**한국어**

### 축 / Axes

| 축 / Axis | 값 / Value | 단위 / Unit |
|---|---|---|
| X축 | A 이벤트 기준 상대 시간 (A = 0 ms) | ms |
| Y축 | 음향 신호 진폭 (raw PCM) | Amplitude (정규화) |

### 오버레이 요소 / Overlay Elements

| 요소 / Element | 색상 / Color | 역할 / Role |
|---|---|---|
| A 마커 수직선 | 녹색 (dark green) | T1 이벤트 위치 (항상 x = 0 ms) |
| C 마커 수직선 | 빨간색 (red) | T3 이벤트 위치 (peak 또는 onset 기준) |
| Δ ms 텍스트 | 검정 bold | 마커 중앙에 경과 시간 표시 (e.g. "Δ 9.43 ms") |
| A-C 구간 배경 | 반투명 오렌지 | A → C 측정 구간 하이라이트 |

### 컨트롤 바 / Control Bar

| 컨트롤 / Control | 설명 / Description |
|---|---|
| **C reference** 콤보박스 | Peak / Onset 전환. Onset 데이터가 없으면 "onset unavailable, using peak" 표시 |
| **A → C interval** 헤더 레이블 | 현재 beat의 t_AC 값과 사용 중인 reference 표시 |
| **σ stability** 레이블 | `σ peak: X ms | σ onset: Y ms (N=20) → Peak/Onset more stable` |

**English**

### Axes

X-axis: time relative to A event (A = 0 ms); Y-axis: raw PCM amplitude (normalised).

### Overlay elements

Green vertical line = A (T1) marker at x = 0 ms; red vertical line = C (T3) at computed t_AC; bold Δ label at midpoint; translucent orange background rect spanning the A-to-C interval.

### Control bar

C reference combo (Peak / Onset); interval header label; σ stability label showing rolling σ for both references and which is more stable.

---

## 5. 요구사항 / Requirements

**한국어**

Project Plan p.20 기준:

| 수준 / Level | 내용 / Requirement |
|---|---|
| **shall** (필수) | A, C 이벤트 마커 배치 및 표시 |
| **shall** (필수) | A-C 간격 ms 계산 및 표시 |
| **shall** (필수) | 마커가 waveform 신호 특징에 실시간 정렬 유지 |
| **should** (권고) | onset vs peak 비교로 더 안정적인 reference point 결정 지원 |

**English**

Based on Project Plan p.20:

| 수준 / Level | 내용 / Requirement |
|---|---|
| **shall** (mandatory) | Place and display markers for A and C events |
| **shall** (mandatory) | Calculate and display A-to-C elapsed time in ms |
| **shall** (mandatory) | Marker positions remain visually aligned with signal features in real time |
| **should** (recommended) | Allow user to compare onset vs peak to determine the more stable timing reference |

---

## 6. Witschi 비교 / Witschi Comparison

**한국어**

Witschi Chronoscope X1 G3에는 이 디스플레이의 직접 대응 모드가 없다. 레퍼런스 Figure 15는 Witschi가 아닌 **etimer.net**에서 인용.

가장 유사한 Witschi 모드는 **Scope Display Mode (5.6)**이지만 목적이 다르다:

| 항목 / Item | Witschi Scope | 우리 구현 / Our Implementation |
|---|---|---|
| 표시 대상 | Beat noise 파형 반복 패턴 (tic/tac 형태 비교) | 단일 beat 내 A-C 마커 간격 |
| 마커 | C beat 위치 표시 | A, C 이벤트 모두 마커 + ms 레이블 |
| 주요 목적 | 파형 형태와 반복성, lift angle 시각화 | fine-grained timing, t_AC 측정 정확도 |
| Zoom | 20 ms / 200 ms / 400 ms 선택 | 자유 zoom in/out (QCustomPlot) |
| Stability 수치 | 없음 | σ peak vs σ onset (N=20) |
| 레퍼런스 구간 강조 | 없음 | 반투명 오렌지 배경 |

**English**

No direct equivalent in Witschi X1 G3; Figure 15 source is etimer.net. The closest Witschi mode is Scope (5.6), but its purpose (beat-noise shape and repeatability) differs from this display's focus on A-to-C marker interval accuracy. See table above for a detailed comparison.

---

## 7. 우리 구현 / Our Implementation

**한국어**

### 7.1 구현 파일 / Implementation Files

| 파일 / File | 역할 / Role |
|---|---|
| `src/tabs/EscapementTab.h` | 클래스 선언, 멤버 변수 |
| `src/tabs/EscapementTab.cpp` | 파형 버퍼링, 이벤트 처리, 마커 렌더링, σ 계산 |

### 7.2 핵심 구현 내용 / Key Implementation Notes

- **Rolling PCM buffer**: 1초 분량의 raw PCM을 유지. A 이벤트 기준 -3 ms ~ C peak +5 ms 윈도우를 스냅샷으로 저장
- **Beat struct**: `aPos`, `cPeakPos`, `cOnsetPos`, `cOnsetValid` 필드로 한 beat의 모든 이벤트 타임스탬프 관리
- **σ rolling window**: `mPeakHistory`, `mOnsetHistory` 각각 최근 20개 beat t_AC 값을 저장. 2개 이상 쌓이면 σ 계산 및 비교 표시
- **A-C 구간 rect**: `QCPItemRect`로 `(0, yTop) → (cMs, yBot)` 설정. `yAxis->rescale()` 이후 좌표 캡처

### 7.3 Witschi 대비 추가 구현 / Additional vs Witschi

| 항목 / Item | 상태 / Status |
|---|---|
| onset unavailable fallback 표시 | ✅ 구현 |
| beat-to-beat σ 비교 | ✅ 구현 (Witschi에 없음) |
| A-C 구간 오렌지 배경 | ✅ 구현 (etimer.net Figure 15 기준) |

**English**

### 7.1 Implementation Files

See table in Korean section above.

### 7.2 Key Implementation Notes

- **Rolling PCM buffer**: maintains 1 second of raw PCM; snapshots a window from A−3 ms to C_peak+5 ms per beat.
- **Beat struct**: holds `aPos`, `cPeakPos`, `cOnsetPos`, `cOnsetValid` timestamps for one beat.
- **σ rolling window**: `mPeakHistory` and `mOnsetHistory` store the last 20 beats' t_AC values; σ and winner label update each beat.
- **A-C rect**: `QCPItemRect` from `(0, yTop)` to `(cMs, yBot)`, coordinates captured after `yAxis->rescale()`.

---

## 8. TC 목록 및 실행 결과 / Test Cases and Results

**한국어**

테스트 파일: `src/tests/test_added_tabs.cpp`  
실행 타겟: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|---|---|---|
| `escapementTab_deltaMs_matchesEventSpacing` | A/C 이벤트 간격이 `currentEscapementMs()` 반환값과 일치 | ✅ PASS |
| `escapementTab_reset_clearsState` | `reset()` 호출 시 history 및 플롯 상태 초기화 | ✅ PASS |

**English**

Test file: `src/tests/test_added_tabs.cpp`  
Build target: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|---|---|---|
| `escapementTab_deltaMs_matchesEventSpacing` | A/C event spacing matches `currentEscapementMs()` return value | ✅ PASS |
| `escapementTab_reset_clearsState` | History and plot state cleared after `reset()` | ✅ PASS |

### 실행 결과 / Execution Output

```
********* Start testing of TestAddedTabs *********
PASS   : TestAddedTabs::escapementTab_deltaMs_matchesEventSpacing()
PASS   : TestAddedTabs::escapementTab_reset_clearsState()
Totals: 16 passed, 0 failed, 0 skipped, 0 blacklisted, 143ms
********* Finished testing of TestAddedTabs *********
```
