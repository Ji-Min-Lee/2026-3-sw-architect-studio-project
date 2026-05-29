# Final Demo Checklist

**Milestone**: M3 | **Date**: 2026-07-01 | **Platform**: Raspberry Pi 5

---

## Pre-Demo Setup Checklist

### Hardware
- [ ] Raspberry Pi 5 powered on and booted
- [ ] 8" touchscreen (1280×800) connected via HDMI + USB
- [ ] USB Sensor Stand + Converter Box connected to RPi USB port
- [ ] Mechanical watch mounted on sensor stand
- [ ] WeiShi No.1000 reference device available (for comparison demo)
- [ ] AGC **disabled** in AlsaMixer — verify with: `alsamixer` → Auto Gain Control = MM

### Software
- [ ] Latest TimeGrapher build deployed to RPi
- [ ] Application launches without errors
- [ ] All implemented graph tabs visible in tab bar
- [ ] Live mode selected and confirmed working
- [ ] Sim mode available as fallback

---

## Demo Script

### 1. System Introduction (2 min)
- Show watch on sensor stand
- Launch TimeGrapher GUI on RPi touchscreen
- Point out: Measurement Summary Bar (Rate, Amplitude, Beat Error, BPH)
- Briefly explain what each value means

### 2. Live Mode Demonstration
- [ ] Start measurement in Live mode
- [ ] Show real-time values stabilizing
- [ ] Point out Rate in s/d — is watch fast or slow?
- [ ] Show Amplitude — is it in healthy range (270°–310°)?
- [ ] Show Beat Error — is it within 0.6 ms?

### 3. Demonstrate Implemented Graph Tabs

For each graph tab, demonstrate:
- What it shows
- How it helps the user interpret the watch
- How it was integrated into the existing app (not a separate prototype)

| Tab | Demo Points | Status |
|-----|------------|--------|
| **Trace Display** | Real-time rate + amplitude scrolling; smoothing function; alert when out of range | [ ] |
| **Vario Display** | Min/Max/Avg/σ updating live; acceptable range visualization | [ ] |
| **Multi-Position Sequence** | Record CH position, switch to 9H, compare; X·D summary values | [ ] |
| **Beat-Noise Scope 1** | Beat strips; selectable time range (20/200/400ms); A/C markers | [ ] |
| **Beat-Noise Scope 2** | Tic/tac dual-axis; averaging (Σ) on/off | [ ] |
| **Beat Error Trace** | Numerical + trace line; slope indicates fast/slow | [ ] |
| **Long-Term Performance** | Extended time series; rate/amplitude/beat error | [ ] |
| **Escapement Analyzer** | Waveform + A/C marker lines + ms labels | [ ] |
| **Scope Mode** | Oscilloscope sweep; configure sweep time | [ ] |
| **Scope Function** | F0/F1/F2/F3 simultaneously; explain filter differences | [ ] |

*(Only demo tabs that are fully implemented — skip or note any incomplete)*

### 4. Quality Attribute Evidence

#### Low Latency
- [ ] Show latency numbers in GUI (or explain how measured)
- Values to report:
  - Capture → Process latency: ___ ms (avg), ___ ms (worst)
  - Process → Display latency: ___ ms (avg), ___ ms (worst)
  - End-to-end latency: ___ ms (avg), ___ ms (worst)

#### Real-Time Performance on Raspberry Pi
- [ ] Confirm GUI is running on RPi (show device, not PC)
- [ ] Show FPS counter or demonstrate smooth real-time update
- [ ] Report: sample rate = ___k sps, dropped blocks = ___

#### Consistency (Measurement Stability)
- [ ] Let measurement run for 30+ seconds — show values remain stable
- [ ] Point to Vario display showing low σ (std deviation)
- Values: Rate σ = ___ s/d, Beat Error σ = ___ ms

#### Accuracy
- [ ] Compare displayed Rate/Amplitude/Beat Error with WeiShi No.1000 reading
- [ ] Difference within tolerance? Rate ≤ ±2 s/d, Amplitude ≤ ±5°, Beat Error ≤ ±0.1 ms

#### Extensibility
- [ ] Open source code and show: adding a new graph tab touches only Presentation layer
- [ ] Point to architecture diagram showing zero changes to Domain/Processing when adding graphs
- Or: demonstrate a simple added display to prove the claim

---

## Fallback Demo Plan

> If hardware fails or signal is poor, switch to:

| Scenario | Fallback |
|----------|----------|
| Watch microphone not working | Use Playback mode with recorded PCM file |
| RPi performance issue | Demo on PC (macOS/Windows build) + explain RPi results separately |
| Graph tab crashes | Skip tab, note issue, demo remaining tabs |
| No WeiShi reference available | Use pre-recorded comparison data |

---

## Quality Attribute Summary Table

> Prepare this table to present during demo Q&A.

| QA | Target | Achieved | Evidence |
|----|--------|----------|---------|
| Real-Time Performance | 96k sps, no dropped blocks | | |
| Low Latency (end-to-end) | Minimize ms | ___ ms avg | Latency display |
| Consistency | Rate σ ≤ 1.0 s/d | | Vario σ value |
| Accuracy | Within ±2 s/d of WeiShi | | Side-by-side comparison |
| Extensibility | New graph = Presentation only | | Code walkthrough |

---

## Post-Demo Checklist

- [ ] Demo video or screenshots captured
- [ ] Latency numbers recorded for presentation
- [ ] Accuracy comparison data saved
- [ ] All team members can answer questions about architecture decisions
