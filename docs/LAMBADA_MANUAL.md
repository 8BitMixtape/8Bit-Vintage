# LAMBADA ("Chorando se foi") — Tropical Carnival Synth

An authentic multi-voice 8-bit synthesizer arrangement of Kaoma's legendary 1989 summer anthem **"Lambada"** (*"Chorando se foi"*) for the **8Bit Mixtape** (ATtiny85).

---

## 🌴 Overview & Musical Architecture

The sketch runs a 10kHz sample-rate audio engine and step sequencer running simultaneously alongside the 5×4 NeoPixel matrix:

1. **Lead Voice (Accordion / Tropical Lead)**:
   - Plays the full 16-bar (256 sixteenth-note) Lambada arrangement in D minor:
     - **Bars 1–4**: The world-famous Accordion Hook (Part 1).
     - **Bars 5–8**: Accordion Hook (Part 2 with high $G_5$ variation and verse pickup).
     - **Bars 9–12**: Verse Phrase 1 (*"Chorando se foi quem um dia só me fez chorar..."*).
     - **Bars 13–16**: Verse Phrase 2 (*"Chorando estará ao lembrar de um amor que um dia não soube cuidar..."*).
2. **Bass Voice (Brazilian Carimbó / Zouk)**:
   - Deep syncopated bass bouncing on steps 0, 4, 6, 8, 12, 14 with octave pops.
3. **Percussion (Latin Dembow / Zouk Groove)**:
   - Deep pitched 808-style kick on beats 1 & 3 (steps 0, 8).
   - Syncopated rimshot/snare hits on steps 6 & 14.
   - Sizzling Latin shaker accents on steps 3 & 11.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Tempo (BPM)**:
  - Turn clockwise to speed up the tempo from a slow sultry 70 BPM to a 160 BPM Brazilian Carnival frenzy!
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Groove & Timbre Modulator**:
  - Blends sub-bass depth, kick punch, and accordion brightness/harmonics.
  - Turn left for a warm, nocturnal acoustic vibe; turn right for maximum punch and sizzling brassy leads!

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Synth Voices**:
    1. **Kaoma Accordion** (Dual reeds with tremolo/chorus vibrato — authentic sound!)
    2. **Tropical Marimba / Steel Pan** (Wooden acoustic pluck with crisp decay)
    3. **8-Bit Chiptune** (Classic 25% duty cycle retro Gameboy pulse wave)
    4. **Sizzling Saw** (Fat analog sawtooth with raw harmonics)
  - *Matrix flashes the voice color indicator on change.*

* **Right Button (Tap)**:
  - **Cycles 4 NeoPixel Visualizer Modes**:
    1. **Copacabana Sunset & Waves**: Golden sun and shifting sunset skies over turquoise ocean waves leaping to the bass.
    2. **Dancing Lambada Couple**: Two dancers animated on the 5×4 grid swaying side-to-side in sync with the 3-3-2 rhythm.
    3. **Carnival 5-Band Equalizer**: Dynamic VU spectrum meter displaying Kick, Bass, Snare, Lead Pitch, and Timbre.
    4. **Samba Fire & Tropical Sparks**: Confetti bursts and tropical fire leaping on downbeats.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Carnival Turbo Stutter Roll!**
  - Triggers a rapid 32nd-note snare & kick roll with Brazilian yellow/green strobe lighting for live DJ fills and drops. Releasing drops right back into the Lambada groove!

---

## 🚀 Building & Uploading

1. Build the firmware and WAV:
   ```bash
   pio run
   ```
2. Power-cycle the ATtiny into the AudioBoot bootloader, then manually upload:
   ```bash
   pio run --target upload
   ```
   *(Or play `.pio/build/attiny85/firmware.wav` into the audio jack).*
