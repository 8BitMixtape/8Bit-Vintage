---
name: Antigravity Codebase Management
description: Comprehensive framework for generating 8-bit POV/Audio matrices, creating visuals, and managing the WAV pipeline on ATtiny85 hardware. Includes rules for iterative hardware testing before pipeline asset rendering.
---

# Antigravity ATtiny85 Project Context

This document provides essential context, architectural patterns, and workflow instructions for AI assistants (Claude, Gemini, Cascade) working on the Antigravity codebase.

---

## 1. Project Overview
**Antigravity** is a highly optimized audio/visual synthesizer running on the ATtiny85. It pushes the hardware to its absolute limits by generating 8-bit PWM audio (via 64MHz PLL) while simultaneously driving a 5x4 NeoPixel matrix.

### Hardware Specifications
*   **MCU**: ATtiny85 (8KB Flash, 512B SRAM, 512B EEPROM)
*   **Clock**: 16MHz Internal PLL (required for NeoPixel timing)
*   **Audio**: 8-bit PWM @ ~250kHz carrier (Timer1 PLL)
*   **Visuals**: 20 NeoPixels (WS2812B) in a 5x4 grid
*   **Input**: 
    *   2x Potentiometers (PB2, PB4)
    *   Multiple buttons on a single resistor ladder (PB3)

### Pin Mapping
| Pin | Function | Macro | Description |
| :--- | :--- | :--- | :--- |
| **PB0** | NeoPixel Data | `NEOPIXELPIN` | Timing-critical output (800kHz protocol) |
| **PB1** | Audio PWM | `SPEAKERPIN` | OC1A output (Timer1) |
| **PB2** | Analog In | `POTI_LEFT` / `ADC1` | Left Control Pot |
| **PB3** | Analog In | `BUTTON_PIN` | Resistor Ladder Buttons |
| **PB4** | Analog In | `POTI_RIGHT` / `ADC2` | Right Control Pot |
| **PB5** | Reset | - | ISP Programming / Reset |

---

## 2. Core Architectural Patterns

### The "NeoPixel Problem" (Dual-Clock Strategy)
**Constraint**: WS2812B LEDs require precise timing that cannot be interrupted. Audio generation usually requires interrupts.
**Solution**:
1.  **Silence Audio**: Disable interrupts before sending LED data.
2.  **Clock Boost**: Ensure CPU is at 16MHz (`clock_prescale_set(clock_div_1)`) for `pixels.show()`.
3.  **Restore State**: Re-enable interrupts and restore audio clock prescaler immediately after.
*Warning*: This results in a ~600µs silence per frame, which may cause audio glitches.

### High-Speed PWM Audio
**Constraint**: No DAC available.
**Solution**: Use Timer1's specialized 64MHz PLL mode.
```cpp
PLLCSR = 1 << PCKE | 1 << PLLE;       // Enable 64MHz PLL
TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10; // PWM on PB1
```
**Mixing**: `OCR1A = (voiceA + voiceB) >> 1;` (Avoid overflow).

### Memory Optimization
**Constraint**: 512 Bytes SRAM is extremely limited.
**Rules**:
1.  **PROGMEM**: ALL lookup tables (Sine, Notes, Gamma) must be in Flash. Access with `pgm_read_byte()`.
2.  **Avoid Floats**: Use `uint16_t` fixed-point math or pre-calculated tables.
3.  **Stack Safety**: Avoid deep recursion or large local arrays.

---

## 3. Creating Visuals

Visuals are implemented as C++ functions in `src/blinkeshit3000.cpp` (or included via `include/visuals.h`).

### Visual Function Signature
```cpp
void matrixEffect(uint16_t runtime_ms, uint16_t stepDelay_ms) {
    uint32_t start = millis();
    while (millis() - start < runtime_ms) {
        // 1. Calculate Frame
        // 2. Draw to Matrix
        // 3. Show & Wait
        pixels.show(); 
        delay(stepDelay_ms); // Use my_delay() on hardware if Timer0 is hijacked
    }
}
```

### Steps to Add a New Visual (Iterative Workflow)
Always follow this exact step-by-step iterative order to avoid overwriting assets prematurely:
1.  **Define & Implement**: Add function logic and prototype in `include/visuals.h`.
2.  **Test on Hardware First**: Add a hardcoded call at the bottom of the `loop()` function in `src/blinkeshit3000.cpp` (e.g., `matrixNewVisual(10000, 100);`). The user will flash and review this physically. Do NOT batch-generate WAVs or update the library yet!
3.  **Create Custom Preview GIF**: Run the emulator manually for just this specific visual. Ensure you pass a custom `--fps` that matches the actual hardware `stepDelay_ms` speed:
    `python3 scripts/run_emulator.py --vid <ID> --fps <MATCH_SPEED> --out wav/gifs/<ID>_<slug>.gif --cleanup`
4.  **Refine & Iterate**: Adjust colors/timing based on user feedback. Hardware NeoPixels behave differently than monitors (e.g., intense green-bleed on yellow tones, retina opponent-process afterimages). 
5.  **Register & Automate (ONLY ON REQUEST)**: When the user explicitly requests to update the web index and generate the final WAVs:
    *   Add the ID to the `switch(VISUAL_ID)` block in `src/blinkeshit3000.cpp`.
    *   Add the ID to the dispatch switch in `emu/main.cpp`.
    *   Add the ID and slug list to `scripts/gen_visual_gifs.py` and `scripts/gen_visual_wavs.py`.
    *   Run the targeted pipeline: `python3 scripts/gen_visual_wavs.py --only <ID> && python3 scripts/gen_wav_index.py`.

### Matrix Helpers (`include/matrix_helpers.h`)
*   `matrixIndex(x, y)`: Handles serpentine (zig-zag) mapping.
*   `matrixSet(x, y, color)`: Sets pixel at (x,y).
*   `matrixFade(decay)`: Dims all pixels by `decay` amount (0-255).
*   `dimColor(color, scale)`: Scales brightness of a packed RGB color.

---

## 4. Emulation & GIFs

Use the Native Emulator to preview visuals without hardware.

### Interactive Preview
Run the emulator locally (Linux/macOS):
```bash
pio run -e native
.pio/build/native/program
```
*   Set `VISUAL_ID` in `platformio.ini` under `[env:native]` to change the default visual.

### Generating GIFs
The project includes scripts to automate GIF creation from emulator frames.

**Single GIF**:
```bash
python3 scripts/run_emulator.py --vid <ID> --frames 200 --fps 12 --out preview.gif
```

**Batch Process (All Visuals)**:
```bash
python3 scripts/gen_visual_gifs.py --frames 100 --fps 10
```
*   Reads IDs from the internal list in `gen_visual_gifs.py`.
*   Outputs to `wav/gifs/`.

---

## 5. WAV Export & Deployment

The firmware is distributed as an **Audio Bootloader WAV file**.

### Automatic WAV Generation
The build system is hooked to automatically convert `.hex` to `.wav`.
```bash
pio run
# Output: .pio/build/attiny85/firmware.wav
```

### Batch WAV Generation
To generate WAVs for ALL visuals (for a library/website):
```bash
python3 scripts/gen_visual_wavs.py
```
*   Builds firmware for each Visual ID.
*   Generates variants for Brightness (Low, Med, High).
*   Outputs to `wav/`.

### Web Library Generation
To create an `index.html` gallery of all generated WAVs and GIFs:
```bash
python3 scripts/gen_wav_index.py
```
*   Scans `wav/` folder.
*   Matches WAVs with GIFs.
*   Generates `wav/index.html` and `wav/README.md`.

---

## 6. Development Tips

*   **Switching Main Logic**: The `platformio.ini` `build_src_filter` determines which `.cpp` file is the "Main" program.
    *   `+<vco_1voct.cpp>`: Synth Engine.
    *   `+<blinkeshit3000.cpp>`: Visuals Demo.
    *   `+<ATPC.cpp>`: Punk Console.
*   **Hex2Wav Configuration**: Settings for the audio encoder are in `platformio.ini` (e.g., `custom_hex2wav_cmd`).
*   **Debugging**: Since there is no Serial port on the ATtiny85 (unless bit-banged), use the LED Matrix for status or the Emulator for logic debugging.
