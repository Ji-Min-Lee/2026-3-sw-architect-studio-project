#!/usr/bin/env python3
"""
convert_wav_float32.py — Convert 16-bit integer WAV files to 32-bit float PCM.

The TimeGrapher app requires audioFormat=3 (IEEE float), bitsPerSample=32.
These test WAVs are 16-bit integer PCM; this script converts them in-place or
to a target directory.

Usage:
  python convert_wav_float32.py <src_dir> <dst_dir>
"""
import sys
import os
import struct
import wave
import numpy as np


def convert(src_path: str, dst_path: str):
    with wave.open(src_path, "rb") as wf:
        n_channels   = wf.getnchannels()
        sampwidth    = wf.getsampwidth()
        framerate    = wf.getframerate()
        n_frames     = wf.getnframes()
        raw          = wf.readframes(n_frames)

    # decode integer PCM to float32 in [-1.0, 1.0]
    if sampwidth == 2:
        samples = np.frombuffer(raw, dtype=np.int16).astype(np.float32) / 32768.0
    elif sampwidth == 3:
        # 24-bit: unpack manually
        arr = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3)
        i32 = (arr[:, 0].astype(np.int32)
               | (arr[:, 1].astype(np.int32) << 8)
               | (arr[:, 2].astype(np.int32) << 16))
        i32 = np.where(i32 >= 0x800000, i32 - 0x1000000, i32)
        samples = i32.astype(np.float32) / 8388608.0
    elif sampwidth == 4:
        samples = np.frombuffer(raw, dtype=np.float32)  # already float
    else:
        raise ValueError(f"unsupported sample width {sampwidth}")

    # write 32-bit float PCM WAV manually (wave module only writes integer PCM)
    n_samples = len(samples)
    data_size = n_samples * 4
    with open(dst_path, "wb") as f:
        # RIFF header
        f.write(b"RIFF")
        f.write(struct.pack("<I", 36 + data_size))
        f.write(b"WAVE")
        # fmt chunk: audioFormat=3 (float), 32-bit
        f.write(b"fmt ")
        f.write(struct.pack("<I", 16))          # chunk size
        f.write(struct.pack("<H", 3))           # IEEE float
        f.write(struct.pack("<H", n_channels))
        f.write(struct.pack("<I", framerate))
        f.write(struct.pack("<I", framerate * n_channels * 4))
        f.write(struct.pack("<H", n_channels * 4))
        f.write(struct.pack("<H", 32))
        # data chunk
        f.write(b"data")
        f.write(struct.pack("<I", data_size))
        f.write(samples.tobytes())

    print(f"  {os.path.basename(src_path)} ({sampwidth*8}-bit {framerate}Hz)"
          f" -> {os.path.basename(dst_path)} (float32 {framerate}Hz)")
    return framerate


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <src_dir> <dst_dir>")
        sys.exit(1)

    src_dir = sys.argv[1]
    dst_dir = sys.argv[2]
    os.makedirs(dst_dir, exist_ok=True)

    wav_files = [f for f in os.listdir(src_dir) if f.lower().endswith(".wav")]
    if not wav_files:
        print(f"No WAV files found in {src_dir}")
        sys.exit(1)

    for fname in sorted(wav_files):
        convert(os.path.join(src_dir, fname), os.path.join(dst_dir, fname))

    print(f"\nDone — {len(wav_files)} files written to {dst_dir}")
