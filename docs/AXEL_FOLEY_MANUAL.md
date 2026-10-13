# AXEL FOLEY ("Axel F") — 80s Retro Chiptune Synth

An authentic multi-voice 8-bit synthesizer arrangement of Harold Faltermeyer's iconic 1984 theme **"Axel F"** from *Beverly Hills Cop* for the **8Bit Mixtape** (ATtiny85).

---

## 🕶️ Overview & Musical Architecture

The sketch runs a 10kHz sample-rate audio engine and step sequencer simultaneously driving the 5×4 NeoPixel matrix:

1. **Lead Voice (80s Staccato Synth Lead)**:
   - Full 8-bar (128 sixteenth-note) arrangement in **F minor**:
     - **Bars 1–3**: The iconic staccato theme (*F4, Ab4, Bb4, C5, Db5, F5*).
     - **Bar 4**: Driving Moog bass breakdown and turnaround.
     - **Bars 5–7**: Theme reprise with dynamic bass accents.
     - **Bar 8**: Ascending 80s synth run (*F4 → Ab4 → Bb4 → C5 → Db5 → Eb5 → F5*).
2. **Bass Voice (Driving 16th-Note Moog Bass)**:
   - Punchy synth bassline driving on 16th-note pulses with octave jumps.
3. **Percussion (80s LinnDrum / Simmons Beats)**:
   - Punchy low-end kick (beats 1 & 3).
   - Gated electro snare/clap with crack on beats 2 & 4.
   - Sizzling 16th-note electro hi-hat with open hats on offbeats.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Tempo (BPM)**:
  - Turn clockwise to dial tempo from 90 BPM (deep synthwave groove) up to 150 BPM (high-octane Beverly Hills car chase).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — 80s Filter Cutoff & Resonance**:
  - Morphs between warm analog brass and razor-sharp cutting FM lead with boosted sub-bass punch.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Vintage 80s Synth Voices**:
    1. **Jupiter-8 Synth Brass** (Brassy saw with pulse width modulation)
    2. **Yamaha DX7 FM Bell / Chime** (Crisp FM electric tine harmonics)
    3. **Commodore 64 SID Lead** (Pure 8-bit pulse with authentic vibrato)
    4. **Moog Resonant Saw** (Fat analog synth lead)
  - *Matrix flashes voice color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Retro Visualizer Modes**:
    1. **Police Cruiser Beacon**: Alternating Beverly Hills Red & Blue flashing lights with white flash on snares.
    2. **Outrun 80s Synthwave Grid**: Glowing neon magenta sun with animated rolling horizon gridlines.
    3. **80s Boombox Graphic Equalizer**: Retro 5-band spectrum meter (Kick, Bass, Snare, Lead, Timbre).
    4. **Miami Vice Neon Chase**: Fast electric cyan and hot magenta neon trails dashing across the screen.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Turbo 80s Machine-Gun Stutter Roll!**
  - Triggers a rapid electro snare roll with police red/blue strobe lighting for live drops.

---

## 🚀 Building & Uploading

1. Build firmware:
   ```bash
   pio run
   ```
2. Power-cycle the ATtiny into the AudioBoot bootloader, then manually upload:
   ```bash
   pio run --target upload
   ```
   *(Or play `.pio/build/attiny85/firmware.wav` into the audio jack).*
