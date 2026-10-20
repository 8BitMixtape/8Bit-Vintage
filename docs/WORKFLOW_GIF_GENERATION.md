# 5x4 Matrix Animated GIF Preview Workflow

This document details the standardized method for creating animated GIF previews of the 20-LED ($5\times 4$) NeoPixel matrix for all sound applications on the **8Bit Mixtape** and web library (`docs/index.html`).

---

## 🎨 Overview & Specifications

Every application listed in `docs/apps.json` features an animated LED matrix preview displayed in the web catalog at `docs/gifs/<app_id>.gif`.

* **Matrix Dimensions**: $5\text{ columns} \times 4\text{ rows}$ (20 WS2812B RGB LEDs).
* **Rendered Cell Size**: $30\text{ px} \text{ wide} \times 24\text{ px} \text{ high}$ per LED.
* **Cell Border**: $1\text{ px}$ dark border (`RGB(10, 10, 10)`) simulating physical PCB solder masks and LED package separation.
* **Overall GIF Resolution**: **$150\text{ px} \times 96\text{ px}$**.
* **Framerate & Duration**: Typically 30 to 60 frames @ 50 ms delay ($20\text{ FPS}$), looping seamlessly over 1.5 to 3.0 seconds.
* **Renderer Engine**: Python Pillow (`PIL.Image`).

---

## 🛠️ Step-by-Step Implementation Guide

### 1. Add the Frame Generator in `scripts/gen_previews.py`

Open `scripts/gen_previews.py` and write a generator function named `<app_id>_frames()`:

```python
def my_synth_frames():
    total_frames = 60
    frames = []

    for f in range(total_frames):
        rows = []
        # Calculate dynamic animation parameters for frame f
        phase = f * 0.25

        for y in range(MATRIX_H):      # y: 0 .. 3
            row = []
            for x in range(MATRIX_W):  # x: 0 .. 4
                # Example: Calculate color based on hardware visualizer logic
                # (e.g. waveform, VU meter, plasma wave, frequency spectrum)
                r, g, b = compute_pixel_color(x, y, phase)
                row.append((r, g, b))
            rows.append(row)

        # render_frame packs the 4 rows of 5 RGB tuples into a 150x96 Image
        frames.append(render_frame(rows))

    # Return list of Image frames and frame delay in milliseconds (e.g. 50ms = 20 FPS)
    return frames, 50
```

---

### 2. Register the Renderer in `scripts/gen_previews.py`

At the bottom of `scripts/gen_previews.py`, map the generator to your application ID:

```python
RENDERERS["my_synth"] = my_synth_frames
```

---

### 3. Ensure Registration in `docs/apps.json`

The CLI script loads the list of valid application IDs directly from `docs/apps.json`:

```json
{
  "id": "my_synth",
  "source": "my_synth.cpp",
  "title": "My Synth Title",
  "tagline": "Short descriptive tagline",
  "description": "Full application summary...",
  "tags": ["synth", "8bit"],
  "manual": "MY_SYNTH_MANUAL.md",
  "flash_bytes": 5120,
  "wav_bytes": 1120000
}
```

---

### 4. Execute the Generator

Run the script passing the target ID as an argument:

```bash
python3 scripts/gen_previews.py my_synth
```

*Output*:
```
==> docs/gifs/my_synth.gif (60 frames, 290 KB)
```

---

## 📺 The Vintage "Analog Noise" Texture Technique

When previewing vintage, classical, or retro sci-fi synthesizers (such as *Léon Theremin*, *Bach Air on the G String*, or *Schaltkreiskontrolle*), you can achieve an authentic CRT phosphor glow and vacuum-tube radio-static grain:

### How It Works:
Instead of restricting colors to flat, saturated RGB blocks (e.g., solid `(0, 255, 0)`), use **continuous mathematical gradients**:

1. **Continuous Oscilloscope Trace + Glow Persistence**:
   ```python
   wave = math.sin(x * 1.5 + phase_offset)
   beam_y = int(1.5 - wave * 1.2)
   if y == beam_y:
       row.append((40, 255, 60))        # Bright phosphor electron beam
   elif abs(y - beam_y) == 1:
       row.append((10, 80, 20))         # CRT phosphor persistence halo
   ```

2. **Floating Ambient Plasma Glow**:
   ```python
   glow = int((math.sin(x * 0.8 + y * 0.9 + f * 0.2) * 0.5 + 0.5) * 35)
   row.append((glow // 2, glow, glow * 2))  # Sub-threshold cosmic EMF field
   ```

3. **Palette Dithering Physics**:
   When Pillow exports to the standard 256-color GIF specification, it runs **Floyd-Steinberg error-diffusion dithering**. Because the gradient values fall between discrete palette entries, intermediate pixels are subtly randomized across frames. This produces a warm, organic visual grain reminiscent of 1920s vacuum tubes and analogue radar screens!

---

## 🎵 Companion: Generating the Audio Preview MP3

Every application should also have an accompanying synthesized audio preview in `docs/audio/<app_id>.mp3`:

1. In `scripts/gen_audio_previews.py`, define:
   ```python
   def synth_my_synth():
       sample_rate = 10000
       duration_sec = 6.0
       samples = bytearray(int(sample_rate * duration_sec))
       # Synthesize audio matching the firmware algorithm...
       return samples, sample_rate
   ```
2. Add `("my_synth", synth_my_synth)` to `synths` in `main()`.
3. Generate the MP3 sample:
   ```bash
   python3 -c "from scripts.gen_audio_previews import synth_my_synth, write_wav_to_mp3, AUDIO_DIR; s, r = synth_my_synth(); write_wav_to_mp3(s, r, AUDIO_DIR / 'my_synth.mp3')"
   ```

