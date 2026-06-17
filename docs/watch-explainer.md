# Watch Explainer (AI Feature — Step 2: On-Device LLM Explanation)

When the user clicks the `DIAGNOSIS:` label in the main window, a
dialog opens and asks a locally-running LLM to explain **why** the
current diagnosis was reached and **what to service**. Inference is
fully on-device via [Ollama](https://ollama.com) — no network call
leaves the machine.

---

## Why this exists

Step 1 (`WatchDiagnostics`) tells the user *what* the diagnosis is.
Step 2 answers *why* and *what to do about it* in plain English,
without requiring the user to look up watchmaking references manually.
The rule-based classifier (`WatchDiagnostics::Evaluate()`) is
intentionally left unchanged — the LLM adds a human-readable
explanation layer on top of the existing result, not a replacement for
the rules.

---

## Architecture

```
MainWindow
  │  click on DiagnosisLabel (eventFilter)
  │
  ▼
DiagnosisDialog          ← popup dialog, shows colored banner + text
  │  explain(req)
  ▼
WatchExplainer           ← QObject, owns QNetworkAccessManager
  │  POST /api/chat (stream:true, num_ctx:512, num_thread:2)
  ▼
Ollama (localhost:11434) ← fully on-device, no cloud
  │  newline-delimited JSON stream
  ▼
tokenReceived(token)     ← signal per token → dialog inserts text
explanationReady(text)   ← signal when stream finishes
```

All network I/O is async — the Qt event loop is never blocked.

---

## Modules

| File | Contents |
|------|----------|
| [`src/engine/WatchExplainer.h`](../src/engine/WatchExplainer.h) | `ExplainRequest` struct, `WatchExplainer` class |
| [`src/engine/WatchExplainer.cpp`](../src/engine/WatchExplainer.cpp) | HTTP POST to Ollama, streaming JSON parse, model list fetch |
| [`src/ui/DiagnosisDialog.h`](../src/ui/DiagnosisDialog.h) | Popup dialog declaration |
| [`src/ui/DiagnosisDialog.cpp`](../src/ui/DiagnosisDialog.cpp) | Dialog UI, signal wiring, error display |

---

## WatchExplainer API

```cpp
struct ExplainRequest {
    DiagnosisInput  input;      // raw measurements (rate, amplitude, beat error)
    DiagnosisResult result;     // diagnosis level + label from WatchDiagnostics
    QString         modelName;  // Ollama model to use (auto-selected at startup)
};

class WatchExplainer : public QObject {
signals:
    void tokenReceived(const QString &token);    // one token at a time (streaming)
    void explanationReady(const QString &text);  // full text when done
    void errorOccurred(const QString &errorMsg); // Ollama unreachable or timeout
    void modelsAvailable(const QStringList &models); // sorted by size ascending

public:
    void explain(const ExplainRequest &req);           // start inference
    void warmup(const QString &modelName);             // preload model into RAM
    void checkAvailability();                          // async ping /api/tags
};
```

---

## Ollama request parameters

| Parameter | Value | Reason |
|-----------|-------|--------|
| `stream` | `true` | Tokens emitted as they arrive via `readyRead` |
| `num_ctx` | `512` | Reduces KV-cache from ~1.5 GB → ~192 MB on RPi5 |
| `num_thread` | `2` | Leaves 2 of 4 RPi5 cores free for audio/DSP pipeline |

Inference only runs when the user clicks the label — it does not run
continuously and does not affect the real-time measurement loop.

---

## Prompt

The prompt is intentionally short (~50 tokens) to keep inference fast
on RPi5:

```
You are a watchmaker. A {men's|ladies'} watch timegrapher reading:
Rate {R} s/d, Amplitude {A} deg, Beat Error {E} ms. Diagnosis: {D}.
In 3 sentences: why this diagnosis, likely mechanical cause, what to service.
```

The short prompt, combined with `num_ctx=512`, keeps total inference
time to ~3–5 seconds on RPi5 with `qwen2.5:0.5b`.

---

## Model selection

On startup, `WatchExplainer::checkAvailability()` fetches `/api/tags`
from the local Ollama server. The model list is sorted by file size
(ascending) so the **smallest installed model is selected by default**
— fastest on RPi5. The user can override via the **AI Model** dropdown
in the Misc. Parameters panel.

The selected model is also used for `warmup()`, which fires a
zero-prompt POST at startup to preload the model weights into RAM so
the first real click has no model-load delay.

### Recommended models

| Model | Size | RPi5 speed | Notes |
|-------|------|------------|-------|
| `qwen2.5:0.5b` | 394 MB | ~15–20 tok/s | Default; fast enough for streaming feel |
| `gemma3:1b` | 815 MB | ~8–10 tok/s | Slightly better English quality |
| `phi3:mini` | 2.2 GB | ~2–5 tok/s | High quality; slow on RPi5 CPU |

---

## Ollama setup

**Windows / x86-64:**
```
winget install Ollama.Ollama
ollama pull qwen2.5:0.5b
```

**RPi5 / ARM64:**
```bash
curl -fsSL https://ollama.com/install.sh | sh
ollama pull qwen2.5:0.5b
# systemd service is registered automatically
```

Restart Ollama:
```bash
sudo systemctl stop ollama
sudo systemctl start ollama
journalctl -u ollama -f   # live logs
```

---

## Error handling

If Ollama is not running or unreachable, the dialog shows:

```
Could not get AI explanation.

Ollama not reachable: ...

Make sure Ollama is running:
  ollama serve
  ollama pull qwen2.5:0.5b
```

A 120-second timeout aborts the request if the model is still loading
(common on first launch before warmup completes).

---

## What this step does *not* do

- The rule-based `WatchDiagnostics::Evaluate()` is unchanged — the LLM
  only provides a natural-language explanation, not a new diagnosis.
- No RAG, no fine-tuning — the model relies entirely on its pre-trained
  watchmaking knowledge and the short prompt above.
- No persistent chat history — each click is an independent request.

RAG (injecting project PDFs and documentation as context) is the
planned Step 3, which would allow the model to reference the Witschi
training material and project-specific thresholds when explaining a
diagnosis.
