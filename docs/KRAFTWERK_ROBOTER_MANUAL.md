# KRAFTWERK — DIE ROBOTER ("THE ROBOTS") [MOS 6581 SID SOUND ENGINE]

An authentic 8-bit multi-voice synthesizer tribute to **Kraftwerk's** 1978 electro anthem **"Die Roboter" / "The Robots"** (from *Die Mensch-Maschine*) powered by a true **Commodore 64 MOS 6581/8580 "SID-style" Direct Digital Synthesis (DDS)** sound engine for the **8Bit Mixtape** (ATtiny85).

---

## 🎹 MOS 6581 SID Sound Architecture

Brings the legendary C64 SID chip synthesis methods to Kraftwerk's automated live aesthetic:

1. **Voice 1: Lead / Vocoder / Sync / Ring Modulator**:
   - The iconic vocoder melody (*"Wir sind die Ro-bo-ter"*) plus the Russian vocoder phrase (*"Я твой слуга, я твой работник"* / *"Ja tvoi sluga, ja tvoi rabotnik"*).
   - **Dynamic Pulse-Width Modulation (PWM LFO)**: Sweeps pulse width with a ~4Hz LFO for fat, rich chorused brass.
   - **Authentic SID Ring Modulation**: Modulates Voice 1's triangle wave MSB with Voice 3's MSB for metallic, clanging robotic vocoders.
   - **Authentic SID Hard Sync**: Forced phase-reset slave to Voice 3 for screaming, tearing harmonic sync leads.
   - **Independent Hardware-Style ADSR Envelope**: Fast attack, snappy decay, 75% sustain, smooth musical release.
2. **Voice 2: 50Hz Fast SID Tracker Arpeggiator**:
   - Classic Commodore 64 tracker arpeggio (`0xy` effect) cycling 3-note chords (Em, D, C, B) at 50Hz with 25% duty-cycle pulse wave.
3. **Voice 3: Driving Bass & Percussive Noise Drums (Sync/Mod Master)**:
   - Deep, resonant 16th-note octave-jumping bassline (E2 <-> E3 bounce).
   - Also functions as the master oscillator for Voice 1 Hard Sync and Ring Modulation!
   - Synthesizes electronic kick (pitch-sweep triangle) and Simmons electronic snare/clap (Galois LFSR white-noise burst).
4. **Resonant Low-Pass SID Filter Simulation (MOS 6581 VCF)**:
   - 1-pole state variable filter simulation with dynamic cutoff and resonance feedback sweeps controlled by potentiometer.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Robot Metronome Tempo (BPM)**:
  - Adjusts sequencer clock from 85 BPM to 140 BPM (nominal 115 BPM).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — SID Filter Cutoff & PWM Modulation Depth**:
  - Sweeps between mellow sub-heavy warmth and screaming, resonant SID chorusing with deep PWM modulation!

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 SID Voice Timbres**:
    1. **SID PWM Brass Lead (Cyan)**: LFO-modulated pulse width with rich chorusing.
    2. **SID Ring Modulation (Orange)**: Metallic robotic vocoder ring-mod with Voice 3!
    3. **SID Hard Sync (Magenta)**: Ripping sync sweeps synced to Voice 3.
    4. **Raw MOS 6581 Sawtooth (Green)**: Aggressive classic chiptune saw.
  - *Matrix flashes voice color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Visualizer Modes**:
    1. **Die Roboter Head**: Iconic red casing, steel cheeks, and glowing cyan visor eyes pulsing to the vocoder syllables.
    2. **Walking Automaton**: Mechanical robot stepping back and forth to the 16th-note beat.
    3. **5-Band Vocoder Spectrum**: Real-time frequency analyzer dancing across 5 LED columns.
    4. **Cyrillic Teletype / Binary Cascade**: Matrix data stream cascade in glowing phosphor green.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Robot Malfunction Overload! (ERROR 404)**
  - Triggers a 4x turbo stutter breakdown with rapid industrial noise bursts and emergency red/white strobe!

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
