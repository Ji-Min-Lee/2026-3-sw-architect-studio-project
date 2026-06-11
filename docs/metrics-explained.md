# Metrics & Graphs Explained

What a "frame" is in the TimeGrapher logging, how it leads to FPS/SPS/SPF, and
what every graph panel shows.

---

## 1. What is a "frame"?

A **frame** is **one foreground processing pass** — a single
`HandleInputData()` call that actually processed audio samples.

It is **not** a video/display frame. It is one unit of audio work.

### How a frame is produced

```
[mic / sim / playback]
        │  produces a chunk of PCM (~480 samples = 10 ms @ 48 kHz)
        ▼
BG worker thread            (AudioWorker / SimWorker / PlaybackWorker)
   - writes chunk into the shared ring buffer
   - emit XxxDataReady(T0)        T0 = emit timestamp
        │
   Qt event queue  (cross-thread, queued connection)
        │
        ▼
FG main thread              (Qt event loop)
   - dequeues the event
   - HandleInputData()  ← THIS invocation = 1 frame
        - reads everything new from the ring buffer (= `samples`)
        - ProcessSamples(): copy -> sound -> tg -> ui -> plot
```

- The BG worker emits roughly every **10 ms** (≈ 100 times/sec) because that is
  the OS audio chunk period (WASAPI on Windows, ALSA on the Pi).
- Each emit becomes one FG frame — **if** the main thread can keep up.
- A frame is only **logged** when it processed `samples > 0`.

### `samples` per frame (backlog indicator)

```
samples = TotalSamplesWritten(BG) - LastTotalSamplesWritten(FG, previous)
```

- `samples ≈ 480` → normal: the FG consumed exactly one chunk.
- `samples > 480` → the FG fell behind; multiple chunks piled up in the ring
  buffer and were drained in one frame. Higher = worse backlog.

So a frame normally carries **~480 samples**, but when the FG slows down a
single frame can carry far more — it drains whatever the BG accumulated while
the FG was blocked:

```
FG keeps up    → samples = 480              (one 10 ms chunk)
FG falls behind → samples = 480 × N         (N chunks drained at once)
                  e.g. 960, 1440, ... thousands
```

This also shows up as **FG SPF rising above 480** (same quantity, consumer side)
and is a direct symptom of a large `wait`.

### The 480 baseline depends on sample rate × chunk period

`480` is not hard-coded — it is `sample_rate × chunk_period`:

```
48 kHz × 10 ms = 480     (baseline used in the examples here)
96 kHz × 10 ms = 960
```

The actual baseline for a run is the measured **BG SPF**. Read `samples`
relative to BG SPF: equal = healthy, well above = backlog.

Observed in sample runs: mic avg ≈ 774 (max 12480), sim avg ≈ 1496 (max 6240) —
i.e. both were, on average, processing more than one chunk per frame (falling
behind).

---

## 2. From frames to FPS / SPS / SPF

Both the background (producer) and foreground (consumer) compute the same three
rates **independently**, each averaged over a ~2-second window.

| Metric | Meaning | Formula |
|--------|---------|---------|
| **FPS** | Frames per second | frames / elapsed |
| **SPS** | Samples per second | samples / elapsed |
| **SPF** | Samples per frame | samples / frames |

### BG vs FG

```
BG FPS / SPS / SPF   measured in the worker thread
   = the rate audio is PRODUCED
   ≈ 100 fps, 48000 sps, 480 spf  (steady, set by the OS)

FG FPS / SPS / SPF   measured in the main thread
   = the rate audio is CONSUMED / handled
   - FG SPS should match BG SPS (no net sample loss on average)
   - FG FPS drops below BG FPS when the main thread is blocked
   - FG SPF rises above 480 when frames are processed in bursts
```

### Why FG SPF can be huge

If the main thread is blocked (e.g. a long paint), it wakes up rarely and drains
a large backlog in one frame:

```
FG SPF = total samples handled / number of FG frames
       → few frames, many samples  ⇒  SPF spikes (e.g. 2000, 12000)
```

This is the consumer-side symptom of the same backlog seen in `samples`.

### Throughput vs latency (important)

- **SPS matching (BG ≈ FG)** means throughput is fine — over time, everything
  was processed. It does **not** mean it was processed on time.
- **wait / FPS / samples** reveal latency and backlog — whether it was timely.

So good SPS + bad wait = "all data handled, but late and in bursts."

---

## 3. Per-frame timing fields

Measured in microseconds (us), reported in milliseconds (ms).

```
T0 ──────────── wait ──────────► FG start ──── exec ──── FG end
(BG emit)                                                (done)

total = wait + exec
```

| Field | Meaning |
|-------|---------|
| `wait` | T0 → FG handler start. Time in the Qt queue + OS scheduling. Large when the main thread was busy with something else. |
| `exec` | FG handler start → end. Actual processing time. |
| `total`| `wait + exec`. The code-observable end-to-end per frame. |

### `exec` breakdown

| Section | Work |
|---------|------|
| `copy`  | ring buffer → input block (memcpy) |
| `sound` | SoundImageRenderer.processSamples() |
| `tg`    | tg_process() — beat detection (scales with samples) |
| `ui`    | ScopePlot addData + A/C event markers |
| `plot`  | PurgeHistory + replot + DrawImage (usually dominant, ~fixed cost) |

> Full end-to-end latency also includes the OS audio buffering (~10 ms) that
> happens before T0 and cannot be measured in code.

---

## 4. Pipeline graph — `log_<timestamp>.png`

Generated by `analyze_log.py` from the per-frame CSV. Panels 1–4 use a rolling
average (default window 100 frames) for a smooth overview; the bottom row shows
raw per-frame detail.

**Panel 1 — Latency (rolling avg): total = wait + exec**
- Lines: `total` (red), `wait` (orange), `exec` (blue); dashed line at 10 ms.
- Read it: if `total` tracks `wait` (they overlap), latency is dominated by
  queue waiting, not by processing. `exec` under 10 ms = processing is healthy.

**Panel 2 — samples + BG/FG FPS**
- Left axis: `samples` (purple) with a dashed line at SPF (480).
- Right axis: `FG fps` (green), `BG fps` (gray).
- Read it: `samples` riding above 480 and `FG fps` below `BG fps` = backlog /
  the consumer is not keeping up frame-by-frame.

**Panel 3 — exec breakdown (rolling avg)**
- Lines: `plot`, `tg`, `ui`, `copy`.
- Read it: which section dominates `exec`. Usually `plot` is largest.

**Panel 4 — BG vs FG SPS (throughput)**
- Lines: `BG sps` (gray), `FG sps` (green).
- Read it: if they overlap, no net sample loss on average (throughput OK).

**Bottom row — per-frame horizontal breakdowns** (one bar = one frame)
- **e2e** = `wait + copy + sound + tg + ui + plot` stacked → full per-frame time.
- **wait only** → isolates queue/scheduling wait.
- **exec only** (own scale) → composition of processing (plot/tg/ui/copy/sound).
- Read it: compare *e2e* and *wait* — if they look the same, wait dominates;
  the *exec* panel (zoomed) shows what processing itself spent time on.

---

## 5. System graph — `log_<timestamp>_sys.png` (RPi)

Generated only when a `_sys.csv` exists (Linux/RPi). Sampled once per console
window (~1 s).

**Panel 1 — CPU utilization (total + per core)**
- `total` (black) plus `cpu0..cpuN`.
- Read it: a single core pinned at ~100% can explain `wait` spikes even when
  overall CPU looks moderate.

**Panel 2 — Memory (used / total)**
- `used` (purple) vs `total` (dashed). Watch for growth over a long run
  (the RPi has limited RAM; the spec calls for careful memory use).

**Panel 3 — CPU temperature**
- `temp` (red) with an 80 °C reference line.
- Read it: approaching the throttle threshold predicts frequency drops.

**Panel 4 — CPU frequency (red lines = throttled)**
- `freq` (blue); vertical red lines mark samples where the throttled bitmask ≠ 0.
- Read it: frequency dropping together with high temperature = thermal
  throttling, a common reason sustained real-time performance degrades on the Pi.

---

## 6. Quick reading guide

| Question | Look at |
|----------|---------|
| Is processing fast enough? | `exec` (Panel 1) — should stay < 10 ms |
| Are we keeping up in real time? | `wait`, `samples`, `FG fps` (Panels 1–2) |
| Where does processing time go? | exec breakdown (Panel 3 / bottom-right) |
| Are samples being lost on average? | BG vs FG SPS (Panel 4) |
| Is the Pi throttling? | temperature + frequency (System Panels 3–4) |
| Is one core the bottleneck? | per-core CPU (System Panel 1) |
