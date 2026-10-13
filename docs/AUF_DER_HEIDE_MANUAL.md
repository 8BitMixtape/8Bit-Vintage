# AUF DER HEIDE ("Erika") — Vintage German Brass & Chiptune Marsch

An authentic multi-voice 8-bit synthesizer arrangement of the traditional German tune **"Auf der Heide blüht ein kleines Blümelein"** (*"Erika"*, Herms Niel, 1938) for the **8Bit Mixtape** (ATtiny85).

---

## 🎺 Overview & Musical Architecture

The sketch runs a 10kHz sample-rate multi-voice audio engine alongside the 5×4 NeoPixel matrix:

1. **Lead Voice (Oom-Pah Brass / Volkstümliche Melodie)**:
   - Full 8-bar (128 sixteenth-note) arrangement in **G major**:
     - **Bars 1–4**: Phrase 1 (*"Auf der Heide blüht ein kleines Blümelein... BUM BUM BUM ... Erika!"*).
     - **Bars 5–8**: Phrase 2 (*"Heiß von hunderttausend kleinen Bienelein... BUM BUM BUM ... Erika!"*).
2. **Tuba Bass Voice (Alternating Oom-Pah Bass)**:
   - Alternating root-fifth marching bass (*G2 → D3 → G2 → D3*).
3. **Percussion & The 3 Stomps**:
   - The iconic three heavy stomps (**"BUM! BUM! BUM!"**) rendered on bars 2, 3, 6, 7 with deep kick and snare impact.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Marsch-Tempo (BPM)**:
  - Adjust tempo from a relaxed 75 BPM stroll to a 140 BPM brisk march.
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Tuba Depth & Stomp Punch**:
  - Blends the deep oom-pah tuba sub-bass weight and brass horn cutoff.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 Synth Voices**:
    1. **Oom-Pah Brass Horn** (Full-bodied German brass ensemble sound)
    2. **Hohner Alpine Accordion** (Dual-reed accordion with tremolo)
    3. **8-Bit Chiptune Marsch** (Classic square wave)
    4. **Alpine Glockenspiel / Carillon** (High ringing bell chime)
  - *Matrix flashes voice color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Visualizer Modes**:
    1. **Blühendes Heidekraut**: Purple/violet heather blossoms under alpine blue skies.
    2. **Alpine Bergwanderer**: Moving mountain peaks and green valley horizon.
    3. **Brass Oom-Pah Equalizer**: Traditional brass spectrum meter.
    4. **The 3 Stomps Concentric Pulse**: Expanding shockwave rings pulsing on every stomp.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Trommelwirbel Snare Roll / Stomp Hold**:
  - Triggers a rapid marching snare roll with golden strobe light for dramatic drum fills.

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
