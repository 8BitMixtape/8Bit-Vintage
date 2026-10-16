# The History, Architecture & Constellation of the 8Bit Mixtape & Coconut Ecosystem

> *"Offene Do-It-Yourself-Arbeitswoche für geniale Experimentierer: The art of 8Bit sounds on microcontrollers!"*  
> — SGMK HomeMade, 2007

---

## Executive Summary

The **8Bit Mixtape** is an open-source, lo-fi algorithmic synthesizer, sonic sketchbook, and community pedagogy platform anchored around the Atmel/Microchip **ATtiny85** 8-bit microcontroller. Emerging from an international constellation of DIY electronics societies, South Asian hacker residencies, and Southeast Asian open-culture collectives, the project redefines microcontrollers as accessible folk instruments.

This compendium charts:
1. **The Cultural Lineage & Coconut Lore**: From alpine SGMK camps in Switzerland to the Center for Alternative Coconut Research in Goa, and the fertile bytebeat jam sessions of Yogyakarta.
2. **The SGMK Archive**: The early BOM, the "Berliner Schule" 8-step sequencer, and the cartridge/video experiments at PechBlenda Lab.
3. **The Constellation of Sister ATtiny Projects**: How the 8Bit Mixtape cross-pollinates with **Noisio's ATtiny Punk Console**, **Jan Ostman's Synsonique Drum**, **Bastl Instruments / Standuino**, **Auduino**, and **Illutron**.
4. **The Technical Feats**: Extreme 8-pin multiplexing, 64 MHz PLL PWM audio synthesis, and the game-changing **TinyAudioBoot** sound bootloader.
5. **The Lineage to the ABK-20**: The evolutionary arc culminating in the hybrid digital/analog Korg MS-20 resonant desktop synth.

---

## 1. Deep History & The Coconut Lineage

```
2007            2011            2012             2013                  2015            2016-2018         2019+
SGMK            CACR, Goa       BabyGnusbuino    Yogyakarta            Bangalore       NEO & TinyAudioBoot  ABK-20
HomeMade Camps  "Wave-to-MIDI   (Michi Egger,    Lifepatch & HONF      CocoMake7       Cassettes, EuroRack,  MS-20 Filter
(Switzerland)    Coconut"        a.n.y.m.a.)      Viznut Bytebeats      (Hackteria)     Synsonique Drum,      Desktop Wedge
                                                 First Cassette Tape                   Noisio & Global Labs   Console
```

### 1.1 SGMK & HomeMade SummerCamps (2007–2011)
The philosophical foundation originated within the **Schweizerische Gesellschaft für Mechatronische Kunst (SGMK)**. At annual HomeMade SummerCamps held in remote Swiss valleys, artists, tinkerers, and engineers met for unstructured do-it-yourself research weeks:
* Early workshops explored 8-bit sound generation on Atmel microcontrollers, heavily inspired by early AVR audio pioneers like Christoph Haberer (*atmegaSID*, Roboterclub Freiburg).
* The core ethos was established: direct manipulation of silicon timers, hands-on etching, scavenging discarded electronic waste, and demystifying digital signal processing.

### 1.2 The Center for Alternative Coconut Research (Goa, 2011)
During an art-and-science residency in Goa, India, the **Center for Alternative Coconut Research (CACR)** was founded by dusjagr (Marc Dusseiller) and cohorts. The inaugural project addressed a delightfully surreal brief:
> *"We really needed to put a GNUSB into a Coconut for a 'Wave-to-MIDI' converter."*

The coconut became a lasting emblem of tropical DIY bio-hacking, open-source hardware independence, and low-cost nomad science.

### 1.3 The BabyGnusbuino (2012)
In 2012, Michael Egger (*a.n.y.m.a.*) introduced the **BabyGnusbuino** (BabyGnusbuino-v2) at the SGMK DIY-Umtrunk in Zurich. Packaging an ATtiny85 with a direct PCB USB tongue and Arduino compatibility, it proved that an ultra-cheap 8-pin microchip could handle complex interactive tasks and serve as a versatile musical generator.

### 1.4 Yogyakarta: The Bytebeat Epiphany (2013)
In 2013, dusjagr traveled to Yogyakarta, Indonesia, collaborating with the pioneering citizen-science and media-art communities **Lifepatch** and **HONF (House of Natural Fiber)**:
* During workshops with Budi Prakosa (*manticore*), Andreas Siagian, and local hackers, they encountered Finnish demoscener **Ville-Matias Heikkilä's (Viznut)** 2011 discovery: **Algorithmic Bytebeats**.
* Viznut demonstrated that a single line of C code evaluating bitwise operators against an incrementing time variable `t` could generate polyphonic counterpoints, percussion, and evolving basslines without tables or memory buffers.
* Realizing the ATtiny85 was ideally suited to compute these formulas at an 8 kHz clock rate, they crafted the circuit into the shape and dimensions of an **audio compact cassette**, placing the control potentiometers directly inside the cassette tape reel holes.
* Thus, the **8Bit Mixtape** emerged: a cassette that contained its own synthesizer.

### 1.5 Bangalore & Kathmandu (2015–2016)
* **CocoMake7 (Bangalore, 2015)**: Collaboration between Hackteria, Srishti Institute of Art, Design and Technology, and local artists to develop ultra-accessible paper circuit boards and wearable sound modules.
* **K_Space / CocoSat (Kathmandu, 2016)**: The Kathmandu Space Programme, exploring micro-satellite telemetry and sensory electronics humorously framed as *"How to put a coconut satellite in low earth orbit?"*.

---

## 2. Archival Gems from the SGMK Wiki

The early SGMK project notes document the raw beginnings of the Mixtape:

### 2.1 The Original 2013 BOM
* 1× ATtiny85 (or ATtiny45 / ATtiny84)
* 1× Potentiometer 20kΩ linear (Pitch / Rate)
* 1× Potentiometer 10kΩ logarithmic (Modulation)
* 1× Tactile pushbutton (flat breadboard type for algorithm cycling)
* 1× Switched 3.5mm mini-jack (mono in v0.2, later stereo TRS)
* 2× CR2032 3V coin cells in series with spring clips
* 1× Mini-speaker or piezo element
* 1× Mini-USB socket (or scavenged stripped USB cable)
* *(Optional)* 1× Magnetic tape read/write head for direct tape emulation!

### 2.2 The "Berliner Schule" 8-Step Sequencer
Adapted by SGMK from Adam Berger's pocket sequencer, the 8-step sequencer variant expanded the Mixtape concept:
* Configured an **ATtiny84** to output analog CV and clock gate triggers.
* Featured in *Matrixsynth* (2015) driving vintage modular monsters like the **Roland System 100**.

### 2.3 The "Advanced 2000" & PechBlenda VideoOut Editions
* **8Bit MixTape Pro Advanced 2000**: Upgraded with a Seeeduino ATMega328 and Nokia 5110 LCD screen inside a Game Boy-style cartridge.
* **VideoOut Edition**: Experiments at **PechBlenda Lab** by Paula Pin and collaborators, generating composite glitch video patterns directly from the microchip's PWM stream.

---

## 3. The Constellation of Sister ATtiny85 Synth Projects

The 8Bit Mixtape does not exist in isolation; it belongs to a vibrant international movement exploring the limits of 8-pin microcontrollers:

```
                                [ATtiny85 Silicon]
                                        |
       +-------------------+------------+------------+-------------------+
       |                   |                         |                   |
[8Bit Mixtape]    [Noisio ATPC]             [Synsonique Drum]     [Standuino / Bastl]
(Yogya/Zurich)    (Dresden, Germany)        (Jan Ostman, Sweden)  (Brno, Czech Rep.)
AudioBoot Tapes,  10 Algorithms, Body-      Vintage Drum Remake,  microGranny roots,
Bytebeat Engines  Contacts, Dual Jacks      Uses TinyAudioBoot    Kastle pocket modular
```

### 3.1 Noisio & The ATtiny Punk Console (ATPC)
Created by electronic artist and musician **Steffen Koritsch (Noisio)** in Dresden, Germany:
* **ATtiny Punk Console (ATPC)**: A digital reimagining of Forrest Mims III's classic 555-timer *Atari Punk Console*.
* **10 Switchable Algorithms**: Includes pure sines, 8-bit drums, chaotic drones, and harsh noise generators.
* **Body Contacts**: Exposed PCB copper pads allow the performer's skin resistance to warp pitch and modulation directly.
* **Audio-Flashing Synergy**: Like the Mixtape, modern ATPC revisions allow firmware updating and patch loading via smartphone audio.
* **Other Noisio Micro-Synths**:
  * *Photonotron*: Light-controlled dual-voice optical theremin.
  * *Levitation OSC*: 10-voice hypnotic drone synth.
  * *AT LFO / ATKeys*: Micro-modular control voltage and keyboard blocks.

### 3.2 Jan Ostman & The Synsonique Drum (DSP Synthesizers)
Swedish DSP engineer **Jan Ostman** revolutionized minimalist sound design with his *DSP Synthesizers* chips:
* **Synsonique Drum**: A modern, credit-card-sized (92 × 76 mm) reimagining of the Mattel *Synsonics* electronic drums (1981).
* **Capacitive Touch Pads**: Replaced fragile mechanical triggers with capacitive touch zones directly on the PCB.
* **TinyAudioBoot Adoption**: Ostman officially adopted the 8Bit Mixtape's **TinyAudioBoot** into the Synsonique Drum, enabling users to re-flash drum sounds, rhythmic fills, and firmware using audio WAV files from a smartphone or tape deck.

### 3.3 Bastl Instruments & Standuino (Brno, Czech Republic)
Before founding Bastl Instruments, Václav Peloušek and Ondřej Merta developed **Standuino**:
* *FraAngelico* and *2pi-whitenoise*: Early handmade wooden digital synthesizers pushing 8-bit AVR microcontrollers into granular synthesis and bytebeat territory.
* *Bastl Kastle*: Directly inherits this lineage as an open-source, pocket-sized modular algorithmic synthesizer.

### 3.4 Auduino & Illutron
* **Auduino (Peter Knight / Tinker.it)**: Classic granular synthesis running two triangle-wave phase accumulators and a resonant filter emulation.
* **Illutron (Mads & Ditte Hobye, Copenhagen)**: 4-voice real-time wavetable synthesis on AVR chips with phase modulation.

---

## 4. Hardware Evolution & The ABK-20 Synthesis

| Variant | Form Factor | Visual Interface | Audio Engine | Key Contribution |
| :--- | :--- | :--- | :--- | :--- |
| **8Bit Mixtape Classic** | Audio Cassette PCB | 2× 5mm LEDs | Bytebeat fast PWM (`TCCR0B`) | Established cassette format with pots in reel holes. |
| **8Bit Mixtape NEO** | Audio Cassette PCB | 8× WS2812B RGB LEDs | 64 MHz Timer1 PLL PWM | Added **TinyAudioBoot** sound flashing & RGB feedback. |
| **Micro Coconut NEO** | Round/Square PCB | Single RGB NeoPixel | Minimalist RC PWM | "One Coconut Per Child" (OCPC) educational tool. |
| **DIMI-8** | Handheld Console | Multi-LED Matrix | Algorithmic logic synthesis | Homage to Finnish synthesizer visionary **Erkki Kurenniemi**. |
| **8Bit NEO Euro-Rack** | 3U Eurorack Module | LED Bar / Indicators | Buffered ±5V CV/Audio | Bridges ATtiny bytebeats into Eurorack modular rigs. |
| **Gär Lämpli** | Biological Sensor Lamp | High-intensity RGB LED | Metabolic audio chirps | Real-time monitoring of yeast/bacterial fermentation. |
| **ABK-20 (MS-20 Synth)** | **90 × 90 mm Wedge Console** | **5×4 NeoPixel Grid (20 LEDs)** | **ATtiny85 + Analog Korg MS-20 Resonant VCF** | **Hybrid digital/analog desktop synth with USB-C and wood cheeks.** |

---

## 5. Technical Architecture & Engineering Feats

### 5.1 Extreme Pin Multiplexing on ATtiny85
To achieve a full synthesizer interface on an 8-pin package:

```
                          ATtiny85 Pinout
                            +---+--+---+
     (RESET / PCINT5)  PB5 -| 1    8 |- VCC (+3.3V to +5.0V)
     (ADC3 / PCINT3)   PB3 -| 2    7 |- PB2 (ADC1 / SCK / CV In / Right Pot)
     (ADC2 / PCINT4)   PB4 -| 3    6 |- PB1 (MISO / OC1A / PWM Audio Out)
                       GND -| 4    5 |- PB0 (MOSI / AREF / NeoPixel Data)
                            +----------+
```

1. **PB0 (Pin 5)**: High-speed 800 kHz single-wire stream driving NeoPixels (20 WS2812B/SK6812 LEDs in a 5×4 matrix on ABK-20).
2. **PB1 (Pin 6)**: Non-inverting PWM output from Timer1 running on the **64 MHz asynchronous PLL** (~250 kHz carrier), eliminating audible switching whine with a single 100Ω + 100nF filter.
3. **PB2 / ADC1 (Pin 7)**: Analog wiper of the right 10kΩ linear potentiometer, paralleled with external Control Voltage (0–5V CV) input.
4. **PB3 / ADC3 (Pin 2)**: Analog resistor ladder sensing multiple buttons (Left, Right, Mode) through discrete voltage thresholds.
5. **PB4 / ADC2 (Pin 3)**: Dual-purpose input — read as DC analog voltage from the left potentiometer during play, or AC-coupled via 100nF capacitor to decode FSK audio bootloader tones.
6. **PB5 (Pin 1)**: Hardware reset; when `RSTDISBL` fuse is programmed, serves as pulse clock output for Korg Volca and Teenage Engineering Pocket Operator sync.

### 5.2 The TinyAudioBoot Protocol
Developed by Budi Prakosa (*manticore*) with Chris Micro and Fredrik Olofsson:
* **Baud Rate**: ~1200–2400 baud FSK modulation.
* **Carrier Tones**: 1200 Hz (Space) and 2400 Hz (Mark).
* **Zero Programmer Hardware**: The user plugs a standard 3.5mm aux cable from a phone, laptop, or Walkman into the synth's jack and plays a `.wav` file.
* **EEPROM Direct Writing**: A specialized packet type writes variables into EEPROM without touching flash memory, allowing custom scales, presets, and calibration.

### 5.3 PCB Art as Medium (`svg2shenzhen`)
Developed by badgeek and dusjagr, `svg2shenzhen` connects graphic design with circuit fabrication:
* Inkscape SVG layers (`F.Cu`, `B.Cu`, `F.Mask`, `B.Mask`, `F.SilkS`, `Edge.Cuts`) are exported directly into KiCad footprint files (`.kicad_mod`).
* Enables exposed copper/gold graphics, negative soldermask texturing, and custom artistic silhouettes without relying on rigid geometric CAD primitives.

---

## 6. The ABK-20 Milestone

The **ABK-20** combines this rich lineage into a mature desktop performance instrument:

1. **Analog/Digital Fusion**: Pairs the ATtiny85 algorithmic sound engine with an authentic analog recreation of the **Korg MS-20 resonant low-pass filter** (developed by Claude Winterberg and Christoph Stähli).
2. **Desktop Wedge Architecture**: Breaks out of the flat cassette profile into an angled **90 × 90 mm desktop console** nestled between laser-cut wooden cheek panels.
3. **5×4 Visual Matrix**: A 20-cell RGB LED display providing real-time audio visualization, step sequencer tracking, and parameter feedback.
4. **USB-PD Universal Power**: Modern 6-pin USB-C receptacle with dual 5.1kΩ pull-downs, ensuring seamless power from modern USB-C chargers and MacBooks.

---

## 7. Key References & Archival Repositories

* **SGMK 8Bit Mixtape Wiki**: [wiki.sgmk-ssam.ch/wiki/8bit_Mix_Tape](https://wiki.sgmk-ssam.ch/wiki/8bit_Mix_Tape)
* **8Bit Mixtape NEO Wiki**: [github.com/8BitMixtape/8Bit-Mixtape-NEO/wiki](https://github.com/8BitMixtape/8Bit-Mixtape-NEO/wiki)
* **TinyAudioBoot Repository**: [github.com/ATtinyTeenageRiot/TinyAudioBoot](https://github.com/ATtinyTeenageRiot/TinyAudioBoot)
* **Noisio Experimental Instruments**: [noisio.de](https://noisio.de/)
* **Jan Ostman / DSP Synthesizers (Synsonique Drum)**: [matrixsynth.com/2017/07/synsonique-drum-atmel-attiny85.html](https://www.matrixsynth.com/2017/07/synsonique-drum-atmel-attiny85.html)
* **svg2shenzhen Toolchain**: [github.com/badgeek/svg2shenzhen](https://github.com/badgeek/svg2shenzhen)
* **Center for Alternative Coconut Research**: [cocomake7.github.io](https://cocomake7.github.io/)
* **Hackteria Global Network**: [hackteria.org](https://hackteria.org/)
