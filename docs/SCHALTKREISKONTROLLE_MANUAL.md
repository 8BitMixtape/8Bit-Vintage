# SCHALTKREISKONTROLLE! — THE TALKING CHIP

A speech playback and Lo-Fi bitcrush synthesizer for the **8Bit Mixtape** (ATtiny85), playing back a digitized German robotic voice sample saying:

> ***"Schaltkreiskontrolle!"*** *(Circuit Control!)*

---

## 🗣️ Speech Synthesis Architecture

By utilizing compressed 8-bit unsigned PCM audio stored directly in ATtiny85 Flash memory (`PROGMEM`), this sketch turns the 8-pin microcontroller into a talking chip:

1. **8-Bit High-Speed PWM Speech DAC**:
   - Audio is streamed to `PB1` via the 64 MHz Timer1 PLL running at a 250 kHz PWM carrier.
2. **Variable Sample Rate & Pitch Engine**:
   - By modulating the microsecond delay per sample between 50µs and 800µs, the left potentiometer continuously sweeps the playback speed and pitch from a deep robotic underworld monster to a fast high-pitched chipmunk.
3. **Dynamic Bitcrusher (Resolution Quantizer)**:
   - Discards lower bits of the 8-bit audio waveform in real time (from 8-bit clarity down to 1-bit extreme fuzz square crunch).
4. **Speech-Waveform Visualizer on 5x4 Matrix**:
   - The 20 NeoPixels pulse red in direct proportion to the speech waveform amplitude, while the rightmost column displays the current bitcrush setting.
   - When idle, an ambient RGB breathing animation reflects the positions of both potentiometers.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Pitch & Playback Speed**:
  - Sweeps sample delay from 50µs (very fast/high chipmunk pitch) to 800µs (slow/deep robotic monster).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Bitcrush / Lo-Fi Quantization**:
  - Sets bitcrush depth from clean 8-bit speech to 1-bit harsh robotic crunch.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Button Tap (< midLevel)**:
  - Plays clean pitch-shifted speech ("Schaltkreiskontrolle!").
* **Button Tap (> midLevel)**:
  - Plays speech with the Bitcrush resolution quantizer engaged.

---

### 💡 NeoPixel Visuals (5x4 Matrix)
* **During Speech**: All 20 NeoPixels flash bright red proportional to voice volume (instant VU-meter / speech mouth), with the right column indicating bitcrush pot depth.
* **Idle State**: Gentle pulsing RGB wave smoothly blending colors mapped to the two potentiometers.

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
   *(Or play `docs/wav/schaltkreiskontrolle.wav` into the audio jack).*

