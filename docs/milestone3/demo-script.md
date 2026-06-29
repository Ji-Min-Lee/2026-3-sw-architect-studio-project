# TimeGrapher — Final Demo Script (20 min)
**Team**: Blue Sky (Team 3) | **Date**: July 1, 2026 | **Platform**: Raspberry Pi 5

> **Setup before demo**: TimeGrapher GUI running on RPi, WeiShi No.1000 powered on with the same watch loaded, log output visible on screen.

---

## 0:00 – 1:30 | System Introduction + UI Overview

> "Good morning. We are Team Blue Sky."

- Launch TimeGrapher, press F11

> "On startup the app reads the screen resolution and sizes itself automatically.
> F11 for fullscreen — hopefully everyone can see that.
>
> Quick layout: control panel on the left, 14 graph tabs on the right.
> Status bar at the top — Rate, Amplitude, Beat Error, BPH, live.
> Color coded: green is good, amber is borderline, red needs attention.
> AI Diagnosis badge in the top-right corner."

- Press Space to start

---

## 1:30 – 6:30 | Area 1 — Watch-Position Testing + Area 2 — Base Graph Enhancements

### Watch-Position Testing (5 pts)

- Show watch on stand, status bar showing "CH"

> "Each watch position changes how gravity acts on the balance wheel — rate and amplitude shift.
> We detect position automatically using amplitude tracking.
> Two positions work reliably — CH and 6H.
> The other four were too noisy to distinguish; the user selects those manually.
> Two out of six — partial result, honest about it."

- Rotate watch CH → 6H, show POS label update

---

### Area 2 — Sound Print Enhancements (8 pts)

- Switch to Sound Print tab

> "Two improvements. First — dot coloring by signal strength.
> A events green-to-yellow, C events blue-to-cyan.
> If C dots are cyan while A dots are green, the C-event signal is weaker —
> amplitude accuracy may suffer.
>
> Second — click any column to open the raw PCM waveform for that beat.
> Green waveform on dark background, red dashed peak marker.
> Prev and Next buttons to step through adjacent beats."

- Click a column, show waveform popup, click Prev/Next

---

### Area 2 — Rate / Scope Enhancements (8 pts)

- Switch to Rate/Scope tab

> "Three improvements.
> Stats box top-left: mean, sigma, beat count — session-wide at a glance.
> Green trend line: 20-beat rolling average cutting through scatter noise.
> Orange crosshair: click the scope panel to cross-reference that beat in the rate plot above."

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

## 13:00 – 15:00 | Area 2 — AI Feature (9 pts)

- Switch to AI Diagnosis panel (Ctrl+D)

> "Two parts. First — a rule-based classifier.
> Rate, Amplitude, Beat Error evaluated against watchmaker tolerances.
> Verdict: Excellent, Good, or Needs Service.
>
> Second — a local LLM via Ollama on this Raspberry Pi. No internet, no cloud.
> It explains the verdict in plain English — why, likely mechanical cause, what to service.
> Before answering, it runs RAG retrieval over a local knowledge base:
> Witschi training course, Chronoscope X1 manual, our own domain docs.
> The LLM also receives additional signals beyond the three core metrics —
> Tic/Toc asymmetry, rate jitter, escapement variation — for richer context.
>
> Let me trigger a diagnosis now."

- Trigger diagnosis

> "The badge shows [Excellent / Good / Needs Service].
> The explanation streams token by token.
> You can follow up with a question — conversation history is maintained.
> Ask something unrelated — it refuses. Watchmaking only."

- Type a follow-up question

---

## 15:00 – 17:00 | Area 4 — Accuracy Verification (25 pts)

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

## 17:00 – 18:00 | Area 6 — GUI Modifications (25 pts)

- Unplug microphone

> "Unplug the mic — system detects it immediately, shows alert. Plug back in — recovers automatically, no restart."

- Play music from phone near the microphone

> "Now I'm playing music next to the microphone — high ambient noise.
> The status bar flags it immediately. The graphs show the disruption.
> Stop the noise — the system recovers on its own, no intervention needed."

- Stop music

- Show scope view

> "A and C events rendered at consistent positions every beat — easy to compare timing visually."

- Point to dropdown controls

> "Rarely-used controls moved into dropdowns. Screen space reserved for signal and measurements."

- Point to status bar

> "The status bar also shows the session state at all times.
> Ready before you start. Acquiring while it collects enough beats for a diagnosis.
> Running once the diagnosis locks in. Paused when you freeze the graphs for a closer look.
> If the mic is pulled or ambient noise spikes — it tells you immediately."

---

## 18:00 – 19:00 | Area 4 — Latency & Real-Time Evidence

> "Let me show the system working in real time — not just claim it.
> First — I'll turn on Developer Info."

- More → Developer Info (check)

> "This overlay is hidden by default — built for this demo,
> and would be excluded from a production release.
>
> You can see two counters: BG and DSP.
> BG is the background audio thread — it captures raw audio from the microphone.
> DSP is the signal processing thread — it detects beats and computes measurements.
> They run independently on separate threads.
>
> The key thing to notice is that the FPS and samples-per-second of both threads match.
> That means neither thread is falling behind the other —
> audio is being captured and processed at exactly the same rate.
> No queue building up, no backlog, no dropped frames.
> The exact numbers are in our experiment report — the point here is that they're equal.
>
> Now let me show it working."

- More → Developer Info (uncheck)
- Tap near the microphone or introduce a noise event

> "Watch the graph — I'm introducing a noise event right now.
> You can see the scope react immediately — the spike appears with no visible delay.
> That's the end-to-end latency: microphone to screen update, under 100 milliseconds.
> On the Raspberry Pi 5, we measured it at 2 milliseconds.
>
> Dropped audio blocks over a 10-minute session: zero.
> Before our threading refactor, DSP wait time was 77 milliseconds.
> After — 0.03 milliseconds. The exact numbers are in our written report."

---

## 19:00 – 22:00 | Area 8 — Best UI Showcase (10 pts)

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

## 22:00 – 24:00 | Bonus — Radar Chart + Diagnosis Classification (+15 pts)

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

## 24:00 – 25:00 | Buffer / Q&A

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
