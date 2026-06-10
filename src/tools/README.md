# Build & Run Scripts

TimeGrapher build & run scripts. The two native scripts target different OSes but share the same mode interface.

| Script | Target | Method | Build Tool |
|--------|--------|--------|------------|
| `run_timegrapher.ps1` | Windows / MinGW      | native | cmake --build (`-j4`) |
| `run_timegrapher.sh`  | Raspberry Pi / Linux | native | make (`-j4`) |
| [`../../build-rpi.sh`](../../build-rpi.sh) | Dev PC → RPi | Docker cross-compile + deploy | Docker |

Native builds use a unified output path: `src/build`.

---

## Common Modes (native scripts)

```
<script> [build|run|rebuild|all]
```

| Mode | Action |
|------|--------|
| `all` (default) | build, then run |
| `build`   | build only |
| `run`     | run only (skip build) |
| `rebuild` | delete `build` dir, then build + run |

---

## Raspberry Pi (Linux)

```bash
cd <repo>/src/tools
./run_timegrapher.sh           # build + run
./run_timegrapher.sh build     # build only
```

- Auto-sets GUI env vars (`DISPLAY`, `XAUTHORITY`, `XDG_RUNTIME_DIR`, `QT_QPA_PLATFORM=xcb`)
- Launches with `sudo` on the RPi local display (`:0`)
- Includes a guard against concurrent build/run
- Qt path: `QT_PREFIX=/home/lg/Qt/6.11.1/gcc_arm64` (edit the top of the script if your environment differs)

---

## Windows (MinGW)

```powershell
cd <repo>\src\tools
.\run_timegrapher.ps1          # build + run
.\run_timegrapher.ps1 build    # build only
```

- Adds Qt/MinGW/CMake to PATH automatically
- Qt path: `C:\Qt\6.11.1\mingw_64`, compiler: `C:\Qt\Tools\mingw1310_64` (edit the top of the script if your environment differs)

### If you hit an execution policy error

When `.\run_timegrapher.ps1` is blocked by the Execution Policy, use one of:

```powershell
# Allow for the current user permanently (recommended, no admin rights needed)
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned

# Or bypass for this invocation only
powershell -ExecutionPolicy Bypass -File .\run_timegrapher.ps1 build
```

---

## Dev PC → RPi (Docker cross-compile)

Use [`build-rpi.sh`](../../build-rpi.sh) at the repo root. Instead of building on the RPi, it produces an arm64 binary on the dev PC and deploys it.

```bash
./build-rpi.sh                        # build arm64 binary (Docker)
RPI_HOST=lg@<rpi-ip> ./build-rpi.sh   # build, then scp-deploy
```

- arm64 cross-compile based on `Dockerfile.rpi`
- Output: `build-rpi/TimeGrapher`
- Auto-deploys to the RPi when `RPI_HOST` is set

---

## Common Features (native scripts)

- **Auto path detection**: resolves `src/` relative to the script's own location (`src/tools/`), so it works regardless of the current working directory or where the repo folder is moved.
- **Stale cache handling**: if `CMakeCache.txt` points to an old path (after the folder was moved), the cache is removed and cmake is reconfigured automatically.
- **Skip configure**: when a valid cache exists, cmake configure is skipped and only compilation runs, making rebuilds fast.
