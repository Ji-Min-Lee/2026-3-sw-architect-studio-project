# Enhancement SP-3: Beat Period Grid Overlay on Sound Print

**Area**: System Enhancements & AI Feature (Area 2) — Sound Print enhancements (8 pts)
**Branch**: `feature/enhancements`
**Status**: Implemented

---

## Summary

Added two semi-transparent horizontal grid lines to the Sound Print display:
a green line at the beat boundary (A event expected position) and a blue line
at the half-period mark (C event expected position). Colors match the A/C event
dot markers. A legend overlay and hover tooltip explain the display to the user.

---

## Before / After

Both screenshots captured from the same playback file (28800 BPH, Playback/Sim mode).

| | Before | After |
|--|--------|-------|
| Grid lines | None | Green (A boundary) + Blue (C half-period) |
| Legend | None | Top-right overlay — A event / C event / A boundary / C boundary |
| Tooltip | None | Hover near each line shows its meaning |

**Before** — event dots visible but no reference baseline; impossible to tell at a glance whether positions are normal or deviated:

![Before SP-3](sp3_before.png)

**After** — green line marks A event boundary, blue line marks C event boundary; legend explains each element:

![After SP-3](sp3_after.png)

The Sound Print now shows:
- Green line at the top of the signal band — A event boundary (beat start)
- Blue line at the bottom of the signal band — C event boundary (half-period)
- Top-right legend box with color-coded labels for all four visual elements
- Signal waveform (red), A event dots (green), C event dots (blue) unchanged

---

## What Changed

### Files modified

| File | Change |
|------|--------|
| `src/external/SoundImageRenderer.h` | Added `beat_grid_enabled`, `beat_grid_color`, `beat_grid_half_color` to `Config` |
| `src/external/SoundImageRenderer.cpp` | `renderBinsToColumn()`: alpha-blend grid lines at `natural_bucket=0` and `height/2` after signal render |
| `src/tabs/SoundPrintTab.cpp` | Enabled grid; set green/blue colors matching A/C event markers |
| `src/external/SoundImageWidget.h` | Added `mouseMoveEvent()` declaration, `setMouseTracking(true)` |
| `src/external/SoundImageWidget.cpp` | `mouseMoveEvent()`: QToolTip by Y-fraction zone; `paintEvent()`: legend overlay |

### Grid line positions

| Line | `natural_bucket` | Color | Meaning |
|------|-----------------|-------|---------|
| A event boundary | `0` | Green `rgba(0,200,0,160)` | Beat start — where A (tick/toc) impulse is expected |
| C event boundary | `height/2` | Blue `rgba(0,0,220,100)` | Half-period — where C impulse is expected for balanced beat error |

---

## Theoretical Basis

The placement of the two grid lines is derived from the watch measurement
literature and the project's own equation specification.

### Green line — A event boundary (`natural_bucket = 0`)

**Source: Witschi Training Course p. 14; TimeGrapher Equations p. 1–2**

The Sound Print renderer maps each beat period to one image column.
`natural_bucket = 0` is defined as the start of that column — i.e., the moment
the new beat period begins. This is precisely when the A impulse (tic or toc)
is expected to arrive, as stated in the TimeGrapher Equations core formula:

```
E_n = T_measured − (T_start + n × I_target)
```

where `I_target = 3600 / BPH` seconds per beat. Beat n is anchored to
`T_start + n × I_target`, meaning each A event *defines* the beat boundary.
The Witschi Training Course p. 14 confirms this visually: in the healthy-watch
Sound Print, the upper row of dots (A events) forms a straight horizontal band
at a fixed position within each column.

### Blue line — C event boundary (`natural_bucket = height/2`)

**Source: TimeGrapher Equations pp. 4–5 (Part II, Alternating Event Model)**

The C impulse is the complementary escapement sound that falls between two
consecutive A events. For a perfectly balanced watch (beat error = 0 ms), the
C event arrives exactly at the half-period:

```
T_nom, same-phase = 7200 / BPH   (full tic-to-tic or tac-to-tac period)
I_target           = 3600 / BPH   (half-beat interval, A-to-C gap at zero beat error)
```

Because the image column height maps linearly to one full beat period,
`natural_bucket = height/2` corresponds to `I_target / 2 × 2 = I_target` from
the column start — the ideal C event position when beat error is zero.
Any vertical offset of the C dots from this line directly equals the beat error,
confirmed by the Witschi Training Course p. 14: a displaced lower dot row
indicates beat error requiring pallet adjustment.

### Summary table

| Line | Position | Equation reference | Witschi reference |
|------|----------|--------------------|-------------------|
| Green (A boundary) | `natural_bucket = 0` | `T_n = T_start + n × I_target` (Equations p. 1) | Upper dot row, p. 14 |
| Blue (C half-period) | `natural_bucket = height/2` | `I_target = 3600/BPH`, beat error = 0 (Equations p. 4–5) | Lower dot row offset = beat error, p. 14 |

---

## How to Interpret the Display

The Sound Print is a scrolling bitmap:
- **X axis** — beat number (newest beats enter from the right)
- **Y axis** — time within one beat period (top = beat start, middle = half-period)

The two grid lines act as reference baselines. The position and pattern of the
A/C dots relative to these lines directly reveals the watch condition.

### State examples

![Sound Print state examples](sp3_state_examples.png)

| State | Pattern | Diagnosis |
|-------|---------|-----------|
| **Healthy watch** | A·C dots both sit tightly on their reference lines, horizontally level | Stable escapement — no beat error, no rate deviation |
| **Beat error** | A dots on green line (ok); C dots displaced consistently below blue line (+14 ms shown) | A-C asymmetry in escapement (pallet fork imbalance) — requires pallet adjustment |
| **Rate drift (fast)** | Both A and C dots drift diagonally upward across beats | Overall rate error — watch is running fast; requires regulator adjustment |
| **Jitter / amplitude instability** | Dots scattered vertically with no consistent position relative to either line | Timing jitter from noise, mainspring amplitude variation, or mechanical wear |

---

## Value for Grading (Area 2)

The grading rubric asks whether the Sound Print enhancement "improves event
detection, readability, or interpretation."

| Criterion | How SP-3 addresses it |
|-----------|----------------------|
| **Readability** | Grid lines make the beat period structure immediately visible without prior knowledge of the display format |
| **Interpretation** | Color-coded reference lines let the observer classify watch state (healthy / beat error / drift / jitter) at a glance without computing offsets manually |
| **Discoverability** | Legend and hover tooltip explain every visual element — no manual required |

---

## Implementation Notes

- Grid lines are **alpha-blended** over the signal pixels, so A/C event markers
  remain fully visible and are never obscured by the grid.
- `beat_grid_half_color` uses lower alpha (100 vs 160) than `beat_grid_color`
  because the C boundary is a derived reference (assumes beat error = 0), not a
  hard structural boundary.
- The legend is rendered in `paintEvent()` via QPainter **overlay** — it does not
  modify the underlying QImage, so it has no effect on `sound_ms` or renderer
  pipeline performance.
- Tooltip zones use Y-fraction bands (±4% for A line, 46–54% for C line)
  relative to widget height so they remain correct under widget resize.
