# TimeGrapher Domain Knowledge

> **Date**: 2026-06-02  
> **Source**: TimeGrapher Equations_v0.docx.pdf

---

## 1. How a Mechanical Watch Produces Sound

Inside a mechanical watch is a component called the **escapement**. Its role is to release mainspring energy in a regulated, step-by-step manner.

```
[Mainspring] → energy supply → [Escapement] → regular vibration → drives hands
```

Each time the escapement operates, it produces **3 impact sounds**.

---

## 2. Event Types (A/B/C = T1/T2/T3)

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

## 4. Rate (unit: s/day)

> "How many seconds per day does this watch gain or lose?"

### Concept

```
Ideal A events (28,800 BPH):
A0        A1        A2        A3
|----125ms----|----125ms----|----125ms----|

Actual measurement (watch running fast):
A0        A1        A2        A3
|---124ms----|---124ms----|---124ms----|
              (arriving slightly early each time = watch is fast)
```

Normal range: **within ±5 s/day**

### Calculation — Why tic and tac are separated

When Beat Error is present, tic intervals and tac intervals differ slightly. Mixing them contaminates the Rate with asymmetry, so **same-phase beats** are grouped together.

```
tic phase: A0 → A2 → A4 (even indices)
tac phase: A1 → A3 → A5 (odd indices)
```

### Step 1 — Measure same-phase periods

```
T_tic = A2 - A0   (elapsed time from one tic to the next)
T_tac = A3 - A1   (elapsed time from one tac to the next)
```

### Step 2 — Calculate Rate for each phase

```
rate_tic = 86400 × (T_nom / T_tic - 1)
rate_tac = 86400 × (T_nom / T_tac - 1)
```

- `T_nom / T_tic > 1` → actual interval shorter than nominal → watch is fast → Rate +
- `T_nom / T_tic < 1` → actual interval longer than nominal → watch is slow → Rate −
- Multiply by 86400 to convert the ratio into **s/day (86,400 seconds per day)**

### Step 3 — Final Rate

```
Rate = (rate_tic + rate_tac) / 2
```

**Example (28,800 BPH):**

| Item | Value |
|------|-------|
| T_nom | 250 ms |
| T_tic (measured) | 249.980 ms |
| T_tac (measured) | 249.970 ms |
| rate_tic | +6.912 s/day |
| rate_tac | +10.368 s/day |
| **Rate** | **+8.64 s/day** |

### Graphical interpretation

```
Line trending upward   → Rate + (fast)
Line trending downward → Rate − (slow)
Flat line              → Rate ≈ 0 (accurate)
Thick/scattered line   → high noise or large Beat Error
```

---

## 5. Beat Error (unit: ms)

> "How asymmetric are the tic and tac intervals?"

### A perfect watch

```
tic ────────── tac ────────── tic
     125ms          125ms
```

The tic→tac and tac→tic intervals should be **exactly equal**.

### A watch with Beat Error

If the escapement is slightly misadjusted:

```
tic ──────────────── tac ──────── tic
        t1=125.8ms        t2=124.2ms
```

```
Beat Error = (t1 - t2) / 2
           = (125.8 - 124.2) / 2
           = 0.8 ms
```

Divided by 2 because each of t1 and t2 deviates **half** the asymmetry from the ideal 125 ms.

- t1 = t2 → Beat Error = 0 (perfect symmetry)
- t1 ≠ t2 → escapement adjustment required

Normal range: **0.6 ms or less**

### Comparison with Rate

| | Rate | Beat Error |
|--|------|-----------|
| Question | Is the watch overall fast or slow? | Is the left-right swing symmetric? |
| Cause | Overall escapement speed | Left-right escapement balance |
| Unit | s/day | ms |

> Even if Rate is within ±5 s/day, a large Beat Error means the watch still needs adjustment.

### Connection to QA

- **Measurement Accuracy**: t1 and t2 are derived from A event timing — even one misdetected A event skews Beat Error
- **Correctness**: Beat Error view and Trace graph must use the same A event data to remain consistent

---

## 6. Amplitude (unit: °)

> "How far does the balance wheel swing?"

### What is the balance wheel?

A circular component inside the watch that swings back and forth. Its oscillation is the energy source that drives the escapement.

```
Sufficient mainspring energy → balance wheel swings wide  → Amplitude high
Insufficient energy          → balance wheel swings narrow → Amplitude low
```

### Relationship between A and C events

Within one beat, Amplitude is calculated from the time interval t_AC between A and C.

```
A ────────────── C
(balance impulse)  (escape wheel locks)
└──── t_AC ─────┘
```

- Balance wheel swings **wider** → reaches C from A **faster** → t_AC **smaller**
- Balance wheel swings **narrower** → reaches C from A **slower** → t_AC **larger**

**t_AC and Amplitude are inversely proportional.**

### Formula

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

### Connection to QA

- **Measurement Accuracy**: t_AC is in the denominator, so C event timing errors have a directly amplified impact on Amplitude. C is harder to detect than A.
- **Correctness**: Amplitude view and Trace graph must use the same A and C event data to remain consistent.

**Example (28,800 BPH, λ=52°, t_AC=0.009 s):**

```
Amplitude = (3600 × 52) / (π × 28800 × 0.009)
          ≈ 187200 / 814.3
          ≈ 230°
```

---

## 7. End-to-End Flow Summary

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

> Precisely detecting A(T1) and C(T3) timing is the starting point for all three metrics.

---

## 8. Domain Understanding per QA

### QA 1: Real-Time Performance — sps and Samples

**sps (Samples Per Second)** = how many times per second the acoustic signal from the microphone is digitally measured.

```
Acoustic signal (continuous)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ↓ digital conversion
| | | | | | | | | | | | | | | | | |
 ←──── 96,000 measurements per second ────→
```

**Sample vs Beat — completely different concepts**

```
Sample: one snapshot taken by the microphone (a single instantaneous amplitude value)
Beat:   one operation of the watch escapement (one tic or one tac)
```

In one second for a 28,800 BPH watch:

```
Samples: 96,000  ←── dense
Beats:        8  ←── sparse (28,800 ÷ 3,600)

Samples per beat = 96,000 ÷ 8 = 12,000
```

**Beat detection principle**

The system continuously computes sample energy (amplitude²) and marks a beat whenever the energy exceeds a threshold.

```
Energy
  |              ← threshold
  |    ╭╮   ╭╮ ─────────────    ╭╮   ╭╮
  |   ╭╯╰───╯╰╮               ╭╯╰───╯╰╮
  |───╯        ╰───────────────╯        ╰──
  |
  └──────────────────────────────────────▶ time
       ↑                          ↑
    beat1 (tic)              beat2 (tac)
```

How A and C are distinguished within a single beat:

```
Zoomed in on one beat:

Energy
  |
  |   ╭╮          ╭╮
  |  ╭╯╰╮        ╭╯╰╮
  |──╯  ╰────────╯  ╰──
  |
  └────────────────────▶ time
       ↑          ↑
     A(T1)      C(T3)
  (large peak) (small peak)
```

- First large peak → A(T1)
- Second peak → C(T3) (appears within a fixed window after A)
- B(T2) is irregular → removed by filtering

**sps and timing precision**

```
48,000 sps  → 1 sample = up to 0.021 ms error
96,000 sps  → 1 sample = up to 0.010 ms error
192,000 sps → 1 sample = up to 0.005 ms error
```

Higher sps means finer resolution for locating the exact moment of A and C events.

**Why RPi is the challenge**

```
Everything that must run simultaneously at 96,000 sps:
1. Sample acquisition from microphone
2. Low-pass / high-pass filtering
3. A·C event detection
4. Rate · Beat Error · Amplitude calculation
5. Rendering 11 GUI graphs
→ All of this, uninterrupted, on an RPi
```

Dropped audio blocks → missed A·C events → metrics cannot be computed.

---

### QA 2: Low Latency — Processing Delay

**End-to-end pipeline**

```
Watch produces tic sound
    ↓
Microphone captures samples
    ↓  ① capture→process
A·C event detection + Rate·Beat Error·Amplitude calculation
    ↓  ② process→display
Metrics and waveform displayed on GUI
```

**Why it matters**

A 28,800 BPH watch produces a beat every 125 ms.

```
beat1    beat2    beat3
  |        |        |
  ←125ms→ ←125ms→
```

If end-to-end latency exceeds 125 ms:

```
beat1 sound occurs
    ↓ (processing...)
beat2 already occurred  ← beat1 not yet displayed
    ↓ (processing...)
beat3 already occurred
→ backlog accumulates → dropped beats
```

**Three measurement segments**

| Segment | What is measured | Reported metrics |
|---------|-----------------|-----------------|
| ① capture→process | End of sample capture to completion of A·C detection and calculation | average + worst-case (ms) |
| ② process→display | Calculation complete to display on GUI | average + worst-case (ms) |
| ③ end-to-end | Total of ①+② | average + worst-case (ms) |

Additional reported metrics:
- Dropped audio block count — sample blocks that were not processed
- Missed beat detection count — beats that were not detected

**Conflict with Real-Time Performance**

```
Higher sps
    → more samples
    → longer ① processing time
    → harder to achieve Low Latency
```

How high to set sps = how much latency to tolerate. Target values are finalized after Experiment 1 and 2 results.

---

### QA 3: Correctness — Internal Consistency

**Concept**

Rate, Beat Error, and Amplitude are all computed from the same A·C events. With 11 GUI views, if each view pulls data from different moments in time:

```
Trace graph      → based on beat n:   Rate = +8.6 s/day
Rate Stability   → based on beat n+1: Rate = +7.2 s/day
Beat Error view  → based on beat n-1: Rate = +9.1 s/day
→ same screen, different numbers = Incorrectness
```

**Difference from Measurement Accuracy**

| | Measurement Accuracy | Correctness |
|--|---------------------|------------|
| Question | Do our numbers match the real watch? | Do our views agree with each other? |
| Reference | WeiShi No.1000 (external standard) | Internal consistency across GUI views |
| Cause | A·C detection error | Data source mismatch |

```
Measurement Accuracy: our Rate = +8.6, WeiShi = +8.4 → 0.2 difference vs. external
Correctness:          Trace Rate = +8.6, Stability Rate = +7.2 → internal mismatch
```

**View = one graph or display shown on screen**

The 11 views to implement:

| # | View (Graph) |
|---|-------------|
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

**Solution = single data source architecture**

```
[A·C detection] → [calculation module] → [single data store]
                                                ↓
                                ┌───────────────┼───────────────┐
                              view1           view2    ...    view11
```

All 11 views must read from the same store to guarantee Correctness. "Single data source" is the key architectural decision.

**Ambient Noise conditions**

```
Normal signal:    A·C detected clearly every beat → stable metrics
Noisy environment: A·C detection fails or misfires → metrics spike
```

Metrics must remain stable even under noise. Filtering is essential.

---

### QA 4: Measurement Accuracy — Agreement with the Real Watch

**Reference: WeiShi No.1000**

This is the ground truth used to judge whether our system is correct.

```
Same watch, same conditions:
WeiShi No.1000 → Rate: +8.4 s/day
Our system     → Rate: +8.6 s/day
               → error: 0.2 s/day
```

How much of this error is acceptable is the core question for Measurement Accuracy.

**Sources of error**

Even one misdetected sample for an A·C event changes the result.

```
At 96,000 sps, 1 sample = 0.010 ms

Rate:       directly affects tic/tac period calculation → s/day error
Beat Error: directly affects t1, t2 calculation → sensitive relative to 0.6 ms threshold
Amplitude:  directly affects t_AC = C − A → error amplified because it is in the denominator
```

C(T3) events are especially problematic:

```
Within one beat:
A(T1) → large, sharp peak → easy to detect
C(T3) → small, faint peak → hard to detect
                             ↑
                   denominator in Amplitude → error amplified
```

**Graceful Degradation**

When the signal is weak or noisy, showing a warning is better than displaying unstable values.

```
Normal:     A·C detection succeeds → display Rate·Beat Error·Amplitude
Degraded:   A·C detection fails    → display "weak signal" warning instead of values
```

Showing incorrect values causes the user to misjudge the watch condition.

**Difference from Correctness**

```
Measurement Accuracy: our values vs. WeiShi (external reference)
Correctness:          our view1 values vs. our view2 values (internal consistency)

Both can be bad independently:
- Poor Accuracy:    all views consistently show a wrong value
- Poor Correctness: each view shows a different value
```

**Connection to Experiment 3**

```
Experiment 3: T1/T3 Detection Accuracy
Goal: compare our Rate·Beat Error·Amplitude against WeiShi No.1000
Outcome: establish acceptable error thresholds
         → Rate error vs. WeiShi:       ___ s/day or less
         → Beat Error error vs. WeiShi: ___ ms or less
```

---

### QA 5: Extensibility — Cost of Adding a New Graph

**Why it matters**

There are 11 graphs to implement plus Enhanced features. If adding each one requires large-scale modifications to existing code:

```
Add 1 graph → modify 5 files → risk of breaking existing functionality
11 graphs   → 55 modifications → schedule collapse
```

**Poor structure**

```
[A·C detection] ─── [calculation] ─── [display]
                                           ↑
                               all graph code tangled together

When adding a new graph:
- modify detection code
- modify calculation code
- modify existing graph code
- modify display code
→ more files touched = higher risk of breaking existing functionality
```

**Good structure**

```
[A·C detection] → [calculation module] → [single data store]
                                                ↓
                                ┌───────────────┼────────────────┐
                              view1           view2    ...   [new view]
                                                                  ↑
                                                     only this needs to be written
```

Adding a new graph = write one new view file. No changes to existing code.

**How to measure**

```
Number of files changed when adding one new graph → this is the Extensibility metric
```

At the Final Demo, the mentor will ask directly:
> "When you added the new graph, how many existing code files did you have to touch?"

The target file count is decided by team consensus.

**Related architectural patterns**

| Pattern | Extensibility benefit |
|---------|----------------------|
| Plugin / Observer | New views automatically receive data by registration alone |
| Pipeline | Detection, calculation, and display as independent stages |
| Layer separation | Signal acquisition ↔ processing ↔ calculation ↔ display each independent |
