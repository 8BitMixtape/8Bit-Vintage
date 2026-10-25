/*
  TinyAudioBoot Skipper — Fast-booting Audio Bootloader for ATtiny85
  Customized for ABK-20 and 8Bit Mixtape

  Origins:
  - AudioBoot by C. Haberer
  - TinyAudioBoot rework for ATtiny85 by Budi Prakosa (Iyok)
  - Skipper branch by ChrisMicro (bypasses bootloader unless button pressed)
  - Voltage-ladder button threshold integration for ABK-20 by Dusjagr & Antigravity

  Hardware: ATtiny85 @ 16MHz PLL
  - Input: PB3 (ADC3) with 100nF audio AC coupling and resistor divider ladder.
  - Button SW2 on PB3: Lowest voltage tap on ladder (~51 / 255 = ~1.0V).
    - If SW2 is NOT pressed at power-on (ADCH > 60): Exits immediately to user sketch (<1ms).
    - If SW2 IS pressed at power-on (ADCH <= 60): Stays in bootloader for audio flashing.
  - Status LED: PB1 (default) or PB0 (configurable via -DLED_PIN).

  Fuses:
  - lfuse: 0xe1  (PLL 16MHz, fast startup)
  - hfuse: 0xdd  (SPI enabled, BOD 2.7V, reset enabled)
  - efuse: 0xfe  (Self-Programming enabled)
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define BOOT_TIMEOUT 120
#define WAITBLINKTIME 1500

#ifndef BOOTLOADER_ADDRESS
#define BOOTLOADER_ADDRESS 0x1C00
#endif

#define RJMP (0xC000U - 1)
#define RESET_SECTION __attribute__((section(".bootreset"))) __attribute__((used))

uint16_t resetVector RESET_SECTION = RJMP + BOOTLOADER_ADDRESS / 2;

// LED Configuration
// Multi-color LED configuration for ABK-20 KS01 bi-directional LED (PB1 <-> PB0)
#define INITLED do { \
    DDRB |= (1 << PB0) | (1 << PB1); \
    PORTB &= ~((1 << PB0) | (1 << PB1)); \
} while(0)

#define LED_GREEN()    do { PORTB = (PORTB & ~(1 << PB0)) | (1 << PB1); } while(0)
#define LED_RED()      do { PORTB = (PORTB & ~(1 << PB1)) | (1 << PB0); } while(0)
#define LED_OFF()      do { PORTB &= ~((1 << PB0) | (1 << PB1)); } while(0)
#define TOGGLE_GREEN() do { PORTB &= ~(1 << PB0); PORTB ^= (1 << PB1); } while(0)
#define TOGGLE_RED()   do { PORTB &= ~(1 << PB1); PORTB ^= (1 << PB0); } while(0)

#define LEDON      LED_RED()
#define LEDOFF     LED_OFF()
#define TOGGLELED  TOGGLE_RED()

#define INITDEBUGPIN
#define DEBUGPINON
#define DEBUGPINOFF
#define TOGGLEDEBUGPIN

// Audio Input Configuration
#define INPUTAUDIOPIN (1 << PB3)
#define PINVALUE (PINB & INPUTAUDIOPIN)
#define INITAUDIOPORT { DDRB &= ~INPUTAUDIOPIN; }
#define PINLOW (PINVALUE == 0)
#define PINHIGH (!PINLOW)

#define TIMER TCNT0

#define STARTBYTE 0x01
#define STOPBYTE  0x04

#define FRAMELENGTH 71
#define DATAPAGESIZE 64

#define COMMAND         0
#define PAGEINDEXHIGH   1
#define PAGEINDEXLOW    2
#define LENGTHHIGH      3
#define LENGTHLOW       4
#define CHECKSUM        5
#define DATAPAGESTART   6

#define NOCOMMAND       0
#define TESTCOMMAND     1
#define PROGCOMMAND     2
#define RUNCOMMAND      3
#define EEPROMCOMMAND   4
#define EXITCOMMAND     5

#define SPM_PAGESIZE 64

#ifndef RWWSRE
#define RWWSRE CTPB
#endif

void (*start_appl_main)(void);
#define BOOTLOADER_FUNC_ADDRESS (BOOTLOADER_ADDRESS - sizeof(start_appl_main))

#ifndef sei
#define sei() asm volatile("sei")
#endif
#ifndef cli
#define cli() asm volatile("cli")
#endif
#ifndef nop
#define nop() asm volatile("nop")
#endif
#ifndef wdr
#define wdr() asm volatile("wdr")
#endif

void eeprom_write(unsigned short address, unsigned char data)
{
  while (EECR & (1 << EEPE));
  EECR = (0 << EEPM1) | (0 << EEPM0);
  if (address < 512) {
    EEAR = address;
  } else {
    EEAR = 511;
  }
  EEDR = data;
  EECR |= (1 << EEMPE);
  EECR |= (1 << EEPE);
}

static uint8_t FrameData[FRAMELENGTH];

#define TOLERANCE 50
#define HIGHMAX ( 70 + ( 70 * TOLERANCE ) / 100 )
#define HIGHMIN ( 70 - ( 70 * TOLERANCE ) / 100 )
#define LOWMAX  ( 35 + ( 35 * TOLERANCE ) / 100 )
#define LOWMIN  ( 35 - ( 35 * TOLERANCE ) / 100 )

#define ISLONGCYCLE  ( ( pulsewidth > LOWMAX ) && ( pulsewidth < HIGHMAX ) )
#define ISSHORTCYCLE ( ( pulsewidth > LOWMIN ) && ( pulsewidth < LOWMAX  ) )

static uint8_t getbit(void)
{
  static uint8_t pulsewidth;
  static uint8_t lastBit = 0;

  TIMER = 0;
  while (PINHIGH) {
    if (TIMER > HIGHMAX) return 2;
  }
  pulsewidth = TIMER;

  TIMER = 0;
  while (PINLOW) {
    if (TIMER > HIGHMAX) return 2;
  }
  pulsewidth += TIMER;

  if (ISLONGCYCLE) {
    lastBit = !lastBit;
    return lastBit;
  }
  if (ISSHORTCYCLE) {
    return lastBit;
  }
  return 2;
}

static uint8_t getByte(void)
{
  uint8_t n;
  uint8_t dataByte = 0;
  for (n = 0; n < 8; n++) {
    uint8_t b = getbit();
    if (b > 1) return 0;
    dataByte = (dataByte << 1) | b;
  }
  return dataByte;
}

static uint8_t receiveFrame(void)
{
  uint8_t n;
  uint8_t checkSum = 0;
  uint8_t b;

  for (n = 0; n < 200; n++) {
    b = getByte();
    if (b == STARTBYTE) break;
  }
  if (b != STARTBYTE) return 0;

  for (n = 0; n < FRAMELENGTH; n++) {
    b = getByte();
    FrameData[n] = b;
    if (n != CHECKSUM) checkSum += b;
  }

  b = getByte();
  if (b != STOPBYTE) return 0;
  if (FrameData[CHECKSUM] != checkSum) return 0;

  return 1;
}

void pgm_write_block(uint16_t address, uint16_t *data, uint8_t length)
{
  uint16_t word;
  uint8_t i;

  for (i = 0; i < length; i += 2) {
    word = *data++;
    boot_page_fill(address + i, word);
  }
  boot_page_write(address);
  boot_spm_busy_wait();
}

void boot_program_page(uint32_t page, uint8_t *buf)
{
  uint16_t i;
  cli();

  boot_page_erase(page);
  boot_spm_busy_wait();

  for (i = 0; i < SPM_PAGESIZE; i += 2) {
    uint16_t w = *buf++;
    w += (*buf++) << 8;

    if (page == 0 && i == 0) {
      void *appl = (void *)(w - RJMP);
      start_appl_main = ((void (*)(void))appl);
      w = 0xC000 + (BOOTLOADER_ADDRESS / 2) - 1;
    }

    boot_page_fill(page + i, w);
    boot_spm_busy_wait();
  }

  boot_page_write(page);
  boot_spm_busy_wait();
}

void resetRegister(void)
{
  DDRB = 0;
  PORTB = 0;
  cli();
  TCCR0B = 0;
  ADCSRA = 0;
  ADMUX = 0;
}

void startMainApplication(void)
{
  resetRegister();
  (*start_appl_main)();
}

void exitBootloader(void)
{
  memcpy_P(&start_appl_main, (PGM_P)BOOTLOADER_FUNC_ADDRESS, sizeof(start_appl_main));
  if (start_appl_main && (uint16_t)start_appl_main != 0xFFFF) {
    startMainApplication();
  }
}

void runProgramm(void)
{
  resetRegister();
  pgm_write_block(BOOTLOADER_FUNC_ADDRESS, (uint16_t *)&start_appl_main, sizeof(start_appl_main));
  start_appl_main();
}

// Skipper button check on PB3 analog ladder
static inline void checkBootloaderSkip(void)
{
  _delay_ms(20); // Power-on voltage settling

  // ADC3 on PB3, VCC reference, Left-adjust result (8-bit reading in ADCH)
  ADMUX = (1 << ADLAR) | (1 << MUX1) | (1 << MUX0);
  // Enable ADC, start first warm-up conversion, prescaler 64 (~250kHz @ 16MHz)
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1 << ADPS1);
  while (ADCSRA & (1 << ADSC)); // wait for first conversion
  ADCSRA |= (1 << ADSC);        // start 2nd conversion
  while (ADCSRA & (1 << ADSC)); // wait for 2nd conversion
  uint8_t val = ADCH;
  ADCSRA = 0;                   // disable ADC to save power

  // On ABK-20 ladder (@ 5.0V VCC):
  // Idle (Center toggle, no button) = 2.63 V -> ADCH ~134
  // Toggle Switch Pos 1 (DOWN)      = 1.72 V -> ADCH ~88
  // Toggle Switch Pos 2 (UP)        = 1.29 V -> ADCH ~66
  // Button SW2 pressed (solo)       = 0.99 V -> ADCH ~51 (enters bootloader)
  // Button SW2 + Toggle 1 (DOWN)    = 0.82 V -> ADCH ~42 (enters bootloader)
  // Button SW2 + Toggle 2 (UP)      = 0.70 V -> ADCH ~36 (enters bootloader)
  //
  // Calibrated threshold: ~1.14 V (ADCH = 58)
  // SW2 pressed: 0.99 V -> ADCH ~51 (solo), 0.82 V -> ADCH ~42, 0.70 V -> ADCH ~36
  // Toggle 2 (UP) alone: 1.29 V -> ADCH ~66
  // Toggle 1 (DOWN) alone: 1.72 V -> ADCH ~88
  // Idle (no button): 2.63 V -> ADCH ~134
  //
  // If val > 58: Button SW2 is NOT pressed -> skip bootloader and immediately run user sketch!
  if (val > 58) {
    exitBootloader();
  }
}

static inline void a_main(void)
{
  uint8_t p;
  uint16_t time = WAITBLINKTIME;
  uint8_t timeout = BOOT_TIMEOUT;

  p = PINVALUE;
  uint8_t exitcounter = 3;

  while (1) {
    if (TIMER > 100) {
      TIMER = 0;
      time--;
      if (time == 0) {
        TOGGLELED;
        time = WAITBLINKTIME;
        timeout--;
        if (timeout == 0) {
          LEDOFF;
          exitBootloader();
        }
      }
    }
    if (p != PINVALUE) {
      p = PINVALUE;
      exitcounter--;
    }
    if (exitcounter == 0) break;
  }

  LEDON;
  while (1) {
    if (!receiveFrame()) {
      while (1) {
        if (TIMER > 100) {
          TIMER = 0;
          time--;
          if (time == 0) {
            TOGGLE_RED();
            time = 1000;
          }
        }
      }
    } else {
      switch (FrameData[COMMAND]) {
        case PROGCOMMAND: {
          uint16_t pageNumber = (((uint16_t)FrameData[PAGEINDEXHIGH]) << 8) + FrameData[PAGEINDEXLOW];
          uint16_t address = SPM_PAGESIZE * pageNumber;
          if (address < BOOTLOADER_ADDRESS) {
            boot_program_page(address, FrameData + DATAPAGESTART);
            if (pageNumber & 1) {
              LED_RED();
            } else {
              LED_GREEN();
            }
          }
          break;
        }
        case RUNCOMMAND: {
          LED_OFF();
          runProgramm();
          break;
        }
        case EEPROMCOMMAND: {
          uint8_t pageNumber = FrameData[PAGEINDEXLOW];
          uint8_t data_length = FrameData[LENGTHLOW];
          uint8_t address = SPM_PAGESIZE * pageNumber;
          uint8_t *buf = FrameData + DATAPAGESTART;
          for (uint8_t i = 0; i < data_length; i++) {
            eeprom_write(address + i, *buf++);
          }
          LEDOFF;
          exitBootloader();
          break;
        }
      }
      FrameData[COMMAND] = NOCOMMAND;
    }
  }
}

int main(void)
{
  INITAUDIOPORT;
  checkBootloaderSkip();
  INITLED;
  TCCR0B = _BV(CS01); // clk/8 -> ~7812.5Hz timer ticks
  a_main();
  return 0;
}

