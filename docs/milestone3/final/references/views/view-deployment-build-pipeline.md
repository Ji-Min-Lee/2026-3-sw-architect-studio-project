# Deployment View: Build-Deploy Pipeline

This view shows which hardware nodes run which software artifacts and what the deploy path is from the developer's machine to the Raspberry Pi 5. It answers: "How does code built on macOS reach the target hardware for experiments and the final demo?"

![RPi 5 Deployment View](../../assets/view4-deployment.png)

## Element Catalog

#### Dev Machine (macOS)
- Primary development environment: Qt Creator, C++17 toolchain.
- Runs the full build, unit tests, and structural validation (layered + C&C views).
- Executes macOS experiments (e.g., EXP-02 R2–R4 for baseline latency measurements).
- Does **not** require direct access to microphone hardware for structural validation.

#### Raspberry Pi 5 (Target Hardware)
- ARM64, Ubuntu 24.04, 4-core Cortex-A76.
- Runs the production binary: AudioCapture via ALSA, DSPWorker, Qt 6 GUI.
- Used for hardware experiments (EXP-01, EXP-02 R5, EXP-05) and M3 final demo.
- **Constraint**: AGC must be disabled on every boot (`alsamixer`) — AGC enabled causes Amplitude and Beat Error measurements to be unreliable.

#### Git Repository (Shared)
- The transport layer between dev machine and RPi.
- Dev machine pushes; RPi pulls. No file transfer or re-imaging required.

#### Microphone (Hardware Input)
- USB microphone connected to RPi; captures the watch's acoustic beat signal at 96kHz (target) or 48kHz (fallback).

## Behavior

**Standard deploy cycle:**

```
Dev Machine
    1. edit / build / unit test (macOS toolchain)
    2. git push
         │
         ▼
    Git Repository
         │
         ▼
    Raspberry Pi 5
    3. git pull
    4. build (arm64 toolchain)
    5. run experiment / validate
```

**Why this shortens the cycle**: structural validation (layer rule violations, API contracts) happens on macOS. The RPi is reserved for hardware-dependent experiments and the final demo. No repeated re-imaging or manual file transfers.

**Boot checklist for every RPi session:**

| Step | Command | Reason |
|------|---------|--------|
| Disable AGC | `alsamixer` → disable AGC | AGC on → Amplitude / Beat Error unreliable |
| Confirm sample rate | check ALSA config | Must match ADR-003 decision (96kHz or 48kHz fallback) |
| Run experiment script | `./run_exp.sh` | Structured CSV logging for EXP analysis |

## Related ADRs

- [ADR-003: Audio Sample Rate Selection](../adr/ADR-003-sample-rate-selection.md) — determines ALSA sample rate configured on RPi
- [ADR-001: T2 DSP Offload Thread](../adr/ADR-001-t2-dsp-offload-thread.md) — thread model confirmed on macOS; RPi confirmation via EXP-02 R5

## Related views

- [C&C View: DSP Pipeline Thread Model](view-cc-dsp-pipeline.md) — shows the runtime components that execute on the RPi node
- [Layered View: 4-Layer Allowed-to-Use](view-layered-4layer.md) — module structure validated on dev machine before RPi deployment
