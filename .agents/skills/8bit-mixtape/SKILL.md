---
name: 8Bit Mixtape & Coconut Ecosystem
description: Comprehensive domain knowledge, historical lineage, hardware variants, and technical architecture for the 8Bit Mixtape and Coconut ecosystem. Covers TinyAudioBoot/Hex2Wav audio bootloading, pin multiplexing on ATtiny85, algorithmic bytebeat generation, KORG/Pocket Operator sync, PCB art workflows (svg2shenzhen), and the evolution towards the ABK-20 MS-20 synthesizer.
---

# 8Bit Mixtape & Coconut Ecosystem Guide

This skill captures the philosophy, history, hardware architecture, firmware conventions, and creative workflows developed across 15+ years of the **8Bit Mixtape** and the **Center for Alternative Coconut Research (CACR)**.

> For an in-depth chronicle, hardware comparisons, and design deep-dive, see the companion document:  
> [docs/HISTORY_AND_ARCHITECTURE.md](file:///home/dusjagr/Documents/PlatformIO/Projects/8Bit-Vintage/docs/HISTORY_AND_ARCHITECTURE.md)

---

## 1. Lineage & Philosophy

### Cultural Roots & The Coconut Origin
* **SGMK & HomeMade SummerCamps (2007–2011)**: Emerging from the Swiss Mechatronic Art Society (*Schweizerische Gesellschaft für Mechatronische Kunst*), HomeMade camps, and *a.n.y.m.a.* research weeks as hands-on DIY labs for *geniale Experimentierer*.
* **Goa & Center for Alternative Coconut Research (2011)**: Founded in Goa around the playful challenge of installing a GNUSB inside a physical coconut shell to build a "Wave-to-MIDI" converter.
* **BabyGnusbuino (2012)**: Designed by Michael Egger (*a.n.y.m.a.*) and presented at the SGMK DIY-Umtrunk — a minimalist USB-programmable ATtiny85 breakout stick that established the core platform.
* **Yogyakarta & Bytebeats (2013)**: Collaborations in Yogyakarta, Indonesia with Lifepatch and House of Natural Fiber (HONF). Developers discovered **Ville-Matias Heikkilä's (Viznut)** algorithmic bytebeat one-liners (`t * (t >> 12 & t >> 8)...`) and realized an ATtiny85 could generate complex, evolving techno and chiptunes directly in C code.
* **First 8Bit Mixtape Prototype (2013)**: The ATtiny85 circuit was shaped into a classic compact audio cassette, placing the potentiometers directly inside the cassette reel holes.
* **Global Nomadic Workshops**: Proliferated through grassroots workshops across Yogyakarta, Bangalore (CocoMake7), Kathmandu (K_Space / CocoSat), Zagreb (*Mixtapeica* / Radiona), Barcelona (MutanMonkey), Berlin, Tokyo, Hong Kong, and Zurich.

---

## 2. Hardware Variants & Ecosystem

| Variant | Form Factor / Core Concept | Key Features |
| :--- | :--- | :--- |
| **8Bit Mixtape Classic** | Standard Audio Cassette (FR4) | 2x Potentiometers in reel holes, 2x tactile buttons, mono audio out, DIP/SOIC ATtiny85. |
| **8Bit Mixtape NEO** | Audio Cassette with NeoPixels | Added 8 addressable WS2812B/SK6812 RGB LEDs, TinyAudioBoot sound bootloader, CV I/O. |
| **Micro Coconut NEO** | Ultra-minimal square/round PCB | Low-cost educational core ("One Coconut Per Child" - OCPC). |
| **DIMI-8** | Experimental handheld synth | Dedicated homage to Finnish avant-garde instrument designer **Erkki Kurenniemi** (DIMI series). |
| **8Bit NEO Euro-Rack** | Eurorack Modular (3U panel) | Brings ATtiny85 bytebeat algorithms, CV inputs, and gate clocks into modular synth racks. |
| **Gär Lämpli** | Bio-sensing visual/audio module | Born from Hackteria DIY bio-art camps to monitor fermentation activity via audio/light. |
| **MS-20 Filter Mixtape & ABK-20** | Desktop Wedge Console / Cassette | Merges the 8-bit digital synth core with an analog clone of the iconic **Korg MS-20 resonant state-variable VCF** (developed by Claude Winterberg & Christoph Stähli). Features 5x4 matrix, rotary knob, toggle switch, and 3.5mm jacks. |

### Sister ATtiny85 Projects & Community Constellation
* **Noisio (Steffen Koritsch, Dresden)**: The **ATtiny Punk Console (ATPC)** features 10 built-in algorithms, skin-resistance body contacts, tone low-pass filter, CV/gate, and smartphone audio programming. Also created the *Photonotron* (light-controlled optical theremin) and *Levitation OSC* (10-voice drone synth).
* **Jan Ostman / DSP Synthesizers (Sweden)**: Creator of the **Synsonique Drum** (capacitive-touch pocket drum machine) which officially adopted **TinyAudioBoot** for audio-jack flashing. Also designed 8-pin DSP chips emulating Solina strings, Minimoog, and 808 drums.
* **Standuino / Bastl Instruments (Brno)**: Václav Peloušek and Ondřej Merta's DIY micro-synth roots (*FraAngelico*, *2pi-whitenoise*, and *microGranny*), evolving into the open-source *Kastle* modular synth.
* **Auduino (Peter Knight / Tinker.it)**: Granular synthesis running two triangle-wave phase accumulators on 8-bit AVR.
* **SGMK 8Step Mixtape "Berliner Schule"**: Adam Berger's pocket 8-step sequencer driving vintage modular gear (featured on Matrixsynth).

---

## 3. Electrical Architecture & ATtiny85 Pin Multiplexing

The ATtiny85 offers only 8 physical pins (6 usable GPIOs). Squeezing a full synthesizer onto it requires dense multiplexing:

```
                  +---+--+---+
   RESET / PB5 -- | 1    8 | -- VCC (+3.3V to +5V)
    (A3) / PB3 -- | 2    7 | -- PB2 / ADC1 (Right Pot & CV In)
    (A2) / PB4 -- | 3    6 | -- PB1 / MISO / OC1A (PWM Audio Out)
           GND -- | 4    5 | -- PB0 / MOSI / AREF (NeoPixel Data)
                  +----------+
```

### Pin Allocation Matrix
* **PB0 (Pin 5)**: **NeoPixel LED Data**. High-speed 800 kHz single-wire protocol driving WS2812B / SK6812 arrays (8 LEDs on cassette, 20 LEDs on ABK-20 5x4 matrix).
* **PB1 (Pin 6)**: **Audio PWM Output**. Driven by high-speed Timer1 PLL (64 MHz carrier ~250 kHz). Filtered by an RC low-pass filter (`R8` = 100Ω/330Ω, `C2` = 100nF/1µF) to line-level / headphone output.
* **PB2 / ADC1 (Pin 7)**: **Right Control Potentiometer & CV Input**. 10kΩ linear pot tapped for pitch, speed, or filter parameter. Can take external 0–5V CV modulation.
* **PB3 / ADC3 (Pin 2)**: **Resistor Ladder Navigation Buttons**. Buttons (Left, Right, Mode) connect through a resistor voltage divider to a single analog input, producing discrete voltage steps for distinct button presses.
* **PB4 / ADC2 (Pin 3)**: **Left Control Potentiometer & Audio Bootloader Input**. Serves dual purpose: read as analog input for control during synth execution, or AC-coupled via `C1` (100nF) to receive FSK audio programming data.
* **PB5 / ADC0 (Pin 1)**: **Reset / Sync Output / Gate Clock**. Normally hardware reset. When `RSTDISBL` fuse is set, operates as GPIO for clock sync output (Korg Volca / Pocket Operator) or digital serial communication.

---

## 4. The Audio Programming Pipeline (TinyAudioBoot & Hex2Wav)

The hallmark feature of the 8Bit Mixtape NEO is eliminating dedicated hardware programmers (FTDI, USBasp, AVRISP):

### Workflow
1. **Compilation**: Arduino IDE or PlatformIO compiles sketch into `.hex`.
2. **Audio Conversion (`Hex2Wav` / `Hex2WavJS`)**:
   - Converts the Intel HEX binary into FSK audio tones (1200 / 2400 baud modulation).
   - Can be processed offline via CLI or online in-browser via Web Audio API.
3. **Transmission**:
   - Audio cable plugged from phone/laptop headphone jack into the 3.5mm input (`AudioProg`).
   - The user plays the generated `.wav` file into the tape.
4. **Bootloader Execution (`TinyAudioBoot`)**:
   - On power cycle, the bootloader listens on PB4 for incoming audio carrier pulses.
   - Decodes bytes, writes to flash memory page by page, and executes the user application once transmission completes.
5. **EEPROM Parameter Tuning**:
   - TinyAudioBoot supports write modes that directly update EEPROM parameters without reflashing the program flash.

> **CRITICAL RULE**: Never trigger PlatformIO automated uploads (`--target upload`) automatically. The physical power cycle must be manually coordinated with audio playback.

---

## 5. Clock Sync & Modular Interoperability

### Korg Volca Synchronization
* **Sync Out**: The Mixtape sends pulse triggers from PB5 / stereo ring.
* **Sync In**: Volca pulse output (4 pulses per beat) is read via pin-change interrupts, advancing sequencer steps or re-triggering bytebeat phase counters.
* **Cable Note**: Volca uses mono/tip connections. Custom splitters or inverted socket soldering route sync to the Volca-compatible conductor.

### Teenage Engineering Pocket Operator (PO) Sync
* PO expects audio on one channel (Left/Tip) and click sync on the other channel (Right/Ring).
* The Mixtape dual-channel output structure maps directly to PO sync modes (`SYNC 2` / `SYNC 4`).

---

## 6. PCB as Art Workflow (`svg2shenzhen`)

The 8Bit Mixtape community pioneered turning vector illustration into production circuit boards:
1. **Vector Artwork in Inkscape**: Drawings are organized into dedicated SVG layers matching PCB manufacturing masks:
   - `F.Cu` / `B.Cu` (Copper traces and solid art)
   - `F.Mask` / `B.Mask` (Soldermask openings exposing raw copper/gold)
   - `F.SilkS` / `B.SilkS` (Silkscreen graphics, labels, and borders)
   - `Edge.Cuts` (Milled board outlines and internal cutouts)
2. **`svg2shenzhen` Processing**: Inkscape extension converting SVG paths into native KiCad footprint modules (`.kicad_mod`) and board layers without quality loss.
3. **Direct-to-Fab**: Ready for immediate fabrication (Gerber export) for standard matte black, green, or red FR4 with HASL or ENIG gold finishes.

---

## 7. Firmware & Synthesis Engines

### Bytebeat Formula Archetypes
Bytebeats evaluate a single mathematical function of an incrementing integer `t` on every audio sample tick:
```c
// Classic Viznut One-Liner
output = (t * ((t >> 12 | t >> 8) & 63 & t >> 4));

// Bitwise Groove / Rhythmic Breakdown
output = (t * 5 & t >> 7) | (t * 3 & t >> 10);

// Melodic / Arpeggiated Canon
output = (t >> 7 | t | t >> 6) * 10 + 4 * (t & t >> 13 | t >> 6);
```

### Sound Generation Techniques on ATtiny85
* **64 MHz PLL PWM**: Timer1 clocked from high-speed PLL generates analog output carrier far above human hearing (>250 kHz), resulting in clear audio with a single capacitor filter.
* **Dual-Clock Interruption Handling**: NeoPixel data transmission demands strict cycle timing (~800 kHz) with zero interrupts. Audio interrupts are temporarily muted during `pixels.show()`, then re-enabled immediately to prevent audible pitch drift.
* **PROGMEM Lookups**: Wavetables, note frequencies, and visual LED patterns are stored in Flash memory to preserve the tiny 512-byte SRAM.
