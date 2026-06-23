# ADR 004: Use Qt as the Application Framework

TimeGrapher requires a GUI framework that simultaneously handles real-time audio capture, multi-threaded DSP, and interactive graph rendering — and must run on Raspberry Pi 5 (Linux ARM64) as well as developer machines (macOS / Windows) for efficient iteration. A single framework that covers all three concerns avoids the integration overhead of combining separate libraries.

## Decision

Use **Qt 5/6** as the sole application framework for GUI, audio capture, threading, and cross-thread communication.

Key Qt subsystems used:

| Subsystem | Role in TimeGrapher |
|-----------|-------------------|
| `QWidget` / `QPainter` | Graph tab rendering and main window layout |
| `Qt Multimedia` / ALSA bridge | USB audio capture (`AudioCapture / LiveCapture`) |
| `QThread` / `QObject` | DSP Thread lifecycle management (see [ADR 001](ADR001-dsp-offload-thread.md)) |
| `Qt::QueuedConnection` | Thread-safe `Measurement` dispatch from DSP Thread to UI Thread |
| `QTimer` | Lazy rendering catch-up (`QTimer::singleShot(0)` on tab switch, see [ADR 002](ADR002-lazy-rendering.md)) |
| `qmake` / `CMake` | Cross-platform build system for macOS, Windows, and RPi ARM64 |

## Rationale

Qt provides a unified solution for all runtime concerns in a single dependency:

- **Audio capture**: `Qt Multimedia` wraps ALSA on Linux and CoreAudio on macOS, allowing the same `AudioCapture` interface to work on all platforms without conditional compilation.
- **Cross-thread communication**: Qt's signal-slot with `Qt::QueuedConnection` is the idiomatic solution for safe inter-thread data transfer without manual mutexes on the hot path.
- **Cross-platform builds**: A single `.pro` / `CMakeLists.txt` produces native binaries for RPi ARM64 and developer machines, enabling fast iteration on macOS before deploying to RPi.
- **Proven for embedded**: Qt runs on RPi with hardware-accelerated rendering available; the existing `TimeGrapher_v10.5` sample code already uses Qt.

## Rejected Alternatives

- **SDL2 + custom threading**: SDL handles audio and input well but provides no GUI widget toolkit; a separate GUI library (e.g., Dear ImGui) would be needed, doubling the dependency surface. Cross-thread communication would require manual queue implementation.
- **JUCE**: Strong audio DSP focus and good GUI, but primarily targets audio plugin development. Less mature on Linux ARM; smaller ecosystem for general Qt-style GUI widgets.
- **Native ALSA + GTK / raw X11**: Maximum control but no unified cross-platform layer; macOS validation would require a completely separate code path.

## Status

Accepted. The existing `TimeGrapher_v10.5` codebase is Qt-based; this decision formalizes the commitment and defines which subsystems are relied upon.

## Consequences

- **Positive**: Single build system and single audio/threading abstraction across all target platforms.
- **Positive**: Qt's signal-slot mechanism eliminates the need to write manual thread-safe queues for the `Measurement` dispatch path.
- **Positive**: Large Qt community and documentation; team members can find answers quickly.
- **Trade-off**: Qt is a large dependency (~200 MB installed). Not a concern for RPi 5 (8 GB RAM) but would be significant for a constrained embedded target.
- **Trade-off**: Qt's event loop is the UI thread's scheduler. All graph tab rendering and `ControlPanel` signal processing must complete quickly within each event loop iteration. Violated by the original baseline (see ADR 001 root cause); resolved by the DSP Offload Thread.

## Related ADRs
- [ADR 001 — DSP Offload Thread](ADR001-dsp-offload-thread.md)
- [ADR 003 — Four-Layer Architecture](ADR003-layered-architecture.md)
