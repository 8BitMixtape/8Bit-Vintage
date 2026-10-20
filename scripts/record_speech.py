#!/usr/bin/env python3
"""
Interactive Voice Recorder & Sample Generator for ATtiny85 / 8Bit Mixtape.
Inspired by the original scratch/record_scream.py tool.

Records audio directly from the computer microphone using 'arecord',
normalizes volume, trims silence, resamples to fit ATtiny85 Flash budget,
and writes out a C PROGMEM header ready for compilation.

Usage:
  python3 scripts/record_speech.py -o include/my_voice.h
  python3 scripts/record_speech.py --duration 2 --length 2700
"""

import argparse
import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def record_audio(output_wav: Path, duration_sec: float = 2.0, sample_rate: int = 4000):
    """Record mono 8-bit unsigned audio from microphone using ALSA arecord."""
    print("\n" + "=" * 50)
    print("🎙️   8BIT MIXTAPE — VOICE & SAMPLE RECORDER   🎙️")
    print("=" * 50)
    print(f"Target duration: {duration_sec}s @ {sample_rate} Hz (8-bit mono)")
    input("\n👉 Press ENTER when ready to record...")

    for i in range(3, 0, -1):
        print(f"Recording in {i}...")
        time.sleep(0.7)

    print("\n🔴 RECORDING NOW! Speak into microphone...")
    cmd = [
        "arecord",
        "-d", str(int(duration_sec + 0.5)),
        "-f", "U8",
        "-r", str(sample_rate),
        "-c", "1",
        str(output_wav)
    ]
    try:
        subprocess.run(cmd, check=True)
    except FileNotFoundError:
        print("Error: 'arecord' command not found. Install ALSA utils: sudo apt install alsa-utils")
        sys.exit(1)

    print("✅ Recording finished!")


def process_wav_to_header(wav_path: Path, out_header: Path, target_len: int = 2700, var_name: str = "sample_data"):
    """Read WAV, normalize around 128, trim silence, resample to target_len, write C header."""
    with open(wav_path, "rb") as f:
        data = f.read()

    # Strip 44-byte standard RIFF WAV header
    pcm = list(data[44:]) if len(data) > 44 else list(data)

    if not pcm:
        print("Error: No audio samples found in recording.")
        return

    # Trim leading silence (values close to 128)
    thresh = 6
    start = 0
    while start < len(pcm) and abs(pcm[start] - 128) < thresh:
        start += 1
    end = len(pcm) - 1
    while end > start and abs(pcm[end] - 128) < thresh:
        end -= 1

    if end > start:
        pcm = pcm[start : end + 1]

    # Resample to target_len
    orig_len = len(pcm)
    resampled = []
    ratio = (orig_len - 1) / max(1, target_len - 1)
    for i in range(target_len):
        pos = i * ratio
        idx = int(pos)
        frac = pos - idx
        if idx + 1 < orig_len:
            val = pcm[idx] * (1.0 - frac) + pcm[idx + 1] * frac
        else:
            val = pcm[idx]
        resampled.append(val)

    # Normalize amplitude around 128
    max_dev = max(abs(s - 128) for s in resampled)
    if max_dev > 0:
        scale = 124.0 / max_dev
        print(f"Normalizing volume... (boosting by {scale:.2f}x)")
        final_samples = []
        for s in resampled:
            v = int(round((s - 128) * scale)) + 128
            v = max(0, min(255, v))
            final_samples.append(v)
    else:
        print("Warning: Audio was completely silent!")
        final_samples = [128] * target_len

    # Write formatted C header
    out_header.parent.mkdir(parents=True, exist_ok=True)
    lines = []
    lines.append("// Recorded with scripts/record_speech.py")
    lines.append("// 8-bit unsigned PCM audio sample for ATtiny85 PWM playback")
    lines.append("#pragma once")
    lines.append("#include <avr/pgmspace.h>")
    lines.append("")
    lines.append(f"const unsigned int sample_len = {len(final_samples)};")
    lines.append(f"const unsigned char {var_name}[] PROGMEM = {{")

    row = []
    for i, s in enumerate(final_samples):
        row.append(f"{s:3d}")
        if len(row) == 16 or i == len(final_samples) - 1:
            comma = "," if i < len(final_samples) - 1 else ""
            lines.append("  " + ", ".join(row) + comma)
            row = []

    lines.append("};")
    lines.append("")

    out_header.write_text("\n".join(lines))
    print(f"\n🎉 Saved {out_header} ({len(final_samples)} bytes PROGMEM).")
    print(f"Total Flash footprint with code: ~{len(final_samples) + 3400} bytes (safely under 6300 TinyAudioBoot ceiling).")


def main():
    parser = argparse.ArgumentParser(description="Record live voice and convert to ATtiny85 C header")
    parser.add_argument("-o", "--output", type=Path, default=ROOT / "include" / "speech.h", help="Output C header (default: include/speech.h)")
    parser.add_argument("-d", "--duration", type=float, default=2.0, help="Recording duration in seconds (default: 2.0s)")
    parser.add_argument("-l", "--length", type=int, default=2700, help="Target byte length (default: 2700, fits TinyAudioBoot)")
    parser.add_argument("-r", "--rate", type=int, default=3600, help="Recording sample rate in Hz (default: 3600)")
    parser.add_argument("--wav", type=Path, default=Path("/tmp/speech_record.wav"), help="Intermediate WAV file")

    args = parser.parse_args()

    record_audio(args.wav, duration_sec=args.duration, sample_rate=args.rate)
    process_wav_to_header(args.wav, args.output, target_len=args.length)


if __name__ == "__main__":
    main()
