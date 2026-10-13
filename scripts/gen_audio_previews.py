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
# 1. Drone Synth
# ---------------------------------------------------------------------------
def synth_drone():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase1 = 0
    phase2 = 0
    base_pitch = 140.0
    detune = 2.5

    for i in range(total_samples):
        mode = int((i / total_samples) * 4)
        lfo = math.sin(i * 0.003) * 3.0
        inc1 = int((base_pitch + lfo) * (65536.0 / sample_rate))
        inc2 = int((base_pitch + detune - lfo * 0.5) * (65536.0 / sample_rate))

        phase1 = (phase1 + inc1) & 0xFFFF
        phase2 = (phase2 + inc2) & 0xFFFF

        t1 = phase1 >> 8
        if t1 > 127: t1 = 255 - t1
        out1 = (t1 << 1) & 0xFF

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
# 2. Peepsy Acid
# ---------------------------------------------------------------------------
def synth_acid():
    sample_rate = 15600
    pitches = [137, 274, 137, 137, 274, 137, 205, 244, 137, 326, 137, 183, 274, 137, 326, 548]
    slides  = [0,   1,   0,   0,   0,   0,   1,   0,   0,   1,   0,   0,   0,   0,   1,   0]
    accents = [1,   0,   0,   1,   1,   0,   0,   1,   1,   0,   0,   1,   1,   0,   0,   1]
    drums   = [1,   0,   2,   0,   1,   0,   2,   0,   1,   2,   2,   1,   1,   0,   2,   2]

    step_duration = 0.115
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 32
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
        d = drums[s_idx]
        if d == 1:
            drum_type = 0
            drum_pitch = 1000
            drum_env = 255
        elif d == 2:
            drum_type = 1
            drum_env = 120

        next_p = pitches[s_idx]
        if slides[s_idx]:
            bass_target_pitch = next_p
        else:
            bass_pitch = next_p
            bass_target_pitch = next_p
            bass_env = 255 if accents[s_idx] else 100

        for _ in range(samples_per_step):
            if (sample_idx & 7) == 0:
                if bass_env > 1: bass_env -= 2
                else: bass_env = 0
                if drum_type == 0:
                    if drum_env > 0: drum_env -= 1
                else:
                    if drum_env > 7: drum_env -= 8
                    else: drum_env = 0

            if (sample_idx & 15) == 0:
                if bass_pitch < bass_target_pitch: bass_pitch += 2
                elif bass_pitch > bass_target_pitch: bass_pitch -= 2

            if lfsr & 1: lfsr = (lfsr >> 1) ^ 0xB400
            else: lfsr >>= 1

            bass_phase = (bass_phase + (bass_pitch << 1)) & 0xFFFF
            raw_wave = (bass_phase >> 8) - 128

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

            outB = 0
            if drum_type == 0:
                drum_phase = (drum_phase + (drum_pitch << 1)) & 0xFFFF
                if drum_pitch > 30: drum_pitch -= 2
                kw = drum_phase >> 8
                if drum_phase & 0x8000: kw = 255 - kw
                outB = (kw * drum_env) >> 8
            else:
                outB = ((lfsr & 0xFF) * drum_env) >> 8

            mixed = max(-128, min(127, (outA - 128) + outB))
            samples[sample_idx] = (mixed + 128) & 0xFF
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 3. Flappy Slider
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

    for i in range(10): add_tone(200 + i * 100, 35)
    for i in range(10, 20): add_tone(1200 - (i - 10) * 90, 35)
    add_tone(150, 90)
    add_tone(350, 130)
    add_tone(0, 150)

    for score in [1, 2, 3]:
        offset = score * 20
        add_tone(440 + offset, 15)
        add_tone(880 + offset, 15)
        add_tone(0, 250)

    for f in [440, 554, 659, 880]: add_tone(f, 60)
    add_tone(0, 100)

    return bytearray(frames), sample_rate


# ---------------------------------------------------------------------------
# 4. Bass Patterns
# ---------------------------------------------------------------------------
def synth_bass_patterns():
    sample_rate = 10000
    step_duration = 0.125
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 32
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
            phase_inc = 750
            amp = 255

        if snares[s_idx]:
            noise_amp = 140

        for _ in range(samples_per_step):
            bass_sample = 0
            if amp > 0:
                phase = (phase + phase_inc) & 0xFFFF
                if phase_inc > 45: phase_inc -= 3
                if (sample_idx % 8) == 0 and amp > 0: amp -= 1
                if phase & 0x8000: bass_sample = amp

            noise_sample = 0
            if noise_amp > 0:
                if (sample_idx % 4) == 0 and noise_amp > 0: noise_amp -= 1
                lsb = lfsr & 1
                lfsr >>= 1
                if lsb:
                    lfsr ^= 0xB400
                    noise_sample = noise_amp

            val = (bass_sample + noise_sample) >> 1
            samples[sample_idx] = val & 0xFF
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 5. Minimal Techno
# ---------------------------------------------------------------------------
def synth_minimal_techno():
    sample_rate = 15600
    P_C2 = 137; P_Eb2 = 163; P_F2 = 183; P_C3 = 274
    pitches = [P_C2, P_Eb2, P_C2, P_C2, P_Eb2, P_C2, P_C2, P_Eb2, P_C2, P_F2, P_C2, P_C2, P_Eb2, P_C2, P_C2, P_F2]
    drums   = [1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2]

    step_duration = 0.120
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 32
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    bass_phase = 0
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
        d = drums[s_idx]
        if d == 1:
            drum_type = 0
            drum_pitch = 1000
            drum_env = 255
        elif d == 2:
            drum_type = 1
            drum_env = 100

        bass_pitch = pitches[s_idx]
        bass_env = 180 if (s_idx % 2 == 1) else 100

        for _ in range(samples_per_step):
            if (sample_idx & 7) == 0:
                if bass_env > 2: bass_env -= 3
                else: bass_env = 0
                if drum_type == 0:
                    if drum_env > 0: drum_env -= 1
                else:
                    if drum_env > 7: drum_env -= 8
                    else: drum_env = 0

            if lfsr & 1: lfsr = (lfsr >> 1) ^ 0xB400
            else: lfsr >>= 1

            bass_phase = (bass_phase + (bass_pitch << 1)) & 0xFFFF
            raw_wave = (bass_phase >> 8) - 128

            cutoff = 75 + (bass_env >> 1)
            cutoff = max(10, min(190, cutoff))

            diff = raw_wave - filter_val
            drive = (diff * (cutoff >> 1)) / 128.0
            filter_vel += drive
            filter_vel -= (filter_vel * 28.0) / 256.0
            filter_val += filter_vel
            filter_val = max(-300.0, min(300.0, filter_val))

            outA = int(filter_val + 128)
            outA = max(0, min(255, outA))

            outB = 0
            if drum_type == 0:
                drum_phase = (drum_phase + (drum_pitch << 1)) & 0xFFFF
                if drum_pitch > 30: drum_pitch -= 2
                kw = drum_phase >> 8
                if drum_phase & 0x8000: kw = 255 - kw
                outB = (kw * drum_env) >> 8
            else:
                outB = ((lfsr & 0xFF) * drum_env) >> 8

            mixed = max(-128, min(127, (outA - 128) + outB))
            samples[sample_idx] = (mixed + 128) & 0xFF
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 6. Deep Minimal
# ---------------------------------------------------------------------------
def synth_deep_minimal():
    sample_rate = 15600
    P_C2 = 137; P_Eb2 = 163
    pitches = [0, 0, P_C2, 0, 0, 0, P_C2, 0, 0, 0, P_Eb2, 0, 0, 0, P_C2, 0]
    drums   = [1, 0, 2,    0, 1, 0, 2,    0, 1, 0, 2,     0, 1, 0, 2,    0]

    step_duration = 0.125
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 32
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    bass_phase = 0
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
        d = drums[s_idx]
        if d == 1:
            drum_type = 0
            drum_pitch = 1000
            drum_env = 240
        elif d == 2:
            drum_type = 1
            drum_env = 80

        p = pitches[s_idx]
        bass_env = 240 if (p > 0) else 0

        for _ in range(samples_per_step):
            if (sample_idx & 7) == 0:
                if bass_env > 3: bass_env -= 4
                else: bass_env = 0
                if drum_type == 0:
                    if drum_env > 0: drum_env -= 1
                else:
                    if drum_env > 9: drum_env -= 10
                    else: drum_env = 0

            if lfsr & 1: lfsr = (lfsr >> 1) ^ 0xB400
            else: lfsr >>= 1

            raw_wave = 0
            if p > 0:
                bass_phase = (bass_phase + (p << 1)) & 0xFFFF
                raw_wave = (bass_phase >> 8) - 128

            cutoff = 55 + (bass_env >> 1)
            cutoff = max(10, min(160, cutoff))

            diff = raw_wave - filter_val
            drive = (diff * (cutoff >> 1)) / 128.0
            filter_vel += drive
            filter_vel -= (filter_vel * 32.0) / 256.0
            filter_val += filter_vel
            filter_val = max(-300.0, min(300.0, filter_val))

            outA = int(filter_val + 128)
            outA = max(0, min(255, outA))

            outB = 0
            if drum_type == 0:
                drum_phase = (drum_phase + (drum_pitch << 1)) & 0xFFFF
                if drum_pitch > 30: drum_pitch -= 2
                kw = drum_phase >> 8
                if drum_phase & 0x8000: kw = 255 - kw
                outB = (kw * drum_env) >> 8
            else:
                outB = ((lfsr & 0xFF) * drum_env) >> 8

            mixed = max(-128, min(127, (outA - 128) + outB))
            samples[sample_idx] = (mixed + 128) & 0xFF
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 7. Quantum DJ: Granular 1-bit / bytebeat alien stream
# ---------------------------------------------------------------------------
def synth_quantum_matrix():
    sample_rate = 16000
    duration_sec = 5.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)
    t = 0
    for i in range(total_samples):
        lfo = int(math.sin(i * 0.002) * 5) + 6
        val = ((t * lfo & (t >> 7)) | (t * 3 & (t >> 9))) & 0xFF
        out = 220 if (val > 115) else 40
        samples[i] = out
        t += 1
    return samples, sample_rate

# ---------------------------------------------------------------------------
# 8. Ave Maria (Siberia): Deep Russian Orthodox D2 drone, sacred arpeggios & chant
# ---------------------------------------------------------------------------
def synth_ave_maria_siberia():
    sample_rate = 10000
    step_duration = 0.110 # 110 ms per arpeggio step
    samples_per_step = int(sample_rate * step_duration)

    arp_freqs = [147, 175, 220, 294, 349, 294, 220, 175, 110, 147, 175, 220, 294, 220, 175, 147]
    # Melody: note frequency, duration in steps
    melody = [
        (220, 4), (220, 4), (294, 6), (294, 2), (294, 3), (277, 1), (233, 2), (220, 6),
        (220, 2), (349, 4), (330, 3), (294, 1), (277, 2), (294, 4), (330, 4), (220, 6),
        (220, 4), (220, 4), (349, 6), (349, 2), (349, 3), (330, 1), (294, 2), (277, 6),
        (294, 2), (349, 3), (440, 3), (392, 2), (349, 2), (330, 2), (294, 6), (294, 6)
    ]

    total_steps = sum(d for _, d in melody) # 96 steps
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    phase_bass = 0
    phase_arp = 0
    phase_lead = 0
    inc_bass = int(73.4 * (65536.0 / sample_rate))

    mel_idx = 0
    mel_step_cnt = 0
    cur_mel_freq, cur_mel_len = melody[0]
    inc_lead = int(cur_mel_freq * (65536.0 / sample_rate))
    env_lead = 250

    sample_idx = 0
    for step in range(total_steps):
        # Arp
        arp_f = arp_freqs[step % 16]
        inc_arp = int(arp_f * (65536.0 / sample_rate))

        # Check melody step advance
        if mel_step_cnt >= cur_mel_len:
            mel_idx = (mel_idx + 1) % len(melody)
            cur_mel_freq, cur_mel_len = melody[mel_idx]
            inc_lead = int(cur_mel_freq * (65536.0 / sample_rate))
            env_lead = 250
            mel_step_cnt = 0
        mel_step_cnt += 1

        for _ in range(samples_per_step):
            # Voice 1: Deep bass drone (triangle)
            phase_bass = (phase_bass + inc_bass) & 0xFFFF
            tb = phase_bass >> 8
            if tb > 127: tb = 255 - tb
            s_bass = tb << 1

            # Voice 2: Arp (triangle)
            phase_arp = (phase_arp + inc_arp) & 0xFFFF
            ta = phase_arp >> 8
            if ta > 127: ta = 255 - ta
            s_arp = (ta << 1) * 180 >> 8

            # Voice 3: Sacred lead (triangle with slight vibrato)
            vib = math.sin(sample_idx * 0.004) * 1.5
            phase_lead = (phase_lead + inc_lead + int(vib * 10)) & 0xFFFF
            tl = phase_lead >> 8
            if tl > 127: tl = 255 - tl
            s_lead = (tl << 1) * env_lead >> 8

            # Mix
            mix = (s_bass + s_arp + (s_lead << 1)) >> 2
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

            if env_lead > 40 and (sample_idx % 40) == 0:
                env_lead -= 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 9. lambada.cpp — Kaoma Lambada Tropical Synth
# ---------------------------------------------------------------------------
def synth_lambada():
    sample_rate = 10000
    tempo_bpm = 124
    # 16th note duration in seconds: 60 / (124 * 4) = 0.12097s
    step_duration = 60.0 / (tempo_bpm * 4.0)
    samples_per_step = int(sample_rate * step_duration)
    
    # 64 steps (4 bars of the legendary hook + 2 bars verse pickup)
    total_steps = 96
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    note_table = [
        0, 392, 440, 466, 523, 554, 587, 659, 698, 784, 880, 932, 1046, 1175
    ]
    # Melody indices from lambada.cpp:
    melody = [
       7,  8,  7,  6,  5,  6,  7,  7,  0,  0, 10, 10, 10, 10,  8,  7,
       6,  6,  6,  6,  0,  0,  0,  0,  7,  8,  7,  6,  5,  6,  5,  2,
       5,  6,  7,  8,  7,  6,  5,  6,  6,  6,  6,  6,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  6,
       7,  8,  7,  6,  5,  6,  7,  7,  0,  0, 10, 10, 10, 10,  8,  7,
       6,  6,  6,  6,  0,  0,  0,  0,  7,  8,  7,  6,  5,  6,  5,  2
    ]

    phase_lead = 0
    phase_bass = 0
    phase_drum = 0
    drum_pitch = 0
    env_lead = 0
    env_bass = 0
    env_drum = 0
    drum_type = 0
    lfsr = 0xACE1
    sample_idx = 0

    for step in range(total_steps):
        bar_step = step & 15
        note_idx = melody[step % len(melody)]
        lead_freq = note_table[note_idx]
        inc_lead = int(lead_freq * (65536.0 / sample_rate)) if lead_freq > 0 else 0
        if note_idx > 0:
            env_lead = 250

        # Carimbo bass
        root_freq = 73.42 # D2
        if bar_step in (0, 8):
            inc_bass = int(root_freq * (65536.0 / sample_rate))
            env_bass = 255
        elif bar_step in (4, 12):
            inc_bass = int(root_freq * (65536.0 / sample_rate))
            env_bass = 180
        elif bar_step in (6, 14):
            inc_bass = int((root_freq * 2.0) * (65536.0 / sample_rate))
            env_bass = 230

        # Drums
        if bar_step in (0, 8):
            drum_type = 1
            drum_pitch = 950
            env_drum = 255
        elif bar_step in (6, 14):
            drum_type = 2
            env_drum = 230
        elif bar_step in (3, 11):
            drum_type = 3
            env_drum = 160
        else:
            drum_type = 3
            env_drum = 70

        for s in range(samples_per_step):
            # Noise generator
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr = lfsr >> 1

            # Decays
            if (sample_idx & 7) == 0:
                if env_drum > 12: env_drum -= 12
                else: env_drum = 0
                if env_bass > 4: env_bass -= 4
                else: env_bass = 0
            if (sample_idx & 15) == 0:
                if env_lead > 160: env_lead -= 2
                elif env_lead > 20: env_lead -= 1

            # Lead (Accordion simulation: saw + pulse with slight chorus)
            s_lead = 0
            if inc_lead > 0 and env_lead > 0:
                vib = 9 if (sample_idx & 0x40) else -9
                phase_lead = (phase_lead + inc_lead + vib) & 0xFFFF
                saw = phase_lead >> 8
                reed2 = 220 if ((phase_lead * 2) & 0x8000) else 35
                acc = (saw + reed2) >> 1
                s_lead = (acc * env_lead) >> 8

            # Bass
            s_bass = 0
            if env_bass > 0:
                phase_bass = (phase_bass + inc_bass) & 0xFFFF
                btri = phase_bass >> 8
                if btri > 127: btri = 255 - btri
                s_bass = ((btri << 1) * env_bass) >> 8

            # Drums
            s_drum = 0
            if env_drum > 0:
                if drum_type == 1:
                    phase_drum = (phase_drum + (drum_pitch << 1)) & 0xFFFF
                    if drum_pitch > 35: drum_pitch -= 2
                    kw = phase_drum >> 8
                    if kw > 127: kw = 255 - kw
                    s_drum = ((kw << 1) * env_drum) >> 8
                elif drum_type == 2:
                    phase_drum = (phase_drum + 400) & 0xFFFF
                    tone = 180 if (phase_drum & 0x8000) else 70
                    noise = lfsr & 0xFF
                    s_drum = (((tone + noise) >> 1) * env_drum) >> 8
                else:
                    noise = lfsr & 0xFF
                    s_drum = (noise * env_drum) >> 8

            mix = (s_bass + (s_lead << 1) + s_drum) >> 2
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 10. axel_foley.cpp — Harold Faltermeyer Axel F 80s Synth
# ---------------------------------------------------------------------------
def synth_axel_foley():
    sample_rate = 10000
    tempo_bpm = 117
    step_duration = 60.0 / (tempo_bpm * 4.0) # ~0.1282s per 16th note
    samples_per_step = int(sample_rate * step_duration)
    
    # 64 steps (4 bars)
    total_steps = 64
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    note_table = [
        0, 262, 311, 349, 392, 415, 466, 523, 554, 622, 698
    ]
    # Axel F melody indices:
    melody = [
       3,  0,  0,  3,  0,  5,  0,  3,  3,  0,  6,  3,  3,  0,  2,  0,
       3,  0,  0,  7,  0,  3,  3,  0,  8,  0,  7,  0,  5,  0,  0,  0,
       3,  0,  7,  0, 10,  0,  3,  0,  2,  0,  2,  1,  4,  0,  3,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    ]

    phase_lead = 0
    phase_bass = 0
    phase_drum = 0
    drum_pitch = 0
    env_lead = 0
    env_bass = 0
    env_drum = 0
    drum_type = 0
    lfsr = 0x5432
    sample_idx = 0

    for step in range(total_steps):
        bar_step = step & 15
        note_idx = melody[step]
        lead_freq = note_table[note_idx]
        inc_lead = int(lead_freq * (65536.0 / sample_rate)) if lead_freq > 0 else 0
        if note_idx > 0:
            env_lead = 255 # Sharp staccato hit!

        # Moog 16th-note bass
        root_freq = 87.31 # F2
        if bar_step in (0, 4, 8, 12):
            inc_bass = int(root_freq * (65536.0 / sample_rate))
            env_bass = 250
        elif bar_step in (2, 6, 10, 14):
            mult = 2.0 if bar_step == 14 else 1.0
            inc_bass = int((root_freq * mult) * (65536.0 / sample_rate))
            env_bass = 200

        # LinnDrum Beat
        if bar_step in (0, 8):
            drum_type = 1 # Linn Kick
            drum_pitch = 1000
            env_drum = 255
        elif bar_step in (4, 12):
            drum_type = 2 # Gated 80s Snare / Clap
            env_drum = 240
        elif bar_step in (2, 6, 10, 14):
            drum_type = 3 # Open Hat
            env_drum = 150
        else:
            drum_type = 3 # Closed Hat
            env_drum = 60

        for s in range(samples_per_step):
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr = lfsr >> 1

            # Fast decays
            if (sample_idx & 7) == 0:
                if env_drum > 14: env_drum -= 14
                else: env_drum = 0
                if env_bass > 8: env_bass -= 8
                else: env_bass = 0
            if (sample_idx & 15) == 0:
                if env_lead > 24: env_lead -= 24
                else: env_lead = 0

            # Lead (Jupiter-8 Synth Brass)
            s_lead = 0
            if inc_lead > 0 and env_lead > 0:
                phase_lead = (phase_lead + inc_lead) & 0xFFFF
                saw = phase_lead >> 8
                pulse = 230 if (phase_lead & 0x6000) else 25
                brass = (saw + pulse) >> 1
                s_lead = (brass * env_lead) >> 8

            # Moog Bass
            s_bass = 0
            if env_bass > 0:
                phase_bass = (phase_bass + inc_bass) & 0xFFFF
                btri = phase_bass >> 8
                if btri > 127: btri = 255 - btri
                s_bass = ((btri << 1) * env_bass) >> 8

            # Drums
            s_drum = 0
            if env_drum > 0:
                if drum_type == 1:
                    phase_drum = (phase_drum + (drum_pitch << 1)) & 0xFFFF
                    if drum_pitch > 40: drum_pitch -= 2
                    kw = phase_drum >> 8
                    if kw > 127: kw = 255 - kw
                    s_drum = ((kw << 1) * env_drum) >> 8
                elif drum_type == 2:
                    phase_drum = (phase_drum + 450) & 0xFFFF
                    crack = 200 if (phase_drum & 0x8000) else 50
                    noise = lfsr & 0xFF
                    s_drum = (((crack + noise) >> 1) * env_drum) >> 8
                else:
                    noise = lfsr & 0xFF
                    s_drum = (noise * env_drum) >> 8

            mix = (s_bass + (s_lead << 1) + s_drum) >> 2
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 11. auf_der_heide.cpp — Vintage German Brass & Oom-Pah Marsch
# ---------------------------------------------------------------------------
def synth_auf_der_heide():
    sample_rate = 10000
    tempo_bpm = 115
    step_duration = 60.0 / (tempo_bpm * 4.0)
    samples_per_step = int(sample_rate * step_duration)
    
    # 64 steps (4 bars)
    total_steps = 64
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    note_table = [
        0, 196, 220, 247, 262, 294, 330, 370, 392, 440, 494, 523, 587, 659
    ]
    # Erika melody indices (bars 1-4):
    melody = [
       5,  5,  8,  8, 10, 10, 12, 12, 12, 12, 10, 10,  8,  8,  0,  0,
      11, 11,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  5,
      12, 12, 11, 11,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       8,  8, 10, 10,  8,  8,  8,  8,  0,  0,  0,  0,  0,  0,  0,  0
    ]

    phase_lead = 0
    phase_bass = 0
    phase_drum = 0
    drum_pitch = 0
    env_lead = 0
    env_bass = 0
    env_drum = 0
    drum_type = 0
    lfsr = 0x9876
    sample_idx = 0

    for step in range(total_steps):
        bar = step >> 4
        bar_step = step & 15
        note_idx = melody[step]
        lead_freq = note_table[note_idx]
        inc_lead = int(lead_freq * (65536.0 / sample_rate)) if lead_freq > 0 else 0
        if note_idx > 0:
            env_lead = 250

        # Oom-pah tuba bass (alternating root/fifth)
        root_freq = 98.0 if bar in (0, 3) else 146.83 # G2 or D3
        fifth_freq = 146.83 if bar in (0, 3) else 220.0
        if bar_step in (0, 8):
            inc_bass = int(root_freq * (65536.0 / sample_rate))
            env_bass = 255
        elif bar_step in (4, 12):
            inc_bass = int(fifth_freq * (65536.0 / sample_rate))
            env_bass = 210

        # Stomps & March Drums
        stomp_now = False
        if bar == 1 and bar_step in (6, 8, 10): stomp_now = True
        elif bar == 2 and bar_step in (7, 9, 11): stomp_now = True

        if stomp_now:
            drum_type = 1 # Heavy stomp!
            drum_pitch = 1100
            env_drum = 255
        elif bar_step in (0, 8):
            drum_type = 1
            drum_pitch = 850
            env_drum = 200
        elif bar_step in (4, 12):
            drum_type = 2 # March snare
            env_drum = 190
        elif (bar_step & 1) == 0:
            drum_type = 3
            env_drum = 80
        else:
            drum_type = 0

        for s in range(samples_per_step):
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr = lfsr >> 1

            # Decays
            if (sample_idx & 7) == 0:
                if env_drum > 10: env_drum -= 10
                else: env_drum = 0
                if env_bass > 5: env_bass -= 5
                else: env_bass = 0
            if (sample_idx & 15) == 0:
                if env_lead > 16: env_lead -= 16
                else: env_lead = 0

            # Lead (German Brass Horn)
            s_lead = 0
            if inc_lead > 0 and env_lead > 0:
                phase_lead = (phase_lead + inc_lead) & 0xFFFF
                saw = phase_lead >> 8
                pulse = 220 if (phase_lead & 0x7000) else 35
                brass = (saw + pulse) >> 1
                s_lead = (brass * env_lead) >> 8

            # Tuba Bass
            s_bass = 0
            if env_bass > 0:
                phase_bass = (phase_bass + inc_bass) & 0xFFFF
                btri = phase_bass >> 8
                if btri > 127: btri = 255 - btri
                s_bass = ((btri << 1) * env_bass) >> 8

            # Drums & Stomp
            s_drum = 0
            if env_drum > 0:
                if drum_type == 1:
                    phase_drum = (phase_drum + (drum_pitch << 1)) & 0xFFFF
                    if drum_pitch > 35: drum_pitch -= 2
                    kw = phase_drum >> 8
                    if kw > 127: kw = 255 - kw
                    crunch = kw + ((lfsr & 0x3F) >> 1)
                    s_drum = (crunch * env_drum) >> 8
                elif drum_type == 2:
                    phase_drum = (phase_drum + 400) & 0xFFFF
                    tone = 190 if (phase_drum & 0x8000) else 60
                    noise = lfsr & 0xFF
                    s_drum = (((tone + noise) >> 1) * env_drum) >> 8
                else:
                    noise = lfsr & 0xFF
                    s_drum = (noise * env_drum) >> 8

            mix = (s_bass + (s_lead << 1) + s_drum) >> 2
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 12. zelda_sid.cpp — Legend of Zelda Overworld Theme (SID Sound Engine)
# ---------------------------------------------------------------------------
def synth_zelda_sid():
    sample_rate = 10000
    tempo_bpm = 130
    step_duration = 60.0 / (tempo_bpm * 4.0) # ~0.1154s per 16th note
    samples_per_step = int(sample_rate * step_duration)
    
    # 64 steps (4 bars)
    total_steps = 64
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    note_table = [
        0, 349, 370, 392, 415, 440, 466, 523, 554, 587, 622, 659, 698, 740, 784, 831, 932
    ]
    # Bars 1-4 of Zelda Overworld:
    melody = [
       6,  6,  6,  6,  6,  6,  6,  6,  0,  0,  1,  1,  6,  7,  9, 10,
      12, 12, 12, 12, 12, 12, 12, 12,  0,  0, 12,  0, 12, 12, 13, 15,
      16, 16, 16, 16, 16, 16, 16, 16,  0,  0, 16,  0, 16, 16, 15, 13,
      15, 15, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
    ]

    # SID Arp Chords (Bb, Eb, Gb, F)
    arp_chords = [
        [233, 294, 349], # Bb Major
        [233, 311, 392], # Eb Major
        [233, 277, 370], # Gb Major
        [220, 262, 349]  # F Major
    ]

    # Bass roots
    bar_bass = [116.54, 155.56, 185.0, 174.61]

    v1_phase = 0
    v2_phase = 0
    v3_phase = 0
    v1_env = 0
    v2_env = 0
    v3_env = 0
    v3_freq = 0
    v3_is_drum = 0
    lfsr = 0x7654
    sample_idx = 0

    for step in range(total_steps):
        bar = step >> 4
        bar_step = step & 15
        note_idx = melody[step]
        lead_freq = note_table[note_idx]
        inc_lead = int(lead_freq * (65536.0 / sample_rate)) if lead_freq > 0 else 0
        if note_idx > 0:
            v1_env = 250

        # Voice 2: Arp trigger
        v2_env = 180

        # Voice 3: Zelda Snare march & Bass
        is_snare = bar_step in (2, 3, 4, 6, 10, 11, 12, 14)
        if bar_step in (0, 8):
            v3_is_drum = 2 # Kick
            v3_freq = 900
            v3_env = 250
        elif is_snare:
            v3_is_drum = 1 # Snare roll
            v3_env = 240
        else:
            v3_is_drum = 0 # Bass
            bass_f = bar_bass[bar]
            v3_freq = int(bass_f * (65536.0 / sample_rate))
            v3_env = 220

        for s in range(samples_per_step):
            # 50Hz Fast SID Arp clock (every 200 samples)
            arp_note_idx = (sample_idx // 180) % 3
            arp_f = arp_chords[bar][arp_note_idx]
            inc_arp = int(arp_f * (65536.0 / sample_rate))

            # Noise generator
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr = lfsr >> 1

            # ADSR decays
            if (sample_idx & 7) == 0:
                if v3_env > 12: v3_env -= 12
                else: v3_env = 0
            if (sample_idx & 15) == 0:
                if v1_env > 190: v1_env -= 4
                if v2_env > 8: v2_env -= 8
                else: v2_env = 0

            # Voice 1: PWM LFO Lead (chorused singing pulse width)
            s1 = 0
            if inc_lead > 0 and v1_env > 0:
                lfo = math.sin(sample_idx * 0.003) * 60.0
                dyn_pw = int(128 + lfo)
                v1_phase = (v1_phase + inc_lead) & 0xFFFF
                p1 = v1_phase >> 8
                pulse = 240 if (p1 < dyn_pw) else 15
                s1 = (pulse * v1_env) >> 8

            # Voice 2: SID Fast Arp (25% pulse)
            s2 = 0
            if v2_env > 0:
                v2_phase = (v2_phase + inc_arp) & 0xFFFF
                p2 = v2_phase >> 8
                pulse2 = 220 if (p2 < 64) else 25
                s2 = (pulse2 * v2_env) >> 8

            # Voice 3: Bass & March Snare
            s3 = 0
            if v3_env > 0:
                if v3_is_drum == 1:
                    noise = lfsr & 0xFF
                    s3 = (noise * v3_env) >> 8
                elif v3_is_drum == 2:
                    v3_phase = (v3_phase + (v3_freq << 1)) & 0xFFFF
                    if v3_freq > 35: v3_freq -= 3
                    tri = v3_phase >> 8
                    if tri > 127: tri = 255 - tri
                    s3 = ((tri << 1) * v3_env) >> 8
                else:
                    v3_phase = (v3_phase + v3_freq) & 0xFFFF
                    tri = v3_phase >> 8
                    if tri > 127: tri = 255 - tri
                    s3 = ((tri << 1) * v3_env) >> 8

            mix = (s3 + (s1 << 1) + s2) >> 2
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

    return samples, sample_rate

def main():
    AUDIO_DIR.mkdir(parents=True, exist_ok=True)
    synths = [
        ("drone_synth", synth_drone),
        ("peepsy_acid", synth_acid),
        ("flappy_slider", synth_flappy),
        ("bass_patterns", synth_bass_patterns),
        ("minimal_techno", synth_minimal_techno),
        ("deep_minimal", synth_deep_minimal),
        ("quantum_matrix", synth_quantum_matrix),
        ("ave_maria_siberia", synth_ave_maria_siberia),
        ("lambada", synth_lambada),
        ("axel_foley", synth_axel_foley),
        ("auf_der_heide", synth_auf_der_heide),
        ("zelda_sid", synth_zelda_sid),
    ]

    for name, fn in synths:
        samples, rate = fn()
        out = AUDIO_DIR / f"{name}.mp3"
        write_wav_to_mp3(samples, rate, out)

if __name__ == "__main__":
    main()



