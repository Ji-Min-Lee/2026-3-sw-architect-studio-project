# Final Demo Checklist — Team 3 : Blue Sky

## Schedule

| Item | Detail |
|------|--------|
| Date | Wednesday, July 1, 2026 |
| Demo order | **Team 4 → Team 3 → Team 5 → Team 2 → Team 1** |
| Team 3 demo slot | ~9:00 AM (Team 4: 8:30 + 20 min + 10 min buffer) |
| Demo duration | 20 min per team |
| Buffer between teams | 10 min |
| Presentation order | Same order as demo, after all demos complete |
| Presentation duration | 20 min per team |

**Sequence**: Demo first, then presentation.
**Strategy**: Follow the rubric order top-to-bottom — easier for evaluators to check off items as you go. Mark rubric section numbers on slides so evaluators can follow along.

---

## Accuracy Verification (Steve's guidance)

Use **two systems** side by side and compare their measurements:
- If results are similar → strong evidence of accurate measurement
- If results differ → investigate and explain why (calibration, noise, microphone, filter issues, etc.)
- Multiple measurements over time with a consistently wound watch strengthen the accuracy argument
- Do not just note discrepancies — reason from evidence

> "Think like a scientist! You are one!" — Steve

---

## Pre-Demo Setup

### Hardware
- [ ] Raspberry Pi 5 powered on and booted
- [ ] Touchscreen connected and working
- [ ] USB microphone / sensor connected to RPi
- [ ] Mechanical watch mounted on sensor stand
- [ ] WeiShi No.1000 or second reference device available for accuracy comparison
- [ ] AGC disabled — verify with `alsamixer` (Auto Gain Control = MM)

### Software
- [ ] Latest TimeGrapher build deployed to RPi
- [ ] Application launches without errors
- [ ] All graph tabs visible and reachable
- [ ] Live mode working; Playback / Sim mode available as fallback

---

## Grading Rubric Checklist (Total: 200 pts + 15 bonus)

> Follow this order during the demo. Each checked item = points on the board.

---

### Area 1 — Additional Real-Time Graphs and Diagnostic Displays (60 pts)

Each display: 5 pts (full = functional + integrated; partial = incomplete; zero = missing)

- [ ] Watch-Position Testing display
- [ ] Trace Display
- [ ] Rate and Amplitude Stability Over Time
- [ ] Multi-Position Sequence Display
- [ ] Beat-Noise Scope Display
- [ ] Beat Error Display and Diagnostic Trace
- [ ] Long-Term Performance Graph
- [ ] Escapement Analyzer and Marker-Line Display
- [ ] Time-Frequency Spectrogram Display
- [ ] Waveform Comparison Display with Timing Markers
- [ ] Scope Mode with Synchronized Sweep Display
- [ ] Scope Function with Multiple Filter Views

---

### Area 2 — System Enhancements & AI Feature (25 pts)

- [ ] Sound Print enhancements — event detection, readability, or interpretation improved (8 pts)
- [ ] Rate/Scope enhancements — usefulness, accuracy, navigation, or measurement clarity improved (8 pts)
- [ ] Team-selected AI feature implemented as a useful proof of concept (5 pts)
- [ ] Explain what problem the AI feature addresses and how well it works (4 pts)

> Grading scale: Excellent / Strong / Moderate / Minimal / None

---

### Area 3 — Quality Attribute Tradeoff Discussion (20 pts) — slides + demo

- [ ] Identify the major quality attributes relevant to this project (5 pts)
- [ ] Clearly explain tradeoffs among quality attributes (5 pts)
- [ ] Show that **accuracy** was treated as the highest-priority attribute (5 pts)
- [ ] Explain what was actually achieved and what limitations remain (5 pts)

---

### Area 4 — Performance, Latency, and Correctness (25 pts) — slides + demo

- [ ] Demonstrate real-time performance on the Raspberry Pi (8 pts)
- [ ] Demonstrate low latency from signal capture to display/update (6 pts)
- [ ] Demonstrate correctness of calculations, event detection, and displayed values (6 pts)
- [ ] Present supporting evidence: experiments, measurements, logs, or comparisons (5 pts)

> Full points require live demonstration on target hardware with meaningful evidence (e.g., latency logs, accuracy comparison data).

---

### Area 5 — Extensibility of the Architecture (20 pts) — slides

- [ ] Architecture is modular and separates major concerns clearly (6 pts)
- [ ] Architecture supports adding new measurements, filters, graphs, or displays with limited redesign (6 pts)
- [ ] Explain how the structure supports future requirements or enhancements (4 pts)
- [ ] Code organization and interfaces make the system understandable and maintainable (4 pts)

---

### Area 6 — Remote User Interface / GUI Modifications (25 pts)

- [ ] GUI improvements make the system easier to use and understand (4 pts)
- [ ] System detects and responds to sensor or microphone unplug/replug events (5 pts)
- [ ] UI layout uses screen space effectively; less-used controls in drop-downs or similar (4 pts)
- [ ] Beat-synchronized display of A and C events in the same relative graph position per cycle (4 pts)
- [ ] GUI reduces or filters handling noise while preserving useful signal features (A and C) (4 pts)
- [ ] GUI provides clear overall system health, status, or measurement-readiness feedback (4 pts)

---

### Area 7 — Use of AI in Building the Software (15 pts) — slides

- [ ] Clearly explain how AI tools were used in development (5 pts)
- [ ] Show thoughtful use of AI for design, coding, debugging, testing, documentation, or analysis (5 pts)
- [ ] Reflect on the strengths, limitations, and risks of AI-assisted development (5 pts)

---

### Area 8 — Best User Interface (10 pts)

- [ ] Sponsor comparative judgment — 1st: 10 / 2nd: 8 / 3rd: 6 / 4th–5th: 4

---

### Bonus — Additional Advanced Features (up to 15 pts)

- [ ] Radar chart using multi-position watch data to assess overall watch health (8 pts)
- [ ] Diagnosis / classification feature based on readings (7 pts)

---

## Grading Scale Reference

| Max pts | Outstanding | Satisfactory | Marginal | Unsatisfactory | Not Acceptable |
|---------|-------------|--------------|----------|----------------|----------------|
| 10 | 10 | 8 | 5 | 3 | 0 |
| 8 | 8 | 6 | 4 | 2 | 0 |
| 5 | 5 | 4 | 3 | 1 | 0 |
| 4 | 4 | 3 | 2 | 1 | 0 |

- **Outstanding**: Above and beyond — robust, original design rationale fully supported
- **Satisfactory**: Completed all aspects — critical thinking, clear design rationale
- **Marginal**: No code changes but feature executes
- **Unsatisfactory**: Does not meet performance or quality requirements
- **Not Acceptable**: Feature does not work

---

## Fallback Plan

| Scenario | Fallback |
|----------|----------|
| Watch microphone not working | Use Playback mode with pre-recorded PCM file |
| RPi performance issue | Demo on PC build; explain RPi results separately |
| Graph tab crashes | Skip tab, note issue, demo remaining tabs |
| No reference device available | Use pre-recorded comparison data |

---

## Q&A Preparation

**Quality attribute evidence table** — prepare this for Q&A:

| QA | Target | Achieved | Evidence |
|----|--------|----------|---------|
| Real-time performance | No dropped blocks | | FPS / log |
| Low latency (E2E) | Minimize | ___ ms avg | Latency display |
| Measurement consistency | Rate σ ≤ 1.0 s/d | | Vario σ value |
| Accuracy | Within ±2 s/d of reference | | Side-by-side comparison |
| Extensibility | New graph = presentation layer only | | Code walkthrough |

---

## Steve's Q&A Summary

**Demo order (random shuffle):** 4 → 3 → 5 → 2 → 1

**Presentation flow:** Demo first, then presentation (same order). Annotate slides with rubric section markers so evaluators can follow along without asking you to jump around.

**Accuracy verification:** Compare two systems simultaneously. If they differ, investigate — poor science is noting a discrepancy without attempting to explain it. Consistent repeated measurements over time strengthen the accuracy argument.

**Demo scenario:** You design the flow. The closer it follows the rubric, the easier it is for evaluators to award full points.
