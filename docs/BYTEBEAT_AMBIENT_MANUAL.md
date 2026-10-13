# BYTEBEAT AMBIENT & COSMIC FRACTALS

A meditative, ethereal, and cosmic soundscape machine generating evolving ambient drones, celestial arpeggios, and fractal harmonic washes from 4 pure mathematical one-liners, running on the **8Bit Mixtape** (ATtiny85).

---

## 🌌 The Cosmic Mathematics of Space Sound

While bytebeat is often associated with raw chiptune noise, certain harmonic symmetries produce deeply serene, meditative, and cosmic acoustic spaces:

1. **Formula 0 — Cosmic Choir & Celestial Strings**:
   ```c
   ((t >> 6 | t >> 7 | t >> 8) * (t >> 4 & t >> 8))
   ```
   Slow-moving, breathing polyphonic harmonic washes evoking an angelic choir floating in interstellar space.
2. **Formula 1 — Starlight Arpeggios & Harmonic Crystals**:
   ```c
   (((t * 9 & t >> 4) | (t * 5 & t >> 7) | (t * 3 & t >> 10)) - 1)
   ```
   Cascading crystal arpeggios that twinkle and fold over each other like light through an ice prism.
3. **Formula 2 — Deep Space Alien Communications & Pulsar Drone**:
   ```c
   (((t * ((t >> 14 & t >> 8) & 248) & 128) + ((t >> 8 & t >> 13) & (t >> 3))))
   ```
   A deep, rumbling tectonic sub-drone punctuated by mysterious stepped radio transmissions and pulsar clicks.
4. **Formula 3 — Fractal Mathematical Void**:
   ```c
   (((t >> 7) * (t >> 11) ^ ((t >> 5) * (t >> 9))))
   ```
   Hypnotic, ever-shifting mathematical interference patterns and deep spatial phase movements.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Knob (`POTI_LEFT` / PB4 / A2) — Time Dilation & Pitch**:
  - Stretches time across cosmic scales: from ultra-slow, sub-harmonic ambient drones up to delicate, sparkling starlight chimes.
* **Right Slider (`POTI_RIGHT` / PB2 / A1) — Harmonic Shimmer & Overtone Diffusion**:
  - Dynamically infuses harmonic shimmers, alters bitwise overtone dispersion, and adds spatial resonance into the equations.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Cosmic Equations**:
    1. **Cosmic Choir (Cyan)**
    2. **Starlight Arp (Lavender)**
    3. **Alien Pulsar (Pink)**
    4. **Fractal Void (Emerald)**
  - *Matrix instantly flashes the mode badge color.*

* **Right Button (Tap)**:
  - **Cycles 4 Cosmic Visualizers**:
    1. **Aurora Borealis**: Breathing waves of cyan, emerald, and violet drifting across the sky.
    2. **Starlight Plasma**: Twinkling crystalline star field with glowing nebula dust.
    3. **Void Vortex**: Hypnotic concentric event-horizon rings spinning in deep space.
    4. **Celestial Constellation**: Interlinked starlight nodes pulsing with harmonic activity.
  - *Matrix instantly flashes white on change.*

* **Both Buttons (Hold)**:
  - **Supernova Quantum Freeze!**
  - Suspends time progression into an infinite frozen drone with shimmering diamond-white and lavender starlight strobing!

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
