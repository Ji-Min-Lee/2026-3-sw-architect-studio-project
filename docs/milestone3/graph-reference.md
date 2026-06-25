# TimeGrapher — Graph Reference

**Team**: Blue Sky (Team 3) | **Platform**: Raspberry Pi 5 + Qt/C++

> Place screenshot images in `docs/milestone3/graphs/` with the filenames referenced below.

---

## Status Bar

All graph tabs share the same status bar at the top:

```
POS <position> | RATE <s/d> | AMP <°> | ERR <ms> | BPH <value>   [DIAGNOSIS badge]
```

- **POS** — current watch position (CH, CB, 9H, 6H, 3H, 12H, CU, CD)
- **RATE** — rate deviation in seconds per day (green = within tolerance, red = out of range)
- **AMP** — balance wheel amplitude in degrees
- **ERR** — beat error in milliseconds
- **BPH** — beats per hour (purple)
- **DIAGNOSIS badge** — green = Excellent, amber = Good, red = Needs Service

---

## 1. Rate / Scope

![Rate/Scope](graphs/01-rate-scope.png)

Two-panel display. The primary measurement view.

**Top panel — Timing offset scatter plot**

- X-axis: Beat count (cumulative)
- Y-axis: Timing offset in milliseconds
- **Red dots** — tic (A event) timing offset per beat
- **Blue dots** — toc (C event) timing offset per beat
- **Green line** — 20-beat rolling average trend
- Upper-left box: session statistics — mean, σ (standard deviation), n (total beats)
- A flat line near zero means the watch is on-rate and consistent

**Bottom panel — Scope view**

- X-axis: Time in seconds (real-time scrolling)
- Y-axis: Amplitude (signal strength)
- **Blue line** — rectified (absolute value) acoustic signal — the raw sound envelope
- **Red line** — detection trigger threshold
- **Green dashed lines** — A events (tic)
- **Red dashed lines** — C events (tac)
- Arrows on the plot show measured values directly: A→C interval (ms), derived amplitude (°), and full beat period (ms)
- Every number in the status bar is computed from the intervals visible here

---

## 2. Sound Print

![Sound Print](graphs/02-sound-print.png)

Acoustic fingerprint view — plots every detected beat event as a dot.

- **Green / Yellow / Orange dots** — A events (tic), colored by signal strength: strong / medium / weak
- **Blue dots** — C events (tac), same three strength levels
- **Green horizontal line** — A event boundary (expected position of tic)
- **Blue horizontal line** — C event boundary (half-period mark)
- **Pink background band** — normal operating zone

**How to read it:**
- Tightly clustered dots = consistent beats, good signal
- Vertically scattered dots = timing inconsistency beat to beat
- Missing color levels = signal too weak to detect that event reliably (move microphone closer)

---

## 3. Trace

![Trace](graphs/03-trace.png)

Continuous time-series of Rate and Amplitude. Shows how both metrics evolve over a session.

**Top panel — Rate Error**

- X-axis: Time in seconds
- Y-axis: Rate error in seconds per day
- **Blue line** — rolling average rate
- Zero = perfect rate. Positive = running fast. Negative = running slow.
- Header text shows overall assessment ("Rate and amplitude within normal limits")

**Bottom panel — Amplitude**

- X-axis: Time in seconds
- Y-axis: Amplitude in degrees
- **Green line** — current amplitude
- **Color bands** (background):
  - Green zone (270–310°): Strong — balance wheel swinging well
  - Amber zone (220–270°): Acceptable — borderline, monitor closely
  - Red zone (< 220°): Needs service — mainspring running down or excessive friction

---

## 4. Vario

![Vario](graphs/04-vario.png)

Statistical summary of the entire session. Updates live.

**Header** — elapsed session time (e.g., 3:03)

**Rate section (top half)**

- Displays: Min / Max / Now / σ (standard deviation)
- **Blue arrows** — Min and Max positions on the horizontal scale
- **Red arrow** — current (Now) value
- **Green background** — all values within acceptable tolerance
- Checkmark (✓) next to the label confirms within-tolerance status

**Amplitude section (bottom half)**

- Same layout: Min / Max / Now / σ plotted as arrows on the scale
- Green background and checkmark = amplitude stable and acceptable

**How to read it:**
- Narrow gap between Min and Max = consistent watch
- Wide gap = inconsistent; investigate rate drift or amplitude drop
- σ near zero = very stable measurement-to-measurement

---

## 5. Sequence

![Sequence](graphs/05-sequence.png)

Multi-position measurement table with embedded Radar Chart.

**Left — Position table**

Rows:
- **CH** — Crown Horizontal (face up)
- **CB** — Crown Back (face down)
- **9H / 6H / 3H / 12H** — four vertical positions (crown pointing left/down/right/up)
- **CU(R) / CU(L)** — Crown Up right/left
- **CD(R) / CD(L)** — Crown Down right/left

Columns: Rate (s/d) | Beat Error (ms) | Amplitude (°)

Summary rows at the bottom:
- **X (mean)** — average across all measured positions
- **D (max−min)** — positional spread; large D means the watch behaves very differently by position
- **DVH** — difference between vertical and horizontal mean; indicator of balance and poising quality

Active position is highlighted in purple.

**Right — Radar Chart (embedded)**

- Switchable metric: Amplitude (°) or Rate (s/d)
- **Red dots** — measured value at each position
- **Blue polygon** — connects the measured values
- **Green dashed circles** — tolerance boundaries
- A circular polygon means uniform performance across all positions
- Asymmetry reveals which positions are weak

Warning at bottom flags positions out of tolerance and suggests likely mechanical cause.

---

## 6. Beat Scope

![Beat Scope](graphs/06-beat-scope.png)

Single-beat waveform zoomed into approximately 20 milliseconds.

**Main plot**

- X-axis: Time in milliseconds (one beat window)
- Y-axis: |Amplitude| (absolute signal strength)
- **Green dashed line** — A event (tic, T1)
- **Red dashed line** — C event (tac, T3)
- The gap between green and red lines is the A→C interval — directly related to Amplitude and Beat Error

**Bottom strip** — thumbnails of the last 10 beats, each scaled to fit

- Consistent shape beat-to-beat = well-regulated watch
- Shape variation = beat-to-beat inconsistency
- Peaks shifting left/right = timing instability

**Controls**

- Single Beat / Average mode toggle
- Window width selector (ms)
- Latest beat / historical beat selector
- Σ averaging checkbox (overlays N beats for noise reduction)

---

## 7. Beat Error

![Beat Error](graphs/07-beat-error.png)

Two-panel view of tic/toc timing symmetry over time.

**Top panel — Beat Error trace**

- X-axis: Time in seconds (scrolling window, e.g., 30 s)
- Y-axis: Beat Error in milliseconds (scale: 1.2 ms default)
- **Purple line** — rolling Beat Error value
- **Red dots** — tic (A event) timing offset
- **Blue dots** — toc (C event) timing offset
- **Green background band** — tolerance zone (roughly 0–0.5 ms)
- Line inside green band = well adjusted. Rising line = beat going out of adjustment.

**Bottom panel — Timing offset by beat number**

- X-axis: Beat count
- Y-axis: Timing offset in milliseconds
- **Red dots** — tic offset, **Blue dots** — toc offset
- Both flat at zero = tic and toc perfectly even
- One series drifting while the other stays flat = beat adjustment needed

---

## 8. Long Term

![Long Term](graphs/08-long-term.png)

Three-panel view for extended monitoring sessions (minutes to hours).

Header bar: Rate / Amplitude / Beat Error current values + granularity mode

**Top panel — Rate (s/d)**

- **Pink/red line** — rate measurement
- **Dashed line** — session mean
- Initial noise settling into a stable value is normal (watch warm-up effect)

**Middle panel — Amplitude (°)**

- **Blue line** — amplitude per measurement
- **Shaded blue band** — ±1σ range
- **Dashed line** — mean
- Narrow band = consistent amplitude. Sudden drop = mechanical issue (friction, mainspring).

**Bottom panel — Beat Error (ms)**

- **Green line** — beat error per measurement
- **Shaded green band** — ±1σ range
- **Dashed line** — mean
- All three panels share the same time axis for easy correlation

---

## 9. Escapement

![Escapement](graphs/09-escapement.png)

Zooms into the A→C interval of a single beat with sub-millisecond precision.

- X-axis: Time from A event in milliseconds (spans roughly −2.5 ms to +11 ms)
- Y-axis: Normalized amplitude
- **Green vertical line** — A event (T1, tic)
- **Red vertical line** — C event (T3, tac, using peak or onset reference — selectable)
- **Amber shaded area** — the measured A→C interval
- **Δ label** — interval in milliseconds (e.g., Δ 6.84 ms)

Header shows:
- C reference mode (Peak or Onset)
- A→C interval value
- σ peak and σ onset across the last N beats
- Stability comparison between peak and onset detection

**How to read it:**
- Clean quiet zone between A and C = escapement releasing cleanly
- Extra bumps in the quiet zone = escapement not disengaging properly
- Low σ (< 0.02 ms) = highly consistent, reliable beat detection

---

## 10. Spectrogram

![Spectrogram](graphs/10-spectrogram.png)

Time-frequency view of the acoustic signal — shows which frequencies are present and when.

- X-axis: Time in milliseconds (one beat window, e.g., 200 ms)
- Y-axis: Frequency in Hz (0–20,000 Hz)
- **Color scale** (right side): Yellow = loud (−40 dB), Purple = quiet (−65 dB)
- **True Peak Programme Meter** (top bar): signal level in dBFS — green = healthy signal

**What you see:**
- **Two bright vertical bursts** — A event (left) and C event (right)
- Each impact produces energy across a very wide frequency range (broadband = sharp mechanical impact)
- Clean purple background between bursts = no spurious noise

**Red flags:**
- Extra burst between A and C = loose part or secondary impact
- Smeared / diagonal pattern = resonance or worn components
- Meter showing clipping (red) = microphone too close or gain too high

---

## 11. Waveform

![Waveform](graphs/11-waveform.png)

Stacks the last N individual beats side by side for direct comparison.

Each panel:
- X-axis: Time from A event in milliseconds (0–20 ms)
- Top x-axis: Balance wheel angle from A in degrees (−6° to lift angle, e.g., 52°)
- Y-axis: HPF amplitude (high-pass filtered signal)
- **Blue vertical line** — A event (tic)
- **Yellow spike** — C event (tac)
- Panel header: beat number and t_AC (A→C interval) for that beat

Summary header at top:
- Rate / Beat Error / BPH
- t_AC min / max / σ across all captured beats

**How to read it:**
- Yellow spike landing at the same position across all panels = consistent timing
- Spike shifting left/right = timing variation beat to beat
- Lift angle marker on top axis = where the C event should land for a correctly adjusted escapement

---

## 12. Sweep

![Sweep](graphs/12-sweep.png)

Shows multiple beats across one continuous time axis — oscilloscope-style.

- X-axis: Sweep time in milliseconds (configurable window, e.g., 500 ms)
- Y-axis: |Amplitude|
- Sweep length selector: number of ticks per sweep (e.g., 4 ticks = 4 A+C pairs)
- Each beat appears as a **pair of spikes** (A then C, separated by ~6.8 ms)
- Beat pairs repeat at the BPH-defined interval (~125 ms at 28800 BPH)

**How to read it** (per x-axis label):
- Stable pattern (spikes at same positions each sweep) = watch is on-rate
- Pattern drifting left = running fast
- Pattern drifting right = running slow

Header shows: Daily Rate / Amplitude / Beat Error / BPH confirmation

---

## 13. Filter Scope

![Filter Scope](graphs/13-filter-scope.png)

Shows one full beat cycle through four successive filter stages. Diagnostic tool for signal processing pipeline.

Header: beat cycle length in ms, sample count, sample rate, BPH

**4 panels (top to bottom):**

| Panel | Filter | Description |
|-------|--------|-------------|
| **Raw** | HPF only | High-pass filtered raw signal. Sharp event spike visible but noisy. |
| **Smoothed** | HPF + moving average | Noise reduced. Event shape cleaner. Both positive and negative halves visible. |
| **Envelope** | HPF + envelope | Absolute value of smoothed — energy profile of the event. |
| **Upper Envelope** | HPF + upper envelope | Single clean positive spike. This is what the beat detector uses. |

- X-axis on all panels: Time in milliseconds (one beat cycle)
- Y-axis: Amplitude (normalized)
- Each successive stage removes noise and sharpens the detectable peak
- Used during development to tune the High Pass Cutoff parameter (Misc. Parameters)

---

## 14. Radar Chart (Bonus)

![Radar Chart](graphs/14-radar-chart.png)

Full-page radar chart — same data as the embedded chart in the Sequence tab, but larger.

- Metric selector (top): Amplitude (°) or Rate (s/d)
- Position labels around the perimeter: CH, CB, 9H, 6H, 3H, 12H, CU(R), CU(L), CD(R), CD(L)
- **Red dots** — measured value at each position
- **Blue polygon** — connects measured values; filled light blue
- **Green dashed circles** — tolerance bands (inner = minimum, outer = target)

**How to read it:**
- Circular, symmetric polygon hugging the outer circle = uniform performance, well-regulated
- Pulled-in segment = that position has lower amplitude or abnormal rate
- Asymmetry between horizontal (CH/CB) and vertical (9H–12H) groups → DVH imbalance → poising issue
- Warning message at bottom identifies worst position and suggests mechanical cause

---

## Image File Checklist

Place screenshots in `docs/milestone3/graphs/`:

| Filename | Tab name |
|----------|----------|
| `01-rate-scope.png` | Rate/Scope |
| `02-sound-print.png` | Sound Print |
| `03-trace.png` | Trace |
| `04-vario.png` | Vario |
| `05-sequence.png` | Sequence |
| `06-beat-scope.png` | Beat Scope |
| `07-beat-error.png` | Beat Error |
| `08-long-term.png` | Long Term |
| `09-escapement.png` | Escapement |
| `10-spectrogram.png` | Spectrogram |
| `11-waveform.png` | Waveform |
| `12-sweep.png` | Sweep |
| `13-filter-scope.png` | Filters |
| `14-radar-chart.png` | Radar |
