/* ---------------------------------------------------------------------------------------
    BYTEBEAT TECHNO & ACID — ALGORITHMIC RHYTHM GENERATOR
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An algorithmic techno, acid, and industrial drum/bass machine powered by 4 relentless
    mathematical bytebeat "one-liners":
      Formula 0: ((t >> 6 | t | t >> (t >> 16)) * 10 + ((t >> 11) & 7))           (Driving Cyber Techno)
      Formula 1: (((t >> 7 | t | t >> 6) * 10 + 4 * (t & (t >> 13) | (t >> 6))))   (Heavy Industrial Stomp)
      Formula 2: (t * ((t >> 12 | t >> 8) & (t >> 4) & (t >> 7)))                  (Modular Acid 303 Groove)
      Formula 3: ((t >> 5) | (t << 4) | ((t >> 10) ^ ((t >> 9) - 1) % 11 * t))    (Dystopian Cyberpunk Beat)

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Master Tempo & Groove Clock Speed: 70 to 180 BPM)
      PB2 (A1): Right Pot (Bitwise Overdrive, Fold Distortion & Glitch Shredder)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle 4 Techno/Acid One-Liners
                - Right Button: Cycle 4 Rhythm Visualizers (808 Kick Flash, Industrial Strobe, Cyber Waterfall, Acid Pulse)
                - Both Buttons: Stutter Roll Overdrive!
--------------------------------------------------------------------------------------- */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"

// --- Hardware Pin Definitions ---
#define NUMPIXELS   20
#define NEOPIXELPIN 0
#define SPEAKERPIN  1
#define BUTTON_PIN  A3
#define POTI_LEFT   A2 // PB4 (ADC2)
#define POTI_RIGHT  A1 // PB2 (ADC1)

// --- Button Voltage Calibration ---
#define Vbutton_releaseLevel  490
#define Vbutton_left          460
#define Vbutton_right         395
#define Vbutton_both          320

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Synth State ---
volatile uint32_t t = 0;
volatile uint16_t t_step = 256;      // Time step
volatile uint8_t  drive_mod = 0;     // Right pot distortion
volatile uint8_t  techno_mode = 0;   // 0..3
volatile uint8_t  visual_mode = 0;   // 0..3
volatile uint8_t  stutter_active = 0;// Both buttons held
volatile uint8_t  kick_flash = 0;    // Kick drum detector for visuals
volatile uint8_t  last_sample = 0;
volatile uint16_t ticks = 0;

// Custom delay
void my_delay(uint16_t ms) {
  uint16_t duration = ms * 10;
  uint16_t start;
  uint8_t sreg = SREG; cli(); start = ticks; SREG = sreg;

  while (1) {
    uint16_t current;
    sreg = SREG; cli(); current = ticks; SREG = sreg;
    if ((uint16_t)(current - start) >= duration) break;
  }
}

// --- 10kHz Audio Interrupt: Bytebeat Techno Generator ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // Advance time counter (stutter roll doubles speed when held)
  uint16_t step = stutter_active ? (t_step << 2) : t_step;
  t += step;
  uint32_t tt = t >> 8; // Integer time

  uint8_t out = 0;
  uint8_t d = drive_mod;

  switch (techno_mode) {
    case 0:
      // Driving Cyber Techno
      out = (uint8_t)(((tt >> 6 | tt | tt >> (tt >> 16)) * (10 + (d >> 5)) + ((tt >> 11) & 7)));
      break;

    case 1:
      // Heavy Industrial Stomp with Kick & Hats
      out = (uint8_t)((((tt >> 7 | tt | tt >> 6) * 10 + 4 * ((tt & (tt >> 13)) | (tt >> 6))) ^ (d >> 2)));
      break;

    case 2:
      // Modular Acid 303 Groove
      out = (uint8_t)((tt * (((tt >> 12 | tt >> 8) & (tt >> 4) & (tt >> 7)) + (d >> 4))));
      break;

    case 3:
    default:
      // Dystopian Cyberpunk Beat
      out = (uint8_t)(((tt >> 5) | (tt << 4) | (((tt >> 10) ^ ((tt >> 9) - 1) % 11 * tt) + (d >> 3))));
      break;
  }

  // Detect low-frequency drum beat pulses for reactive visuals
  if ((out > 220) || (out < 35)) {
    if (kick_flash < 240) kick_flash += 15;
  } else if (kick_flash > 0) {
    kick_flash--;
  }

  last_sample = out;
  OCR1A = out;
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (stutter_active) {
    // Stutter Overdrive: Red & Yellow Industrial Hazard Strobe
    uint32_t hazard = (ticks & 32) ? pixels.Color(255, 0, 0) : pixels.Color(255, 200, 0);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, hazard);
    }
    pixels.show();
    return;
  }

  uint32_t tt = t >> 8;
  uint8_t s = last_sample;
  uint8_t kick = kick_flash;

  if (visual_mode == 0) {
    // Mode 0: 808 Sub Kick Flare & Rhythm Columns
    // Center column explodes on kick drums, outer columns show hi-hat activity
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == 2) {
          // Center Kick Tower
          pixels.setPixelColor(idx, pixels.Color(kick, kick >> 1, 0));
        } else {
          // Hi-Hat / Bass Rhythm Lines
          uint8_t level = (s >> (x + y)) & 3;
          uint32_t col = (level == 3) ? pixels.Color(0, 240, 255) :
                         (level == 2) ? pixels.Color(0, 100, 180) :
                                        pixels.Color(5, 10, 20);
          pixels.setPixelColor(idx, col);
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Industrial Techno Strobe & Barcode Scan
    uint8_t bar = (tt >> 6) % 5;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == bar) {
          pixels.setPixelColor(idx, pixels.Color(255, 255, 255)); // Active sweep line
        } else if ((s & (1 << (y + x))) != 0) {
          pixels.setPixelColor(idx, pixels.Color(255, 0, 50));    // Industrial red strobe
        } else {
          pixels.setPixelColor(idx, pixels.Color(10, 0, 5));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Acid Matrix Digital Rain (Cyberpunk Phosphor Green)
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t drop = ((tt >> 5) + x * 7 + y * 11) & 15;
        uint32_t col = (drop == 0) ? pixels.Color(200, 255, 200) : // Leading raindrop head
                       (drop < 4)  ? pixels.Color(0, 230, 40)    : // Matrix green trail
                                     pixels.Color(0, 15, 5);
        pixels.setPixelColor(matrixIndex(x, y), col);
      }
    }
  } else {
    // Mode 3: Cyber Radar Radial Scanner
    uint8_t phase_step = (tt >> 5) % 12;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t ring = (abs((int8_t)x - 2) + abs((int8_t)y - 2));
        if (ring == (phase_step & 3)) {
          pixels.setPixelColor(idx, pixels.Color(255, 120, 0)); // Expanding sonar ring
        } else {
          pixels.setPixelColor(idx, pixels.Color(15, 5, 20));
        }
      }
    }
  }

  pixels.show();
}

// --- Setup ---
void setup() {
  cli();

  // NeoPixel Setup
  pixels.begin();
  pixels.setBrightness(55);
  pixels.show();

  // High-Speed 64MHz PLL Audio PWM on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  TCCR1  = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10);
  OCR1A  = 128;
  pinMode(SPEAKERPIN, OUTPUT);

  // 10kHz Audio Sample Clock on Timer0
  TCCR0A = (3 << WGM00);
  TCCR0B = (1 << WGM02) | (2 << CS00);
  TIMSK  = (1 << OCIE0A);
  OCR0A  = 199;

  sei();
}

// --- Main Loop ---
void loop() {
  // 1. Read Potentiometers
  uint16_t pot_l = analogRead(POTI_LEFT);  // Clock / Tempo (70 to 180 BPM feel)
  uint16_t pot_r = analogRead(POTI_RIGHT); // Overdrive / Distortion / Glitch

  // Tempo Clock: Smooth exponential progression
  uint32_t p_sq = ((uint32_t)pot_l * pot_l) >> 10;
  t_step = 60 + (p_sq * 18) / 10;

  // Distortion / Bitwise Shredder
  drive_mod = pot_r >> 2; // 0..255

  // 2. Read Button Resistor Ladder (PB3 / A3) — Instant Tactile Feedback
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons held: Stutter Roll Overdrive!
    stutter_active = 1;
  } else {
    stutter_active = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers INSTANTLY
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Techno Formulas INSTANTLY
      techno_mode = (techno_mode + 1) & 3;
      uint32_t badge = (techno_mode == 0) ? pixels.Color(255, 0, 0) :   // Cyber Techno (Red)
                       (techno_mode == 1) ? pixels.Color(255, 120, 0) : // Industrial Stomp (Amber)
                       (techno_mode == 2) ? pixels.Color(0, 255, 60) :  // Modular Acid (Green)
                                            pixels.Color(0, 180, 255);  // Dystopian Beat (Cyan)
      pixels.fill(badge, 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn > Vbutton_releaseLevel) {
      prev_pressed = 0;
    }
  }

  // 3. Render Visuals
  update_visuals();
  my_delay(15);
}
