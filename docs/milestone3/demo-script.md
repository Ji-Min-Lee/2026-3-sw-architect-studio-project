# TimeGrapher — Final Demo Script (20 min)
**Team**: Blue Sky (Team 3) | **Date**: July 1, 2026 | **Platform**: Raspberry Pi 5

> **Setup before demo**: TimeGrapher GUI running on RPi, WeiShi No.1000 powered on with the same watch loaded, log output visible on screen.

---

## 0:00 – 1:00 | System Introduction + UI Overview

> "Good morning. We are Team Blue Sky.
> Let me start by launching the application."

- Launch TimeGrapher from the desktop shortcut or terminal

> "The app is now starting on the Raspberry Pi.
> First thing it does on startup — it reads the screen resolution
> and automatically sizes the window to fit the display.
> No manual resizing needed.
> Whether you're on a 7-inch touchscreen or a 1080p monitor,
> the layout adapts."

- App window appears, sized to the screen

> "And now — fullscreen mode."

- Press F11

> "There we go. Hopefully everyone in the back can see that.
> If not — I can't make the watch any louder either, so we're even."

*(pause for laughs)*

> "Alright. Let me walk through the layout.
>
> The screen is divided into two areas.
> On the left is the control panel — Run Parameters at the top,
> Watch Parameters below, and Advanced settings at the bottom.
> This is where you set the mode, sample rate, BPH, and lift angle.
>
> On the right is the graph area.
> The tab bar across the top gives you access to all 14 displays.
> Tabs that don't fit in the bar are accessible through the More button on the right.
>
> At the very top is the status bar.
> Five live values always visible: position, rate, amplitude, beat error, and BPH.
> Color coding tells you the health at a glance —
> green means within tolerance, amber is borderline, red means attention needed.
> In the top-right corner is the AI Diagnosis badge — Excellent, Good, or Needs Service.
>
> At the bottom-left, the session controls: Start, Pause, Stop.
> Space bar starts or pauses. Escape stops."

- Point to each area as you describe it
- Press Space to start the session — confirm signal is live on Rate/Scope tab

---

## 1:00 – 3:00 | Area 1 — Watch-Position Testing + Area 2 — Base Graph Enhancements

### Watch-Position Testing (5 pts)

- Show the watch on the sensor stand, status bar showing "CH"

> "A mechanical watch runs differently depending on how it's oriented.
> Dial up, dial down, crown left — each position changes how gravity acts
> on the balance wheel, which shifts the rate and amplitude.
>
> We implemented automatic position detection using amplitude tracking.
> The idea: when you flip the watch from dial-up to crown-left,
> the average amplitude shifts measurably within a few seconds.
> Our system monitors that change and infers the position.
>
> We were able to reliably distinguish two positions —
> CH, dial up, and 6H, crown left.
> These two produce a clear enough amplitude difference to detect automatically."

- Demonstrate: watch in CH, then rotate to 6H, show POS label update

> "For the other four standard positions — CB, 9H, 3H, 12H —
> the amplitude differences were too small or too noisy to distinguish reliably.
> Automation for those positions failed.
> The user still selects them manually in the Sequence tab.
>
> Two out of six is a partial result, and we're being honest about that.
> The architecture is in place — with better sensor data or a longer averaging window,
> the remaining positions could be added."

---

### Area 2 — Sound Print Enhancements (8 pts)

- Switch to Sound Print tab

> "Sound Print has two improvements over the original.
>
> First, dot health coloring.
> Every event dot is now colored by signal strength.
> A events — the tic — are colored on a green-to-yellow scale:
> green means strong, yellow-green means medium, yellow means weak.
> C events — the tac — use a blue-to-cyan scale:
> blue is strong, light blue is medium, cyan is weak.
> This immediately shows you whether both events are detected with equal confidence —
> if the C dots are consistently cyan while the A dots are green,
> the C-event signal is weaker, and amplitude accuracy may suffer.
>
> Second — beat drill-down.
> Click any column on the Sound Print display
> and a dialog opens showing the raw PCM waveform for exactly that beat.
> The title bar shows the beat number.
> Inside the plot header: beat number, duration in milliseconds, and sample count.
> The waveform is drawn in green on a dark background,
> centered on the loudest peak in that beat window.
> A red dashed vertical line marks the exact peak position.
> At the bottom of the dialog, Prev and Next buttons let you step
> through adjacent beats one at a time — both buttons disable automatically
> when you reach the beginning or end of the buffer.
> You can walk through every beat in the session and inspect the raw signal directly."

- Click one column on the Sound Print to open the waveform popup
- Point to the beat number, ms, and sample count in the plot header
- Click Prev and Next to browse adjacent beats

> "This turns Sound Print from a summary view into a beat-level diagnostic tool —
> any suspicious dot can be opened, inspected, and compared with its neighbors."

---

### Area 2 — Rate / Scope Enhancements (8 pts)

- Switch to Rate/Scope tab, point to the top-left stats box

> "Rate/Scope has three improvements.
>
> First — the statistics box in the top-left of the upper panel.
> Mean, sigma, and beat count, always visible.
> Right now: mean −0.029 ms, σ 0.023 ms, 121 beats.
> That tells you immediately how consistent this watch has been
> across the entire session — not just the last few beats.
>
> Second — the green Trend line.
> It's a 20-beat rolling average overlaid on the scatter plot.
> Individual beats bounce around, but the trend line cuts through the noise
> and shows you the actual direction the watch is drifting.
>
> Third — the orange crosshair marker.
> Click anywhere on the bottom scope panel
> and a vertical orange dashed line appears on the upper panel
> at the corresponding beat position.
> It lets you pinpoint exactly which beat you're looking at in the scope
> and cross-reference it with the rate data above."

- Click on the scope panel to demonstrate the orange marker

---

## 3:00 – 9:00 | Area 1 — Additional Graph Displays (55 pts)

> "Now the additional graph displays — eleven tabs total.
> I'll go through each one quickly."

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
> The strip at the bottom shows the last ten beats — consistency check at a glance."

---

**⑤ Beat Error** — Switch to Beat Error tab

> "Beat Error over time, with the green tolerance band.
> Red and blue dots are the Tic and Toc offsets — both near zero means the watch is well adjusted."

---

**⑥ Long Term** — Switch to Long Term tab

> "Rate, Amplitude, and Beat Error over the full session — hours if needed.
> To avoid unbounded memory growth, we use bucket averaging:
> the data is compressed automatically as the session grows —
> every measurement live for the first 5 minutes,
> then averaged 10-into-1 past 5 min, 30-into-1 past 30 min, 60-into-1 past 2 hours.
> The header shows the current granularity. Statistics stay accurate on the full raw stream."

---

**⑦ Escapement** — Switch to Escapement tab

> "Zooms into the A-to-C interval of a single beat — the amber shaded region.
> Shows the exact timing with sub-millisecond sigma across repeated measurements."

---

**⑧ Spectrogram** — Switch to Spectrogram tab

> "Watch sound split into time and frequency.
> Vertical stripes are the beats — 8 per second at 28800 BPH, each spanning the full frequency range.
> That wide spread is the fingerprint of a sharp mechanical impact.
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
> Each stage removes noise until one clean unambiguous spike reaches the beat detector."

---

## 9:00 – 10:00 | Area 2 — AI Feature (9 pts)

- Switch to AI Diagnosis panel (Ctrl+D)

> "Our team-selected AI feature is an on-device watch diagnosis system.
> It has two parts working together.
>
> The first part is a rule-based classifier.
> It evaluates Rate, Amplitude, and Beat Error against known watchmaker tolerances
> and produces one of three verdicts: Excellent, Good, or Needs Service.
> Every metric is scored independently, and all three must pass at the same band
> for the overall verdict to reach that level.
>
> The second part is a local LLM running via Ollama on this Raspberry Pi —
> no internet, no cloud, no external server.
> When the classifier produces a verdict, the LLM explains it in plain English:
> why this diagnosis, the likely mechanical cause, and what a watchmaker should check.
> The LLM receives not just the three core metrics, but also additional signals
> when available — Tic/Toc amplitude asymmetry, rate jitter, and
> escapement beat-to-beat variation — giving it richer context for the explanation.
>
> Before the LLM answers, it runs a RAG retrieval step —
> cosine similarity search over a local knowledge base
> built from the Witschi training course, the Chronoscope X1 manual,
> and our own domain documents.
> The most relevant chunks are injected into the prompt as context,
> so the explanation is grounded in actual watchmaking knowledge.
>
> Let me trigger a diagnosis now."

- Trigger diagnosis (point to verdict badge and LLM explanation text)

> "The badge shows [Excellent / Good / Needs Service] —
> and below it, the LLM streams the explanation token by token.
> You can also type a follow-up question and continue the conversation —
> the model keeps the full conversation history across turns.
>
> Everything runs locally on the RPi 5 — the classifier, the LLM, and the RAG database."

- (Optional) Type a follow-up question to demonstrate chat

> "The system prompt constrains the model to watchmaking topics only.
> If you ask something unrelated, it refuses."

---

## 10:00 – 12:30 | Area 4 — Accuracy Verification (25 pts)

> "Now the most important quality attribute: Measurement Accuracy.
>
> Our governing design goal — the reason everything else exists —
> is that our measurements must match a reference device.
> We're using the WeiShi No.1000 as that reference.
>
> Here's how we verify: the same watch, at the same time,
> measured by both systems simultaneously."

- Point to WeiShi display and TimeGrapher display side by side

> "Let me read the values."

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

## 12:30 – 14:30 | Area 6 — GUI Modifications (25 pts)

**Sensor unplug / replug detection** (~30 sec)

- Physically unplug the microphone

> "We handle hardware events gracefully.
> I've just unplugged the microphone —
> the system immediately detects the lost audio source
> and shows an alert in the status bar."

- Replug the microphone

> "Plugging it back in — the system recovers automatically.
> No restart needed. The session continues."

---

**Beat-synchronized A/C display** (~30 sec)

- Show the scope view

> "On the scope display, the A and C events —
> the tic and the toc of each beat —
> are rendered at consistent relative positions across every beat cycle.
> This makes it easy to visually compare their timing
> without the display jumping around."

---

**UI layout — dropdowns and screen space** (~30 sec)

- Show the main window, demonstrate a dropdown

> "We reorganized the UI to reduce clutter.
> Controls that are rarely needed during active measurement
> are moved into dropdown menus — out of the way, but still accessible.
> The main screen space is reserved for the signal and measurements."

---

**System health and status** (~30 sec)

- Point to the status bar

> "The status bar always shows you whether the system is ready.
> Sample rate, dropped block count, signal quality indicator —
> all visible at a glance.
> If something is wrong — the microphone is too far away,
> or the signal is too noisy for a reliable measurement —
> the system tells you before you waste time taking bad data."

---

## 14:30 – 16:00 | Area 4 — Latency & Real-Time Evidence

> "Let me put some numbers on the real-time performance claims.
> First — I'll turn on Developer Info."

- More 메뉴 → Developer Info 체크

> "This is a diagnostic overlay we built specifically for this demo —
> it's hidden by default and would be excluded from a production release.
> But it lets us show the system's internals live.
>
> The status bar now shows BG and DSP frame rate, samples per second,
> and on the Raspberry Pi — CPU and memory usage in real time.
> You can see the Pi handling audio capture, DSP, beat detection, and rendering
> all at once — without saturating the CPU.
>
> Now the latency numbers.
> End-to-end — from microphone to GUI update — is 2.05 milliseconds on average.
> Our target was under 100 milliseconds. We're at 2.
>
> Dropped audio blocks at 96 kHz over a 10-minute session: zero.
>
> Before our threading refactor, the DSP queue wait time was 77 milliseconds.
> After separating DSP into its own thread, it dropped to 0.03 milliseconds —
> a 2,600x improvement.
>
> All of this, live, on the Raspberry Pi 5."

- More 메뉴 → Developer Info 체크 해제

---

## 16:00 – 17:30 | Area 8 — Best UI Showcase (10 pts)

> "Now, the UI improvements."

**Status bar — live metric colors:**

> "The status bar always shows RATE, AMP, and Beat Error live.
> We added three-level color coding — green means excellent, amber means acceptable, red means needs service.
> The color matches the AI diagnosis badge, so the user gets the same signal in two places at once."

- Point to the status bar with colors visible

**Keyboard shortcuts:**

> "The full shortcut map is in the status bar hint at all times.
> Space to start or pause. Escape to stop. Left and right arrows to navigate tabs.
> F11 for fullscreen — useful when presenting or on a small display.
> F1 opens the User Guide. Ctrl+T opens tab management. Ctrl+backslash toggles split view. Ctrl+D opens AI Diagnosis.
> Everything reachable without touching the mouse."

- Demonstrate: Space to pause, arrow keys to switch tabs, F11 fullscreen then back

**F1 — User Guide:**

> "F1 opens the built-in help panel.
> Every graph has its own entry — what it shows, how to read it, what to watch for."

- Press F1, scroll briefly

**Ctrl+T — Manage Tabs:**

> "Ctrl+T opens tab management.
> The app starts with four tabs visible by default — Rate, Sound Print, Trace, and Vario.
> You can add or remove tabs depending on what the session needs."

- Press Ctrl+T, toggle a tab on and off

**Ctrl+\ — Split View:**

> "Ctrl+backslash toggles split view —
> two tabs side by side, both updating live.
> Trace on the left, Beat Error on the right — no switching."

- Press Ctrl+\, show two live tabs

**Ctrl+D — AI Diagnosis:**

> "Ctrl+D opens AI Diagnosis — same panel we showed earlier, one shortcut away."

- Press Ctrl+D briefly

**Record session — checkbox:**

> "Recording a session used to open a separate dialog.
> We replaced it with a simple checkbox in the Run Parameters panel —
> check it before starting, and the session is recorded automatically.
> The checkbox is disabled while a session is running so you can't accidentally change it mid-measurement."

- Show the checkbox in the left panel

**About TimeGrapher:**

> "Finally — About TimeGrapher in the More menu.
> Version, build info, team.
> Small detail, but it's what a finished application looks like."

- Open More → About TimeGrapher

> "All of this is the same integrated application running live on the Raspberry Pi."

---

## 17:30 – 19:00 | Bonus — Radar Chart + Diagnosis Classification (+15 pts)

**Radar Chart** (~45 sec)

- Switch to Radar Chart tab

> "For the bonus feature — the Radar Chart.
>
> Each position label around the edge corresponds to a watch orientation —
> CH is dial up, CB is dial down, 9H is crown down, 6H is crown left, 3H is crown up, 12H is crown right,
> and CU, CD for crown up and down.
>
> The red dots are the measured Amplitude for each position.
> The blue polygon connects them.
> The green dashed circles are the tolerance bands —
> the inner circle is the minimum acceptable Amplitude,
> the outer circle is the target.
>
> A perfectly regulated watch produces a polygon that hugs the outer green circle uniformly.
> Here you can see the polygon is asymmetric —
> the CH and CB positions are close to 300 degrees,
> but 12H and 3H are pulling inward toward 275 degrees.
> That asymmetry tells you exactly which positions are weak
> and by how much — without needing to read a table.
>
> The warning at the bottom confirms: 6 of 6 positions out of tolerance,
> worst at CH at 302 degrees.
> Positional variance suggests a poising or balance review is needed.
> That is the kind of diagnostic that used to require a trained watchmaker
> to manually compare six separate measurements."

- Point to the asymmetric shape and the warning message

---

**Diagnosis with multi-position data** (~45 sec)

- Trigger full diagnosis

> "With multi-position data loaded,
> the diagnosis classifier now has more information to work with.
> It produces a condition assessment across all positions —
> [read result: e.g., 'Rate is inconsistent between crown-up and dial-up positions,
> suggesting the balance spring may need centering.']
>
> This is the kind of insight that used to require a trained watchmaker.
> We're running it on a 80-dollar computer, offline, in under 50 milliseconds."

---

## 19:00 – 20:00 | Buffer / Q&A

- Handle evaluator questions
- Re-demonstrate any item on request
- Recovery time if anything went wrong earlier

---

## Pre-Demo Checklist

- [ ] RPi powered on, TimeGrapher GUI running and receiving signal
- [ ] Watch fully wound and seated on sensor stand
- [ ] WeiShi No.1000 powered on with the same watch loaded
- [ ] Microphone plugged in — signal visible on Sound Print
- [ ] Log output visible (terminal overlay or metrics panel)
- [ ] HDMI output confirmed on external display
- [ ] Backup laptop with compiled binary ready
- [ ] Multiple measurements already taken and visible in Trace / Long-Term for consistency demo

---

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
| Intro + UI Overview | — | 0:00 – 1:00 |
| 1 — Watch-Position Testing + 2 — Base Graph Enhancements | 5+16 | 1:00 – 3:00 |
| 1 — 11 Additional Graphs | 55 | 3:00 – 9:00 |
| 2 — AI Feature | 9 | 9:00 – 10:00 |
| 4 — Accuracy (Witschi comparison) | 25 | 10:00 – 12:30 |
| 6 — GUI Modifications | 25 | 12:30 – 14:30 |
| 4 — Latency & Real-Time Evidence | (supporting) | 14:30 – 16:00 |
| 8 — Best UI | 10 | 16:00 – 17:30 |
| Bonus — Radar Chart + Diagnosis | +15 | 17:30 – 19:00 |
| Buffer / Q&A | — | 19:00 – 20:00 |
| **Total** | **145 + 15** | |
