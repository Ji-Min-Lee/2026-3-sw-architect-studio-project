# TimeGrapher 도메인 지식 / Domain Knowledge

> **작성일 / Date**: 2026-06-02  
> **출처 / Source**: TimeGrapher Equations_v0.docx.pdf

---

## 1. 시계 소리가 나는 원리 / How a Mechanical Watch Produces Sound

**한국어**

기계식 시계 안에는 **탈진기(escapement)** 라는 부품이 있습니다. 태엽 에너지를 일정하게 흘려보내는 역할을 합니다.

```
[태엽] → 에너지 공급 → [탈진기] → 규칙적인 진동 → 시계바늘 구동
```

탈진기가 한 번 동작할 때마다 **3개의 충격음**이 발생합니다.

**English**

Inside a mechanical watch is a component called the **escapement**. Its role is to release mainspring energy in a regulated, step-by-step manner.

```
[Mainspring] → energy supply → [Escapement] → regular vibration → drives hands
```

Each time the escapement operates, it produces **3 impact sounds**.

---

## 2. 이벤트 종류 (A/B/C = T1/T2/T3) / Event Types

**한국어**

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

**English**

Within a single beat (tic or tac), three sounds occur in sequence.

```
Sound 1 ──── Sound 2 ── Sound 3
   A             B          C
  (T1)          (T2)       (T3)
```

| Event | Alias | Cause | Used? |
|-------|-------|-------|-------|
| **A** | **T1** | Impulse pin strikes the pallet fork | ✅ Rate, Beat Error calculation |
| **B** | **T2** | Escape wheel contacts pallet stone | ❌ Irregular — not used |
| **C** | **T3** | Escape wheel locks and fork reaches banking pin | ✅ Amplitude calculation |

- **A(T1)**: Most consistent timing of the three → reference point for Rate and Beat Error
- **B(T2)**: Irregular due to friction and bounce → not used
- **C(T3)**: Time interval from A to C (t_AC) is proportional to balance wheel swing amplitude → used for Amplitude calculation

---

## 3. BPH (Beats Per Hour)

**한국어**

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

**English**

> "How many times does this watch's escapement vibrate per hour?"

Different mechanical watches are designed with different vibration rates.

| BPH | Characteristic |
|-----|---------------|
| 18,000 | Slow, older watches |
| 21,600 | Common |
| **28,800** | Modern watch standard |
| 36,000 | High-beat, luxury watches |

**Beat interval calculation for a 28,800 BPH watch:**

```
1 hour = 3,600 seconds → 28,800 beats
→ time per beat = 3600 / 28800 = 0.125 s = 125 ms

tic──tac──tic──tac──tic
 125ms 125ms 125ms 125ms
```

**Same-phase period (tic→tic):**

```
tic ──── tac ──── tic
 125ms        125ms
 └──── 250ms ────┘

T_nom = 7200 / BPH = 7200 / 28800 = 0.250 s = 250 ms
```

---

## 4. Rate (단위 / unit: s/day)

**한국어**

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

Beat Error가 존재하면 tic 간격과 tac 간격이 미묘하게 다릅니다. 섞어서 계산하면 비대칭이 Rate를 오염시키므로 **같은 위상끼리** 묶어서 계산합니다.

```
tic 위상: A0 → A2 → A4 (짝수 인덱스)
tac 위상: A1 → A3 → A5 (홀수 인덱스)
```

**Step 1** — 같은 위상 주기 측정

```
T_tic = A2 - A0   (tic → 다음 tic 까지 걸린 시간)
T_tac = A3 - A1   (tac → 다음 tac 까지 걸린 시간)
```

**Step 2** — 각 위상의 Rate 계산

```
rate_tic = 86400 × (T_nom / T_tic - 1)
rate_tac = 86400 × (T_nom / T_tac - 1)
```

- `T_nom / T_tic > 1` → 실제가 이상보다 짧음 → 시계 빠름 → Rate +
- `T_nom / T_tic < 1` → 실제가 이상보다 길음 → 시계 느림 → Rate -
- 86400을 곱하는 이유: 초 단위 비율을 **하루(86,400초) 기준 s/day로 환산**

**Step 3** — 최종 Rate

```
Rate = (rate_tic + rate_tac) / 2
```

예시 (28,800 BPH):

| 항목 | 값 |
|------|---|
| T_nom | 250ms |
| T_tic (측정값) | 249.980ms |
| T_tac (측정값) | 249.970ms |
| rate_tic | +6.912 s/day |
| rate_tac | +10.368 s/day |
| **Rate** | **+8.64 s/day** |

```
위로 올라가는 선   → Rate + (빠름)
아래로 내려가는 선 → Rate - (느림)
평평한 선          → Rate ≈ 0 (정확)
두껍고 흩어진 선   → 노이즈 많거나 Beat Error 큼
```

**English**

> "How many seconds per day does this watch gain or lose?"

Normal range: **within ±5 s/day**

When Beat Error is present, tic and tac intervals differ slightly. **Same-phase beats** are grouped together to avoid contaminating Rate with asymmetry.

```
tic phase: A0 → A2 → A4 (even indices)
tac phase: A1 → A3 → A5 (odd indices)
```

**Step 1** — Measure same-phase periods

```
T_tic = A2 - A0   (elapsed time from one tic to the next)
T_tac = A3 - A1   (elapsed time from one tac to the next)
```

**Step 2** — Calculate Rate for each phase

```
rate_tic = 86400 × (T_nom / T_tic - 1)
rate_tac = 86400 × (T_nom / T_tac - 1)
```

- `T_nom / T_tic > 1` → actual shorter than nominal → watch fast → Rate +
- `T_nom / T_tic < 1` → actual longer than nominal → watch slow → Rate −
- Multiply by 86400 to convert ratio to **s/day**

**Step 3** — Final Rate

```
Rate = (rate_tic + rate_tac) / 2
```

Example (28,800 BPH):

| Item | Value |
|------|-------|
| T_nom | 250 ms |
| T_tic (measured) | 249.980 ms |
| T_tac (measured) | 249.970 ms |
| rate_tic | +6.912 s/day |
| rate_tac | +10.368 s/day |
| **Rate** | **+8.64 s/day** |

```
Line trending upward   → Rate + (fast)
Line trending downward → Rate − (slow)
Flat line              → Rate ≈ 0 (accurate)
Thick/scattered line   → high noise or large Beat Error
```

---

## 5. Beat Error (단위 / unit: ms)

**한국어**

> "tic과 tac 간격이 얼마나 비대칭인가"

완벽한 시계라면 tic→tac 간격과 tac→tic 간격이 **정확히 같아야** 합니다.

탈진기 조정이 약간 틀어지면:

```
tic ──────────────── tac ──────── tic
        t1=125.8ms        t2=124.2ms

Beat Error = (t1 - t2) / 2
           = (125.8 - 124.2) / 2
           = 0.8ms
```

2로 나누는 이유: t1과 t2 각각이 이상적 125ms에서 **절반씩** 벗어난 것이기 때문

정상 범위: **0.6ms 이하**

| | Rate | Beat Error |
|--|------|-----------|
| 질문 | 전체적으로 빠른가 느린가? | 좌우 진동이 대칭인가? |
| 원인 | 탈진기 전체 속도 | 탈진기 좌우 균형 |
| 단위 | s/day | ms |

> Rate가 ±5 s/day로 정상이어도 Beat Error가 크면 시계 조정이 필요한 상태

**English**

> "How asymmetric are the tic and tac intervals?"

In a perfect watch the tic→tac and tac→tic intervals should be **exactly equal**.

If the escapement is slightly misadjusted:

```
tic ──────────────── tac ──────── tic
        t1=125.8ms        t2=124.2ms

Beat Error = (t1 - t2) / 2
           = (125.8 - 124.2) / 2
           = 0.8 ms
```

Divided by 2 because each of t1 and t2 deviates **half** the asymmetry from the ideal 125 ms.

Normal range: **0.6 ms or less**

| | Rate | Beat Error |
|--|------|-----------|
| Question | Is the watch overall fast or slow? | Is the left-right swing symmetric? |
| Cause | Overall escapement speed | Left-right escapement balance |
| Unit | s/day | ms |

> Even if Rate is within ±5 s/day, a large Beat Error means the watch still needs adjustment.

---

## 6. Amplitude (단위 / unit: °)

**한국어**

> "밸런스 휠이 얼마나 크게 흔들리는가"

밸런스 휠은 시계 안에서 좌우로 흔들리는 원형 부품으로, 탈진기를 구동하는 에너지원입니다.

```
태엽 에너지 충분 → 밸런스 휠이 크게 흔들림 → Amplitude 높음
태엽 에너지 부족 → 밸런스 휠이 작게 흔들림 → Amplitude 낮음
```

한 beat 안에서 A에서 C까지의 시간(t_AC)으로 계산합니다.

- 밸런스 휠이 **크게** 흔들릴수록 → t_AC **작아짐**
- 밸런스 휠이 **작게** 흔들릴수록 → t_AC **커짐**
- **t_AC와 Amplitude는 반비례**

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

예시 (28,800 BPH, λ=52°, t_AC=0.009s):

```
Amplitude = (3600 × 52) / (π × 28800 × 0.009) ≈ 230°
```

**English**

> "How far does the balance wheel swing?"

The balance wheel is a circular component inside the watch that swings back and forth — the energy source that drives the escapement.

```
Sufficient mainspring energy → balance wheel swings wide  → Amplitude high
Insufficient energy          → balance wheel swings narrow → Amplitude low
```

Amplitude is calculated from the time interval t_AC between A and C within one beat.

- Balance wheel swings **wider** → t_AC **smaller**
- Balance wheel swings **narrower** → t_AC **larger**
- **t_AC and Amplitude are inversely proportional**

```
Amplitude = (3600 × λ) / (π × BPH × t_AC)

λ     = lift angle (degrees) — varies by movement, typically 52°
BPH   = beats per hour
t_AC  = time from A to C (seconds)
```

Normal range:

```
270–310° → strong (mainspring sufficient)
220–250° → acceptable
Below    → mainspring low or component wear
```

Example (28,800 BPH, λ=52°, t_AC=0.009 s):

```
Amplitude = (3600 × 52) / (π × 28800 × 0.009) ≈ 230°
```

---

## 7. 전체 흐름 요약 / End-to-End Flow Summary

**한국어**

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

**English**

```
Microphone → acoustic signal detection
                     ↓
          A(T1) · C(T3) timing extraction
                     ↓
         ┌───────────┴──────────────┐
         ↓                          ↓
  A events only              A + C events
         ↓                          ↓
  Rate, Beat Error            Amplitude
```

| Metric | Question | Normal Range |
|--------|----------|-------------|
| **Rate** | How many seconds per day does the watch gain or lose? | ±5 s/day |
| **Beat Error** | Are tic and tac symmetric? | ≤ 0.6 ms |
| **Amplitude** | Is the balance wheel swinging sufficiently? | 270–310° |

---

## 8. QA별 도메인 이해 / Domain Understanding per QA

### QA 1: Real-Time Performance — sps와 샘플 / sps and Samples

**한국어**

**sps(Samples Per Second)** = 마이크로 들어오는 음향 신호를 1초에 몇 번 디지털로 측정하는가.

28,800 BPH 시계 기준 1초 동안:

```
샘플: 96,000개  ←── 촘촘함
beat:      8개  ←── 드문드문 (28,800 ÷ 3,600)

beat 하나 안에 샘플 수 = 96,000 ÷ 8 = 12,000개
```

beat 하나 안에서 A와 C를 구분하는 방법:

```
에너지
  |
  |   ╭╮          ╭╮
  |  ╭╯╰╮        ╭╯╰╮
  |──╯  ╰────────╯  ╰──
       ↑          ↑
     A(T1)      C(T3)
   (큰 피크)   (작은 피크)
```

```
48,000 sps  → 1샘플 = 0.021ms 오차 가능
96,000 sps  → 1샘플 = 0.010ms 오차 가능
192,000 sps → 1샘플 = 0.005ms 오차 가능
```

dropped audio block 발생 시 → A·C 이벤트 누락 → 수치 계산 불가.

**English**

**sps (Samples Per Second)** = how many times per second the acoustic signal from the microphone is digitally measured.

In one second for a 28,800 BPH watch:

```
Samples: 96,000  ←── dense
Beats:        8  ←── sparse (28,800 ÷ 3,600)

Samples per beat = 96,000 ÷ 8 = 12,000
```

How A and C are distinguished within a single beat:

```
Energy
  |
  |   ╭╮          ╭╮
  |  ╭╯╰╮        ╭╯╰╮
  |──╯  ╰────────╯  ╰──
       ↑          ↑
     A(T1)      C(T3)
  (large peak) (small peak)
```

```
48,000 sps  → 1 sample = up to 0.021 ms error
96,000 sps  → 1 sample = up to 0.010 ms error
192,000 sps → 1 sample = up to 0.005 ms error
```

Dropped audio blocks → missed A·C events → metrics cannot be computed.

---

### QA 2: Low Latency — 처리 지연 / Processing Delay

**한국어**

28,800 BPH 시계는 125ms마다 beat가 발생합니다. end-to-end 지연이 125ms를 넘으면 backlog(처리 밀림) 쌓임 → dropped beat 발생.

| 구간 | 측정 내용 | 보고 항목 |
|------|---------|---------|
| ① capture→process | 샘플 수집 완료 ~ A·C 감지·계산 완료 | average + worst-case (ms) |
| ② process→display | 계산 완료 ~ GUI 화면 표시 | average + worst-case (ms) |
| ③ end-to-end | ①+② 전체 | average + worst-case (ms) |

**English**

A 28,800 BPH watch produces a beat every 125 ms. If end-to-end latency exceeds 125 ms, backlog accumulates → dropped beats.

| Segment | What is measured | Reported metrics |
|---------|-----------------|-----------------|
| ① capture→process | End of sample capture to A·C detection and calculation | average + worst-case (ms) |
| ② process→display | Calculation complete to GUI display | average + worst-case (ms) |
| ③ end-to-end | Total of ①+② | average + worst-case (ms) |

---

### QA 3: Correctness — 내부 일관성 / Internal Consistency

**한국어**

GUI 뷰가 11개인데 각 뷰가 서로 다른 시점의 데이터를 가져다 쓰면:

```
Trace 그래프    → beat n 기준   Rate = +8.6 s/day
Rate Stability → beat n+1 기준 Rate = +7.2 s/day
→ 같은 화면인데 수치가 다 다름 = Incorrectness
```

**해결책 = 단일 데이터 소스 구조**: 11개 뷰 모두 같은 저장소에서 데이터를 읽어야 Correctness 보장.

**English**

With 11 GUI views, if each view pulls data from different moments:

```
Trace graph    → based on beat n:   Rate = +8.6 s/day
Rate Stability → based on beat n+1: Rate = +7.2 s/day
→ same screen, different numbers = Incorrectness
```

**Solution = single data source**: all 11 views must read from the same store to guarantee Correctness.

---

### QA 4: Measurement Accuracy — 실제 시계와의 일치도 / Agreement with the Real Watch

**한국어**

**기준: WeiShi No.1000** — 우리 시스템이 맞는지 틀린지를 판단할 정답지.

```
WeiShi No.1000 → Rate: +8.4 s/day
우리 시스템    → Rate: +8.6 s/day → 오차: 0.2 s/day
```

신호가 약하거나 노이즈가 많을 때 불안정한 수치보다 경고를 보여주는 것이 낫습니다 (Graceful Degradation).

**English**

**Reference: WeiShi No.1000** — the ground truth used to judge whether our system is correct.

```
WeiShi No.1000 → Rate: +8.4 s/day
Our system     → Rate: +8.6 s/day → error: 0.2 s/day
```

When signal is weak or noisy, showing a warning is better than displaying unstable values (Graceful Degradation).

---

### QA 5: Extensibility — 새 그래프 추가 비용 / Cost of Adding a New Graph

**한국어**

구현해야 할 그래프가 11개 + Enhanced 기능입니다. 구조가 나쁘면 매 추가마다 일정 리스크 발생.

```
[A·C 감지] → [계산 모듈] → [단일 데이터 저장소]
                                  ↓
                    ┌─────────────┼──────────────┐
                   뷰1           뷰2    ...    [새 뷰 추가]
                                                  ↑
                                          이것만 새로 작성
```

새 그래프 추가 = 새 뷰 파일 1개만 작성. 기존 코드 건드릴 필요 없음.

Final Demo에서 멘토가 직접 물어봅니다:
> "새 그래프 추가할 때 기존 코드 몇 개 파일 건드렸나요?"

**English**

There are 11 graphs to implement plus Enhanced features. A poor structure creates schedule risk with every addition.

```
[A·C detection] → [calculation module] → [single data store]
                                                ↓
                                ┌───────────────┼────────────────┐
                              view1           view2    ...   [new view]
                                                                  ↑
                                                     only this needs to be written
```

Adding a new graph = write one new view file. No changes to existing code.

At the Final Demo, the mentor will ask directly:
> "When you added the new graph, how many existing code files did you have to touch?"
