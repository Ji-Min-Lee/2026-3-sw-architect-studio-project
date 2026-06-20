#!/usr/bin/env python3
"""
gen_exp04_sample_csv.py — Generate synthetic EXP-04 Part B CSV files.

Simulates three noise conditions (snr40db / snr20db / snr00db) with
noise_floor, ref_peak, noise_ratio, and sync_lost columns added to the
standard Logger CSV format.

Usage:
  python gen_exp04_sample_csv.py

Outputs to src/logs/EXP-04/:
  sim_snr40db.csv   (low noise  — clean signal)
  sim_snr20db.csv   (medium noise)
  sim_snr00db.csv   (high noise — detector struggles)
"""

import csv
import math
import os
import random

random.seed(42)

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "..", "logs", "EXP-04")
os.makedirs(OUT_DIR, exist_ok=True)

FRAMES   = 600          # ~12.5 s at 48k/1024 SPF (21.3 ms per frame)
SPF      = 1024
SPS      = 48000
DEADLINE = SPF / SPS * 1000.0   # 21.33 ms

def clamp(v, lo, hi):
    return max(lo, min(hi, v))

def gauss(mu, sigma, lo, hi):
    return clamp(random.gauss(mu, sigma), lo, hi)

# Warmup ramp: fps/sps reach steady state after ~50 frames
def ramp(i, target, warmup=50):
    return target * min(1.0, i / warmup)

HEADER = (
    "frame,samples,block_drops,buffer_pct,"
    "total_ms,wait_ms,exec_ms,"
    "copy_ms,sound_ms,tg_ms,ui_ms,plot_ms,"
    "bg_fps,bg_sps,bg_spf,fg_fps,fg_sps,fg_spf,"
    "noise_floor,ref_peak,noise_ratio,sync_lost,beat_missed"
)

# noise_floor and ref_peak characteristics per condition.
#
# Key constraint from Detector.cpp:550:
#   reference_peak = max(median_of_peaks, 10 * noise_floor)
# → noise_ratio = noise_floor / reference_peak  is ALWAYS <= 0.10
#
# Physical interpretation:
#   snr40db: noise_floor << ref_peak  → ratio ~0.015-0.030 (clamp inactive)
#   snr20db: noise_floor rises        → ratio ~0.045-0.070 (clamp activates intermittently)
#   snr00db: noise ≈ signal amplitude → ratio ~0.075-0.100 (clamp active most frames)
CONDITIONS = {
    "snr40db": dict(
        noise_floor_mu=0.005, noise_floor_sigma=0.0015,
        noise_floor_lo=0.001, noise_floor_hi=0.012,
        ref_peak_mu=0.22,     ref_peak_sigma=0.025,
        ref_peak_lo=0.14,     ref_peak_hi=0.32,
        tg_mu=2.05,           tg_sigma=0.35,
        sync_lost_prob=0.0,
        beat_missed_prob=0.00,   # clean signal: 0% missed
    ),
    "snr20db": dict(
        noise_floor_mu=0.014, noise_floor_sigma=0.003,
        noise_floor_lo=0.006, noise_floor_hi=0.025,
        ref_peak_mu=0.22,     ref_peak_sigma=0.025,
        ref_peak_lo=0.14,     ref_peak_hi=0.32,
        tg_mu=2.15,           tg_sigma=0.45,
        sync_lost_prob=0.0,
        beat_missed_prob=0.03,   # medium noise: ~3% missed (noise bumps
                                 # mis-classified or ticks below min_peak)
    ),
    "snr00db": dict(
        noise_floor_mu=0.019, noise_floor_sigma=0.003,
        noise_floor_lo=0.010, noise_floor_hi=0.026,
        ref_peak_mu=0.22,     ref_peak_sigma=0.025,
        ref_peak_lo=0.14,     ref_peak_hi=0.32,
        tg_mu=2.40,           tg_sigma=0.65,
        sync_lost_prob=0.010,
        beat_missed_prob=0.12,   # high noise: ~12% missed ticks
    ),
}

for cond_name, p in CONDITIONS.items():
    fname = os.path.join(OUT_DIR, f"sim_{cond_name}.csv")
    with open(fname, "w", newline="") as f:
        f.write(f"# platform=debian kernel=linux host=lg1 "
                f"device=rpi2 git_commit=synthetic sample_rate={SPS} "
                f"noise_condition={cond_name}\n")
        f.write(HEADER + "\n")

        for i in range(1, FRAMES + 1):
            warm    = min(1.0, i / 60)
            bg_fps  = gauss(43.5, 0.3, 40.0, 47.5) * warm
            fg_fps  = gauss(43.5, 0.3, 40.0, 47.5) * warm
            bg_sps  = bg_fps * SPF if bg_fps > 0 else 0.0

            tg_ms   = gauss(p["tg_mu"], p["tg_sigma"], 0.8, 6.0) * warm
            copy_ms = gauss(0.007, 0.002, 0.003, 0.015)
            exec_ms = copy_ms + tg_ms
            wait_ms = gauss(0.030, 0.015, 0.005, 0.150)
            total_ms = wait_ms + exec_ms

            nf = gauss(p["noise_floor_mu"], p["noise_floor_sigma"],
                       p["noise_floor_lo"], p["noise_floor_hi"])
            rp = gauss(p["ref_peak_mu"],    p["ref_peak_sigma"],
                       p["ref_peak_lo"],    p["ref_peak_hi"])
            # ref_peak always clamped above 10x noise_floor (Detector.cpp:550)
            rp = max(rp, nf * 10.0)
            ratio = nf / rp

            sync_lost    = 1 if (random.random() < p["sync_lost_prob"])    else 0
            beat_missed  = 1 if (random.random() < p["beat_missed_prob"])  else 0
            # sync_lost frames always count as a missed beat too
            if sync_lost:
                beat_missed = 1

            f.write(
                f"{i},{SPF},0,{gauss(2.5,1.0,0.0,15.0):.1f},"
                f"{total_ms:.3f},{wait_ms:.3f},{exec_ms:.3f},"
                f"{copy_ms:.3f},0.000,{tg_ms:.3f},0.000,0.000,"
                f"{bg_fps:.1f},{bg_sps:.1f},{float(SPF):.1f},"
                f"{fg_fps:.1f},{fg_fps*SPF:.1f},{float(SPF):.1f},"
                f"{nf:.5f},{rp:.5f},{ratio:.5f},{sync_lost},{beat_missed}\n"
            )

    print(f"Wrote {FRAMES} frames -> {fname}")

print("\nDone. Run analyze_exp04_noise.py to visualize.")
