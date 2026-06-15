#!/bin/bash
# ──────────────────────────────────────────────────────────────
# run_experiment1.sh — Experiment 1: block drop + buffer usage
#
# Runs TimeGrapher 9 times:
#   3 sample rates (48k / 96k / 192k)  x
#   3 scheduling policies (default / SCHED_RR / SCHED_FIFO)
#
# Requirements:
#   - Build the logging binary first:  ./run_timegrapher.sh build --log
#   - Run on the Raspberry Pi (needs DISPLAY=:0, sudo for chrt)
#
# Usage:
#   ./run_experiment1.sh                  # default 30s per run
#   ./run_experiment1.sh --duration 10    # 10s per run (quick test)
#   ./run_experiment1.sh --duration 60    # 60s per run (longer)
# ──────────────────────────────────────────────────────────────
set -e

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SRC_DIR=$(dirname "$SCRIPT_DIR")
BIN="$SRC_DIR/build-log/TimeGrapher"
LOG_DIR="$SRC_DIR/logs"
DURATION=300
SCHED_PRIORITY=50

# ── Parse args ────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --duration) DURATION="$2"; shift 2 ;;
        *) echo "[warn] Unknown arg: $1"; shift ;;
    esac
done

# ── Sanity checks ─────────────────────────────────────────────
if [ ! -x "$BIN" ]; then
    echo "[error] Binary not found: $BIN"
    echo "        Build first: cd $SRC_DIR/.. && ./src/tools/run_timegrapher.sh build --log"
    exit 1
fi
if ! command -v chrt &>/dev/null; then
    echo "[error] 'chrt' not found (install util-linux)"
    exit 1
fi

mkdir -p "$LOG_DIR"

# ── X11 environment (RPi) ─────────────────────────────────────
export DISPLAY=:0
export XAUTHORITY=/home/lg/.Xauthority
export XDG_RUNTIME_DIR=/run/user/1000
export QT_QPA_PLATFORM=xcb

RATES=(48000 96000 192000)
SCHEDS=("default" "rr" "fifo")

echo "============================================================"
echo " Experiment 1: Block Drop & Buffer Usage"
echo " Binary  : $BIN"
echo " Duration: ${DURATION}s per run"
echo " Runs    : ${#RATES[@]} rates x ${#SCHEDS[@]} scheds = $((${#RATES[@]} * ${#SCHEDS[@]})) total"
echo "============================================================"
echo ""

RUN=0
for RATE in "${RATES[@]}"; do
    for SCHED in "${SCHEDS[@]}"; do
        RUN=$((RUN + 1))
        LABEL="${RATE}_${SCHED}"
        echo "── Run $RUN/9: rate=${RATE}Hz  sched=${SCHED}  (${DURATION}s) ──"

        # Kill any stale instance
        pkill -x TimeGrapher 2>/dev/null || true
        sleep 1

        # Launch with appropriate scheduling policy
        case "$SCHED" in
            default)
                sudo DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY \
                    "$BIN" --rate "$RATE" --autostart --no-record --duration "$DURATION" &
                ;;
            rr)
                sudo chrt --rr "$SCHED_PRIORITY" \
                    env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY \
                    "$BIN" --rate "$RATE" --autostart --no-record --duration "$DURATION" &
                ;;
            fifo)
                sudo chrt --fifo "$SCHED_PRIORITY" \
                    env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY \
                    "$BIN" --rate "$RATE" --autostart --no-record --duration "$DURATION" &
                ;;
        esac

        APP_PID=$!

        # Wait for the app to finish (it exits after --duration seconds)
        echo "  [pid=$APP_PID] waiting ${DURATION}s..."
        wait "$APP_PID" 2>/dev/null || true

        # Give the Logger destructor a moment to flush CSV
        sleep 1

        # Rename the newest log file to include rate + sched label
        NEWEST=$(ls -t "$LOG_DIR"/log_*.csv 2>/dev/null | grep -v '_sys\.csv' | head -1)
        if [ -n "$NEWEST" ]; then
            BASE="${NEWEST%.csv}"
            mv "$NEWEST"        "${BASE}_${LABEL}.csv"
            SYS="${BASE}_sys.csv"
            if [ -f "$SYS" ]; then
                mv "$SYS"       "${BASE}_${LABEL}_sys.csv"
            fi
            echo "  -> ${BASE}_${LABEL}.csv"
        else
            echo "  [warn] No log file found for run $RUN"
        fi
        echo ""
    done
done

echo "============================================================"
echo " All 9 runs complete. Logs are in: $LOG_DIR"
echo ""
echo " Analyze each run:"
echo "   python3 $SCRIPT_DIR/analyze_log.py $LOG_DIR/<logfile>.csv"
echo ""
echo " Quick summary of block drops across all runs:"
grep -h "block_drops" "$LOG_DIR"/log_*.csv 2>/dev/null | head -20 || true
echo "============================================================"
