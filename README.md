# 8Bit-Vintage

Sound applications for the **8Bit Mixtape** (ATtiny85), uploaded via audio
using the AudioBoot bootloader.

## Sound apps

| Sketch | Description | Docs |
|---|---|---|
| `drone_synth.cpp` | Dual-oscillator drone synthesizer with generative plasma visuals | [Manual](docs/DRONE_SYNTH_MANUAL.md) |
| `peepsy_acid.cpp` | TB-303 style acid bassline & drum sequencer | - |
| `flappy_slider.cpp` | Interactive obstacle game with chiptune sound effects | - |
| `bass_patterns.cpp` | Punchy 808 sub-bass, kicks, snares & rhythm generator | - |

## Web Audio Player & Flasher

Flash your 8Bit Mixtape straight from your browser:
**https://8bitmixtape.github.io/8Bit-Vintage/**

## Hardware

- ATtiny85 @ 16 MHz (internal PLL)
- 8-bit PWM audio output
- WS2812B NeoPixels

## Build & upload

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
docs/      web player (GitHub Pages) & manuals
```
