# BENJOLIN / RUNGLER CHAOTIC NOISE SYNTHESIZER

An authentic digital tribute to Rob Hordijk's legendary Dutch modular masterpiece, the **"Benjolin"** chaotic instrument, developed for the **8Bit Mixtape** (ATtiny85).

---

## ⚡ The Benjolin & "Rungler" Chaotic Circuit Topology

The Benjolin is world-renowned in modular synthesis for its ability to operate on the boundary between order, generative melody, and chaotic noise:
1. **Dual Cross-Modulating Oscillators (VCO1 & VCO2)**:
   - VCO1 functions as master clock and audio/sub-audio oscillator.
   - VCO2 functions as modulation and audio oscillator with non-linear cross-modulation.
2. **The "Rungler" (8-Bit Shift Register + 3-Bit DAC)**:
   - VCO1 clocks the 8-bit shift register.
   - VCO2's comparator pulse feeds data into the shift register.
   - Three taps from the shift register pass into a 3-bit Digital-to-Analog Converter (DAC) to generate a stepped chaotic Control Voltage (CV).
   - This Rungler CV is fed back into VCO1, VCO2, and the resonant filter cutoff, forming an intricate non-linear feedback loop (strange attractor).
3. **Resonant State Variable Filter**:
   - Resonant Low-Pass / Band-Pass filter whose cutoff tracks the Rungler stepped CV.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — VCO1 Frequency / Master Clock**:
  - Sweeps from low sub-audio rhythmic ticking (~2 Hz) up to high audio frequencies (2.5 kHz).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — VCO2 Frequency / Rungler Feedback & Filter**:
  - Tunes VCO2 and sweeps Rungler feedback depth and resonant filter cutoff.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Synthesis Modes**:
    1. **Classic Benjolin Rungler (Cyan)**: Hypnotic repeating melodic patterns and alien chatter.
    2. **Industrial Heavy Drone (Amber)**: Deep sub-octave intermodulation and wavefolded saturation.
    3. **Screaming Filter Chaos (Magenta)**: High-Q band-pass self-oscillating filter sweeps.
    4. **Bit-Slicer Glitch (Green)**: 8-bit digital sample-and-hold decimation slicing.
  - *Matrix flashes mode color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Reactive Visualizer Modes**:
    1. **Rungler 8-Bit Shift Register**: Top 8 LEDs display the live bits shifting; bottom rows show the 3-bit DAC level.
    2. **1D Cellular Automaton**: Rule 30 chaotic pattern evolution perturbed by Rungler state.
    3. **Lissajous Phase Orbit**: Real-time phase plot of VCO1 vs VCO2 chaotic trajectory.
    4. **Chaos Radar Scanner**: Sweeping radar beam with Rungler anomaly blips.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Entropy Injection & Feedback Meltdown!**
  - Injects Galois pseudo-random noise directly into the Rungler data line and cranks resonance to maximum!

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
