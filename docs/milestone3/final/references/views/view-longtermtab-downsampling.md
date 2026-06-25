# Decomposition View: LongTermTab Downsampling

This view decomposes `LongTermTab` into its internal structural elements, focusing on
the adaptive bucket aggregation strategy (`mBucketSize`) and the three-series layout.
It answers: "How does `LongTermTab` keep the plotted point count bounded over multi-day
sessions while maintaining accurate statistical overlays?"

```mermaid
classDiagram
    class LongTermTab {
        <<BaseGraphTab subclass>>
        +onMeasurement(m: Measurement)
        +reset()
        +replotAll()
        -mPlot: QCustomPlot*
        -mSummaryLabel: QLabel*
        -mRate: Series
        -mAmp: Series
        -mBeat: Series
        -mTimeElapsed: double
        -mBucketSize: int
        -addPoint(series, timeSec, value)
        -updateOverlay(series)
        -makeSeries(row, name, color): Series
        -addTolLine(series, yVal)
    }

    class Series {
        <<inner struct>>
        +graph: QCPGraph*
        +meanLine: QCPItemLine*
        +band: QCPItemRect*
        +rect: QCPAxisRect*
        +sum: double
        +sumSq: double
        +n: quint64
        +bucketSum: double
        +bucketN: int
        +addRunning(v: double)
        +mean(): double
        +sigma(): double
    }

    class BucketPolicy {
        <<logic, not a class>>
        0–5 min → bucketSize = 1
        5–30 min → bucketSize = 10
        30 min–2 hr → bucketSize = 30
        > 2 hr → bucketSize = 60
    }

    class QCustomPlot {
        +addGraph(): QCPGraph*
        +replot(mode)
    }

    class QCPGraph {
        +addData(x, y)
        +data(): QCPDataContainer*
    }

    class BaseGraphTab {
        <<abstract>>
        +onMeasurement()*
        +replotAll()*
        #mPaused: bool
    }

    class MeasurementEngine {
        <<Subject>>
        +measurementReady(m: Measurement)
    }

    class Measurement {
        +metrics.rate: optional~double~
        +metrics.amplitude: optional~double~
        +metrics.beatError: optional~double~
        +signal.pcm: PCMBlock
        +signal.samplesPerSecond: int
    }

    BaseGraphTab <|-- LongTermTab
    LongTermTab *-- Series : mRate, mAmp, mBeat
    LongTermTab *-- QCustomPlot : mPlot
    LongTermTab ..> BucketPolicy : «applies» in onMeasurement()
    Series o-- QCPGraph : graph
    QCustomPlot o-- QCPGraph : owns
    MeasurementEngine ..> Measurement : «emits»
    MeasurementEngine ..> LongTermTab : measurementReady → onMeasurement()
```

## Element Catalog

#### LongTermTab
The sole concrete class in this view. Subscribes to `MeasurementEngine::measurementReady`
via the `BaseGraphTab` observer contract. Maintains three `Series` objects (rate, amplitude,
beat error) and the shared `mBucketSize` scalar. Every `onMeasurement()` call:
1. Advances `mTimeElapsed` by the PCM block duration.
2. Calls `addPoint()` for each available metric.
3. Recomputes `mBucketSize` from `BucketPolicy`.
4. Updates the summary label.
5. Rescales and queues a repaint (guarded by `isVisible()` / `mPaused`).

#### Series (inner struct)
Dual-purpose accumulator:
- **Statistical track** (`sum`, `sumSq`, `n`): accumulates every raw value; never reset
  between buckets. Used for the running mean dotted line and ±σ band overlay.
- **Bucket track** (`bucketSum`, `bucketN`): accumulates values within one bucket window.
  When `bucketN == mBucketSize`, the average is pushed to `QCPGraph` and the bucket resets.

This separation ensures the mean/σ overlay is computed over all raw measurements while
the plotted trace is coarsened to stay within the point-count budget.

#### BucketPolicy (logical, not a class)
The four-threshold mapping from `mTimeElapsed` to `mBucketSize`, applied unconditionally
in `onMeasurement()`. The policy is monotonically non-decreasing: once coarsened, it never
returns to a finer granularity within a session (bucket transitions are one-way).

| Phase | Elapsed time | `mBucketSize` | Plot frequency |
|-------|-------------|:-------------:|:-------------:|
| Live  | 0–5 min     | 1             | every measurement |
| Coarse-1 | 5–30 min | 10           | 1 per 10 measurements |
| Coarse-2 | 30 min–2 hr | 30         | 1 per 30 measurements |
| Coarse-3 | > 2 hr   | 60            | 1 per 60 measurements |

#### QCustomPlot / QCPGraph
Third-party plotting library. `QCPGraph::addData()` appends a `(x, y)` point to an
internal `QCPDataContainer`. Render time scales linearly with the number of stored points.
All `replot()` calls from `LongTermTab` use `rpQueuedReplot` to coalesce multiple
same-frame update requests.

## Behavior

### Sequence: one measurement event processed in LongTermTab

```mermaid
sequenceDiagram
    participant ME as MeasurementEngine
    participant LT as LongTermTab
    participant S  as Series (e.g. mRate)
    participant G  as QCPGraph

    ME->>LT: onMeasurement(m)
    LT->>LT: mTimeElapsed += pcm.size() / sps
    LT->>S: addPoint(mRate, mTimeElapsed, *m.metrics.rate)
    S->>S: addRunning(value)  [always — updates sum, sumSq, n]
    S->>S: bucketSum += value; bucketN++
    alt bucketN == mBucketSize
        S->>G: addData(timeSec, bucketSum/bucketN)
        S->>S: bucketSum=0; bucketN=0
        S->>LT: updateOverlay(series)  [mean/σ refresh]
    else bucketN < mBucketSize
        note over S: point suppressed — accumulating
    end
    LT->>LT: recompute mBucketSize from BucketPolicy
    LT->>LT: update mSummaryLabel
    alt isVisible() && !mPaused
        LT->>LT: rescale axes
        LT->>LT: mPlot->replot(rpQueuedReplot)
    end
```

### State: mBucketSize transitions

```mermaid
stateDiagram-v2
    [*] --> Live : reset() / session start
    Live : mBucketSize = 1
    Coarse1 : mBucketSize = 10
    Coarse2 : mBucketSize = 30
    Coarse3 : mBucketSize = 60

    Live --> Coarse1 : mTimeElapsed > 300 s (5 min)
    Coarse1 --> Coarse2 : mTimeElapsed > 1800 s (30 min)
    Coarse2 --> Coarse3 : mTimeElapsed > 7200 s (2 hr)
    Coarse3 --> [*] : reset() called
```

Transitions are **one-way** within a session. `reset()` returns `mBucketSize` to 1.

## Related ADRs

- [ADR-007: LongTermTab Downsampling](../adr/ADR-007-longtermtab-downsampling.md) —
  rationale for time-based over point-count-based thresholds; justification of the four
  threshold values; rejected alternatives.
- [ADR-006: BaseGraphTab Observer Pattern](../adr/ADR-006-basegraphtab-observer-pattern.md) —
  `LongTermTab` inherits from `BaseGraphTab`; `onMeasurement()` is the observer hook.
- [ADR-002: R1 Lazy Rendering](../adr/ADR-002-r1-lazy-rendering.md) —
  `isVisible()` guard and `rpQueuedReplot` in `onMeasurement()` follow the R1 contract.

## Related Views

- [Decomposition View: Graph Tab](view-decomposition-graph-tab.md) — shows `LongTermTab`
  within the full 14-tab observer hierarchy alongside all other `BaseGraphTab` subclasses.
- [C&C View: DSP Pipeline](view-cc-dsp-pipeline.md) — runtime thread model; `LongTermTab`
  runs on the Qt main thread and receives `Measurement` objects via `Qt::QueuedConnection`
  from the `DSPWorker` thread.
