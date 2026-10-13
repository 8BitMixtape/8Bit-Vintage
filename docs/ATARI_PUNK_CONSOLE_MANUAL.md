# ATARI PUNK CONSOLE DELUXE (APC) — STEPPED TONE & SUB-OCTAVE NOISE SYNTH

An authentic digital recreation and expansion of Forrest M. Mims III's legendary 1980 **"Stepped Tone Generator"** (popularly known as the **Atari Punk Console**) for the **8Bit Mixtape** (ATtiny85).

---

## ⚡ The Stepped Tone Sound Architecture

The original hardware circuit couples two 555 timers in an astable/monostable multivibrator topology:
1. **Timer A (Astable Multivibrator)**: Square wave oscillator that sets the fundamental repetition rate (Pitch).
2. **Timer B (Monostable Multivibrator)**: Fixed pulse-width one-shot timer triggered by Timer A. When the monostable pulse duration exceeds the astable's period, it ignores incoming triggers (true retrigger lockout). This produces the iconic **stepped harmonic octave and fifth staircases** that give the APC its signature retro arcade sound.
3. **Dual Sub-Octave Dividers (/2 and /4)**: Hardware-style flip-flop frequency dividers that add $-1$ and $-2$ octaves of thunderous sub-bass rumble beneath the stepped tone melody.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Astable Frequency (Master Pitch)**:
  - Sweeps continuously from low 30 Hz sub-rumble up to a screaming 3.5 kHz chiptune whistle.
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Monostable Pulse Width (Harmonic Formant)**:
  - Adjusts pulse duration from 0.2 ms to 100 ms, sweeping through stepped harmonic integer divisors ($/1, /2, /3, /4, /5 \dots$).

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Sound Modes**:
    1. **Classic Pure APC (Red/Amber)**: Authentic 555 stepped tone generator.
    2. **Sub-Bass Beast (Cyan)**: APC blended with $/2$ and $/4$ sub-octaves for heavy wall-of-sound bass.
    3. **Ring-Mod Cross-Modulator (Magenta)**: Astable XOR Monostable cross-modulation for metallic industrial ring-mod noise.
    4. **Bitcrush & Overdrive (Green)**: Non-linear wavefolding and decimation distortion.
  - *Matrix flashes mode color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Reactive Visualizer Modes**:
    1. **Harmonic Staircase**: 5-column harmonic ladder showing active divisor steps ($/1$ to $/5$).
    2. **Phosphor CRT Scope**: Glowing amber CRT trace reflecting pulse width and electron beam position.
    3. **Sub-Bass Shockwave**: Expanding cyan/magenta ripple rings pulsing with sub-octave triggers.
    4. **Glitch Matrix Sparks**: Electric purple sparks jumping with oscillator overflows.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Laser Pitch Dive-Bomb & Overdrive Strobe!**
  - Triggers an exponential laser pitch dive-bomb with high-intensity emergency strobe flashing!

---

## 🚀 Building & Uploading

1. Build firmware & generate bootloader WAV:
   ```bash
   pio run
   ```
2. Upload via AudioBoot:
   - Connect your 8Bit Mixtape audio jack to your computer's sound output.
   - Power-cycle the ATtiny85 into AudioBoot bootloader mode (LEDs flashing).
   - Manually run:
     ```bash
     pio run --target upload
     ```
