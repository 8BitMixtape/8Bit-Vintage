# The History, Architecture & Culture of the 8Bit Mixtape & Coconut Ecosystem

> *"Offene Do-It-Yourself-Arbeitswoche für geniale Experimentierer: The art of 8Bit sounds on microcontrollers!"*  
> — SGMK HomeMade, 2007

---

## Executive Summary

The **8Bit Mixtape** is an open-source, lo-fi algorithmic synthesizer and educational platform built around the Atmel/Microchip **ATtiny85** 8-bit microcontroller. Originating from the Swiss DIY mechatronics scene, South Asian hackathons, and Indonesian open-culture hacker spaces, the project challenges the conventional boundaries between electronic music instruments, circuit design, and folk culture.

Its signature innovations include:
1. **The Audio Bootloader (`TinyAudioBoot` / `Hex2Wav`)**: Eliminating programmer dongles and FTDI drivers by transmitting compiled firmware directly through the headphone jack as audio frequency-shift keyed (FSK) `.wav` data.
2. **Extreme Pin Multiplexing**: Routing 2 potentiometers, multi-button ladders, addressable NeoPixels, 64 MHz PWM audio, and clock sync onto an 8-pin DIP/SOIC microchip.
3. **Algorithmic Bytebeat Synthesis**: Evaluating compact bitwise mathematical formulas in real time to produce evolving rhythms, melodies, and drones.
4. **PCB Art as Medium (`svg2shenzhen`)**: Treating the printed circuit board as an artistic canvas via Inkscape vector integration rather than purely industrial layout.
5. **The Lineage to the ABK-20**: The natural evolution from cassette-shaped bytebeat player to a desktop wedge performance synthesizer combining digital code engines with an analog Korg MS-20 resonant VCF.

---

## 1. Historical Timeline & The Coconut Lore

```
2007          2011          2012           2013                2015           2016-2017        2018+
SGMK          CACR, Goa     BabyGnusbuino  Yogyakarta          Bangalore      NEO Era          ABK-20
HomeMade      "Wave-to-MIDI (Michi Egger,  Lifepatch & HONF    CocoMake7      TinyAudioBoot,   MS-20 Filter
SummerCamps   Coconut"      a.n.y.m.a.)    Viznut Bytebeats    (Hackteria)    NeoPixels,       & Desktop Wedge
                                           First Cassette Tape                Euro-Rack, DIMI  Console
```

### 1.1 SGMK & HomeMade SummerCamps (2007–2011)
The philosophical foundation began within the **Schweizerische Gesellschaft für Mechatronische Kunst (SGMK)**. Annual HomeMade SummerCamps in Switzerland gathered artists, engineers, and hackers in alpine retreats to experiment with low-cost electronics, microcontroller audio, and noise toys without institutional hierarchies. Early experiments explored ATmega sound synthesis with developers like Christoph Haberer (*atmegaSID*).

### 1.2 The Center for Alternative Coconut Research (Goa, 2011)
During residency and research in Goa, India, the **Center for Alternative Coconut Research (CACR)** was formed around a delightfully absurd challenge: building a "Wave-to-MIDI" converter inside an actual coconut shell using a USB-capable microcontroller (GNUSB). This cemented the "Coconut" as the enduring mascot of nomadic hardware hacking.

### 1.3 The BabyGnusbuino (2012)
Presented by Michael Egger (*a.n.y.m.a.*) at the SGMK DIY-Umtrunk in 2012, the **BabyGnusbuino** condensed the ATtiny85 into a thumb-drive USB form factor. It proved that an 8-pin microcontroller was capable of expressive interaction, fast prototyping, and standalone instrument creation.

### 1.4 Yogyakarta: Bytebeats & The First Mixtape (2013)
In 2013, dusjagr (Marc Dusseiller) brought the BabyGnusbuino to Yogyakarta, Indonesia, collaborating closely with **Lifepatch** (citizen initiative in art, science and technology) and **HONF** (House of Natural Fiber).

There, the team connected the ATtiny85 with the pioneering discoveries of Finnish demoscener **Ville-Matias Heikkilä (Viznut)** on algorithmic bytebeats. They realized that a single line of C code evaluating bitwise operations against an incrementing time counter `t` could yield rich, polyrhythmic chiptune symphonies without external storage or complex synthesis libraries.

To house the synth, the circuit was designed directly into the physical dimensions and silhouette of an **audio compact cassette**, placing the two potentiometers where the tape reels had once been. The **8Bit Mixtape** was born.

### 1.5 Bangalore & Kathmandu (2015–2016)
* **CocoMake7 (Bangalore, 2015)**: Hosted between Srishti Institute of Art, Design & Technology and Hackteria, focusing on creative circuit design, paper PCB prototyping, and accessible pedagogy.
* **K_Space / CocoSat (Kathmandu, 2016)**: Expanding into the Kathmandu Space Programme — humorously asking *"How to put a coconut satellite in low earth orbit?"* while running low-cost electronics workshops in Nepal.

### 1.6 Global Proliferation & The NEO Era (2017–Present)
The project traveled to hacklabs and festivals worldwide:
* **Zagreb & Ljubljana**: *Mixtapeica* workshops with Radiona (Zagreb Makerspace).
* **Barcelona**: MutanMonkey.
* **Taoyuan & Taipei**: Dimension Plus LAB residencies exploring PCB etching and mass manufacturing.
* **Tokyo**: Makezine Japan feature introducing the *"warm tropical heatwave of Indonesian chiptune culture"*.
* **Media Acclaim**: Features in *Create Digital Music (CDM)*, *Synthtopia*, *Synth Anatomy*, and *Keyboards.de*.

---

## 2. Hardware Lineage & Matrix of Variants

```
                          [BabyGnusbuino (2012)]
                                     |
                         [8Bit Mixtape Classic (2013)]
                                     |
               +---------------------+---------------------+
               |                     |                     |
     [Micro Coconut NEO]   [8Bit Mixtape NEO]        [DIMI-8]
       ("One Coconut"        (Cassette Tape,         (Erkki Kurenniemi
        Educational)          8x NeoPixels)           Homage Synth)
               |                     |
               |             [8Bit Euro-Rack]
               |             (3U Modular Synth)
               |                     |
               +----------+----------+
                          |
             [MS-20 Filter Mixtape (2018)]
             (Claude Winterberg & Christoph Stähli)
                          |
                [ABK-20 Desktop Console]
                (90x90mm Wedge, 5x4 Matrix,
                 Analog VCF, USB-C & Wood Cheeks)
```

### Detailed Hardware Comparison

| Variant | Target Use Case | Display / Visuals | Audio Circuitry | Power & Connectors |
| :--- | :--- | :--- | :--- | :--- |
| **8Bit Mixtape Classic** | Standalone pocket synth | Single status LED | 8-bit Timer1 PWM, passive RC low-pass | Dual AAA battery clips, 3.5mm stereo jack |
| **8Bit Mixtape NEO** | Audio-bootable cassette | 8x WS2812B RGB NeoPixels | 64 MHz PLL PWM, PAM8002 amp option | AAA batteries / 5V USB micro / 3.5mm CV & sync |
| **Micro Coconut NEO** | Scalable classroom workshops | 1x NeoPixel or status LED | Minimal RC PWM | Coin cell / AAA / direct pins ("OCPC") |
| **DIMI-8** | Algorithmic experimentalism | LED matrix / bargraph | Experimental bitwise sound engines | Handheld form factor |
| **8Bit Euro-Rack** | Modular synthesizer integration | NeoPixel strip / indicators | Dedicated CV buffers, 10V Eurorack protection | 10-pin Eurorack power header (±12V to 5V rail) |
| **Gär Lämpli** | Bio-art fermentation sensor | High-brightness RGB LED | Audio chirp on metabolic activity | USB power / LiPo battery |
| **ABK-20 (MS-20 Synth)**| Performance desktop wedge console | **5×4 NeoPixel Matrix (20 LEDs)** + Status LED | **ATtiny85 Digital Engine + Analog Korg MS-20 VCF** | 6-pin USB-C receptacle, master power toggle, 3.5mm I/O |

---

## 3. Electrical Architecture & ATtiny85 Pin Multiplexing

The Microchip ATtiny85 is an 8-pin microcontroller with 8 KB of Flash, 512 bytes of SRAM, and 512 bytes of EEPROM.

### 3.1 The Pin Allocation Map

```
                          ATtiny85 Pinout
                            +---+--+---+
     (RESET / PCINT5)  PB5 -| 1    8 |- VCC (+3.3V to +5.0V)
     (ADC3 / PCINT3)   PB3 -| 2    7 |- PB2 (ADC1 / SCK / PCINT2)
     (ADC2 / PCINT4)   PB4 -| 3    6 |- PB1 (MISO / OC1A / PCINT1)
                       GND -| 4    5 |- PB0 (MOSI / AREF / OC0A / PCINT0)
                            +----------+
```

| Pin | Primary Function | Secondary Function | Circuit Implementation Details |
| :--- | :--- | :--- | :--- |
| **PB0** | **NeoPixel Data Out** | ISP MOSI | Direct 800 kHz bit-banged stream to WS2812B / SK6812 DIN. |
| **PB1** | **PWM Audio Output** | OC1A / Timer1 | Driven by the 64 MHz high-speed asynchronous PLL; filtered by low-pass RC network to output jack. |
| **PB2** | **Right Potentiometer** | ADC1 / CV Input | 10kΩ linear potentiometer wiper; connects to external CV input for pitch/speed control. |
| **PB3** | **Navigation Buttons** | ADC3 | Multi-button resistor ladder voltage divider. |
| **PB4** | **Left Potentiometer** | ADC2 / AudioProg | Left potentiometer wiper + AC-coupled 100nF capacitor receiving FSK audio bootloader tones. |
| **PB5** | **Reset / Sync Output** | ADC0 / PCINT5 | Hardware reset; when `RSTDISBL` is programmed, acts as pulse clock output for Korg / PO sync. |

### 3.2 Resistor Ladder Multi-Button Sensing (PB3)
Instead of dedicating multiple pins to pushbuttons, buttons are wired to a voltage divider ladder feeding ADC3:
```
 VCC (+5V)
   |
  [R1: 22k]
   |--------- (SW_Left)   --> Reads ~2.5V (ADC ~512)
  [R2: 22k]
   |--------- (SW_Right)  --> Reads ~1.6V (ADC ~340)
  [R3: 22k]
   |--------- (SW_Mode)   --> Reads ~0.0V (ADC 0)
  GND
```
A single `analogRead(BUTTON_PIN)` differentiates `NONE`, `LEFT`, `RIGHT`, `MODE`, and simultaneous combinations with software debounce.

---

## 4. The Audio Bootloader (`TinyAudioBoot` & `Hex2Wav`)

### 4.1 Why an Audio Bootloader?
Traditional microcontrollers require specialized hardware (AVRISP, USBasp, FTDI cables) and driver installations (Zadig, libusb). For artists, musicians, and workshop participants, this barrier often stalled workshops before sound could even be made.

The **TinyAudioBoot** concept inverted this: **Every modern device (smartphones, tablets, laptops, Walkmans) already has a digital-to-analog audio converter through its headphone jack.**

```
+---------------+      Hex2Wav       +----------------+
| Arduino / PIO | -----------------> | .WAV Soundfile |
|  .HEX Binary  |   FSK Modulation   | (1200/2400 Hz) |
+---------------+                    +----------------+
                                              |
                                      Headphone Cable
                                              v
+------------------+   TinyAudioBoot  +----------------+
| ATtiny85 Program | <--------------- | 3.5mm Audio In |
|   Flash Memory   |  Edge Detection  |  (AC-Coupled)  |
+------------------+                  +----------------+
```

### 4.2 Encoding Specifications
* **Modulation Scheme**: Frequency Shift Keying (FSK).
* **Frequencies**: Standard carrier tones at 1200 Hz and 2400 Hz.
* **Transmission Speed**: ~1200 baud (flashing takes 15–40 seconds for an 8 KB binary).
* **Web Implementation (`Hex2WavJS`)**: Generates the audio stream directly in the browser using the Web Audio API, allowing users to compile in an online sandbox and flash by clicking "Play".
* **EEPROM Write Protocol**: A specialized payload header allows writing configuration variables directly to the ATtiny85 EEPROM without touching program flash.

---

## 5. Algorithmic Bytebeat Sound Engines

Bytebeat synthesis—pioneered by Viznut in 2011—generates music through pure arithmetic operations on an incrementing 32-bit time variable `t`. Running at an 8 kHz sample rate, each tick computes:
$$\text{Sample} = f(t) \pmod{256}$$

### 5.1 Classic Formula Archetypes Used in Mixtape Firmware

```c
// 1. Viznut's Iconic "Crowd / Chiptune" Canon:
OCR1A = (t * ((t >> 12 | t >> 8) & 63 & t >> 4));

// 2. Techno Groove with Percussion:
OCR1A = (t * 5 & t >> 7) | (t * 3 & t >> 10);

// 3. Ambient Harmonic Landscape:
OCR1A = ((t >> 6 | t | t >> (t >> 16)) * 10 + ((t >> 11) & 7));

// 4. Industrial Acid Drone:
OCR1A = (t * ((t >> 9 | t >> 13) & 15)) & (t >> 5);
```

### 5.2 High-Speed 64 MHz PLL PWM Audio (PB1)
Standard Arduino `analogWrite()` runs at ~490 Hz or 980 Hz, causing loud, intolerable carrier whine. The 8Bit Mixtape configures Timer1 to run from the internal **64 MHz PLL**:
```cpp
// Enable 64MHz PLL clock source for Timer1
PLLCSR = (1 << PCKE) | (1 << PLLE);

// Configure Timer1 for non-inverting high-speed PWM on PB1 (OC1A)
TCCR1 = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10);
```
This produces a PWM switching carrier at:
$$\frac{64\text{ MHz}}{256} = 250\text{ kHz}$$
Because 250 kHz is more than an octave above human hearing ($>20\text{ kHz}$), a minimal 100Ω + 100nF low-pass filter completely removes the carrier, yielding clear, punchy 8-bit analog audio.

### 5.3 The Dual-Clock Dilemma (NeoPixels vs. Audio Interrupts)
* **The Conflict**: Addressable WS2812B/SK6812 NeoPixels demand strict $\pm 150\text{ ns}$ signal timings that will be corrupted by any audio interrupt. Conversely, disabling interrupts while transmitting 20 NeoPixels (~600 µs) creates an audible dropout.
* **The Mixtape Strategy**: The main loop disables global interrupts (`cli()`), ensures the CPU prescaler is at 16 MHz, writes the NeoPixel frame, and immediately re-enables interrupts (`sei()`). By aligning LED update rates to musical bar subdivisions or drum hits, the dropout is masked as percussive silence.

---

## 6. External Interoperability & Sync

The 8Bit Mixtape was designed to integrate into analog synthesizer setups and modular gear:

### 6.1 Korg Volca Sync
* **Pulse Protocol**: Korg Volcas output and expect 2 pulses per quarter note (4 pulses per beat, 5V trigger).
* **Sync Out**: The Mixtape outputs sharp digital pulses from PB5 on the Ring channel of the 3.5mm stereo jack.
* **Sync In**: Pin-change interrupts on PB5 detect incoming Volca pulses and advance sequencer steps in real time.

### 6.2 Teenage Engineering Pocket Operator (PO) Sync
* Pocket Operators transmit a stereo click-track where the **Left channel carries mono audio** and the **Right channel carries analog pulse sync**.
* The Mixtape firmware matches PO sync standards (`SYNC 2` slave, `SYNC 3` master), enabling daisy-chaining with PO-12, PO-20, and PO-33 units.

### 6.3 Control Voltage (CV)
* The second potentiometer input (PB2 / ADC1) accepts 0 to +5V analog control voltage from modular synthesizers or external expression pedals to modulate pitch, sample rate, or filter sweeps.

---

## 7. PCB Art as Medium (`svg2shenzhen`)

The visual design of the 8Bit Mixtape is as central as its sound. Rather than using conventional schematic-to-board routing exclusively, the community developed **`svg2shenzhen`**—an automated toolchain that bridges Inkscape vector illustration with KiCad.

```
+------------------+     svg2shenzhen      +--------------------+
|  Inkscape SVG    | --------------------> | KiCad Footprints   |
| (F.Cu, F.Mask,   |   Vector-to-Gerber    | (.kicad_mod)       |
|  F.SilkS, Cuts)  |   Conversion          | & Board Layers     |
+------------------+                       +--------------------+
                                                     |
                                            Fab in Shenzhen
                                            (Matte Black, ENIG)
```

* **Exposed Gold & Copper**: By layering cutouts in `F.Mask` over `F.Cu`, raw metallic copper/ENIG gold becomes part of the graphic design.
* **Silkscreen Typography**: Bypassing KiCad’s default vector font in favor of custom graphic typography, retro cassette branding, and mascot illustrations.

---

## 8. The ABK-20: Evolution to the Analog Desktop Wedge

The **ABK-20** represents the newest chapter in this 15-year journey:

1. **Hybrid Synthesis**: Pairing the ATtiny85 algorithmic digital engine with the screaming, self-oscillating analog **Korg MS-20 resonant VCF** (circuit adaptation by Claude Winterberg & Christoph Stähli).
2. **From Cassette to Desktop Console**: Moving from the handheld tape format into a **90 × 90 mm wedge console** framed by wooden cheek side panels.
3. **5×4 NeoPixel Grid (20 LEDs)**: Expanding the 8-LED line of the Mixtape NEO into a full visualizer matrix under a diffused grid window.
4. **Modern Power Architecture**: Transitioning from AAA batteries to a 6-pin USB-C receptacle with 5.1kΩ CC pull-down resistors for seamless power negotiation with modern USB-PD and MacBook chargers.

---

## 9. Key References & Community Repositories

* **Main Project Wiki**: [8BitMixtape/8Bit-Mixtape-NEO Wiki](https://github.com/8BitMixtape/8Bit-Mixtape-NEO/wiki)
* **TinyAudioBoot Bootloader**: [ATtinyTeenageRiot/TinyAudioBoot](https://github.com/ATtinyTeenageRiot/TinyAudioBoot)
* **Hex2Wav Audio Converter**: [8BitMixtape/Hex2Wav](https://github.com/8BitMixtape/Hex2Wav)
* **svg2shenzhen PCB Art Workflow**: [badgeek/svg2shenzhen](https://github.com/badgeek/svg2shenzhen)
* **Center for Alternative Coconut Research**: [cocomake7.github.io](https://cocomake7.github.io/)
* **SGMK (Swiss Mechatronic Art Society)**: [wiki.sgmk-ssam.ch](http://wiki.sgmk-ssam.ch/)
* **Hackteria Global Network**: [hackteria.org](https://hackteria.org/)
