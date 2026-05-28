# Phase 0: 필수 문서 정독 결과

> Week 0 산출물 — 프로젝트 착수 전 도메인·요구사항·공식 이해 기록

---

## 정독 대상 문서

| 파일 | 범위 | 상태 |
|------|------|------|
| `Time Grapher Project Plan (Draft).pdf` | 전체 (10 pp.) | ✅ 완료 |
| `TimeGrapher Equations_v0.docx.pdf` | 전체 (11 pp.) | ✅ 완료 |
| `Witschi-Training-Course.pdf` | pp. 14–19 | ✅ 완료 |

---

## 1. 도메인 이해 — Swiss Lever Escapement 음향 이벤트

기계식 시계 1비트당 3개의 음향 이벤트가 순서대로 발생한다.

```
[T1 (A)]  Impulse pin → Pallet fork        ← 가장 정밀·반복적 → Rate, Beat Error 계산
[T2 (B)]  Escape wheel tooth → Pallet stone ← 불규칙 → 측정 미사용
[T3 (C)]  Escape wheel locks + Fork → Banking pin ← T1과 pair → Amplitude 계산
```

> **핵심**: 소프트웨어가 실제로 감지·사용해야 하는 이벤트는 **T1(A)와 T3(C)** 뿐이다.
> T2(B)는 신호가 불규칙해 계산에 포함하지 않는다.

### 측정값 정상 기준

| 측정값 | 단위 | 정상 (OK) | 주의 | 문제 |
|--------|------|-----------|------|------|
| Rate | s/d | -5 ~ +15 | ±15 초과 | ±30 초과 |
| Amplitude (H, 수평) | ° | 250 ~ 330 | 220 ~ 250 | 200 이하 |
| Amplitude (V, 수직) | ° | 250 ~ 270 | 220 ~ 250 | 200 이하 |
| Beat Error | ms | 0.0 ~ 0.5 | 0.5 ~ 3.0 | 3.0 초과 |

---

## 2. 핵심 계산 공식 (TimeGrapher Equations)

### 2-1. Rate Error — 순간 오차 그래프

beat n의 순간 오차:

```
E_n = T_measured - (T_start + n × I_target)
```

- `T_start`: 첫 번째 beat의 타임스탬프 (anchor)
- `I_target`: 이상적 beat 간격 = 3600 / BPH (초)
- `E_n`: 누적 타이밍 오차 → Y축에 플롯

플롯 Y 좌표 (화면 범위를 벗어나지 않도록 modulo 적용):

```
Y = E_n (mod Plot Height)
```

**trace가 직선으로 기울어질수록 rate error가 크다.** 수평에 가까울수록 정확한 시계.

#### Rate 수치 계산 (tic/tac 분리)

A 이벤트 스트림 A₀, A₁, A₂, A₃, A₄ … 에서:

```
T_tic = A_(2k+2) - A_(2k)       # tic-to-tic period
T_tac = A_(2k+3) - A_(2k+1)     # tac-to-tac period
T_nom,same-phase = 7200 / BPH   # 같은 phase 간 이상 간격

rate_tic = 86400 × (T_nom,same-phase / T_tic - 1)
rate_tac = 86400 × (T_nom,same-phase / T_tac - 1)
Rate     = (rate_tic + rate_tac) / 2              # s/d
```

> tic과 tac을 분리해 계산하는 이유: beat error가 존재할 때 두 phase가 비대칭이므로
> 단순 평균보다 오염이 적다.

sample index 기반 구현 (fs = 샘플레이트):

```
T_tic = (n_(2k+2) - n_(2k)) / fs
T_tac = (n_(2k+3) - n_(2k+1)) / fs
```

#### 즉시 플롯 pseudocode

```cpp
if (first_beat) {
    T_start = T_measured;
    n = 0;
}
ideal_time = T_start + n * I_target;
E_n = T_measured - ideal_time;
Y = wrap_or_scale(E_n);
plot_point(x_index=n, y=Y);
n++;
```

---

### 2-2. Beat Error

연속된 세 A 이벤트 A₀, A₁, A₂로 반주기 비대칭을 측정:

```
t1 = A₁ - A₀    # 첫 번째 반박
t2 = A₂ - A₁    # 두 번째 반박

BE = (t1 - t2) / 2    # signed; 표시는 |BE|
```

- 0 ms = 완전 대칭 (이상적)
- 0.5 ms 이하 = 양호
- sample index 기반: `BE = ((n₁-n₀) - (n₂-n₁)) / (2 × fs)`

> Rate는 "시계가 얼마나 빠르거나 느린가", Beat Error는 "두 반박이 얼마나 대칭인가"—
> 두 값은 독립적이며 동시에 표시되어야 한다.

---

### 2-3. Amplitude

balance wheel의 각도 스윙을 **A→C 간격**으로 추정:

```
Amp = (3600 × λ) / (π × n × t_AC)

  λ      = lift angle (°) — 사용자 설정, 기본 52°
  n      = BPH (beats per hour)
  t_AC   = 같은 beat packet의 A 이벤트 onset → C 이벤트 peak 간격 (초)
```

sample index 기반:

```
t_AC = (c_idx - a_idx) / fs
Amp  = (3600 × λ × fs) / (π × n × (c_idx - a_idx))
```

**주의**: A와 C는 반드시 같은 beat packet에서 pair해야 한다. 다른 beat의 A-C를 묶으면 계산 오류.

**실용 감각**: t_AC가 작을수록 Amp가 크다 (강한 스윙 = 빠른 복귀 = 짧은 A-C 간격).

#### 28,800 bph 예시

| 입력 | 계산 |
|------|------|
| λ = 52°, n = 28,800, t_AC = 0.009 s | Amp = (3600 × 52) / (π × 28800 × 0.009) ≈ 230° |

---

## 3. 그래프 해석 — Trace 패턴 카탈로그 (Witschi pp.14-15)

### 정상 패턴

| 패턴 | 의미 |
|------|------|
| 점들이 촘촘하고 직선에 가까운 약간의 기울기 | 정상 (Rate ±15 s/d 이내) |
| 두 선이 거의 평행하고 간격 좁음 | Beat error 양호 |

### 비정상 패턴과 원인

| 패턴 | 원인 | 조치 |
|------|------|------|
| 두 선 간격 넓음 (beat error ~3 ms) | Beat error 과다 | beat error 조정 후 rate 재조정 |
| 선 전체가 가파르게 위/아래로 기울음 | rate 빠름/느림 | rate 조정 |
| 포지션별 rate 차이 큼 (H +30, V -40) | balance 편심, 마그네틱, 마모 | 조정·교체 |
| 정규 주기로 큰 파형 변동 | gear train 결함 | gear train 점검 |
| 불규칙 산란 패턴, 낮은 amplitude | 전체적 이상 | 오버홀 |
| 두 선이 가끔 분리됨 (knocking) | amplitude 과다(>330°) → double tic-tac | mainspring/pallet stone 교체 |

---

## 4. Scope 파형 해석 — 오류 감지 (Witschi pp.16-19)

Scope 화면에서 beat 파형의 **비정상적 피크 위치와 개수**로 물리적 결함을 식별한다.

### 파형 패턴 → 물리 원인 매핑

| Scope 파형 특징 | 물리적 원인 |
|----------------|------------|
| 첫 번째 피크가 작고 두 번째가 큼 | Escapement fitting too weak |
| 두 피크가 모두 강하고 간격 가까움 | Escapement fitting too strong |
| 두 번째 피크가 앞으로 이동 (Unlocking 강함) | Unlocking too strong |
| 첫 번째 피크 아래로 꺾임 (화살표↓) | Additional friction |
| 두 beat에서 모두 이상 피크 (양쪽 화살표↓) | Dart touching the roller |
| 두 번째 피크 뒤에 이상 피크 (화살표↓) | Not enough clearance (horns ↔ impulse pin) |
| 피크 세기가 beat마다 교번함 | Weak amplitude |
| 피크 사이 간격이 비규칙적 | Too much axial end shake (pivot) |
| 두 beat 양쪽에 knocking 피크 | Fork horn touches impulse pin |
| 파형에 노이즈 꼬리 (위아래 화살표) | Rough pivot / seizure |
| beat 끝에 추가 피크 (화살표↓) | Grazing balance wheel / hair spring |
| 한 beat에서 피크 하나만 나타남 | Escape wheel tooth penetrates impulse plane |

> **구현 관점**: Scope Function(F0/F1/F2/F3)은 이 패턴들을 필터 처리 단계별로
> 시각화해 사용자가 원인을 판독할 수 있도록 돕는 것이 목적이다.

---

## 5. 현재 베이스라인 GUI 구조 요약

```
┌─────────────────────────────────────────────────┐
│  Measurement Summary Bar                         │
│  (RATE / AMPLITUDE / BEAT ERROR / BEAT 표시)     │
├─────────────────────────────────────────────────┤
│  Tabbed Graph Panel                              │
│  ┌──────────────┬──────────────┐                 │
│  │ Rate/Scope   │ Sound Print  │  ← 현재 2개     │
│  │   Tab        │    Tab       │  ← 여기에 추가  │
│  └──────────────┴──────────────┘                 │
├─────────────────────────────────────────────────┤
│  Control Panel                                   │
│  Run Params │ Watch Params │ Sim Params │ Misc   │
└─────────────────────────────────────────────────┘
```

**확장 포인트**: Tabbed Graph Panel에 새 탭을 추가하는 방식으로 11개 그래프를 구현한다.
기존 코드 대규모 수정 없이 탭 추가만으로 확장 가능한 구조가 Extensibility QA의 핵심이다.

### 구현 대상 그래프 목록 (우선순위 순)

| 우선순위 | 그래프 | 사용 데이터 |
|---------|--------|------------|
| 1 | Trace Display | Rate deviation + Amplitude 연속 기록 |
| 2 | Rate & Amplitude Stability (Vario) | Min/Max/Avg/σ 통계 |
| 3 | Beat Error Display & Diagnostic Trace | Rate/Amplitude/Beat Error 수치 |
| 4 | Beat-Noise Scope (Scope 1 & 2) | 개별 beat 파형 + Σ 평균화 |
| 5 | Multi-Position Sequence Display | 최대 10 포지션 비교 |
| 6 | Long-Term Performance Graph | 장시간 변화 추이 |
| 7 | Escapement Analyzer & Marker-Line Display | A/C 이벤트 마커 |
| 8 | Time-Frequency Spectrogram | 시간-주파수 에너지 분포 |
| 9 | Waveform Comparison Display | 연속 beat 정렬 비교 |
| 10 | Scope Mode (Synchronized Sweep) | oscilloscope 스타일 |
| 11 | Scope Function (F0/F1/F2/F3) | 4가지 필터 처리 뷰 |

---

## 6. 아키텍처 목표와 채점 연계

| QA | 목표 수치 | 데모 증거 |
|----|----------|----------|
| Real-Time Performance | 96,000 sps (최소 48,000 sps) | FPS + sps 수치 화면 표시 |
| Low Latency | end-to-end 최소화 | 캡처→처리, 처리→표시, 전체 ms 단위 측정값 |
| Correctness | WeiShi 1000 기준 일치 | 동일 시계 동시 비교 측정 결과 |
| Measurement Accuracy | T1/T3 onset 정밀 감지 | beat event 오감지율 |
| Extensibility | 새 그래프 추가 = 최소 파일 변경 | Module view 기반 영향 분석 설명 |

---

## 7. 주요 주의사항 (구현 시 함정)

1. **AGC 반드시 비활성화** — Raspberry Pi AlsaMixer에서 확인. 활성화 시 신호 왜곡으로 모든 측정값 오염.
2. **Lift Angle은 사용자 설정값** — 52°가 흔하지만 universal이 아님. 하드코딩 금지.
3. **A-C pair는 같은 beat 내에서만** — 다른 beat의 A와 C를 묶으면 Amplitude 계산 오류.
4. **T_start는 매 restart마다 갱신** — 화면 초기화·재시작 시 새 anchor로 재설정해야 trace가 올바르게 그려짐.
5. **rate 계산은 tic/tac 분리** — 단순 평균 interval로 계산하면 beat error 존재 시 rate 수치 오염.
