# LÉON THEREMIN 1920 — CONTINUOUS SPACE SYNTH & ETHERPHONE

A high-resolution, expressive recreation of Léon Theremin's revolutionary 1920 **"Etherphone" (Theremin)** for the **8Bit Mixtape** (ATtiny85), bringing continuous microtonal gliding, organic vocal vibrato, heterodyne space oscillation, and interactive electromagnetic field (EMF) visualizers to 8-bit hardware.

---

## ⚡ Sound & Physics Architecture

Invented in Russia in 1920 by physicist Léon Theremin, the Theremin was the first electronic musical instrument played without physical contact. The instrument operates on the **heterodyne principle**, where two radio-frequency oscillators interfere to produce an audible difference frequency beat.

This digital engine implements:
1. **Continuous Microtonal Pitch Antenna (Left Pot / PB4)**:
   - Smooth portamento glide with low-pass trajectory filtering, completely eliminating ADC zipper noise.
   - Broad continuous frequency range from deep cello bass ($55\text{ Hz}$, A1) to ethereal soprano ($2100\text{ Hz}$, C7).
2. **Dynamic Volume Antenna (Right Pot / PB2)**:
   - Real-time expressive amplitude control with quadratic acoustic curve.
   - From total silence/muting (enabling rapid staccato tapping and articulation) up to full soaring forte with warm tube saturation.
3. **Organic 5.5 Hz Hand-Tremor Vibrato**:
   - Subtle $1.5\%$ pitch modulation modeled after the natural tremor of a virtuoso thereminist's hand (such as Clara Rockmore).
4. **Heterodyne Dual-Oscillator Synthesis**:
   - Primary and secondary phase accumulators simulating radio frequency interference beat-frequencies and vocal vowel formants.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers (The Virtual Antennas)
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Pitch Antenna Hand**:
  - Distance to antenna: smooth continuous glissando from deep cello rumble to piercing whistle.
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Volume Antenna Hand**:
  - Height above volume loop: silence ($0$) to full amplitude ($255$), shaping phrasing and staccato notes.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Pitch & Scale Modes**:
    1. **Pure Continuous Glissando**: Infinite microtonal gliding etherphone (authentic 1920s behavior).
    2. **Vocal Formant Singer**: Dynamic resonant vowel formants (*"Ooo" – "Aaa" – "Eee"*) that shift with pitch.
    3. **Sci-Fi B-Movie Flying Saucer**: Heterodyne dual-oscillator ring-mod wail with eerie alien beat frequencies.
    4. **Quantized Harmonic Minor Scale**: 17 discrete musical steps spanning 2+ octaves for melodic playing without pitch drift.
  - *Randomly jumps to a new visualizer theme on each switch!*

* **Right Button (Tap)**:
  - **Cycles 4 Instrument Timbres**:
    1. **Clara Rockmore Pure Sine**: Round, warm vintage vacuum tube tone.
    2. **Heterodyne Space UFO**: Ring-modulated carrier beat interference.
    3. **Ghost Cello / Singing Saw**: Rich odd-harmonic reed/string timbre.
    4. **Celestial Glass Harmonica**: Crystalline overtone shimmer.

* **Both Buttons (Hold)**:
  - **Antenna Ground Touch & Space Ray Strobe!**
  - Instantly dive-bombs pitch into sub-bass while flashing full-matrix violet/white electromagnetic lightning.

---

## 🌈 5x4 NeoPixel EMF Visualizers

The matrix renders the invisible electromagnetic field between player and antennas:
1. **EMF Plasma Field**: Glowing cyan/violet pitch aura on the left and amber/emerald volume aura on the right, pulsing with audio ripples.
2. **Phosphor CRT Oscilloscope**: Authentic 1920s green cathode-ray tube trace vibrating with the exact theremin waveform.
3. **Chromatic Spectral Ribbon**: Rainbow light spectrum tracking pitch frequency vertically and horizontally.
4. **Sci-Fi UFO Radar**: 1950s radar sweep beam scanning for alien spacecraft signals.

---

## 🚀 Building & Uploading

1. Build firmware & generate bootloader WAV:
   ```bash
   pio run
   ```
2. Upload via AudioBoot:
   - Connect your 8Bit Mixtape audio jack to your computer's sound output.
   - Power-cycle the ATtiny85 into AudioBoot bootloader mode.
   - Manually run:
     ```bash
     pio run --target upload
     ```
   *(Or play `docs/wav/theremin.wav` directly into the tape's audio input).*

