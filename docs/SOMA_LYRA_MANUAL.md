# SOMA LYRA-8 — ORGANISMIC SYNTHESIZER

An authentic digital tribute to Vladislav Kreimer / SOMA Laboratory's legendary **"Lyra-8"** organismic synthesizer, developed for the **8Bit Mixtape** (ATtiny85).

---

## 🌌 The Organismic Sound Architecture

The Lyra-8 is an instrument without linear boundaries where voices, modulations, and feedback influence each other organically:
1. **8 Interlinked Voice Oscillators (4 Voice Pairs)**:
   - Grouped into 4 harmonic pairs: $(V_0, V_1)$, $(V_2, V_3)$, $(V_4, V_5)$, $(V_6, V_7)$.
   - Each voice pair features bi-directional cross-modulation and inter-voice FM loops.
2. **The Iconic "HYPER LFO"**:
   - Two interconnected Low-Frequency Oscillators (LFO A & LFO B) modulating each other.
   - Non-linear logic combination (stepped XOR/AND) generating living, unpredictable, non-repeating organic modulation sweeps.
3. **Simulated SOMA Dual Delay & "DRIVE" Saturation**:
   - Diode-style non-linear wavefolding overdrive saturation.
   - Lo-fi space delay line simulating the metallic, breathing reverberation of the Lyra-8 delay processor.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Knob (`POTI_LEFT` / PB4 / A2) — Master Tuning & Cluster Pitch**:
  - Sweeps the fundamental pitch of the 8-voice cluster across the full musical spectrum from deep 38 Hz sub-drones up to 700 Hz (harmonics reaching 3.3 kHz).
  - Scaled via `neolib` for full 0–1023 sweep range.
* **Right Slider (`POTI_RIGHT` / PB2 / A1) — Hyper LFO, FM Depth, Space Delay & Drive**:
  - Dynamically morphs the sound in **every mode**:
    - **Slider down (0)**: Pure, serene harmonic chord cluster, clean and dry.
    - **Slider mid**: Lush inter-voice beating, organic Hyper LFO breathing pulses, and warm space delay echoes.
    - **Slider up (1023)**: Aggressive inter-voice FM chaos, screaming delay feedback, and crunchy diode overdrive!

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Lyra Playing Modes**:
    1. **Organismic Drone (Cyan)**: Ethereal beating microtonal chord clusters.
    2. **Hyper LFO FM Chaos (Magenta)**: Living alien modulation sweeps and non-linear FM textures.
    3. **Industrial Screaming Drive (Amber)**: Heavy diode clipping and self-oscillating delay screams.
    4. **Touch Pulsar Arp (Green)**: Stepped rhythmic gating across the 8 voice nodes.
  - *Matrix flashes mode color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Reactive Visualizers**:
    1. **8-Voice Activity Matrix**: Displays the live amplitude and activity of all 8 voices across 4 pairs with center delay glow.
    2. **Hyper LFO Lissajous Orbit**: Real-time phase plot of LFO A vs LFO B organic trajectory.
    3. **Dual Delay Space Echo Rings**: Expanding concentric echo ripples.
    4. **Bio-Electric Touch Field**: Capacitive skin contact illumination field.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Full 8-Voice Resonant Overload!**
  - Cranks FM modulation, drive, and delay feedback to 100% with high-intensity electric blue & magenta lightning strobing!

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
