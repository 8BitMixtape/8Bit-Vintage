#!/usr/bin/env python3
"""
Render animated GIF previews of the 5x4 LED matrix for each sound app into
docs/gifs/<id>.gif, using the same cell style as the old emulator
(30x24 px cells with a 1 px dark border -> 150x96 GIF).

Usage:
  python3 scripts/gen_previews.py              # all apps with a renderer
  python3 scripts/gen_previews.py drone_synth  # selected ids

Requires Pillow (pip install pillow).
"""
import json
import math
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
APPS_JSON = ROOT / "docs" / "apps.json"
GIF_DIR = ROOT / "docs" / "gifs"

MATRIX_W, MATRIX_H = 5, 4
CELL_W, CELL_H = 30, 24
BG = (10, 10, 10)


def render_frame(pixels):
    """pixels: list of MATRIX_H rows of MATRIX_W (r,g,b) tuples."""
    img = Image.new("RGB", (MATRIX_W * CELL_W, MATRIX_H * CELL_H), BG)
    px = img.load()
    for y in range(MATRIX_H):
        for x in range(MATRIX_W):
            c = pixels[y][x]
            for dy in range(1, CELL_H - 1):
                for dx in range(1, CELL_W - 1):
                    px[x * CELL_W + dx, y * CELL_H + dy] = c
    return img


# ---------------------------------------------------------------------------
# 1. drone_synth.cpp — "Trickster Plasma" cycling through 4 modes
# ---------------------------------------------------------------------------
DRONE_THEMES = [
    ((30, 15, 0), (255, 220, 60)),   # 0 Triangle  - yellow/brown
    ((40, 0, 0), (255, 50, 0)),      # 1 Fuzz      - deep red/orange
    ((20, 10, 0), (200, 100, 0)),    # 2 Bitcrush  - burnt orange
    ((30, 0, 10), (255, 20, 60)),    # 3 RingMod   - crimson
]


def drone_synth_frames():
    frames_per_mode = 30          # 1.5 s per mode at 50 ms/frame
    speed, zoom = 1.0, 1.5
    p1 = p2 = p3 = 0.0
    frames = []
    for mode, (dark, bright) in enumerate(DRONE_THEMES):
        for _ in range(frames_per_mode):
            p1 += 0.18 * speed
            p2 += 0.11 * speed
            p3 += 0.08 * speed
            rows = []
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    zx, zy = x * zoom, y * zoom
                    v = (math.sin((zx + p1) * 0.8)
                         + math.cos((zy - p2) * 1.1)
                         + math.sin((zx + zy + p3) * 0.6)
                         + 0.6 * math.sin((zx * 0.5 - zy * 0.7) + p2 * 0.9))
                    v = min(max(v / 7.2 + 0.5, 0.0), 1.0)
                    vv = min(max(v + math.sin(p1 * 0.25) * 0.15, 0.0), 1.0)
                    vv = min(vv * 1.05, 1.0) ** 2.4
                    if vv < 0.06:
                        vv = 0.0
                    a = int(vv * 255 + 0.5)
                    row.append(tuple(d + ((b - d) * a >> 8) for d, b in zip(dark, bright)))
                rows.append(row)
            frames.append(render_frame(rows))
    return frames, 50


# ---------------------------------------------------------------------------
# 2. peepsy_acid.cpp — TB-303 Acid sequencer (Red/White flash + pattern bar)
# ---------------------------------------------------------------------------
def peepsy_acid_frames():
    drum_seq = [1, 0, 2, 0, 1, 0, 2, 0, 1, 2, 2, 1, 1, 0, 2, 2] # 1=Kick, 2=Hat, 0=Rest
    accents  = [1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1]
    frames = []

    for step in range(16):
        d = drum_seq[step]
        acc = accents[step]
        bass_vis = 140 if acc else 70
        drum_vis = 130 if d == 1 else (60 if d == 2 else 0)

        rows = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == 0 else (30, 0, 0))
                else:
                    r = min(255, bass_vis + drum_vis)
                    g = drum_vis
                    b = drum_vis
                    row.append((r, g, b))
            rows.append(row)
        frames.append(render_frame(rows))

        # Decay
        bass_decay = bass_vis // 2
        drum_decay = drum_vis // 4
        rows_decay = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == 0 else (30, 0, 0))
                else:
                    r = min(255, bass_decay + drum_decay)
                    g = drum_decay
                    b = drum_decay
                    row.append((r, g, b))
            rows_decay.append(row)
        frames.append(render_frame(rows_decay))

    return frames, 60


# ---------------------------------------------------------------------------
# 3. flappy_slider.cpp — Flappy bird dodging pipes
# ---------------------------------------------------------------------------
def flappy_slider_frames():
    frames = []
    total_frames = 40
    pipe_x = 4.5
    pipe_gap_y = 1

    for f in range(total_frames):
        pipe_x -= 0.25
        if pipe_x < -1.0:
            pipe_x = 4.5
            pipe_gap_y = (pipe_gap_y % 2) + 1

        bird_y = 1 if (math.sin(f * 0.35) > 0) else 2
        bird_x = 1

        bg_col = (0, 10, 20)
        wall_col = (0, 150, 0)
        bird_col = (255, 220, 0)

        rows = [[bg_col for _ in range(MATRIX_W)] for _ in range(MATRIX_H)]
        px_int = int(round(pipe_x))
        if 0 <= px_int < MATRIX_W:
            for y in range(MATRIX_H):
                if y != pipe_gap_y and y != (pipe_gap_y + 1):
                    rows[y][px_int] = wall_col

        rows[bird_y][bird_x] = bird_col
        frames.append(render_frame(rows))

    return frames, 65


# ---------------------------------------------------------------------------
# 4. bass_patterns.cpp — 808 Sub-bass visualizer + snare corner flashes
# ---------------------------------------------------------------------------
def bass_patterns_frames():
    kicks  = [1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0]
    snares = [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1]
    frames = []

    for step in range(16):
        is_kick = kicks[step]
        is_snare = snares[step]

        rows = [[(5, 5, 10) for _ in range(MATRIX_W)] for _ in range(MATRIX_H)]
        if is_kick:
            for y in range(4):
                rows[y][2] = (0, 220, 255)
            for y in range(1, 4):
                rows[y][1] = (140, 40, 255)
                rows[y][3] = (140, 40, 255)
            for y in range(2, 4):
                rows[y][0] = (220, 0, 180)
                rows[y][4] = (220, 0, 180)

        if is_snare:
            white = (240, 240, 240)
            rows[0][0] = white
            rows[0][1] = white
            rows[0][3] = white
            rows[0][4] = white

        frames.append(render_frame(rows))

        rows_decay = [[(3, 3, 5) for _ in range(MATRIX_W)] for _ in range(MATRIX_H)]
        if is_kick:
            rows_decay[3][2] = (0, 100, 140)
            rows_decay[3][1] = (60, 20, 120)
            rows_decay[3][3] = (60, 20, 120)

        frames.append(render_frame(rows_decay))

    return frames, 55


# ---------------------------------------------------------------------------
# 5. minimal_techno.cpp — Rolling bassline & minimal techno groove
# ---------------------------------------------------------------------------
def minimal_techno_frames():
    drum_seq = [1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2]
    accents  = [1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0]
    frames = []

    for step in range(16):
        d = drum_seq[step]
        acc = accents[step]
        bass_vis = 130 if acc else 60
        drum_vis = 120 if d == 1 else 70

        rows = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == 1 else (0, 20, 20))
                else:
                    r = drum_vis
                    g = min(255, bass_vis + drum_vis)
                    b = min(255, int(bass_vis * 1.2) + drum_vis)
                    row.append((r, g, b))
            rows.append(row)
        frames.append(render_frame(rows))

        rows_decay = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == 1 else (0, 20, 20))
                else:
                    r = drum_vis // 4
                    g = min(255, (bass_vis // 2) + (drum_vis // 4))
                    b = min(255, int((bass_vis // 2) * 1.2) + (drum_vis // 4))
                    row.append((r, g, b))
            rows_decay.append(row)
        frames.append(render_frame(rows_decay))

    return frames, 60


# ---------------------------------------------------------------------------
# 6. deep_minimal.cpp — Plucky off-beat sub & syncopated percussion
# ---------------------------------------------------------------------------
def deep_minimal_frames():
    bass_hits = [0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0]
    drum_hits = [1, 0, 2, 0, 1, 0, 2, 0, 1, 0, 2, 0, 1, 0, 2, 0]
    frames = []

    for step in range(16):
        b_hit = bass_hits[step]
        d_hit = drum_hits[step]

        bass_vis = 150 if b_hit else 20
        drum_vis = 120 if d_hit == 1 else (70 if d_hit == 2 else 0)

        rows = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == 0 else (15, 0, 25))
                else:
                    r = min(255, int(bass_vis * 0.9) + drum_vis)
                    g = drum_vis
                    b = min(255, int(bass_vis * 1.4) + drum_vis)
                    row.append((r, g, b))
            rows.append(row)
        frames.append(render_frame(rows))

        rows_decay = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == 0 else (15, 0, 25))
                else:
                    r = (bass_vis // 3) + (drum_vis // 4)
                    g = drum_vis // 4
                    b = int((bass_vis // 3) * 1.3) + (drum_vis // 4)
                    row.append((r, g, b))
            rows_decay.append(row)
        frames.append(render_frame(rows_decay))

    return frames, 60


RENDERERS = {
    "drone_synth": drone_synth_frames,
    "peepsy_acid": peepsy_acid_frames,
    "flappy_slider": flappy_slider_frames,
    "bass_patterns": bass_patterns_frames,
    "minimal_techno": minimal_techno_frames,
    "deep_minimal": deep_minimal_frames,
}


def main():
    apps = json.loads(APPS_JSON.read_text())
    wanted = set(sys.argv[1:])
    GIF_DIR.mkdir(parents=True, exist_ok=True)
    for app in apps:
        aid = app["id"]
        if wanted and aid not in wanted:
            continue
        fn = RENDERERS.get(aid)
        if not fn:
            print(f"-- {aid}: no preview renderer, skipped")
            continue
        frames, ms = fn()
        out = GIF_DIR / f"{aid}.gif"
        frames[0].save(out, save_all=True, append_images=frames[1:],
                       duration=ms, loop=0, optimize=True)
        print(f"==> {out.relative_to(ROOT)} ({len(frames)} frames, {out.stat().st_size // 1024} KB)")

# ---------------------------------------------------------------------------
# 7. quantum_matrix_new.cpp — Chaotic Pink/Blue quantum granular visualizer
# ---------------------------------------------------------------------------
def quantum_matrix_frames():
    import random
    random.seed(42)
    frames = []
    matrix = [[(0, 0, 0) for _ in range(MATRIX_W)] for _ in range(MATRIX_H)]

    for f in range(40):
        intensity = int(120 + math.sin(f * 0.35) * 100)
        peak = (intensity - 220) * 2 if intensity > 220 else 0
        potBase = 180

        num_updates = 2 + (intensity >> 5)
        for _ in range(num_updates):
            px = random.randint(0, MATRIX_W - 1)
            py = random.randint(0, MATRIX_H - 1)
            r = min(255, intensity)
            g = min(255, peak)
            b = min(255, potBase)
            matrix[py][px] = (r, g, b)

        for _ in range(3):
            if random.random() < 0.4:
                px = random.randint(0, MATRIX_W - 1)
                py = random.randint(0, MATRIX_H - 1)
                matrix[py][px] = (0, 0, 0)

        frame_rows = [list(row) for row in matrix]
        frames.append(render_frame(frame_rows))

    return frames, 60

RENDERERS["quantum_matrix"] = quantum_matrix_frames

# ---------------------------------------------------------------------------
# 8. ave_maria_siberia.cpp — Soviet Red Star & Siberian Liturgical visualizer
# ---------------------------------------------------------------------------
def ave_maria_siberia_frames():
    soviet_star = [
        0, 0, 1, 0, 0,
        0, 1, 1, 1, 0,
        1, 1, 1, 1, 1,
        0, 1, 0, 1, 0
    ]
    frames = []
    total_frames = 36

    for f in range(total_frames):
        # Pulsing gold intensity
        pulse = int(190 + math.sin(f * 0.4) * 60)
        gold = (pulse, int(pulse * 0.75), 0)
        ruby_red = (45, 0, 0)
        beacon_y = 3 - (f % 4)

        rows = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                idx = y * MATRIX_W + x
                is_star = soviet_star[idx]
                if f < 24:
                    # Soviet Red Star pulsing
                    row.append(gold if is_star else ruby_red)
                else:
                    # Kremlin Beacon / Spire ascension
                    if x == 2:
                        row.append((240, 240, 255) if y == beacon_y else (200, 0, 0))
                    elif is_star and y >= 2:
                        row.append((180, 130, 0))
                    else:
                        row.append((25, 0, 0))
            rows.append(row)

        frames.append(render_frame(rows))

    return frames, 65

RENDERERS["ave_maria_siberia"] = ave_maria_siberia_frames

# ---------------------------------------------------------------------------
# 9. lambada.cpp — Copacabana Sunset Waves & Dancing Couple Visualizer
# ---------------------------------------------------------------------------
def lambada_frames():
    frames = []
    total_frames = 48
    
    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Copacabana Sunset & Waves
            sun_glow = int(170 + math.sin(f * 0.5) * 70)
            sun_color = (sun_glow, int(sun_glow * 0.75), 0)
            wave_crest = (f * 2) % MATRIX_W
            
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if y == 0:
                        row.append(sun_color)
                    elif y == 1:
                        shift = (x + f) % 5
                        row.append((max(0, 210 - shift * 25), 30 + shift * 15, 80))
                    elif y == 2:
                        shift = (x + f) % 5
                        row.append((max(0, 130 - shift * 20), 15, 150 + shift * 10))
                    else: # Ocean wave
                        if x == wave_crest and (f % 4 in (0, 1)):
                            row.append((190, 255, 255)) # Whitecap
                        else:
                            row.append((0, 120 + x * 20, 220))
                rows.append(row)
        else:
            # Mode 1: Dancing Lambada Couple swaying to 3-3-2 rhythm
            dancer_pink = (255, 20, 120)
            partner_gold = (255, 210, 0)
            foot_accent = (0, 255, 220)
            bg = (20, 0, 30)
            sway = 1 if ((f // 4) % 2 == 0) else 2

            for y in range(MATRIX_H):
                row = [bg] * MATRIX_W
                if y == 0: # Heads
                    row[sway] = dancer_pink
                    row[sway + 1] = partner_gold
                elif y == 1: # Torso
                    row[sway] = dancer_pink
                    row[sway + 1] = partner_gold
                elif y == 2: # Hips
                    hip = sway - 1 if (f % 4 < 2) else sway + 1
                    if 0 <= hip < MATRIX_W:
                        row[hip] = dancer_pink
                    row[sway] = partner_gold
                else: # Feet
                    bounce = (f % 4 == 0)
                    row[sway] = foot_accent if bounce else (40, 10, 60)
                    row[sway + 1] = foot_accent if bounce else (40, 10, 60)
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["lambada"] = lambada_frames

# ---------------------------------------------------------------------------
# 10. axel_foley.cpp — Beverly Hills Police Beacon & Outrun Synthwave Grid
# ---------------------------------------------------------------------------
def axel_foley_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Beverly Hills Police Cruiser emergency beacons
            strobe = (f * 2) & 7
            red = (255, 0, 0) if strobe < 4 else (30, 0, 0)
            blue = (0, 40, 255) if strobe >= 4 else (0, 5, 30)
            flash = (255, 255, 255) if (f % 4 == 0) else (20, 20, 20)

            for y in range(MATRIX_H):
                row = [red, red, flash, blue, blue]
                rows.append(row)
        else:
            # Mode 1: Outrun 80s Synthwave Sunset & Cyber Grid
            sun_glow = int(140 + math.sin(f * 0.4) * 80)
            sun = (sun_glow, 20, 160)
            dusk = (80, 0, 120)
            grid_shift = f & 3
            cyan_grid = (0, 220, 255)
            dark_grid = (0, 20, 40)

            # Row 0: Sun
            row0 = [(30, 0, 60), sun, (255, 220, 0), sun, (30, 0, 60)]
            # Row 1: Dusk horizon
            row1 = [dusk] * MATRIX_W
            # Row 2 & 3: Moving Cyber Grid
            row2 = [cyan_grid if ((x + grid_shift) % 2 == 0) else dark_grid for x in range(MATRIX_W)]
            row3 = [(255, 0, 180) if ((x + grid_shift) % 2 == 0 and f % 2 == 0) else dark_grid for x in range(MATRIX_W)]
            rows = [row0, row1, row2, row3]

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["axel_foley"] = axel_foley_frames

# ---------------------------------------------------------------------------
# 11. auf_der_heide.cpp — Blühendes Heidekraut & The 3 Stomps Visualizer
# ---------------------------------------------------------------------------
def auf_der_heide_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        is_stomp = (f in (14, 18, 22, 38, 42, 46))

        if is_stomp:
            # Stomp explosion in bright golden light!
            gold = (255, 215, 0)
            rows = [[gold] * MATRIX_W for _ in range(MATRIX_H)]
        elif f < 24:
            # Mode 0: Blühendes Heidekraut (Violet Flowers under Blue Alpine Sky)
            sky = (20, 80, 220)
            heath_green = (20, 140, 30)
            violet = (200, 30, 160)
            blossom = (255, 230, 80) if (f % 4 == 0) else violet

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if y == 0:
                        row.append(sky)
                    else:
                        is_flower = ((x + y + (f >> 1)) % 3 == 0)
                        row.append(blossom if is_flower else heath_green)
                rows.append(row)
        else:
            # Mode 1: Alpine Mountain peaks & clouds
            shift = (f - 24) % MATRIX_W
            sky = (10, 40, 160)
            peak = (240, 240, 255)
            mountain = (60, 80, 100)
            valley = (40, 160, 40) if (f % 2 == 0) else (20, 80, 20)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if y == 0: row.append(sky)
                    elif y == 1: row.append(peak if ((x + shift) % 3 == 0) else mountain)
                    elif y == 2: row.append(mountain)
                    else: row.append(valley)
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["auf_der_heide"] = auf_der_heide_frames

# ---------------------------------------------------------------------------
# 12. zelda_sid.cpp — The Golden Triforce & Master Sword Visualizer
# ---------------------------------------------------------------------------
def zelda_sid_frames():
    triforce_bmp = [
        0, 0, 1, 0, 0,
        0, 1, 1, 1, 0,
        1, 1, 0, 1, 1,
        1, 0, 0, 0, 1
    ]
    master_sword_bmp = [
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 1, 0, 0
    ]
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: The Golden Triforce & Sacred Realm Aura
            gold_pulse = int(180 + math.sin(f * 0.4) * 75)
            gold = (gold_pulse, int(gold_pulse * 0.82), 0)
            emerald = (0, 70 + int(math.cos(f * 0.4) * 30), 25)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    is_gold = triforce_bmp[y * MATRIX_W + x]
                    row.append(gold if is_gold else emerald)
                rows.append(row)
        else:
            # Mode 1: The Master Sword & Holy Light Beam
            silver = (220, 240, 255)
            guard = (0, 190, 255)
            pedestal = (40, 50, 70)
            beam_glow = int(120 + math.sin(f * 0.5) * 80)
            ray = (beam_glow // 2, beam_glow, 255)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    is_sword = master_sword_bmp[y * MATRIX_W + x]
                    if is_sword:
                        row.append(guard if y == 2 else silver)
                    else:
                        row.append(ray if x == 2 else pedestal)
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["zelda_sid"] = zelda_sid_frames


# ---------------------------------------------------------------------------
# 13. kraftwerk_model.cpp — Die Mensch-Maschine Mannequin & Kling Klang CRT
# ---------------------------------------------------------------------------
def kraftwerk_model_frames():
    mannequin_bmp = [
        0, 1, 1, 1, 0,
        1, 1, 0, 1, 1,
        1, 1, 0, 1, 1,
        0, 1, 0, 1, 0
    ]
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Die Mensch-Maschine (Red Shirt, Black Tie & Beating Heart)
            red_shirt = (230, 20, 20)
            black_tie = (15, 15, 18)
            head_glow = (240, 200, 180)
            heart_beat = int(180 + math.sin(f * 0.6) * 75)
            heart_color = (heart_beat, 0, 40)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    idx = y * MATRIX_W + x
                    if mannequin_bmp[idx]:
                        if y == 0:
                            row.append(head_glow)
                        elif x == 2:
                            row.append(heart_color if y == 1 else black_tie)
                        else:
                            row.append(red_shirt)
                    else:
                        row.append(black_tie)
                rows.append(row)
        else:
            # Mode 1: Kling Klang Studio Oscilloscope (Green CRT trace)
            crt_bg = (5, 20, 8)
            beam_y = [
                min(3, max(0, int(1.5 + math.sin(f * 0.5 + x * 1.4) * 1.2)))
                for x in range(MATRIX_W)
            ]
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if y == beam_y[x]:
                        row.append((120, 255, 140)) # Bright phosphor trace
                    elif abs(y - beam_y[x]) == 1:
                        row.append((20, 90, 35))   # Phosphor halo
                    else:
                        row.append(crt_bg)
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["kraftwerk_model"] = kraftwerk_model_frames


# ---------------------------------------------------------------------------
# 14. kraftwerk_roboter.cpp — Die Roboter Head & 5-Band Vocoder Spectrum
# ---------------------------------------------------------------------------
def kraftwerk_roboter_frames():
    robot_head_bmp = [
        0, 1, 1, 1, 0,
        1, 0, 1, 0, 1,
        1, 1, 1, 1, 1,
        0, 1, 1, 1, 0
    ]
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Robot Head (Communist Red Casing & Glowing Cyan Visor Eyes)
            red_casing = (200, 15, 15)
            metal_dark = (20, 20, 25)
            eye_pulse = int(180 + math.sin(f * 0.7) * 75)
            cyan_eyes = (0, eye_pulse, eye_pulse)
            mouth_led = (240, 240, 255) if (f & 2) else (40, 40, 40)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    idx = y * MATRIX_W + x
                    is_part = robot_head_bmp[idx]
                    if not is_part:
                        row.append(metal_dark)
                    elif y == 1 and (x == 1 or x == 3):
                        row.append(cyan_eyes)
                    elif y == 3:
                        row.append(mouth_led)
                    else:
                        row.append(red_casing)
                rows.append(row)
        else:
            # Mode 1: 5-Band Vocoder Spectrum Analyzer
            phase = (f - 24) * 0.5
            heights = [
                min(4, max(1, int(2.5 + math.sin(phase + i * 1.2) * 1.8)))
                for i in range(5)
            ]
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    bar_h = heights[x]
                    level = 3 - y # 0 is bottom, 3 is top
                    if level < bar_h:
                        if level == 3:
                            row.append((255, 30, 30))   # Red peak
                        elif level == 2:
                            row.append((255, 200, 0))  # Amber mid
                        else:
                            row.append((0, 230, 60))    # Green base
                    else:
                        row.append((5, 15, 8))         # Dark off-LED
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["kraftwerk_roboter"] = kraftwerk_roboter_frames


# ---------------------------------------------------------------------------
# 15. atari_punk_console.cpp — Stepped Tone Staircase & CRT Scope
# ---------------------------------------------------------------------------
def atari_punk_console_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Stepped Tone Harmonic Staircase (Divisors 1 to 5)
            step = (f // 5) % 5
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    level = 3 - y # 0 bottom, 3 top
                    if x <= step and level <= x:
                        col = (255, 30, 0) if x == 0 else (
                              (255, 120, 0) if x == 1 else (
                              (255, 220, 0) if x == 2 else (
                              (0, 255, 80) if x == 3 else (0, 180, 255))))
                        row.append(col)
                    else:
                        row.append((10, 10, 15))
                rows.append(row)
        else:
            # Mode 1: Phosphor Amber CRT Oscilloscope
            beam_x = (f - 24) % MATRIX_W
            pulse_h = int(1.5 + math.sin((f - 24) * 0.4) * 1.5)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x == beam_x:
                        row.append((255, 235, 120)) # Electron beam
                    elif y >= (3 - pulse_h):
                        row.append((170, 65, 0))    # Phosphor glow
                    else:
                        row.append((12, 6, 0))
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["atari_punk_console"] = atari_punk_console_frames


# ---------------------------------------------------------------------------
# 16. benjolin_noise.cpp — The Rungler Shift Register & Cellular Automaton
# ---------------------------------------------------------------------------
def benjolin_noise_frames():
    frames = []
    total_frames = 48
    sr = 0xAC
    ca = [0, 0, 1, 0, 0]
    ca_history = [[0]*5 for _ in range(4)]

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: The Rungler Shift Register & 3-bit DAC
            data_bit = 1 if ((f * 7) & 8) else 0
            sr = ((sr << 1) | data_bit) & 0xFF
            dac = ((sr & 1) << 2) | (sr & 2) | ((sr & 4) >> 2)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if y < 2:
                        # Top 2 rows: 8 bits of shift register
                        idx = y * 4 + (x if x < 4 else 3)
                        bit = (sr >> (7 - (idx % 8))) & 1
                        row.append((0, 255, 200) if bit else (5, 20, 15))
                    else:
                        # Bottom 2 rows: 3-bit DAC stepped bar
                        bar_pos = x + (0 if y == 2 else 5)
                        row.append((255, 120, 0) if bar_pos <= (dac + 1) else (15, 8, 0))
                rows.append(row)
        else:
            # Mode 1: Rule 30 Cellular Automaton
            next_row = []
            for x in range(5):
                l = ca[x - 1] if x > 0 else ca[4]
                m = ca[x]
                r = ca[x + 1] if x < 4 else ca[0]
                next_row.append(l ^ (m | r))
            ca = next_row
            ca_history.pop()
            ca_history.insert(0, next_row)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    is_on = ca_history[y][x]
                    row.append((255, 0, 110) if is_on else (12, 0, 22))
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["benjolin_noise"] = benjolin_noise_frames


# ---------------------------------------------------------------------------
# 17. dub_siren_echo.cpp — Sound-System VU Meter & Rastafari Waves
# ---------------------------------------------------------------------------
def dub_siren_echo_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Sound System Tri-Color VU Meter (Green, Gold, Red)
            bars = [
                min(4, max(1, int(2.5 + math.sin(f * 0.6 + x * 1.1) * 1.8)))
                for x in range(MATRIX_W)
            ]
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    level = 3 - y # 0 bottom, 3 top
                    if level < bars[x]:
                        col = (255, 0, 0) if level == 3 else (
                              (255, 200, 0) if level == 2 else (0, 255, 40))
                        row.append(col)
                    else:
                        row.append((5, 12, 5))
                rows.append(row)
        else:
            # Mode 1: Rastafari Waves (Red, Gold, Green flowing bands)
            for y in range(MATRIX_H):
                rasta = (255, 10, 10) if y == 0 else (
                        (255, 210, 0) if y == 1 else (0, 230, 30))
                row = []
                for x in range(MATRIX_W):
                    wave = (x + (f // 2)) & 3
                    row.append((255, 255, 255) if wave == 0 else rasta)
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["dub_siren_echo"] = dub_siren_echo_frames


# ---------------------------------------------------------------------------
# 18. industrial_cyberdrone.cpp — Nuclear Trefoil & Geiger Particle Sparks
# ---------------------------------------------------------------------------
def industrial_cyberdrone_frames():
    nuke_bmp = [
        1, 0, 1, 0, 1,
        0, 1, 1, 1, 0,
        1, 1, 1, 1, 1,
        0, 1, 0, 1, 0
    ]
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Nuclear Trefoil (Pulsing yellow/gold radiation alert)
            pulse = int(190 + math.sin(f * 0.5) * 60)
            nuke_yellow = (pulse, int(pulse * 0.85), 0)
            dark_core = (15, 8, 0)

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    idx = y * MATRIX_W + x
                    is_nuke = nuke_bmp[idx]
                    row.append(nuke_yellow if is_nuke else dark_core)
                rows.append(row)
        else:
            # Mode 1: Geiger Ionizing Particle Strikes
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    p = ((f * 37 + x * 41 + y * 67) ^ (f >> 2)) & 255
                    if p > 220:
                        row.append((255, 230, 110)) # White-hot ionization strike
                    elif p > 165:
                        row.append((0, 240, 50))    # Radiation green
                    else:
                        row.append((0, 15, 6))      # Dark background
                rows.append(row)

RENDERERS["industrial_cyberdrone"] = industrial_cyberdrone_frames


# ---------------------------------------------------------------------------
# 19. soma_lyra.cpp — 8-Voice Organismic Node Matrix & Hyper LFO Orbit
# ---------------------------------------------------------------------------
def soma_lyra_frames():
    # 8 voice nodes on 5x4 matrix:
    # Pair 1: (0,0), (1,0)  Pair 2: (3,0), (4,0)
    # Pair 3: (0,3), (1,3)  Pair 4: (3,3), (4,3)
    # Center: interconnect lines & Hyper LFO core
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: 8-Voice Organismic Nodes with FM cross-coupling pulses
            pulse_a = int(180 + math.sin(f * 0.45) * 75)
            pulse_b = int(180 + math.cos(f * 0.55) * 75)
            amber = (255, 120 + pulse_a // 3, 20)
            cyan = (20, pulse_b, 255)
            core = (int(140 + math.sin(f * 0.9) * 90), 0, int(200 + math.cos(f * 0.9) * 55))

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    # Check if voice node
                    is_p1 = (y == 0 and x in (0, 1))
                    is_p2 = (y == 0 and x in (3, 4))
                    is_p3 = (y == 3 and x in (0, 1))
                    is_p4 = (y == 3 and x in (3, 4))

                    if is_p1 or is_p3:
                        row.append(amber)
                    elif is_p2 or is_p4:
                        row.append(cyan)
                    elif x == 2 and (y in (1, 2)):
                        # Central Hyper LFO nexus
                        row.append(core)
                    else:
                        # Interconnection energy trail
                        glow = int(25 + math.sin(f * 0.4 + (x + y)) * 20)
                        row.append((glow, glow // 2, glow * 2))
                rows.append(row)
        else:
            # Mode 1: Hyper LFO Cosmic Orbit (Lissajous curves)
            center_x = 2.0 + math.sin(f * 0.35) * 1.5
            center_y = 1.5 + math.cos(f * 0.5) * 1.2

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    dist = math.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
                    intensity = max(0.0, 1.0 - (dist / 2.2))
                    r = int(intensity * 255)
                    g = int(intensity * 60 + math.sin(f * 0.4) * 40)
                    b = int(intensity * 230)
                    row.append((r, max(0, g), b))
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["soma_lyra"] = soma_lyra_frames


# ---------------------------------------------------------------------------
# 20. bytebeat_explorer.cpp — Bit-Waterfall & CRT Scope
# ---------------------------------------------------------------------------
def bytebeat_explorer_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Bit Waterfall Matrix (cyan data lanes with orange center marker)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    bit_on = (((f * 7 + x * 3 + y * 5) ^ (f >> 1)) & 3) == 0
                    if x == 2:
                        col = (255, 120, 0) if bit_on else (25, 8, 0)
                    else:
                        col = (0, 220, 255) if bit_on else (5, 12, 25)
                    row.append(col)
                rows.append(row)
        else:
            # Mode 1: Sierpinski Fractal Matrix
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    u = (x + (f >> 1)) & 15
                    v = (y + (f >> 2)) & 15
                    fractal = ((u & v) == 0)
                    col = (190, 0, 255) if fractal else (10, 15, 35)
                    row.append(col)
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["bytebeat_explorer"] = bytebeat_explorer_frames


# ---------------------------------------------------------------------------
# 21. bytebeat_techno.cpp — 808 Sub Kick Flare & Acid Digital Rain
# ---------------------------------------------------------------------------
def bytebeat_techno_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: 808 Kick Flare (Center column explodes with thermal orange flare)
            kick_beat = (f % 6 == 0)
            kick_flare = 255 if kick_beat else max(30, 255 - ((f % 6) * 45))

            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x == 2:
                        row.append((kick_flare, kick_flare // 2, 0))
                    else:
                        hat = (((f * 5 + x * 7) ^ y) & 3) == 0
                        row.append((0, 240, 255) if hat else (5, 10, 20))
                rows.append(row)
        else:
            # Mode 1: Acid Digital Rain (Matrix Phosphor Green trails)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    drop = ((f * 2) + x * 7 + y * 11) & 15
                    if drop == 0:
                        row.append((220, 255, 220)) # Leading raindrop head
                    elif drop < 4:
                        row.append((0, 240, 50))    # Phosphor green trail
                    else:
                        row.append((0, 15, 5))      # Dark background
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["bytebeat_techno"] = bytebeat_techno_frames


# ---------------------------------------------------------------------------
# 22. bytebeat_ambient.cpp — Aurora Borealis & Cosmic Starlight
# ---------------------------------------------------------------------------
def bytebeat_ambient_frames():
    frames = []
    total_frames = 48

    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Aurora Borealis (Breathing waves of Cyan, Emerald, Violet)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    wave = int(128 + math.sin(f * 0.25 + x * 0.8 + y * 1.2) * 120)
                    g = wave
                    b = 255 - (wave // 2)
                    r = int(math.sin(f * 0.15 + y) * 40 + 40)
                    row.append((max(0, r), max(0, g), max(0, b)))
                rows.append(row)
        else:
            # Mode 1: Cosmic Starlight & Diamond Nebula
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    star = ((f * 13) ^ (x * 47) ^ (y * 83)) & 15
                    if star == 0:
                        row.append((255, 255, 255)) # Sparkling white star
                    elif star < 3:
                        row.append((120, 190, 255)) # Cyan star
                    elif star < 6:
                        row.append((190, 70, 230))  # Violet nebula
                    else:
                        row.append((5, 5, 22))      # Deep space void
                rows.append(row)

        frames.append(render_frame(rows))

    return frames, 60

RENDERERS["bytebeat_ambient"] = bytebeat_ambient_frames


# ---------------------------------------------------------------------------
# 23. bach_air_g_string.cpp — Baroque Candelabra & Cathedral Stained Glass
# ---------------------------------------------------------------------------
def bach_air_frames():
    frames = []
    total_frames = 48
    for f in range(total_frames):
        rows = []
        if f < 24:
            # Mode 0: Golden Baroque Candelabra & Candlelight flicker
            flicker = (f * 7) & 31
            c_gold = (255 - flicker, 175 - (flicker // 2), 10)
            c_candle = (180, 70, 5)
            c_bg = (12, 4, 1)
            flame_h = int(1 + (math.sin(f * 0.4) + 1.0) * 1.0)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x in (1, 2, 3) and y >= 2:
                        row.append(c_candle)
                    elif x == 2 and y == max(0, 3 - flame_h):
                        row.append(c_gold)
                    elif x == 1 and y == (1 if (f & 2) else 0):
                        row.append(c_gold)
                    elif x == 3 and y == (0 if (f & 4) else 1):
                        row.append(c_gold)
                    else:
                        row.append(c_bg)
                rows.append(row)
        else:
            # Mode 1: Cathedral Stained Glass Rose Window
            palette = [
                (0, 40, 220),   # Sapphire
                (210, 10, 30),  # Ruby
                (10, 190, 60),  # Emerald
                (170, 20, 200), # Amethyst
            ]
            pulse = 0.7 + 0.3 * math.sin(f * 0.5)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    base = palette[(x + y + (f >> 2)) & 3]
                    row.append(tuple(int(c * pulse) for c in base))
                rows.append(row)
        frames.append(render_frame(rows))
    return frames, 60

RENDERERS["bach_air_g_string"] = bach_air_frames


# ---------------------------------------------------------------------------
# 24. schaltkreiskontrolle.cpp — Speech Amplitude Flash & Cyber Indicator
# ---------------------------------------------------------------------------
def schaltkreiskontrolle_frames():
    frames = []
    total_frames = 48
    for f in range(total_frames):
        rows = []
        if f < 30:
            # Speaking phase: Red amplitude flash
            amp = int(abs(math.sin(f * 0.45) * math.cos(f * 0.9)) * 255)
            ind_y = (f // 8) % 4
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x == 4:
                        row.append((255, 255, 255) if y == ind_y else (25, 0, 0))
                    else:
                        row.append((amp, 0, 0))
                rows.append(row)
        else:
            # Idle phase: Breathing RGB glow
            val = int(abs(math.sin(f * 0.2)) * 110 + 15)
            r = (200 * val) >> 7
            g = (120 * val) >> 7
            b = (240 * val) >> 7
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x == 4:
                        row.append((255, 255, 255) if y == 2 else (20, 0, 0))
                    else:
                        row.append((r, g, b))
                rows.append(row)
        frames.append(render_frame(rows))
    return frames, 60

RENDERERS["schaltkreiskontrolle"] = schaltkreiskontrolle_frames


# ---------------------------------------------------------------------------
# 22. theremin.cpp — Léon Theremin 1920 Continuous Space Synth
# ---------------------------------------------------------------------------
def theremin_frames():
    total_frames = 60
    frames = []
    for f in range(total_frames):
        rows = []
        phase_offset = f * 0.4
        pitch_pos = int((math.sin(f * 0.1) * 0.5 + 0.5) * 4.9)
        vol_height = int((math.cos(f * 0.15) * 0.5 + 0.5) * 3.9)
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                wave = math.sin(x * 1.5 + phase_offset)
                beam_y = int(1.5 - wave * 1.2)
                if y == beam_y:
                    row.append((40, 255, 60))
                elif abs(y - beam_y) == 1:
                    row.append((10, 80, 20))
                elif x == pitch_pos and y == (3 - vol_height):
                    row.append((255, 255, 255))
                else:
                    glow = int((math.sin(x * 0.8 + y * 0.9 + f * 0.2) * 0.5 + 0.5) * 35)
                    row.append((glow // 2, glow, glow * 2))
            rows.append(row)
        frames.append(render_frame(rows))
    return frames, 50

RENDERERS["theremin"] = theremin_frames


# ---------------------------------------------------------------------------
# 23. spaghetti.cpp — Spaghetti! The Italian Talking Chip
# ---------------------------------------------------------------------------
def spaghetti_frames():
    total_frames = 50
    frames = []
    for f in range(total_frames):
        rows = []
        if f < 25:
            # Speaking phase: Italian Tricolore amplitude burst
            amp = int(abs(math.sin(f * 0.5) * math.cos(f * 0.8)) * 255)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x == 4 and y == 1:
                        row.append((255, 255, 255)) # Pot indicator
                    elif x < 2:
                        row.append((0, amp, amp >> 3)) # Basil Green
                    elif x == 2:
                        row.append((amp, amp, amp))     # Mozzarella White
                    else:
                        row.append((amp, 0, 0))         # Pomodoro Red
                rows.append(row)
        else:
            # Idle phase: Breathing Italian flag glow
            val = int(abs(math.sin(f * 0.25)) * 120 + 20)
            for y in range(MATRIX_H):
                row = []
                for x in range(MATRIX_W):
                    if x == 4 and y == 1:
                        row.append((255, 255, 255))
                    elif x < 2:
                        row.append((0, val, 0))
                    elif x == 2:
                        row.append((val, val, val))
                    else:
                        row.append((val, 0, 0))
                rows.append(row)
        frames.append(render_frame(rows))
    return frames, 60

# ---------------------------------------------------------------------------
# 24. interface_test_akb20 — ABK-20 Hardware & Interface Diagnostic Test
# ---------------------------------------------------------------------------
def interface_test_akb20_frames():
    def wheel_rgb(pos, b):
        pos = (255 - pos) & 255
        if pos < 85:
            r = 255 - pos * 3
            g = 0
            bl = pos * 3
        elif pos < 170:
            pos -= 85
            r = 0
            g = pos * 3
            bl = 255 - pos * 3
        else:
            pos -= 170
            r = pos * 3
            g = 255 - pos * 3
            bl = 0
        r = (r * b) // 255
        g = (g * b) // 255
        bl = (bl * b) // 255
        return (r, g, bl)

    total_frames = 60
    frames = []

    for f in range(total_frames):
        rows = []
        if f < 30:
            # Phase 1: Dual Level Meters + Dedicated Indicator Display
            left_bars = int(abs(math.sin(f * 0.18)) * 3.99) + 1  # 1 to 4
            right_bars = int(abs(math.cos(f * 0.16)) * 3.99) + 1 # 1 to 4
            toggle_up = (f < 15)
            toggle_down = (f >= 15)
            btn3 = (10 <= f <= 14) or (25 <= f <= 29)

            pulse = int(abs(math.sin(f * 0.3)) * 15 + 15)

            for y in range(MATRIX_H):
                row = []
                row_from_bot = 3 - y
                for x in range(MATRIX_W):
                    if x in (0, 1):
                        # Left Pot: Deep Green to Electric Cyan
                        if row_from_bot < left_bars:
                            row.append((0, 150 + row_from_bot * 25, row_from_bot * 65))
                        else:
                            row.append((0, 0, 0))
                    elif x in (3, 4):
                        # Right Fader: Warm Amber to Fiery Red
                        if row_from_bot < right_bars:
                            row.append((160 + row_from_bot * 25, (3 - row_from_bot) * 35, 0))
                        else:
                            row.append((0, 0, 0))
                    else:
                        # Center Column
                        if y == 0:
                            row.append((0, 255, 255) if toggle_up else (0, 18, 18))
                        elif y == 3:
                            row.append((255, 200, 0) if toggle_down else (22, 16, 0))
                        else:
                            if btn3:
                                row.append((255, 255, 255) if y == 1 else (255, 0, 180))
                            else:
                                row.append((pulse, 0, pulse))
                rows.append(row)
        else:
            # Phase 2: Toggle DOWN + Button 3 Pressed -> Full Matrix Beat-Synced Rainbow Wash
            beat_phase = (f - 30) % 8
            beat_decay = math.exp(-beat_phase * 0.45)
            beat_bright = int(45 + 210 * beat_decay)
            hue = int(((f - 30) / 30.0) * 255)
            wash_color = wheel_rgb(hue, beat_bright)

            for y in range(MATRIX_H):
                rows.append([wash_color] * MATRIX_W)

        frames.append(render_frame(rows))

    return frames, 50

RENDERERS["interface_test_akb20"] = interface_test_akb20_frames


# ---------------------------------------------------------------------------
# 25. acid_akb20 — Acid ABK-20 Synthesizer (TB-303 + Neon Acid Lime Strobe)
# ---------------------------------------------------------------------------
def acid_akb20_frames():
    drum_seq = [1, 0, 2, 0, 1, 0, 2, 0, 1, 2, 2, 1, 1, 0, 2, 2] # 1=Kick, 2=Hat, 0=Rest
    accents  = [1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1]
    frames = []

    total_steps = 24 # 48 frames (2 frames per step)
    for s in range(total_steps):
        step = s % 16
        pattern = 0 if s < 16 else 1
        indicator_row = 3 - pattern # Pattern 0 at bottom (y=3) to Pattern 3 at top (y=0)
        btn3 = (8 <= s < 16) # Button 3 active on steps 8-15

        d = drum_seq[step]
        acc = accents[step]
        bass_vis = 140 if acc else 70
        drum_vis = 130 if d == 1 else (60 if d == 2 else 0)

        # Hit frame
        rows = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == indicator_row else (25, 0, 0))
                else:
                    if btn3:
                        # Neon Acid Lime Flash on Button 3!
                        row.append((bass_vis, 255, bass_vis >> 1))
                    else:
                        r = min(255, bass_vis + drum_vis)
                        g = drum_vis
                        b = drum_vis
                        row.append((r, g, b))
            rows.append(row)
        frames.append(render_frame(rows))

        # Decay frame
        bass_decay = bass_vis // 2
        drum_decay = drum_vis // 4
        rows_decay = []
        for y in range(MATRIX_H):
            row = []
            for x in range(MATRIX_W):
                if x == 4:
                    row.append((255, 255, 255) if y == indicator_row else (25, 0, 0))
                else:
                    if btn3:
                        row.append((bass_decay, 180, bass_decay >> 1))
                    else:
                        r = min(255, bass_decay + drum_decay)
                        g = drum_decay
                        b = drum_decay
                        row.append((r, g, b))
            rows_decay.append(row)
        frames.append(render_frame(rows_decay))

    return frames, 55

RENDERERS["acid_akb20"] = acid_akb20_frames


if __name__ == "__main__":
    main()






