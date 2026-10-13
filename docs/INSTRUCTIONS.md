# AudioBoot PIO Test — Quick Instructions

This guide shows how to generate WAVs for audio bootloading, play them from GitHub, and preview visuals on your laptop via the emulator that reuses the same C/C++ code.


## Prerequisites

- PlatformIO CLI installed (VS Code PlatformIO extension or `pip install platformio`).
- For GIF generation: ImageMagick (`convert`) or Python Pillow (`pip install pillow`).

Project root: this folder (where `platformio.ini` is).


## Generate all visual WAVs

- Run the batch generator:

```
python3 scripts/gen_visual_wavs.py
```

- Outputs: `wav/visual-<id>-<slug>.wav`
- Playlist page (local): `wav/README.md`
- Web player page (HTML): `wav/index.html`

Tip: If you only want a subset, edit the `VISUALS` list at the top of `scripts/gen_visual_wavs.py`.


## Play WAVs in the browser (GitHub)

1. Commit and push `wav/` (and `wav/index.html`).
2. Enable GitHub Pages in repo Settings → Pages → Deploy from a branch → choose your default branch and `/(root)`.
3. Open the published page:

```
https://dusjagr.github.io/AudioBoot-pio-test/wav/index.html
```

This page has playable HTML5 audio for every generated WAV. You can also open `wav/README.md` in the repo web UI (it contains audio tags too), but the HTML page is more reliable across viewers.


## Change which visual runs (VID)

You can switch visuals in a few ways:

- Easiest (for emulator runs):

```
python3 scripts/run_emulator.py --vid 7
```

- Via PlatformIO (native emulator env): edit `platformio.ini`, section `[env:native]`, `build_flags`:

```
-DVISUAL_ID=7
```

- For firmware builds (WAV generation uses per-build define):
  - The batch generator cycles through a list. To build only one, temporarily keep only that `(id, slug)` in `VISUALS` in `scripts/gen_visual_wavs.py`.

See mapping in `src/AetzLampliu_mini.ino` inside `loop()` under the `#if (VISUAL_ID >= 0)` switch.


## Preview visuals on your laptop (emulator + GIF)

This uses the same `include/visuals.h` code and a minimal host stub for `Adafruit_NeoPixel`.

- Build native emulator once:

```
platformio run -e native
```

- Run emulator to write frames, then convert to GIF (two-step):

```
.pio/build/native/program
python3 scripts/gen_visual_gif.py --in emu_out/frames --out emu_out/preview.gif --fps 10
```

- One-command helper (build+run+gif):

```
python3 scripts/run_emulator.py
```

- Override visual and output:

```
python3 scripts/run_emulator.py --vid 18 --frames 200 --fps 12 --out emu_out/visual-18.gif
```

- Control animation length and frames via env vars when running the emulator directly:

```
EMU_RUNTIME_MS=12000 EMU_FRAMES=240 .pio/build/native/program
```

Definitions:
- `EMU_RUNTIME_MS`: how long the visual runs (milliseconds). Default 4000.
- `EMU_FRAMES`: cap of frames saved. Default 120.
- Frames directory: `emu_out/frames/`
- Default GIF output: `emu_out/preview.gif`


## Generate and play a single WAV quickly

- Build normally to hex, then convert to WAV is automatic in this project. For a specific visual, set `-DVISUAL_ID=<n>` in the environment used for the build or use the batch generator script filtered to that ID.

Result goes to `wav/visual-<id>-<slug>.wav`.


## Troubleshooting

- No inline audio controls in README: some viewers (IDE) don’t render `<audio>`. Use `wav/index.html` or the GitHub Pages link.
- `platformio run -e native` says “Nothing to build”: ensure `src/emulator_entry.cpp` exists (it does in this repo) and `[env:native]` is selected.
- Missing GIF: ensure you have ImageMagick (`convert`) or install Pillow: `pip install pillow`.
- Emulator produced few frames: increase `EMU_RUNTIME_MS` and/or `EMU_FRAMES`.
- WAV not generated: ensure tools in `tools/hex2wav/` are present. The build also falls back to the Java JAR if the Linux binary is missing.


## Useful commands recap

- Build all WAVs:
```
python3 scripts/gen_visual_wavs.py
```

- Emulator quick run to GIF (current VISUAL_ID):
```
python3 scripts/run_emulator.py
```

- Emulator manual run + GIF:
```
EMU_RUNTIME_MS=12000 EMU_FRAMES=240 .pio/build/native/program
python3 scripts/gen_visual_gif.py --in emu_out/frames --out emu_out/preview.gif --fps 12
```

- Change visual for emulator permanently:
```
# in platformio.ini → [env:native]
-DVISUAL_ID=7
```

- Open web player for WAVs (GitHub Pages):
```
https://dusjagr.github.io/AudioBoot-pio-test/wav/index.html
```
