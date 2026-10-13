# KRAFTWERK — THE MODEL ("DAS MODEL")

An authentic 8-bit multi-voice synthesizer tribute to **Kraftwerk's** 1978 electro-pop masterpiece **"Das Model"** (from *Die Mensch-Maschine*) for the **8Bit Mixtape** (ATtiny85).

---

## 🎹 Kling Klang Sound Architecture

Models the iconic electronic instruments and production techniques developed at Düsseldorf's legendary Kling Klang Studio:

1. **Minimoog Lead Melody**:
   - The haunting, precise electronic lead hook (*E5 - C5 - A4 - E5 / F5 - D5 - A4 - F5 / E5 - B4 - G#4...*) plus the classic German vocal theme (*"Sie ist ein Model und sie sieht gut aus..."*).
   - Warm analog pulse with punchy filter decay, variable cutoff, and 4 switchable synthesizer voicings.
2. **Kling Klang 16th-Note Arpeggiator**:
   - Continuous hypnotic 16th-note triads (Am, Dm, E) bubbling underneath the lead in true electro-pop precision.
3. **Driving 16th-Note Analog Bassline**:
   - Resonant sub-bass foundation delivering the robotic, metronomic groove.
4. **Electronic Drum Synthesis (Böhm / Simmons style)**:
   - Analog kick with exponential pitch drop, crisp electronic noise clap/snare on beats 2 & 4, and 16th-note electro hats.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Kling Klang Clock Tempo (BPM)**:
  - Precise metronomic tempo control from 90 BPM to 145 BPM (centered around the original 124 BPM).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Minimoog VCF Filter Cutoff & Sub-Bass Resonance**:
  - Sweeps the analog filter cutoff and sub-bass depth from dark Kraftwerk minimalism to bright, punchy synthesizer sheen.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Synth Lead Voicings**:
    1. **Minimoog Lead**: Warm analog pulse with snappy filter envelope.
    2. **Computerwelt Bleeps**: Pure electronic triangle waveform.
    3. **Robot Vocoder Pulse**: Narrow 20% robotic pulse wave.
    4. **Sawtooth Lead**: Raw, aggressive analog saw.
  - *Matrix flashes voice color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Kling Klang Visualizer Modes**:
    1. **Die Mensch-Maschine**: Red shirt & black tie mannequin silhouette with beating heart pulse.
    2. **Kling Klang Oscilloscope**: Green analog CRT oscilloscope trace scanning the audio waveform.
    3. **Die Roboter (The Robots)**: Robotic cyan scanner with synchronized mechanical step bars.
    4. **Radioaktivität (Radioactivity)**: Iconic yellow & black radioactive hazard trefoil symbol pulsing to the beat.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Kling Klang Tape Stutter & Computer Glitch Break!**
  - Triggers a 4x turbo roll with rapid electronic tape stuttering and rhythmic white-noise bursts.

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
