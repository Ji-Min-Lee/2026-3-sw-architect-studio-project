# Phase 0: Required Document Reading Summary

> Week 0 deliverable — domain, requirements, and formula comprehension recorded prior to project start

---

## Documents Read

| File | Scope | Status |
|------|-------|--------|
| `Time Grapher Project Plan (Draft).pdf` | Full (10 pp.) | ✅ Done |
| `TimeGrapher Equations_v0.docx.pdf` | Full (11 pp.) | ✅ Done |
| `Witschi-Training-Course.pdf` | pp. 14–19 | ✅ Done |

---

## 1. Domain Understanding — Swiss Lever Escapement Acoustic Events

Three acoustic events occur in sequence per beat of a mechanical watch.

```
[T1 (A)]  Impulse pin → Pallet fork        ← most precise & repeatable → Rate, Beat Error calculation
[T2 (B)]  Escape wheel tooth → Pallet stone ← irregular → not used for measurement
[T3 (C)]  Escape wheel locks + Fork → Banking pin ← paired with T1 → Amplitude calculation
```

> **Key insight**: The software only needs to detect and use **T1(A) and T3(C)**.
> T2(B) is excluded from calculation due to its irregular signal.

### Measurement Normal Ranges

| Measurement | Unit | Normal (OK) | Warning | Problem |
|-------------|------|-------------|---------|---------|
| Rate | s/d | -5 ~ +15 | exceeds ±15 | exceeds ±30 |
| Amplitude (H, horizontal) | ° | 250 ~ 330 | 220 ~ 250 | below 200 |
| Amplitude (V, vertical) | ° | 250 ~ 270 | 220 ~ 250 | below 200 |
| Beat Error | ms | 0.0 ~ 0.5 | 0.5 ~ 3.0 | exceeds 3.0 |

---

## 2. Core Calculation Formulas (TimeGrapher Equations)

### 2-1. Rate Error — Instantaneous Error Graph

Instantaneous error for beat n:

```
E_n = T_measured - (T_start + n × I_target)
```

- `T_start`: timestamp of the first beat (anchor)
- `I_target`: ideal beat interval = 3600 / BPH (seconds)
- `E_n`: cumulative timing error → plotted on Y-axis

Plot Y coordinate (with modulo to prevent going off-screen):

```
Y = E_n (mod Plot Height)
```

**The more the trace tilts as a straight line, the larger the rate error.** Closer to horizontal = more accurate watch.

#### Rate Value Calculation (tic/tac separated)

From A event stream A₀, A₁, A₂, A₃, A₄ …:

```
T_tic = A_(2k+2) - A_(2k)       # tic-to-tic period
T_tac = A_(2k+3) - A_(2k+1)     # tac-to-tac period
T_nom,same-phase = 7200 / BPH   # ideal interval between same-phase events

rate_tic = 86400 × (T_nom,same-phase / T_tic - 1)
rate_tac = 86400 × (T_nom,same-phase / T_tac - 1)
Rate     = (rate_tic + rate_tac) / 2              # s/d
```

> Reason for separating tic and tac: when beat error exists, the two phases are asymmetric,
> so this approach is less contaminated than a simple average.

Sample index-based implementation (fs = sample rate):

```
T_tic = (n_(2k+2) - n_(2k)) / fs
T_tac = (n_(2k+3) - n_(2k+1)) / fs
```

#### Immediate Plot Pseudocode

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

Measure half-period asymmetry using three consecutive A events A₀, A₁, A₂:

```
t1 = A₁ - A₀    # first half-beat
t2 = A₂ - A₁    # second half-beat

BE = (t1 - t2) / 2    # signed; displayed as |BE|
```

- 0 ms = perfectly symmetric (ideal)
- 0.5 ms or less = acceptable
- Sample index-based: `BE = ((n₁-n₀) - (n₂-n₁)) / (2 × fs)`

> Rate measures "how fast or slow the watch is"; Beat Error measures "how symmetric the two half-beats are" —
> the two values are independent and must be displayed simultaneously.

---

### 2-3. Amplitude

Estimate the balance wheel's angular swing from the **A→C interval**:

```
Amp = (3600 × λ) / (π × n × t_AC)

  λ      = lift angle (°) — user setting, default 52°
  n      = BPH (beats per hour)
  t_AC   = interval from A event onset to C event peak within the same beat packet (seconds)
```

Sample index-based:

```
t_AC = (c_idx - a_idx) / fs
Amp  = (3600 × λ × fs) / (π × n × (c_idx - a_idx))
```

**Important**: A and C must be paired within the same beat packet. Pairing A and C from different beats causes calculation errors.

**Practical intuition**: smaller t_AC = larger Amp (strong swing = fast return = short A-C interval).

#### 28,800 bph Example

| Input | Calculation |
|-------|-------------|
| λ = 52°, n = 28,800, t_AC = 0.009 s | Amp = (3600 × 52) / (π × 28800 × 0.009) ≈ 230° |

---

## 3. Graph Interpretation — Trace Pattern Catalog (Witschi pp.14-15)

### Normal Patterns

| Pattern | Meaning |
|---------|---------|
| Dense dots with a slight linear tilt | Normal (Rate within ±15 s/d) |
| Two lines nearly parallel with narrow gap | Beat error acceptable |

### Abnormal Patterns and Causes

| Pattern | Cause | Action |
|---------|-------|--------|
| Wide gap between two lines (beat error ~3 ms) | Excessive beat error | Adjust beat error, then re-adjust rate |
| Both lines steeply tilted up/down | Rate fast/slow | Adjust rate |
| Large rate difference by position (H +30, V -40) | Balance eccentricity, magnetism, wear | Adjust or replace |
| Large waveform variation at regular intervals | Gear train defect | Inspect gear train |
| Irregular scatter pattern, low amplitude | General malfunction | Overhaul |
| Two lines occasionally split (knocking) | Excessive amplitude (>330°) → double tic-tac | Replace mainspring/pallet stone |

---

## 4. Scope Waveform Interpretation — Error Detection (Witschi pp.16-19)

On the Scope screen, physical defects are identified by **abnormal peak positions and counts** in the beat waveform.

### Waveform Pattern → Physical Cause Mapping

| Scope Waveform Characteristic | Physical Cause |
|-------------------------------|----------------|
| First peak small, second peak large | Escapement fitting too weak |
| Both peaks strong and closely spaced | Escapement fitting too strong |
| Second peak shifted forward (strong unlocking) | Unlocking too strong |
| First peak deflected downward (↓ arrow) | Additional friction |
| Abnormal peaks on both beats (↓ arrows on both) | Dart touching the roller |
| Abnormal peak after second peak (↓ arrow) | Not enough clearance (horns ↔ impulse pin) |
| Peak intensity alternates between beats | Weak amplitude |
| Irregular spacing between peaks | Too much axial end shake (pivot) |
| Knocking peaks on both beats | Fork horn touches impulse pin |
| Noise tail on waveform (↑↓ arrows) | Rough pivot / seizure |
| Extra peak at end of beat (↓ arrow) | Grazing balance wheel / hair spring |
| Only one peak per beat | Escape wheel tooth penetrates impulse plane |

> **Implementation perspective**: The Scope Function (F0/F1/F2/F3) is intended to visualize these patterns
> at each filter processing stage so the user can diagnose the cause.

---

## 5. Current Baseline GUI Structure Summary

```
┌─────────────────────────────────────────────────┐
│  Measurement Summary Bar                         │
│  (RATE / AMPLITUDE / BEAT ERROR / BEAT display)  │
├─────────────────────────────────────────────────┤
│  Tabbed Graph Panel                              │
│  ┌──────────────┬──────────────┐                 │
│  │ Rate/Scope   │ Sound Print  │  ← currently 2  │
│  │   Tab        │    Tab       │  ← add here     │
│  └──────────────┴──────────────┘                 │
├─────────────────────────────────────────────────┤
│  Control Panel                                   │
│  Run Params │ Watch Params │ Sim Params │ Misc   │
└─────────────────────────────────────────────────┘
```

**Extension point**: Implement all 11 graphs by adding new tabs to the Tabbed Graph Panel.
A structure where extension requires only adding tabs — no major changes to existing code — is the core of the Extensibility QA.

### Required Graph List (priority order)

| Priority | Graph | Data Used |
|----------|-------|-----------|
| 1 | Trace Display | Rate deviation + Amplitude continuous recording |
| 2 | Rate & Amplitude Stability (Vario) | Min/Max/Avg/σ statistics |
| 3 | Beat Error Display & Diagnostic Trace | Rate/Amplitude/Beat Error values |
| 4 | Beat-Noise Scope (Scope 1 & 2) | Individual beat waveform + Σ averaging |
| 5 | Multi-Position Sequence Display | Up to 10 position comparison |
| 6 | Long-Term Performance Graph | Long-term trend |
| 7 | Escapement Analyzer & Marker-Line Display | A/C event markers |
| 8 | Time-Frequency Spectrogram | Time-frequency energy distribution |
| 9 | Waveform Comparison Display | Aligned continuous beat comparison |
| 10 | Scope Mode (Synchronized Sweep) | Oscilloscope style |
| 11 | Scope Function (F0/F1/F2/F3) | 4 filter processing views |

---

## 6. Architecture Goals and Grading Alignment

| QA | Target Value | Demo Evidence |
|----|-------------|---------------|
| Real-Time Performance | 96,000 sps (minimum 48,000 sps) | FPS + sps values displayed on screen |
| Low Latency | Minimize end-to-end | capture→process, process→display, total in ms |
| Correctness | Match WeiShi 1000 reference | Simultaneous comparison measurement with same watch |
| Measurement Accuracy | Precise T1/T3 onset detection | Beat event false detection rate |
| Extensibility | New graph = minimum file changes | Module view-based impact analysis explanation |

---

## 7. Key Cautions (Implementation Pitfalls)

1. **AGC must be disabled** — Verify in Raspberry Pi AlsaMixer. If enabled, all measurements are corrupted by signal distortion.
2. **Lift Angle is a user setting** — 52° is common but not universal. Do not hardcode.
3. **A-C pair must be within the same beat** — Pairing A from one beat with C from another causes Amplitude calculation errors.
4. **T_start must be reset on every restart** — Re-anchor to a new T_start on screen reset/restart for the trace to render correctly.
5. **Rate calculation requires tic/tac separation** — Computing from a simple average interval contaminates the rate value when beat error exists.
