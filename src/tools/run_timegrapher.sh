#!/bin/bash
# ──────────────────────────────────────────────────────────────
# TimeGrapher build & run script (Raspberry Pi)
#
# Usage:
#   ./run_timegrapher.sh           # build + run
#   ./run_timegrapher.sh build     # build only
#   ./run_timegrapher.sh run       # run only (skip build)
#   ./run_timegrapher.sh rebuild   # clean build dir + build + run
# ──────────────────────────────────────────────────────────────
set -e  # 에러 발생 시 즉시 중단

# ── 설정 (경로 바뀌면 여기만 수정) ────────────────────────────
QT_PREFIX=/home/lg/Qt/6.11.1/gcc_arm64
JOBS=4

# src 디렉토리 = 이 스크립트(src/tools/)의 한 단계 위
# 실행 위치(pwd)와 무관하게 항상 정확
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SRC_DIR=$(dirname "$SCRIPT_DIR")     # src/  (CMakeLists.txt 위치)

BUILD_DIR=$SRC_DIR/build
BIN=$BUILD_DIR/TimeGrapher

# ── 실행 환경변수 (GUI) ───────────────────────────────────────
export DISPLAY=:0
export XAUTHORITY=/home/lg/.Xauthority
export XDG_RUNTIME_DIR=/run/user/1000
export QT_QPA_PLATFORM=xcb

MODE=${1:-all}   # 인자 없으면 all (build + run)

# ── 충돌 체크 ─────────────────────────────────────────────────
# 이미 빌드 중인지 확인 (make / cmake 프로세스)
if pgrep -f "make.*-j" > /dev/null 2>&1 || pgrep -x cmake > /dev/null 2>&1; then
    echo "[error] 현재 빌드가 진행 중입니다. 완료 후 다시 실행하세요."
    exit 1
fi

# 이미 TimeGrapher가 실행 중인지 확인
if pgrep -x TimeGrapher > /dev/null 2>&1; then
    echo "[error] TimeGrapher가 이미 실행 중입니다. 종료 후 다시 실행하세요."
    exit 1
fi

# ── 함수 ──────────────────────────────────────────────────────
do_build() {
    echo "[build] SRC_DIR=$SRC_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # 캐시가 옛 경로(폴더 이동 전)를 가리키면 stale → 재configure
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
        cmake .. -DCMAKE_PREFIX_PATH="$QT_PREFIX"
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
    sudo DISPLAY=:0 XAUTHORITY=/home/lg/.Xauthority "$BIN"
}

# ── 분기 ──────────────────────────────────────────────────────
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
        echo "Usage: $0 [build|run|rebuild|all]"
        exit 1
        ;;
esac
