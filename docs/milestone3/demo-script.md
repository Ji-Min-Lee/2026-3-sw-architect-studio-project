# TimeGrapher — Final Demo Script (20 min)
**Team**: Blue Sky (Team 3) | **Date**: July 1, 2026 | **Platform**: Raspberry Pi 5

> **Setup before demo**: TimeGrapher GUI running on RPi, WeiShi No.1000 powered on with the same watch loaded, log output visible on screen.

---

## 0:00 – 0:30 | System Introduction

> "Good morning. We are Team Blue Sky.
> Let's get started."

- Confirm signal is live on Sound Print or Rate display

---

## 0:30 – 2:00 | Area 1 — Watch-Position Testing (5 pts)

> "First, Watch-Position Testing."

- Show the sensor stand with the watch mounted
- The position indicator in the status bar currently reads "CH" (Crown Horizontal)

> "A mechanical watch behaves differently depending on how it's oriented —
> crown up, crown down, dial up, and so on.
> Traditionally, the user had to manually note the position and record measurements one by one.
> We automated that.
>
> The sensor stand has an accelerometer built in.
> As I rotate the watch into each position,
> the system detects the orientation automatically
> and updates the active position label here."

- Rotate the watch to 9H (crown left)

> "Crown left — the system detected it and switched to 9H automatically.
> No button press, no dropdown."

- Rotate to 6H (crown down)

> "Crown down — 6H. Detected instantly."

- Rotate back to CH

> "Each time the position changes, the system tags the incoming measurements
> with the detected position and routes them to the Sequence tab automatically.
> We'll see the full picture across all positions in a moment."

---

## 2:00 – 7:30 | Area 1 — Additional Graph Displays (55 pts)

> "Now the additional graph displays.
> I'll navigate to each tab using the More button in the tab bar."

- Click "More" to show the full tab list, then select the first tab

---

**① Trace Display** (~30 sec)

- Switch to Trace tab

> "The Trace display draws a continuous line of Rate and Amplitude over time.
> Rate on top — if the line drifts up, the watch is running fast.
> If it slopes down, it's running slow.
> Amplitude on the bottom tracks the swing of the balance wheel.
> Think of it as the watch's heartbeat on paper."

---

**② Rate and Amplitude Stability Over Time — Vario** (~30 sec)

- Switch to Vario tab

> "Vario summarizes the session into a single table —
> Min, Max, Mean, and standard deviation for both Rate and Amplitude.
> A well-regulated watch shows a tight range.
> A wide spread means the watch is inconsistent."

---

**③ Multi-Position Sequence Display** (~30 sec)

- Switch to Sequence tab

> "Here's where the position data lands.
> Each row is a position we measured — CH, 9H, 6H, and so on.
> Rate and Amplitude are shown side by side for each.
> You can immediately see which positions are problematic
> and by how much."

---

**④ Beat-Noise Scope Display** (~45 sec)

- Switch to Beat Scope tab

> "This display shows the acoustic waveform of a single beat — zoomed in to about 20 milliseconds.
> Every mechanical watch produces two impacts per beat — we call them A and C.
> The green dashed line marks the A event — the tic.
> The red dashed line marks the C event — the tac.
> The distance between those two lines, relative to the half-beat interval,
> is what gives us Beat Error.
>
> The bottom strip shows the last ten beats as thumbnails.
> If every thumbnail looks the same shape and the two peaks land in the same spots,
> the watch is consistent.
> If the peaks shift or the waveform changes shape between beats,
> something is off mechanically.
>
> Right now you can see the green line at roughly 1.5 milliseconds
> and the red line at roughly 12 milliseconds.
> The gap between them is the measured A-to-C interval —
> our system uses that to compute amplitude and beat error."

---

**⑤ Beat Error Display and Diagnostic Trace** (~30 sec)

- Switch to Beat Error tab

> "Beat Error is the number that comes out of that A-to-C measurement.
> It tells you how evenly the tic and toc are spaced within each beat.
> A perfect watch is at zero.
> Anything under 1 millisecond is generally acceptable for a well-adjusted watch.
> The current reading is 0.2 milliseconds — well within tolerance.
> The trace below shows how it has been changing over time.
> A flat line means the watch is stable.
> A drifting trace means the beat is slowly going out of adjustment."

---

**⑥ Long-Term Performance Graph** (~30 sec)

- Switch to Long-Term tab

> "This graph is for extended sessions.
> It tracks Rate, Amplitude, and Beat Error as trends over time —
> minutes to hours.
> If Rate is slowly climbing, the mainspring is running down.
> If Amplitude drops suddenly, something changed mechanically."

---

**⑦ Escapement Analyzer and Marker-Line Display** (~30 sec)

- Switch to Escapement tab

> "The escapement releases the gear train one step at a time on every beat.
> This display marks the exact moment of each release — the A and C events —
> on a time axis, with millisecond labels.
> Uneven spacing between markers means the escapement is misfiring."

---

**⑧ Time-Frequency Spectrogram Display** (~30 sec)

- Switch to Spectrogram tab

> "The spectrogram shows which frequencies are in the acoustic signal
> and how they change over time.
> The horizontal band here is the fundamental tick frequency.
> If unexpected bands appear above it, something else is vibrating —
> a loose part, a worn gear, or external interference."

---

**⑨ Waveform Comparison Display with Timing Markers** (~30 sec)

- Switch to Waveform Comparison tab

> "This overlays multiple beat waveforms aligned to the same start point.
> If every beat looks the same shape, the watch is consistent.
> The timing markers show exactly how far apart the A and C peaks are —
> deviations here map directly to Beat Error."

---

**⑩ Scope Mode with Synchronized Sweep Display** (~30 sec)

- Switch to Sweep Scope tab

> "This is an oscilloscope-style view.
> Every beat triggers a sweep from left to right, always starting at the same edge.
> It's the most familiar way to inspect the raw waveform
> and quickly spot anything unusual in the beat shape."

---

**⑪ Scope Function with Multiple Filter Views** (~30 sec)

- Switch to Filter Scope tab

> "Finally, this shows the signal through four filter stages simultaneously —
> F0 is raw, F1 through F3 apply progressively more processing.
> You can see exactly what each filter stage is doing to the signal
> before it reaches beat detection.
> This is the diagnostic tool we used to tune our filter parameters."

---

## 7:30 – 8:30 | Area 2 — Sound Print Enhancements (8 pts)

- Switch to Sound Print tab

> "Now let me show our Sound Print improvements.
> The original Sound Print just plotted the raw acoustic signal.
> We added two things.
>
> First, a moving average window overlay —
> that bright line is the smoothed version of the signal.
> It makes it much easier to spot where the actual beat events are
> against the background noise.
>
> Second, we added noise reduction for handling noise.
> Watch this — I'll tap the sensor stand deliberately."

- Tap the sensor deliberately

> "The tap barely registers.
> But the A and C beat events are still clearly visible.
> The filter preserves the useful signal and suppresses the noise."

---

## 8:30 – 9:30 | Area 2 — Rate / Scope Enhancements (8 pts)

- Switch to Rate Scope tab

> "On the Rate and Scope displays, we added interactive controls.
> You can Start, Stop, and Pause the session at any time."

- Demonstrate Pause

> "While paused, you can navigate backward and forward through the history
> using the time-axis controls.
> You don't lose your data, and you don't need to restart the session.
>
> We also added a raw signal waveform overlay on the Rate graph —
> so you can see the actual acoustic input at the same time as the derived Rate value.
> This makes it easy to correlate signal quality with measurement quality."

---

## 9:30 – 10:30 | Area 2 — AI Feature (9 pts)

- Switch to AI Diagnosis panel

> "Our team-selected AI feature is an on-device watch diagnosis system.
>
> The idea is simple: a skilled watchmaker can look at Rate, Amplitude,
> and Beat Error and tell you whether the watch is in good condition,
> needs adjustment, or shows signs of wear.
> That takes years of experience.
> We asked — can we encode some of that into a classifier?
>
> What we built is a rule-based and machine learning hybrid classifier
> that runs entirely on this Raspberry Pi — no internet, no cloud, no Python runtime.
> It uses the ONNX format so inference is fast and portable.
>
> Let me trigger a diagnosis now."

- Trigger diagnosis

> "It classifies the watch as [Good / Needs Adjustment / Worn],
> and gives a plain-English explanation of why.
> In this case: [read out the result].
>
> Inference time is under 50 milliseconds on the RPi.
> This feature runs live, every time you take a measurement."

---

## 10:30 – 13:00 | Area 4 — Accuracy Verification (25 pts)

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

| Metric | WeiShi No.1000 | Our TimeGrapher |
|--------|:--------------:|:---------------:|
| Rate (s/day) | ___ | ___ |
| Amplitude (°) | ___ | ___ |
| Beat Error (ms) | ___ | ___ |

**If values match:**
> "Rate, Amplitude, and Beat Error are consistent between the two systems —
> within our target tolerance.
> This confirms that our signal processing pipeline produces accurate measurements."

**If values differ:**
> "There is a small difference here.
> Our analysis suggests this is due to [microphone placement distance / ambient noise attenuation /
> filter cutoff tuning].
> The WeiShi uses a contact sensor directly on the watch case —
> our system uses a non-contact microphone,
> so some signal attenuation is expected.
> The relative trend and directional reading are consistent between both systems."

**Consistency demonstration:**

> "Accuracy isn't just a single match — it needs to be repeatable.
> Here are measurements we took earlier at different points in the session."

- Show repeated measurement log or Trace display

> "The values are stable across multiple readings under the same conditions.
> A well-wound watch produces consistent output — and our system reflects that consistently."

---

## 13:00 – 15:00 | Area 6 — GUI Modifications (25 pts)

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

## 15:00 – 16:30 | Area 4 — Latency & Real-Time Evidence

> "Let me put some numbers on the real-time performance claims."

- Show log output or metrics panel

> "End-to-end latency — from the moment the microphone picks up a beat
> to the moment the GUI updates — is 2.05 milliseconds on average on the RPi.
> Our target was under 100 milliseconds. We're at 2.
>
> Dropped audio blocks at 96 kilohertz over a 10-minute session: zero.
>
> Before our threading refactor, the DSP queue wait time was 77 milliseconds.
> After separating DSP into its own thread,
> it dropped to 0.03 milliseconds — that's a 2,600x improvement.
>
> All of this runs on the Raspberry Pi 5. Not a workstation."

---

## 16:30 – 18:00 | Bonus — Radar Chart + Diagnosis Classification (+15 pts)

**Radar Chart** (~45 sec)

- Switch to Radar Chart tab

> "For the bonus features, first — the Radar Chart.
>
> After measuring the watch in multiple positions,
> this chart plots Rate and Amplitude for each position
> as a polygon on a radar grid.
> A perfectly regulated watch would produce a near-perfect circle.
> Deviations from the circle show exactly which positions are off
> and by how much.
> It's a quick, visual health check of the whole watch."

- Rotate the watch into 2–3 positions to populate the chart

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

## 18:00 – 19:30 | Area 8 — Best UI Showcase (10 pts)

> "Finally, a look at the UI improvements we made."

**Status bar — live metric colors:**

> "The status bar always shows RATE, AMP, and Beat Error live.
> We added three-level color coding — green means excellent, amber means acceptable, red means needs service.
> The color matches the AI diagnosis badge, so the user gets the same signal in two places at once."

- Point to the status bar with colors visible

**Keyboard shortcuts:**

> "The full shortcut map is in the status bar at all times.
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

## 19:30 – 20:00 | Buffer

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

## Rubric Coverage Summary

| Area | Points | Time Slot |
|------|-------:|-----------|
| 1 — 12 Graph Displays | 60 | 0:30 – 6:30 |
| 2 — Sound Print + Rate/Scope Enhancements | 16 | 6:30 – 8:30 |
| 2 — AI Feature | 9 | 8:30 – 9:30 |
| 4 — Accuracy (WeiShi comparison) | 25 | 9:30 – 12:30 |
| 6 — GUI Modifications | 25 | 12:30 – 14:30 |
| 4 — Latency & Real-Time Evidence | (supporting) | 14:30 – 16:00 |
| Bonus — Radar Chart + Diagnosis | +15 | 16:00 – 17:30 |
| 8 — Best UI | 10 | 17:30 – 18:30 |
| **Total** | **145 + 15** | |
