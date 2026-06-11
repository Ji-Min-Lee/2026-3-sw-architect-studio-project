# Long-term 성능 그래프 / Long-Term Performance Graph

> **작성일 / Date**: 2026-06-12  
> **출처 / Source**: Time Grapher Project Plan (Draft) p.19 Fig 14, Witschi Chronoscope X1 G3 Instruction Manual p.14 (Trace), p.15 (Vario)

---

## 1. 목적 / Purpose

**한국어**

Long-term 그래프는 시계의 Rate(s/d), Amplitude(°), Beat Error(ms) 세 가지 측정값이 장시간에 걸쳐 어떻게 변화하는지를 공통 시간축 위에 동시에 표시하는 다중 시계열 그래프다. 단기 측정(분 단위)에서는 보이지 않는 파워 리저브 감소, 날짜판 작동 같은 주기적 충격, 온도 드리프트 등 장기 변화를 수 시간 단위로 추적하여 시계의 장기 일관성과 기계 결함 여부를 진단하는 데 사용한다.

**English**

The Long-Term Performance Graph is a multi-series time-series display that plots Rate (s/d), Amplitude (°), and Beat Error (ms) simultaneously on a shared time axis over an extended period. It captures slow-moving changes invisible in short measurements — such as power-reserve decay, periodic shocks from date-change complications, or temperature drift — enabling diagnosis of long-term consistency and mechanical faults.

---

## 2. 요구사항 / Requirements

**한국어**

Project Plan p.19 기준:

| 수준 / Level | 내용 / Requirement |
|-------------|-------------------|
| **shall** (필수) | Rate / Amplitude / Beat Error 세 가지 측정값을 장시간 시계열로 기록·표시 |
| **shall** (필수) | 전체 측정 구간의 평균(mean) 시각 표시 |
| **shall** (필수) | 전형적 변동 범위(±σ 밴드) 시각 표시 |
| **shall** (필수) | 경과 시간 증가에 따라 업데이트 빈도를 줄여 장시간 세션 지원 |
| **should** (권고) | 파워 리저브 감소, 날짜 변경 등 주기적 변동 포착 가능 |

**English**

Based on Project Plan p.19:

| 수준 / Level | 내용 / Requirement |
|-------------|-------------------|
| **shall** (mandatory) | Record and display Rate / Amplitude / Beat Error as long-term time series |
| **shall** (mandatory) | Show overall mean visually as a reference line |
| **shall** (mandatory) | Show typical variation range as a ±σ band |
| **shall** (mandatory) | Reduce update frequency as elapsed time grows to support multi-hour sessions |
| **should** (recommended) | Capture periodic fluctuations such as power-reserve decay and date-change shocks |

---

## 3. Witschi Ground Truth

**한국어**

Witschi Chronoscope X1 G3 매뉴얼에 Long-term 그래프와 1:1 대응하는 모드는 없다. 아래 두 모드를 결합·확장한 형태가 요구사항에 해당한다.

| 항목 / Item | Witschi Trace (p.14) | Witschi Vario (p.15) | Long-term (요구사항) |
|------------|----------------------|----------------------|---------------------|
| 표시 측정값 | Rate + Amplitude | Rate + Amplitude 통계 | **Rate + Amplitude + Beat Error** |
| 시간축 | 연속 시계열 (수 분) | 없음 (통계 요약) | 연속 시계열 (수 시간) |
| 평균선 | 측정 종료 후 수치 표시 | X̄ 수치 표시 | **rolling mean 시각 표시** |
| 범위 밴드 | 없음 | Min/Max 화살표 + 허용 범위 초록 밴드 | **±σ 밴드 시각 표시** |
| 업데이트 방식 | 연속 실시간 스크롤 | 연속 실시간 업데이트 | **경과 시간에 따라 bucket 크기 증가** |
| Beat Error 트렌드 | 없음 (수치만) | 없음 (수치만) | **시계열 그래프로 표시** |
| 허용 범위 기준선 | 없음 | 없음 | **점선으로 표시** |

Witschi Trace + Vario를 결합하고 Beat Error 시계열 및 adaptive update를 추가한 형태다. 레퍼런스 Figure 14는 Witschi가 아닌 Watch-C-Scope 1.4 화면이다.

**English**

The Witschi Chronoscope X1 G3 has no single mode that directly maps to the Long-Term graph. The requirement corresponds to a combination and extension of two Witschi modes (Trace + Vario), as shown in the table above.

The reference screenshot is Figure 14 from the Project Plan, sourced from Watch-C-Scope 1.4, not from Witschi.

---

## 4. 데이터 소스 및 연산 / Data Sources and Computation

**한국어**

### 4.1 데이터 소스 / Data Sources

| 소스 / Source | 내용 / Content | 역할 / Role |
|--------------|---------------|------------|
| 마이크 음향 신호 | A(T1) · C(T3) 이벤트 타임스탬프 | 모든 측정값의 원천 |
| 샘플 클록 | 48,000 / 96,000 / 192,000 sps | 타임스탬프 정밀도 |
| Watch Parameters | BPH, Lift Angle (λ) | Amplitude 계산 입력 |
| 측정 이력 버퍼 | 누적 (timestamp, rate, amp, BE) 레코드 | 그래프 플롯 데이터 |

### 4.2 연산 / Formulas

**Rate (s/d)**

```
T_tic = A_{2k+2} − A_{2k}          (tic-to-tic same-phase period)
T_tac = A_{2k+3} − A_{2k+1}        (tac-to-tac same-phase period)
T_nom = 7200 / BPH

rate_tic = 86400 × (T_nom / T_tic − 1)
rate_tac = 86400 × (T_nom / T_tac − 1)
Rate     = (rate_tic + rate_tac) / 2   [s/d]
```

**Amplitude (°)**

```
t_AC = C_timestamp − A_timestamp   [seconds]
Amp  = (3600 × λ) / (π × BPH × t_AC)   [degrees]
```

**Beat Error (ms)**

```
t1 = A_1 − A_0,  t2 = A_2 − A_1   [seconds]
BE = |t1 − t2| / 2 × 1000          [ms]
```

**English**

See formulas above (language-independent).  
Source: `TimeGrapher Equations_v0.docx.pdf` Part I–IV.

---

## 5. 그래프 구조 / Graph Structure

**한국어**

세 개의 서브그래프가 수직으로 스택되며 X축(시간)은 공통으로 동기화된다.

| 서브그래프 / Subplot | X축 / X-Axis | Y축 / Y-Axis | 색상 / Color | 허용 범위 기준선 / Tolerance |
|-------------------|-------------|-------------|------------|--------------------------|
| Rate | 경과 시간 (s) | s/d | 분홍 (170, 30, 90) | ±5 s/d 점선 |
| Amplitude | 경과 시간 (s) | 도(°) | 파랑 (40, 70, 200) | 270°, 310° 점선 |
| Beat Error | 경과 시간 (s) | ms | 초록 (30, 140, 60) | 0.6 ms 점선 |

각 서브그래프에 공통 오버레이:
- **평균선(mean line)**: 전체 누적 평균, 동색 점선
- **±σ 밴드**: 전체 표준편차 기준 반투명 배경 밴드

**English**

Three subgraphs stacked vertically; all X-axes are synchronised.

See table above for axis definitions, colors, and tolerance reference lines.

Common overlays per subplot:
- **Mean line**: running cumulative mean, dashed line in series color
- **±σ band**: semi-transparent filled band around the mean

---

## 6. Adaptive Bucket Aggregation

**한국어**

장시간 세션에서 데이터 포인트 수를 제한하기 위해 경과 시간에 따라 bucket 크기를 증가시킨다.

| 경과 시간 / Elapsed | Bucket 크기 / Size | 효과 / Effect |
|--------------------|-------------------|--------------|
| 0 ~ 5분 | 1 (개별 측정) | 최대 해상도 |
| 5 ~ 30분 | 10 | 10개 평균을 1포인트로 |
| 30분 ~ 2시간 | 30 | 30개 평균을 1포인트로 |
| 2시간 이상 | 60 | 60개 평균을 1포인트로 |

**English**

To limit the number of plotted points over long sessions, the bucket size increases with elapsed time (see table above). Each plotted point represents the average of `bucketSize` consecutive measurements.

---

## 7. Witschi 대비 차이점 / Differences from Witschi

**한국어**

| 항목 / Item | Witschi | 우리 구현 / Our Implementation |
|------------|---------|-------------------------------|
| Beat Error 트렌드 | 수치만 표시 | 시계열 그래프 ✅ |
| 허용 범위 기준선 | 없음 | Rate ±5 s/d, Amp 270/310°, BE 0.6 ms 점선 ✅ |
| 업데이트 방식 | 연속 실시간 | 시간 기반 adaptive bucket ✅ |
| X축 동기화 | N/A (별도 모드) | 세 서브그래프 공통 시간축 ✅ |
| 평균선 | 종료 후 수치 | 실시간 시각 표시 ✅ |

**English**

See table above.

---

## 8. TC 목록 및 실행 결과 / Test Cases and Results

**한국어**

테스트 파일: `src/tests/test_added_tabs.cpp`  
실행 타겟: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|--------------------|---------------------------|--------------|
| `longTermTab_timeAndRate_matchInput` | 두 measurement의 X(시간)·Y(Rate) 값이 입력과 일치 | ✅ PASS |
| `longTermTab_threeSeries_populateIndependently` | Rate·Amplitude·Beat Error 세 시리즈가 독립적으로 포인트를 추가 | ✅ PASS |
| `longTermTab_invalidRate_notDrawn` | rateValid=false인 경우 Rate 그래프에 포인트가 추가되지 않음 | ✅ PASS |

**English**

Test file: `src/tests/test_added_tabs.cpp`  
Build target: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|--------------------|---------------------------|--------------|
| `longTermTab_timeAndRate_matchInput` | X (elapsed time) and Y (Rate) values match input across two measurements | ✅ PASS |
| `longTermTab_threeSeries_populateIndependently` | Rate, Amplitude, Beat Error series each receive points independently | ✅ PASS |
| `longTermTab_invalidRate_notDrawn` | No point added to Rate graph when rateValid is false | ✅ PASS |

### 실행 결과 / Execution Output

```
********* Start testing of TestAddedTabs *********
PASS   : TestAddedTabs::longTermTab_timeAndRate_matchInput()
PASS   : TestAddedTabs::longTermTab_threeSeries_populateIndependently()
PASS   : TestAddedTabs::longTermTab_invalidRate_notDrawn()
Totals: 16 passed, 0 failed, 0 skipped, 0 blacklisted, 170ms
********* Finished testing of TestAddedTabs *********
```
