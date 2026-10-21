# ACID ABK-20 — HARDWARE ACID SYNTHESIZER

A Roland TB-303 inspired acid bassline synthesizer and drum machine calibrated specifically for the **ABK-20** (ATtiny85 / 8Bit Mixtape platform).

---

## 🎛️ Architecture & Features

* **303 State-Variable Resonant Filter (SVF)**:
  - Fast 16-bit state-variable filter calculation executing inside Timer1 PWM interrupt at $15.6\text{ kHz}$.
  - Delivers authentic screaming resonance, acid slides, and exponential accent decays.
* **Dual Drum Engine**:
  - Punchy 808-style pitch-decay kick drum.
  - LFSR white-noise hi-hat.
* **Hardware Calibration (4.7k Series Resistors)**:
  - Specially calibrated for the ABK-20 voltage divider geometry on both potentiometers, providing smooth full $0\text{--}255$ filter cutoff control and clean 4-quadrant pattern selection across the physical travel.
* **Dual-Tempo Performance Modes**:
  - SPDT toggle switch switches between classic $130\text{ BPM}$ acid house groove and $166\text{ BPM}$ fast driving techno.
* **Button 3 Instant Acid Octave Boost**:
  - Live performance button transposing the bassline up $+1$ full octave with maximum accent resonance bite and a neon acid lime matrix strobe.

---

## 🕹️ Hardware Interface Controls

```
    Col 0   Col 1   Col 2   Col 3     Col 4 (Pattern Indicator)
  +-------+-------+-------+-------+-----------------------------+
  |  (0)  |  (1)  |  (2)  |  (3)  |  (4) Pattern 0: Classic Acid |  Row 0
  |  (5)  |  (6)  |  (7)  |  (8)  |  (9) Pattern 1: Arpeggio     |  Row 1
  | (10)  | (11)  | (12)  | (13)  | (14) Pattern 2: G-Min Groove |  Row 2
  | (15)  | (16)  | (17)  | (18)  | (19) Pattern 3: Octave Jumps |  Row 3
  +-------+-------+-------+-------+-----------------------------+
              303 RHYTHM MATRIX              PATTERN METER
```

### 🎚️ Potentiometers
* **Rotary Pot (`POT2` / PB4 / A2) — Filter Cutoff**:
  - Clockwise opens the filter into wide, screaming 303 resonance ($0 \to 255$).
  - Counter-clockwise closes the filter down into a deep, muffled sub-bass thud.
* **Slide Fader (`RV1` / PB2 / A1) — Pattern Selector**:
  - Divided into 4 equal segments across the 20mm fader travel:
    - **Position 0 (Bottom)**: *Classic 303 Acid* (Four-on-the-floor kick, rolling 16th bassline)
    - **Position 1**: *Techno Arpeggio* (High-speed running arpeggio with driving hi-hats)
    - **Position 2**: *G-Minor Groove* (Funky syncopated breakbeat groove)
    - **Position 3 (Top)**: *Octave Jumps* (Hypnotic jumping octaves with electro rhythm)

---

### 🔘 Toggle Switch & Buttons (PB3 / A3)
* **Toggle UP — Classic Acid Tempo**:
  - Runs sequence continuously at **$115\text{ ms}$ per step** ($\sim 130\text{ BPM}$).
* **Toggle DOWN — Fast Techno Tempo**:
  - Runs sequence continuously at **$90\text{ ms}$ per step** ($\sim 166\text{ BPM}$).
* **Toggle Center — Standby / Idle**:
  - Stops playback; matrix transitions into an ambient breathing glow mapped to potentiometer colors.
* **Button 3 (`SW4`) — Instant Octave Scream & Accent**:
  - Transposes the sequence **$+1$ full octave** up in pitch.
  - Forces maximum accent envelope punch (`bass_env = 255`) and instant slide.
  - Flashes the entire matrix in brilliant **Neon Acid Lime**.

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
   *(Or play `docs/wav/acid_akb20.wav` directly into the board's input).*
