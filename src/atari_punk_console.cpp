/* ---------------------------------------------------------------------------------------
    ATARI PUNK CONSOLE DELUXE (APC) — STEPPED TONE & SUB-OCTAVE NOISE SYNTH
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    A tribute to Forrest M. Mims III's legendary 1980 "Stepped Tone Generator"
    (Atari Punk Console), supercharged with 8-bit digital audio features:
      - Astable square wave oscillator (Pitch) triggering a retrigger-lockout Monostable
      - True stepped tone harmonic jumps (octaves, fifths, sub-harmonics)
      - Dual Sub-Octave Divider (/2 and /4 flip-flops) for deep sub-bass weight
      - 4 Sound Modes: Pure APC, Sub-Bass Beast, Ring-Mod Cross, Bitcrush Overdrive
      - Real-time potentiometer control over pitch and monostable pulse width
      - 4 Reactive LED Visualizers including Harmonic Staircase & CRT Scope

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Astable Frequency / Master Pitch: 30 Hz - 3.5 kHz)
      PB2 (A1): Right Pot (Monostable Pulse Width / Harmonic Formant Step)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Sound Modes (Classic APC, Sub Beast, Ring Mod, Bitcrush)
                - Right Button: Cycle Visualizers (Staircase, Scope, Shockwave, Sparks)
                - Both Buttons: Laser Pitch Dive-Bomb & Overdrive Strobe!
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

// --- Synth State (DDS & Monostable Emulation) ---
// Sample rate = 10,000 Hz (100 us tick)
volatile uint16_t phase_astable   = 0;
volatile uint16_t inc_astable     = 400; // Controlled by POTI_LEFT
volatile uint16_t mono_width      = 3000; // Controlled by POTI_RIGHT
volatile uint16_t mono_timer      = 0;
volatile uint8_t  mono_state      = 0;

// Sub-octave flip-flops
volatile uint8_t  sub1_state      = 0; // /2 divider
volatile uint8_t  sub2_state      = 0; // /4 divider
volatile uint8_t  sub_count       = 0;

// Sound & Visual Modes
volatile uint8_t  sound_mode      = 0; // 0=Pure APC, 1=Sub Beast, 2=Ring Mod, 3=Bitcrush
volatile uint8_t  visual_mode     = 0; // 0=Staircase, 1=Scope, 2=Shockwave, 3=Sparks
volatile uint8_t  laser_dive      = 0; // 1=Dive bomb active
volatile uint16_t dive_pitch      = 0;

// Metrics for visuals
volatile uint8_t  current_step_div = 1; // Harmonic ratio
volatile uint16_t audio_out_val    = 128;
volatile uint16_t trigger_count    = 0;
volatile uint16_t ticks            = 0;

// Custom delay using 10kHz ISR ticks
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

// --- 10kHz Audio Interrupt ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // 1. Calculate Active Pitch
  uint16_t cur_inc = inc_astable;
  if (laser_dive) {
    // Exponential laser pitch dive
    if (dive_pitch > 60) dive_pitch = ((uint32_t)dive_pitch * 253) >> 8;
    else dive_pitch = 6000; // Loop dive
    cur_inc = dive_pitch;
  }

  // 2. Astable Multivibrator (555 Timer A)
  uint16_t prev_phase = phase_astable;
  phase_astable += cur_inc;
  bool astable_trig = (phase_astable < prev_phase); // Overflow edge trigger!

  // 3. Monostable Multivibrator (555 Timer B with retrigger lockout)
  if (astable_trig) {
    trigger_count++;
    sub_count++;
    sub1_state ^= 1;
    if ((sub_count & 3) == 0) sub2_state ^= 1;

    // Trigger monostable only if previous pulse has expired!
    if (mono_timer == 0) {
      mono_state = 1;
      mono_timer = mono_width;
    }
  }

  if (mono_timer > 0) {
    mono_timer--;
    if (mono_timer == 0) {
      mono_state = 0;
    }
  }

  // 4. Calculate Harmonic Step Ratio (for display)
  // Step div = mono_width / astable_period
  if (cur_inc > 0) {
    uint16_t div = ((uint32_t)mono_width * cur_inc) >> 16;
    current_step_div = (div > 15) ? 15 : (uint8_t)div;
  }

  // 5. Sound Mode Synthesizer Output
  uint8_t out = 128;
  uint8_t astable_sq = (phase_astable < 32768) ? 240 : 15;
  uint8_t mono_pulse = mono_state ? 245 : 10;

  if (sound_mode == 0) {
    // Classic Pure APC: Raw Monostable output
    out = mono_pulse;
  } else if (sound_mode == 1) {
    // Sub-Bass Beast: APC + Sub1 (/2) + Sub2 (/4)
    uint8_t s1 = sub1_state ? 230 : 25;
    uint8_t s2 = sub2_state ? 240 : 15;
    out = ((uint16_t)mono_pulse + s1 + (s2 << 1)) >> 2;
  } else if (sound_mode == 2) {
    // Ring-Mod Cross: Astable XOR Monostable
    uint8_t xor_wave = (astable_sq ^ mono_pulse);
    out = xor_wave;
  } else {
    // Bitcrush & Overdrive: Wavefolded non-linear overdrive
    uint8_t folded = (phase_astable >> 8) ^ (mono_state ? 0xFF : 0x00);
    folded = ((folded & 0xF0) | (sub1_state ? 0x0F : 0x00));
    out = folded;
  }

  audio_out_val = out;
  OCR1A = out;
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (laser_dive) {
    // Laser Dive Strobe: Emergency Red / Electric Blue strobe
    uint32_t strobe_col = (ticks & 32) ? pixels.Color(255, 0, 50) : pixels.Color(0, 220, 255);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, strobe_col);
    }
    pixels.show();
    return;
  }

  if (visual_mode == 0) {
    // Mode 0: Stepped Tone Harmonic Staircase
    // Shows the current stepped harmonic divisor (1..5) as columns of LEDs!
    uint8_t step = current_step_div;
    if (step > 4) step = 4;

    for (uint8_t x = 0; x < 5; x++) {
      for (uint8_t y = 0; y < 4; y++) {
        uint8_t idx = matrixIndex(x, 3 - y);
        if (x <= step && y <= x) {
          // Harmonic ladder
          uint32_t step_col = (x == 0) ? pixels.Color(255, 30, 0) :   // Fundamental (Red)
                              (x == 1) ? pixels.Color(255, 120, 0) :  // 2nd Harmonic (Amber)
                              (x == 2) ? pixels.Color(255, 220, 0) :  // 3rd Harmonic (Yellow)
                              (x == 3) ? pixels.Color(0, 255, 80) :   // 4th Harmonic (Green)
                                         pixels.Color(0, 180, 255);   // 5th Harmonic (Cyan)
          pixels.setPixelColor(idx, step_col);
        } else {
          pixels.setPixelColor(idx, pixels.Color(8, 8, 12));
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Phosphor Amber CRT Oscilloscope
    // Emulates a retro oscilloscope showing the pulse width and audio output
    uint8_t p_width_bars = (mono_width >> 8) & 3; // 0..3 height
    uint8_t beam_pos = (phase_astable >> 13) % 5;

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == beam_pos && mono_state) {
          pixels.setPixelColor(idx, pixels.Color(255, 230, 100)); // Bright electron beam
        } else if (y >= (3 - p_width_bars)) {
          pixels.setPixelColor(idx, pixels.Color(160, 60, 0)); // Phosphor amber trace
        } else {
          pixels.setPixelColor(idx, pixels.Color(10, 5, 0));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Sub-Bass Shockwave Rings
    // Concentric ripple rings pulsed by sub-octave divider triggers
    uint8_t pulse = sub1_state ? 220 : 30;
    uint32_t ring_outer = pixels.Color(0, pulse, pulse);
    uint32_t ring_inner = sub2_state ? pixels.Color(255, 0, 120) : pixels.Color(20, 0, 40);

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if ((x == 0 || x == 4) || (y == 0 || y == 3)) {
          pixels.setPixelColor(idx, ring_outer);
        } else {
          pixels.setPixelColor(idx, ring_inner);
        }
      }
    }
  } else {
    // Mode 3: Glitch Matrix Chaotic Sparks
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t spark = ((trigger_count + x * 7 + y * 13) ^ (ticks >> 5)) & 3;
        if (spark == 0 && mono_state) {
          pixels.setPixelColor(idx, pixels.Color(255, 255, 255)); // White spark
        } else if (spark == 1) {
          pixels.setPixelColor(idx, pixels.Color(200, 0, 255));   // Purple electric
        } else {
          pixels.setPixelColor(idx, pixels.Color(5, 0, 15));
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
  pixels.setBrightness(45);
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
  uint16_t pot_l = analogRead(POTI_LEFT);  // Astable Frequency (Pitch)
  uint16_t pot_r = analogRead(POTI_RIGHT); // Monostable Pulse Width

  // Astable Frequency: ~30 Hz (inc=196) up to ~3.5 kHz (inc=22900)
  // Non-linear response curve for wide musical range
  uint32_t p_sq = ((uint32_t)pot_l * pot_l) >> 10;
  inc_astable = 196 + (p_sq * 22) / 10;

  // Monostable Pulse Width: 2 samples (~0.2ms) up to 1000 samples (~100ms)
  mono_width = 2 + ((uint32_t)pot_r * 998) / 1023;

  // 2. Read Button Resistor Ladder (PB3 / A3)
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons: Laser Dive-Bomb Overload!
    if (!laser_dive) {
      laser_dive = 1;
      dive_pitch = 6000;
    }
  } else {
    laser_dive = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Sound Modes
      sound_mode = (sound_mode + 1) & 3;
      uint32_t badge = (sound_mode == 0) ? pixels.Color(255, 50, 0) :   // Pure APC (Red/Amber)
                       (sound_mode == 1) ? pixels.Color(0, 180, 255) :  // Sub Beast (Cyan)
                       (sound_mode == 2) ? pixels.Color(255, 0, 180) :  // Ring Mod (Magenta)
                                           pixels.Color(0, 255, 50);    // Bitcrush (Green)
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
  my_delay(20);
}
