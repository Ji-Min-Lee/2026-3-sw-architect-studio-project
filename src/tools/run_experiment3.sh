#!/bin/bash
# ──────────────────────────────────────────────────────────────
# run_experiment3.sh — EXP-03: Detector Robustness Under Noise
#
# Sweeps onset_fraction × min_peak_fraction × noise level using
# the --onset / --min-peak / --file CLI args added to TimeGrapher.
# No xdotool or GUI automation needed.
#
# Phase 1 defaults (넓게 훑기): 3 onset × 3 min_peak × 3 noise = 27 runs
#
# Usage:
#   ./run_experiment3.sh                        # 27 runs × 60s
#   ./run_experiment3.sh --duration 30          # 30s per run
#   ./run_experiment3.sh --reps 5              # 5 reps per combo
#   ./run_experiment3.sh --noise 00,30,60      # custom noise levels
#   ./run_experiment3.sh --onset 0.02,0.05     # custom onset values
#   ./run_experiment3.sh --min-peak 0.10,0.20  # custom min_peak values
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
NOISE_LEVELS=(00 10 20 30 40 50 60) # 00=clean, 60=max noise
ONSET_LEVELS=(0.02 0.05 0.08)       # onset_fraction values to sweep
MIN_PEAK_LEVELS=(0.10 0.20 0.30)    # min_peak_fraction values to sweep
DURATION=45                          # seconds per run (= original WAV length)
REPS=5                               # reps per parameter combination

# ── Parse args ────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --duration)  DURATION="$2"; shift 2 ;;
        --reps)      REPS="$2"; shift 2 ;;
        --noise)     IFS=',' read -ra NOISE_LEVELS <<< "$2"; shift 2 ;;
        --onset)     IFS=',' read -ra ONSET_LEVELS <<< "$2"; shift 2 ;;
        --min-peak)  IFS=',' read -ra MIN_PEAK_LEVELS <<< "$2"; shift 2 ;;
        *) echo "[warn] Unknown arg: $1"; shift ;;
    esac
done

# ── Sanity checks ─────────────────────────────────────────────
if [ ! -x "$BIN" ]; then
    echo "[error] Binary not found: $BIN"
    echo "        Build first: ./src/tools/run_timegrapher.sh build --log"
    exit 1
fi
if ! command -v sox &>/dev/null; then
    echo "[error] sox not found — sudo apt install sox"
    exit 1
fi

for NOISE in "${NOISE_LEVELS[@]}"; do
    WAV="$NOISE_DIR/28800BPH_3235_Starbucks_noise${NOISE}db.wav"
    if [ ! -f "$WAV" ]; then
        echo "[error] Missing: $WAV"
        echo "        Generate noise WAVs first: python3 $SCRIPT_DIR/add_pink_noise.py"
        exit 1
    fi
done

mkdir -p "$RAW_LOG_DIR" "$LOG_DIR"

# ── X11 environment ───────────────────────────────────────────
export DISPLAY=:0
export XAUTHORITY=/home/lg/.Xauthority
export XDG_RUNTIME_DIR=/run/user/1000
export QT_QPA_PLATFORM=xcb

# ── Step 1: Run experiments ───────────────────────────────────
TOTAL_RUNS=$(( ${#ONSET_LEVELS[@]} * ${#MIN_PEAK_LEVELS[@]} * ${#NOISE_LEVELS[@]} * REPS ))
echo "============================================================"
echo " EXP-03: Detector Robustness — Parameter Sweep"
echo " onset    : ${ONSET_LEVELS[*]}"
echo " min_peak : ${MIN_PEAK_LEVELS[*]}"
echo " noise    : ${NOISE_LEVELS[*]} dB"
echo " Duration : ${DURATION}s per run"
echo " Reps     : ${REPS}"
echo " Total    : ${TOTAL_RUNS} runs (~$(( (TOTAL_RUNS * (DURATION + 12)) / 60 )) min)"
echo "============================================================"
echo ""

RUN=0
for ONSET in "${ONSET_LEVELS[@]}"; do
  ONSET_TAG=$(echo "$ONSET" | tr -d '.')        # 0.02 → 002
  for MINPEAK in "${MIN_PEAK_LEVELS[@]}"; do
    MINPEAK_TAG=$(echo "$MINPEAK" | tr -d '.')  # 0.10 → 010
    for NOISE in "${NOISE_LEVELS[@]}"; do
      WAV_EXT="$NOISE_DIR/28800BPH_3235_Starbucks_noise${NOISE}db.wav"
      for REP in $(seq 1 "$REPS"); do
          RUN=$((RUN + 1))
          LABEL="onset${ONSET_TAG}_minpk${MINPEAK_TAG}_noise${NOISE}db_r${REP}"
          echo "── Run $RUN/$TOTAL_RUNS: onset=${ONSET}  min_peak=${MINPEAK}  noise=${NOISE}dB  rep=${REP}/${REPS} ──"

          # Skip if already done (resume support)
          if ls "$LOG_DIR"/*_${LABEL}.csv &>/dev/null; then
              echo "  [skip] already exists"
              echo ""
              continue
          fi

          pkill -x TimeGrapher 2>/dev/null || true
          sleep 1

          DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY \
              "$BIN" --no-record \
                     --file "$WAV_EXT" \
                     --duration "$DURATION" \
                     --onset "$ONSET" \
                     --min-peak "$MINPEAK" &
          APP_PID=$!
          echo "  [pid=$APP_PID] onset=${ONSET} min_peak=${MINPEAK}"

          echo "  [playing] ${WAV_EXT##*/}  (${DURATION}s)"
          sleep "$DURATION"

          kill "$APP_PID" 2>/dev/null || true
          wait "$APP_PID" 2>/dev/null || true
          sleep 1

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
done

echo "============================================================"
echo " All ${TOTAL_RUNS} runs complete. Logs: $LOG_DIR"
echo "============================================================"
echo ""

# ── Step 3: Summary ───────────────────────────────────────────
echo " Generating summary (onset × min_peak × noise)..."
echo ""

python3 - "$LOG_DIR" "${ONSET_LEVELS[@]}" "---" "${MIN_PEAK_LEVELS[@]}" "===" "${NOISE_LEVELS[@]}" "$REPS" <<'PYEOF'
import sys, os, csv, glob, math

args          = sys.argv[1:]
log_dir       = args[0]
sep1          = args.index("---")
sep2          = args.index("===")
onset_levels  = args[1:sep1]
minpk_levels  = args[sep1+1:sep2]
noise_levels  = args[sep2+1:-1]
reps          = int(args[-1])

def onset_tag(v):  return v.replace(".", "")
def minpk_tag(v):  return v.replace(".", "")

def read_file(f):
    """Return list of row dicts, skipping # comment lines."""
    try:
        with open(f, newline="") as fh:
            lines = [l for l in fh if not l.startswith("#")]
        return list(csv.DictReader(lines))
    except Exception as e:
        print(f"  [warn] {os.path.basename(f)}: {e}")
        return []

def stats(rows):
    """Compute detection accuracy stats from a list of frame rows.
    lock% counts frames where sync_locked=1.
    rate/beat/amp are averaged over frames where the value is non-empty
    (naturally excludes pre-lock and frames where RLS hasn't converged).
    std = sample standard deviation."""
    if not rows:
        return None
    total = len(rows)
    locked = sum(1 for r in rows if r.get("sync_locked") == "1")

    def mean_std(col):
        vals = [float(r[col]) for r in rows if r.get(col) not in (None, "")]
        if not vals:
            return None, None, 0
        m = sum(vals) / len(vals)
        s = math.sqrt(sum((v - m)**2 for v in vals) / len(vals)) if len(vals) > 1 else 0.0
        return m, s, len(vals)

    rate_m, rate_s, rn = mean_std("rate_spd")
    beat_m, beat_s, bn = mean_std("beat_error_ms")
    amp_m,  amp_s,  an = mean_std("amplitude_deg")
    lock_pct = 100.0 * locked / total
    return total, lock_pct, rate_m, rate_s, beat_m, beat_s, amp_m, amp_s

# ── Per-combination detail ─────────────────────────────────────
for onset in onset_levels:
  for minpk in minpk_levels:
    print(f"{'='*78}")
    print(f" onset={onset}  min_peak={minpk}")
    print(f"{'='*78}")
    print(f"{'noise':>6}  {'rep':>3}  {'lock%':>5}  "
          f"{'rate_spd':>9} {'±':>1}{'std':>6}  "
          f"{'beat_ms':>7} {'±':>1}{'std':>5}  "
          f"{'amp_deg':>7} {'±':>1}{'std':>5}")
    print("-" * 78)

    combo = {}
    for noise in noise_levels:
        combo[noise] = []
        pat = os.path.join(log_dir,
              f"*_onset{onset_tag(onset)}_minpk{minpk_tag(minpk)}_noise{noise}db_r*.csv")
        files = sorted(f for f in glob.glob(pat) if "_sys.csv" not in f)

        for f in files[:reps]:
            rep_tag = os.path.basename(f).split("_r")[-1].replace(".csv","")
            rows = read_file(f)
            s = stats(rows)
            if s is None:
                continue
            total, lk, rm, rs, bm, bs, am, as_ = s
            combo[noise].append((lk, rm, bm, am))
            r_str = f"{rm:+.2f}" if rm is not None else "  N/A "
            rs_str= f"{rs:.2f}"  if rs is not None else "  N/A"
            b_str = f"{bm:.3f}"  if bm is not None else " N/A "
            bs_str= f"{bs:.3f}"  if bs is not None else " N/A"
            a_str = f"{am:.1f}"  if am is not None else " N/A"
            as_str= f"{as_:.1f}" if as_ is not None else " N/A"
            print(f"{noise+'dB':>6}  {rep_tag:>3}  {lk:>5.1f}%  "
                  f"{r_str:>9} {rs_str:>6}  "
                  f"{b_str:>7} {bs_str:>5}  "
                  f"{a_str:>7} {as_str:>5}")

        if combo[noise]:
            n = len(combo[noise])
            def cavg(i): return sum(r[i] for r in combo[noise] if r[i] is not None) / n
            print(f"{'':>6}  {'AVG':>3}  {cavg(0):>5.1f}%  "
                  f"{cavg(1):>+9.2f} {'':>6}  "
                  f"{cavg(2):>7.3f} {'':>5}  "
                  f"{cavg(3):>7.1f} {'':>5}")
        print()

# ── Ranking: best combinations by noise robustness ────────────
print(f"{'='*78}")
print(" Ranking — avg |rate_spd| across all noise levels (lower = more robust)")
print(f"{'='*78}")
print(f"{'onset':>6}  {'minpk':>6}  {'noise':>6}  "
      f"{'rate_spd':>9}  {'beat_ms':>7}  {'amp_deg':>7}  {'lock%':>5}")
print("-" * 65)

ranking = []
for onset in onset_levels:
  for minpk in minpk_levels:
    for noise in noise_levels:
        pat = os.path.join(log_dir,
              f"*_onset{onset_tag(onset)}_minpk{minpk_tag(minpk)}_noise{noise}db_r*.csv")
        files = sorted(f for f in glob.glob(pat) if "_sys.csv" not in f)
        all_rows = []
        for f in files[:reps]:
            all_rows.extend(read_file(f))
        s = stats(all_rows)
        if s is None or s[2] is None:
            continue
        total, lk, rm, rs, bm, bs, am, as_ = s
        ranking.append((abs(rm), onset, minpk, noise, rm, bm, am, lk))

ranking.sort()
for absrate, onset, minpk, noise, rm, bm, am, lk in ranking:
    b_str = f"{bm:.3f}" if bm is not None else " N/A "
    a_str = f"{am:.1f}" if am is not None else " N/A"
    print(f"{onset:>6}  {minpk:>6}  {noise+'dB':>6}  "
          f"{rm:>+9.2f}  {b_str:>7}  {a_str:>7}  {lk:>5.1f}%")
PYEOF
