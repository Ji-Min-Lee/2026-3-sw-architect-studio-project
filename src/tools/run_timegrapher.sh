#!/bin/bash
# ──────────────────────────────────────────────────────────────
# TimeGrapher build & run script (Raspberry Pi)
#
# Usage:
#   ./run_timegrapher.sh                   # build + run (no logging)
#   ./run_timegrapher.sh build             # build only
#   ./run_timegrapher.sh run               # run only (skip build)
#   ./run_timegrapher.sh rebuild           # clean build dir + build + run
#   ./run_timegrapher.sh build --log       # build with performance logging
#   ./run_timegrapher.sh all --log         # build + run with logging
# (--log uses a separate build dir: build-log/)
# ──────────────────────────────────────────────────────────────
set -e  # exit immediately on error

# ── Config (edit here if paths change) ────────────────────────
QT_PREFIX=/home/lg/Qt/6.11.1/gcc_arm64
JOBS=4

# src dir = one level up from this script (src/tools/),
# always correct regardless of the current working directory
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SRC_DIR=$(dirname "$SCRIPT_DIR")     # src/  (location of CMakeLists.txt)

# ── Parse args: mode + optional "logging" flag (any order) ────
MODE=all
LOGGING=OFF
for a in "$@"; do
    case "$a" in
        --log|logging) LOGGING=ON ;;
        build|run|rebuild|all) MODE=$a ;;
    esac
done

# Logging and non-logging binaries use separate build dirs so their
# CMake caches never thrash.
if [ "$LOGGING" = "ON" ]; then
    BUILD_DIR=$SRC_DIR/build-log
else
    BUILD_DIR=$SRC_DIR/build
fi
BIN=$BUILD_DIR/TimeGrapher

# ── Runtime env vars (GUI) ────────────────────────────────────
export DISPLAY=:0
export XAUTHORITY=/home/lg/.Xauthority
export XDG_RUNTIME_DIR=/run/user/1000
export QT_QPA_PLATFORM=xcb

# ── Conflict check ────────────────────────────────────────────
# Check if a build is already in progress (make / cmake process)
if pgrep -f "make.*-j" > /dev/null 2>&1 || pgrep -x cmake > /dev/null 2>&1; then
    echo "[error] A build is already in progress. Try again after it finishes."
    exit 1
fi

# Check if TimeGrapher is already running
if pgrep -x TimeGrapher > /dev/null 2>&1; then
    echo "[error] TimeGrapher is already running. Quit it before running again."
    exit 1
fi

# ── Functions ─────────────────────────────────────────────────
do_build() {
    echo "[build] SRC_DIR=$SRC_DIR  ENABLE_LOGGING=$LOGGING  ($(basename "$BUILD_DIR"))"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # If cache points to an old path (after folder move), reconfigure
    cached_src=""
    if [ -f CMakeCache.txt ]; then
        cached_src=$(grep '^CMAKE_HOME_DIRECTORY' CMakeCache.txt | cut -d= -f2)
    fi

    if [ ! -f CMakeCache.txt ] || [ "$cached_src" != "$SRC_DIR" ]; then
        if [ -n "$cached_src" ] && [ "$cached_src" != "$SRC_DIR" ]; then
            echo "[build] stale cache ($cached_src != $SRC_DIR) -> reconfigure"
            rm -f CMakeCache.txt
        fi
        echo "[build] configuring..."
        cmake .. -DCMAKE_PREFIX_PATH="$QT_PREFIX" -DENABLE_LOGGING="$LOGGING"
    fi

    echo "[build] compiling (-j$JOBS)..."
    make -j"$JOBS"
    echo "[build] done -> $BIN"
}

do_run() {
    if [ ! -x "$BIN" ]; then
        echo "[run] binary not found: $BIN"
        echo "[run] run build first."
        exit 1
    fi
    echo "[run] launching TimeGrapher..."
    sudo DISPLAY=:0 XAUTHORITY=/home/lg/.Xauthority \
         XDG_RUNTIME_DIR=/run/user/1000 QT_QPA_PLATFORM=xcb "$BIN"
}

# ── Dispatch ──────────────────────────────────────────────────
case "$MODE" in
    build)
        do_build
        ;;
    run)
        do_run
        ;;
    rebuild)
        echo "[rebuild] removing $BUILD_DIR"
        rm -rf "$BUILD_DIR"
        do_build
        do_run
        ;;
    all)
        do_build
        do_run
        ;;
    *)
        echo "Usage: $0 [build|run|rebuild|all] [--log]"
        exit 1
        ;;
esac
