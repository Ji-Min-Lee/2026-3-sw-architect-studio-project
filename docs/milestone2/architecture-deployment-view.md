# Architecture — Deployment View

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [ ] Draft / [ ] Final

> Describes hardware placement and communication channels.  
> Required per project plan: deployment view showing component allocation and communication.

---

## 1. Deployment Diagram

```mermaid
graph TD
    subgraph DevMachine["Development Machine\n(macOS / Windows PC)"]
        QtCreator["Qt Creator IDE"]
        SourceCode["Source Code\n(TimeGrapher_v10.5 + extensions)"]
        BuildSystem["qmake / CMake\nbuild system"]
    end

    subgraph RPi5["Raspberry Pi 5\n(Target Hardware — ARM64)"]
        subgraph RPiSW["Software"]
            TimeGrapherBin["TimeGrapher\nExecutable (Qt)"]
            RPiOS["Raspberry Pi OS\n(Linux ARM64)"]
            ALSA["ALSA / Qt Multimedia\naudio subsystem"]
            QtRuntime["Qt Runtime Libraries"]
        end
        subgraph RPiHW["Hardware Interfaces"]
            USB_Audio["USB Port\n(audio input)"]
            HDMI["HDMI Port\n(display output)"]
            USB_Touch["USB Port\n(touch input)"]
        end
    end

    subgraph Peripherals["Connected Hardware"]
        WatchMic["USB Sensor Stand\n+ Converter Box\n(watch microphone)"]
        Touchscreen["8\" IPS Touchscreen\n1280×800"]
        Watch["Mechanical Watch"]
        RefDevice["WeiShi No.1000\n(reference, standalone)"]
    end

    Watch -->|"acoustic vibration"| WatchMic
    WatchMic -->|"USB (analog→digital PCM)"| USB_Audio
    USB_Audio -->|"PCM samples"| ALSA
    ALSA -->|"audio blocks"| TimeGrapherBin
    TimeGrapherBin -->|"Qt rendering"| HDMI
    HDMI -->|"HDMI video"| Touchscreen
    Touchscreen -->|"USB HID (touch)"| USB_Touch
    USB_Touch -->|"touch events"| TimeGrapherBin

    SourceCode -->|"SSH / SCP deploy\nor SD card flash"| TimeGrapherBin
    QtCreator -->|"build"| BuildSystem
    BuildSystem -->|"produces"| SourceCode

    RefDevice -.->|"reference comparison\n(manual)"| Watch
```

---

## 2. Hardware Component Allocation

| Hardware | Software Component(s) | Communication |
|----------|----------------------|---------------|
| Raspberry Pi 5 (ARM64, 8GB) | All runtime components | — (host) |
| USB Sensor Stand + Converter | `AudioCapture` (LiveCapture) | USB Audio (ALSA) |
| 8" Touchscreen (1280×800) | Qt GUI (all graph tabs) | HDMI (video) + USB (touch) |
| Dev Machine (macOS/Windows) | Qt Creator, build toolchain | SSH/SCP or SD card for deploy |
| WeiShi No.1000 (standalone) | Not integrated in software | Manual reference comparison |

---

## 3. Communication Channels

| Channel | Protocol | Between | Latency Concern |
|---------|----------|---------|-----------------|
| Watch → Converter | Physical contact (vibration) | Watch → USB sensor stand | Critical — mechanical coupling must be secure |
| Converter → RPi | USB Audio (ALSA, PCM) | USB sensor stand → RPi | Buffer size determines capture latency |
| RPi → Touchscreen | HDMI | RPi GPU → Display | Display refresh rate (60 Hz) |
| Touch → RPi | USB HID | Touchscreen → RPi | Negligible for UI interaction |
| Dev Machine → RPi | SSH / SCP | Development PC → RPi | One-time deploy, not runtime |

---

## 4. AGC Configuration (Critical)

> Auto Gain Control (AGC) **must be disabled** before running TimeGrapher.

```
Tool: AlsaMixer (on Raspberry Pi)
Command: alsamixer -c [USB_DEVICE]
Setting: Auto Gain Control → OFF (MM = Muted)
Verify: On each RPi boot
```

If AGC is enabled: microphone gain fluctuates → signal amplitude inconsistent → Amplitude and Beat Error calculations become unreliable.

---

## 5. Development vs Production Environment

| Aspect | Development (PC) | Production (RPi) |
|--------|-----------------|-----------------|
| OS | macOS / Windows | Raspberry Pi OS (Linux ARM64) |
| Build | Qt Creator (native) | Native build on RPi, or cross-compile |
| Audio | Sim mode (no hardware) | Live mode (USB mic) |
| Display | PC monitor (any resolution) | 8" touchscreen 1280×800 |
| Performance target | Not constrained | 96k sps, responsive GUI |

---

## 6. Review Checklist

- [ ] Hardware placement shown for all components
- [ ] Communication channels identified (protocol, direction)
- [ ] Raspberry Pi 5 as primary runtime target shown
- [ ] Development vs deployment environment distinguished
- [ ] AGC configuration requirement documented
- [ ] WeiShi reference device role described
