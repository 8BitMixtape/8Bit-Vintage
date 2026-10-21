# ABK-20 HARDWARE & INTERFACE DIAGNOSTIC TEST

A comprehensive diagnostic suite and interactive synthesizer designed to verify all visual and audio capabilities of the **ABK-20** (ATtiny85 / 8Bit Mixtape platform).

---

## 🎛️ Hardware Interface & Visual Mapping

The 5×4 NeoPixel matrix (20 LEDs total) is divided into dedicated functional zones:

```
    Col 0   Col 1           Col 2 (Center)          Col 3   Col 4
  +-------+-------+-------------------------------+-------+-------+
  |  (0)  |  (1)  | (2) Toggle UP (Bright Cyan)   |  (3)  |  (4)  |  Row 0 (Top)
  |  (5)  |  (6)  | (7) Button 3 (Magenta Strobe) |  (8)  |  (9)  |  Row 1
  | (10)  | (11)  | (12) Button 3 (Magenta Strobe)| (13)  | (14)  |  Row 2
  | (15)  | (16)  | (17) Toggle DOWN (Yellow)     | (18)  | (19)  |  Row 3 (Bottom)
  +-------+-------+-------------------------------+-------+-------+
    ROTARY POT2              CONTROLS                 SLIDE FADER RV1
    (Pitch Meter)                                     (Timbre Meter)
```

---

## 🎚️ Controls & Audio Modes

### 1. Rotary Potentiometer (`POT2` / PB4 / A2)
* **Visual**: Columns 0 & 1 dual vertical bar graph (Deep Green at bottom $\to$ Electric Cyan at top).
* **Audio**: Continuous, smooth pitch sweep across the full audio range ($\sim 65\text{ Hz}$ to $\sim 880\text{ Hz}$).
* **Hardware Compensation**: Automatically compensates for the high-side $4.7\text{k}\Omega$ series resistor and inverted mechanical sweep.

### 2. Slide Fader (`RV1` / PB2 / A1)
* **Visual**: Columns 3 & 4 dual vertical bar graph (Warm Amber at bottom $\to$ Fiery Red at top).
* **Audio**: Real-time waveform timbre morphing:
  - *Bottom third*: Smooth, warm **Triangle wave**
  - *Middle third*: Bright, cutting **Sawtooth wave**
  - *Top third*: Edgy **Pulse wave** with variable pulse width modulation (PWM)

---

### 3. SPDT Toggle Switch (`SW3` / PB3 / A3)
* **Center (Idle / Lead Tone)**:
  - **Visual**: Center column middle pixels (7 & 12) breathe in a soft purple pulse; top (2) and bottom (17) LEDs remain dimly lit in cyan/amber standby.
  - **Audio**: Clean, steady continuous synthesizer tone.
* **Toggle UP (Major Acid Arpeggiator)**:
  - **Visual**: Pixel 2 bursts into solid **Bright Cyan**.
  - **Audio**: Fast, interlocking 4-step major arpeggiator (Root $\to$ Major 3rd $\to$ 5th $\to$ Octave) tracking the rotary pot pitch.
* **Toggle DOWN (Bouncy Bunny Hop Mode)**:
  - **Visual**: Pixel 17 bursts into solid **Bright Yellow**.
  - **Audio**: Rubbery, springy cartoon "boing!" synthesizer. Pitch sweeps upward on each hop and decays exponentially down to the root tone with resonant spring bite.
  - **Bounce Tempo**: Controlled in real time by the **Slide Fader** (lazy cartoon trampoline boings up to rapid rabbit chatter).

---

### 4. Pushbutton 3 (`SW4` / PB3 / A3)
* **Solo Action**:
  - **Visual**: Pixels 7 & 12 burst into high-intensity **White / Magenta**.
  - **Audio**: Instantly transposes the pitch **$+1$ full octave**, injects aggressive overdrive clipping, and triggers a tactile audio click.
* **Mega Kangaroo Squeak (in Toggle DOWN)**:
  - Leaps **$+2$ full octaves** into a high cartoon squeak.

---

### 🌈 5. Secret Mode: Full-Matrix Rainbow Beat Wash!
* **Trigger**: Flip Toggle **DOWN** and hold **Button 3** simultaneously!
* **Visual**: The entire 5×4 matrix (all 20 NeoPixels) illuminates in a unified color field pulsing intensely in sync with the audio hop beat.
* **Color Sweep**: Rotating the rotary knob (`POT2`) sweeps smoothly through the entire 360° rainbow spectrum (Red $\to$ Gold $\to$ Lime $\to$ Cyan $\to$ Blue $\to$ Magenta).

---

## 🚀 Building & Uploading

1. Build firmware:
   ```bash
   pio run
   ```
2. Upload via AudioBoot:
   - Connect the ABK-20 audio jack to your computer output.
   - Power-cycle the ATtiny85 into TinyAudioBoot bootloader mode.
   - Run the upload command:
     ```bash
     pio run --target upload
     ```
   *(Or play `docs/wav/interface_test_akb20.wav` directly into the board's input).*

