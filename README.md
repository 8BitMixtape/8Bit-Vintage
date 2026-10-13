# 8Bit-Vintage

Sound applications for the **8Bit Mixtape** (ATtiny85), uploaded via audio
using the AudioBoot bootloader.

## Sound Apps Catalog

| Sketch | Description | Docs |
|---|---|---|
| `drone_synth.cpp` | Dual-oscillator drone synthesizer with generative plasma visuals | [Manual](docs/DRONE_SYNTH_MANUAL.md) |
| `peepsy_acid.cpp` | TB-303 style acid bassline & drum sequencer | - |
| `flappy_slider.cpp` | Interactive obstacle game with chiptune sound effects | - |
| `bass_patterns.cpp` | Punchy 808 sub-bass, kicks, snares & rhythm generator | - |
| `minimal_techno.cpp` | Hypnotic rolling basslines & tight minimal techno groove | - |
| `deep_minimal.cpp` | Sub-heavy plucks & stripped-down micro-grooves | - |
| `quantum_matrix_new.cpp` | Quantum DJ: Granular 1-bit bytebeat with chaotic NeoPixel sparks | - |
| `ave_maria_siberia.cpp` | Ave Maria (Siberia): Liturgical chant, deep pedal drone & Soviet Star | - |
| `lambada.cpp` | Kaoma's Lambada ("Chorando se foi"): Tropical accordion, Carimbó bass & Zouk groove | [Manual](docs/LAMBADA_MANUAL.md) |
| `axel_foley.cpp` | Axel Foley ("Axel F"): Harold Faltermeyer 80s synth classic, Moog bass & LinnDrum | [Manual](docs/AXEL_FOLEY_MANUAL.md) |
| `auf_der_heide.cpp` | Auf der Heide ("Erika"): Vintage German brass march, Oom-Pah tuba & the 3 stomps | [Manual](docs/AUF_DER_HEIDE_MANUAL.md) |
| `zelda_sid.cpp` | Legend of Zelda (Overworld Theme): 3-voice SID sound engine, dynamic PWM & fast arps | [Manual](docs/ZELDA_SID_MANUAL.md) |

## Web Audio Player & Flasher

Pre-listen to each synth and flash your 8Bit Mixtape straight from your browser:
**https://8bitmixtape.github.io/8Bit-Vintage/**

## Hardware

- ATtiny85 @ 16 MHz (internal PLL)
- 8-bit PWM audio output
- WS2812B NeoPixels

## Build & Upload

1. Pick the sketch in `platformio.ini` under `build_src_filter`.
2. Build: `pio run` — produces `.pio/build/attiny85/firmware.hex` and `.wav`.
3. Upload: power-cycle the ATtiny into the AudioBoot bootloader, then run
   `pio run --target upload`. This plays the WAV through your audio output.

The hex→wav converter is set per OS via `custom_hex2wav_cmd` in
`platformio.ini` (Linux 64-bit by default; macOS, Windows and Java builds are
in `tools/hex2wav/`).

## Layout

```
src/       sound app sketches (one main file selected at a time)
include/   shared headers
lib/       vendored 8Bit-Mixtape-NEO-Lib
scripts/   hex2wav post-build script & WAV/preview generators
tools/     hex2wav converters
docs/      web player (GitHub Pages), audio demos, GIFs & manuals
```
