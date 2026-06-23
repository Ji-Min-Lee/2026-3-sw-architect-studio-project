#include "UserGuideContent.h"

namespace {

QString wrap(const QString &title, const QString &body)
{
    return QStringLiteral(
               "<html><head><style>"
               "body { font-family: Segoe UI, sans-serif; font-size: 10pt; line-height: 1.45; color: #1f2937; }"
               "h2 { color: #111827; margin-top: 0; }"
               "h3 { color: #374151; margin-bottom: 0.3em; }"
               "p { margin: 0.5em 0; }"
               "ul { margin: 0.4em 0 0.8em 1.2em; }"
               "li { margin: 0.25em 0; }"
               "table { border-collapse: collapse; margin: 0.6em 0; }"
               "td, th { border: 1px solid #d1d5db; padding: 4px 8px; text-align: left; }"
               "th { background: #f3f4f6; }"
               ".note { background: #f0f9ff; border-left: 3px solid #0284c7; padding: 6px 10px; margin: 0.8em 0; }"
               ".source { color: #6b7280; font-size: 9pt; margin-top: 1.2em; }"
               "</style></head><body>")
           + QStringLiteral("<h2>%1</h2>").arg(title.toHtmlEscaped())
           + body
           + QStringLiteral(
                 "<p class=\"source\">Source: <i>Time Grapher Project Plan (Draft).pdf</i>, "
                 "LG Software Architectures Training Program.</p>"
                 "</body></html>");
}

} // namespace

QVector<UserGuideEntry> UserGuideContent::entries()
{
    return {
        {UserGuideSection::Overview,
         QStringLiteral("Getting Started"),
         QStringLiteral("Overview"),
         wrap(QStringLiteral("Overview"),
              QStringLiteral(
                  "<p>A timegrapher listens to a mechanical watch through a microphone and analyzes "
                  "timing signals to estimate how well the watch is running.</p>"
                  "<p>The GUI is organized into three areas:</p>"
                  "<ul>"
                  "<li><b>Results bar</b> — live Rate, Amplitude, Beat Error, and BPH.</li>"
                  "<li><b>Graph tabs</b> — complementary views of the same measurement stream.</li>"
                  "<li><b>Control panel</b> — Run, Watch, Simulation, and Advanced settings.</li>"
                  "</ul>"
                  "<p>Straight, clean traces usually indicate stable performance. Jagged or wandering "
                  "lines may suggest instability or a mechanical fault.</p>"
                  "<div class=\"note\">Click the <b>Diagnosis</b> label during a session for an "
                  "AI explanation of your current readings.</div>"))},

        {UserGuideSection::CoreMeasurements,
         QStringLiteral("Getting Started"),
         QStringLiteral("Core Measurements"),
         wrap(QStringLiteral("Core Measurements"),
              QStringLiteral(
                  "<p>These values appear in the results bar and drive most graph tabs.</p>"
                  "<table>"
                  "<tr><th>Measurement</th><th>Unit</th><th>Meaning</th><th>Guideline</th></tr>"
                  "<tr><td><b>Rate</b></td><td>s/day</td>"
                  "<td>How fast or slow the watch runs vs. ideal time. "
                  "+5 s/d = gaining 5 seconds per day; &minus;12 s/d = losing 12 seconds per day.</td>"
                  "<td>Smaller magnitude is better.</td></tr>"
                  "<tr><td><b>Amplitude</b></td><td>degrees (&deg;)</td>"
                  "<td>Angular swing of the balance wheel; indicates movement health and power.</td>"
                  "<td>270&deg;&ndash;310&deg; often strong; 220&deg;&ndash;250&deg; acceptable but may suggest wear; "
                  "below 200&deg; often indicates a problem.</td></tr>"
                  "<tr><td><b>Beat Error</b></td><td>ms</td>"
                  "<td>Symmetry between tick and tock half-beats. 0 ms is ideal.</td>"
                  "<td>Under 0.6 ms generally good; higher values may need adjustment.</td></tr>"
                  "<tr><td><b>BPH</b></td><td>beats/hour</td>"
                  "<td>Beat rate of the movement (e.g. 21,600, 28,800, 36,000 BPH).</td>"
                  "<td>Must match the watch movement.</td></tr>"
                  "</table>"
                  "<h3>Acoustic events (Swiss lever escapement)</h3>"
                  "<ul>"
                  "<li><b>T1 (A)</b> — impulse pin strikes the pallet fork. Most precise; used for "
                  "Rate and Beat Error.</li>"
                  "<li><b>T2 (B)</b> — escape wheel tooth slides on the pallet stone. Irregular; "
                  "not used for measurement.</li>"
                  "<li><b>T3 (C)</b> — escape wheel locks and fork strikes banking pin. Used with T1 "
                  "to calculate Amplitude.</li>"
                  "</ul>"))},

        {UserGuideSection::RunParameters,
         QStringLiteral("Parameters"),
         QStringLiteral("Run Parameters"),
         wrap(QStringLiteral("Run Parameters"),
              QStringLiteral(
                  "<p>Controls in the <b>Run</b> section affect acquisition and display smoothing.</p>"
                  "<h3>Refresh</h3>"
                  "<p>Restores run settings to their initial values.</p>"
                  "<h3>Sample Rate</h3>"
                  "<p>Sampling rate in Hz. Higher rates give more timing resolution and can improve "
                  "event detection, but increase processing cost and memory use.</p>"
                  "<h3>Averaging Period</h3>"
                  "<p>Time window over which measurements are averaged before display. A longer period "
                  "produces steadier readings; a shorter period responds faster to changes. This "
                  "setting affects smoothed graphs such as Trace and Beat Error.</p>"
                  "<h3>Start / Pause / Stop</h3>"
                  "<ul>"
                  "<li><b>Start</b> — begin acquisition and analysis with current settings.</li>"
                  "<li><b>Pause</b> — freeze graphs for review while data continues to accumulate.</li>"
                  "<li><b>Stop</b> — stop acquisition; preserve the display for review.</li>"
                  "</ul>"
                  "<h3>Input device &amp; microphone level</h3>"
                  "<p>Select the microphone and adjust input level. Place the watch securely on the "
                  "microphone and minimize ambient noise.</p>"))},

        {UserGuideSection::WatchParameters,
         QStringLiteral("Parameters"),
         QStringLiteral("Watch Parameters"),
         wrap(QStringLiteral("Watch Parameters"),
              QStringLiteral(
                  "<h3>BPH (Beats Per Hour)</h3>"
                  "<p>Selects the nominal beat rate of the watch movement. Used in timing calculations "
                  "for Rate, Beat Error, and Amplitude. Choose a common rate or use automatic detection "
                  "when available.</p>"
                  "<h3>Lift Angle</h3>"
                  "<p>Lift angle used in the amplitude calculation. Because it depends on the movement, "
                  "select the correct value for your watch. An incorrect lift angle produces incorrect "
                  "amplitude estimates.</p>"
                  "<h3>Watch Type (Men / Women)</h3>"
                  "<p>Affects rate diagnosis thresholds. Amplitude and beat-error bands are the same for "
                  "both types.</p>"
                  "<h3>Watch positions</h3>"
                  "<p>Test positions (CH, 3H, 6H, etc.) are used with the Sequence tab to compare "
                  "positional performance.</p>"))},

        {UserGuideSection::RateScope,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Rate / Scope"),
         wrap(QStringLiteral("Rate / Scope Tab"),
              QStringLiteral(
                  "<p>Two graphs for direct inspection of timing and amplitude.</p>"
                  "<h3>Rate Error graph (upper)</h3>"
                  "<p>Shows the timing relationship between tic and tac events. Ideally the two lines "
                  "stay close together and nearly horizontal — small beat error and stable timing. "
                  "Separated lines suggest increasing beat error; both lines sloping up means running "
                  "fast; both sloping down means running slow.</p>"
                  "<h3>Amplitude graph (lower / scope)</h3>"
                  "<p>Shows balance-wheel amplitude over time. Use with the Scope scale control to "
                  "inspect individual beat waveforms and acoustic event shapes.</p>"))},

        {UserGuideSection::SoundPrint,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Sound Print"),
         wrap(QStringLiteral("Sound Print"),
              QStringLiteral(
                  "<p>Visualizes the watch acoustic signal over time.</p>"
                  "<p>The display helps you see small timing variations that may be hard to notice in "
                  "a basic trace. It supports scope-like inspection of the waveform for detailed fault "
                  "analysis and indicates the active averaging window.</p>"
                  "<p>Use this view to observe fluctuations, filter ambient noise while preserving watch "
                  "sounds, and monitor the signal during measurement.</p>"))},

        {UserGuideSection::Trace,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Trace"),
         wrap(QStringLiteral("Trace Display"),
              QStringLiteral(
                  "<p>Continuously records and displays <b>rate deviation</b> and <b>amplitude</b> over "
                  "time in real time (stacked or separate graphs).</p>"
                  "<ul>"
                  "<li>Rate is smoothed by the <b>Averaging Period</b> so short-term noise does not "
                  "obscure the trend.</li>"
                  "<li>Alerts when the watch is running late.</li>"
                  "<li>Amplitude shows whether the watch stays in a normal range (generally 270&deg;&ndash;300&deg;) "
                  "and alerts when amplitude falls outside that range.</li>"
                  "</ul>"
                  "<p>Use Trace for short-term behavior; pair with Vario or Long Term for extended "
                  "summaries.</p>"))},

        {UserGuideSection::Vario,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Vario"),
         wrap(QStringLiteral("Rate and Amplitude Stability (Vario)"),
              QStringLiteral(
                  "<p>Shows long-term stability of rate and amplitude with continuously updated "
                  "statistics:</p>"
                  "<ul>"
                  "<li>Minimum, maximum, average, and standard deviation (&sigma;)</li>"
                  "<li>Elapsed measurement time and current reading</li>"
                  "</ul>"
                  "<p>A smaller min&ndash;max rate spread indicates better stability. Average reflects overall "
                  "adjustment quality; &sigma; shows how much values fluctuate. Color cues mark measured "
                  "min/max and the average.</p>"))},

        {UserGuideSection::Sequence,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Sequence"),
         wrap(QStringLiteral("Multi-Position Sequence"),
              QStringLiteral(
                  "<p>Captures and compares measurements across up to <b>10 watch test positions</b> "
                  "in one sequence.</p>"
                  "<p>For each position the tab shows Rate, Amplitude, and Beat Error, plus summary "
                  "values:</p>"
                  "<ul>"
                  "<li><b>X</b> — mean of all test positions</li>"
                  "<li><b>D</b> — difference between largest and smallest measured value</li>"
                  "</ul>"
                  "<p>Comparisons between vertical and horizontal positions help reveal balance-wheel "
                  "unbalance and positional faults.</p>"
                  "<div class=\"note\">Workflow: select a position, measure, then capture the row before "
                  "moving to the next position.</div>"))},

        {UserGuideSection::BeatScope,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Beat Scope"),
         wrap(QStringLiteral("Beat-Noise Scope"),
              QStringLiteral(
                  "<p>Two related views for detailed inspection of beat noise shape, timing, and "
                  "repeatability — beyond summary numbers alone.</p>"
                  "<h3>Scope 1</h3>"
                  "<p>Displays alternating tick and tock beat noises. Selectable time ranges (e.g. "
                  "20 ms, 200 ms, 400 ms). Recent beats appear as strips for comparison.</p>"
                  "<h3>Scope 2</h3>"
                  "<p>Shows tic and tac beat noises on separate horizontal traces with optional "
                  "averaging (e.g. after 10 or 20 intervals) to highlight repeatability.</p>"))},

        {UserGuideSection::BeatError,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Beat Error"),
         wrap(QStringLiteral("Beat Error &amp; Diagnostic Trace"),
              QStringLiteral(
                  "<p>Presents beat error as a smoothed time series plus a diagnostic tic/toc trace.</p>"
                  "<ul>"
                  "<li>Numeric beat error should match the graphical trace.</li>"
                  "<li>Lines should stay as horizontal as possible.</li>"
                  "<li>When two lines are shown, separation beyond an acceptable range triggers an alert.</li>"
                  "<li>Excessive positive or negative slope (major fault) is highlighted.</li>"
                  "</ul>"
                  "<p>The green band (typically &le; 0.6 ms) marks generally good beat error.</p>"))},

        {UserGuideSection::LongTerm,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Long Term"),
         wrap(QStringLiteral("Long-Term Performance Graph"),
              QStringLiteral(
                  "<p>Records how Rate, Amplitude, and Beat Error change over an extended period.</p>"
                  "<p>Reveals fluctuations invisible in short tests: power-reserve decay, date-change "
                  "shocks, cyclic behaviors, and temperature drift.</p>"
                  "<p>The graph updates periodically, shows the overall average for the test period, "
                  "and indicates the range of typical variation. Update frequency may decrease as "
                  "elapsed time grows so many hours remain readable.</p>"))},

        {UserGuideSection::Escapement,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Escapement"),
         wrap(QStringLiteral("Escapement Analyzer"),
              QStringLiteral(
                  "<p>Inspects fine-grained timing within each beat: acoustic waveform with vertical "
                  "markers and millisecond labels for escapement events.</p>"
                  "<ul>"
                  "<li>Markers for A and C events with elapsed A&rarr;C time in ms.</li>"
                  "<li>Compare onset vs. peak reference points for stable timing.</li>"
                  "</ul>"
                  "<p>Use when summary graphs hide detail needed to verify event detection and amplitude "
                  "pairing (A and C must belong to the same beat).</p>"))},

        {UserGuideSection::Spectrogram,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Spectrogram"),
         wrap(QStringLiteral("Time-Frequency Spectrogram"),
              QStringLiteral(
                  "<p>Shows how acoustic energy is distributed across time (horizontal) and frequency "
                  "(vertical); color intensity is signal strength.</p>"
                  "<p>Helps identify repeating beat patterns, distinguish acoustic components, and "
                  "compare frequency bands during each tick and tock. Updates in real time; inspect the "
                  "most recent beat or a selected time window. A color scale explains relative "
                  "intensity.</p>"))},

        {UserGuideSection::Waveform,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Waveform"),
         wrap(QStringLiteral("Waveform Comparison"),
              QStringLiteral(
                  "<p>Multiple beat waveforms in aligned lanes for comparing shape, spacing, and "
                  "consistency beat-to-beat.</p>"
                  "<p>Vertical guide markers and key values (rate, beat error, BPH) overlay the signal. "
                  "Use to identify landmarks and see how waveform structure changes from one beat to "
                  "the next.</p>"))},

        {UserGuideSection::Sweep,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Sweep"),
         wrap(QStringLiteral("Scope Mode (Synchronized Sweep)"),
              QStringLiteral(
                  "<p>Displays the watch acoustic signal in real time in an oscilloscope-style sweep "
                  "view synchronized to beat timing.</p>"
                  "<p>Use for live monitoring of the raw or processed signal and to see beat-to-beat "
                  "consistency at a glance.</p>"))},

        {UserGuideSection::Filters,
         QStringLiteral("Graph Tabs"),
         QStringLiteral("Filters"),
         wrap(QStringLiteral("Scope Function (Filter Views)"),
              QStringLiteral(
                  "<p>Four filter views — <b>F0, F1, F2, F3</b> — show the same watch signal under "
                  "different processing stages.</p>"
                  "<p>Compare views to examine escapement faults, friction, and noise that appear "
                  "differently at each filter stage. Intended for advanced diagnostic interpretation "
                  "of beat waveform shape.</p>"))},
    };
}

int UserGuideContent::indexOf(UserGuideSection section)
{
    const auto all = entries();
    for (int i = 0; i < all.size(); ++i) {
        if (all[i].section == section)
            return i;
    }
    return 0;
}
