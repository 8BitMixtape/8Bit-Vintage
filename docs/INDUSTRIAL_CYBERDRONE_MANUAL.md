# INDUSTRIAL CYBER-DRONE — 4-NODE ORGANISMIC NOISE SYNTHESIZER

An organismic harsh noise and ambient cyber-drone synthesizer inspired by the **SOMA Lyra-8**, Throbbing Gristle, and industrial soundscapes, developed for the **8Bit Mixtape** (ATtiny85).

---

## 🌋 Organismic Sound Architecture

Unlike traditional modular synthesizers with separate linear modules, the Industrial Cyber-Drone links 4 non-linear voice nodes into an interconnected organic feedback ring:
1. **4 Cross-Modulating Voice Nodes (Ring Topology)**:
   - Node 1 FM modulates Node 2; Node 2 modulates Node 3; Node 3 modulates Node 4; Node 4 feeds back into Node 1!
   - At subtle modulation settings: yields breathing, complex harmonic chord clusters and ethereal ambient drones.
   - At high modulation settings: explodes into non-linear deterministic chaos, metallic screams, and harsh noise walls.
2. **Granular Noise & Geiger Click Injection**:
   - 16-bit Galois LFSR noise injector introduces random ionizing radiation static and bursts into the intermodulation nodes.
3. **Diode Wavefolding & Screaming Resonant Filter**:
   - Non-linear wavefolding saturation soft-clips and saturates the combined node cluster.
   - 1-pole state variable filter simulation capable of self-oscillating resonant shrieks.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — FM Pitch Cluster & Modulation Spread**:
  - Sweeps base cluster pitch from deep 30 Hz tectonic sub-rumble up to a shrieking 1.6 kHz industrial whine.
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Distortion Saturation & Screaming Filter Resonance**:
  - Controls non-linear wavefolding overdrive and pushes the resonant filter into self-oscillating screaming feedback.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Drone Modes**:
    1. **Organismic Ambient (Cyan)**: Ethereal breathing multi-voice FM pads and sci-fi textures.
    2. **Harsh Industrial (Red)**: Screaming metallic feedback and brutal overdriven walls of noise.
    3. **Geiger Biohazard Glitch (Gold)**: Granular radiation static and clicking chaotic pulses.
    4. **Seismic Sub-Rumble (Magenta)**: Low frequency earthquake drone and deep seismic harmonics.
  - *Matrix flashes drone color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Reactive Visualizers**:
    1. **Nuclear Hazard Trefoil**: Pulsing nuclear yellow & black radiation symbol.
    2. **Geiger Counter Sparks**: Random ionizing radiation strikes across a green phosphor field.
    3. **Cyber Plasma Vortex**: Swirling FM attractor orbit in vivid magenta and white.
    4. **Seismic Fault Line**: Shaking molten magma tectonic fracture line.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Nuclear Meltdown Overload!**
  - Cranks resonance and distortion to 100% and triggers flashing red/yellow hazard siren strobing!

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
