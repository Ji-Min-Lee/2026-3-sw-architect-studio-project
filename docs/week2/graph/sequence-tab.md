# Sequence 그래프 / Sequence Graph

> **작성일 / Date**: 2026-06-12  
> **출처 / Source**: Time Grapher Project Plan (Draft) Fig 10, Witschi Chronoscope X1 G3 Instruction Manual p.15

---

## 1. 목적 / Purpose

**한국어**

Sequence 그래프는 시계를 여러 자세(포지션)에 놓고 측정한 Rate, Beat Error, Amplitude를 한 화면에 모아서 비교하는 표 형식의 디스플레이다. 포지션별 성능 차이를 한눈에 파악하고, 수직/수평 포지션 간 평균 차이(DVH)를 통해 balance-wheel imbalance 또는 positional fault를 진단하는 데 사용한다.

**English**

The Sequence graph is a table-format display that collects Rate, Beat Error, and Amplitude measurements taken at multiple watch test positions into a single view. It allows the user to compare performance across positions at a glance and diagnose balance-wheel imbalance or positional faults via the vertical-vs-horizontal mean difference (DVH).

---

## 2. 요구사항 / Requirements

**한국어**

Project Plan p.16 기준:

| 수준 / Level | 내용 / Requirement |
|-------------|-------------------|
| **shall** (필수) | 최대 10개 포지션 결과를 캡처하여 하나의 시퀀스로 표시 |
| **shall** (필수) | 각 포지션에서 Rate, Amplitude, Beat Error를 계산·표시 |
| **shall** (필수) | X: 전체 포지션 평균 표시 |
| **shall** (필수) | D: 최대값 − 최소값 표시 |
| **should** (권고) | 수직/수평 포지션 비교(DVH)로 balance-wheel unbalance 지표 제공 |

**English**

Based on Project Plan p.16:

| 수준 / Level | 내용 / Requirement |
|-------------|-------------------|
| **shall** (mandatory) | Capture results for up to 10 test positions in a single sequence |
| **shall** (mandatory) | Calculate and display Rate, Amplitude, Beat Error per position |
| **shall** (mandatory) | X: mean across all captured positions |
| **shall** (mandatory) | D: max − min across captured positions |
| **should** (recommended) | DVH comparison (vertical − horizontal mean) to indicate balance-wheel imbalance |

---

## 3. Witschi Ground Truth

**한국어**

Witschi Chronoscope X1 G3 매뉴얼 p.15 Figure 10 기준 레퍼런스 화면:

| 포지션 / Position | Rate (s/d) | Beat (ms) | Ampl (°) |
|-----------------|-----------|-----------|----------|
| CH | 1.5 | 0.0 | 300 |
| CB | 3.0 | 0.2 | 297 |
| 9H | 2.2 | 0.0 | 262 |
| 6H | 4.0 | 0.1 | 267 |
| 3H | -0.4 | 0.3 | 262 |
| 12H | -3.2 | 0.2 | 262 |
| X | 1.2 | 0.1 | 275 |
| D | 7.2 | 0.3 | 38 |
| DVH | -1.6 | — | -36 |
| Di | 2.5 | — | — |

Witschi 주요 특징:
- DVH와 Di가 별도 테이블 행으로 표시됨
- Beat 열에서 DVH/Di는 빈칸(`—`)
- Lateral Toolbar(우측 사이드 레일)의 포지션 버튼이 모든 화면에서 공통으로 표시됨

**English**

Reference screen from Witschi Chronoscope X1 G3 Manual p.15 Figure 10 (see table above).

Key characteristics:
- DVH and Di displayed as dedicated table rows
- Beat column is blank (`—`) for DVH/Di rows
- Position buttons on the Lateral Toolbar are shared across all display modes

---

## 4. 우리 구현 / Our Implementation

**한국어**

### 4.1 표 구조 / Table Structure

| 행 / Row | Rate (s/d) | Beat (ms) | Ampl (°) |
|---------|-----------|-----------|----------|
| CH ~ CD(L) (10개 포지션) | 소수 1자리 | 소수 1자리 | 정수 |
| X (mean) | 전체 평균 | 전체 평균 | 전체 평균 |
| D (max−min) | 최대−최소 | 최대−최소 | 최대−최소 |
| DVH | 수직 평균 − 수평 평균 | — | 수직 평균 − 수평 평균 |

### 4.2 포지션 목록 / Position List

Witschi 기본 6개(CH, CB, 9H, 6H, 3H, 12H) + 추가 4개(CU(R), CU(L), CD(R), CD(L)) = 최대 10개 지원. Project Plan "up to 10 test positions" 요구사항을 충족한다.

### 4.3 DVH 계산식 / DVH Formula

```
DVH_Rate = mean(9H, 6H, 3H, 12H)_Rate − mean(CH, CB)_Rate
DVH_Ampl = mean(9H, 6H, 3H, 12H)_Ampl − mean(CH, CB)_Ampl
DVH_Beat = — (표시 안 함)
```

### 4.4 Witschi 대비 차이점 / Differences from Witschi

| 항목 / Item | Witschi | 우리 구현 / Our Implementation |
|------------|---------|-------------------------------|
| 포지션 수 / Position count | 6개 (기본) | 10개 |
| DVH | 테이블 행 | 테이블 행 ✅ |
| Di | 테이블 행 | 미구현 (요구사항에 없음) |
| 포지션 선택 UI | 우측 사이드 버튼 레일 | 상단 공통 ComboBox (모든 탭 공통) |
| Pause | 우측 사이드 버튼 | 상단 공통 버튼 (모든 탭 공통) |

**English**

### 4.1 Table Structure

| 행 / Row | Rate (s/d) | Beat (ms) | Ampl (°) |
|---------|-----------|-----------|----------|
| CH ~ CD(L) (10 positions) | 1 decimal | 1 decimal | integer |
| X (mean) | overall mean | overall mean | overall mean |
| D (max−min) | max−min | max−min | max−min |
| DVH | vertical mean − horizontal mean | — | vertical mean − horizontal mean |

### 4.2 Position List

Witschi baseline 6 (CH, CB, 9H, 6H, 3H, 12H) + 4 additional (CU(R), CU(L), CD(R), CD(L)) = up to 10 positions, satisfying the Project Plan requirement.

### 4.3 DVH Formula

```
DVH_Rate = mean(9H, 6H, 3H, 12H)_Rate − mean(CH, CB)_Rate
DVH_Ampl = mean(9H, 6H, 3H, 12H)_Ampl − mean(CH, CB)_Ampl
DVH_Beat = — (not shown)
```

### 4.4 Differences from Witschi

See table in Korean section above.

---

## 5. TC 목록 및 실행 결과 / Test Cases and Results

**한국어**

테스트 파일: `src/tests/test_added_tabs.cpp`  
실행 타겟: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|--------------------|---------------------------|--------------|
| `sequenceTab_capture_recordsAtActivePosition` | 활성 포지션에 Rate/Beat/Ampl 캡처 및 X·D 계산 | ✅ PASS |
| `sequenceTab_reset_clearsTable` | reset() 호출 시 모든 셀이 `—`로 초기화 | ✅ PASS |
| `sequenceTab_dvh_rate_and_ampl` | DVH Rate = 수직 평균 − 수평 평균, DVH Ampl 동일, Beat는 `—` | ✅ PASS |
| `sequenceTab_dvh_missing_positions_shows_dash` | 수직 또는 수평 포지션이 없으면 DVH가 `—` | ✅ PASS |

**English**

Test file: `src/tests/test_added_tabs.cpp`  
Build target: `TestAddedTabs`

| TC 이름 / Test Name | 검증 내용 / What It Verifies | 결과 / Result |
|--------------------|---------------------------|--------------|
| `sequenceTab_capture_recordsAtActivePosition` | Capture Rate/Beat/Ampl at active position; verify X and D computation | ✅ PASS |
| `sequenceTab_reset_clearsTable` | All cells reset to `—` after reset() | ✅ PASS |
| `sequenceTab_dvh_rate_and_ampl` | DVH Rate = vertical mean − horizontal mean; Ampl same; Beat is `—` | ✅ PASS |
| `sequenceTab_dvh_missing_positions_shows_dash` | DVH shows `—` when vertical or horizontal positions are missing | ✅ PASS |

### 실행 결과 / Execution Output

```
********* Start testing of TestAddedTabs *********
PASS   : TestAddedTabs::sequenceTab_capture_recordsAtActivePosition()
PASS   : TestAddedTabs::sequenceTab_reset_clearsTable()
PASS   : TestAddedTabs::sequenceTab_dvh_rate_and_ampl()
PASS   : TestAddedTabs::sequenceTab_dvh_missing_positions_shows_dash()
Totals: 16 passed, 0 failed, 0 skipped, 0 blacklisted, 131ms
********* Finished testing of TestAddedTabs *********
```
