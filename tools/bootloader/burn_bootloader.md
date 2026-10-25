# Burning TinyAudioBoot onto ATtiny85

> **Important**: Always use the two-stage command (`&&` between flash and fuses). 
> When programming with ArduinoISP (`avrisp`), writing flash and fuses in a single command can glitch the clock switch before the session cleanly terminates.

---

### 1. Standard Bootloader (Always waits ~1.5s for audio at startup)
```bash
avrdude -v -p attiny85 -c avrisp -P /dev/ttyACM0 -b 19200 \
  -U flash:w:tools/bootloader/hex/AudioBootAttiny85_AudioPB3_LEDPB1.hex:i && \
avrdude -P /dev/ttyACM0 -b 19200 -c avrisp -p t85 \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

---

### 2. Skipper Bootloader (Instant Boot, enters bootloader ONLY when holding button)

#### Variant A: Status LED / Blinking on PB1 (Analog skip on PB3)
```bash
avrdude -v -p attiny85 -c avrisp -P /dev/ttyACM0 -b 19200 \
  -U flash:w:tools/bootloader/hex/AudioBootAttiny_AudioPB3_PB1_AnalogSkipPB3.hex:i && \
avrdude -P /dev/ttyACM0 -b 19200 -c avrisp -p t85 \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

#### Variant B: Status LED / Blinking on PB0 (Audio out PB1 silent)
```bash
avrdude -v -p attiny85 -c avrisp -P /dev/ttyACM0 -b 19200 \
  -U flash:w:tools/bootloader/hex/AudioBootAttiny_AudioPB3_PB0_skipper.hex:i && \
avrdude -P /dev/ttyACM0 -b 19200 -c avrisp -p t85 \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

---

### Fuse Values Reference (16MHz Internal PLL)
* **lfuse**: `0xe1` (16 MHz PLL system clock, 64 MHz PCK, fast startup)
* **hfuse**: `0xdd` (SPI enabled, BOD 2.7V, RESET enabled)
* **efuse**: `0xfe` (Self-programming enabled for bootloader SPM)
