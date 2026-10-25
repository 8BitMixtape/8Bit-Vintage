# Burn TinyAudioBoot Skipper onto ATtiny85

### Using Arduino as ISP (`/dev/ttyACM0`):
```bash
avrdude -v -p attiny85 -c avrisp -P /dev/ttyACM0 -b 19200 \
  -U flash:w:hex/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex:i \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

### Using USBasp:
```bash
avrdude -v -p attiny85 -c usbasp \
  -U flash:w:hex/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex:i \
  -U efuse:w:0xfe:m -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m
```

> **Usage**:
> * Power on normally $\to$ Boots sketch in $<1\text{ ms}$.
> * Power on while holding **SW2** $\to$ Enters audio bootloader to flash new firmware via audio jack.
