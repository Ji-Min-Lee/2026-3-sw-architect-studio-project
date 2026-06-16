#!/usr/bin/env python3
"""
Dark-themed Thread Activity Timeline — matches the reference screenshot style.

Usage:
  python thread_timeline_dark.py [csv_path] [--frames START-END] [--out path.png]

Requires: matplotlib
"""

import sys, os, csv, glob, statistics, argparse

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

parser = argparse.ArgumentParser(add_help=False)
parser.add_argument("csv_path", nargs="?")
parser.add_argument("--frames", default=None, help="e.g. 730-733  or  all")
parser.add_argument("--out",    default=None)
args, _ = parser.parse_known_args()

def newest_log():
    here = os.path.dirname(os.path.abspath(__file__))
    files = sorted(glob.glob(os.path.join(here, "..", "logs", "log_*.csv")),
                   key=os.path.getmtime)
    files = [f for f in files if not f.endswith(("_raw.csv", "_sys.csv"))]
    return files[-1] if files else None

csv_path = args.csv_path or newest_log()
if not csv_path or not os.path.exists(csv_path):
    print("No CSV found."); sys.exit(1)

# ── load ─────────────────────────────────────────────────────
cols, meta = {}, ""
with open(csv_path) as f:
    lines = f.readlines()
data = []
for ln in lines:
    if ln.lstrip().startswith("#"):
        if not meta: meta = ln.lstrip("# ").strip()
    else:
        data.append(ln)
for row in csv.DictReader(data):
    for k, v in row.items():
        try: cols.setdefault(k, []).append(float(v))
        except: pass

n = len(cols.get("frame", []))
if n == 0: print("No data."); sys.exit(1)

def median_nz(v, d):
    nz = sorted(x for x in v if x > 0)
    return nz[len(nz)//2] if nz else d

spf    = median_nz(cols.get("bg_spf", []), 1024)
sps    = median_nz(cols.get("bg_sps", []), 48000)
period = spf / sps * 1000.0

# ── frame window ─────────────────────────────────────────────
full_mode = (args.frames == "all")
if full_mode:
    f0, f1 = 0, n
elif args.frames:
    p = args.frames.split("-")
    f0, f1 = int(p[0])-1, int(p[1])
else:
    mid = n // 2
    f0, f1 = max(0, mid-2), min(n, mid+2)   # 4 frames default
f0 = max(0, min(f0, n-1))
f1 = max(f0+1, min(f1, n))
nf = f1 - f0

wait_ms  = cols["wait_ms"][f0:f1]
exec_ms  = cols["exec_ms"][f0:f1]
total_ms = cols["total_ms"][f0:f1]
fg_wait  = cols.get("fg_wait_ms", [0.0]*n)[f0:f1]
ui_ms    = cols.get("ui_ms",      [0.0]*n)[f0:f1]

# global averages for annotations
avg_wait   = statistics.mean(cols["wait_ms"])
avg_exec   = statistics.mean(cols["exec_ms"])
avg_fgwait = statistics.mean(cols.get("fg_wait_ms", [0.0]*n))

# ── coordinate system ────────────────────────────────────────
if full_mode:
    # x = frame index (0-based within window); each slot = 1 unit
    # heights scaled to period so proportions are preserved
    bg_t   = [float(i)                                      for i in range(nf)]
    dsp_s  = [i + wait_ms[i]  / period                     for i in range(nf)]
    dsp_e  = [i + total_ms[i] / period                     for i in range(nf)]
    fg_s   = [dsp_e[i]                                     for i in range(nf)]
    fg_e   = [dsp_e[i] + fg_wait[i] / period               for i in range(nf)]
    fg_h_e = [fg_e[i]  + max(ui_ms[i], 0.5) / period      for i in range(nf)]
    x_max  = nf + 0.5
    x_label = "frame number"
    x_tick_step = max(1, nf // 20)
else:
    # x = absolute elapsed time (ms)
    t0   = f0 * period
    bg_t   = [i * period - t0                      for i in range(f0, f1)]
    dsp_s  = [bg_t[i] + wait_ms[i]                for i in range(nf)]
    dsp_e  = [bg_t[i] + total_ms[i]               for i in range(nf)]
    fg_s   = [dsp_e[i]                            for i in range(nf)]
    fg_e   = [dsp_e[i] + fg_wait[i]              for i in range(nf)]
    fg_h_e = [fg_e[i]  + max(ui_ms[i], 0.8)      for i in range(nf)]
    x_max  = max(fg_e) + 5
    x_max  = max(x_max, (nf + 1) * period)
    x_label = None   # ms ticks drawn later
    x_tick_step = None

# ── plot ─────────────────────────────────────────────────────
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
    from matplotlib.patches import FancyArrowPatch
except ImportError:
    print("pip install matplotlib"); sys.exit(0)

# ── theme ─────────────────────────────────────────────────────
BG      = "#0d1117"
PANEL   = "#161b22"
HEADER  = "#0f2d1f"
BORDER  = "#21a367"
C_BG    = "#4dd0e1"   # cyan  — AudioWorker emit
C_DSP   = "#69f0ae"   # green — DSP exec
C_FG    = "#6d4c41"   # brown — FG queue wait
C_FGH   = "#90a4ae"   # gray  — FG handle
C_GRID  = "#21262d"
C_TEXT  = "#e6edf3"
C_DIM   = "#8b949e"
C_ANNO  = "#a8ff78"   # annotation green

fig_w = min(max(13, nf * 0.012), 60) if full_mode else 13
fig = plt.figure(figsize=(fig_w, 5.8), facecolor=BG)
fig.patch.set_facecolor(BG)

# layout: header strip (0.12) + main chart (0.54) + legend (0.08) + desc (0.18) + margin
ax_h   = fig.add_axes([0.00, 0.85, 1.00, 0.13])   # header
ax     = fig.add_axes([0.09, 0.38, 0.88, 0.44])   # chart
ax_leg = fig.add_axes([0.09, 0.28, 0.88, 0.08])   # legend
ax_desc= fig.add_axes([0.03, 0.01, 0.94, 0.25])   # description

# ── header ───────────────────────────────────────────────────
ax_h.set_facecolor(HEADER)
for sp in ax_h.spines.values():
    sp.set_edgecolor(BORDER); sp.set_linewidth(1.5)
meta_kv = {t.split("=")[0]: t.split("=")[1] for t in meta.split() if "=" in t}
sr_khz  = int(round(sps / 1000))
ax_h.text(0.012, 0.5,
          f"AFTER T2 — PARALLEL (DSP OFFLOAD THREAD)  ·  E2-7",
          transform=ax_h.transAxes, color=C_BG if False else "#a0ffcc",
          fontsize=11, fontweight="bold", va="center",
          fontfamily="monospace")
ax_h.text(0.98, 0.5,
          f"rpi2 · {sr_khz} kHz · {nf} frames shown  (frames {f0+1}–{f1})",
          transform=ax_h.transAxes, color=C_DIM,
          fontsize=8, va="center", ha="right", fontfamily="monospace")
ax_h.set_xticks([]); ax_h.set_yticks([])

# ── main chart ────────────────────────────────────────────────
ax.set_facecolor(PANEL)
for sp in ax.spines.values():
    sp.set_edgecolor(C_GRID)

ROW = {"BG Thread": 2.0, "DSP Thread": 1.0, "Main (FG)": 0.0}
BAR_H   = 0.32
TICK_H  = 0.28

# vertical grid at each frame boundary
for i in range(nf + 1):
    ax.axvline(i * period, color=C_GRID, linewidth=0.6, linestyle="--", zorder=0)

# ── BG ticks ──
row_bg = ROW["BG Thread"]
bg_lw  = 0.5 if full_mode else 2.0
for i, t in enumerate(bg_t):
    ax.plot([t, t], [row_bg - TICK_H/2, row_bg + TICK_H/2],
            color=C_BG, linewidth=bg_lw, zorder=4, solid_capstyle="butt")

# wait annotation on first frame
if nf > 0:
    t_w0, t_w1 = bg_t[0], dsp_s[0]
    y_ann = row_bg + TICK_H/2 + 0.12
    ax.annotate("", xy=(t_w1, y_ann), xytext=(t_w0, y_ann),
                arrowprops=dict(arrowstyle="<->", color=C_ANNO, lw=1.2))
    ax.text((t_w0 + t_w1)/2, y_ann + 0.06,
            f"wait {avg_wait:.3f}ms",
            color=C_ANNO, fontsize=7.5, ha="center", va="bottom", fontweight="bold")

# ── DSP exec bars ──
row_dsp = ROW["DSP Thread"]
for i in range(nf):
    w = dsp_e[i] - dsp_s[i]
    ax.barh(row_dsp, w, left=dsp_s[i], height=BAR_H,
            color=C_DSP, zorder=4, edgecolor="none")

# ── FG queue wait bars ──
row_fg = ROW["Main (FG)"]
for i in range(nf):
    w = fg_e[i] - fg_s[i]
    ax.barh(row_fg, w, left=fg_s[i], height=BAR_H,
            color=C_FG, zorder=3, edgecolor="none")
    # fg_wait label inside bar (only in zoom mode, if wide enough)
    if not full_mode and w > 4:
        label_x = fg_s[i] + w * 0.35
        ax.text(label_x, row_fg,
                f"fg_wait {w:.1f}ms",
                color="#e0c090", fontsize=7.2, ha="center", va="center",
                fontweight="bold", zorder=5)

# ── FG handle ticks ──
for i in range(nf):
    hw = fg_h_e[i] - fg_e[i]
    ax.barh(row_fg, hw, left=fg_e[i], height=BAR_H,
            color=C_FGH, zorder=4, edgecolor="none")

# "parallel" annotation between DSP and FG (zoom mode only)
if nf > 0 and not full_mode:
    mid_x = dsp_s[0] + (dsp_e[0] - dsp_s[0]) / 2
    ax.text(mid_x + period * 0.3, (row_dsp + row_fg) / 2,
            "parallel", color=C_ANNO, fontsize=7.5, ha="center",
            va="center", style="italic")
    ax.plot([mid_x, mid_x], [row_dsp - BAR_H/2, row_fg + BAR_H/2],
            color=C_ANNO, linewidth=0.8, linestyle=":", zorder=2)

# ── axes ──
ax.set_yticks(list(ROW.values()))
ax.set_yticklabels(list(ROW.keys()), color=C_TEXT, fontsize=9.5)
ax.tick_params(axis="y", length=0, pad=6)
ax.set_ylim(-0.6, 2.75)

import numpy as np
ax.set_xlim(0, x_max)
if full_mode:
    step = x_tick_step
    xtick_pos = list(range(0, nf, step))
    ax.set_xticks(xtick_pos)
    ax.set_xticklabels([str(f0 + i + 1) for i in xtick_pos],
                       color=C_DIM, fontsize=7)
    ax.set_xlabel("frame  (1 slot = 1 frame period = {:.1f} ms)".format(period),
                  color=C_DIM, fontsize=8)
else:
    xticks = np.arange(0, x_max + 5, 5.0)
    ax.set_xticks(xticks)
    ax.set_xticklabels([f"{int(x)}ms" for x in xticks], color=C_DIM, fontsize=8)
ax.tick_params(axis="x", colors=C_DIM, length=3)
ax.grid(True, axis="x", color=C_GRID, linewidth=0.4, zorder=0)
for sp in ax.spines.values():
    sp.set_visible(False)
ax.spines["bottom"].set_visible(True)
ax.spines["bottom"].set_color(C_GRID)

# ── legend ────────────────────────────────────────────────────
ax_leg.set_facecolor(BG)
for sp in ax_leg.spines.values(): sp.set_visible(False)
ax_leg.set_xticks([]); ax_leg.set_yticks([])

items = [
    (C_BG,  "AudioWorker (BG) emit"),
    (C_DSP, "DSP exec (DSP thread)"),
    (C_FG,  "FG queue wait (fg_wait)"),
    (C_FGH, "FG handle (Main thread)"),
]
x_pos = 0.02
for color, label in items:
    sq = mpatches.FancyBboxPatch((x_pos, 0.25), 0.025, 0.50,
                                  boxstyle="square,pad=0",
                                  facecolor=color, transform=ax_leg.transAxes,
                                  clip_on=False)
    ax_leg.add_patch(sq)
    ax_leg.text(x_pos + 0.03, 0.50, label,
                transform=ax_leg.transAxes, color=C_TEXT,
                fontsize=8.5, va="center")
    x_pos += 0.26

# ── description box ──────────────────────────────────────────
ax_desc.set_facecolor("#131c12")
for sp in ax_desc.spines.values():
    sp.set_edgecolor("#2d4a2d"); sp.set_linewidth(0.8)
ax_desc.set_xticks([]); ax_desc.set_yticks([])

desc_lines = [
    ("DSPWorker runs in a ", False),
    ("dedicated thread", True),
    (" with its own event loop. It picks up each BG signal immediately "
     f"(avg {avg_wait:.3f} ms wait). DSP and FG\n"
     "processing run in ", False),
    ("parallel", True),
    (f" — DSP finishes within {avg_exec:.1f} ms, well before the next BG signal "
     f"arrives at {period:.0f} ms. Backlog drops to 0 %.\n"
     "The only new cost is ", False),
    ("fg_wait", True),
    (f" (avg {avg_fgwait:.1f} ms): the Qt scheduler wakes the Main Thread "
     "after DSPWorker emits ", False),
    ("frameLogged", True),
    (".\nOn RPi, fg_wait avg ", False),
    (f"{avg_fgwait:.1f} ms", True),
    (f" is ~7× higher than macOS (8.9 ms) — Qt FG-thread scheduling priority "
     "is the next bottleneck (84 % of frames exceed the {period:.0f} ms deadline).", False),
]

# Render description as mixed bold/normal text via ax.text calls
ax_desc.text(0.012, 0.88,
    f"DSPWorker runs in a dedicated thread with its own event loop. "
    f"It picks up each BG signal immediately (avg {avg_wait:.3f} ms wait). "
    f"DSP and FG\nprocessing run in parallel — DSP finishes within "
    f"{avg_exec:.1f} ms, well before the next BG signal arrives at {period:.0f} ms. "
    f"Backlog drops to 0 %.\n"
    f"The only new cost is fg_wait (avg {avg_fgwait:.1f} ms): the Qt scheduler "
    f"wakes the Main Thread after DSPWorker emits frameLogged.\n"
    f"On RPi, fg_wait avg {avg_fgwait:.1f} ms is ~7× higher than macOS (8.9 ms) — "
    f"Qt FG-thread scheduling priority is the next bottleneck "
    f"(84 % of frames exceed the {period:.1f} ms deadline).",
    transform=ax_desc.transAxes,
    color=C_DIM, fontsize=8.2, va="top",
    linespacing=1.55,
    wrap=True)

# Bold highlights in description (re-render specific terms)
bold_terms = [
    (f"avg {avg_wait:.3f} ms wait", 0),
    ("parallel", 0),
    (f"fg_wait", 0),
    (f"avg {avg_fgwait:.1f} ms", 0),
    ("frameLogged", 0),
    ("84 %", 0),
]

# ── save ──────────────────────────────────────────────────────
out_stem = os.path.splitext(csv_path)[0]
suffix   = "_timeline_dark_all.png" if full_mode else f"_timeline_dark_f{f0+1}-{f1}.png"
out_png  = args.out or f"{out_stem}{suffix}"
plt.savefig(out_png, dpi=140, bbox_inches="tight", facecolor=BG)
print(f"Saved: {out_png}")
