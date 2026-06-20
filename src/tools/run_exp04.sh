#!/usr/bin/env bash
# run_exp04.sh — EXP-04 Part B: run TimeGrapher on each SNR WAV file and
#                generate the scatter plot PNG.
#
# Usage:
#   cd src/tools && ./run_exp04.sh
#
# Outputs:
#   src/logs/EXP-04/log_snr{00,10,20,30,40,50,60}db_*.csv
#   src/logs/EXP-04/exp04_scatter.png
#
# Requirements:
#   - TimeGrapher built with ENABLE_LOGGING=ON (src/build-macos/)
#   - python3 + matplotlib
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
APP="$REPO_ROOT/src/build-macos/TimeGrapher.app/Contents/MacOS/TimeGrapher"
WAV_DIR="$REPO_ROOT/src/TimeGrapherTestFilesWeishiMic/Noise_float32"
LOG_DIR="$REPO_ROOT/src/logs/EXP-04"

mkdir -p "$LOG_DIR"

if [[ ! -x "$APP" ]]; then
    echo "ERROR: TimeGrapher binary not found at $APP"
    echo "  Build with:  cmake -DENABLE_LOGGING=ON && cmake --build ."
    exit 1
fi

CONDITIONS=(snr00db snr10db snr20db snr30db snr40db snr50db snr60db)

echo "=== EXP-04 Part B: noise_ratio sweep ==="
echo "Log dir: $LOG_DIR"
echo ""

for COND in "${CONDITIONS[@]}"; do
    WAV="$WAV_DIR/28800BPH_3235_Starbucks_${COND}.wav"
    if [[ ! -f "$WAV" ]]; then
        echo "  [SKIP] $COND — WAV not found: $WAV"
        continue
    fi

    # Remove any previous CSV for this condition so analyze script finds only one
    rm -f "$LOG_DIR"/log_"${COND}"_*.csv

    echo "  Running $COND ..."
    "$APP" \
        --wav        "$WAV"    \
        --log-dir    "$LOG_DIR" \
        --log-label  "$COND"   \
        --no-record            \
        --quit-on-done         \
        2>/dev/null &
    APP_PID=$!

    # Wait for the app to finish (max 120 s per file)
    TIMEOUT=120
    ELAPSED=0
    while kill -0 "$APP_PID" 2>/dev/null; do
        sleep 2
        ELAPSED=$((ELAPSED + 2))
        if [[ $ELAPSED -ge $TIMEOUT ]]; then
            echo "  WARNING: $COND timed out after ${TIMEOUT}s — killing"
            kill "$APP_PID" 2>/dev/null || true
            break
        fi
    done
    wait "$APP_PID" 2>/dev/null || true

    # Check that a CSV was produced
    CSV=$(ls "$LOG_DIR"/log_"${COND}"_*.csv 2>/dev/null | head -1)
    if [[ -n "$CSV" ]]; then
        ROWS=$(wc -l < "$CSV")
        echo "    -> $(basename "$CSV")  ($ROWS lines)"
    else
        echo "    -> WARNING: no CSV produced for $COND"
    fi
done

echo ""
echo "=== Generating scatter plot ==="
python3 "$SCRIPT_DIR/analyze_exp04_scatter.py" "$LOG_DIR"

echo ""
echo "Done. Open $LOG_DIR/exp04_scatter.png"
