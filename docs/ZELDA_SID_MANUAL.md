# THE LEGEND OF ZELDA — OVERWORLD THEME (SID SOUND ENGINE)

An authentic full-scale arrangement of Koji Kondo's legendary 1986 **The Legend of Zelda** Overworld Theme powered by a true Commodore 64 MOS 6581/8580 **"SID-style"** sound engine for the **8Bit Mixtape** (ATtiny85).

---

## 🗡️ The SID Chiptune Sound Architecture

Unlike basic square-wave players, this sketch models the iconic hardware features of the Commodore 64 SID sound chip:

1. **3 Independent Polyphonic SID Voices**:
   - **Voice 1 (Heroic Lead)**: Pulse wave with dynamic **Pulse-Width Modulation (PWM LFO)** that smoothly modulates the duty cycle at ~4Hz, creating the rich, fat, chorused singing brass tone of C64 demoscene anthems.
   - **Voice 2 (SID Fast Arpeggiator Chords)**: 50Hz tracker-style arpeggiator (Tracker `0xy` command) cycling 3-note triads every 20ms to produce shimmering chiptune chords on a single oscillator.
   - **Voice 3 (Bass & Military Snare March)**: Multiplexed deep triangle bass and pitched LFSR noise bursts rendering the driving Zelda marching groove.
2. **Hardware-style ADSR Envelope Generators**:
   - Independent 4-stage state machines (Attack, Decay, Sustain, Release) providing punchy attacks and natural acoustic sustains.
3. **Resonant Low-Pass SID Filter Simulation**:
   - Dynamic 1-pole state variable filter controllable via the right potentiometer.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Quest Tempo (BPM)**:
  - Adjust tempo from a majestic 80 BPM Hyrule stroll to a 145 BPM boss sprint.
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — SID Filter Cutoff & PWM Modulation Depth**:
  - Sweeps between mellow authentic Famicom tone and screaming, resonant, singing 6581 SID brass with deep PWM chorusing!

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles 4 SID Voice Timbres**:
    1. **SID PWM Brass Lead** (LFO-modulated pulse width chorusing)
    2. **SID Fast Arp Lead** (Tracker arpeggio chords + sawtooth blend)
    3. **NES 2A03 Classic APU** (Pure 50% square & triangle chiptune)
    4. **Overdriven 6581 SID Monster** (Raw resonant sawtooth bite)
  - *Matrix flashes voice color badge on change.*

* **Right Button (Tap)**:
  - **Cycles 4 Zelda Visualizer Modes**:
    1. **The Golden Triforce & Sacred Realm**: Three golden triangles with pulsing emerald aura.
    2. **The Master Sword**: Pedestal of Time with glowing holy rays.
    3. **Heart Containers**: Life meter beating in time with the march.
    4. **Rupee Storm**: Shimmering 1-Rupee (green), 5-Rupee (blue), and 20-Rupee (red) gems falling across Hyrule.
  - *Matrix flashes white briefly on mode change.*

* **Both Buttons (Hold)**:
  - **Secret Item Fanfare & Fairy Revival Stutter!**
  - Triggers a rapid triumphant chiptune arpeggio flourish with rainbow Triforce strobe.

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
