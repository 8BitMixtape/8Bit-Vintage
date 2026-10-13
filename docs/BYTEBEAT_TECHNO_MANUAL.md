# BYTEBEAT TECHNO & ACID — ALGORITHMIC RHYTHM GENERATOR

An algorithmic techno, acid, and industrial rhythm generator powered by 4 relentless mathematical bytebeat "one-liners", developed for the **8Bit Mixtape** (ATtiny85).

---

## ⚡ The Algorithmic Beat Engine

Unlike traditional step sequencers that cycle through fixed memory arrays, bytebeat techno machines generate infinite, ever-evolving rhythmic grooves, kicks, hi-hats, and acid 303 basslines purely through integer bit-twiddling:

1. **Formula 0 — Driving Cyber Techno**:
   ```c
   ((t >> 6 | t | t >> (t >> 16)) * 10 + ((t >> 11) & 7))
   ```
   A relentless underground Berlin warehouse groove with four-on-the-floor sub kicks, off-beat hi-hats, and shifting percussion bars.
2. **Formula 1 — Heavy Industrial Stomp**:
   ```c
   (((t >> 7 | t | t >> 6) * 10 + 4 * ((t & (t >> 13)) | (t >> 6))))
   ```
   Massive distorted industrial stomps with syncopated metallic clicks and distorted clatter.
3. **Formula 2 — Modular Acid 303 Groove**:
   ```c
   (t * ((t >> 12 | t >> 8) & (t >> 4) & (t >> 7)))
   ```
   A classic Roland TB-303-style resonant acid line with squelching octave leaps and shifting accent patterns.
4. **Formula 3 — Dystopian Cyberpunk Beat**:
   ```c
   ((t >> 5) | (t << 4) | ((t >> 10) ^ ((t >> 9) - 1) % 11 * t))
   ```
   Aggressive dystopian synthwave groove with hard punchy snares and driving bassline.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Knob (`POTI_LEFT` / PB4 / A2) — Master Tempo & Clock Speed**:
  - Sets the groove clock speed from slow sludgy 70 BPM industrial beats up to 180 BPM gabber and hardcore techno.
* **Right Slider (`POTI_RIGHT` / PB2 / A1) — Overdrive, Distortion Fold & Glitch Shredder**:
  - Dynamically injects bitwise XOR distortion, folds the dynamic range, and adds screaming overdrive crunch to the drum transients.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Techno/Acid One-Liners**:
    1. **Cyber Techno (Red)**
    2. **Industrial Stomp (Amber)**
    3. **Modular Acid (Green)**
    4. **Dystopian Beat (Cyan)**
  - *Matrix instantly flashes the mode badge color.*

* **Right Button (Tap)**:
  - **Cycles 4 Rhythm Visualizers**:
    1. **808 Sub Kick Flare**: Center tower flares with thermal brightness on low-end kick hits; outer bars track hi-hats.
    2. **Industrial Techno Strobe**: Fast sweep barcode scanner with reactive red strobe flashes.
    3. **Acid Matrix Digital Rain**: Phosphor green cascading raindrops flowing downward.
    4. **Cyber Radar Scanner**: Rotating concentric sonar rings pulsed by beat subdivisions.
  - *Matrix instantly flashes white on change.*

* **Both Buttons (Hold)**:
  - **Stutter Roll Overdrive!**
  - Quads the clock speed into a machine-gun 1/64th snare roll with industrial hazard warning strobe!

---

## 🚀 Building & Uploading

1. Build firmware & generate bootloader WAV:
   ```bash
   pio run
   ```
2. Upload via AudioBoot:
   - Power-cycle your 8Bit Mixtape into bootloader mode.
   - Manually run:
     ```bash
     platformio run --target upload
     ```
