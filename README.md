# TimeGrapher

A Qt application that captures, analyzes, and visualizes watch acoustic signals
(tick/tock) in real time. LG SW Architect Training Program × CMU MSE.

- Source: [`src/`](src/)

---

## Documentation

| Document | What it covers |
|----------|----------------|
| [docs/README.md](docs/README.md) | Deliverables index (milestones) |
| [docs/logging-design.md](docs/logging-design.md) | Performance logging facility — `ENABLE_LOGGING` / `--log`, zero-overhead design, CSV format |
| [docs/metrics-explained.md](docs/metrics-explained.md) | What a *frame* is, FPS/SPS/SPF, and every analysis graph panel |
| [src/tools/README.md](src/tools/README.md) | Build & run scripts (Windows / RPi / Docker), `--log` flag |

---

## Build & Run

Native build scripts (details in [src/tools/README.md](src/tools/README.md)):

```powershell
# Windows
.\src\tools\run_timegrapher.ps1            # build + run
.\src\tools\run_timegrapher.ps1 all --log  # build + run with performance logging
```

```bash
# Raspberry Pi
./src/tools/run_timegrapher.sh             # build + run
./src/tools/run_timegrapher.sh all --log   # build + run with performance logging
```

`--log` enables per-frame performance logging (console + CSV in `src/logs/`).
Without it, builds carry zero logging overhead.

---

## Analysis

```bash
python src/tools/analyze_log.py            # newest src/logs/log_*.csv
```

Produces `log_*.png` (pipeline latency/throughput) and, on the RPi,
`log_*_sys.png` (CPU/memory/temperature/frequency). See
[docs/metrics-explained.md](docs/metrics-explained.md) for how to read them.
