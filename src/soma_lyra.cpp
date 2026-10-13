/* ---------------------------------------------------------------------------------------
    SOMA LYRA-8 — ORGANISMIC SYNTHESIZER
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic digital tribute to Vladislav Kreimer / SOMA Laboratory's legendary
    "Lyra-8" organismic synthesizer:
      - 8 interlinked voice oscillators grouped in 4 pairs (1-2, 3-4, 5-6, 7-8)
      - Non-linear cross-modulation & FM feedback loops between voice pairs
      - The iconic "HYPER LFO": dual interconnected LFOs (A & B) with non-linear logic
      - Simulated SOMA Dual Delay & non-linear diode "DRIVE" saturation
      - Ethereal ambient microtonal drones, sacred choir clusters, and feedback screams!

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Master Tuning Cluster & Pitch Spread)
      PB2 (A1): Right Pot (Hyper LFO Depth, Drive Saturation & Dual Delay Time)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Lyra Modes (Organismic Drone, Hyper LFO FM, Industrial Drive, Touch Pulsar)
                - Right Button: Cycle Visualizers (8-Voice Nodes, Hyper LFO Orbit, Dual Delay, Touch Field)
                - Both Buttons: Full 8-Voice Resonant Overload & Space Strobe!
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

// --- Synth State: 8 Organismic Voices ---
// Sample rate = 10,000 Hz
volatile uint16_t phase[8];
volatile uint16_t freq[8];

// Intervals for the 8 voices (Lyra-8 natural harmonic tuning ratios)
// Fundamental, Octave, 5th, Octave+Minor3rd, etc.
const uint8_t voice_ratios[8] PROGMEM = {
  10, 12, 15, 20, 24, 30, 40, 48
};

// Hyper LFO (dual cross-modulating LFO A & B)
volatile uint16_t phase_lfo_a     = 0;
volatile uint16_t phase_lfo_b     = 0;
volatile uint16_t rate_lfo_a      = 35;
volatile uint16_t rate_lfo_b      = 47;
volatile uint8_t  hyper_lfo_val   = 128;

// SOMA Dual Delay Simulation (compact 96-byte circular buffer to preserve SRAM!)
#define SOMA_DELAY_SIZE 96
volatile uint8_t  soma_delay[SOMA_DELAY_SIZE];
volatile uint8_t  delay_idx       = 0;
volatile uint8_t  delay_time      = 60;
volatile uint8_t  delay_feedback  = 140;
volatile uint8_t  delay_decim     = 0;

// Controls & Modes
volatile uint8_t  lyra_mode       = 0; // 0=Drone, 1=Hyper LFO, 2=Drive, 3=Pulsar
volatile uint8_t  visual_mode     = 0; // 0=8-Voices, 1=LFO Orbit, 2=Delay, 3=Touch Field
volatile uint8_t  overload_active = 0; // Both buttons held
volatile uint8_t  drive_amt       = 80;
volatile uint8_t  fm_depth        = 60;
volatile uint16_t master_pitch    = 350;
volatile uint16_t ticks           = 0;

// Individual voice amplitude envelopes (for visuals)
volatile uint8_t  voice_amp[8];

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

// Inline fast triangle wave generator
static inline uint8_t get_triangle(uint16_t p) {
  uint8_t t = p >> 8;
  return (t < 128) ? (t << 1) : ((255 - t) << 1);
}

// --- 10kHz Audio Interrupt (Optimized: Zero Software Division!) ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // 1. Hyper LFO
  phase_lfo_a += rate_lfo_a;
  phase_lfo_b += rate_lfo_b;
  uint8_t lfo_a_tri = get_triangle(phase_lfo_a);
  uint8_t lfo_b_tri = get_triangle(phase_lfo_b);
  hyper_lfo_val = (lfo_a_tri ^ lfo_b_tri);

  // 2. Advance 8 Voice Phases
  phase[0] += freq[0];
  phase[1] += freq[1];
  phase[2] += freq[2];
  phase[3] += freq[3];
  phase[4] += freq[4];
  phase[5] += freq[5];
  phase[6] += freq[6];
  phase[7] += freq[7];

  // 3. Compute Triangle Waves (Only ONCE per voice)
  uint8_t t0 = get_triangle(phase[0]);
  uint8_t t1 = get_triangle(phase[1]);
  uint8_t t2 = get_triangle(phase[2]);
  uint8_t t3 = get_triangle(phase[3]);
  uint8_t t4 = get_triangle(phase[4]);
  uint8_t t5 = get_triangle(phase[5]);
  uint8_t t6 = get_triangle(phase[6]);
  uint8_t t7 = get_triangle(phase[7]);

  voice_amp[0] = t0;
  voice_amp[1] = t1;
  voice_amp[2] = t2;
  voice_amp[3] = t3;
  voice_amp[4] = t4;
  voice_amp[5] = t5;
  voice_amp[6] = t6;
  voice_amp[7] = t7;

  uint16_t voice_sum = (uint16_t)t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
  uint8_t mix = voice_sum >> 3;

  // 4. Non-Linear Diode Drive Saturation
  uint8_t drive = overload_active ? 250 : drive_amt;
  int16_t sat = ((int16_t)mix - 128);
  sat = (sat * (128 + drive)) >> 7;
  if (sat > 110) sat = 110;
  else if (sat < -110) sat = -110;
  uint8_t driven = (uint8_t)(sat + 128);

  // 5. SOMA Dual Delay Space Modulator (Decimated, Fast Wraparound)
  delay_decim++;
  if ((delay_decim & 7) == 0) {
    uint8_t read_i = (delay_idx - delay_time);
    if (read_i >= SOMA_DELAY_SIZE) read_i += SOMA_DELAY_SIZE;
    uint8_t delay_sample = soma_delay[read_i];

    uint16_t fb = overload_active ? 245 : delay_feedback;
    int16_t fb_sig = ((int16_t)delay_sample - 128);
    int16_t d_mix = ((int16_t)driven - 128) + ((fb_sig * fb) >> 8);

    if (d_mix > 115) d_mix = 115;
    else if (d_mix < -115) d_mix = -115;

    soma_delay[delay_idx] = (uint8_t)(d_mix + 128);
    delay_idx++;
    if (delay_idx >= SOMA_DELAY_SIZE) delay_idx = 0;
  }

  // 6. Master Output
  uint8_t read_i = (delay_idx - delay_time);
  if (read_i >= SOMA_DELAY_SIZE) read_i += SOMA_DELAY_SIZE;
  uint8_t wet = soma_delay[read_i];
  uint16_t master = ((uint16_t)driven + ((uint16_t)wet << 1)) / 3;

  OCR1A = (uint8_t)master;
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (overload_active) {
    // SOMA Overload Strobe: Electric Blue / Magenta lightning
    uint32_t col = (ticks & 32) ? pixels.Color(0, 220, 255) : pixels.Color(255, 0, 180);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, col);
    }
    pixels.show();
    return;
  }

  if (visual_mode == 0) {
    // Mode 0: Lyra-8 Eight Voice Nodes Matrix
    // Columns 0 & 1: Voice Pair 1 (V0, V1) and Voice Pair 2 (V2, V3)
    // Columns 3 & 4: Voice Pair 3 (V4, V5) and Voice Pair 4 (V6, V7)
    // Center Column 2: Space Delay Feedback Glow
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t pair = i >> 1; // 0..3
      uint8_t sub  = i & 1;  // 0 or 1
      uint8_t x = (pair < 2) ? pair : (pair + 1);
      uint8_t y = sub * 2;

      uint8_t amp = voice_amp[i];
      uint32_t col = (pair == 0) ? pixels.Color(0, amp, 255) :    // Cyan pair
                     (pair == 1) ? pixels.Color(0, 255, amp) :    // Green pair
                     (pair == 2) ? pixels.Color(amp, 180, 0) :    // Amber pair
                                   pixels.Color(255, 0, amp);     // Magenta pair

      pixels.setPixelColor(matrixIndex(x, y), col);
      pixels.setPixelColor(matrixIndex(x, y + 1), col);
    }
    uint8_t d_glow = soma_delay[delay_idx];
    for (uint8_t y = 0; y < 4; y++) {
      pixels.setPixelColor(matrixIndex(2, y), pixels.Color(d_glow >> 2, d_glow >> 3, d_glow >> 2));
    }
  } else if (visual_mode == 1) {
    // Mode 1: Hyper LFO Lissajous Phase Orbit
    uint8_t ax = (phase_lfo_a >> 13) % 5;
    uint8_t by = (phase_lfo_b >> 14) % 4;

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == ax && y == by) {
          pixels.setPixelColor(idx, pixels.Color(255, 255, 255)); // Hyper LFO cursor
        } else if (abs((int8_t)x - (int8_t)ax) <= 1 && abs((int8_t)y - (int8_t)by) <= 1) {
          pixels.setPixelColor(idx, pixels.Color(0, 180, 255));   // Organic halo
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 10, 30));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Dual Delay Space Echo Rings
    uint8_t ring = (delay_idx >> 4) & 3;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        bool edge = (x == 0 || x == 4 || y == 0 || y == 3);
        if (edge && ring == 0) {
          pixels.setPixelColor(idx, pixels.Color(255, 120, 0));
        } else if (!edge && ring == 1) {
          pixels.setPixelColor(idx, pixels.Color(0, 255, 150));
        } else {
          pixels.setPixelColor(idx, pixels.Color(15, 5, 25));
        }
      }
    }
  } else {
    // Mode 3: Bio-Electric Touch Field
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t touch = ((ticks >> 6) + x * 3 + y * 7) & 7;
        if (touch == 0) {
          pixels.setPixelColor(idx, pixels.Color(255, 220, 180)); // Skin contact gold
        } else {
          pixels.setPixelColor(idx, pixels.Color(10, 0, 20));
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

  // Clear delay buffer
  for (uint8_t i = 0; i < SOMA_DELAY_SIZE; i++) {
    soma_delay[i] = 128;
  }

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
  uint16_t pot_l = analogRead(POTI_LEFT);  // Left Knob: Master Pitch
  uint16_t pot_r = analogRead(POTI_RIGHT); // Right Slider: Hyper LFO, FM, Drive, Delay

  // Master Pitch: Exponential curve sweeping from 32 Hz up to 750 Hz (highest voice at 3.6 kHz!)
  uint32_t p_sq = ((uint32_t)pot_l * pot_l) >> 10;
  master_pitch = 220 + (p_sq * 48) / 10;

  // Slider controls parameters across all modes
  fm_depth       = ((uint32_t)pot_r * 220) / 1023;
  drive_amt      = 20 + ((uint32_t)pot_r * 200) / 1023;
  rate_lfo_a     = 10 + ((uint32_t)pot_r * 130) / 1023;
  rate_lfo_b     = 18 + ((uint32_t)pot_r * 190) / 1023;
  delay_time     = 14 + ((uint32_t)pot_r * 76) / 1023;
  delay_feedback = 60 + ((uint32_t)pot_r * 180) / 1023;

  // Compute Voice Frequencies for the 8 Voices (4 Pairs)
  int16_t lfo_cv = ((int16_t)hyper_lfo_val - 128) * (int16_t)fm_depth >> 7;

  for (uint8_t i = 0; i < 8; i++) {
    uint8_t r = pgm_read_byte(&voice_ratios[i]);
    int32_t f = ((uint32_t)master_pitch * r) / 10;

    if (overload_active) {
      f += lfo_cv * (i + 1);
      f += ((int16_t)voice_amp[i ^ 1] - 128) * (fm_depth >> 1);
    } else if (lyra_mode == 0) {
      // Organismic Drone: lush inter-voice beating between pairs
      int16_t pair_mod = ((int16_t)voice_amp[i ^ 1] - 128) * (fm_depth >> 3);
      f += pair_mod;
    } else if (lyra_mode == 1) {
      // Hyper LFO Chaos: alien modulation sweeps
      if (i & 1) f += lfo_cv * (i + 1);
      else f += ((int16_t)voice_amp[i ^ 1] - 128) * (fm_depth >> 2);
    } else if (lyra_mode == 2) {
      // Industrial Screaming Drive: intense FM modulation
      f += ((int16_t)voice_amp[i ^ 1] - 128) * (fm_depth >> 2);
    } else if (lyra_mode == 3) {
      // Touch Pulsar: 8-step sequencer
      uint8_t step = (ticks >> 9) & 7;
      if (i != step && ((i + 4) & 7) != step) f = 0;
      else f += lfo_cv * 2;
    }

    if (f < 10 && f != 0) f = 10;
    freq[i] = (uint16_t)f;
  }

  // 2. Read Button Resistor Ladder (PB3 / A3) — Instant Tactile Response
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons held: Instant Overload!
    overload_active = 1;
  } else {
    overload_active = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers INSTANTLY
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Lyra Modes INSTANTLY
      lyra_mode = (lyra_mode + 1) & 3;
      uint32_t badge = (lyra_mode == 0) ? pixels.Color(0, 180, 255) : // Organismic Drone (Cyan)
                       (lyra_mode == 1) ? pixels.Color(255, 0, 150) : // Hyper LFO (Magenta)
                       (lyra_mode == 2) ? pixels.Color(255, 80, 0) :  // Industrial Drive (Amber)
                                          pixels.Color(0, 255, 80);   // Touch Pulsar (Green)
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
