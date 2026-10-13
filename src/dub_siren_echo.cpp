/* ---------------------------------------------------------------------------------------
    JAMAICAN DUB SIREN & LO-FI TAPE ECHO MACHINE
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic sound-system dub siren instrument with simulated vintage space
    tape echo, inspired by King Tubby, Lee "Scratch" Perry, and Jah Shaka:
      - 4 Authentic Dub Siren waveforms (Laser Sweep, UFO Warble, Hi-Lo Klaxon, Machine Gun)
      - Decimated 256-byte circular tape delay buffer with non-linear feedback saturation
      - Infinite feedback runaway oscillation for classic dub sound-system madness
      - Real-time modulation rate, pitch, and echo feedback controls
      - Tri-color Sound-System VU meter & Rastafari visualizers

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Siren Modulation Speed & Base Pitch)
      PB2 (A1): Right Pot (Tape Delay Time & Feedback Regeneration)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Siren Types (Laser Sweep, UFO Warble, Klaxon, Machine Gun)
                - Right Button: Cycle Visualizers (Sound System VU, Space Radar, Rasta Waves, Echo Rings)
                - Both Buttons: Airhorn & Infinite Echo Blast!
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
volatile uint16_t phase_osc       = 0;
volatile uint16_t phase_lfo       = 0;
volatile uint16_t base_pitch      = 1200;
volatile uint16_t lfo_rate        = 80;

// Lo-Fi Decimated Delay Line (128 samples @ 1kHz = ~130 ms tape echo)
// 128 bytes keeps SRAM low (189B / 36%), leaving ample heap for Adafruit_NeoPixel malloc(60)!
#define DELAY_BUF_SIZE 128
volatile uint8_t  echo_buffer[DELAY_BUF_SIZE];
volatile uint8_t  echo_write_idx  = 0;
volatile uint8_t  delay_length    = 90;  // Controlled by POTI_RIGHT
volatile uint8_t  feedback_gain   = 160; // 0..255 (values > 220 = runaway!)
volatile uint8_t  delay_decim     = 0;

// Modes & Controls
volatile uint8_t  siren_type      = 0; // 0=Laser Sweep, 1=UFO Warble, 2=Klaxon, 3=Machine Gun
volatile uint8_t  visual_mode     = 0; // 0=SoundSystem VU, 1=Radar, 2=Rasta Waves, 3=Echo Rings
volatile uint8_t  airhorn_blast   = 0; // Both buttons held
volatile uint16_t ticks           = 0;
volatile uint8_t  last_out        = 128;
volatile uint8_t  vu_level        = 0;

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

// --- 10kHz Audio Interrupt ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // 1. LFO Modulation
  phase_lfo += lfo_rate;
  uint8_t lfo_ramp = phase_lfo >> 8;
  int16_t pitch_mod = 0;

  if (siren_type == 0) {
    // 0: Laser Sweep (Sawtooth ramp down)
    pitch_mod = ((int16_t)(255 - lfo_ramp) * 12);
  } else if (siren_type == 1) {
    // 1: UFO Warble (Triangle fast vibrato)
    uint8_t tri = (lfo_ramp < 128) ? (lfo_ramp << 1) : ((255 - lfo_ramp) << 1);
    pitch_mod = ((int16_t)tri - 128) * 8;
  } else if (siren_type == 2) {
    // 2: Hi-Lo Police Klaxon (Square alternation)
    pitch_mod = (lfo_ramp < 128) ? 800 : -600;
  } else {
    // 3: Machine Gun 1-Shot Dub Bleep (Pulsed chirps)
    if (lfo_ramp < 80) pitch_mod = (80 - lfo_ramp) * 18;
    else pitch_mod = -3000; // Silence between bursts
  }

  // Airhorn override
  if (airhorn_blast) {
    pitch_mod = 2500 + ((ticks & 64) ? 400 : 0);
  }

  int32_t eff_pitch = (int32_t)base_pitch + pitch_mod;
  if (eff_pitch < 30) eff_pitch = 0;

  // 2. Siren Oscillator (Square / Pulse)
  uint8_t dry = 128;
  if (eff_pitch > 0) {
    phase_osc += (uint16_t)eff_pitch;
    dry = (phase_osc < 32768) ? 230 : 25;
  }

  // 3. Vintage Tape Echo Emulation (Decimated to ~1.25 kHz)
  delay_decim++;
  if ((delay_decim & 7) == 0) {
    // Read from delayed tap
    uint8_t read_idx = (echo_write_idx - delay_length) & 127;
    uint8_t echo_sample = echo_buffer[read_idx];

    // Saturated feedback mixing: in + (echo * gain)
    uint16_t gain = airhorn_blast ? 245 : feedback_gain;
    int16_t fb_sig = ((int16_t)echo_sample - 128);
    int16_t mixed = ((int16_t)dry - 128) + ((fb_sig * gain) >> 8);

    // Tape saturation soft-clipping
    if (mixed > 110) mixed = 110;
    else if (mixed < -110) mixed = -110;

    echo_buffer[echo_write_idx] = (uint8_t)(mixed + 128);
    echo_write_idx = (echo_write_idx + 1) & 127;
  }

  // 4. Mix Dry Siren + Wet Echo
  uint8_t read_idx = (echo_write_idx - delay_length) & 127;
  uint8_t wet = echo_buffer[read_idx];
  uint16_t final_mix = ((uint16_t)dry + ((uint16_t)wet << 1)) / 3;

  last_out = (uint8_t)final_mix;
  OCR1A = last_out;

  // VU Peak Meter Decay
  if ((ticks & 63) == 0) {
    uint8_t amp = abs((int16_t)last_out - 128);
    if (amp > vu_level) vu_level = amp;
    else if (vu_level > 2) vu_level -= 2;
  }
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (airhorn_blast) {
    // Airhorn Flash: Sound-system yellow & green emergency blast!
    uint32_t blast_col = (ticks & 32) ? pixels.Color(255, 230, 0) : pixels.Color(0, 255, 30);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, blast_col);
    }
    pixels.show();
    return;
  }

  if (visual_mode == 0) {
    // Mode 0: Sound System Tri-Color VU Meter (Green / Gold / Red)
    // 5 vertical columns dancing to the dub echo amplitude!
    uint8_t bars = (vu_level * 5) / 100;
    if (bars > 4) bars = 4;

    for (uint8_t x = 0; x < 5; x++) {
      for (uint8_t y = 0; y < 4; y++) {
        uint8_t idx = matrixIndex(x, 3 - y);
        if (y < bars) {
          uint32_t col = (y == 3) ? pixels.Color(255, 0, 0) :   // Red peak
                         (y == 2) ? pixels.Color(255, 200, 0) : // Gold mid
                                    pixels.Color(0, 255, 40);   // Green base
          pixels.setPixelColor(idx, col);
        } else {
          pixels.setPixelColor(idx, pixels.Color(5, 10, 5));
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Jamaican Space Radar Scanner
    uint8_t sweep = (phase_lfo >> 13) % 5;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == sweep) {
          pixels.setPixelColor(idx, pixels.Color(0, 240, 255)); // Scanning beam
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 15, 30));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Rastafari Waves (Red, Gold, Green flowing horizontal bands)
    for (uint8_t y = 0; y < 4; y++) {
      uint32_t rasta_col = (y == 0) ? pixels.Color(255, 10, 10) : // Red
                           (y == 1) ? pixels.Color(255, 210, 0) : // Gold
                                      pixels.Color(0, 230, 30);   // Green
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t wave = (x + (ticks >> 7)) & 3;
        pixels.setPixelColor(idx, (wave == 0) ? pixels.Color(255, 255, 255) : rasta_col);
      }
    }
  } else {
    // Mode 3: Echo Rings (Concentric ripples expanding with delay taps)
    uint8_t ring = (echo_write_idx >> 6) & 3;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        bool edge = (x == 0 || x == 4 || y == 0 || y == 3);
        if (edge && ring == 0) {
          pixels.setPixelColor(idx, pixels.Color(255, 0, 150));
        } else if (!edge && ring == 1) {
          pixels.setPixelColor(idx, pixels.Color(0, 220, 255));
        } else {
          pixels.setPixelColor(idx, pixels.Color(10, 5, 20));
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
  pixels.setBrightness(65);
  pixels.show();

  // Clear delay buffer
  for (uint16_t i = 0; i < DELAY_BUF_SIZE; i++) {
    echo_buffer[i] = 128;
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
  uint16_t pot_l = analogRead(POTI_LEFT);  // Siren Pitch & Speed
  uint16_t pot_r = analogRead(POTI_RIGHT); // Echo Delay Time & Feedback

  // Base pitch: 200 Hz up to 2.4 kHz
  base_pitch = 600 + ((uint32_t)pot_l * 3800) / 1023;
  // LFO rate: ~0.5 Hz up to 18 Hz
  lfo_rate   = 15  + ((uint32_t)pot_l * 180) / 1023;

  // Echo delay time: 15 to 120 samples (~15ms to ~130ms)
  delay_length  = 15  + ((uint32_t)pot_r * 105) / 1023;
  // Feedback gain: up to runaway saturation (235)
  feedback_gain = 80  + ((uint32_t)pot_r * 155) / 1023;

  // 2. Read Button Resistor Ladder (PB3 / A3)
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons: Airhorn & Infinite Echo Blast!
    airhorn_blast = 1;
  } else {
    airhorn_blast = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Siren Types
      siren_type = (siren_type + 1) & 3;
      uint32_t badge = (siren_type == 0) ? pixels.Color(255, 0, 0) :   // Laser (Red)
                       (siren_type == 1) ? pixels.Color(0, 240, 255) : // UFO (Cyan)
                       (siren_type == 2) ? pixels.Color(255, 200, 0) : // Klaxon (Gold)
                                           pixels.Color(0, 255, 50);   // Machine Gun (Green)
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
