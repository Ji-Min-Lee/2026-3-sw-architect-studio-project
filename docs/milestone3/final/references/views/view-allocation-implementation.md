# TimeGrapher test binaries and build artifacts view

**Style**: Implementation (할당 뷰 — 구현 스타일)  
**Mapping**: Software modules → File system / Build infrastructure  
**Purpose**: Shows which source modules are realized as which build artifacts in `src/build-mac/`, and what each binary's test coverage scope is. Answers: "Which modules produce which test executables at build time, and what is their verification scope?"

---

## Diagram

```
src/ (source tree)
│
├── core/
│   ├── WatchMath.*          ──────────────────► TestWatchMath        (44 tests)
│   ├── MeasurementEngine.*  ──────────────────► TestMeasurementEngine (8 tests)
│   ├── RollingAverage.*     ──────────────────► TestRollingAverage   (14 tests)
│   └── RollingLeastSquares.*──────────────────► TestRollingLeastSquares (13 tests)
│
├── ui/tabs/
│   ├── GraphTab.*           ──────────────────► TestGraphTabs        (17 tests)
│   ├── AddedTab.*           ──────────────────► TestAddedTabs        (20 tests)
│   ├── RateScopeTab.*       ──────────────────► TestRateScopeTab     ( 7 tests)
│   ├── SweepScopeTab.*      ──────────────────► TestSweepScopeTab    ( 6 tests)
│   ├── FilterScopeTab.*     ──────────────────► TestFilterScopeTab   ( 7 tests)
│   └── SoundPrintTab.*      ──────────────────► TestSoundPrintTab    ( 6 tests)
│
└── CMakeLists.txt  ◄── build script that wires all targets
                              │
                              ▼
                    src/build-mac/   (CMake release build)
                    ├── TestWatchMath
                    ├── TestMeasurementEngine
                    ├── TestRollingAverage
                    ├── TestRollingLeastSquares
                    ├── TestGraphTabs
                    ├── TestAddedTabs
                    ├── TestRateScopeTab
                    ├── TestSweepScopeTab
                    ├── TestFilterScopeTab
                    └── TestSoundPrintTab
```

**KEY**: `A.*` = source module (`.h` + `.cpp`) → `TestX` = test binary in build dir

---

## Element Catalog

### Software Elements (Modules)

| Module | Layer | Responsibility |
|--------|-------|---------------|
| `WatchMath` | Domain | Pure math: beat error, amplitude, rate equations |
| `MeasurementEngine` | Domain | Beat event stream → Rate / Amplitude / Beat Error output |
| `RollingAverage` | Domain | Sliding window average (Rate, Amplitude, Beat Error smoothing) |
| `RollingLeastSquares` | Domain | Least-squares slope estimator for long-term rate trend |
| `GraphTab` | UI | TraceDisplay / BeatErrorTrace / VarioDisplay data contract |
| `AddedTab` | UI | Additional tab data model (20 behaviors) |
| `RateScopeTab` | UI | PCM block → rate scope plot pipeline |
| `SweepScopeTab` | UI | PCM block → sweep buffer plot pipeline |
| `FilterScopeTab` | UI | f0/f1 filter output → graph pipeline |
| `SoundPrintTab` | UI | Null-safety guards for sound print rendering |

### Environmental Elements (Build Artifacts)

| Binary | Build Path | Environment |
|--------|-----------|-------------|
| `TestWatchMath` | `src/build-mac/` | macOS 14.6.1, Qt 6.11.1, Apple LLVM 16.0.0 (arm64) |
| `TestMeasurementEngine` | `src/build-mac/` | same |
| `TestRollingAverage` | `src/build-mac/` | same |
| `TestRollingLeastSquares` | `src/build-mac/` | same |
| `TestGraphTabs` | `src/build-mac/` | same |
| `TestAddedTabs` | `src/build-mac/` | same |
| `TestRateScopeTab` | `src/build-mac/` | same |
| `TestSweepScopeTab` | `src/build-mac/` | same |
| `TestFilterScopeTab` | `src/build-mac/` | same |
| `TestSoundPrintTab` | `src/build-mac/` | same |

---

## Allocation Mapping Summary

| Software Module | Allocated-to Binary | Tests | Result |
|----------------|--------------------:|------:|:------:|
| WatchMath | TestWatchMath | 44 | ✅ PASS |
| MeasurementEngine | TestMeasurementEngine | 8 | ✅ PASS |
| RollingAverage | TestRollingAverage | 14 | ✅ PASS |
| RollingLeastSquares | TestRollingLeastSquares | 13 | ✅ PASS |
| GraphTab (Trace/BeatError/Vario) | TestGraphTabs | 17 | ✅ PASS |
| AddedTab variants | TestAddedTabs | 20 | ✅ PASS |
| RateScopeTab | TestRateScopeTab | 7 | ✅ PASS |
| SweepScopeTab | TestSweepScopeTab | 6 | ✅ PASS |
| FilterScopeTab | TestFilterScopeTab | 7 | ✅ PASS |
| SoundPrintTab | TestSoundPrintTab | 6 | ✅ PASS |
| **Total** | **10 binaries** | **142** | ✅ ALL PASS |

---

## What This View Reveals

- **Layer coverage**: Domain layer (core math + measurement engine) and UI layer (tab data models) each have dedicated test binaries — no module is tested through another module's binary.
- **Visibility guard pattern**: Several UI modules required `tab.show() + processEvents()` before assertions because `redraw()` is gated on `isVisible()`. This constraint is now encoded as the allocation rule: *UI tab binaries must call `show()` before asserting plot state.*
- **Build isolation**: Each binary links only the module(s) it tests — no cross-module link-time coupling in the test layer.

---

## Related Views

- [Deployment View: Build-Deploy Pipeline](view-deployment-build-pipeline.md) — shows which node (macOS vs RPi) runs these binaries
- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — layer structure that these binaries verify does not regress

## Related References

- [ADR-008: WatchMath Module Isolation](../adr/ADR-008-watchmath-module-isolation.md) — decision that isolated domain math into independently testable modules
