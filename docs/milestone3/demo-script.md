# TimeGrapher — Final Demo Script (20 min)
**Team**: Blue Sky (Team 3) | **Date**: July 1, 2026 | **Platform**: Raspberry Pi 5

> **Setup before demo**: TimeGrapher GUI running on RPi, WeiShi No.1000 powered on with the same watch loaded, log output visible on screen.

---

## 0:00 – 3:00 | System Introduction + UI Overview

> "Good morning. We are Team Blue Sky. Let's get started by running our application.
> When the app starts, it reads the screen resolution and sizes itself automatically.
> Let me go fullscreen — hopefully everyone can see that."

- Set Sample Rate to 96000 Hz, Mode to Extended
- Launch TimeGrapher, press F11

> "Before we start, I'm setting the sample rate to 96 kHz.
> We tried 192 kHz — it gives better resolution, but after a few minutes on the Pi it starts slowing down.
> So let's play live mode first."

> "Let me walk you through the layout quickly.
> The left side is the control panel. The right side has multiple graph tabs.
> Along the top you can see the live readings — Position, Rate, Amplitude, Beat Error, and BPH.
> And in the top-right corner, that's the AI diagnosis badge.
>
> In the control panel, we made a few usability improvements.
> For example, recording a session used to open a separate dialog.
> We replaced that with a simple checkbox — you check it before starting, and the session records automatically.
> We also added a Watch Type selector for men and women, because the rate tolerances are actually different between the two."

- Click Mode → Sim to show SimFrame

> "Let me stop the live mode for a second. If I switch to Sim, you can see that a simulation panel appears.
> That's useful for testing without a real watch attached. We'll go back to Live for today."

- Switch back to Live, click Advanced

> "There's also an Advanced section here, collapsed by default.
> Inside you can pick which AI model Ollama uses for diagnosis,
> and there's a checkbox for auto position detection — if that's on, the app detects watch orientation automatically."

- Collapse Advanced

> "And the status bar shows the session state as you go.
> It starts as Ready. Once you start measuring, it moves to Acquiring — it's collecting enough beats to confirm a diagnosis.
> Then it goes to Running once it locks in. And Paused if you freeze the graphs to take a closer look."

- Press Space to start

---

## 3:00 – 6:00 | Area 1 — Watch-Position Testing + Area 2 — Base Graph Enhancements

### Watch-Position Testing (5 pts)

- Show watch on stand, status bar showing "CH"

> "When you change the watch position, gravity acts differently on the balance wheel — so rate and amplitude shift.
> We detect the position automatically using amplitude tracking.
> Two positions work reliably: CH, which is dial up, and 6H, which is crown left.
> Let me show you."

- Rotate watch from CH to 6H, show POS label update in status bar

> "POS just switched automatically — no button pressed.
>
> The other four positions — CB, 3H, 9H, 12H — the amplitude difference between them
> was too small and too noisy to distinguish reliably with just a microphone.
> We ran experiments, tried different approaches, but couldn't get it below the noise floor.
> So for those four, the user selects the position manually from the dropdown."

- Switch to Sequence tab, select a position manually from the dropdown

> "This is the Sequence tab. Each row is a watch position.
> As measurements come in, the table fills in — Rate, Amplitude, Beat Error for each position.
> If I select a position manually here, the system starts recording data for that orientation.
>
> On the right you can see the Radar Chart.
> Each point around the edge is a position, and the polygon connects the measured amplitudes.
> A perfectly balanced watch gives you a circle.
> Where the polygon pulls inward, that position is weaker.
> So at a glance, you can see which orientations need attention — without reading through a table."

---

### Area 2 — Sound Print Enhancements (8 pts)

- Switch to Sound Print tab

> "We made two improvements here. First, dot coloring by signal strength.
> For the A-event, green means a strong signal and yellow means it's weaker.
> Second, if you click any column, you can see the raw PCM waveform for that beat.
> And there are Prev and Next buttons so you can step through neighboring beats."

- Click a column, show waveform popup, click Prev/Next

---

### Area 2 — Rate / Scope Enhancements (8 pts)

- Switch to Rate/Scope tab

> "Three improvements here.
> In the top-left corner, we added a stat box showing mean, sigma, and beat count — session-wide stats at a glance.
> Second, there's a green trend line, which is a 20-beat rolling average, so you can see the direction the rate is moving.
> Third, you can click the scope panel at the bottom to cross-reference that beat in the rate plot above.
> You can see the orange marker appearing up here whenever I click down there."

- Click scope panel to show orange marker

---

## 6:00 – 10:30 | Area 1 — Additional Graph Displays (55 pts)

> "We have twelve additional graph displays. I'll go through each one quickly."

- Click "More" to show the full tab list

---

**① Trace** — Switch to Trace tab

> "This shows Rate and Amplitude as continuous scrolling lines.
> The color bands show the tolerance zone — green means healthy, red means it needs service.
> Right now both are flat and green, so we're in good shape."

---

**② Vario** — Switch to Vario tab

> "This one gives you a session summary at a glance — Rate and Amplitude shown as Min, Max, current value, and sigma.
> The arrows on the scale show where the values land. Green background means within tolerance."

---

**③ Sequence** — Switch to Sequence tab

> "All six watch positions in one table — Rate, Amplitude, and Beat Error per position.
> The Radar Chart on the right shows amplitude balance across positions.
> A perfect circle means a perfectly balanced watch."

---

**④ Beat Scope** — Switch to Beat Scope tab

> "This zooms into the acoustic waveform of a single beat, at a 20ms window.
> The green line is the A event, the red line is C. The gap between them gives us Beat Error and Amplitude.
> The thumbnail strip at the bottom shows the last ten beats, so you can check consistency at a glance."

---

**⑤ Beat Error** — Switch to Beat Error tab

> "Beat Error plotted over time, with the green tolerance band.
> The red and blue dots are the Tic and Toc offsets — if both are close to zero, the watch is well adjusted."

---

**⑥ Long Term** — Switch to Long Term tab

> "This shows Rate, Amplitude, and Beat Error over the full session — hours if needed.
> To keep memory from growing unbounded, we compress the data automatically as the session gets longer.
> The granularity label at the top shows you whether you're looking at live data or averaged points."

---

**⑦ Escapement** — Switch to Escapement tab

> "This zooms into the A-to-C interval of a single beat, which is the yellow shaded region you can see here.
> Every measurement lands almost exactly the same spot. The variation is under a millisecond.
> That means the watch is swinging consistently, and our detector is picking it up reliably every time."

---

**⑧ Spectrogram** — Switch to Spectrogram tab

> "This splits the watch sound into time and frequency.
> The vertical stripes are the beats — at 28800 BPH, that's 8 per second, each one spanning the full frequency range.
> The green bar at the top is signal strength — green means the microphone is picking up clearly."

---

**⑨ Waveform** — Switch to Waveform tab

> "This shows three consecutive beats stacked and aligned to the A event.
> The blue line is A, the yellow spike is C — the time between them is how we calculate amplitude.
> If the sigma across all three beats is tight, the watch is swinging consistently."

---

**⑩ Sweep** — Switch to Sweep tab

> "This overlays one beat window on top of each other — the window size adjusts to BPH automatically.
> If the watch is on rate, the spikes stack sharp and narrow.
> If it's drifting, the spikes spread out. You can see here they're sharp — rate is stable."

---

**⑪ Filter Scope** — Switch to Filters tab

> "This shows the signal going through four DSP stages — Raw, Smoothed, Envelope, and Upper Envelope.
> Each stage removes noise, until one clean spike reaches the beat detector."

---

**⑫ Radar Chart** — Switch to Radar Chart tab

> "The Radar Chart shows amplitude across all six watch positions as a polygon.
> A perfectly balanced watch gives you a circle.
> Where it pulls inward, that position is weaker — and you can see exactly by how much, at a glance.
> The warning at the bottom names the worst position and what it likely means mechanically."

- Point to the asymmetric polygon and warning message

---

## 10:30 – 13:30 | Area 2 — AI Feature (9 pts)

- More → Developer Info (check)

> "Now let me show the AI features. I'm turning on Developer Info so you can watch the CPU usage while it runs.
> By the way — this overlay is hidden by default. It's just for this demo, and wouldn't be in a production release.
>
> One constraint to know upfront. We wanted to use a more powerful cloud model, but the project required all inference to run locally.
> The Pi 5 has no GPU, no NPU — just four CPU cores.
> So the model is CPU-heavy. You might notice the graphs update a little slower while it's running — that's the Pi sharing resources between audio processing and the LLM.
> Watch the CPU climb as it goes."

- Switch to AI Diagnosis panel (Ctrl+D)

> "Let me run it now — I'll explain what's happening while it goes
>
> The AI feature has two parts.
>
> First, a rule-based classifier. It looks at Rate, Amplitude, and Beat Error,
> compares them against standard watchmaker tolerances,
> and gives you a result — Excellent, Good, or Needs Service.
>
> Second, we have a local LLM running via Ollama framework.
> Before it generates anything, it uses RAG. We built a vector DB on our server and deploy it to the PI.
> 
> The answer from LLM doesn't just tell you the result.
> It tells you why. What's likely causing it mechanically. And what you should do to fix it.
> That's the kind of explanation you'd normally only get from a trained watchmaker."

- Type a follow-up question

> "You can also ask follow-up questions and it remembers the conversation."

---

## 13:30 – 16:30 | Area 4 — Accuracy Verification (25 pts)

> "Now accuracy — the most important quality attribute for us.
> Our measurements need to match a reference device.
> We're using the WeiShi No.1000 as that reference — it's powered on over there with the same watch loaded.
>
> I'm going to move the watch to the WeiShi now and take a measurement."

- Detach watch from RPi sensor, move to WeiShi No.1000
- Start measurement on WeiShi

> "As soon as I removed the watch, the mic lost signal and a dialog appeared.
> The system noticed immediately — no polling, no delay."

- Place watch on WeiShi, let it run briefly, read WeiShi values
- Move watch back to RPi sensor

> "Watch is back on our system. The dialog closes automatically and the signal is detected.
> You'll notice Rate drops for a moment — that's expected.
> Rate is a rolling average, so it needs a few beats to stabilize after reconnection.
> Let's give it a few seconds."

- Wait for Rate to stabilize

> "There — stable now. Let me read the values."

| Metric | WeiShi No.1000 | Our TimeGrapher | Notes |
|--------|:--------------:|:---------------:|-------|
| Rate (s/day) | 11 | 11.2 | Match — Weishi shows integers only |
| Amplitude (°) | 309–321 | 282–296 | ~25° systematic offset (C-event detection) |
| Beat Error (ms) | 0.1 | 0.1 | Match |
| BPH | 21600 | 21600 | Match |

**If values match:**
> "Rate, Amplitude, and Beat Error are consistent between the two systems, within our target tolerance.
> That confirms our signal processing pipeline is producing accurate measurements."

**If values differ (especially Amplitude):**
> "There's a small difference, most visible in Amplitude. This is expected and we understand why.
> Let me switch to Beat Scope to explain."

- Switch to Beat Scope tab, point to green and red markers

> "Rate and Beat Error only use A-to-A intervals — the time between tic events.
> So even if our A detection is slightly delayed, that delay cancels out across beats.
> Rate and Beat Error stay accurate.
>
> Amplitude is different. It uses the A-to-C interval — the gap between these two markers here.
> If A is detected even slightly late, that delay doesn't cancel — the gap comes out shorter,
> and amplitude reads lower as a result.
>
> WeiShi also uses a microphone, but their detection threshold isn't documented,
> so there may be differences on their end too.
>
> The gap is small, consistent, and understood — it's not random noise, it's a known offset.
> Rate and Beat Error are accurate. Amplitude has a systematic offset we can explain."

---

## 16:30 – 18:30 | Area 6 — GUI Modifications (25 pts)

> "Next — GUI and real-time behavior.
> We made a few usability improvements, and I want to show two things:
> how the system reacts to acoustic noise, and that the pipeline is genuinely real-time — low latency, no dropped frames.
> Let me show both at once."

> "I'm playing music next to the microphone now — watch for the dialog that pops up."

- Play music from phone near the microphone

> "You can see the graphs show the disruption. And when I stop, the system recovers on its own — no intervention needed."

- Stop music

- Switch to Rate/Scope tab, tap near the microphone a few times

> "Now I'm tapping near the microphone — that's handling noise.
> The bottom panel shows the noise spikes in the raw scope.
> But the top panel stays clean — the rate scatter is completely unaffected.
> The DSP filter chain is rejecting those noise beats before they reach the measurement engine.
>
> And this is also real-time evidence. You can see the scope react immediately when I tap — the spike appears with no visible delay.
> We measured 2ms of end-to-end latency on the Pi 5.
>
> In terms of architecture — we have two threads running independently: BG and DSP.
> BG captures audio. DSP detects beats and computes measurements.
> Both threads run at the same FPS and samples-per-second — that means no queue building up, no dropped frames.
> Over a 10-minute session, we measured zero dropped audio blocks."

---

## 18:30 – 19:30 | Area 8 — Best UI Showcase (10 pts)

> "Now, some of the UI improvements."

**Keyboard shortcuts:**

> "The full shortcut map is always visible in the status bar in bottom-right.
> Space to start or pause. Escape to stop. Arrow keys to navigate between tabs.
> F11 for fullscreen. F1 opens the User Guide. Ctrl+backslash toggles split view. Ctrl+D opens AI Diagnosis.
> Everything is reachable without touching the mouse."

**About TimeGrapher:**

> "And finally, we have an About page in the More menu.
> It shows the version, build info, and team name.
> It's a small thing, but it's the kind of detail that makes it feel like a finished product."

- Open More → About TimeGrapher

> "That's our demo. Thank you."

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
Overlays one beat window on top of each other — window size adjusts to BPH automatically. On-rate = spikes stack sharp. Running fast = spikes drift left and spread. Running slow = drift right. Persistence-oscilloscope principle — more windows = sharper stack if rate is stable.
