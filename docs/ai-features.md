# AI Features

Two-step AI feature track built on top of the existing real-time
measurement pipeline. Both steps share the same `DiagnosisInput /
DiagnosisResult` contract so either can be swapped independently.

| Step | Module | What it does |
|------|--------|--------------|
| 1 | `WatchDiagnostics` | Rule-based classifier → live diagnosis label |
| 2 | `WatchExplainer` | On-device LLM → plain-English explanation on click |

---

## Step 1 — Rule-Based Diagnosis

A deterministic classifier that consumes Rate / Amplitude / Beat-Error
measurements and produces a coarse diagnosis label shown live in the
GUI. It is the integration point for the AI feature track: zero
inference cost, same interface a future trained model will sit behind.

### Modules

| File | Contents |
|------|----------|
| [`src/engine/WatchDiagnostics.h`](../src/engine/WatchDiagnostics.h) | `DiagnosisInput`, `DiagnosisResult`, `DiagnosisLevel`, `WatchDiagnostics::Evaluate()` |
| [`src/engine/WatchDiagnostics.cpp`](../src/engine/WatchDiagnostics.cpp) | Threshold logic |

```cpp
struct DiagnosisInput {
    double    rate_spd;        bool rate_valid;
    double    amplitude_deg;   bool amplitude_valid;
    double    beat_error_ms;   bool beat_error_valid;
    WatchType watch_type = WatchType::Men;
};

struct DiagnosisResult {
    DiagnosisLevel level;   // Unknown | Excellent | Good | NeedsService
    QString        label;   // human-readable, ready for the UI
};

class WatchDiagnostics {
public:
    DiagnosisResult Evaluate(const DiagnosisInput &input) const;
};
```

`Evaluate()` is a pure function — no state, no I/O, no Qt event
dependency. That is what makes it a safe drop-in target for a trained
model later: same inputs, same output shape, different implementation
inside.

### Watch type (Men / Women)

`MainWindow.ui` adds a `WatchTypeComboBox` inside the `WatchFrame`
panel, defaulting to **Men**. Selecting it sets `mWatchType`, which is
read into `DiagnosisInput::watch_type` on every `DisplayResults()` call.

Only rate thresholds differ by watch type; amplitude and beat-error
bands are identical (Witschi's table, p.15).

| Watch type | Excellent rate | Good rate |
|------------|---------------|-----------|
| Men (default) | −5 … +15 s/d | −10 … +10 s/d |
| Women | −5 … +25 s/d | −10 … +20 s/d |

### Diagnosis states

Evaluation order: **Excellent → Good → else NeedsService**. A watch
must clear the strict band on *all three* axes to be called Excellent.

| State | Rate (Men) | Amplitude | Beat Error | Meaning |
|-------|-----------|-----------|------------|---------|
| **Unknown** | not yet valid | not yet valid | not yet valid | Not enough beats measured yet |
| **Excellent** | −5 … +15 s/d | ≥ 270° | ≤ 0.5 ms | Witschi "watch movement ok" |
| **Good** | −10 … +10 s/d | ≥ 220° | ≤ 0.8 ms | Usable but not optimal |
| **Needs Service** | outside Good | outside Good | outside Good | Fails on at least one axis |

#### Threshold sources

| Axis | Tier | Value | Source |
|------|------|-------|--------|
| Rate | Excellent | −5 … +15 s/d | Witschi Training Course p.15, Gent's row |
| Rate | Good | −10 … +10 s/d | Project judgment: ~double the Excellent width, between "a few seconds" and "minutes/day" |
| Amplitude | Excellent | ≥ 270° | Project overview.md "strong" band (270–310°) |
| Amplitude | Good | ≥ 220° | Project overview.md "acceptable" band, corroborated by BeyondTheDial |
| Beat Error | Excellent | ≤ 0.5 ms | Witschi p.14–15 "watch movement ok" |
| Beat Error | Good | ≤ 0.8 ms | Boundary between rotatewatches "usable ≤ 1.0 ms" and BeyondTheDial "serious issue > 0.8 ms" |

### Integration point

Called once per beat inside `MainWindow::DisplayResults()`, reusing
the `Measurement` struct already published by `MeasurementEngine`:

```cpp
DiagnosisInput diagInput;
diagInput.rate_spd        = m.rateErrorSpd;   diagInput.rate_valid       = m.rateValid;
diagInput.amplitude_deg   = m.amplitudeDeg;   diagInput.amplitude_valid  = m.amplitudeValid;
diagInput.beat_error_ms   = m.beatErrorMs;    diagInput.beat_error_valid = m.beatErrorValid;
diagInput.watch_type      = mWatchType;

DiagnosisResult diagResult = mWatchDiagnostics.Evaluate(diagInput);
ui->DiagnosisLabel->setText(diagResult.label);
```

No new thread, no new timer — a handful of comparisons per beat,
negligible next to the audio DSP pipeline.

### Verification logging

Logs to console only when the diagnosis **level changes** (not every beat):

```
[WatchDiagnostics] "DIAGNOSIS: Excellent"     rate= -3.5  amplitude= 303.4  beatError= 0.004
[WatchDiagnostics] "DIAGNOSIS: Unknown"       rate= -0.3  amplitude=   0.0  beatError= 0.000
[WatchDiagnostics] "DIAGNOSIS: Needs Service" rate= -4.5  amplitude= 203.4  beatError= 0.195
[WatchDiagnostics] "DIAGNOSIS: Good"          rate=  9.9  amplitude= 226.5  beatError= 0.194
```

---

## Step 2 — On-Device LLM Explanation

When the user clicks the `DIAGNOSIS:` label, a dialog opens and asks a
locally-running LLM to explain **why** the diagnosis was reached and
**what to service**. Inference is fully on-device via
[Ollama](https://ollama.com) — no network call leaves the machine.
`WatchDiagnostics::Evaluate()` is intentionally unchanged; the LLM
adds a human-readable layer on top of the existing result.

### Architecture

```
MainWindow
  │  click on DiagnosisLabel (eventFilter → QEvent::MouseButtonPress)
  │
  ▼
DiagnosisDialog          ← popup: colored banner + progress bar + text
  │  explain(req)
  ▼
WatchExplainer           ← QObject, owns QNetworkAccessManager
  │  POST /api/chat  (stream:true, num_ctx:512, num_thread:2)
  ▼
Ollama (localhost:11434) ← fully on-device, no cloud
  │  newline-delimited JSON stream
  ▼
tokenReceived(token)     ← signal per token → dialog inserts text
explanationReady(text)   ← signal when stream finishes
```

All network I/O is async — the Qt event loop is never blocked.

### Modules

| File | Contents |
|------|----------|
| [`src/engine/WatchExplainer.h`](../src/engine/WatchExplainer.h) | `ExplainRequest` struct, `WatchExplainer` class |
| [`src/engine/WatchExplainer.cpp`](../src/engine/WatchExplainer.cpp) | HTTP POST to Ollama, streaming JSON parse, model list fetch |
| [`src/ui/DiagnosisDialog.h`](../src/ui/DiagnosisDialog.h) | Popup dialog declaration |
| [`src/ui/DiagnosisDialog.cpp`](../src/ui/DiagnosisDialog.cpp) | Dialog UI, signal wiring, error display |

### WatchExplainer API

```cpp
struct ExplainRequest {
    DiagnosisInput  input;      // raw measurements passed through from DisplayResults()
    DiagnosisResult result;     // diagnosis level + label from WatchDiagnostics
    QString         modelName;  // Ollama model (auto-selected at startup)
};

class WatchExplainer : public QObject {
signals:
    void tokenReceived(const QString &token);        // one token at a time
    void explanationReady(const QString &text);      // full text when done
    void errorOccurred(const QString &errorMsg);     // Ollama unreachable or timeout
    void modelsAvailable(const QStringList &models); // sorted by size ascending
public:
    void explain(const ExplainRequest &req);  // start inference
    void warmup(const QString &modelName);    // preload model into RAM at startup
    void checkAvailability();                 // async ping /api/tags
};
```

### Ollama request parameters

| Parameter | Value | Reason |
|-----------|-------|--------|
| `stream` | `true` | Tokens forwarded via `readyRead` as they arrive |
| `num_ctx` | `512` | Reduces KV-cache from ~1.5 GB → ~192 MB on RPi5 |
| `num_thread` | `2` | Leaves 2 of 4 RPi5 cores free for the audio/DSP pipeline |

Inference only runs when the user clicks — it does not run
continuously and does not affect the real-time measurement loop.

### Prompt

Intentionally short (~50 tokens) to keep inference fast on RPi5:

```
You are a watchmaker. A {men's|ladies'} watch timegrapher reading:
Rate {R} s/d, Amplitude {A} deg, Beat Error {E} ms. Diagnosis: {D}.
In 3 sentences: why this diagnosis, likely mechanical cause, what to service.
```

Combined with `num_ctx=512`, total inference time is ~3–5 s on RPi5
with `qwen2.5:0.5b`.

### Model selection

On startup, `checkAvailability()` fetches `/api/tags` from the local
Ollama server. Models are sorted by file size (ascending) so the
**smallest installed model is auto-selected** — fastest on RPi5. The
user can override via the **AI Model** dropdown in Misc. Parameters.

The selected model is also used for `warmup()`, which fires a
zero-prompt POST at startup to preload weights into RAM before the
first click.

#### Recommended models

| Model | Size | RPi5 speed | Notes |
|-------|------|------------|-------|
| `qwen2.5:0.5b` | 394 MB | ~15–20 tok/s | Default; best speed/quality tradeoff on RPi5 |
| `gemma3:1b` | 815 MB | ~8–10 tok/s | Slightly better English quality |
| `phi3:mini` | 2.2 GB | ~2–5 tok/s | High quality; slow on RPi5 CPU |

### Ollama setup

**Windows / x86-64:**
```
winget install Ollama.Ollama
ollama pull qwen2.5:0.5b
```

**RPi5 / ARM64:**
```bash
curl -fsSL https://ollama.com/install.sh | sh
ollama pull qwen2.5:0.5b
# systemd service registered automatically
```

Restart / logs:
```bash
sudo systemctl restart ollama
journalctl -u ollama -f
```

### Error handling

If Ollama is unreachable the dialog shows an install reminder. A
120-second timeout aborts the request if the model is still loading on
first launch.

---

## Roadmap

| Step | Status | Description |
|------|--------|-------------|
| 1 | Done | Rule-based diagnosis (`WatchDiagnostics`) |
| 2 | Done | On-device LLM explanation (`WatchExplainer` + `DiagnosisDialog`) |
| 3 | Planned | RAG — inject project PDFs and Witschi documentation as context so the model can reference project-specific thresholds when explaining a diagnosis |
