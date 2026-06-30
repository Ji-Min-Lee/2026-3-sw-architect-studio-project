# TimeGrapher — Final Demo Script (20 min)
**Team**: Blue Sky (Team 3) | **Date**: July 1, 2026 | **Platform**: Raspberry Pi 5

> **Setup before demo**: TimeGrapher GUI running on RPi, WeiShi No.1000 powered on with the same watch loaded, log output visible on screen.

---

## 0:00 – 1:30 | System Introduction + UI Overview

> "Good morning. We are Team Blue Sky. Let's get started by running our application.
> On startup the app reads the screen resolution and sizes itself automatically.
> Let's go to fullscreen mode — hopefully everyone back can see that."

- Set Sample Rate to 96000 Hz, Mode to Extended
- Launch TimeGrapher, press F11

> "Before we start, I will set sample rate to 96 kHz.
> 192000 kHz gives better resolution, but after a few minutes on the Pi it starts slowing down.
> So let's run live mode first."

> "let me walk you through the layout quickly.
> Left side is the control panel. Right side has multiple graph tabs.
> On top you can see the live readings — Position, Rate, Amplitude, Beat Error, and BPH.
> In top-right corner, there is the AI diagnosis badge.
>
> In the control panel, we update some of run parameters and watch parameters for better usuablity.
> For example, recording a session used to open a separate dialog.
> We replaced it with a simple checkbox, so that we can check it before starting, and the session is recorded automatically.
> Also, for example, we added watch type for men and women as well, this is because the rate tolerances are different."

- Click Mode → Sim to show SimFrame

> "I am gonna stop the live mode for a moment. I switch to the mode Sim, you can see a simulation panel appears"

- Click Advanced

> "There's also an Advanced section — collapsed by default.
> In here you can pick which AI model Ollama uses for diagnosis,
> and there's a checkbox for auto position detection. If it is checked, watch position will be detected automatically"

- Collapse Advanced

> "And the status bar here shows the session state as you go.
> It starts as Ready. Once you start measuring, it goes to Acquiring — collecting enough beats to confirm a diagnosis.
> Then Running once it locks in. And Paused if you freeze the graphs to take a closer look."


- Press Space to start

---

## 1:30 – 6:30 | Area 1 — Watch-Position Testing + Area 2 — Base Graph Enhancements

### Watch-Position Testing (5 pts)

- Show watch on stand, status bar showing "CH"

> "Each watch position changes how gravity acts on the balance wheel — rate and amplitude shift.
> We detect position automatically using amplitude tracking.
> Two positions work reliably — CH, dial up, and 6H, crown left.
> Let me show you."

- Rotate watch from CH to 6H, show POS label update in status bar

> "POS switched automatically — no button pressed.
>
> The other four positions — CB, 3H, 9H, 12H — the amplitude difference between them
> was too small and too noisy to distinguish reliably with just a microphone.
> We tried, ran experiments, couldn't get it below the noise floor.
> So for those, the user selects the position manually from the dropdown."

- Switch to Sequence tab, select a position manually from the dropdown

> "This is the Sequence tab. Each row is a watch position.
> As measurements come in for each position, the table fills in — Rate, Amplitude, Beat Error.
> And if I select a position manually here, the system starts recording data for that orientation.
>
> On the right you can see the Radar Chart.
> Each point around the edge is a position. The polygon connects the measured amplitudes.
> A perfectly balanced watch gives you a circle.
> Where the polygon pulls inward, that position is weaker.
> At a glance you can see which orientations need attention — without reading a table."

---

### Area 2 — Sound Print Enhancements (8 pts)

- Switch to Sound Print tab

> "Two improvements. First — dot coloring by signal strength.
> For example for A-event, we can see green for strong signal and yellow for weak signal.
> Second — click any column. We can check the raw PCM waveform for that beat.
> We also have Prev and Next buttons to look around neighbor beats.

- Click a column, show waveform popup, click Prev/Next

---

### Area 2 — Rate / Scope Enhancements (8 pts)

- Switch to Rate/Scope tab

> "There are three improvements.
> On top-left corner, we have stat box for mean, sigma, beat count. We can check these session-wide stats at a glance.
> Second, we have Green trend line, which is 20-beat rolling average.
> Third, we can click the below scope panel to cross-reference that beat in the rate plot above. Here, you can see orange marker above > whenever I click the plot below"

- Click scope panel to show orange marker

---

## 6:30 – 13:00 | Area 1 — Additional Graph Displays (55 pts)

> "Eleven additional graph displays. Each one quickly."

- Click "More" to show the full tab list

---

**① Trace** — Switch to Trace tab

> "Rate and Amplitude as continuous scrolling lines.
> Color bands show the tolerance zone — green is healthy, red means service needed.
> Right now both are flat and green."

---

**② Vario** — Switch to Vario tab

> "Session summary at a glance — Rate and Amplitude as Min, Max, Now, and sigma.
> Arrows on the scale show where the values land. Green background means within tolerance."

---

**③ Sequence** — Switch to Sequence tab

> "All six watch positions in one table — Rate, Amplitude, Beat Error per position.
> The Radar Chart on the right shows amplitude balance across positions.
> A perfect circle means a perfectly balanced watch."

---

**④ Beat Scope** — Switch to Beat Scope tab

> "Acoustic waveform of a single beat, zoomed to 20ms.
> Green line is the A event, red is C. The gap gives us Beat Error and Amplitude.
> The thumbnail at the bottom shows the last ten beats. We can check consistency at a glance."

---

**⑤ Beat Error** — Switch to Beat Error tab

> "Beat Error over time, with the green tolerance band.
> Red and blue dots are the Tic and Toc offsets."

---

**⑥ Long Term** — Switch to Long Term tab

> "Rate, Amplitude, and Beat Error over the full session — hours if needed.
> To avoid unbounded memory growth, we use bucket averaging:
> the data is compressed automatically as the session grows."

---

**⑦ Escapement** — Switch to Escapement tab

> "Zooms into the A-to-C interval of a single beat — the yellow shaded region.
> Shows the exact timing with sub-millisecond sigma across repeated measurements."

---

**⑧ Spectrogram** — Switch to Spectrogram tab

> "Watch sound split into time and frequency.
> Vertical stripes are the beats — 8 per second at 28800 BPH, each spanning the full frequency range.
> Green bar at the top is signal strength — green means the microphone is picking up clearly."

---

**⑨ Waveform** — Switch to Waveform tab

> "Three consecutive beats stacked and aligned to the A event.
> Blue line is A, yellow spike is C — the t_AC interval between them is how we calculate amplitude.
> Tight sigma across all three beats means the watch is swinging consistently."

---

**⑩ Sweep** — Switch to Sweep tab

> "Overlays every 250ms window on top of each other.
> If the watch is on rate, the spikes stack sharp and narrow.
> If it's drifting, the spikes spread out. Here — sharp. Rate is stable."

---

**⑪ Filter Scope** — Switch to Filters tab

> "Shows the signal through four DSP stages — Raw, Smoothed, Envelope, Upper Envelope.
> Each stage removes noise until clean unambiguous spike reaches the beat detector."

---

**⑫ Radar Chart** — Switch to Radar Chart tab

> "Radar Chart visualizes amplitude across all six watch positions as a polygon.
> A perfectly balanced watch produces a circle.
> Deviations show exactly which positions are weak and by how much —
> at a glance, without reading a table.
> The warning at the bottom names the worst position and what it likely means mechanically."

- Point to the asymmetric polygon and warning message

---

## 13:00 – 15:00 | Area 2 — AI Feature (9 pts)

- More → Developer Info (check)
- Lower Sample Rate to 44100 Hz before triggering diagnosis

> "I will explain AI features we have. I'm turning on Developer Info so you can watch the CPU usage while the AI runs.
> By the way — this overlay is hidden by default. It's built just for this demo, and wouldn't ship in a production release.
> One thing to keep in mind before I show this.
> We want to use cloud AI model, but the studio project required all inference to run locally — no internet, no cloud.
> And, the Pi has no GPU, no NPU. It's just four CPU cores, so our AI model is CPU-heavy.
> So, I'm dropping the sample rate down to 44 kHz for now, look at the CPU usage increases while it runs.
> Let me run it now — I'll explain what's happening while it goes."

- Trigger diagnosis

> "The AI feature has two parts.
>
> First, a rule-based classifier. It looks at Rate, Amplitude, and Beat Error,
> compares them against standard watchmaker tolerances,
> and gives you a three result — Excellent, Good, or Needs Service.
>
> Second, a local LLM runs via Ollama framework.
> the model is receiving Rate, Amplitude, and Beat Error,
> plus three extra signals — Tic/Toc asymmetry, rate jitter, and escapement variation.
>
> Before it generates anything, it does a retrieval step —
> it searches a local knowledge base we built: Witschi training materials,
> the Chronoscope X1 manual, and our own domain docs.
> We built that vector database on a Windows server and deployed it to the Pi.
> 
> it doesn't just tell you the result.
> It tells you why. What's likely causing it mechanically. And what you should do to fix it.
> That's the kind of explanation you'd normally only get from a trained watchmaker."

- Type a follow-up question
- More → Developer Info (uncheck)
- Restore Sample Rate to 96000 Hz Extended

> You can also ask follow-up questions — it remembers the conversation."

---

## 15:00 – 17:00 | Area 4 — Accuracy Verification (25 pts)

> "Now accuracy — the most important quality attribute.
> Our measurements must match a reference device.
> We're using the WeiShi No.1000 as that reference.
>
> I'm going to move the watch to the WeiShi now — and take a measurement."

- Detach watch from RPi sensor, move to WeiShi No.1000
- Start measurement on WeiShi

> "As soon as the watch is removed — the mic loses signal and a dialog appears.
> The system notices immediately, no polling, no delay."

- Place watch on WeiShi, let it run briefly, read WeiShi values
- Move watch back to RPi sensor

> "Watch is back on our system — dialog closes automatically, signal detected.
> You'll notice Rate drops for a moment — that's expected.
> Rate is a rolling average, so it takes a few beats to stabilize after reconnection.
> Give it a few seconds."

- Wait for Rate to stabilize

> "There — stable. Let me read the values."

| Metric | WeiShi No.1000 | Our TimeGrapher | Notes |
|--------|:--------------:|:---------------:|-------|
| Rate (s/day) | 11 | 11.2 | Match — Weishi shows integers only |
| Amplitude (°) | 309–321 | 282–296 | ~25° systematic offset (C-event detection) |
| Beat Error (ms) | 0.1 | 0.1 | Match |
| BPH | 21600 | 21600 | Match |

**If values match:**
> "Rate, Amplitude, and Beat Error are consistent between the two systems —
> within our target tolerance.
> This confirms that our signal processing pipeline produces accurate measurements."

**If values differ (especially Amplitude):**
> "There is a small difference — most visible in Amplitude. This is expected and understood.
>
> Rate and Beat Error both match. Those only use the A-event — the loud tic.
> Amplitude is the only metric that also needs the C-event, which is much quieter.
> A quieter signal takes longer to cross the detection threshold,
> so our detector catches it slightly late — and that pushes amplitude down.
>
> Weishi also uses a microphone, but their internal threshold and signal processing
> are not documented — so we can't rule out differences on their side either.
>
> The gap is small, consistent, and understood — not random noise, a known offset.
> Overall: Rate and Beat Error are accurate. Amplitude has a systematic offset we can explain."

---

## 17:00 – 18:00 | Area 6 — GUI Modifications (25 pts)

> "Next — GUI and real-time behavior.
> We made a few usability improvements to the interface, and I want to show two things:
> how the system reacts to acoustic noise, and that the pipeline is genuinely real-time — low latency, no dropped frames.
> Let me demonstrate both at once."

> "Now I'm playing music next to the microphone — watch for the dialog that pops up."

- Play music from phone near the microphone

> "The graphs show the disruption, and the system recovers on its own"

- Stop music

- Switch to Rate/Scope tab, tap near the microphone a few times

> "Now tapping near the microphone — handling noise.
> Bottom panel shows the noise spikes in the raw scope.
> But the top panel stays clean — the rate scatter is unaffected.
> The DSP filter chain rejects noise beats before they reach the measurement engine.
> This is also evidence of low latency and real-time. You can see the scope react immediately — the spike appears with no visible delay. We have 2ms of end-to-end latency.
>
> If you want the numbers — we have two threads, BG and DSP, running independently.
> BG captures audio. DSP detects beats and computes measurements.
> Both threads are running at the same FPS and samples-per-second.
> That means no queue building up, no dropped frames.
> We measured zero dropped audio blocks over a 10-minute session."

---

## 18:00 – 21:00 | Area 8 — Best UI Showcase (10 pts)

> "Now, the UI improvements."

**Keyboard shortcuts:**

> "The full shortcut map is in the status bar hint at all times.
> Space to start or pause. Escape to stop. Left and right arrows to navigate tabs.
> F11 for fullscreen — useful when presenting or on a small display.
> F1 opens the User Guide. Ctrl+T opens tab management. Ctrl+backslash toggles split view. Ctrl+D opens AI Diagnosis.
> Everything reachable without touching the mouse."

**About TimeGrapher:**

> "Finally — About TimeGrapher in the More menu.
> Version, build info, team.
> Small detail, but it's what a finished application looks like."

- Open More → About TimeGrapher



---

## Q&A Reference — Graph Explanations (Deep Dive)

> Use these if a professor asks for more detail on any tab.

### Trace
Rate Error (s/d) top panel — zero is perfect, positive is fast, negative is slow. Amplitude bottom panel with color bands: green 270–310° (strong), amber 220–270° (acceptable), red below 220° (needs service). Line flat and green = well regulated.

### Vario
Session-wide statistics — Min, Max, Now, sigma for both Rate and Amplitude. Arrows plotted on a scale. Wide spread between Min and Max means the watch is inconsistent — warming up or uneven mainspring tension.

### Sequence
Six positions: CH (dial up), CB (dial down), 9H (crown down), 6H (crown left), 3H (crown up), 12H (crown right). Summary rows: X = mean, D = max−min spread, DVH = vertical minus horizontal average — key for diagnosing balance and poising issues. Radar Chart on the right: amplitude per position as a polygon — a perfect circle is a perfectly balanced watch.

### Beat Scope
20ms zoom into one beat. Green dashed line = A event (tic), red dashed line = C event (tac). Distance between them relative to half-beat period = Beat Error. Strip at the bottom = last ten beat thumbnails — if shapes shift or peaks move, something is off mechanically.

### Beat Error
Top panel: Beat Error over time with green tolerance band (0–0.5ms). Purple line = rolling value. Red dots = tic offset, blue dots = toc offset. Bottom panel: same offsets vs beat number — shows whether drift is consistent or random.

### Long Term
Three stacked panels (Rate, Amplitude, Beat Error) over the full session. Bucket averaging keeps memory bounded: live for first 5 min, then 10-into-1 past 5 min, 30-into-1 past 30 min, 60-into-1 past 2 hours. Mean and sigma computed on the full raw stream, not the bucketed points — statistics stay accurate. A full-day session fits in a few hundred points per metric.

### Escapement
Zoomed view of one beat aligned to the A event (x from −2.5ms to ~11ms). Green line = A event, red line = C event, amber shaded area = measured A-to-C interval. Sigma peak and sigma onset shown at top — sub-millisecond precision across repeated measurements. Extra bumps in the quiet zone between A and C = escapement not releasing cleanly.

### Spectrogram
X = time (1 second window), Y = frequency (0–20kHz), color = loudness (yellow = loud ~−20dB, purple = quiet ~−60dB). Vertical stripes = beats — 8 per second at 28800 BPH. Wide frequency spread per stripe = signature of a sharp mechanical impact. Extra stripe between beats or smeared pattern = loose or worn part. Green bar at top = signal strength meter.

### Waveform Comparison
Three consecutive beats stacked and aligned to the A event at t=0. Blue line = A event, yellow spike = C event. t_AC = A-to-C interval per beat, used to calculate amplitude. Two x-axes: bottom in ms, top in balance wheel degrees (same window, two units). Beat 1 and 3 are Tic, Beat 2 is Toc — slight t_AC difference between them is expected. Summary line shows min/max/sigma across all beats.

### Sweep
Overlays every 250ms window (2 ticks at 28800 BPH) on top of each other. On-rate = spikes stack sharp. Running fast = spikes drift left and spread. Running slow = drift right. Persistence-oscilloscope principle — more windows = sharper stack if rate is stable.

### Filter Scope
Four DSP stages per beat cycle (125ms): Raw (HPF output, noisy), Smoothed (moving average, cleaner shape), Envelope (absolute value, energy profile), Upper Envelope (single clean positive spike for detection). Each stage removes noise. Used to verify filter chain and tune High Pass Cutoff during development.

---

## Rubric Coverage Summary

| Area | Points | Time Slot |
|------|-------:|-----------|
| Intro + UI Overview | — | 0:00 – 1:30 |
| 1 — Watch-Position Testing + 2 — Base Graph Enhancements | 5+16 | 1:30 – 6:30 |
| 1 — 11 Additional Graphs | 55 | 6:30 – 13:00 |
| 2 — AI Feature | 9 | 13:00 – 15:00 |
| 4 — Accuracy (Witschi comparison) | 25 | 15:00 – 17:00 |
| 6 — GUI Modifications | 25 | 17:00 – 18:00 |
| 4 — Latency & Real-Time Evidence | (supporting) | 18:00 – 19:00 |
| 8 — Best UI | 10 | 19:00 – 22:00 |
| Bonus — Radar Chart + Diagnosis | +15 | 22:00 – 24:00 |
| Buffer / Q&A | — | 24:00 – 25:00 |
| **Total** | **145 + 15** | |
