# Lessons Learned

---

## What Went Right

**Decisions backed by evidence, not opinion**  
Every architectural decision was tied to a measurable experiment. The team never argued about whether a design was "better" — they ran a test and showed the numbers. This discipline made the architecture defensible and the reviews fast.

**Parallel team structure accelerated delivery**  
The team split into a development sub-team and an experiment sub-team working simultaneously. While one group implemented new features, the other ran experiments and validated architecture decisions. This agile structure prevented the bottleneck of waiting for implementation before measurement could begin.

**Architectural clarity enables fast fixes**  
The most impactful change in the project — moving DSP off the main thread — was completed quickly because the architecture already had a clean boundary between audio capture, DSP, and rendering. Good structure made the hard fix easy.

**Extensibility proved, not assumed**  
The Observer Pattern for graph tabs was not just a design choice — it was verified empirically. Adding a new tab required touching ≤ 3 files with zero signal processing knowledge. The architecture delivered on its promise because the team measured it.

**AI as a development multiplier**  
AI tools (Claude Code) accelerated the parts of engineering that are skill-intensive but time-consuming: logging instrumentation, data analysis scripts, debugging, and documentation. The team moved faster not by cutting corners, but by offloading mechanical work.

---

## What Went Wrong

**Requirements and grading criteria were not reviewed thoroughly enough upfront**  
Some features and requirements were discovered late — after implementation had already started in a different direction. Had the team reviewed the final demo guidelines and grading rubric more carefully at the beginning, priorities would have been set differently and late-stage scrambles could have been avoided.

**Performance assumptions don't survive real hardware**  
The system ran well on both macOS and Windows development machines. On the target RPi it failed 43 % of deadlines. Platform differences — thermal throttling, scheduler behavior, single-core saturation — are invisible until you test on the actual target. Passing on multiple dev machines is still not evidence of real-time correctness on embedded hardware.

**Misunderstanding a quality attribute wastes significant effort**  
The team initially misclassified "accuracy" as a standalone quality attribute equal in weight to real-time performance and latency. This led to misdirected documentation and experiment planning before the misunderstanding was caught and corrected. Clarifying what each quality attribute truly means — and how it ranks — must happen at the start, not after weeks of work.

---

## What We Would Have Done Differently

**Read the rubric before writing the first line of code**  
Grading criteria and demo requirements define what "done" means. Reviewing them thoroughly at kickoff — not just at the end — would have aligned the team's priorities from day one.

**Start on target hardware, not the dev machine**  
Real-time performance risk only exists on the real platform. Testing on RPi from week 1 would have surfaced architectural problems earlier, when the cost to fix them was low.

**Treat interface contracts as first-class artifacts**  
Format requirements and coordinate conventions were buried in source code, not surfaced as documented contracts. Any assumption that can cause silent failure should be written down, validated at the boundary, and shared with everyone who touches that interface.
