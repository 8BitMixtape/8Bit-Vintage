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


# ---------------------------------------------------------------------------
# 13. kraftwerk_model.cpp — Kraftwerk "Das Model"
# ---------------------------------------------------------------------------
def synth_kraftwerk_model():
    sample_rate = 10000
    bpm = 124.0
    step_duration = (60.0 / bpm) / 4.0 # 16th note ~ 0.121s
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 128 # 8 bars of electro-pop perfection
    total_samples = total_steps * samples_per_step
    samples = bytearray(total_samples)

    # Note frequencies in Hz
    # 0: REST, 1: G#4, 2: A4, 3: B4, 4: C5, 5: D5, 6: E5, 7: F5
    note_table = [0.0, 415.3, 440.0, 493.88, 523.25, 587.33, 659.25, 698.46]

    lead_melody = [
        # Bar 1: E5 C5 A4 E5
        6, 6, 6, 6, 4, 4, 4, 4, 2, 2, 2, 2, 6, 6, 6, 6,
        # Bar 2: F5 D5 A4 F5
        7, 7, 7, 7, 5, 5, 5, 5, 2, 2, 2, 2, 7, 7, 7, 7,
        # Bar 3: E5 B4 G#4 E5
        6, 6, 6, 6, 3, 3, 3, 3, 1, 1, 1, 1, 6, 6, 6, 6,
        # Bar 4: E5 C5 A4 hold
        6, 6, 6, 6, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2,
        # Bar 5: E5 C5 A4 E5
        6, 6, 6, 6, 4, 4, 4, 4, 2, 2, 2, 2, 6, 6, 6, 6,
        # Bar 6: F5 D5 A4 F5
        7, 7, 7, 7, 5, 5, 5, 5, 2, 2, 2, 2, 7, 7, 7, 7,
        # Bar 7: E5 B4 G#4 B4
        6, 6, 6, 6, 3, 3, 3, 3, 1, 1, 1, 1, 3, 3, 3, 3,
        # Bar 8: A4 hold ... turnaround
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,
    ]

    # Bar roots (Am, Dm, E, Am, Am, Dm, E, Am)
    bar_roots = [110.0, 146.83, 164.81, 110.0, 110.0, 146.83, 164.81, 110.0]
    arp_triads = [
        [220.0, 261.63, 329.63], # Am
        [146.83, 174.61, 220.0],  # Dm
        [164.81, 207.65, 246.94], # E
        [220.0, 261.63, 329.63]  # Am
    ]

    phase_lead = 0
    phase_arp = 0
    phase_bass = 0
    phase_drum = 0
    drum_pitch = 0
    drum_type = 0
    env_lead = 0
    env_arp = 0
    env_bass = 0
    env_drum = 0
    arp_tick = 0
    inc_bass = 0
    inc_lead = 0
    inc_arp = 0
    lfsr = 0x8351
    sample_idx = 0
    prev_note_idx = 0

    for step in range(total_steps):
        bar = step >> 4
        bar_step = step & 15
        chord_idx = bar & 3

        # Lead melody
        note_idx = lead_melody[step]
        lead_freq = note_table[note_idx]
        inc_lead = int(lead_freq * (65536.0 / sample_rate)) if lead_freq > 0 else 0
        if note_idx > 0:
            if note_idx != prev_note_idx or (bar_step & 3) == 0:
                env_lead = 255
        prev_note_idx = note_idx

        # 16th-note Kling Klang Arp
        arp_tick = (arp_tick + 1) % 3
        arp_freq = arp_triads[chord_idx][arp_tick]
        inc_arp = int(arp_freq * (65536.0 / sample_rate))
        env_arp = 150

        # Driving Bass
        if bar_step in (0, 3, 6, 8, 11, 14):
            root_f = bar_roots[bar]
            inc_bass = int(root_f * (65536.0 / sample_rate))
            env_bass = 250

        # Electronic Drums
        if bar_step in (0, 8):
            drum_type = 1
            drum_pitch = 950
            env_drum = 250
        elif bar_step in (4, 12):
            drum_type = 2
            env_drum = 230
        elif (bar_step & 1) == 0:
            drum_type = 3
            env_drum = 90
        else:
            drum_type = 3
            env_drum = 40

        for s in range(samples_per_step):
            # Noise generator
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr = lfsr >> 1

            # Decays
            if (sample_idx & 7) == 0:
                if env_drum > 14: env_drum -= 14
                else: env_drum = 0
                if env_bass > 10: env_bass -= 10
                else: env_bass = 0
                if env_arp > 12: env_arp -= 12
                else: env_arp = 0
            if (sample_idx & 15) == 0:
                if env_lead > 16: env_lead -= 16
                else: env_lead = 0

            # Voice 1: Lead (Minimoog pulse)
            s_lead = 0
            if inc_lead > 0 and env_lead > 0:
                phase_lead = (phase_lead + inc_lead) & 0xFFFF
                p = phase_lead >> 8
                pulse = 230 if p < 120 else 25
                s_lead = (pulse * env_lead) >> 8
                s_lead = (s_lead * 210) >> 8

            # Voice 2: Arp (Narrow pulse)
            s_arp = 0
            if inc_arp > 0 and env_arp > 0:
                phase_arp = (phase_arp + inc_arp) & 0xFFFF
                ap = 220 if (phase_arp >> 8) < 64 else 35
                s_arp = (ap * env_arp) >> 8

            # Voice 3: Bass (Triangle)
            s_bass = 0
            if inc_bass > 0 and env_bass > 0:
                phase_bass = (phase_bass + inc_bass) & 0xFFFF
                btri = phase_bass >> 8
                if btri > 127: btri = 255 - btri
                s_bass = ((btri << 1) * env_bass) >> 8
                s_bass = (s_bass * 220) >> 8

            # Voice 4: Drums
            s_drum = 0
            if env_drum > 0:
                if drum_type == 1:
                    phase_drum = (phase_drum + (drum_pitch << 1)) & 0xFFFF
                    if drum_pitch > 35: drum_pitch -= 3
                    kw = phase_drum >> 8
                    if kw > 127: kw = 255 - kw
                    s_drum = ((kw << 1) * env_drum) >> 8
                else:
                    noise = lfsr & 0xFF
                    s_drum = (noise * env_drum) >> 8

            mix = (s_bass + (s_lead << 1) + s_arp + s_drum) >> 2
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 14. kraftwerk_roboter.cpp — Kraftwerk "Die Roboter" (MOS 6581 SID Engine)
# ---------------------------------------------------------------------------
def synth_kraftwerk_roboter():
    sample_rate = 10000
    bpm = 115.0
    step_duration = (60.0 / bpm) / 4.0 # 16th note ~ 0.1304s
    samples_per_step = int(sample_rate * step_duration)
    total_steps = 128 # 8 bars (Hook riff + Vocoder "Wir sind die Roboter" + "Ja tvoi sluga...")
    total_samples = total_steps * samples_per_step
    samples = bytearray(total_samples)

    # Note frequencies in Hz (11 notes)
    # 0: REST, 1: B3, 2: D4, 3: E4, 4: F#4, 5: G4, 6: A4, 7: B4, 8: D5, 9: E5, 10: G5
    roboter_notes = [0.0, 246.94, 293.66, 329.63, 369.99, 392.00, 440.00, 493.88, 587.33, 659.25, 783.99]

    lead_melody = [
        # Bar 1: E5 B4 G4 E4 D4 G4 B4 D5
        9,  9,  7,  7,  5,  5,  3,  3,  2,  2,  5,  5,  7,  7,  8,  8,
        # Bar 2: E5 B4 G4 E4 D4 G4 B4 D5
        9,  9,  7,  7,  5,  5,  3,  3,  2,  2,  5,  5,  7,  7,  8,  8,
        # Bar 3: D5 B4 F#4 D4 C4 E4 G4 B4
        8,  8,  7,  7,  4,  4,  2,  2,  3,  3,  5,  5,  7,  7,  8,  8,
        # Bar 4: E5 B4 G4 E4 turnaround
        9,  9,  7,  7,  5,  5,  3,  3,  9,  9,  7,  7,  0,  0,  0,  0,

        # Bar 5: "Wir sind die Ro-bo-ter"
        3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
        # Bar 6: "Wir sind die Ro-bo-ter"
        3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
        # Bar 7: "Ja tvoi sluga, ja tvoi rabotnik"
        1,  1,  2,  2,  3,  3,  5,  5,  5,  4,  3,  3,  2,  2,  3,  0,
        # Bar 8: "Wir sind die Ro-bo-ter"
        3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
    ]

    # Bar roots (Em, Em, D, Em, Em, Em, C, Em)
    bar_roots = [82.41, 82.41, 73.42, 82.41, 82.41, 82.41, 65.41, 82.41]

    # 50Hz Fast SID Arp Chords (Em, D, C, B)
    sid_arp_chords = [
        [329.63, 392.00, 493.88], # Em: E4, G4, B4
        [293.66, 369.99, 440.00], # D:  D4, F#4, A4
        [261.63, 329.63, 392.00], # C:  C4, E4, G4
        [246.94, 311.13, 369.99], # B:  B3, D#4, F#4
    ]

    v1_phase = 0
    v2_phase = 0
    v3_phase = 0
    v1_env = 0
    v2_env = 0
    v3_env = 0
    v3_freq = 0
    v3_is_drum = 0
    lfsr = 0x5183
    sample_idx = 0
    prev_note_idx = 0
    flt_lp = 0.0
    flt_bp = 0.0

    for step in range(total_steps):
        bar = step >> 4
        bar_step = step & 15
        chord_idx = bar & 3

        # Lead melody
        note_idx = lead_melody[step]
        lead_freq = roboter_notes[note_idx]
        inc_lead = int(lead_freq * (65536.0 / sample_rate)) if lead_freq > 0 else 0
        if note_idx > 0:
            if note_idx != prev_note_idx or (bar_step & 1) == 0:
                v1_env = 255
        else:
            v1_env = 0
        prev_note_idx = note_idx

        # Voice 2: SID Tracker Arp Trigger
        v2_env = 190

        # Voice 3: Bass & Drums
        root_f = bar_roots[bar]
        if bar_step in (0, 8):
            v3_is_drum = 2 # Kick
            v3_freq = 980
            v3_env = 255
        elif bar_step in (4, 12):
            v3_is_drum = 1 # Snare / Clap
            v3_env = 240
        else:
            v3_is_drum = 0 # Bass (octave bounce)
            bass_f = (root_f * 2.0) if (bar_step & 1) else root_f
            v3_freq = int(bass_f * (65536.0 / sample_rate))
            v3_env = 240

        for s in range(samples_per_step):
            # 50Hz Fast Tracker Arp clock (every 200 samples)
            arp_idx = (sample_idx // 180) % 3
            arp_freq = sid_arp_chords[chord_idx][arp_idx]
            inc_arp = int(arp_freq * (65536.0 / sample_rate))

            # LFSR Noise generator
            if lfsr & 1:
                lfsr = (lfsr >> 1) ^ 0xB400
            else:
                lfsr = lfsr >> 1

            # ADSR Envelopes
            if (sample_idx & 7) == 0:
                d = 16 if (v3_is_drum == 1) else (14 if v3_is_drum == 2 else 10)
                if v3_env > d: v3_env -= d
                else: v3_env = 0
                if v2_env > 8: v2_env -= 8
                else: v2_env = 0
            if (sample_idx & 15) == 0:
                if v1_env > 4: v1_env -= 4

            # Voice 1: SID Dynamic PWM Brass Lead (~4Hz LFO chorus)
            s1 = 0
            if inc_lead > 0 and v1_env > 0:
                v1_phase = (v1_phase + inc_lead) & 0xFFFF
                lfo = (sample_idx & 0x80) if not (sample_idx & 0x100) else (255 - (sample_idx & 0x80))
                dyn_pw = 32768 + (lfo - 128) * 120
                pulse1 = 245 if (v1_phase < dyn_pw) else 15
                s1 = (pulse1 * v1_env) >> 8

            # Voice 2: SID 25% Duty Cycle Pulse Wave Fast Arp
            s2 = 0
            if v2_env > 0:
                v2_phase = (v2_phase + inc_arp) & 0xFFFF
                pulse2 = 230 if ((v2_phase >> 8) < 64) else 25
                s2 = (pulse2 * v2_env) >> 8

            # Voice 3: SID Bass & Drums
            s3 = 0
            if v3_env > 0:
                if v3_is_drum == 1:
                    noise = lfsr & 0xFF
                    s3 = (noise * v3_env) >> 8
                elif v3_is_drum == 2:
                    v3_phase = (v3_phase + (v3_freq << 1)) & 0xFFFF
                    if v3_freq > 40: v3_freq -= 4
                    tri = v3_phase >> 8
                    if tri > 127: tri = 255 - tri
                    s3 = ((tri << 1) * v3_env) >> 8
                else:
                    v3_phase = (v3_phase + v3_freq) & 0xFFFF
                    tri = v3_phase >> 8
                    if tri > 127: tri = 255 - tri
                    s3 = ((tri << 1) * v3_env) >> 8

            # 3-Voice Mix
            dry = (s3 + (s1 << 1) + s2) >> 2

            # MOS 6581 Resonant Filter Simulation
            cutoff = 0.65
            res = 0.25
            res_in = dry - (flt_bp * res)
            flt_lp += (res_in - flt_lp) * cutoff
            flt_bp = res_in - flt_lp

            filtered = int(flt_lp)
            samples[sample_idx] = min(255, max(0, filtered))
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 15. atari_punk_console.cpp — Atari Punk Console Deluxe
# ---------------------------------------------------------------------------
def synth_atari_punk_console():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase_astable = 0
    mono_timer = 0
    mono_state = 0
    sub1_state = 0
    sub2_state = 0
    sub_count = 0

    for i in range(total_samples):
        t = i / float(sample_rate)
        # Sweeping pitch from ~120 Hz to 1200 Hz with harmonic modulation
        f_pitch = 140.0 + math.sin(t * 1.5) * 110.0 + (t / 6.0) * 350.0
        inc_astable = int(f_pitch * (65536.0 / sample_rate))

        # Sweeping monostable width to hit stepped tone jumps
        mono_width = int(15 + (math.sin(t * 3.2) * 0.5 + 0.5) * 220)

        prev_phase = phase_astable
        phase_astable = (phase_astable + inc_astable) & 0xFFFF
        astable_trig = (phase_astable < prev_phase)

        if astable_trig:
            sub_count += 1
            sub1_state ^= 1
            if (sub_count & 3) == 0:
                sub2_state ^= 1
            if mono_timer == 0:
                mono_state = 1
                mono_timer = mono_width

        if mono_timer > 0:
            mono_timer -= 1
            if mono_timer == 0:
                mono_state = 0

        # Sound mode demonstration:
        mono_pulse = 240 if mono_state else 15
        astable_sq = 240 if (phase_astable < 32768) else 15

        if t < 1.5:
            out = mono_pulse
        elif t < 3.5:
            s1 = 230 if sub1_state else 25
            s2 = 240 if sub2_state else 15
            out = (mono_pulse + s1 + (s2 << 1)) >> 2
        elif t < 5.0:
            out = astable_sq ^ mono_pulse
        else:
            out = mono_pulse

        samples[i] = min(255, max(0, out))

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 16. benjolin_noise.cpp — Benjolin / Rungler Chaotic Noise Synth
# ---------------------------------------------------------------------------
def synth_benjolin_noise():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase1 = 0
    phase2 = 0
    rungler_sr = 0xAC
    flt_lp = 0.0
    flt_bp = 0.0

    def tri(p):
        t = (p >> 8) & 0xFF
        return (t << 1) if t < 128 else ((255 - t) << 1)

    for i in range(total_samples):
        t = i / float(sample_rate)

        # Sweeping master VCO1 clock and VCO2
        base_f1 = 80.0 + (math.sin(t * 1.8) * 0.5 + 0.5) * 450.0
        base_f2 = 120.0 + (math.cos(t * 1.2) * 0.5 + 0.5) * 600.0

        r_bits = ((rungler_sr & 0x01) << 2) | (rungler_sr & 0x02) | ((rungler_sr & 0x04) >> 2)
        cv_mod = (r_bits - 3.5) * 35.0

        f1 = max(15.0, base_f1 + cv_mod)
        f2 = max(20.0, base_f2 + cv_mod * 0.6)

        inc1 = int(f1 * (65536.0 / sample_rate))
        inc2 = int(f2 * (65536.0 / sample_rate))

        prev_p1 = phase1
        phase1 = (phase1 + inc1) & 0xFFFF
        vco1_clk = (phase1 < prev_p1)

        phase2 = (phase2 + inc2) & 0xFFFF

        if vco1_clk:
            data_bit = 1 if (phase2 < 32768) else 0
            rungler_sr = ((rungler_sr << 1) | data_bit) & 0xFF

        t1 = tri(phase1)
        t2 = tri(phase2)

        # Sound modes over 6 seconds
        if t < 2.5:
            # Classic Benjolin Rungler
            raw = ((t1 + t2) >> 1) ^ (0x20 if (rungler_sr & 0x80) else 0x00)
        elif t < 4.5:
            # Industrial wavefolded saturation
            sq2 = 240 if phase2 < 32768 else 15
            raw = (t1 ^ sq2) + (r_bits << 4)
        else:
            # Screaming filter band-pass
            raw = t1 ^ t2

        # Resonant filter
        cutoff = 0.45 + (r_bits / 7.0) * 0.35
        res = 0.28
        res_in = raw - 128 - (flt_bp * res)
        flt_lp += (res_in - flt_lp) * cutoff
        flt_bp = res_in - flt_lp

        out = int(flt_lp + 128) if t < 4.5 else int(flt_bp + 128)
        samples[i] = min(255, max(0, out))

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 17. dub_siren_echo.cpp — Jamaican Dub Siren & Tape Echo
# ---------------------------------------------------------------------------
def synth_dub_siren_echo():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase_osc = 0
    phase_lfo = 0
    delay_buf = [128] * 256
    echo_write_idx = 0
    delay_decim = 0

    for i in range(total_samples):
        t = i / float(sample_rate)

        # 4 Siren phases:
        if t < 1.8:
            # Laser sweep
            lfo_rate = int(4.0 * (65536.0 / sample_rate))
            phase_lfo = (phase_lfo + lfo_rate) & 0xFFFF
            ramp = 255 - (phase_lfo >> 8)
            f = 500.0 + ramp * 4.5
        elif t < 3.4:
            # UFO warble
            lfo_rate = int(7.5 * (65536.0 / sample_rate))
            phase_lfo = (phase_lfo + lfo_rate) & 0xFFFF
            tri = (phase_lfo >> 8)
            tri = (tri << 1) if tri < 128 else ((255 - tri) << 1)
            f = 900.0 + (tri - 128) * 4.0
        elif t < 4.8:
            # Hi-Lo Klaxon
            lfo_rate = int(3.2 * (65536.0 / sample_rate))
            phase_lfo = (phase_lfo + lfo_rate) & 0xFFFF
            f = 1100.0 if (phase_lfo < 32768) else 650.0
        else:
            # Airhorn blast + runaway
            f = 2200.0 + (350.0 if (i & 512) else 0.0)

        inc = int(f * (65536.0 / sample_rate))
        phase_osc = (phase_osc + inc) & 0xFFFF
        dry = 230 if (phase_osc < 32768) else 25

        # Vintage tape echo (decimated to 1.25 kHz)
        delay_decim = (delay_decim + 1) & 7
        delay_len = 160
        if delay_decim == 0:
            read_idx = (echo_write_idx - delay_len) & 255
            echo_samp = delay_buf[read_idx]

            gain = 0.95 if t >= 4.8 else 0.65
            fb_sig = echo_samp - 128
            mixed = (dry - 128) + int(fb_sig * gain)
            mixed = max(-110, min(110, mixed))

            delay_buf[echo_write_idx] = mixed + 128
            echo_write_idx = (echo_write_idx + 1) & 255

        read_idx = (echo_write_idx - delay_len) & 255
        wet = delay_buf[read_idx]
        out = (dry + (wet << 1)) // 3
        samples[i] = min(255, max(0, out))

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 18. industrial_cyberdrone.cpp — Industrial Cyber-Drone Synth
# ---------------------------------------------------------------------------
def synth_industrial_cyberdrone():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase1 = 0
    phase2 = 0
    phase3 = 0
    phase4 = 0
    flt_lp = 0.0
    flt_bp = 0.0
    lfsr = 0xB482

    def tri(p):
        t = (p >> 8) & 0xFF
        return (t << 1) if t < 128 else ((255 - t) << 1)

    for i in range(total_samples):
        t = i / float(sample_rate)

        # LFSR noise
        if lfsr & 1: lfsr = (lfsr >> 1) ^ 0xB400
        else: lfsr >>= 1
        noise = lfsr & 0xFF

        # Master cluster pitch
        master_p = 120.0 + math.sin(t * 1.3) * 60.0 + (t / 6.0) * 180.0
        depth = 60.0 + (math.sin(t * 2.1) * 0.5 + 0.5) * 140.0

        o1 = tri(phase1)
        o2 = tri(phase2)
        o3 = tri(phase3)
        o4 = tri(phase4)

        # 4-node cross FM ring
        m1 = (o4 - 128) * (depth / 128.0)
        m2 = (o1 - 128) * (depth / 128.0)
        m3 = (o2 - 128) * (depth / 128.0)
        m4 = (o3 - 128) * (depth / 128.0)

        # Geiger noise spikes between 3.2s and 4.8s
        if 3.2 < t < 4.8 and noise > 220:
            m1 += 140.0

        f1 = max(10.0, master_p + m1)
        f2 = max(10.0, master_p * 1.3 + m2)
        f3 = max(10.0, master_p * 1.7 + m3)
        f4 = max(10.0, master_p * 2.2 + m4)

        phase1 = (phase1 + int(f1 * (65536.0 / sample_rate))) & 0xFFFF
        phase2 = (phase2 + int(f2 * (65536.0 / sample_rate))) & 0xFFFF
        phase3 = (phase3 + int(f3 * (65536.0 / sample_rate))) & 0xFFFF
        phase4 = (phase4 + int(f4 * (65536.0 / sample_rate))) & 0xFFFF

        sum_sig = (o1 + o2 + o3 + o4) // 4
        if t > 4.8:
            # Overdrive wavefold
            sum_sig = sum_sig ^ (o2 >> 1)

        # Diode wavefold & saturation
        drive = 1.4 if t > 2.0 else 0.9
        sat = int((sum_sig - 128) * drive)
        sat = max(-115, min(115, sat))

        # Resonant filter
        cutoff = 0.55 + math.sin(t * 1.5) * 0.25
        res = 0.32 if t > 4.5 else 0.18
        res_in = sat - (flt_bp * res)
        flt_lp += (res_in - flt_lp) * cutoff
        flt_bp = res_in - flt_lp

        out = int(flt_lp + 128) if t < 4.8 else int(flt_bp + 128)
        samples[i] = min(255, max(0, out))

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 19. soma_lyra.cpp — SOMA Lyra-8 Organismic Synth
# ---------------------------------------------------------------------------
def synth_soma_lyra():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phases = [0] * 8
    lfo_a_phase = 0
    lfo_b_phase = 0
    delay_buf = [128] * 96
    delay_idx = 0
    delay_decim = 0

    def tri_wave(p):
        v = (p >> 8) & 0xFF
        return (v << 1) if v < 128 else ((255 - v) << 1)

    for i in range(total_samples):
        t = i / float(sample_rate)

        # Hyper LFO
        lfo_a_phase = (lfo_a_phase + int(1.2 * (65536.0 / sample_rate))) & 0xFFFF
        lfo_b_phase = (lfo_b_phase + int(0.7 * (65536.0 / sample_rate))) & 0xFFFF
        lfo_a_val = tri_wave(lfo_a_phase)
        lfo_b_val = (lfo_b_phase >> 8) & 0xFF

        # Non-linear combination (SOMA Hyper LFO logic)
        hyper_lfo = (lfo_a_val & lfo_b_val) ^ (lfo_b_val >> 1)
        hyper_mod = (hyper_lfo - 128) * 0.4

        # Pitch contour: gentle evolving cluster building into harmonic swells
        base_f = 65.0 + math.sin(t * 0.8) * 20.0 + (t / 6.0) * 45.0
        fm_depth = 40.0 + (math.sin(t * 1.5) * 0.5 + 0.5) * 75.0 + hyper_mod

        # Calculate 8 voice outputs
        v_outs = [tri_wave(p) for p in phases]

        # Inter-voice FM modulation
        # Pair 1: V1 modulated by V2, V2 modulated by V1
        m1 = (v_outs[1] - 128) * (fm_depth / 128.0)
        m2 = (v_outs[0] - 128) * (fm_depth / 180.0)
        # Pair 2: V3 modulated by V4 + Pair 1 leak
        m3 = (v_outs[3] - 128) * (fm_depth / 128.0) + (v_outs[0] - 128) * 0.2
        m4 = (v_outs[2] - 128) * (fm_depth / 180.0)
        # Pair 3: V5 modulated by V6 + Sub octaves
        m5 = (v_outs[5] - 128) * (fm_depth / 128.0)
        m6 = (v_outs[4] - 128) * (fm_depth / 160.0)
        # Pair 4: V7 modulated by V8 + Hyper LFO
        m7 = (v_outs[7] - 128) * (fm_depth / 128.0) + hyper_mod * 1.2
        m8 = (v_outs[6] - 128) * (fm_depth / 150.0)

        # Harmonies & intervals (Organismic cluster: Unison, 5th, Octave, 9th)
        freqs = [
            max(15.0, base_f * 1.00 + m1),
            max(15.0, base_f * 1.01 + m2),
            max(15.0, base_f * 1.498 + m3),
            max(15.0, base_f * 1.505 + m4),
            max(15.0, base_f * 2.00 + m5),
            max(15.0, base_f * 2.015 + m6),
            max(15.0, base_f * 2.99 + m7),
            max(15.0, base_f * 3.02 + m8),
        ]

        for vi in range(8):
            inc = int(freqs[vi] * (65536.0 / sample_rate))
            phases[vi] = (phases[vi] + inc) & 0xFFFF

        # Sum of 8 voices
        dry_sum = sum(v_outs) // 8

        # SOMA Lo-Fi BBD Dual Delay (decimated)
        delay_decim = (delay_decim + 1) & 7
        if delay_decim == 0:
            delay_len = 72
            read1 = (delay_idx - delay_len) % 96
            read2 = (delay_idx - int(delay_len * 0.75)) % 96
            delay_sig = (delay_buf[read1] + delay_buf[read2]) // 2

            fb_gain = 0.85 if t > 3.5 else 0.60
            mixed = (dry_sum - 128) + int((delay_sig - 128) * fb_gain)
            # Diode saturation
            sat = max(-115, min(115, int(mixed * 1.2)))
            delay_buf[delay_idx] = sat + 128
            delay_idx = (delay_idx + 1) % 96

        read1 = (delay_idx - 72) % 96
        wet = delay_buf[read1]

        # Final mix with Drive saturation
        drive = 1.3 if t > 2.5 else 1.0
        final_sig = int((((dry_sum * 2 + wet) // 3) - 128) * drive)
        out = max(0, min(255, final_sig + 128))
        samples[i] = out

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 20. bytebeat_explorer.cpp — Bytebeat Explorer
# ---------------------------------------------------------------------------
def synth_bytebeat_explorer():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    t = 0
    t_step = 256 # normal speed

    for i in range(total_samples):
        sec = i / float(sample_rate)
        # Advance time
        t += t_step
        tt = (t >> 8) & 0xFFFFFFFF

        if sec < 1.5:
            # 0: The 42 Melody
            out = (tt * ((tt >> 12) | (tt >> 8)) & 63 & (tt >> 4)) & 255
        elif sec < 3.0:
            # 1: Algorithmic Arp
            out = ((tt * 5 & (tt >> 7)) | (tt * 3 & (tt >> 10))) & 255
        elif sec < 4.5:
            # 2: Chiptune Counter
            out = (tt * ((tt >> 9) | (tt >> 13)) & 16) & 255
        else:
            # 3: Sierpinski Glitch
            out = (((tt * (tt >> 8) * (tt >> 12) & 64) + (((tt >> 5) & (tt >> 7)) & (tt >> 3)))) & 255

        # Boost range
        out = ((out << 2) | (out >> 6)) & 255
        samples[i] = out

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 21. bytebeat_techno.cpp — Bytebeat Techno & Acid
# ---------------------------------------------------------------------------
def synth_bytebeat_techno():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    t = 0
    t_step = 270

    for i in range(total_samples):
        sec = i / float(sample_rate)
        t += t_step
        tt = (t >> 8) & 0xFFFFFFFF

        if sec < 1.5:
            # 0: Driving Cyber Techno
            shift = (tt >> 16) & 31
            out = (((tt >> 6) | tt | (tt >> shift)) * 10 + ((tt >> 11) & 7)) & 255
        elif sec < 3.0:
            # 1: Heavy Industrial Stomp
            out = ((((tt >> 7 | tt | tt >> 6) * 10 + 4 * ((tt & (tt >> 13)) | (tt >> 6))))) & 255
        elif sec < 4.5:
            # 2: Modular Acid 303
            out = (tt * (((tt >> 12 | tt >> 8) & (tt >> 4) & (tt >> 7)))) & 255
        else:
            # 3: Dystopian Beat
            mod11 = (((tt >> 9) - 1) % 11) if (((tt >> 9) - 1) % 11) else 1
            out = (((tt >> 5) | (tt << 4) | ((tt >> 10) ^ (mod11 * tt)))) & 255

        samples[i] = out

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 22. bytebeat_ambient.cpp — Bytebeat Ambient & Fractals
# ---------------------------------------------------------------------------
def synth_bytebeat_ambient():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    t = 0
    t_step = 180

    for i in range(total_samples):
        sec = i / float(sample_rate)
        t += t_step
        tt = (t >> 8) & 0xFFFFFFFF

        if sec < 1.5:
            # 0: Cosmic Choir
            out = (((tt >> 6 | tt >> 7 | tt >> 8) * (tt >> 4 & tt >> 8))) & 255
        elif sec < 3.0:
            # 1: Starlight Arp
            out = ((((tt * 9 & tt >> 4) | (tt * 5 & tt >> 7) | (tt * 3 & tt >> 10)) - 1)) & 255
        elif sec < 4.5:
            # 2: Alien Pulsar
            out = ((((tt * ((tt >> 14 & tt >> 8) & 248) & 128) + ((tt >> 8 & tt >> 13) & (tt >> 3))))) & 255
        else:
            # 3: Fractal Void
            out = ((((tt >> 7) * (tt >> 11) ^ ((tt >> 5) * (tt >> 9))))) & 255

        # Soft ambient compression
        out = ((out >> 1) + 64) & 255
        samples[i] = out

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 23. bach_air_g_string.cpp — Baroque Polyphonic Suite
# ---------------------------------------------------------------------------
def synth_bach_air_g_string():
    sample_rate = 10000
    step_duration = 0.125 # 125 ms per step
    samples_per_step = int(sample_rate * step_duration)

    # 64-step Bach Air
    melody = [
        370, 370, 370, 370, 370, 370, 392, 440,
        494, 494, 494, 440, 392, 370, 330, 277,
        294, 294, 294, 330, 370, 392, 440, 494,
        554, 554, 554, 494, 440, 415, 494, 415,
        440, 440, 440, 440, 440, 440, 494, 554,
        587, 587, 587, 587, 554, 554, 494, 494,
        440, 440, 392, 392, 370, 370, 330, 330,
        294, 294, 294, 294, 294, 294, 294, 294
    ]
    bass = [
        147, 73, 139, 69, 123, 123, 110, 110,
        98, 98, 92, 92, 82, 82, 110, 110,
        73, 147, 139, 123, 110, 98, 92, 82,
        92, 98, 104, 110, 110, 98, 92, 82,
        110, 110, 98, 92, 82, 73, 69, 110,
        147, 73, 139, 69, 123, 123, 110, 110,
        98, 92, 82, 73, 110, 110, 110, 110,
        73, 73, 110, 110, 73, 73, 73, 73
    ]
    arp = [
        220, 294, 370, 294, 220, 277, 330, 277,
        147, 196, 247, 196, 147, 185, 220, 165,
        185, 220, 294, 220, 165, 220, 277, 185,
        139, 165, 220, 165, 147, 208, 247, 208,
        165, 220, 277, 220, 165, 220, 277, 220,
        185, 220, 294, 220, 165, 196, 277, 196,
        147, 196, 247, 196, 139, 165, 220, 165,
        147, 185, 220, 185, 147, 185, 220, 294
    ]

    total_steps = len(melody)
    total_samples = samples_per_step * total_steps
    samples = bytearray(total_samples)

    phase_bass = 0
    phase_arp = 0
    phase_lead = 0

    cur_mel = 0
    note_age = 0
    sample_idx = 0

    for step in range(total_steps):
        mel_f = melody[step]
        bass_f = bass[step]
        arp_f = arp[step]

        inc_bass = int(bass_f * (65536.0 / sample_rate))
        inc_arp = int(arp_f * (65536.0 / sample_rate))
        inc_lead = int(mel_f * (65536.0 / sample_rate))

        if mel_f != cur_mel:
            cur_mel = mel_f
            note_age = 0

        for _ in range(samples_per_step):
            note_age += 1

            # Bass: Warm Cello Triangle
            phase_bass = (phase_bass + inc_bass) & 0xFFFF
            tb = phase_bass >> 8
            if tb > 127: tb = 255 - tb
            s_bass = tb << 1

            # Arp: Plucked Harpsichord (Saw + Pulse pluck)
            phase_arp = (phase_arp + inc_arp) & 0xFFFF
            saw = phase_arp >> 8
            pluck = (saw >> 1) + (64 if (phase_arp & 0x4000) else 0)
            env_arp = max(40, 220 - ((note_age % samples_per_step) * 180 // samples_per_step))
            s_arp = (pluck * env_arp) >> 8

            # Lead: Cantabile Violin with delayed vibrato
            vib = 0
            if note_age > 160: # ~16ms delay
                vib = math.sin((sample_idx - 160) * 0.0035) * min(6.0, (note_age - 160) * 0.005)
            phase_lead = (phase_lead + inc_lead + int(vib * 12)) & 0xFFFF
            tl = phase_lead >> 8
            if tl > 127: tl = 255 - tl
            s_lead = tl << 1

            mix = (s_bass * 80 + s_arp * 70 + s_lead * 120) >> 8
            samples[sample_idx] = min(255, max(0, mix))
            sample_idx += 1

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 24. schaltkreiskontrolle.cpp — Speech Chip "Schaltkreiskontrolle!"
# ---------------------------------------------------------------------------
def synth_schaltkreiskontrolle():
    sample_rate = 10000
    header = ROOT / "include" / "speech.h"
    text = header.read_text()
    body = text[text.find("{") + 1 : text.find("}")]
    raw = [int(x.strip()) for x in body.split(",") if x.strip().isdigit()]

    samples = bytearray()

    def play_pass(speed_us, crush_bits):
        step = (1000000.0 / speed_us) / sample_rate
        pos = 0.0
        while pos < len(raw):
            idx = int(pos)
            s = raw[idx]
            if crush_bits > 0:
                s = (s >> crush_bits) << crush_bits
            samples.append(s)
            pos += step

    # Pass 1: Normal clean speech (delay ~290us)
    play_pass(speed_us=290, crush_bits=0)
    # 0.3s pause
    samples.extend([128] * 3000)
    # Pass 2: Pitch-shifted deep bitcrushed robotic voice
    play_pass(speed_us=400, crush_bits=3)

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 25. theremin.cpp — Léon Theremin 1920 Continuous Space Synth
# ---------------------------------------------------------------------------
def synth_theremin():
    sample_rate = 10000
    duration_sec = 6.0
    total_samples = int(sample_rate * duration_sec)
    samples = bytearray(total_samples)

    phase1 = 0.0
    phase2 = 0.0
    phase_vib = 0.0

    for i in range(total_samples):
        t = i / sample_rate

        # 5.5 Hz natural finger vibrato
        phase_vib += 5.5 * 2 * math.pi / sample_rate
        vib = math.sin(phase_vib) * 0.02

        if t < 2.5:
            # Phase 1: Pure Clara Rockmore sine gliding upward (220 Hz to 660 Hz)
            glide = (t / 2.5) ** 1.5
            freq = (220.0 + glide * 440.0) * (1.0 + vib)
            vol = min(1.0, t * 2.0)
            phase1 += freq * 2 * math.pi / sample_rate
            raw = math.sin(phase1)
        elif t < 4.5:
            # Phase 2: Sci-Fi Heterodyne UFO ring-mod wail (400 Hz to 850 Hz with dual osc)
            t_sub = t - 2.5
            freq1 = (440.0 + math.sin(t_sub * 4.0) * 200.0) * (1.0 + vib)
            freq2 = freq1 * 1.35 + 25.0
            vol = 0.9
            phase1 += freq1 * 2 * math.pi / sample_rate
            phase2 += freq2 * 2 * math.pi / sample_rate
            raw = math.sin(phase1) * math.sin(phase2) * 1.5
        else:
            # Phase 3: Vocal / Cello descent with volume fade
            t_sub = t - 4.5
            freq = (550.0 - (t_sub / 1.5) * 350.0) * (1.0 + vib * 1.5)
            vol = max(0.0, 1.0 - (t_sub / 1.5) ** 2)
            phase1 += freq * 2 * math.pi / sample_rate
            # Odd harmonic warmth
            raw = (math.sin(phase1) * 0.7 + math.sin(phase1 * 3) * 0.3)

        val = int(128 + raw * vol * 110)
        samples[i] = min(255, max(0, val))

    return samples, sample_rate


# ---------------------------------------------------------------------------
# 26. spaghetti.cpp — Spaghetti! The Italian Talking Chip
# ---------------------------------------------------------------------------
def synth_spaghetti():
    sample_rate = 10000
    header = ROOT / "include" / "spageti.h"
    text = header.read_text()
    body = text[text.find("{") + 1 : text.find("}")]
    raw = [int(x.strip()) for x in body.split(",") if x.strip().isdigit()]

    samples = bytearray()

    def play_pass(speed_us, crush_bits, max_len=None):
        step = (1000000.0 / speed_us) / sample_rate
        pos = 0.0
        limit = len(raw) if max_len is None else min(len(raw), max_len)
        while pos < limit:
            idx = int(pos)
            s = raw[idx]
            if crush_bits > 0:
                s = (s >> crush_bits) << crush_bits
            samples.append(s)
            pos += step

    # Pass 1: Clean natural speech (speed ~260 us)
    play_pass(speed_us=260, crush_bits=0)
    # Pause
    samples.extend([128] * 2500)
    # Pass 2: Pitch-shifted deep bitcrushed cyber voice
    play_pass(speed_us=380, crush_bits=3)
    # Pause
    samples.extend([128] * 2500)
    # Pass 3: Stutter "Spag-Spaghetti!"
    play_pass(speed_us=220, crush_bits=1, max_len=len(raw) // 2)
    samples.extend([128] * 800)
    play_pass(speed_us=240, crush_bits=0)

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
        ("kraftwerk_model", synth_kraftwerk_model),
        ("kraftwerk_roboter", synth_kraftwerk_roboter),
        ("atari_punk_console", synth_atari_punk_console),
        ("benjolin_noise", synth_benjolin_noise),
        ("dub_siren_echo", synth_dub_siren_echo),
        ("industrial_cyberdrone", synth_industrial_cyberdrone),
        ("soma_lyra", synth_soma_lyra),
        ("bytebeat_explorer", synth_bytebeat_explorer),
        ("bytebeat_techno", synth_bytebeat_techno),
        ("bytebeat_ambient", synth_bytebeat_ambient),
        ("bach_air_g_string", synth_bach_air_g_string),
        ("schaltkreiskontrolle", synth_schaltkreiskontrolle),
        ("theremin", synth_theremin),
        ("spaghetti", synth_spaghetti),
    ]

    for name, fn in synths:
        samples, rate = fn()
        out = (AUDIO_DIR / f"{name}.mp3").resolve()
        write_wav_to_mp3(samples, rate, out)

if __name__ == "__main__":
    main()





