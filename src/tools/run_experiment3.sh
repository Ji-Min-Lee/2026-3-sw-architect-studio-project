#!/bin/bash
# ──────────────────────────────────────────────────────────────
# run_experiment3.sh — EXP-03: Detector Robustness Under Noise
#
# Automates TimeGrapher in PLAYBACK mode via xdotool (no C++ changes).
# Runs 3235_Starbucks noise WAVs at 3 rates × 7 SNR levels × 10 reps = 210 runs × 60s.
#
# Strategy:
#   1. Pre-extend NH35 noise WAVs to WAV_PAD_SEC using sox (once)
#   2. For each run: launch app, xdotool → PLAYBACK mode → Start →
#      file dialog → type path → Enter → sleep DURATION → kill (SIGTERM)
#   3. SIGTERM triggers handleSignal() → QCoreApplication::quit() →
#      Logger::~Logger() → writeCsv()
#
# Requirements:
#   - xdotool  : sudo apt install xdotool
#   - sox      : sudo apt install sox
#   - Run on RPi with DISPLAY=:0
#   - Existing binary: src/build-log/TimeGrapher  (./run_timegrapher.sh build --log)
#
# Usage:
#   ./run_experiment3.sh                   # 70 runs, 60s each
#   ./run_experiment3.sh --duration 30     # quick test, 30s each
#   ./run_experiment3.sh --reps 2          # 2 reps per SNR (14 runs)
# ──────────────────────────────────────────────────────────────
set -e

# ── Paths ─────────────────────────────────────────────────────
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SRC_DIR=$(dirname "$SCRIPT_DIR")
BIN="$SRC_DIR/build-log/TimeGrapher"
NOISE_DIR="$SRC_DIR/TimeGrapherTestFilesWeishiMic/Noise"
EXTENDED_DIR="$NOISE_DIR/Extended"
RAW_LOG_DIR="$SRC_DIR/logs"
LOG_DIR="$SRC_DIR/logs/EXP-03"

# ── Experiment parameters ──────────────────────────────────────
RATES=(48000 96000 192000)
DURATION=60          # seconds of actual measurement per run
WAV_PAD_SEC=75       # extended WAV length — must be > DURATION
REPS=10
SNR_LEVELS=(00 10 20 30 40 50 60)

# ── UI coordinates (from MainWindow.ui, window fixed at 1280×750) ──
# These are relative to the Qt window's client area (xdotool --window)
MODE_COMBO_X=165   # ModeComboBox center X  (pos x=100 + w=131/2)
MODE_COMBO_Y=161   # ModeComboBox center Y  (pos y=150 + h=22/2)
START_BTN_X=40     # StartPushButton center X  (pos x=10 + w=61/2)
START_BTN_Y=220    # StartPushButton center Y  (pos y=210 + h=21/2)
# File dialog: filename field is ~100px above dialog bottom edge
FILE_FIELD_FROM_BOTTOM=100

# ── Parse args ────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --duration) DURATION="$2"; WAV_PAD_SEC=$(( DURATION + 15 )); shift 2 ;;
        --reps)     REPS="$2"; shift 2 ;;
        --rates)    IFS=',' read -ra RATES <<< "$2"; shift 2 ;;
        *) echo "[warn] Unknown arg: $1"; shift ;;
    esac
done

# ── Sanity checks ─────────────────────────────────────────────
if [ ! -x "$BIN" ]; then
    echo "[error] Binary not found: $BIN"
    echo "        Build first: ./src/tools/run_timegrapher.sh build --log"
    exit 1
fi
if ! command -v xdotool &>/dev/null; then
    echo "[error] xdotool not found — sudo apt install xdotool"
    exit 1
fi
if ! command -v sox &>/dev/null; then
    echo "[error] sox not found — sudo apt install sox"
    exit 1
fi

for SNR in "${SNR_LEVELS[@]}"; do
    WAV="$NOISE_DIR/28800BPH_3235_Starbucks_snr${SNR}db.wav"
    if [ ! -f "$WAV" ]; then
        echo "[error] Missing: $WAV"
        echo "        Generate noise WAVs first: python3 $SCRIPT_DIR/add_pink_noise.py"
        exit 1
    fi
done

mkdir -p "$RAW_LOG_DIR" "$LOG_DIR" "$EXTENDED_DIR"

# ── X11 environment ───────────────────────────────────────────
export DISPLAY=:0
export XAUTHORITY=/home/lg/.Xauthority
export XDG_RUNTIME_DIR=/run/user/1000
export QT_QPA_PLATFORM=xcb

# ── Step 1: Pre-extend WAVs to WAV_PAD_SEC ────────────────────
echo "============================================================"
echo " Preparing extended WAVs (${WAV_PAD_SEC}s) in $EXTENDED_DIR"
echo "============================================================"

for SNR in "${SNR_LEVELS[@]}"; do
    IN="$NOISE_DIR/28800BPH_3235_Starbucks_snr${SNR}db.wav"
    OUT="$EXTENDED_DIR/28800BPH_3235_Starbucks_snr${SNR}db_ext.wav"

    if [ -f "$OUT" ]; then
        echo "  [skip] snr${SNR}db_ext.wav already exists"
        continue
    fi

    # Get source duration (seconds, float)
    SRC_DUR=$(sox --info -D "$IN" 2>/dev/null || soxi -D "$IN")
    SRC_DUR_INT=${SRC_DUR%.*}   # integer part for comparison

    if [ "${SRC_DUR_INT}" -ge "${WAV_PAD_SEC}" ]; then
        # Already long enough — just trim
        sox "$IN" "$OUT" trim 0 ${WAV_PAD_SEC}
    else
        # Repeat until longer than target, then trim
        REPEATS=$(python3 -c "import math; print(max(1, math.ceil($WAV_PAD_SEC / max(1,$SRC_DUR_INT)) - 1))")
        sox "$IN" "$OUT" repeat "$REPEATS" trim 0 ${WAV_PAD_SEC}
    fi
    echo "  [ok]   snr${SNR}db: ${SRC_DUR}s -> ${WAV_PAD_SEC}s"
done
echo ""

# ── Helper: wait for a window to appear ───────────────────────
wait_for_window() {
    local name="$1" timeout="${2:-10}" found=""
    for ((i=0; i<timeout*2; i++)); do
        # try with and without --onlyvisible (dialog may not be mapped yet)
        found=$(xdotool search --name "$name" 2>/dev/null | tail -1)
        [ -n "$found" ] && echo "$found" && return 0
        sleep 0.5
    done
    return 1
}

# ── Step 2: Run experiments ───────────────────────────────────
TOTAL_RUNS=$(( ${#RATES[@]} * ${#SNR_LEVELS[@]} * REPS ))
echo "============================================================"
echo " EXP-03: Noise Robustness Test"
echo " Rates   : ${RATES[*]} Hz"
echo " Duration: ${DURATION}s per run"
echo " SNR     : ${SNR_LEVELS[*]} dB  (${#SNR_LEVELS[@]} levels)"
echo " Reps    : ${REPS}"
echo " Total   : ${TOTAL_RUNS} runs (~$(( (TOTAL_RUNS * (DURATION + 12)) / 60 )) min)"
echo "============================================================"
echo ""

RUN=0
for RATE in "${RATES[@]}"; do
  for SNR in "${SNR_LEVELS[@]}"; do
    WAV_EXT="$EXTENDED_DIR/28800BPH_3235_Starbucks_snr${SNR}db_ext.wav"

    for REP in $(seq 1 "$REPS"); do
        RUN=$((RUN + 1))
        LABEL="snr${SNR}db_${RATE}hz_r${REP}"
        echo "── Run $RUN/$TOTAL_RUNS: rate=${RATE}Hz  SNR=${SNR}dB  rep=${REP}/${REPS} ──"

        # Kill any stale instance
        pkill -x TimeGrapher 2>/dev/null || true
        sleep 1

        # Launch — no --autostart; xdotool will drive the GUI
        # --no-record answers "No" to the record-session dialog automatically
        sudo DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY \
            "$BIN" --rate "$RATE" --no-record &
        APP_PID=$!
        echo "  [pid=$APP_PID] launched, waiting for splash..."

        # Wait for splash (main.cpp: QThread::sleep(4)) + window to appear
        sleep 6
        WIN_ID=$(wait_for_window "MainWindow" 8) || \
            WIN_ID=$(wait_for_window "TimeGrapher" 5) || true

        if [ -z "$WIN_ID" ]; then
            echo "  [error] Window not found — skipping run $RUN"
            kill "$APP_PID" 2>/dev/null || true
            sleep 1
            continue
        fi
        echo "  [win=$WIN_ID] window ready"

        # ── Select PLAYBACK mode ───────────────────────────────
        # Click the ModeComboBox to open its dropdown
        xdotool windowactivate --sync "$WIN_ID"
        sleep 0.3
        xdotool mousemove --window "$WIN_ID" $MODE_COMBO_X $MODE_COMBO_Y
        xdotool click 1
        sleep 0.3
        # Navigate dropdown: Home = first item (Live), Down = Playback, Enter = select
        xdotool key Home Down Return
        sleep 0.3

        # ── Click Start ───────────────────────────────────────
        xdotool mousemove --window "$WIN_ID" $START_BTN_X $START_BTN_Y
        xdotool click 1
        sleep 2.5   # give Qt time to open the dialog

        # ── Handle file dialog ────────────────────────────────
        # Try "Open Document" title first; fall back to any QFileDialog window
        DIALOG_ID=$(wait_for_window "Open Document" 8) || \
            DIALOG_ID=$(xdotool search --class "QFileDialog" 2>/dev/null | tail -1) || true

        if [ -z "$DIALOG_ID" ]; then
            echo "  [error] File dialog not found — skipping run $RUN"
            kill "$APP_PID" 2>/dev/null || true
            sleep 1
            continue
        fi

        xdotool windowactivate --sync "$DIALOG_ID"
        sleep 0.3

        # Click the filename field (fixed distance from dialog bottom)
        eval $(xdotool getwindowgeometry --shell "$DIALOG_ID")
        FNAME_X=$(( WIDTH / 2 ))
        FNAME_Y=$(( HEIGHT - FILE_FIELD_FROM_BOTTOM ))
        xdotool mousemove --window "$DIALOG_ID" $FNAME_X $FNAME_Y
        xdotool click 1
        sleep 0.2

        # Clear existing text and type the full WAV path
        xdotool key ctrl+a
        xdotool type --delay 15 "$WAV_EXT"
        xdotool key Return
        sleep 0.5

        # ── Measure for DURATION seconds ──────────────────────
        echo "  [playing] ${WAV_EXT##*/}  (${DURATION}s)"
        sleep "$DURATION"

        # SIGTERM → handleSignal() → QCoreApplication::quit()
        #        → Logger::~Logger() → writeCsv()
        kill "$APP_PID" 2>/dev/null || true
        wait "$APP_PID" 2>/dev/null || true
        sleep 1   # allow Logger destructor to flush

        # ── Collect log file ──────────────────────────────────
        NEWEST=$(ls -t "$RAW_LOG_DIR"/log_*.csv 2>/dev/null \
                 | grep -v '_sys\.csv' \
                 | grep -v '/EXP-' \
                 | head -1)
        if [ -n "$NEWEST" ]; then
            BASENAME=$(basename "${NEWEST%.csv}")
            DEST="$LOG_DIR/${BASENAME}_${LABEL}.csv"
            mv "$NEWEST" "$DEST"
            SYS="${NEWEST%.csv}_sys.csv"
            [ -f "$SYS" ] && mv "$SYS" "$LOG_DIR/${BASENAME}_${LABEL}_sys.csv"
            echo "  -> $DEST"
        else
            echo "  [warn] No log file found for run $RUN (${LABEL})"
        fi
        echo ""
    done
  done
done

echo "============================================================"
echo " All ${TOTAL_RUNS} runs complete. Logs: $LOG_DIR"
echo "============================================================"
echo ""

# ── Step 3: Summary ───────────────────────────────────────────
echo " Generating summary (by rate × SNR)..."
echo ""

python3 - "$LOG_DIR" "${RATES[@]}" "---" "${SNR_LEVELS[@]}" "$REPS" <<'PYEOF'
import sys, os, csv, glob

args       = sys.argv[1:]
log_dir    = args[0]
sep        = args.index("---")
rates      = args[1:sep]
snr_levels = args[sep+1:-1]
reps       = int(args[-1])

def read_mean(f):
    rows = []
    try:
        with open(f, newline="") as fh:
            for row in csv.DictReader(fh):
                try:
                    if float(row.get("fg_fps", 0)) >= 5:
                        rows.append(row)
                except (ValueError, KeyError):
                    pass
    except Exception as e:
        print(f"  [warn] {os.path.basename(f)}: {e}")
        return None
    if not rows:
        return None
    def mean(col):
        vals = [float(r[col]) for r in rows if r.get(col) not in (None, "")]
        return sum(vals) / len(vals) if vals else 0.0
    return len(rows), mean("block_drops"), mean("buffer_pct"), mean("fg_fps")

for rate in rates:
    print(f"{'='*70}")
    print(f" Rate: {rate} Hz")
    print(f"{'='*70}")
    print(f"{'SNR':>6}  {'Rep':>4}  {'Frames':>7}  {'drops_mean':>11}  {'buf_pct':>9}  {'fps':>7}")
    print("-" * 55)

    rate_summary = {}
    for snr in snr_levels:
        rate_summary[snr] = []
        pattern = os.path.join(log_dir, f"*_snr{snr}db_{rate}hz_r*.csv")
        files = sorted(f for f in glob.glob(pattern) if "_sys.csv" not in f)

        for f in files[:reps]:
            rep_tag = os.path.basename(f).split("_r")[-1].replace(".csv", "")
            result = read_mean(f)
            if result is None:
                continue
            nf, dm, bm, fm = result
            rate_summary[snr].append((dm, bm, fm))
            print(f"{snr:>6}  {rep_tag:>4}  {nf:>7}  {dm:>11.4f}  {bm:>9.4f}  {fm:>7.2f}")

        if rate_summary[snr]:
            n = len(rate_summary[snr])
            print(f"{'':>6}  {'AVG':>4}  {'':>7}  "
                  f"{sum(r[0] for r in rate_summary[snr])/n:>11.4f}  "
                  f"{sum(r[1] for r in rate_summary[snr])/n:>9.4f}  "
                  f"{sum(r[2] for r in rate_summary[snr])/n:>7.2f}")
        print()

    print(f" Average by SNR ({rate} Hz):")
    print(f"{'SNR':>6}  {'N':>3}  {'drops_mean':>11}  {'buf_pct':>9}  {'fps':>7}")
    print("-" * 40)
    for snr in snr_levels:
        rows = rate_summary[snr]
        if not rows:
            print(f"{snr:>6}  {'0':>3}  {'N/A':>11}  {'N/A':>9}  {'N/A':>7}")
            continue
        n = len(rows)
        print(f"{snr:>6}  {n:>3}  "
              f"{sum(r[0] for r in rows)/n:>11.4f}  "
              f"{sum(r[1] for r in rows)/n:>9.4f}  "
              f"{sum(r[2] for r in rows)/n:>7.2f}")
    print()
PYEOF
