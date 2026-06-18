# I-7: DSPWorker FPS Statistics Single Source of Truth

## Summary

Removed three private member fields (`mDspFPS`, `mDspSPS`, `mDspSPF`) from
`DSPWorker` and wrote the computed values directly into `Logger::Frame`, which
was already the downstream consumer. Eliminated an intermediate copy and a
second source of truth for the same numbers.

---

## AS-IS

```cpp
// DSPWorker.h — AS-IS
class DSPWorker {
    // ...
    double mDspFPS = 0, mDspSPS = 0, mDspSPF = 0;  // intermediate store
};

// DSPWorker.cpp — AS-IS
if (now - mLastTime > 2.0) {
    double elapsedSec = now - mLastTime;
    mDspFPS = mFrameCount  / elapsedSec;  // (1) written to member
    mDspSPS = mSampleCount / elapsedSec;
    mDspSPF = mSampleCount / (double)mFrameCount;
    // ...
}

frame.fg_fps = mDspFPS;   // (2) copied to Logger::Frame
frame.fg_sps = mDspSPS;
frame.fg_spf = mDspSPF;
emit frameLogged(frame);
```

The three member fields existed solely to bridge the computation site and the
`emit` site within the same `onDataReady()` call. Between two update windows,
the members held stale values that were re-copied into every emitted frame.

**Problems**

| # | Problem | Impact |
|---|---------|--------|
| 1 | Two representations of the same value in one object — members and frame fields | Any read of `mDspFPS` outside `onDataReady()` observes a potentially stale snapshot |
| 2 | Three extra fields in `DSPWorker` with no invariant that keeps them in sync with `Logger::Frame` | Maintenance surface for a value that has one consumer |
| 3 | `Logger::Frame` is the authoritative output type; member fields are an implementation detail that leaked into the class interface (`.h`) | Readers of the header must understand which fields are transient state vs. configuration |

---

## TO-BE

```cpp
// DSPWorker.h — TO-BE: fields removed
class DSPWorker {
    // mDspFPS / mDspSPS / mDspSPF removed
};

// DSPWorker.cpp — TO-BE: write directly into frame
if (now - mLastTime > 2.0) {
    double elapsedSec = now - mLastTime;
    frame.fg_fps = mFrameCount  / elapsedSec;  // written once, to the output
    frame.fg_sps = mSampleCount / elapsedSec;
    frame.fg_spf = mSampleCount / (double)mFrameCount;
    // ...
}

emit frameLogged(frame);
```

`Logger::Frame` is the single location where DSP-thread throughput statistics
exist. Between 2-second update windows, `frame.fg_fps/sps/spf` default to 0.0
(zero-initialized struct) — the same behavior as before.

---

## Rationale

### 1. Single source of truth

The AS-IS design had two representations of the DSP thread's throughput:
the `mDspFPS/SPS/SPF` member fields (intermediate) and `frame.fg_fps/sps/spf`
(output). Both held the same value at the same point in time — the members
existed only to carry the value from one line to another within the same function.
Removing the members makes `Logger::Frame` the only representation.

### 2. Locality of change

If the statistics computation changes (e.g., switching from a 2-second window to
an exponential moving average), the AS-IS design required updating the computation
block **and** verifying the copy block. In the TO-BE design there is only one block.

### 3. Reduced class surface

Three fields removed from `DSPWorker.h` reduces the class interface and removes
a question that any reader of the header would ask: "are these fields used outside
`onDataReady()`?" The answer was no — the fields were never read by any external
caller — but that was not apparent from the header.

---

## Files Changed

| File | Change |
|------|--------|
| `src/audio/DSPWorker.h` | Removed `mDspFPS`, `mDspSPS`, `mDspSPF` fields |
| `src/audio/DSPWorker.cpp` | Computation block writes to `frame.fg_*` directly; three copy lines removed |
