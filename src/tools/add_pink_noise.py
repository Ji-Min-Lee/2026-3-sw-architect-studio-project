#!/usr/bin/env python3
"""
add_pink_noise.py — Mix pink noise into watch audio WAV files at SNR levels.

Usage:
  python add_pink_noise.py                        # process all WAV in default dir
  python add_pink_noise.py <wav_file_or_dir>      # specific file or directory

Output: src/TimeGrapherTestFilesWeishiMic/Noise/<name>_snr<N>db.wav
  SNR 60 dB = noise 60 dB below signal (barely audible)
  SNR  0 dB = noise equals signal power (very noisy)
"""

import sys
import os
import glob
import numpy as np
from scipy.io import wavfile

NOISE_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "TimeGrapherTestFilesWeishiMic", "Noise"
)
SNR_LEVELS = [60, 50, 40, 30, 20, 10, 0]   # dB, high = clean, low = noisy


def generate_pink_noise(n_samples):
    """1/f pink noise via spectral shaping of white noise."""
    white = np.random.randn(n_samples)
    fft = np.fft.rfft(white)
    freqs = np.arange(1, len(fft) + 1, dtype=float)
    pink_filter = 1.0 / np.sqrt(freqs)
    pink = np.fft.irfft(fft * pink_filter, n_samples)
    # normalize to unit RMS
    rms = np.sqrt(np.mean(pink ** 2))
    return pink / rms if rms > 0 else pink


def mix_at_snr(signal_f32, pink_unit, snr_db):
    """Return signal + pink noise scaled to given SNR (dB)."""
    sig_rms = np.sqrt(np.mean(signal_f32 ** 2))
    noise_rms_target = sig_rms / (10 ** (snr_db / 20.0))
    noise = pink_unit * noise_rms_target

    # match length
    if len(noise) < len(signal_f32):
        reps = int(np.ceil(len(signal_f32) / len(noise)))
        noise = np.tile(noise, reps)
    noise = noise[:len(signal_f32)]

    mixed = signal_f32 + noise
    peak = np.max(np.abs(mixed))
    if peak > 1.0:
        mixed /= peak
    return mixed


def process_wav(wav_path):
    rate, data = wavfile.read(wav_path)

    # normalise to float32 [-1, 1]
    if data.dtype == np.int16:
        signal = data.astype(np.float32) / 32768.0
    elif data.dtype == np.int32:
        signal = data.astype(np.float32) / 2147483648.0
    elif data.dtype == np.float32:
        signal = data.copy()
    else:
        signal = data.astype(np.float32)

    # stereo → mono (use first channel for noise mix; keep stereo shape)
    mono = signal[:, 0] if signal.ndim == 2 else signal
    pink = generate_pink_noise(len(mono))

    base = os.path.splitext(os.path.basename(wav_path))[0]
    os.makedirs(NOISE_DIR, exist_ok=True)

    for snr in SNR_LEVELS:
        mixed_mono = mix_at_snr(mono, pink, snr)

        if signal.ndim == 2:
            out = signal.copy()
            out[:, 0] = mixed_mono
            if signal.shape[1] > 1:
                out[:, 1] = mix_at_snr(signal[:, 1], pink, snr)
        else:
            out = mixed_mono

        out_i16 = np.clip(out * 32767, -32768, 32767).astype(np.int16)
        out_path = os.path.join(NOISE_DIR, f"{base}_snr{snr:02d}db.wav")
        wavfile.write(out_path, rate, out_i16)
        print(f"  [{snr:2d} dB SNR] -> {os.path.basename(out_path)}")


def main():
    src_dir = os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "TimeGrapherTestFilesWeishiMic"
    )
    targets = []

    if len(sys.argv) > 1:
        arg = sys.argv[1]
        if os.path.isfile(arg):
            targets = [arg]
        elif os.path.isdir(arg):
            targets = sorted(glob.glob(os.path.join(arg, "*.wav")))
    else:
        targets = sorted(glob.glob(os.path.join(src_dir, "*.wav")))

    if not targets:
        print("No WAV files found.")
        sys.exit(1)

    print(f"Output dir : {NOISE_DIR}")
    print(f"SNR levels : {SNR_LEVELS} dB  (60=clean → 0=noisy)")
    print()

    for wav in targets:
        print(f"Processing : {os.path.basename(wav)}")
        process_wav(wav)
        print()

    print(f"Done. {len(targets) * len(SNR_LEVELS)} files written to {NOISE_DIR}")


if __name__ == "__main__":
    main()
