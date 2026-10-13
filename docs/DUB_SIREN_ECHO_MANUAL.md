# JAMAICAN DUB SIREN & LO-FI TAPE ECHO MACHINE

An authentic sound-system dub siren instrument with simulated vintage space tape echo, inspired by King Tubby, Lee "Scratch" Perry, and Jah Shaka, developed for the **8Bit Mixtape** (ATtiny85).

---

## 🔊 Sound Architecture

1. **4 Vintage Sound-System Siren Waveforms**:
   - **Laser Sweep**: Downward frequency ramp with sharp harmonic decay.
   - **UFO Warble**: Fast triangular pitch vibrato creating the sci-fi sound-system wobble.
   - **Hi-Lo Police Klaxon**: Square-wave alternating dual-tone siren.
   - **Machine Gun 1-Shot Dub Bleep**: Staccato gated short bleep bursts.
2. **Vintage Tape Delay Line Simulation**:
   - 256-byte circular delay line downsampled to 1.25 kHz, yielding an authentic vintage delay time of up to **~205 ms**.
   - **Non-Linear Tape Saturation**: Soft-clipping feedback loop replicating warm analog magnetic tape compression.
   - **Infinite Feedback Runaway**: Cranking the right pot into the upper range drives the echo loop into self-oscillation and runaway sound-system resonance.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Siren Modulation Rate & Pitch**:
  - Sweeps base frequency (200 Hz to 2.4 kHz) and LFO modulation speed (0.5 Hz to 18 Hz).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Tape Delay Time & Feedback Regeneration**:
  - Sets echo tap delay length (25 ms to 205 ms) and feedback gain. When turned past 85%, triggers infinite self-oscillating tape runaway!

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Dub Siren Waveforms**:
    1. **Laser Sweep (Red)**
    2. **UFO Warble (Cyan)**
    3. **Hi-Lo Police Klaxon (Gold)**
    4. **Machine Gun Dub Bleep (Green)**
  - *Matrix flashes siren color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Sound-System Visualizers**:
    1. **Sound System Tri-Color VU Meter**: Green, Gold, Red peak level meters bouncing with delay repeats.
    2. **Jamaican Space Radar**: Cyan scanning beam tracing LFO phase.
    3. **Rastafari Waves**: Flowing Red, Gold, Green harmonic bands.
    4. **Echo Rings**: Expanding concentric ripple rings pulsed by delayed audio taps.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Airhorn & Infinite Echo Blast!**
  - Triggers an authentic reggae airhorn blast with 100% delay feedback and strobe lighting!

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
