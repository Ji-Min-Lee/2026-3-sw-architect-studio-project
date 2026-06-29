# QAS-2: Low Latency and Low Number of Missed Beats

| Field | Detail |
|-------|--------|
| **Source** | Mechanical watch placed on microphone |
| **Stimulus** | A single beat event (tick) occurs at the microphone |
| **Artifact** | Full pipeline — from AudioCapture callback to GUI display update |
| **Environment** | Raspberry Pi 5, normal operating conditions, Qt GUI active |
| **Response** | Corresponding waveform, beat markers, and computed values appear in the GUI |
| **Measure** | capture→detect < 20.8 ms (one beat period at 28,800 BPH); end-to-end < 100 ms |
