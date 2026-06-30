# TimeGrapher

**Team**: Blue Sky (Team 3) | LG SW Architect Training Program

---

## Entry Point

For the Milestone 3 (Final Demo) presentation, start here:

**[docs/milestone3/final/README.md](docs/milestone3/final/README.md)**

For the Milestone 2 presentation, start here:

**[docs/milestone2/final/README.md](docs/milestone2/final/README.md)**

That file contains the full presentation outline, QA → Risk → Experiment → Architecture Decision traceability map, and links to all reference documents.

---

## Developer Setup

### Prerequisites

| Item | Version |
|------|---------|
| Qt | 6.11.1 |
| CMake | 3.16+ (bundled with Qt) |
| Compiler | Apple clang (macOS) / MinGW 64-bit (Windows) / GCC (RPi) |
| Docker | Required for RPi cross-build only |

### 1. Clone

```bash
git clone git@github.com:Ji-Min-Lee/2026-3-sw-architect-studio-project.git
cd 2026-3-sw-architect-studio-project
```

### 2. Configure Git Hooks (required, run once)

```bash
sh scripts/setup-hooks.sh
```

This sets `core.hooksPath` to `.githooks/` and activates the pre-commit unit test hook.
After this, `TestWatchMath` and `TestMeasurementEngine` run automatically before every commit.

### 3. Build

#### macOS

```bash
mkdir -p src/build-mac
cd src/build-mac
~/Qt/6.11.1/macos/bin/qt-cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(sysctl -n hw.ncpu)
```

Artifacts: `src/build-mac/TimeGrapher.app`

#### Windows (MinGW)

```powershell
mkdir src\build
cd src\build
C:\Qt\Tools\CMake_64\bin\cmake.exe .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64
cmake --build . -j4
```

Artifacts: `src\build\TimeGrapher.exe`

For detailed Windows setup, see [docs/week0/pc-build-verification-windows.md](docs/week0/pc-build-verification-windows.md).

#### Raspberry Pi 5 (cross-build via Docker)

```bash
sh build-rpi.sh
```

Artifacts: `build-rpi/TimeGrapher` (arm64 binary)

To deploy directly to the RPi:

```bash
RPI_HOST=pi@<ip-address> sh build-rpi.sh
```

### 4. Run Unit Tests

Tests run automatically on commit via the pre-commit hook. To run manually:

```bash
# macOS
src/build-mac/TestWatchMath
src/build-mac/TestMeasurementEngine

# Windows
src\build\TestWatchMath.exe
src\build\TestMeasurementEngine.exe
```

| Binary | Coverage | Expected |
|--------|----------|---------|
| TestWatchMath | Rate, Amplitude, Beat Error formulas | 44 passed |
| TestMeasurementEngine | End-to-end measurement computation | 8 passed |
