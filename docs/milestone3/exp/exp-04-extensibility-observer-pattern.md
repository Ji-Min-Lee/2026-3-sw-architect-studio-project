# Experiment 4: Observer Pattern Compliance — Tab Extension Cost Measurement

## Results and recommendations

**Outcome: ✅ Pass** — All 14 tabs implemented with ≤ 3 file changes each. Zero layer violations. DSM boundary check clean.

The BaseGraphTab observer pattern cleanly isolates the Presentation layer from Signal Processing and Acquisition. `MeasurementEngine` has zero knowledge of any tab. Adding a new graph tab requires only: one header, one source file, and one registration call.

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab | ≤ 3 | ✅ 2–3 (header + source + registration) |
| Signal Processing / Acquisition refs from Presentation | 0 | ✅ 0 — DSM verified |
| Observer contract compliance (all 14 tabs) | 100 % | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

## Objective

Verify that the BaseGraphTab observer pattern allows adding a new graph tab with ≤ 3 file changes and zero references from the Presentation layer to Signal Processing or Acquisition layers, confirming QAS-3 (Extensibility / Modifiability).

Technical question: What is the actual file-change cost of adding one new graph tab, and does the observer pattern eliminate cross-layer dependencies?

## Status

Concluded

## Expected outcomes

- File change count per new tab (header + source + registration)
- DSM (Dependency Structure Matrix) showing zero Presentation → Signal Processing / Acquisition references
- Unit test results confirming observer contract across all 14 tabs

## Resources required

- TimeChecker source tree (`src/tabs/`)
- Unit test suite (`src/tests/`)
- DSM analysis tool (or manual inspection of `#include` chains)
- ~0.5 person-days

## Experiment description

1. Select one new tab not yet implemented; count the files touched to add it
2. Run `src/tests/` — verify `TestAddedTabs` and `TestGraphTabs` pass
3. Run DSM analysis: confirm no `#include` from `tabs/` into `SignalProcessing/` or `Acquisition/`
4. Repeat for a sample of the 14 tabs to confirm consistency

## Duration

Completed 2026-06-21.

## Links and references

- [QAS-3: Extensibility / Modifiability](../final/references/qa/qas-3-extensibility-modifiability.md)
- [ADR-005: IAudioSource Dependency Inversion](../final/references/adr/ADR-005-p1-iaudiosource-dependency-inversion.md)
- [ADR-006: BaseGraphTab Observer Pattern](../final/references/adr/ADR-006-basegraphtab-observer-pattern.md)
- [experiment-results.md](../experiment-results.md) — verification record and evidence table
