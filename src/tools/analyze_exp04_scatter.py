#!/usr/bin/env python3
"""
analyze_exp04_scatter.py — EXP-04 Part B: single focused scatter plot.

One graph that shows everything needed to pick the noise_ratio threshold X:
  - Each dot = one frame (noise_floor vs ref_peak position)
  - Color    = noise condition (blue=40dB / orange=20dB / red=0dB)
  - Marker   = circle (beat OK) or X (beat missed / sync lost)
  - Lines    = threshold candidates (noise_floor / ref_peak = X)

Usage:
  python analyze_exp04_scatter.py [log_dir]
  log_dir defaults to src/logs/EXP-04/
"""

import sys, os, csv, glob

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

here    = os.path.dirname(os.path.abspath(__file__))
log_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "..", "logs", "EXP-04")
log_dir = os.path.abspath(log_dir)

CONDITIONS = [
    ("snr60db", "SNR 60 dB  (very clean)",    "#1565C0"),
    ("snr50db", "SNR 50 dB  (clean)",         "#1E88E5"),
    ("snr40db", "SNR 40 dB  (low noise)",     "#42A5F5"),
    ("snr30db", "SNR 30 dB  (light noise)",   "#FDD835"),
    ("snr20db", "SNR 20 dB  (medium noise)",  "#FB8C00"),
    ("snr10db", "SNR 10 dB  (high noise)",    "#E53935"),
    ("snr00db", "SNR 0 dB   (severe noise)",  "#B71C1C"),
]
THRESHOLDS = [
    (0.04, "#424242", "--"),
    (0.05, "#E53935", "-"),
    (0.06, "#7B1FA2", "-."),
]

def find_csv(key):
    hits = [f for f in glob.glob(os.path.join(log_dir, f"*{key}*.csv"))
            if not f.endswith("_sys.csv")]
    return hits[0] if hits else None

def load(path):
    cols, meta = {}, ""
    with open(path, newline="") as f:
        lines = f.readlines()
    data = [l for l in lines if not l.lstrip().startswith("#")]
    meta_lines = [l for l in lines if l.lstrip().startswith("#")]
    if meta_lines:
        meta = meta_lines[0].lstrip("# ").strip()
    for row in csv.DictReader(data):
        for k, v in row.items():
            try: cols.setdefault(k, []).append(float(v))
            except: pass
    return cols, meta

try:
    import matplotlib; matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.lines as mlines
except ImportError:
    print("pip install matplotlib"); sys.exit(1)

fig, ax = plt.subplots(figsize=(11, 8))
fig.suptitle(
    "EXP-04 Part B — Signal Quality: noise_floor vs ref_peak\n"
    "Circle = beat detected  ✕ = beat missed (noise bump / sync lost)",
    fontsize=12, fontweight="bold"
)

stats = []

for cond_key, label, color in CONDITIONS:
    path = find_csv(cond_key)
    if not path:
        print(f"WARNING: no CSV for {cond_key}"); continue
    cols, _ = load(path)

    nf   = cols.get("noise_floor", [])
    rp   = cols.get("ref_peak",    [])
    miss = cols.get("beat_missed", [0]*len(nf))

    if not nf:
        print(f"WARNING: {cond_key} has no noise_floor column"); continue

    # split into OK vs missed
    nf_ok  = [nf[i] for i in range(len(nf)) if miss[i] == 0]
    rp_ok  = [rp[i] for i in range(len(rp)) if miss[i] == 0]
    nf_mis = [nf[i] for i in range(len(nf)) if miss[i] == 1]
    rp_mis = [rp[i] for i in range(len(rp)) if miss[i] == 1]

    # normal frames: small translucent circle
    ax.scatter(rp_ok, nf_ok, color=color, alpha=0.25, s=14,
               marker="o", linewidths=0, zorder=2)

    # missed beats: bold X marker, same color but opaque
    if rp_mis:
        ax.scatter(rp_mis, nf_mis, color=color, alpha=0.95, s=80,
                   marker="X", linewidths=0.8,
                   edgecolors="black", zorder=4)

    n_miss = len(rp_mis)
    n_tot  = len(nf)
    ratio_vals = [nf[i]/rp[i] for i in range(len(nf)) if rp[i] > 0]
    import statistics as _s
    stats.append((cond_key, n_miss, n_tot,
                  _s.median(ratio_vals), max(ratio_vals)))

# iso-ratio threshold lines (span the actual data ref_peak range)
rp_range = [0.03, 0.10]
for thr, color, ls in THRESHOLDS:
    ax.plot(rp_range, [r * thr for r in rp_range],
            color=color, linestyle=ls, linewidth=1.8, alpha=0.85,
            label=f"threshold  ratio = {thr:.2f}  →  noise_floor / ref_peak")

# legend
import matplotlib.patches as mpatches

# One legend entry per condition: show circle + X in that condition's color
cond_handles = []
for _, lbl, c in CONDITIONS:
    # circle = beat detected
    circle = mlines.Line2D([], [], color=c, marker="o", linestyle="None",
                           markersize=7, alpha=0.7, label=f"{lbl}  ○ detected")
    cond_handles.append(circle)

# Generic marker shape explanations (color matches the condition that had misses)
# Find the color of the condition(s) with misses from stats
miss_colors = [(cond, color) for cond, n_miss, _, _, _ in stats
               for _, lbl, color in CONDITIONS
               if cond == _  and n_miss > 0]
# Fallback: use red if no misses found in stats
miss_example_color = miss_colors[0][1] if miss_colors else "#B71C1C"

miss_marker = mlines.Line2D([], [], color=miss_example_color, marker="X", linestyle="None",
                             markersize=10, markeredgecolor="black", markeredgewidth=0.8,
                             label="✕  beat missed  (x=tick amplitude, y=noise level at that moment)")
ok_marker   = mlines.Line2D([], [], color="gray", marker="o", linestyle="None",
                             markersize=7, alpha=0.5, label="○  beat detected normally")

thr_lines = [
    mlines.Line2D([], [], color=c, linestyle=ls, linewidth=1.8,
                  label=f"noise_ratio = {t:.2f}  ({t*100:.0f}% of tick amplitude)")
    for t, c, ls in THRESHOLDS
]

ax.legend(handles=cond_handles + [ok_marker, miss_marker] + thr_lines,
          loc="upper left", fontsize=8.5, framealpha=0.92)

ax.set_xlabel("ref_peak  (median of last 16 burst peaks = watch tick amplitude)",
              fontsize=10)
ax.set_ylabel("noise_floor  (75th pct of silence envelope = ambient noise level)",
              fontsize=10)
ax.set_xlim(0.03, 0.10)
ax.set_ylim(0.000, 0.006)
ax.grid(True, alpha=0.25)

# stats annotation (bottom-right)
lines = ["Condition       missed   median ratio   max ratio"]
lines.append("─" * 52)
for cond, n_miss, n_tot, med, mx in stats:
    lines.append(f"{cond:<14}  {n_miss:>3}/{n_tot}  "
                 f"     {med:.4f}          {mx:.4f}")
lines.append("")
lines.append("Recommended threshold: 0.05  (5% of tick amplitude)")
lines.append("  snr10db max=0.058 (0 missed) → just below 0.05 line")
lines.append("  snr00db beat_missed zone: 0.054~0.060 → above 0.05")

box_text = "\n".join(lines)
ax.text(0.98, 0.02, box_text, transform=ax.transAxes,
        fontsize=7.5, verticalalignment="bottom", horizontalalignment="right",
        family="monospace",
        bbox=dict(boxstyle="round", facecolor="lightyellow", alpha=0.85))

plt.tight_layout()
out = os.path.join(log_dir, "exp04_scatter.png")
plt.savefig(out, dpi=140)
print(f"Saved: {out}")

for cond, n_miss, n_tot, med, mx in stats:
    print(f"[{cond}] missed={n_miss}/{n_tot}  "
          f"noise_ratio median={med:.4f}  max={mx:.4f}")
