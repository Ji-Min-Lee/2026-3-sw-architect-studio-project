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
> dial up, dial down, crown down, crown left, and so on.
> Traditionally, the user had to manually note the position and record measurements one by one.
> We automated that.
>
> The sensor stand has an accelerometer built in.
> As I rotate the watch into each position,
> the system detects the orientation automatically
> and updates the active position label here."

- Rotate the watch to 9H (crown down)

> "Crown down — the system detected it and switched to 9H automatically.
> No button press, no dropdown."

- Rotate to 6H (crown left)

> "Crown left — 6H. Detected instantly."

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

**① Rate/Scope** (~45 sec)

- Switch to Rate/Scope tab

> "Rate/Scope is the main measurement display — two panels.
>
> The top panel plots timing offset in milliseconds for every single beat.
> Red dots are tic events, blue dots are toc events.
> The green line is a 20-beat rolling average trend.
> In the top-left corner you can see the session statistics —
> mean, standard deviation, and total beat count.
> Right now: mean −0.002 ms, σ 0.055 ms across 919 beats.
> Those numbers tell you this watch is running almost perfectly.
>
> The bottom panel is the scope view.
> It shows the raw acoustic signal in real time.
> The blue line is the rectified signal — the actual sound envelope.
> The red line is the detection trigger threshold.
> The green dashed lines mark A events, the red dashed lines mark C events.
> The arrows on the graph show the measured values directly —
> 6.8 milliseconds A-to-C interval giving 302 degrees of amplitude,
> and 125 milliseconds for the full beat interval at 28800 BPH.
> Every number you see in the status bar is derived from exactly what's on this screen."

---

**② Sound Print** (~30 sec)

- Switch to Sound Print tab

> "Sound Print is a fingerprint of the watch's acoustic pattern.
> Each dot is a detected beat event, plotted by timing.
> Green, yellow, and orange dots are A events — tic —
> colored by signal strength: strong, medium, weak.
> Blue dots are C events — tac — same three levels.
> The green horizontal line marks the A event boundary.
> The blue horizontal line marks the C event boundary — the half-period.
> The pink background band is the normal operating zone.
>
> When the dots are tightly clustered in one spot,
> the watch is producing consistent beats.
> If they scatter vertically, the timing is inconsistent.
> If a whole color disappears, the signal is too weak to detect that event reliably."

---

**③ Trace Display** (~40 sec)

- Switch to Trace tab

> "Trace shows Rate and Amplitude as continuous lines over time —
> the same measurements as Rate/Scope, but scrolling over a longer window.
>
> The top panel is Rate Error in seconds per day.
> Zero means perfect. Positive means running fast, negative means slow.
> The blue line here is flat near zero — this watch is well regulated.
> The header says 'Rate and amplitude within normal limits.'
>
> The bottom panel is Amplitude in degrees.
> The green flat line sitting at 302 degrees means the balance wheel
> is swinging consistently at a healthy angle.
> The background color bands tell you immediately how to interpret the value —
> green zone is 270 to 310 degrees: strong.
> Amber is 220 to 270: acceptable.
> Red below 220 means the mainspring is running down
> or there is friction — the watch needs service.
> Right now we are firmly in the green zone."

---

**④ Rate and Amplitude Stability — Vario** (~40 sec)

- Switch to Vario tab

> "Vario summarizes the entire session into a compact statistical view.
> The large number at the top is the elapsed session time — here, 3 minutes 3 seconds.
>
> The top half shows Rate.
> Min, Max, Now, and sigma — all shown as colored text, and plotted as arrows on the scale below.
> Blue arrows are the Min and Max, the red arrow is the current value.
> The green background means all values are within tolerance.
> Here: Min −3.5, Max 0.6, Now −0.4 seconds per day, sigma 0.3.
> The checkmark confirms it's within acceptable range.
>
> The bottom half shows Amplitude the same way.
> Right now Min, Max, and Now are all 302 degrees with sigma zero —
> perfectly stable.
>
> If you see a wide spread between Min and Max,
> the watch is behaving inconsistently —
> probably warming up, or the mainspring tension is uneven."

---

**⑤ Multi-Position Sequence Display** (~45 sec)

- Switch to Sequence tab

> "Sequence is where all the position measurements come together.
> Each row is one watch position —
> CH is dial up, CB is dial down,
> 9H is crown down, 6H is crown left, 3H is crown up, 12H is crown right.
> The columns show Rate, Beat Error, and Amplitude for each position.
> The currently active position is highlighted — right now 12H.
>
> At the bottom, three summary rows:
> X is the mean across all positions.
> D is max minus min — the positional spread.
> DVH is the difference between vertical and horizontal averages —
> a key number for diagnosing balance and poising issues.
>
> On the right, the Radar Chart plots Amplitude for each position
> as a polygon on a circular grid.
> A perfectly balanced watch would produce a near-perfect circle.
> Deviations show exactly which positions are pulling the watch off.
> The warning at the bottom tells you which position is worst
> and what it likely means mechanically."

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

**⑤ Beat Error Display and Diagnostic Trace** (~40 sec)

- Switch to Beat Error tab

> "Beat Error has two panels.
>
> The top panel shows Beat Error in milliseconds over time.
> The purple line is the rolling Beat Error value.
> The red dots are the tic timing offset, the blue dots are the toc timing offset.
> The green background band is the tolerance zone — roughly 0 to 0.5 milliseconds.
> As long as the purple line stays inside the green band, the watch is well adjusted.
> Right now the line is flat near zero, well inside tolerance.
>
> The bottom panel plots the same tic and toc offsets against beat number instead of time.
> This lets you see whether the timing is drifting beat to beat,
> or staying consistently at the same level.
> Red and blue dots both sitting on zero means the tic and toc are perfectly even."

---

**⑥ Long-Term Performance Graph** (~45 sec)

- Switch to Long Term tab

> "Long Term stacks three graphs on top of each other —
> Rate, Amplitude, and Beat Error — all over the same time axis.
> The header shows the current values at a glance:
> Rate, Amplitude, Beat Error, and the granularity mode.
>
> The top panel is Rate in seconds per day.
> The pink line shows the raw measurement, the dashed line is the mean.
> You can see it started noisy and then settled — that's normal warm-up behavior.
>
> The middle panel is Amplitude in degrees.
> The blue line fluctuates slightly around 302 degrees.
> The shaded blue band is the one-sigma range — it tells you how much the amplitude varies.
> A narrow band means the watch is consistent. A wide band means something is changing.
>
> The bottom panel is Beat Error in milliseconds.
> The green line oscillates around 0.013 milliseconds — essentially zero.
> Again the shaded band shows the variation range.
>
> All three together give you a complete picture of how the watch behaves over time,
> not just at this moment."

---

**⑦ Escapement Analyzer** (~40 sec)

- Switch to Escapement tab

> "The Escapement display zooms into a single beat and measures the A-to-C interval directly.
>
> The x-axis is time from the A event — from negative 2.5 milliseconds to about 11 milliseconds.
> The green vertical line is the A event — T1 — the tic.
> The red vertical line is the C event — T3 — the tac.
> The amber shaded area between them is the measured interval.
> Right now that interval is 6.84 milliseconds, labeled in the center.
>
> At the top you can see two sigma values —
> sigma peak is 0.012 milliseconds, sigma onset is also 0.012 milliseconds.
> That means the system measured this interval 20 times
> and the standard deviation is 0.012 milliseconds.
> Sub-millisecond precision, consistent across every beat.
>
> The waveform shows the actual acoustic signal —
> the burst at A, quiet space in between, and the spike at C.
> If that quiet space has extra bumps, the escapement is not releasing cleanly."

---

**⑧ Time-Frequency Spectrogram** (~35 sec)

- Switch to Spectrogram tab

> "The Spectrogram shows which frequencies are present in the sound
> and how they change over time — a 200 millisecond window per beat.
>
> X-axis is time in milliseconds, Y-axis is frequency from 0 to 20,000 Hz.
> The color scale on the right goes from yellow — loud — to purple — quiet.
>
> What you see here are two bright vertical bursts.
> The left burst is the A event, the right burst is the C event.
> Both events produce energy across a very wide frequency range,
> from a few hundred Hz all the way up to 20 kilohertz.
> That wide spread is characteristic of a sharp mechanical impact.
>
> The True Peak meter at the top shows −0.1 dBFS — the signal is strong and clean.
> If the watch had a loose part or a worn gear,
> you would see an extra burst appearing between the two main ones,
> or a smeared frequency pattern instead of a clean vertical line."

---

**⑨ Waveform Comparison** (~40 sec)

- Switch to Waveform tab

> "Waveform Comparison stacks the last three beats one below the other —
> all aligned to the same A event at time zero.
>
> The top x-axis shows the balance wheel angle from A in degrees —
> so you can read off where in the swing the C event happens.
> Lift angle is 52 degrees, matching our Watch Parameters setting.
>
> The blue vertical line on each panel marks the A event.
> The yellow spike is the C event — the tac.
> The t_AC value in each panel header is the measured A-to-C interval for that beat.
> Beat 1: 131.85 ms. Beat 2: 131.82 ms. Beat 3: 131.86 ms.
>
> The header at the top summarizes the range —
> min 131.82, max 131.86, sigma 0.01 milliseconds across all captured beats.
> That 0.04 millisecond spread is what produces the Beat Error reading.
>
> If the yellow spike shifts left or right between panels,
> the timing is inconsistent beat to beat.
> If the spike disappears in one panel, that beat was not detected —
> a signal quality problem."

---

**⑩ Scope Mode with Synchronized Sweep Display** (~40 sec)

- Switch to Sweep tab

> "Sweep mode shows multiple beats across one continuous time axis.
> The window is currently set to 4 ticks — so you see 4 complete beat events
> spread across 500 milliseconds.
>
> Each pair of spikes is one beat — the first spike is the A event, the second is C.
> The spacing between pairs is the beat period — here about 125 milliseconds,
> which is exactly what you expect at 28800 BPH.
>
> The key insight from this view is in the x-axis label:
> stable pattern means the watch is on rate.
> If the spikes drift left over time, the watch is running fast.
> If they drift right, it's running slow.
> Here the pattern is perfectly stable — the watch is on rate.
>
> The header confirms: Daily Rate −0.2 s/d, Amplitude 302°, Beat Error 0.02 ms."

---

**⑪ Scope Function with Multiple Filter Views** (~40 sec)

- Switch to Filters tab

> "Filter Scope shows one full beat cycle — 125 milliseconds, 3803 samples —
> through four different filter stages stacked vertically.
>
> The top panel is Raw — the unprocessed high-pass filtered signal.
> You can see the beat event as a sharp spike around 50 to 60 milliseconds.
> But it's noisy — hard to pinpoint the exact moment.
>
> The second panel is Smoothed — a moving average applied on top of the raw signal.
> The event shape is now cleaner and easier to read.
>
> The third panel is Envelope — the absolute value of the smoothed signal.
> This removes the negative half and shows only the energy profile.
>
> The fourth panel is Upper Envelope — a single clean positive spike.
> This is what the beat detector actually sees.
> One unambiguous peak, no noise, no false triggers.
>
> Each stage makes the signal easier to detect reliably.
> This view lets us verify that the filter chain is working correctly —
> and helped us tune the High Pass Cutoff parameter during development."

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

**If values differ (especially Amplitude):**
> "There is a small difference — most visible in Amplitude. This is expected and understood.
>
> Notice that Rate and Beat Error match closely between the two systems.
> That is not a coincidence — it is evidence.
>
> Rate and Beat Error are computed from A-events only:
> the strong impulse sound when the pallet fork engages the balance wheel.
> A-events are loud and sharp, so our onset detection is accurate.
> The fact that these match Weishi confirms our A-event detection is correct.
>
> Amplitude is the only metric that uses both A and C events.
> The C-event — when the pallet fork exits — produces a much weaker sound,
> because no energy is transferred at that moment.
> A weaker signal takes longer to cross our detection threshold,
> making T1 slightly longer than the true mechanical value,
> which lowers the amplitude reading.
>
> Weishi also uses a microphone, but its internal detection threshold and
> signal processing are not publicly documented,
> so differences in C-event detection tuning or formula cannot be ruled out."
>
> The offset is small and consistent — characteristic of a systematic offset, not random error.
> The root cause is the C-event detection timing, not our A-event pipeline."

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
