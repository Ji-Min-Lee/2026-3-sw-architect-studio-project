# I-5: BPH Array Consolidation

## Summary

Replaced two identical static arrays (`ManualAutoBPH`, `SimBPH`) with a single
canonical `kBphValues[]` plus a thin `kManualAutoBPH[]` that prepends the
Auto-detect entry, eliminating a silent DRY violation that caused divergence
risk on every BPH list update.

---

## AS-IS

```cpp
// MainWindow.cpp — two arrays with 56 identical values
static int ManualAutoBPH[] = { 0,  // Auto
    3600, 6000, 7200, ..., 43200 };   // 57 entries total

static int SimBPH[] = {             // same 56 values, no leading 0
    3600, 6000, 7200, ..., 43200 };   // 56 entries total
```

`LoadBPH()` iterated `ManualAutoBPH`; `LoadSimBPH()` iterated `SimBPH`.
`SimStart()` read `SimBPH[ui->SimBPHComboBox->currentIndex()]` directly.

**Problems**

| # | Problem | Impact |
|---|---------|--------|
| 1 | 56 integer literals duplicated verbatim | Adding or removing a BPH value requires two edits; one missed = silent mismatch |
| 2 | No comment explains the relationship between the two arrays | A maintainer reading `SimBPH` cannot tell it is a subset of `ManualAutoBPH` |
| 3 | `static int` (non-`const`) allows accidental mutation | Defensive, since Qt slots can capture raw arrays by accident |

---

## TO-BE

```cpp
// MainWindow.cpp — single source of truth
static constexpr int kBphValues[] = {
    3600, 6000, 7200, ..., 43200 };   // 56 entries — canonical list

// Live/Playback combo: prepends 0 (Auto-detect) to kBphValues.
static constexpr int kManualAutoBPH[] = { 0,
    3600, 6000, 7200, ..., 43200 };   // 57 entries
```

`LoadSimBPH()` now iterates `kBphValues`; `LoadBPH()` iterates `kManualAutoBPH`.
`SimStart()` reads `kBphValues[ui->SimBPHComboBox->currentIndex()]`.

---

## Rationale

### 1. Single source of truth (DRY)

`SimBPH` was `ManualAutoBPH` with the leading `0` removed. This relationship
existed only in the developer's head; the code expressed it as two independent
arrays. Any future BPH addition (new movement frequency) or correction requires
a change in exactly one place — `kBphValues[]`.

### 2. `constexpr` prevents accidental mutation

`static int[]` at file scope is mutable. `static constexpr int[]` makes the
array a compile-time constant with no runtime storage that can be modified.
This matches the semantic intent: BPH values are fixed calibration data, not
runtime state.

### 3. Naming convention (`k` prefix)

The `k` prefix (Google C++ Style Guide convention for compile-time constants)
signals to readers that these are not runtime variables. The previous names
(`ManualAutoBPH`, `SimBPH`) gave no such signal.

---

## Files Changed

| File | Change |
|------|--------|
| `src/ui/MainWindow.cpp` | `ManualAutoBPH[]` → `kManualAutoBPH[]` (constexpr); `SimBPH[]` removed; `kBphValues[]` added as canonical source; `LoadSimBPH()` updated to iterate `kBphValues` |
