#!/usr/bin/env python3
"""
Render animated GIF previews of the 5x4 LED matrix for each sound app into
docs/gifs/<id>.gif, using the same cell style as the old emulator
(30x24 px cells with a 1 px dark border -> 150x96 GIF).

Each app's visual is a small Python port of the sketch's LED code, registered
in RENDERERS below. Apps without a renderer are skipped (the web page simply
shows no preview for them).

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
# drone_synth.cpp — "Trickster Plasma", cycling through the 4 modes/themes
# ---------------------------------------------------------------------------
DRONE_THEMES = [
    ((30, 15, 0), (255, 220, 60)),   # 0 Triangle  - yellow/brown
    ((40, 0, 0), (255, 50, 0)),      # 1 Fuzz      - deep red/orange
    ((20, 10, 0), (200, 100, 0)),    # 2 Bitcrush  - burnt orange
    ((30, 0, 10), (255, 20, 60)),    # 3 RingMod   - crimson
]


def drone_synth_frames():
    frames_per_mode = 60          # 3 s per mode at 50 ms/frame
    speed, zoom = 1.0, 1.5        # defaults (smoothSpeed / smoothZoom)
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


RENDERERS = {
    "drone_synth": drone_synth_frames,
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


if __name__ == "__main__":
    main()
