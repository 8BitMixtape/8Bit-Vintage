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

if __name__ == "__main__":
    main()
