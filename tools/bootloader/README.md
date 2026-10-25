# TinyAudioBoot for ATtiny85 (8Bit Mixtape & ABK-20)

**TinyAudioBoot** is an ultra-compact ($\approx 1\text{ KB}$) audio bootloader for the Microchip ATtiny85 microcontroller, created by **Chris Micro**, **Budi Prakosa (Iyok)**, and **Fredrik Olofsson**.

It allows flashing new sound sketches directly through a 3.5mm audio jack or web browser via `.wav` audio streams—**no USB-to-Serial converter or dedicated hardware programmer required**.

---

## 1. Directory Structure

```
tools/bootloader/
├── README.md                          # This documentation
├── burn_bootloader.md                 # Quick one-liner commands for avrdude
├── src/
│   ├── TinyAudioBoot_skipper.c        # Fast-booting Skipper bootloader (recommended for ABK-20)
│   ├── TinyAudioBoot.c                # Standard TinyAudioBoot C source (with timeout wait)
│   ├── EEPROM.h                       # EEPROM routines
│   └── Makefile                       # AVR-GCC build script (targets ATtiny85)
└── hex/
    ├── TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex  # Skipper build (PB3 Audio/SW2, Status LED PB1) [RECOMMENDED]
    ├── TinyAudioBoot_Skipper_AudioPB3_LedPB0.hex  # Skipper build (PB3 Audio/SW2, Status LED PB0)
    ├── TinyAudioBoot_AudioPB3_LedPB1.hex          # Standard build (Audio=PB3, Status LED=PB1)
    ├── TinyAudioBoot_AudioPB3_LedPB0.hex          # Standard build (Audio=PB3, Status LED=PB0)
    └── AudioBootAttiny85_AudioPB3_LEDPB1.hex      # Legacy 0x1C00 build
```

---

## 2. Fast-Booting "Skipper" Mode (ABK-20 Feature)

Standard TinyAudioBoot blinks and waits several seconds for an audio transmission every time power is applied before launching the user synth sketch.

The **Skipper** version eliminates this waiting time:
* **Normal Power-On**: Immediately launches the user synth sketch in $< 1\text{ ms}$—no delay, no startup blinking.
* **Audio Upload**: Hold down button **`SW2`** (the button with the lowest voltage on the `PB3` ladder) while powering on. The bootloader stays active, blinks the status LED, and waits for the audio WAV stream.

### ABK-20 Resistor Ladder Thresholds (8-bit ADC on `PB3` / ADC3):

| Control State | Voltage on PB3 | 8-bit `ADCH` (0–255) | Bootloader Action |
| :--- | :---: | :---: | :--- |
| **Idle** (no buttons) | $\approx 2.6\text{ V}$ | $\approx 134$ | **Skip** $\to$ Run synth immediately |
| **Toggle SW1 DOWN** | $\approx 1.75\text{ V}$ | $\approx 89$ | **Skip** $\to$ Run synth immediately |
| **Toggle SW1 UP** | $\approx 1.35\text{ V}$ | $\approx 69$ | **Skip** $\to$ Run synth immediately |
| **Button SW2 Solo** | $\approx \mathbf{1.0\text{ V}}$ | $\approx \mathbf{51}$ | **ENTER BOOTLOADER** |
| **Button SW2 + SW1** | $\approx \mathbf{0.7\text{--}0.8\text{ V}}$ | $\approx \mathbf{36\text{--}42}$ | **ENTER BOOTLOADER** |

> Threshold `ADCH <= 60` reliably isolates button `SW2` from all toggle switch combinations and input audio levels.

---

## 3. Hardware Pin Assignment

| Function | ATtiny85 Pin | Physical Leg | Description |
| :--- | :---: | :---: | :--- |
| **Audio In / SW2** | `PB3` | Pin 2 | AC-coupled audio input (100nF) biased at $V_{CC}/2$ and button ladder tap. |
| **Status LED** | `PB1` or `PB0` | Pin 6 / Pin 5 | Toggles on every received packet to provide visual flashing feedback. |

> [!NOTE]
> On the **ABK-20**, the KS01 push-button bi-directional LED is wired **between `PB1` and `PB0`**. Therefore, the button LED flashes during audio uploads with **either** bootloader build (PB1 drives Red/Green, PB0 drives the opposite)!

---

## 4. Flash Memory Budget & Addressing

* **Bootloader Address**: `0x1BC0` (byte address $7,104$).
* **Total ATtiny85 Flash**: $8,192$ bytes.
* **Skipper Bootloader Size**: Only $838$ bytes!
* **User Application Space**: $7,104$ bytes ($0\text{x0000} \to 0\text{x1BBF}$).
* **Safe Flash Budget for Sketches**: $\le 6,300\text{ bytes}$ (leaves headroom for reset vector interception).

---

## 5. ATtiny85 Fuse Configuration (16 MHz Internal PLL)

To run the bootloader and sound engine at $16\text{ MHz}$ using the internal high-speed 64 MHz PLL, burn the following fuses:

| Fuse | Value | Description |
| :--- | :---: | :--- |
| **Low Fuse** | `0xE1` | Internal PLL at 64 MHz ($\to 16\text{ MHz}$ system clock), `CKDIV8` disabled. |
| **High Fuse** | `0xDD` | `SELFPROGEN` enabled (allows bootloader to write to Flash), Brown-out Detection (BOD) at $2.7\text{V}$. |
| **Extended Fuse** | `0xFE` | Self-programming target page protection enabled. |

---

## 6. How to Flash the Bootloader via AVRDUDE

Connect your ISP programmer (e.g. Arduino as ISP on `/dev/ttyACM0` or USBasp) to the ATtiny85 SPI pins (`MOSI=PB0`, `MISO=PB1`, `SCK=PB2`, `RESET=PB5`, `VCC`, `GND`):

### One-Command Flash (Skipper + Fuses via ArduinoISP):
```bash
avrdude -v -p attiny85 -c avrisp -P /dev/ttyACM0 -b 19200 \
  -U flash:w:hex/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex:i \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

### One-Command Flash (Skipper + Fuses via USBasp):
```bash
avrdude -v -p attiny85 -c usbasp \
  -U flash:w:hex/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex:i \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

---

## 7. How to Recompile from Source

```bash
cd tools/bootloader/src
make clean
make
```

This compiles both:
* `tools/bootloader/hex/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex`
* `tools/bootloader/hex/TinyAudioBoot_Skipper_AudioPB3_LedPB0.hex`

---

## 8. Lineage & Credits
* **Chris Haberer (ChrisMicro)**: Original AudioBoot concept & Skipper branch implementation.
* **Budi Prakosa (Iyok / ATtinyTeenageRiot)**: ATtiny85 port, Manchester decoding, & Hex2Wav tool.
* **Fredrik Olofsson**: Web Hex2Wav JavaScript parser.
* **Dusjagr & Antigravity**: Voltage ladder integration and Skipper optimization for ABK-20.
