# BYTEBEAT EXPLORER — VIZNUT 8-BIT CLASSICS & GLITCH MACHINE

An interactive hardware explorer for the legendary algorithmic "one-liner" bytebeat formulas discovered by **Viznut** (Ville-Matias Heikkilä), **Tejeez**, and the demoscene, running in real-time on the **8Bit Mixtape** (ATtiny85).

---

## 💾 The Philosophy of Bytebeat

Bytebeat synthesizers produce rich, hypnotic musical structures from a single line of C code evaluating an integer time counter `t`. In just a few arithmetic and bitwise operations (`*`, `>>`, `&`, `|`, `^`), entire melodies, rhythmic percussion lines, arpeggios, and fractal polyrhythms emerge naturally from the laws of binary math:

1. **Formula 0 — The 42 Melody**:
   ```c
   (t * (t >> 12 | t >> 8) & 63 & t >> 4)
   ```
   The most famous bytebeat expression in computer history, discovered by Viznut in 2011. Generates a melancholic, evolving 8-bit chiptune melody with changing chord progressions.
2. **Formula 1 — Algorithmic Arpeggiator**:
   ```c
   ((t * 5 & t >> 7) | (t * 3 & t >> 10))
   ```
   Dual-speed interlocking harmonic arpeggio sequences with bouncy pentatonic syncopation.
3. **Formula 2 — Minimal Chiptune Counter**:
   ```c
   (t * (t >> 9 | t >> 13) & 16)
   ```
   Pure raw 1-bit chiptune square-wave pulse sequences with rhythmic frequency modulation.
4. **Formula 3 — Sierpinski Glitch Machine**:
   ```c
   ((t * (t >> 8) * (t >> 12) & 64) + ((t >> 5 & t >> 7) & t >> 3))
   ```
   Chaotic binary fractal recursion yielding polyrhythmic percussion and stuttering chiptune breaks.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Knob (`POTI_LEFT` / PB4 / A2) — Clock Speed / Sample Rate Warp**:
  - Sweeps the bytebeat evaluation clock from slow hypnotic bass (2 kHz) up to blazing overclocked chiptune (24 kHz).
* **Right Slider (`POTI_RIGHT` / PB2 / A1) — Pitch Warp & Bitmask Glitch Shredder**:
  - Dynamically modifies the bit-shift offsets and bitmasks in real-time, warping intervals and injecting alien glitches into the equations.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Bytebeat Formulas**:
    1. **The 42 Melody (Cyan)**
    2. **Algorithmic Arp (Magenta)**
    3. **Chiptune Counter (Amber)**
    4. **Sierpinski Glitch (Green)**
  - *Matrix instantly flashes the mode badge color.*

* **Right Button (Tap)**:
  - **Cycles 4 Reactive Bit-Matrix Visualizers**:
    1. **Bit-Waterfall Matrix**: Displays the live 8-bit binary word cascading across the LEDs.
    2. **CRT Oscilloscope Trace**: Horizontal phosphor electron beam tracking real-time waveform height.
    3. **Logic Flame**: Thermal bitmask heat spectrum.
    4. **Sierpinski Fractal**: Real-time rendering of mathematical Sierpinski triangle bit-logic.
  - *Matrix instantly flashes white on change.*

* **Both Buttons (Hold)**:
  - **Turbo Overclock Warp!**
  - Doubles the clock speed instantly with high-speed neon cyber strobe lighting.

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
