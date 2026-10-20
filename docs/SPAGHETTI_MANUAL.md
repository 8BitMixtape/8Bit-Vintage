# SPAGHETTI! — THE TALKING CHIP (ITALIAN VOICE SAMPLER)

An 8-bit digitized speech synthesizer and voice sampler for the **8Bit Mixtape** (ATtiny85), playing back the digitized voice sample: **"Spaghetti!"**

---

## 🍝 The Speech Architecture

By utilizing compressed 8-bit unsigned PCM audio stored directly in ATtiny85 Flash memory (`PROGMEM`), this sketch turns the 8-pin microcontroller into an interactive talking instrument:
* **High-Speed PWM Audio Engine**: Timer1 running in 64 MHz PLL high-speed PWM mode outputting directly on PB1 (`OC1A`) at a ~250 kHz carrier frequency.
* **Non-Linear Pitch/Speed Shifting**: Modulating the microsecond playback delay between samples shifts the voice pitch from a deep sub-bass demonic monster to a hyperactive cartoon chipmunk.
* **Real-time Hardware Bitcrushing**: Truncating low-order bits dynamically scales quantization resolution from warm 8-bit down to a gritty, distorted 1-bit industrial square wave.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Pitch & Speed**:
  - Sweeps sample delay from $50\ \mu\text{s}$ (super fast high pitch) to $800\ \mu\text{s}$ (slow deep monster voice).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Dynamic Bitcrusher**:
  - Controls bit resolution from clean 8-bit down to crushed 1-bit quantization.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Clean Speech**: Triggers the clean, pitch-shifted "Spaghetti!" sample.
* **Right Button (Tap)**:
  - **Bitcrushed Cyber-Voice**: Triggers "Spaghetti!" through the real-time bitcrush distortion engine.
* **Both Buttons (Hold)**:
  - **Machine-Gun Stutter Loop**: Fires rapid-fire "Spag-Spag-Spag-Spaghetti!" loops with high-speed party strobe!

---

## 🌈 5x4 NeoPixel Matrix Visuals

* **Speech Amplitude Tricolore**: The 20 NeoPixels light up as the Italian Tricolore (Basil Green, Mozzarella White, Pomodoro Red) pulsing in real time to the voice amplitude.
* **Bitcrush Column Level**: The rightmost column indicates the current bitcrush depth.
* **Idle Ambient Glow**: When idle, the matrix gently breathes with an ambient Italian flag glow that reacts to pot adjustments.

---

## 🚀 Building & Uploading

1. Build firmware & generate bootloader WAV:
   ```bash
   pio run
   ```
2. Upload via AudioBoot:
   - Connect your 8Bit Mixtape audio jack to your computer's sound output.
   - Power-cycle the ATtiny85 into AudioBoot bootloader mode.
   - Manually run:
     ```bash
     pio run --target upload
     ```
   *(Or play `docs/wav/spaghetti.wav` directly into the tape's audio input).*

---

## 🎙️ Recording Your Own Samples

You can record your own live voice or convert any audio file into a C header using the included tools:

* **Live Microphone Recording**:
  ```bash
  python3 scripts/record_speech.py -o include/my_sample.h
  ```
* **Convert Existing Audio File (WAV/MP3)**:
  ```bash
  python3 scripts/audio2sample.py voice.wav -o include/my_sample.h
  ```
