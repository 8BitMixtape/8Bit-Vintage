# Makefile for TinyAudioBoot & TinyAudioBoot Skipper (ATtiny85)
# Compatible with 8Bit Mixtape and ABK-20 MS-20 synthesizer
# Uses avr-gcc toolchain (e.g. from PlatformIO or system)

F_CPU = 16000000
DEVICE = attiny85
BOOTLOADER_ADDRESS = 0x1BC0

# Path to avr-gcc: check ~/.platformio if not in system PATH
AVR_GCC := $(shell which avr-gcc 2>/dev/null || echo /home/dusjagr/.platformio/packages/toolchain-atmelavr/bin/avr-gcc)
AVR_OBJCOPY := $(shell which avr-objcopy 2>/dev/null || echo /home/dusjagr/.platformio/packages/toolchain-atmelavr/bin/avr-objcopy)
AVR_SIZE := $(shell which avr-size 2>/dev/null || echo /home/dusjagr/.platformio/packages/toolchain-atmelavr/bin/avr-size)

# Programmer defaults for avrdude
PROGRAMMER ?= avrisp
PROGRAMMER_PORT ?= /dev/ttyACM0
BAUD ?= 19200
AVRDUDE = avrdude -p $(DEVICE) -c $(PROGRAMMER) -P $(PROGRAMMER_PORT) -b $(BAUD)

# Standard TinyAudioBoot fuses (16MHz PLL, fast startup, BOD 2.7V, SPM enable)
LFUSE = 0xe1
HFUSE = 0xdd
EFUSE = 0xfe
FUSE_OPTS = -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

CFLAGS = -std=gnu99 -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) \
         -DBOOTLOADER_ADDRESS=$(BOOTLOADER_ADDRESS) \
         -ffunction-sections -fdata-sections -mrelax

LDFLAGS = -mmcu=$(DEVICE) -Wl,--gc-sections -Wl,--relax \
          -Wl,--section-start=.text=$(BOOTLOADER_ADDRESS) \
          -Wl,--section-start=.bootreset=0x0000 \
          -Wl,-u,resetVector

HEXDIR = ../hex

TARGETS = $(HEXDIR)/TinyAudioBoot_Standard_AudioPB3_LedPB1.hex \
          $(HEXDIR)/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex \
          $(HEXDIR)/TinyAudioBoot_Skipper_AudioPB3_LedPB0.hex

all: $(TARGETS)

$(HEXDIR)/TinyAudioBoot_Standard_AudioPB3_LedPB1.hex: TinyAudioBoot.c
	@mkdir -p $(HEXDIR)
	$(AVR_GCC) $(CFLAGS) -DLED_PIN=PB1 -c $< -o standard_pb1.o
	$(AVR_GCC) $(LDFLAGS) standard_pb1.o -o standard_pb1.elf
	$(AVR_OBJCOPY) -j .text -j .data -j .bootreset -O ihex standard_pb1.elf $@
	@echo "--- Size of Standard (LED PB1) ---"
	$(AVR_SIZE) standard_pb1.elf
	@rm -f standard_pb1.o standard_pb1.elf

$(HEXDIR)/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex: TinyAudioBoot_skipper.c
	@mkdir -p $(HEXDIR)
	$(AVR_GCC) $(CFLAGS) -DLED_PIN=PB1 -c $< -o skipper_pb1.o
	$(AVR_GCC) $(LDFLAGS) skipper_pb1.o -o skipper_pb1.elf
	$(AVR_OBJCOPY) -j .text -j .data -j .bootreset -O ihex skipper_pb1.elf $@
	@echo "--- Size of Skipper (LED PB1) ---"
	$(AVR_SIZE) skipper_pb1.elf
	@rm -f skipper_pb1.o skipper_pb1.elf

$(HEXDIR)/TinyAudioBoot_Skipper_AudioPB3_LedPB0.hex: TinyAudioBoot_skipper.c
	@mkdir -p $(HEXDIR)
	$(AVR_GCC) $(CFLAGS) -DLED_PIN=PB0 -c $< -o skipper_pb0.o
	$(AVR_GCC) $(LDFLAGS) skipper_pb0.o -o skipper_pb0.elf
	$(AVR_OBJCOPY) -j .text -j .data -j .bootreset -O ihex skipper_pb0.elf $@
	@echo "--- Size of Skipper (LED PB0) ---"
	$(AVR_SIZE) skipper_pb0.elf
	@rm -f skipper_pb0.o skipper_pb0.elf

fuse:
	$(AVRDUDE) $(FUSE_OPTS)

flash_skipper_pb1: $(HEXDIR)/TinyAudioBoot_Skipper_AudioPB3_LedPB1.hex
	$(AVRDUDE) -v -U flash:w:$<:i $(FUSE_OPTS)

flash_skipper_pb0: $(HEXDIR)/TinyAudioBoot_Skipper_AudioPB3_LedPB0.hex
	$(AVRDUDE) -v -U flash:w:$<:i $(FUSE_OPTS)

clean:
	rm -f *.o *.elf *.hex test.hex
