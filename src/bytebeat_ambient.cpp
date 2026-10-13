/* ---------------------------------------------------------------------------------------
    BYTEBEAT AMBIENT & COSMIC FRACTALS
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    A meditative, ethereal, and cosmic soundscape machine generating evolving ambient
    drones, celestial arpeggios, and fractal harmonic washes from 4 pure mathematical one-liners:
      Formula 0: ((t >> 6 | t >> 7 | t >> 8) * (t >> 4 & t >> 8))                   (Cosmic Choir & Celestial Strings)
      Formula 1: (((t * 9 & t >> 4) | (t * 5 & t >> 7) | (t * 3 & t >> 10)) - 1)     (Starlight Arpeggios & Crystals)
      Formula 2: (((t * ((t >> 14 & t >> 8) & 248) & 128) + ((t >> 8 & t >> 13) & (t >> 3)))) (Deep Space Alien Pulsar)
      Formula 3: (((t >> 7) * (t >> 11) ^ ((t >> 5) * (t >> 9))))                  (Fractal Mathematical Void)

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Time Dilation & Pitch: Ultra-Deep Sub Drone to Crystalline Chime)
      PB2 (A1): Right Pot (Harmonic Shimmer, Bit-Drift & Spatial Overtone Dispersion)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle 4 Cosmic One-Liners
                - Right Button: Cycle 4 Cosmic Visualizers (Aurora Borealis, Starlight Plasma, Void Vortex, Constellation)
                - Both Buttons: Supernova Quantum Freeze Strobe!
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

// --- Ambient State ---
volatile uint32_t t = 0;
volatile uint16_t t_step = 256;      // Time progression step
volatile uint8_t  shimmer_mod = 0;   // Right pot harmonic shimmer
volatile uint8_t  ambient_mode = 0;  // 0..3
volatile uint8_t  visual_mode = 0;   // 0..3
volatile uint8_t  freeze_active = 0; // Both buttons held
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

// --- 10kHz Audio Interrupt: Ambient Bytebeat Evaluator ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // Advance time counter (freeze holds time in suspension!)
  if (!freeze_active) {
    t += t_step;
  }
  uint32_t tt = t >> 8; // Integer time index

  uint8_t out = 0;
  uint8_t sm = shimmer_mod;

  switch (ambient_mode) {
    case 0:
      // Cosmic Choir & Celestial Strings
      out = (uint8_t)(((tt >> 6 | tt >> 7 | tt >> 8) * ((tt >> 4 & tt >> 8) + (sm >> 5))));
      break;

    case 1:
      // Starlight Arpeggios & Harmonic Crystals
      out = (uint8_t)((((tt * (9 + (sm >> 6)) & tt >> 4) | (tt * 5 & tt >> 7) | (tt * 3 & tt >> 10)) - 1));
      break;

    case 2:
      // Deep Space Alien Communications & Pulsar Drone
      out = (uint8_t)((((tt * ((tt >> 14 & tt >> 8) & (248 - (sm >> 4))) & 128) + ((tt >> 8 & tt >> 13) & (tt >> 3)))));
      break;

    case 3:
    default:
      // Fractal Mathematical Void
      out = (uint8_t)((((tt >> 7) * (tt >> 11) ^ ((tt >> 5) * (tt >> 9))) + (sm >> 3)));
      break;
  }

  // Soft compression for ethereal ambient listening
  out = (out >> 1) + 64;
  last_sample = out;
  OCR1A = out;
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (freeze_active) {
    // Supernova Freeze: Shimmering Diamond White & Lavender Starlight
    uint32_t nova = (ticks & 32) ? pixels.Color(240, 240, 255) : pixels.Color(160, 100, 255);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, nova);
    }
    pixels.show();
    return;
  }

  uint32_t tt = t >> 8;
  uint8_t s = last_sample;

  if (visual_mode == 0) {
    // Mode 0: Aurora Borealis (Slow breathing waves of Cyan, Emerald, and Violet)
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t wave = ((tt >> 5) + x * 5 + y * 9) & 31;
        uint8_t g = (wave < 16) ? (wave << 4) : ((31 - wave) << 4);
        uint8_t b = 255 - (g >> 1);
        pixels.setPixelColor(matrixIndex(x, y), pixels.Color(10, g, b));
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Starlight Plasma (Twinkling cosmic crystal stars)
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t twinkle = ((tt >> 4) ^ (x * 47) ^ (y * 83)) & 15;
        uint32_t col = (twinkle == 0) ? pixels.Color(255, 255, 255) : // Bright star
                       (twinkle < 3)  ? pixels.Color(100, 180, 255) : // Cyan star
                       (twinkle < 6)  ? pixels.Color(180, 60, 220)  : // Violet nebula
                                        pixels.Color(5, 5, 20);       // Deep space
        pixels.setPixelColor(matrixIndex(x, y), col);
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Void Vortex (Concentric event-horizon rings)
    uint8_t rot = (tt >> 5) & 7;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t d = abs((int8_t)x - 2) + abs((int8_t)y - 1);
        bool ring = ((d + rot) & 3) == 0;
        uint32_t col = ring ? pixels.Color(255, 80, 160) : pixels.Color(10, 0, 30);
        pixels.setPixelColor(matrixIndex(x, y), col);
      }
    }
  } else {
    // Mode 3: Celestial Constellation (Interlinked starlight nodes)
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t node = (x + y * 2) & 7;
        bool is_lit = (s & (1 << node)) != 0;
        uint32_t col = is_lit ? pixels.Color(255, 220, 140) : pixels.Color(15, 10, 5);
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
  uint16_t pot_l = analogRead(POTI_LEFT);  // Time dilation / pitch
  uint16_t pot_r = analogRead(POTI_RIGHT); // Harmonic shimmer / bit-drift

  // Time step: Wide range from ultra-slow ambient drone (step=20) to bright crystal bells (step=900)
  uint32_t p_sq = ((uint32_t)pot_l * pot_l) >> 10;
  t_step = 20 + (p_sq * 11) / 10;

  // Harmonic shimmer modifier
  shimmer_mod = pot_r >> 2; // 0..255

  // 2. Read Button Resistor Ladder (PB3 / A3) — Instant Tactile Feedback
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons held: Supernova Freeze!
    freeze_active = 1;
  } else {
    freeze_active = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers INSTANTLY
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Ambient Equations INSTANTLY
      ambient_mode = (ambient_mode + 1) & 3;
      uint32_t badge = (ambient_mode == 0) ? pixels.Color(0, 220, 255) : // Cosmic Choir (Cyan)
                       (ambient_mode == 1) ? pixels.Color(160, 80, 255) : // Starlight Arp (Lavender)
                       (ambient_mode == 2) ? pixels.Color(255, 60, 180) : // Alien Pulsar (Pink)
                                             pixels.Color(0, 240, 100);  // Fractal Void (Emerald)
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
