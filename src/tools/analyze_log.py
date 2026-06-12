#!/usr/bin/env python3
"""
TimeGrapher log analyzer.

Reads a per-frame CSV produced by the logging build (src/logs/log_*.csv) and
prints summary statistics plus a 5-panel plot. All durations in the CSV are ms.

Panels 1-4 use a rolling-window average (computed here) for a smooth overview;
panel 5 shows the per-frame timestamp breakdown as horizontal stacked bars.

Usage:
  python analyze_log.py [csv_path] [--window N]

  - no arg   : analyze the newest src/logs/log_*.csv
  - csv_path : analyze a specific file
  - --window : averaging window for overview panels (default 100 frames)

Requires: matplotlib  (pip install matplotlib)
"""

import sys
import os
import csv
import glob
import statistics

# ── args ──────────────────────────────────────────────────────
args = [a for a in sys.argv[1:]]
window = 100
if "--window" in args:
    i = args.index("--window")
    window = int(args[i + 1]); del args[i:i + 2]
csv_path = args[0] if args else None

def newest_log():
    here = os.path.dirname(os.path.abspath(__file__))
    files = sorted(glob.glob(os.path.join(here, "..", "logs", "log_*.csv")),
                   key=os.path.getmtime)
    files = [f for f in files if not f.endswith("_raw.csv")]
    return files[-1] if files else None

if not csv_path:
    csv_path = newest_log()
if not csv_path or not os.path.exists(csv_path):
    print("No CSV found. Pass a path or run the logging build first.")
    sys.exit(1)
print(f"Analyzing: {csv_path}  (overview window={window})\n")

# ── load per-frame CSV (skip '#' metadata lines) ──────────────
cols = {}
meta = ""
with open(csv_path, newline="") as f:
    all_lines = f.readlines()
data_lines = []
for ln in all_lines:
    if ln.lstrip().startswith("#"):
        if not meta:
            meta = ln.lstrip("# ").strip()   # keep first metadata line
    else:
        data_lines.append(ln)
for row in csv.DictReader(data_lines):
    for k, v in row.items():
        try: cols.setdefault(k, []).append(float(v))
        except (ValueError, TypeError): pass

n = len(cols.get("frame", []))
if n == 0:
    print("CSV has no data rows yet (run longer with audio flowing).")
    sys.exit(1)
if meta:
    print(f"Meta: {meta}")
print(f"Frames: {n}\n")

# ── summary stats (per frame) ─────────────────────────────────
def stat(name, unit=""):
    if name not in cols or not cols[name]:
        return
    v = cols[name]
    print(f"  {name:10s} avg={statistics.mean(v):8.3f}  "
          f"max={max(v):8.3f}  min={min(v):8.3f} {unit}")

print("=== Latency per frame (ms) ===")
for k in ("total_ms", "wait_ms", "exec_ms"):
    stat(k, "ms")
print("\n=== exec breakdown per frame (ms) ===")
for k in ("copy_ms", "sound_ms", "tg_ms", "ui_ms", "plot_ms"):
    stat(k, "ms")
print("\n=== Throughput ===")
for k in ("samples", "bg_fps", "fg_fps", "bg_sps", "fg_sps"):
    stat(k)

# Real-time deadline = BG chunk period = BG_SPF / BG_SPS.
# Use the steady-state (median of non-zero) values; warmup rows are 0.
def median_nonzero(vals, default):
    nz = sorted(v for v in vals if v > 0)
    return nz[len(nz) // 2] if nz else default

spf = median_nonzero(cols.get("bg_spf", []), 480)
sps = median_nonzero(cols.get("bg_sps", []), 48000)
deadline_ms = (spf / sps * 1000.0) if sps else 10.0

exec_over = sum(1 for t in cols.get("exec_ms", []) if t > deadline_ms)
backlog = sum(1 for s in cols.get("samples", []) if s > spf * 1.5)
print(f"\n  deadline (BG period) : {deadline_ms:.2f} ms  (spf={spf:.0f}, sps={sps:.0f})")
print(f"  exec > deadline      : {exec_over} / {n}")
print(f"  backlog (>1.5x SPF)  : {backlog} / {n}")

# ── rolling average for overview panels ───────────────────────
def rolling(v, w):
    out, acc = [], 0.0
    from collections import deque
    q = deque()
    for x in v:
        q.append(x); acc += x
        if len(q) > w: acc -= q.popleft()
        out.append(acc / len(q))
    return out

# ── plot ──────────────────────────────────────────────────────
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError:
    print("\n(matplotlib not installed -> skipping plot. pip install matplotlib)")
    sys.exit(0)

x = cols["frame"]
rt = {k: rolling(cols[k], window) for k in
      ("total_ms", "wait_ms", "exec_ms", "samples", "fg_fps", "bg_fps",
       "bg_sps", "fg_sps", "plot_ms", "tg_ms", "ui_ms", "copy_ms")}

# Layout: 4 full-width overview panels + a bottom row of 3 per-frame
# horizontal breakdowns (e2e / wait / exec).
fig = plt.figure(figsize=(14, 17))
gs = fig.add_gridspec(5, 3, height_ratios=[1, 1, 1, 1, 2.2])
ax = [fig.add_subplot(gs[i, :]) for i in range(4)]
fig.suptitle(f"TimeGrapher Log Analysis  ({n} frames, overview window={window})\n"
             f"{os.path.basename(csv_path)}"
             + (f"\n{meta}" if meta else ""), fontsize=11)

# 1) Latency
ax[0].plot(x, rt["total_ms"], label="total", color="red", linewidth=1.2)
ax[0].plot(x, rt["wait_ms"],  label="wait",  color="orange", linewidth=1.0)
ax[0].plot(x, rt["exec_ms"],  label="exec",  color="blue", linewidth=1.0)
ax[0].axhline(y=deadline_ms, color="red", linestyle="--", alpha=0.5, linewidth=0.8,
              label=f"deadline {deadline_ms:.1f}ms")
ax[0].set_ylabel("ms"); ax[0].set_title("Latency (rolling avg): total = wait + exec")
ax[0].legend(loc="upper left", fontsize=8); ax[0].grid(True, alpha=0.3)

# 2) samples + FPS
ax2 = ax[1].twinx()
ax[1].plot(x, rt["samples"], label="samples", color="purple", linewidth=1.2)
ax[1].axhline(y=spf, color="purple", linestyle="--", alpha=0.5, linewidth=0.8,
              label=f"SPF={spf:.0f}")
ax2.plot(x, rt["fg_fps"], label="FG fps", color="green", linewidth=1.0, alpha=0.8)
ax2.plot(x, rt["bg_fps"], label="BG fps", color="gray", linewidth=0.8, alpha=0.5)
ax[1].set_ylabel("samples"); ax2.set_ylabel("FPS")
ax[1].set_title("samples + BG/FG FPS (rolling avg)")
ax[1].legend(loc="upper left", fontsize=8); ax2.legend(loc="upper right", fontsize=8)
ax[1].grid(True, alpha=0.3)

# 3) exec breakdown
for name, color in (("plot_ms", "steelblue"), ("tg_ms", "darkorange"),
                    ("ui_ms", "green"), ("copy_ms", "brown")):
    ax[2].plot(x, rt[name], label=name.replace("_ms", ""), color=color, linewidth=1.0)
ax[2].set_ylabel("ms"); ax[2].set_title("exec breakdown (rolling avg)")
ax[2].legend(loc="upper left", fontsize=8); ax[2].grid(True, alpha=0.3)

# 4) SPS throughput
ax[3].plot(x, rt["bg_sps"], label="BG sps", color="gray", linewidth=1.0)
ax[3].plot(x, rt["fg_sps"], label="FG sps", color="green", linewidth=1.0)
ax[3].set_ylabel("samples/sec"); ax[3].set_xlabel("frame")
ax[3].set_title("BG vs FG SPS (rolling avg)")
ax[3].legend(loc="upper left", fontsize=8); ax[3].grid(True, alpha=0.3)

# 5) per-FRAME horizontal breakdowns, three side by side:
#    (a) e2e = wait + exec sections   (b) wait only   (c) exec sections only
ypos = list(range(n))
step = max(1, n // 20)
yticklabels = [f"{int(x[i])}" for i in ypos[::step]]

def hbreak(axx, segs, title):
    left = [0.0] * n
    for name, label, color in segs:
        vals = cols.get(name, [0.0] * n)
        axx.barh(ypos, vals, left=left, height=1.0, label=label, color=color)
        left = [l + v for l, v in zip(left, vals)]
    axx.set_xlabel("ms"); axx.set_title(title, fontsize=10)
    axx.set_yticks(ypos[::step]); axx.set_yticklabels(yticklabels, fontsize=6)
    axx.invert_yaxis()
    axx.legend(loc="lower right", fontsize=7, ncol=2)
    axx.grid(True, axis="x", alpha=0.3)

exec_seg = [("copy_ms", "copy", "brown"), ("sound_ms", "sound", "olive"),
            ("tg_ms", "tg", "darkorange"), ("ui_ms", "ui", "green"),
            ("plot_ms", "plot", "steelblue")]
e2e_seg  = [("wait_ms", "wait", "orange")] + exec_seg

ax_e2e  = fig.add_subplot(gs[4, 0])
ax_wait = fig.add_subplot(gs[4, 1])
ax_exec = fig.add_subplot(gs[4, 2])
ax_e2e.set_ylabel("frame")
hbreak(ax_e2e,  e2e_seg,                         f"e2e = wait + exec  ({n} frames)")
hbreak(ax_wait, [("wait_ms", "wait", "orange")], "wait only")
hbreak(ax_exec, exec_seg,                        "exec only (copy/sound/tg/ui/plot)")

plt.tight_layout(rect=[0, 0, 1, 0.97])
out_png = os.path.splitext(csv_path)[0] + ".png"
plt.savefig(out_png, dpi=120)
print(f"\nSaved plot: {out_png}")

# ── system metrics (RPi): separate figure if *_sys.csv exists ─
sys_path = os.path.splitext(csv_path)[0] + "_sys.csv"
if os.path.exists(sys_path):
    scols = {}
    with open(sys_path, newline="") as f:
        rdr = csv.DictReader(f)
        sys_fields = rdr.fieldnames or []
        for row in rdr:
            for k, v in row.items():
                try: scols.setdefault(k, []).append(float(v))
                except (ValueError, TypeError): pass
    sx = scols.get("frame", [])
    if sx:
        core_keys = [k for k in sys_fields if k.startswith("cpu") and k != "cpu_total"]
        print(f"\n=== System (RPi) — {len(sx)} samples, {len(core_keys)} cores ===")
        for k in ["cpu_total"] + core_keys + ["mem_used_mb", "temp_c", "freq_mhz"]:
            if k in scols and scols[k]:
                print(f"  {k:13s} avg={statistics.mean(scols[k]):8.1f}  max={max(scols[k]):8.1f}")
        thr = [t for t in scols.get("throttled", []) if t != 0]
        if thr:
            print(f"  ** throttled events: {len(thr)} (bitmask != 0) **")

        sfig, sax = plt.subplots(4, 1, figsize=(13, 12))
        sfig.suptitle(f"TimeGrapher System Metrics (RPi)\n{os.path.basename(sys_path)}",
                      fontsize=12)
        # 1) per-core + total CPU
        sax[0].plot(sx, scols["cpu_total"], label="total", color="black", linewidth=1.5)
        for k in core_keys:
            sax[0].plot(sx, scols[k], label=k, linewidth=0.9, alpha=0.8)
        sax[0].set_ylabel("CPU %"); sax[0].set_ylim(0, 105)
        sax[0].set_title("CPU utilization (total + per core)")
        sax[0].legend(loc="upper left", fontsize=8, ncol=6); sax[0].grid(True, alpha=0.3)
        # 2) memory
        sax[1].plot(sx, scols["mem_used_mb"], label="used", color="purple", linewidth=1.2)
        if "mem_total_mb" in scols:
            sax[1].plot(sx, scols["mem_total_mb"], label="total", color="gray",
                        linestyle="--", linewidth=0.8)
        sax[1].set_ylabel("MB"); sax[1].set_title("Memory (used / total)")
        sax[1].legend(loc="upper left", fontsize=8); sax[1].grid(True, alpha=0.3)
        # 3) temperature
        sax[2].plot(sx, scols["temp_c"], label="temp", color="red", linewidth=1.2)
        sax[2].axhline(y=80, color="red", linestyle="--", alpha=0.5, linewidth=0.8,
                       label="80C throttle")
        sax[2].set_ylabel("C"); sax[2].set_title("CPU temperature")
        sax[2].legend(loc="upper left", fontsize=8); sax[2].grid(True, alpha=0.3)
        # 4) frequency + throttled markers
        sax[3].plot(sx, scols["freq_mhz"], label="freq", color="blue", linewidth=1.2)
        if any(t != 0 for t in scols.get("throttled", [])):
            for i, t in enumerate(scols["throttled"]):
                if t != 0:
                    sax[3].axvline(x=sx[i], color="red", alpha=0.3, linewidth=0.8)
        sax[3].set_ylabel("MHz"); sax[3].set_xlabel("frame")
        sax[3].set_title("CPU frequency (red lines = throttled)")
        sax[3].legend(loc="upper left", fontsize=8); sax[3].grid(True, alpha=0.3)

        plt.tight_layout(rect=[0, 0, 1, 0.96])
        sys_png = os.path.splitext(sys_path)[0] + ".png"
        plt.savefig(sys_png, dpi=120)
        print(f"Saved system plot: {sys_png}")
