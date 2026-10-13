/* ---------------------------------------------------------------------------------------
    BYTEBEAT EXPLORER — VIZNUT 8-BIT CLASSICS & GLITCH MACHINE
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An interactive hardware explorer for the legendary algorithmic "one-liner" bytebeat
    formulas discovered by Viznut (Ville-Matias Heikkilä), Tejeez, and the demoscene:
      Formula 0: (t * (t >> 12 | t >> 8) & 63 & t >> 4)                (The 42 Melody)
      Formula 1: ((t * 5 & t >> 7) | (t * 3 & t >> 10))                (Algorithmic Arp)
      Formula 2: (t * (t >> 9 | t >> 13) & 16)                         (Chiptune Counter)
      Formula 3: ((t * (t >> 8) * (t >> 12) & 64) + ((t >> 5 & t >> 7) & t >> 3)) (Sierpinski Glitch)

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Clock Speed / Sample Rate Warp: 2 kHz to 24 kHz)
      PB2 (A1): Right Pot (Pitch Warp, Bitmask Shift & Bitwise Glitch)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle 4 One-Liner Formulas
                - Right Button: Cycle 4 Bit-Matrix Visualizers (Bit Waterfall, CRT Scope, Logic Flame, Sierpinski)
                - Both Buttons: Turbo Stutter Overclock Warp!
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

// --- Bytebeat State ---
volatile uint32_t t = 0;
volatile uint16_t t_step = 256;      // Fixed-point time increment (256 = 1.0)
volatile uint8_t  warp_param = 0;    // Right pot modifier
volatile uint8_t  formula_mode = 0;  // 0..3
volatile uint8_t  visual_mode = 0;   // 0..3
volatile uint8_t  turbo_active = 0;  // Both buttons held
volatile uint8_t  last_sample = 0;   // Live audio output byte for visuals
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

// --- 10kHz Audio Interrupt: Ultra-Lean Bytebeat Evaluator ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // Advance time counter with fixed-point fractional step
  uint16_t step = turbo_active ? (t_step << 1) : t_step;
  t += step;
  uint32_t tt = t >> 8; // Integer time index

  uint8_t out = 0;
  uint8_t w = warp_param;

  switch (formula_mode) {
    case 0:
      // The 42 Melody with dynamic shift modifier
      out = (uint8_t)((tt * ((tt >> (12 - (w >> 6))) | (tt >> 8)) & (63 + (w >> 3)) & (tt >> 4)));
      break;

    case 1:
      // Algorithmic Arpeggiator
      out = (uint8_t)(((tt * 5 & (tt >> (7 - (w >> 6)))) | (tt * (3 + (w >> 5)) & (tt >> 10))));
      break;

    case 2:
      // Minimal Chiptune Counter
      out = (uint8_t)((tt * ((tt >> 9) | (tt >> 13)) & (16 + (w >> 4))));
      break;

    case 3:
    default:
      // Sierpinski Glitch Machine
      out = (uint8_t)(((tt * (tt >> 8) * (tt >> 12) & (64 + (w >> 3))) + (((tt >> 5) & (tt >> 7)) & (tt >> 3))));
      break;
  }

  // Boost dynamic range if quiet
  out = (out << 2) | (out >> 6);
  last_sample = out;
  OCR1A = out;
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (turbo_active) {
    // Turbo Overclock Strobe: Neon Cyber Strobe
    uint32_t strobe = (ticks & 32) ? pixels.Color(255, 255, 0) : pixels.Color(0, 255, 255);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, strobe);
    }
    pixels.show();
    return;
  }

  uint32_t tt = t >> 8;
  uint8_t s = last_sample;

  if (visual_mode == 0) {
    // Mode 0: Bit-Waterfall Matrix
    // Displays the 8 bits of the current bytebeat output descending across columns
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t bit_idx = (x + y * 2) & 7;
        bool is_set = (s & (1 << bit_idx));
        uint32_t col = is_set ? pixels.Color(0, 220, 255) : pixels.Color(5, 10, 25);
        if (x == 2) col = is_set ? pixels.Color(255, 100, 0) : pixels.Color(20, 5, 0);
        pixels.setPixelColor(matrixIndex(x, y), col);
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: CRT Oscilloscope Trace
    // Maps live audio sample amplitude to vertical beam height
    uint8_t beam_y = (s >> 6) & 3; // 0..3
    uint8_t sweep_x = (tt >> 6) % 5;

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == sweep_x && y == (3 - beam_y)) {
          pixels.setPixelColor(idx, pixels.Color(255, 255, 255)); // Active phosphor beam
        } else if (y == (3 - beam_y)) {
          pixels.setPixelColor(idx, pixels.Color(0, 200, 60));    // Green trace line
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 15, 5));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Logic Flame (Thermal bitmask heat spectrum)
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t heat = ((tt >> (x + 2)) ^ (s >> (y + 1))) & 15;
        uint32_t col = (heat > 11) ? pixels.Color(255, 240, 180) :
                       (heat > 7)  ? pixels.Color(255, 120, 0)   :
                       (heat > 3)  ? pixels.Color(180, 0, 40)    :
                                     pixels.Color(20, 0, 10);
        pixels.setPixelColor(matrixIndex(x, y), col);
      }
    }
  } else {
    // Mode 3: Sierpinski Fractal Matrix
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t u = (x + (tt >> 7)) & 15;
        uint8_t v = (y + (tt >> 8)) & 15;
        bool fractal = ((u & v) == 0);
        uint32_t col = fractal ? pixels.Color(180, 0, 255) : pixels.Color(0, 20, 40);
        pixels.setPixelColor(matrixIndex(x, y), col);
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
  uint16_t pot_l = analogRead(POTI_LEFT);  // Clock speed / sample rate warp
  uint16_t pot_r = analogRead(POTI_RIGHT); // Parameter modifier / bitmask shift

  // Clock Rate: Smooth exponential scaling from slow hypnotic chiptune (step=40) up to hyperspeed (step=1400)
  uint32_t p_sq = ((uint32_t)pot_l * pot_l) >> 10;
  t_step = 40 + (p_sq * 14) / 10;

  // Warp parameter for one-liner equations
  warp_param = pot_r >> 2; // 0..255

  // 2. Read Button Resistor Ladder (PB3 / A3) — Instant Tactile Feedback
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons held: Turbo Overclock Strobe!
    turbo_active = 1;
  } else {
    turbo_active = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers INSTANTLY
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle One-Liner Formulas INSTANTLY
      formula_mode = (formula_mode + 1) & 3;
      uint32_t badge = (formula_mode == 0) ? pixels.Color(0, 220, 255) : // The 42 Melody (Cyan)
                       (formula_mode == 1) ? pixels.Color(255, 0, 160) : // Algorithmic Arp (Magenta)
                       (formula_mode == 2) ? pixels.Color(255, 180, 0) : // Chiptune Counter (Amber)
                                             pixels.Color(0, 255, 60);   // Sierpinski Glitch (Green)
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
