# EXP-08: Tab Expansion File-Change Cost

**QA**: QAS-3 (Extensibility, Modifiability) | **Risk**: TR-08 | **Status**: ✅ Concluded (2026-06-21)

---

## Results and Recommendations

All 14 graph tabs were added within the ≤ 3-file-change budget. No lower-layer files (Domain, Signal Processing, Acquisition) were modified in any batch. **QAS-3 Pass. TR-08 Resolved.**

Recommendation: maintain the `BaseGraphTab` registration pattern (ADR-006) and enforce the ≤ 3-file budget as a review gate for any future tab additions.

---

## Objective

**Question**: When a new graph tab is added to the Presentation layer, how many files outside the new tab itself must be changed?

This directly tests whether the 4-layer allowed-to-use structure and `BaseGraphTab` observer pattern (ADR-006) achieve the QAS-3 modifiability goal: new tab implementable with ≤ 3 file changes, zero references to layers below Domain.

---

## Status

Concluded

---

## Expected Outcomes

- A table of observed file-change counts per tab addition batch
- Confirmation that no Domain / Signal Processing / Acquisition file was modified
- Pass/Fail verdict for QAS-3 measure

---

## Resources Required

- Git history (sprint commits W2 S1 → W3 S1)
- `#include` trace / Dependency Structure Matrix (DSM) to verify zero cross-layer references
- Test suite: `TestAddedTabs`, `TestGraphTabs`

---

## Experiment Description

1. For each sprint batch that added tabs, record: tabs added, trigger, files changed outside the new tab files (header + source).
2. Exclude build system files and test files from the count.
3. Verify Presentation→Signal Processing and Presentation→Acquisition `#include` references = 0 via DSM.
4. Run `TestAddedTabs` and `TestGraphTabs` to confirm observer contract compliance.

---

## Duration

Completed 2026-06-21 (W3 S1 final batch confirmed).

---

## Tab Addition History

| Batch | Tabs added | Trigger | Files changed outside new tab |
|-------|-----------|---------|:----:|
| W2 S1 | 11 (baseline) | Core requirements | **2** (NewTab × N + MainWindow) |
| W2 S2 | +2 → 13 (FilterScopeTab, SweepScopeTab) | Project-plan screen requirements (Fig 7-19) | **2 each** |
| W3 S1 | +1 → **14** (RadarChartTab, bonus) | Radar/Polar chart (bonus) | **3** ¹ |

¹ RadarChartTab reads per-position data from SequenceTab directly (not via `measurementReady`). SequenceTab was modified to expose `capturedReadings()` + `sequenceUpdated()` — this is the one legitimate exception within the Presentation layer; no lower-layer file was modified.

---

## Evidence

| Measure | Target | Result |
|---------|:------:|:------:|
| Files changed per new tab (outside tab files) | ≤ 3 | ✅ 2–3 across all batches |
| Presentation → Signal Processing `#include` refs | 0 | ✅ 0 (DSM verified) |
| Presentation → Acquisition `#include` refs | 0 | ✅ 0 (DSM verified) |
| Observer contract compliance (all 14 tabs) | 100% | ✅ TestAddedTabs 20/20 · TestGraphTabs 17/17 |

---

## Links

- Full run history: [experiment-results.md](experiment-results.md)
- Analysis tool: `src/tools/analyze_log.py`
