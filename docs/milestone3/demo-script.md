# TimeGrapher — Final Demo Script (20 min)
**Team**: Blue Sky (Team 3) | **Date**: July 1, 2026 | **Platform**: Raspberry Pi 5

> **Setup before demo**: TimeGrapher GUI running on RPi, WeiShi No.1000 powered on with the same watch loaded, log output visible on screen.

---

## 0:00 – 0:30 | System Introduction

> "Good morning. We are Team Blue Sky.
> Let's get started."

- Confirm signal is live on Sound Print or Rate display

---

## 0:30 – 6:30 | Area 1 — 12 Graph Displays (60 pts)

> "We implemented all 12 required graph displays.
> I'll show each one and explain what it tells the user."

---

**① Watch-Position Testing** (~30 sec)

- Switch to Watch Position tab

> "A mechanical watch runs slightly differently depending on which way it's facing.
> This display measures the watch in standard positions —
> crown up, crown down, dial up, and so on —
> and shows the Rate and Amplitude for each one.
> Watchmakers use this to adjust for positional errors."

---

**② Trace Display** (~30 sec)

- Switch to Trace tab

> "This is the Trace display.
> It draws a continuous line of how fast or slow the watch is running —
> Rate on top, Amplitude on the bottom — as time goes on.
> If the watch drifts, you see the line slope up or down.
> Think of it as the watch's heartbeat over time."

---

**③ Rate and Amplitude Stability Over Time — Vario** (~30 sec)

- Switch to Vario tab

> "This shows Min, Max, Average, and standard deviation of Rate and Amplitude
> since we started measuring.
> If those numbers are tight and stable, the watch is consistent.
> If they're spread wide, something is off."

---

**④ Multi-Position Sequence Display** (~30 sec)

- Switch to Multi-Position tab

> "This builds on position testing.
> Instead of one position at a time, it lines up all measured positions side by side
> so you can compare them at a glance.
> Up to ten positions can be compared in one view."

---

**⑤ Beat-Noise Scope Display** (~30 sec)

- Switch to Beat-Noise Scope tab

> "This display captures the raw waveform shape of each individual beat.
> The faint lines are single beats, the bright line is the running average.
> You can see if the beats look clean and consistent,
> or if there's noise contaminating the signal."

---

**⑥ Beat Error Display and Diagnostic Trace** (~30 sec)

- Switch to Beat Error tab

> "Every mechanical watch has two ticks per beat — a tic and a toc.
> Ideally they're evenly spaced.
> Beat Error measures how unequal that spacing is, in milliseconds.
> This display shows the current Beat Error as a number,
> and a trace below shows how it's been changing over time.
> A well-adjusted watch should stay close to zero."

---

**⑦ Long-Term Performance Graph** (~30 sec)

- Switch to Long-Term tab

> "This graph shows how Rate, Amplitude, and Beat Error are trending
> over a long session — minutes to hours.
> If Rate is slowly drifting in one direction, that tells you the mainspring
> is running down or the watch needs regulation."

---

**⑧ Escapement Analyzer and Marker-Line Display** (~30 sec)

- Switch to Escapement Analyzer tab

> "The escapement is the mechanism that releases the gear train one tick at a time.
> This display marks the exact moment of each A and C event on a time axis —
> with millisecond labels —
> so you can see how evenly spaced those release points are.
> Any irregularity here points to a problem with the escapement."

---

**⑨ Time-Frequency Spectrogram Display** (~30 sec)

- Switch to Spectrogram tab

> "This is a spectrogram — it shows which frequencies are present in the sound,
> and how they change over time.
> The bright horizontal bands are the main frequencies of the watch ticking.
> If unexpected bands appear, there's something vibrating that shouldn't be."

---

**⑩ Waveform Comparison Display with Timing Markers** (~30 sec)

- Switch to Waveform Comparison tab

> "This display overlays multiple beat waveforms on top of each other,
> aligned to the same start point.
> If all the beats look the same shape, the watch is consistent.
> If they look different, there's beat-to-beat variation.
> The timing markers show exactly how far apart the A and C peaks are."

---

**⑪ Scope Mode with Synchronized Sweep Display** (~30 sec)

- Switch to Sweep Scope tab

> "This works like a classic oscilloscope.
> Each beat triggers a new sweep across the screen,
> always starting from the same left edge.
> It's the easiest way to see the raw shape of every beat
> in a familiar, stable view."

---

**⑫ Scope Function with Multiple Filter Views** (~30 sec)

- Switch to Filter Scope tab

> "Finally, this display shows the signal through four different filters at once —
> F0 is the raw signal, F1 through F3 apply progressively more filtering.
> This lets you see exactly how each filter stage is shaping the signal
> before it reaches beat detection.
> It's a diagnostic tool for tuning the signal processing pipeline."

---

## 6:30 – 7:30 | Area 2 — Sound Print Enhancements (8 pts)

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

## 7:30 – 8:30 | Area 2 — Rate / Scope Enhancements (8 pts)

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

## 8:30 – 9:30 | Area 2 — AI Feature (9 pts)

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

## 9:30 – 12:30 | Area 4 — Accuracy Verification (25 pts)

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

## 16:00 – 17:30 | Bonus — Radar Chart + Diagnosis Classification (+15 pts)

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

## 17:30 – 18:30 | Area 8 — Best UI Showcase (10 pts)

> "Finally, a look at the overall user experience improvements we made."

**Help system:**

> "We added a built-in help system.
> New users — or anyone who forgets what a display means —
> can access contextual guidance without leaving the application.
> No need to look up the manual."

- Demonstrate the help feature

**Dropdown menus:**

> "Less-used settings and options are grouped into dropdown menus.
> The main interface stays clean and uncluttered during a measurement session.
> Everything is still accessible, just one click away instead of always on screen."

- Open a dropdown to show

**Tab visibility management:**

> "Finally, tabs that are rarely used are hidden by default.
> The user can choose which tabs to show — reducing the visual noise
> and making it faster to navigate to the displays that matter.
> The tab bar reorganizes to put the most-used views front and center."

- Toggle tab visibility

> "Every one of these changes is part of the main application —
> the same codebase that runs all 12 graph displays, the AI diagnosis, and the real-time processing.
> Nothing is a prototype. Everything is integrated."

---

## 18:30 – 20:00 | Buffer

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
