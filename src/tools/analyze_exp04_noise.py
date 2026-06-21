#!/usr/bin/env python3
"""
analyze_exp04_noise.py — EXP-04 Part B: noise_ratio threshold visualization.

Reads three per-condition CSV files (snr40db / snr20db / snr00db) produced by
the logging build (or the synthetic generator) and plots:

  Panel 1: noise_ratio time-series for all three conditions + threshold candidates
  Panel 2: noise_ratio distribution (histogram overlay)
  Panel 3: box plot comparison across conditions
  Panel 4: ref_peak vs noise_floor scatter (per condition)

Usage:
  python analyze_exp04_noise.py [log_dir]

  log_dir defaults to src/logs/EXP-04/
  Expected filenames: *snr40db*.csv  *snr20db*.csv  *snr00db*.csv

Requires: matplotlib  (pip install matplotlib)
"""

import sys
import os
import csv
import glob

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

# ── locate CSVs ───────────────────────────────────────────────────────────────
here     = os.path.dirname(os.path.abspath(__file__))
log_dir  = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "..", "logs", "EXP-04")
log_dir  = os.path.abspath(log_dir)

CONDITIONS = [
    ("snr40db", "Low noise (SNR 40 dB)",    "#2196F3"),   # blue
    ("snr20db", "Medium noise (SNR 20 dB)", "#FF9800"),   # orange
    ("snr00db", "High noise (SNR 0 dB)",    "#F44336"),   # red
]

# Threshold candidates to evaluate (noise_floor / ref_peak)
# Optimal X lives somewhere between snr20db and snr00db distributions
# Threshold candidates (noise_ratio <= 0.10 always due to Detector.cpp clamp)
THRESHOLD_CANDIDATES = [0.04, 0.06, 0.08]

def find_csv(cond_key):
    pattern = os.path.join(log_dir, f"*{cond_key}*.csv")
    files = [f for f in glob.glob(pattern) if not f.endswith("_sys.csv")]
    return files[0] if files else None

def load_csv(path):
    cols = {}
    meta = ""
    with open(path, newline="") as f:
        lines = f.readlines()
    data_lines = []
    for ln in lines:
        if ln.lstrip().startswith("#"):
            if not meta:
                meta = ln.lstrip("# ").strip()
        else:
            data_lines.append(ln)
    for row in csv.DictReader(data_lines):
        for k, v in row.items():
            try:
                cols.setdefault(k, []).append(float(v))
            except (ValueError, TypeError):
                pass
    return cols, meta

# ── load data ─────────────────────────────────────────────────────────────────
data = {}
for cond_key, label, color in CONDITIONS:
    path = find_csv(cond_key)
    if not path:
        print(f"WARNING: no CSV found for {cond_key} in {log_dir}")
        continue
    cols, meta = load_csv(path)
    if "noise_ratio" not in cols:
        print(f"WARNING: {path} has no noise_ratio column — run logging build first")
        continue
    data[cond_key] = {"cols": cols, "meta": meta, "label": label, "color": color}
    n = len(cols["noise_ratio"])
    ratios = cols["noise_ratio"]
    import statistics
    print(f"[{cond_key}] n={n}  noise_ratio: "
          f"avg={statistics.mean(ratios):.4f}  "
          f"median={statistics.median(ratios):.4f}  "
          f"max={max(ratios):.4f}  "
          f"min={min(ratios):.4f}")
    sync_losts = int(sum(cols.get("sync_lost", [0.0])))
    if sync_losts:
        print(f"           sync_lost events: {sync_losts}")

if not data:
    print("No valid CSVs loaded. Run gen_exp04_sample_csv.py first.")
    sys.exit(1)

# ── plot ──────────────────────────────────────────────────────────────────────
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
except ImportError:
    print("(matplotlib not installed -> pip install matplotlib)")
    sys.exit(0)

SPF = 1024; SPS = 48000
frame_ms = SPF / SPS * 1000.0   # 21.33 ms per frame

fig, axes = plt.subplots(2, 2, figsize=(16, 11))
fig.suptitle("EXP-04 Part B — noise_floor / reference_peak Analysis\n"
             "Goal: find threshold X where noise_ratio > X triggers ⚠ Noisy signal",
             fontsize=13, fontweight="bold")

ax_ts   = axes[0, 0]   # time-series
ax_hist = axes[0, 1]   # histogram
ax_box  = axes[1, 0]   # box plot
ax_scat = axes[1, 1]   # scatter ref_peak vs noise_floor

thr_colors  = ["#555555", "#9C27B0", "#009688"]
thr_styles  = ["--", "-.", ":"]

# ── Panel 1: time-series ──────────────────────────────────────────────────────
for cond_key, label, color in CONDITIONS:
    if cond_key not in data:
        continue
    cols = data[cond_key]["cols"]
    ratios = cols["noise_ratio"]
    x_sec = [i * frame_ms / 1000.0 for i in range(len(ratios))]
    ax_ts.plot(x_sec, ratios, color=color, linewidth=0.9, alpha=0.75, label=label)

    # mark sync_lost events
    sync_losts = cols.get("sync_lost", [])
    for i, sl in enumerate(sync_losts):
        if sl > 0:
            ax_ts.axvline(x=x_sec[i], color=color, linewidth=1.5,
                          linestyle=":", alpha=0.9)

for thr, tc, ts in zip(THRESHOLD_CANDIDATES, thr_colors, thr_styles):
    ax_ts.axhline(y=thr, color=tc, linestyle=ts, linewidth=1.4, alpha=0.85,
                  label=f"threshold candidate {thr:.2f}")

ax_ts.set_xlabel("time (s)")
ax_ts.set_ylabel("noise_ratio  (noise_floor / ref_peak)")
ax_ts.set_ylim(0, 1.0)
ax_ts.set_title("Panel 1 — noise_ratio over time  (dotted vertical = sync_lost)")
ax_ts.legend(loc="upper right", fontsize=8)
ax_ts.grid(True, alpha=0.3)

# ── Panel 2: histogram ────────────────────────────────────────────────────────
bins = [i * 0.025 for i in range(41)]   # 0.0 to 1.0 in 0.025 steps
for cond_key, label, color in CONDITIONS:
    if cond_key not in data:
        continue
    ratios = data[cond_key]["cols"]["noise_ratio"]
    ax_hist.hist(ratios, bins=bins, color=color, alpha=0.55, label=label,
                 edgecolor="white", linewidth=0.3)

for thr, tc, ts in zip(THRESHOLD_CANDIDATES, thr_colors, thr_styles):
    ax_hist.axvline(x=thr, color=tc, linestyle=ts, linewidth=1.6,
                    label=f"thr {thr:.2f}")

ax_hist.set_xlabel("noise_ratio")
ax_hist.set_ylabel("frame count")
ax_hist.set_title("Panel 2 — Distribution (histogram)")
ax_hist.legend(loc="upper right", fontsize=8)
ax_hist.grid(True, alpha=0.3)

# ── Panel 3: box plot ─────────────────────────────────────────────────────────
box_data   = []
box_labels = []
box_colors = []
for cond_key, label, color in CONDITIONS:
    if cond_key not in data:
        continue
    box_data.append(data[cond_key]["cols"]["noise_ratio"])
    box_labels.append(cond_key.replace("snr", "SNR ").replace("db", " dB"))
    box_colors.append(color)

bp = ax_box.boxplot(box_data, patch_artist=True, labels=box_labels,
                    medianprops=dict(color="black", linewidth=2))
for patch, color in zip(bp["boxes"], box_colors):
    patch.set_facecolor(color)
    patch.set_alpha(0.6)

for thr, tc, ts in zip(THRESHOLD_CANDIDATES, thr_colors, thr_styles):
    ax_box.axhline(y=thr, color=tc, linestyle=ts, linewidth=1.4, alpha=0.85,
                   label=f"thr {thr:.2f}")

ax_box.set_ylabel("noise_ratio")
ax_box.set_title("Panel 3 — Box plot comparison")
ax_box.legend(loc="upper left", fontsize=8)
ax_box.grid(True, alpha=0.3, axis="y")
ax_box.set_ylim(0, 1.0)

# ── Panel 4: scatter noise_floor vs ref_peak ──────────────────────────────────
for cond_key, label, color in CONDITIONS:
    if cond_key not in data:
        continue
    cols = data[cond_key]["cols"]
    nf = cols.get("noise_floor", [])
    rp = cols.get("ref_peak", [])
    if nf and rp:
        ax_scat.scatter(rp, nf, color=color, alpha=0.25, s=6, label=label)

# iso-ratio lines
rp_range = [0.05, 0.40]
for thr, tc, ts in zip(THRESHOLD_CANDIDATES, thr_colors, thr_styles):
    nf_line = [r * thr for r in rp_range]
    ax_scat.plot(rp_range, nf_line, color=tc, linestyle=ts, linewidth=1.4,
                 label=f"ratio={thr:.2f}")

ax_scat.set_xlabel("ref_peak  (median of last 16 burst peaks)")
ax_scat.set_ylabel("noise_floor  (75th pct of silence envelope)")
ax_scat.set_title("Panel 4 — noise_floor vs ref_peak scatter\n"
                  "(diagonal lines = iso-ratio; points above line → noisy)")
ax_scat.legend(loc="upper left", fontsize=8)
ax_scat.grid(True, alpha=0.3)

# ── summary stats box ─────────────────────────────────────────────────────────
import statistics as _stat

summary_lines = ["Threshold decision guide:"]
for cond_key, label, color in CONDITIONS:
    if cond_key not in data:
        continue
    r = data[cond_key]["cols"]["noise_ratio"]
    summary_lines.append(
        f"  {cond_key}: median={_stat.median(r):.3f}  "
        f"p95={sorted(r)[int(len(r)*0.95)]:.3f}"
    )
summary_lines.append("")
summary_lines.append("Pick X between snr20db p95 and snr00db p05:")
for cond_key, _, _ in CONDITIONS:
    if cond_key not in data:
        continue
    r = sorted(data[cond_key]["cols"]["noise_ratio"])
    p05 = r[max(0, int(len(r)*0.05))]
    p95 = r[min(len(r)-1, int(len(r)*0.95))]
    summary_lines.append(f"  {cond_key}: p05={p05:.3f}  p95={p95:.3f}")

print("\n" + "\n".join(summary_lines))

fig.text(0.01, 0.01, "\n".join(summary_lines), fontsize=7.5,
         verticalalignment="bottom", family="monospace",
         bbox=dict(boxstyle="round", facecolor="lightyellow", alpha=0.8))

plt.tight_layout(rect=[0, 0.12, 1, 0.95])

out_png = os.path.join(log_dir, "exp04_noise_ratio_analysis.png")
plt.savefig(out_png, dpi=130)
print(f"\nSaved: {out_png}")
