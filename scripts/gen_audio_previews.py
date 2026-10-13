#!/usr/bin/env python3
"""
Synthesize 8-bit audio preview samples (.mp3) for each sound application
using the exact sound algorithms from the firmware C++ source code.

Usage:
  python3 scripts/gen_audio_previews.py

Outputs:
  docs/audio/<app_id>.mp3
"""
import io
import math
import os
import subprocess
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
AUDIO_DIR = ROOT / "docs" / "audio"


def write_wav_to_mp3(samples_uint8, sample_rate, out_mp3_path):
    """Write unsigned 8-bit mono PCM samples to an MP3 via ffmpeg."""
    wav_io = io.BytesIO()
    with wave.open(wav_io, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(1) # 8-bit unsigned
        wf.setframerate(sample_rate)
        wf.writeframes(bytes(samples_uint8))
    
    wav_bytes = wav_io.getvalue()
    cmd = [
        "ffmpeg", "-y",
        "-f", "wav", "-i", "pipe:0",
        "-codec:a", "libmp3lame",
        "-b:a", "96k",
        str(out_mp3_path)
    ]
    proc = subprocess.run(cmd, input=wav_bytes, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if proc.returncode == 0:
        print(f"==> Wrote {out_mp3_path.relative_to(ROOT)} ({out_mp3_path.stat().st_size // 1024} KB)")
    else:
        print(f"!! ffmpeg error writing {out_mp3_path}")


# ---------------------------------------------------------------------------
# 1. Drone Synth: Dual-oscillator with 4 audio modes (Triangle, Fuzz, Crush, RingMod)
# ---------------------------------------------------------------------------
def synth_drone():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase1 = 0
    phase2 = 0
    base_pitch = 140.0
    detune = 2.5 # slight beating

    for i in range(total_samples):
        # 4 modes, 1.5s each
        mode = int((i / total_samples) * 4)

        # Gentle pitch LFO
        lfo = math.sin(i * 0.003) * 3.0
        inc1 = int((base_pitch + lfo) * (65536.0 / sample_rate))
        inc2 = int((base_pitch + detune - lfo * 0.5) * (65536.0 / sample_rate))

        phase1 = (phase1 + inc1) & 0xFFFF
        phase2 = (phase2 + inc2) & 0xFFFF

        # Triangle 1
        t1 = phase1 >> 8
        if t1 > 127: t1 = 255 - t1
        out1 = (t1 << 1) & 0xFF

        # Triangle 2
        t2 = phase2 >> 8
        if t2 > 127: t2 = 255 - t2
        out2 = (t2 << 1) & 0xFF

        if mode == 0:
            val = (out1 + out2) >> 1
        elif mode == 1:
            val = min(255, out1 + out2)
        elif mode == 2:
            val = ((out1 & 0xE0) + (out2 & 0xE0)) >> 1
        else:
            val = ((out1 * out2) >> 8) & 0xFF

        samples[i] = val

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 2. Peepsy Acid: TB-303 lowpass resonant SVF filter, slides, accents, kick & hat
# ---------------------------------------------------------------------------
def synth_acid():
    sample_rate = 15600
    pitches = [137, 274, 137, 137, 274, 137, 205, 244, 137, 326, 137, 183, 274, 137, 326, 548]
    slides  = [0,   1,   0,   0,   0,   0,   1,   0,   0,   1,   0,   0,   0,   0,   1,   0]
    accents = [1,   0,   0,   1,   1,   0,   0,   1,   1,   0,   0,   1,   1,   0,   0,   1]
    drums   = [1,   0,   2,   0,   1,   0,   2,   0,   1,   2,   2,   1,   1,   0,   2,   2] # 1=Kick, 2=Hat

    step_duration = 0.115 # 115 ms per step
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 32 # 2 bars
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    bass_phase = 0
    bass_pitch = pitches[0]
    bass_target_pitch = pitches[0]
    bass_env = 0
    filter_val = 0.0
    filter_vel = 0.0

    drum_phase = 0
    drum_pitch = 0
    drum_env = 0
    drum_type = 0
    lfsr = 0xACE1

    sample_idx = 0
    for step in range(total_steps):
        s_idx = step % 16
        # Trigger drum
        d = drums[s_idx]
        if d == 1:
            drum_type = 0
            drum_pitch = 1000
            drum_env = 255
        elif d == 2:
            drum_type = 1
            drum_env = 120

        # Trigger bass
        next_p = pitches[s_idx]
        if slides[s_idx]:
            bass_target_pitch = next_p
        else:
            bass_pitch = next_p
            bass_target_pitch = next_p
            bass_env = 255 if accents[s_idx] else 100

        # Render step samples
        for _ in range(samples_per_step):
            # Envelopes decay
            if (sample_idx & 7) == 0:
                if bass_env > 1: bass_env -= 2
                else: bass_env = 0

                if drum_type == 0:
                    if drum_env > 0: drum_env -= 1
                else:
                    if drum_env > 7: drum_env -= 8
                    else: drum_env = 0

            # Pitch slide
            if (sample_idx & 15) == 0:
                if bass_pitch < bass_target_pitch: bass_pitch += 2
                elif bass_pitch > bass_target_pitch: bass_pitch -= 2

            # Noise LFSR
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr >>= 1

            # Bass oscillator (sawtooth)
            bass_phase = (bass_phase + (bass_pitch << 1)) & 0xFFFF
            raw_wave = (bass_phase >> 8) - 128

            # SVF Filter
            # Modulate cutoff over time for signature acid sweep
            sweep = math.sin((sample_idx / total_samples) * math.pi * 3) * 40.0
            cutoff = int(90 + sweep + (bass_env >> 1))
            cutoff = max(10, min(200, cutoff))

            diff = raw_wave - filter_val
            drive = (diff * (cutoff >> 1)) / 128.0
            filter_vel += drive
            filter_vel -= (filter_vel * 25.0) / 256.0
            filter_val += filter_vel
            filter_val = max(-300.0, min(300.0, filter_val))

            outA = int(filter_val + 128)
            outA = max(0, min(255, outA))

            # Drum oscillator
            outB = 0
            if drum_type == 0: # Kick
                drum_phase = (drum_phase + (drum_pitch << 1)) & 0xFFFF
                if drum_pitch > 30: drum_pitch -= 2
                kw = drum_phase >> 8
                if drum_phase & 0x8000: kw = 255 - kw
                outB = (kw * drum_env) >> 8
            else: # Hat
                outB = ((lfsr & 0xFF) * drum_env) >> 8

            sA = outA - 128
            sB = outB
            mixed = sA + sB
            mixed = max(-128, min(127, mixed))
            samples[sample_idx] = (mixed + 128) & 0xFF
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 3. Flappy Slider: Startup fanfare, pipe pass chirps, and score fanfare
# ---------------------------------------------------------------------------
def synth_flappy():
    sample_rate = 15625
    frames = []

    def add_tone(freq, ms):
        n = int(sample_rate * (ms / 1000.0))
        if freq == 0:
            frames.extend([128] * n)
        else:
            period = sample_rate / freq
            for i in range(n):
                val = 220 if ((i % period) < (period / 2)) else 36
                frames.append(val)

    # Startup boop-bloop slide
    for i in range(10):
        add_tone(200 + i * 100, 35)
    for i in range(10, 20):
        add_tone(1200 - (i - 10) * 90, 35)
    add_tone(150, 90)
    add_tone(350, 130)
    add_tone(0, 150)

    # Normal pipe passes
    for score in [1, 2, 3]:
        offset = score * 20
        add_tone(440 + offset, 15)
        add_tone(880 + offset, 15)
        add_tone(0, 250)

    # Mini fanfare
    for f in [440, 554, 659, 880]:
        add_tone(f, 60)
    add_tone(0, 100)

    return bytearray(frames), sample_rate


# ---------------------------------------------------------------------------
# 4. Bass Patterns: 808 Sub-bass kick pitch drops & LFSR snare noise
# ---------------------------------------------------------------------------
def synth_bass_patterns():
    sample_rate = 10000
    step_duration = 0.125 # 125 ms per step (120 BPM 16th notes)
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 32 # 2 bars
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    kicks  = [1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0]
    snares = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1]

    phase = 0
    phase_inc = 0
    amp = 0
    noise_amp = 0
    lfsr = 0xACE1

    sample_idx = 0
    for step in range(total_steps):
        s_idx = step % 16
        if kicks[s_idx]:
            phase_inc = 750 # kick start freq
            amp = 255

        if snares[s_idx]:
            noise_amp = 140

        for _ in range(samples_per_step):
            bass_sample = 0
            if amp > 0:
                phase = (phase + phase_inc) & 0xFFFF
                if phase_inc > 45: # kick end freq
                    phase_inc -= 3 # pitch decay
                if (sample_idx % 8) == 0 and amp > 0:
                    amp -= 1
                # Square wave
                if phase & 0x8000:
                    bass_sample = amp

            noise_sample = 0
            if noise_amp > 0:
                if (sample_idx % 4) == 0 and noise_amp > 0:
                    noise_amp -= 1
                lsb = lfsr & 1
                lfsr >>= 1
                if lsb:
                    lfsr ^= 0xB400
                    noise_sample = noise_amp

            val = (bass_sample + noise_sample) >> 1
            samples[sample_idx] = val & 0xFF
            sample_idx += 1

    return samples, sample_rate


def main():
    AUDIO_DIR.mkdir(parents=True, exist_ok=True)
    synths = [
        ("drone_synth", synth_drone),
        ("peepsy_acid", synth_acid),
        ("flappy_slider", synth_flappy),
        ("bass_patterns", synth_bass_patterns),
    ]

    for name, fn in synths:
        samples, rate = fn()
        out = AUDIO_DIR / f"{name}.mp3"
        write_wav_to_mp3(samples, rate, out)


if __name__ == "__main__":
    main()
