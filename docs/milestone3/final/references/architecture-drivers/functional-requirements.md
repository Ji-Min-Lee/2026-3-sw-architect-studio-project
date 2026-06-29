This document contains the functional requirements elicited for the TimeGrapher system, based on the project plan and grading rubric provided by CMU MSE / LG Software Architectures Training Program. You will find the _actors_ identified and how they interact with the system in major scenarios.

# ***Actors***

- Watch Technician
- TimeGrapher system (Raspberry Pi)
- Acoustic sensor / microphone
- Simulation subsystem

<br>

# ***Watch Technician on the TimeGrapher GUI***

## Session Control
- Select input mode: Live (microphone), Playback (recorded file), or Simulation (synthetic signal)
- Adjust microphone gain to match signal level without clipping or loss of detail
- Set the sample rate (Hz) for signal acquisition and processing
- Set the averaging period for smoothing displayed measurements
- Start, stop, pause, and resume signal acquisition and analysis
- Save current recording, measurements, or display data for later review or comparison
- Restore run settings to their initial values (Refresh)

## Watch Parameter Configuration
- Select the nominal beat rate (BPH) of the watch movement, or use automatic detection
- Set the lift angle used for amplitude calculation, appropriate to the specific movement

## Simulation Parameter Configuration
- Set simulated BPH, error rate, amplitude, and beat error for synthetic signal generation
- Enable realistic simulation mode that introduces variability and noise to approximate a real watch signal

## Signal Filtering
- Set the low-pass filter cutoff to remove high-frequency noise
- Set the high-pass filter cutoff to remove low-frequency drift or background hum
- Configure the C event onset amplitude threshold used for amplitude-related measurements

## Measurement Observation (Summary Bar)
- View real-time computed measurements: rate (s/d), amplitude (degrees), beat error (ms), beat rate (bph)
- Observe whether the watch is running fast or slow relative to its nominal rate
- Identify whether amplitude is in a healthy range (typically 270°–310°)
- Identify whether beat error is within an acceptable range (below 0.6 ms is generally good)

<br>

# ***Watch Technician using the Rate/Scope Tab***

## Rate Graph
- Observe the timing relationship between tic and toc events as two lines on the Rate Error graph
- Identify beat error increase from line separation, and rate deviation from line slope direction
- Navigate backward and forward through captured data using a cursor without losing the recorded signal

## Amplitude (Scope) Graph
- Inspect the acoustic waveform with markers indicating the A event onset (green dotted line) and C event peak (red dotted line)
- Read the A-to-C interval in milliseconds displayed above the C peak
- Read the interval between consecutive A events
- Zoom in and out on the waveform for more or less detailed inspection

<br>

# ***Watch Technician using the Sound Print Tab***

## Sound Print View
- View a sample-based display of the acoustic signal organized vertically by sample position
- Observe detected A events (green dots) and C events (blue dots) aligned with the waveform
- Assess whether the event detector is tracking the signal consistently across repeated beats
- Infer watch rate trend: rising green-dot pattern indicates fast; downward trend indicates slow
- Select sample rate to control display density and timing resolution

<br>

# ***Watch Technician using Additional Graph Tabs***

## Trace Display
- View rate deviation (s/d) and amplitude plotted continuously over time as two stacked or separate graphs
- Observe short-term and long-term stability of both measurements in a single view
- Receive an alert when rate indicates the watch is running late
- Receive an alert when amplitude falls outside the normal operating range (270°–310°)
- Read a smoothed s/d line to reduce short-term fluctuation noise
- View a rolling average and extended-period average for both rate and amplitude

## Rate and Amplitude Stability Over Time (Vario Display)
- View minimum, maximum, average, and standard deviation of rate and amplitude over the measurement session
- View elapsed measurement time
- See acceptable range indicated visually (e.g., green region) and measured min/max values (e.g., blue arrows)
- Assess overall adjustment quality and measurement consistency over a longer period

## Multi-Position Sequence Display
- Initiate a measurement sequence across up to 10 standard watch test positions
- View rate, beat error, and amplitude results for each test position in a single table
- View summary values across the sequence: X (mean of all positions), D (max − min difference)
- Compare vertical and horizontal position results to identify possible balance-wheel imbalance
- Identify the active test position in the GUI at all times during measurement

## Beat-Noise Scope Display (Scope 1 & Scope 2)
- View alternating tick and tock beat noises in detail with selectable time ranges (20 ms, 200 ms, 400 ms)
- Select a prior beat strip beneath the current waveform for enlarged review
- View the signal as its absolute value when that improves readability
- Identify A and C events visually, including a C beat marker, and read the associated lift angle
- View tic and tac beat noises on two horizontal axes with a fixed 20 ms time range (Scope 2)
- Enable/disable averaging (Σ) to combine multiple beat noises and reduce random noise
- View intermediate averaging results (e.g., after 10 or 20 intervals)

## Beat Error Display and Diagnostic Trace
- View numeric values for rate, amplitude, beat error, and beats per hour alongside a corresponding graphical trace
- Interpret watch timing behavior visually: positive rate maps to positively sloped trace, negative to negative
- Receive a fault alert when the trace slope exceeds approximately ±45 degrees in magnitude
- View trace lines that remain as close to horizontal as possible under normal conditions
- Receive an alert when two trace lines diverge beyond an acceptable separation range

## Long-Term Performance Graph
- Record and display rate, amplitude, and beat error change over an extended period
- View overall average and typical variation range for the full testing period
- Observe fluctuations caused by power reserve, date change, or other cyclic behaviors
- Benefit from reduced update frequency as elapsed time increases, supporting multi-hour monitoring

## Escapement Analyzer and Marker-Line Display
- View the acoustic waveform with vertical timing markers and millisecond labels for A and C events
- Read the elapsed time between A and C events in milliseconds
- Place and reposition markers aligned with the waveform
- Compare measurement from event onset vs. event peak to determine which produces more stable timing

## Time-Frequency Spectrogram Display
- View the watch's acoustic energy distributed across time (x-axis) and frequency (y-axis)
- Use color intensity to interpret relative signal strength across the spectrogram
- Inspect the most recent beat or a selected recent time window
- Identify repeating beat patterns and compare one beat to the next
- Read a color scale or legend for interpreting signal intensity levels

## Waveform Comparison Display with Timing Markers
- View multiple beat waveforms aligned in lanes for shape, spacing, and consistency comparison
- Overlay vertical guide markers and numeric values (rate, beat error, bph) on the waveform
- Compare successive beats to identify changes in waveform structure
- Observe signal envelopes and decompose the waveform within each beat
- View degree-based or time-based reference markers to interpret amplitude and timing relationships

## Scope Mode with Synchronized Sweep Display
- View the acoustic signal in real time in a fixed sweep window resembling an oscilloscope
- Configure the sweep time as a configurable multiple of the tick interval
- Observe beat position stability: stable pattern indicates watch is near nominal rate; drifting pattern indicates fast or slow
- View reference values (rate, amplitude, beat error, nominal BPH) from the most recent timing test alongside the live waveform

## Scope Function with Multiple Filter Views (F0–F3)
- Switch among four filter views (F0, F1, F2, F3) to inspect the same watch signal under different processing
- F0: view the signal as captured, mirrored around its average value (closest to raw signal)
- F1: view a moving-average–smoothed waveform with reduced background noise
- F2: view a filter that emphasizes rising slopes to make T3 (and T2) beat features stand out
- F3: view the upper portion of the signal with emphasis on rising edges, useful for identifying T1 and T3

<br>

# ***Watch Technician using AI-Assisted Features***

## Signal Quality Assessment
- Receive a real-time classification of the current signal: good, noisy, clipped, too weak, or corrupted
- Receive user guidance hints such as "signal too noisy," "reposition watch," "microphone gain too high," or "measurement confidence low"

## Bad Data Rejection
- Have unreliable signal segments automatically detected and excluded from rate, beat error, and amplitude calculations
- Identify signal segments corrupted by speech, handling noise, poor microphone contact, clipping, or weak watch signal

## Watch Condition Classification
- Receive a classification of whether the watch is trending fast, slow, unstable, or out of beat based on recent beat patterns

<br>

# ***Watch Technician using Bonus / Advanced Features***

## Radar Chart (Watch Health Overview)
- View a radar chart summarizing measurements taken across multiple watch positions
- Assess overall watch health at a glance from multi-positional measurement data

## Diagnosis / Classification
- Receive a diagnosis or classification of watch condition based on the measured data
- Interpret likely issues or health status derived from rate, amplitude, and beat error readings

<br>

# ***TimeGrapher System (Raspberry Pi)***

## Signal Acquisition
- Capture acoustic signal from the microphone in Live mode at the configured sample rate
- Support sample rate targets: 48,000 samples/s (minimum acceptable), 96,000 samples/s (objective), 192,000 samples/s (stretch goal)
- Detect and respond to sensor or microphone unplug/replug events without requiring a full restart
- Suppress Auto Gain Control (AGC) on the audio input to prevent signal distortion

## Beat Event Detection
- Detect three acoustic events per beat: T1/A (impulse pin strikes fork), T2/B (escape wheel tooth slides), T3/C (escape wheel tooth locks)
- Use T1/A as the primary timing reference for rate and beat error calculation
- Use T1/A and T3/C together to estimate amplitude
- Filter handling noise (e.g., tapping on watch or sensor) while preserving useful A and C signal features

## Measurement Computation
- Compute rate deviation in seconds per day (s/d) from detected beat timing
- Compute beat error in milliseconds from asymmetry between tic and toc intervals
- Compute amplitude in degrees from the A-to-C interval, BPH, and lift angle
- Compute derived timing measures: DiffTicTac (tick vs. tock duration difference), DiffPeriod (short-interval beat deviation), Avg Period (accumulated deviation from start or last reset)
- Maintain internal consistency of all derived values across summary bar, graphs, and sequence displays

## Real-Time Display and Performance
- Update all graphs and the measurement summary bar in real time as data is acquired and processed
- Maintain end-to-end latency from audio capture to display update at a level acceptable for interactive diagnostic use
- Record and report: capture-to-processing latency, processing-to-display latency, total end-to-end latency (average and worst-case), dropped audio blocks, and missed beat detections
- Run all graphs concurrently within the same application without requiring stop/restart to switch views

## Resilience and Error Feedback
- Detect signal interruptions: loss of signal, missed beats, excessive ambient noise, or out-of-range readings
- Display clear status or error feedback rather than leaving the user to guess whether the watch or software is at fault
- Preserve the last useful reading when appropriate and guide the user toward recovery actions
- Degrade gracefully when signal is weak or noisy rather than producing unstable or misleading outputs

<br>

### Notes

<sup>1</sup> T1/A, T2/B, T3/C refer to the three acoustic events produced per beat by a Swiss lever escapement. A is the most timing-stable and is the primary reference for rate and beat error. C, together with A, is used for amplitude estimation.

<sup>2</sup> Standard test positions follow the NIHS 95-10 / ISO 3158 convention (CR, CU, CU(L), CU(R), CL, CD, CD(L), CD(R), DU, DD) as shown in the Witschi Chronoscope X1 G3 Instruction Manual.

<sup>3</sup> The four scope filter views (F0–F3) are based on the approach described at pascalchour.fr/mesures/chour\_rm4\_en.htm.

<sup>4</sup> AI features run on-device (Raspberry Pi) using lightweight or TinyML models, without cloud dependencies.
