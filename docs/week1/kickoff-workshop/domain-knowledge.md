# TimeGrapher 도메인 지식

> **작성일**: 2026-06-02  
> **출처**: TimeGrapher Equations_v0.docx.pdf

---

## 1. 시계 소리가 나는 원리

기계식 시계 안에는 **탈진기(escapement)** 라는 부품이 있습니다. 태엽 에너지를 일정하게 흘려보내는 역할을 합니다.

```
[태엽] → 에너지 공급 → [탈진기] → 규칙적인 진동 → 시계바늘 구동
```

탈진기가 한 번 동작할 때마다 **3개의 충격음**이 발생합니다.

---

## 2. 이벤트 종류 (A/B/C = T1/T2/T3)

하나의 beat(tic 또는 tac) 안에서 3개의 소리가 순서대로 납니다.

```
소리 1 ──── 소리 2 ── 소리 3
  A            B         C
 (T1)         (T2)      (T3)
```

| 이벤트 | 다른 이름 | 발생 원인 | 사용 여부 |
|--------|---------|---------|---------|
| **A** | **T1** | 임펄스 핀이 팔레트 포크를 치는 순간 | ✅ Rate, Beat Error 계산 |
| **B** | **T2** | 탈진 휠이 팔레트 스톤에 닿는 순간 | ❌ 불규칙해서 사용 안 함 |
| **C** | **T3** | 탈진 휠이 잠기고 포크가 뱅킹핀에 닿는 순간 | ✅ Amplitude 계산 |

- **A(T1)**: 3개 중 가장 타이밍이 일정하고 정밀 → Rate·Beat Error의 기준점
- **B(T2)**: 마찰·튐 등으로 불규칙 → 사용 안 함
- **C(T3)**: A와의 시간 간격(t_AC)이 밸런스 휠 흔들림 크기에 비례 → Amplitude 계산

---

## 3. BPH (Beats Per Hour)

> "이 시계의 탈진기가 1시간에 몇 번 진동하는가"

기계식 시계마다 설계된 진동 속도가 다릅니다.

| BPH | 특징 |
|-----|------|
| 18,000 | 저속, 오래된 시계 |
| 21,600 | 일반적 |
| **28,800** | 현대 시계 표준 |
| 36,000 | 고속, 고급 시계 |

**28,800 BPH 시계의 beat 간격 계산:**

```
1시간 = 3,600초 동안 28,800번 beat
→ 1 beat당 걸리는 시간 = 3600 / 28800 = 0.125초 = 125ms

tic──tac──tic──tac──tic
 125ms 125ms 125ms 125ms
```

**같은 위상 주기(tic→tic):**

```
tic ──── tac ──── tic
 125ms        125ms
 └──── 250ms ────┘

T_nom = 7200 / BPH = 7200 / 28800 = 0.250s = 250ms
```

---

## 4. Rate (단위: s/day)

> "이 시계는 하루에 몇 초 빠르거나 느린가"

### 개념

```
이상적인 A 이벤트 (28,800 BPH):
A0        A1        A2        A3
|----125ms----|----125ms----|----125ms----|

실제 측정 (시계가 빠른 경우):
A0        A1        A2        A3
|---124ms----|---124ms----|---124ms----|
              (계속 조금씩 일찍 옴 = 시계가 빠름)
```

정상 범위: **±5 s/day 이내**

### 계산 — tic과 tac을 분리하는 이유

Beat Error가 존재하면 tic 간격과 tac 간격이 미묘하게 다릅니다. 섞어서 계산하면 비대칭이 Rate를 오염시키므로 **같은 위상끼리** 묶어서 계산합니다.

```
tic 위상: A0 → A2 → A4 (짝수 인덱스)
tac 위상: A1 → A3 → A5 (홀수 인덱스)
```

### Step 1 — 같은 위상 주기 측정

```
T_tic = A2 - A0   (tic → 다음 tic 까지 걸린 시간)
T_tac = A3 - A1   (tac → 다음 tac 까지 걸린 시간)
```

### Step 2 — 각 위상의 Rate 계산

```
rate_tic = 86400 × (T_nom / T_tic - 1)
rate_tac = 86400 × (T_nom / T_tac - 1)
```

- `T_nom / T_tic > 1` → 실제가 이상보다 짧음 → 시계 빠름 → Rate +
- `T_nom / T_tic < 1` → 실제가 이상보다 길음 → 시계 느림 → Rate -
- 86400을 곱하는 이유: 초 단위 비율을 **하루(86,400초) 기준 s/day로 환산**

### Step 3 — 최종 Rate

```
Rate = (rate_tic + rate_tac) / 2
```

**예시 (28,800 BPH):**

| 항목 | 값 |
|------|---|
| T_nom | 250ms |
| T_tic (측정값) | 249.980ms |
| T_tac (측정값) | 249.970ms |
| rate_tic | +6.912 s/day |
| rate_tac | +10.368 s/day |
| **Rate** | **+8.64 s/day** |

### 그래프로 보면

```
위로 올라가는 선   → Rate + (빠름)
아래로 내려가는 선 → Rate - (느림)
평평한 선          → Rate ≈ 0 (정확)
두껍고 흩어진 선   → 노이즈 많거나 Beat Error 큼
```

---

## 5. Beat Error (단위: ms)

> "tic과 tac 간격이 얼마나 비대칭인가"

### 완벽한 시계라면

```
tic ────────── tac ────────── tic
     125ms          125ms
```

tic → tac 간격과 tac → tic 간격이 **정확히 같아야** 합니다.

### Beat Error가 있는 시계

탈진기 조정이 약간 틀어지면:

```
tic ──────────────── tac ──────── tic
        t1=125.8ms        t2=124.2ms
```

```
Beat Error = (t1 - t2) / 2
           = (125.8 - 124.2) / 2
           = 0.8ms
```

2로 나누는 이유: t1과 t2 각각이 이상적 125ms에서 **절반씩** 벗어난 것이기 때문

- t1 = t2 → Beat Error = 0 (완벽한 대칭)
- t1 ≠ t2 → 탈진기 조정이 필요한 상태

정상 범위: **0.6ms 이하**

### Rate와 비교

| | Rate | Beat Error |
|--|------|-----------|
| 질문 | 전체적으로 빠른가 느린가? | 좌우 진동이 대칭인가? |
| 원인 | 탈진기 전체 속도 | 탈진기 좌우 균형 |
| 단위 | s/day | ms |

> Rate가 ±5 s/day로 정상이어도 Beat Error가 크면 시계 조정이 필요한 상태

### QA와 연결

- **Measurement Accuracy**: t1과 t2를 A 이벤트 타이밍으로 계산하므로, A 이벤트를 1샘플이라도 잘못 잡으면 Beat Error 수치가 틀어짐
- **Correctness**: Beat Error 뷰와 Trace 그래프가 동일한 A 이벤트 데이터를 써야 일관성 유지

---

## 6. Amplitude (단위: °)

> "밸런스 휠이 얼마나 크게 흔들리는가"

### 밸런스 휠이 뭔가

시계 안에서 좌우로 흔들리는 원형 부품. 이 흔들림이 탈진기를 구동하는 에너지원입니다.

```
태엽 에너지 충분 → 밸런스 휠이 크게 흔들림 → Amplitude 높음
태엽 에너지 부족 → 밸런스 휠이 작게 흔들림 → Amplitude 낮음
```

### A와 C 이벤트의 관계

한 beat 안에서 A에서 C까지의 시간(t_AC)으로 계산합니다.

```
A ────────────── C
(밸런스 휠 충격)    (탈진 휠 잠김)
└──── t_AC ─────┘
```

- 밸런스 휠이 **크게** 흔들릴수록 → A에서 C까지 **빨리** 도달 → t_AC **작아짐**
- 밸런스 휠이 **작게** 흔들릴수록 → A에서 C까지 **느리게** 도달 → t_AC **커짐**

**t_AC와 Amplitude는 반비례**

### 계산식

```
Amplitude = (3600 × λ) / (π × BPH × t_AC)

λ     = lift angle (도) — 시계 기종마다 다름, 보통 52°
BPH   = beats per hour
t_AC  = A에서 C까지 걸린 시간 (초)
```

정상 범위:

```
270~310° → 강함 (태엽 충분)
220~250° → 허용 범위
그 이하  → 태엽 부족 또는 부품 마모
```

### QA와 연결

- **Measurement Accuracy**: t_AC가 분모에 있어서 C 이벤트 타이밍 오차가 Amplitude에 직접적으로 크게 영향을 줌. A보다 C 감지가 더 어려움
- **Correctness**: Amplitude 뷰와 Trace 그래프가 동일한 A·C 이벤트 데이터를 써야 일관성 유지

**예시 (28,800 BPH, λ=52°, t_AC=0.009s):**

```
Amplitude = (3600 × 52) / (π × 28800 × 0.009)
          ≈ 187200 / 814.3
          ≈ 230°
```

---

## 7. 전체 흐름 요약

```
마이크 → 음향 신호 감지
           ↓
    A(T1)·C(T3) 타이밍 추출
           ↓
    ┌──────┴──────────────┐
    ↓                     ↓
A 이벤트만 사용        A + C 이벤트 사용
    ↓                     ↓
Rate, Beat Error       Amplitude
```

| 수치 | 질문 | 정상 범위 |
|------|------|---------|
| **Rate** | 하루에 몇 초 빠르거나 느린가? | ±5 s/day |
| **Beat Error** | tic과 tac이 대칭인가? | 0.6ms 이하 |
| **Amplitude** | 밸런스 휠이 충분히 흔들리는가? | 270~310° |

> A(T1)·C(T3) 타이밍을 정확히 잡는 것이 3가지 수치 모두의 출발점

---

## 8. QA별 도메인 이해

### QA 1: Real-Time Performance — sps와 샘플

**sps(Samples Per Second)** = 마이크로 들어오는 음향 신호를 1초에 몇 번 디지털로 측정하는가.

```
음향 신호 (연속)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ↓ 디지털 변환
| | | | | | | | | | | | | | | | | |
 ←────── 1초 안에 96,000번 측정 ──────→
```

**샘플 vs Beat — 완전히 다른 개념**

```
샘플: 마이크가 찍는 사진 한 장 (음향 신호의 순간 크기 값 하나)
beat: 시계 탈진기가 한 번 동작하는 사건 (tic 하나 또는 tac 하나)
```

28,800 BPH 시계 기준 1초 동안:

```
샘플: 96,000개  ←── 촘촘함
beat:      8개  ←── 드문드문 (28,800 ÷ 3,600)

beat 하나 안에 샘플 수 = 96,000 ÷ 8 = 12,000개
```

**beat 감지 원리**

샘플 에너지(크기²)를 계속 계산하다가 임계값을 넘는 순간을 beat 발생으로 판단합니다.

```
에너지
  |              ← 임계값
  |    ╭╮   ╭╮ ─────────────    ╭╮   ╭╮
  |   ╭╯╰───╯╰╮               ╭╯╰───╯╰╮
  |───╯        ╰───────────────╯        ╰──
  |
  └──────────────────────────────────────▶ 시간
       ↑                          ↑
    beat1 (tic)              beat2 (tac)
```

beat 하나 안에서 A와 C를 구분하는 방법:

```
beat 하나 확대:

에너지
  |
  |   ╭╮          ╭╮
  |  ╭╯╰╮        ╭╯╰╮
  |──╯  ╰────────╯  ╰──
  |
  └────────────────────▶ 시간
       ↑          ↑
     A(T1)      C(T3)
   (큰 피크)   (작은 피크)
```

- 첫 번째 큰 피크 → A(T1)
- 두 번째 피크 → C(T3) (A 이후 일정 시간 내 등장)
- B(T2)는 불규칙 → 필터링으로 제거

**sps와 타이밍 정밀도**

```
48,000 sps  → 1샘플 = 0.021ms 오차 가능
96,000 sps  → 1샘플 = 0.010ms 오차 가능
192,000 sps → 1샘플 = 0.005ms 오차 가능
```

sps가 높을수록 A·C 이벤트의 정확한 순간을 더 잘 집어낼 수 있습니다.

**RPi가 문제인 이유**

```
96,000 sps로 동시에 처리해야 할 것들:
1. 마이크에서 샘플 수집
2. Low-pass / High-pass 필터링
3. A·C 이벤트 감지
4. Rate · Beat Error · Amplitude 계산
5. GUI 11개 그래프 렌더링
→ 이걸 끊김 없이, RPi에서 해야 함
```

dropped audio block 발생 시 → A·C 이벤트 누락 → 수치 계산 불가.

---

### QA 2: Low Latency — 처리 지연

**전체 흐름**

```
시계가 tic 소리 냄
    ↓
마이크가 샘플 수집 (캡처)
    ↓  ① capture→process
A·C 이벤트 감지 + Rate·Beat Error·Amplitude 계산
    ↓  ② process→display
GUI 화면에 수치·파형 표시
```

**왜 중요한가**

28,800 BPH 시계는 125ms마다 beat가 발생합니다.

```
beat1    beat2    beat3
  |        |        |
  ←125ms→ ←125ms→
```

end-to-end 지연이 125ms를 넘으면:

```
beat1 소리 발생
    ↓ (처리 중...)
beat2 소리 이미 발생  ← 아직 beat1도 못 보여줌
    ↓ (처리 중...)
beat3 소리 이미 발생
→ backlog(처리 밀림) 쌓임 → dropped beat 발생
```

**3구간 측정**

| 구간 | 측정 내용 | 보고 항목 |
|------|---------|---------|
| ① capture→process | 샘플 수집 완료 ~ A·C 감지·계산 완료 | average + worst-case (ms) |
| ② process→display | 계산 완료 ~ GUI 화면 표시 | average + worst-case (ms) |
| ③ end-to-end | ①+② 전체 | average + worst-case (ms) |

추가 보고 항목:
- dropped audio block 수 — 처리 못 한 샘플 블록
- missed beat detection 수 — 감지 못 한 beat

**Real-Time Performance와의 충돌**

```
sps 높일수록
    → 샘플 수 증가
    → ① 처리 시간 증가
    → Low Latency 달성 어려워짐
```

sps를 얼마나 높일지 = latency를 얼마나 허용할지의 트레이드오프. Experiment 1·2 결과 후 목표 수치 확정합니다.

---

### QA 3: Correctness — 내부 일관성

**개념**

Rate, Beat Error, Amplitude는 모두 같은 A·C 이벤트에서 계산됩니다. GUI 뷰가 11개인데 각 뷰가 서로 다른 시점의 데이터를 가져다 쓰면:

```
Trace 그래프    → beat n 기준   Rate = +8.6 s/day
Rate Stability → beat n+1 기준 Rate = +7.2 s/day
Beat Error 뷰  → beat n-1 기준 Rate = +9.1 s/day
→ 같은 화면인데 수치가 다 다름 = Incorrectness
```

**Measurement Accuracy와의 차이**

| | Measurement Accuracy | Correctness |
|--|---------------------|------------|
| 질문 | 수치가 실제 시계와 맞는가? | 뷰들 사이 수치가 서로 맞는가? |
| 기준 | WeiShi No.1000 (외부 기준) | GUI 뷰들 간 내부 일관성 |
| 원인 | A·C 감지 오차 | 데이터 소스 불일치 |

```
Measurement Accuracy: 우리 Rate = +8.6, WeiShi = +8.4 → 외부 대비 0.2 차이
Correctness:          Trace Rate = +8.6, Stability Rate = +7.2 → 내부 불일치
```

**뷰(View) = 화면에 표시되는 그래프/디스플레이 하나**

구현해야 할 11개 뷰:

| 번호 | 뷰 (그래프) |
|------|-----------|
| 1 | Trace Display |
| 2 | Rate & Amplitude Stability (Vario) |
| 3 | Multi-Position Sequence Display |
| 4 | Beat-Noise Scope (Scope 1 & 2) |
| 5 | Beat Error Display & Diagnostic Trace |
| 6 | Long-Term Performance Graph |
| 7 | Escapement Analyzer & Marker-Line Display |
| 8 | Time-Frequency Spectrogram |
| 9 | Waveform Comparison Display |
| 10 | Scope Mode (Synchronized Sweep) |
| 11 | Scope Function (F0/F1/F2/F3 Filter Views) |

**해결책 = 단일 데이터 소스 구조**

```
[A·C 감지] → [계산 모듈] → [단일 데이터 저장소]
                                  ↓
                    ┌─────────────┼─────────────┐
                   뷰1           뷰2    ...     뷰11
```

이 11개 뷰가 모두 같은 저장소에서 데이터를 읽어야 Correctness 보장. "단일 데이터 소스"가 핵심 아키텍처 결정입니다.

**Ambient Noise 조건**

```
정상 신호:   beat마다 A·C 명확히 감지 → 수치 안정
노이즈 환경: A·C 감지 실패 or 잘못 감지 → 수치 튐
```

노이즈 환경에서도 수치가 안정적이어야 Correctness 만족. 필터링이 핵심입니다.

---

### QA 4: Measurement Accuracy — 실제 시계와의 일치도

**기준: WeiShi No.1000**

우리 시스템이 맞는지 틀린지를 판단할 정답지입니다.

```
같은 시계, 같은 조건에서:
WeiShi No.1000 → Rate: +8.4 s/day
우리 시스템    → Rate: +8.6 s/day
               → 오차: 0.2 s/day
```

이 오차가 얼마나 허용되는지가 Measurement Accuracy의 핵심입니다.

**오차가 생기는 원인**

A·C 이벤트 감지가 1샘플이라도 틀리면 수치가 달라집니다.

```
96,000 sps 기준 1샘플 = 0.010ms

Rate:       tic/tac 주기 계산에 직접 영향 → s/day 오차
Beat Error: t1, t2 계산에 직접 영향 → 0.6ms 기준 대비 민감
Amplitude:  t_AC = C - A 계산에 직접 영향 → 분모라서 오차 증폭
```

특히 C(T3) 이벤트가 문제입니다:

```
beat 하나:
A(T1) → 크고 선명한 피크 → 감지 쉬움
C(T3) → 작고 흐릿한 피크 → 감지 어려움
                            ↑
                    Amplitude 계산 분모 → 오차 증폭
```

**Graceful Degradation**

신호가 약하거나 노이즈가 많을 때 불안정한 수치보다 경고를 보여주는 게 낫습니다.

```
정상: A·C 감지 성공 → Rate·Beat Error·Amplitude 표시
열화: A·C 감지 실패 → "신호 약함" 경고 표시 (수치 대신)
```

잘못된 수치를 보여주면 사용자가 시계 상태를 오판하게 됩니다.

**Correctness와의 차이**

```
Measurement Accuracy: 우리 수치 vs WeiShi (외부 기준)
Correctness:          우리 뷰1 수치 vs 우리 뷰2 수치 (내부 일관성)

둘 다 나빠질 수 있음:
- Accuracy 나쁨: 모든 뷰가 일관되게 틀린 값을 보여줌
- Correctness 나쁨: 뷰마다 서로 다른 값을 보여줌
```

**Experiment 3과 연결**

```
Experiment 3: T1/T3 Detection Accuracy
목적: 우리 Rate·Beat Error·Amplitude vs WeiShi No.1000 비교
결과: 허용 오차 수치 확정
      → WeiShi 대비 Rate 오차: ___ s/day 이하
      → WeiShi 대비 Beat Error 오차: ___ ms 이하
```

---

### QA 5: Extensibility — 새 그래프 추가 비용

**왜 중요한가**

구현해야 할 그래프가 11개 + Enhanced 기능입니다. 매번 추가할 때마다 기존 코드를 대규모로 수정하면:

```
그래프 1개 추가 → 5개 파일 수정 → 기존 기능 깨질 위험
그래프 11개     → 55번 수정 → 일정 붕괴
```

**구조가 나쁜 경우**

```
[A·C 감지] ─── [계산] ─── [표시]
                              ↑
                    모든 그래프 코드가 뒤섞여 있음

새 그래프 추가 시:
- 감지 코드 수정
- 계산 코드 수정
- 기존 그래프 코드 수정
- 표시 코드 수정
→ 건드리는 파일이 많아질수록 기존 기능 깨질 위험 증가
```

**구조가 좋은 경우**

```
[A·C 감지] → [계산 모듈] → [단일 데이터 저장소]
                                  ↓
                    ┌─────────────┼──────────────┐
                   뷰1           뷰2    ...    [새 뷰 추가]
                                                  ↑
                                          이것만 새로 작성
```

새 그래프 추가 = 새 뷰 파일 1개만 작성. 기존 코드 건드릴 필요 없음.

**측정 방법**

```
새 그래프 1개 추가 시 변경된 파일 수 → 이게 Extensibility 지표
```

Final Demo에서 멘토가 직접 물어봅니다:
> "새 그래프 추가할 때 기존 코드 몇 개 파일 건드렸나요?"

목표 파일 수는 팀 합의로 결정합니다.

**아키텍처 패턴 연결**

| 패턴 | Extensibility 기여 |
|------|-----------------|
| Plugin / Observer | 새 뷰를 등록만 하면 자동으로 데이터 수신 |
| Pipeline | 감지·계산·표시를 독립 단계로 분리 |
| 레이어 분리 | 신호 수집 ↔ 처리 ↔ 계산 ↔ 표시 각각 독립 |
