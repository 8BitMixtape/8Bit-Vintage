/* ---------------------------------------------------------------------------------------
    INDUSTRIAL CYBER-DRONE — 4-NODE ORGANISMIC NOISE SYNTHESIZER
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An organismic harsh noise and ambient cyber-drone synthesizer inspired by
    the SOMA Lyra-8, Throbbing Gristle, and harsh industrial soundscapes:
      - 4 non-linear cross-modulating voice nodes in an organic feedback ring
      - Non-linear diode wavefolding distortion and granular LFSR noise injection
      - 1-pole screaming resonant filter simulation with self-oscillating feedback
      - Organic breathing drones, seismic sub-rumble, and wall-of-sound industrial noise
      - 4 Reactive visuals including Nuclear Hazard Trefoil & Geiger Particle Sparks

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Master FM Pitch Cluster & Modulation Spread)
      PB2 (A1): Right Pot (Distortion Overdrive & Filter Screaming Resonance)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Drone Modes (Organismic Ambient, Harsh Industrial, Geiger Glitch, Seismic Rumble)
                - Right Button: Cycle Visualizers (Nuclear Trefoil, Geiger Sparks, Plasma Vortex, Seismic Fault)
                - Both Buttons: Nuclear Meltdown Overdrive & Radiation Strobe!
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

// --- Synth State (4-Node FM Ring Network) ---
// Sample rate = 10,000 Hz
volatile uint16_t phase1        = 0;
volatile uint16_t phase2        = 0;
volatile uint16_t phase3        = 0;
volatile uint16_t phase4        = 0;

volatile uint16_t base_freq1    = 180;
volatile uint16_t base_freq2    = 240;
volatile uint16_t base_freq3    = 320;
volatile uint16_t base_freq4    = 410;

volatile uint8_t  fm_depth      = 80;
volatile uint8_t  drive_amt     = 100;
volatile uint8_t  filter_cutoff = 190;
volatile uint8_t  filter_res    = 50;

// Filter state
volatile int16_t  flt_lp        = 0;
volatile int16_t  flt_bp        = 0;

// Modes & Controls
volatile uint8_t  drone_mode    = 0; // 0=Ambient, 1=Harsh Industrial, 2=Geiger, 3=Seismic
volatile uint8_t  visual_mode   = 0; // 0=Trefoil, 1=Geiger, 2=Plasma, 3=Seismic
volatile uint8_t  meltdown_mode = 0; // Both buttons held
volatile uint16_t ticks         = 0;
volatile uint16_t trigger_count = 0;

// 16-bit Galois LFSR noise generator
uint16_t lfsr = 0xB482;

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

// Inline triangle wave calculator
static inline uint8_t get_triangle(uint16_t phase) {
  uint8_t t = phase >> 8;
  return (t < 128) ? (t << 1) : ((255 - t) << 1);
}

// --- 10kHz Audio Interrupt ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // 1. Noise Generator
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }
  uint8_t noise = lfsr & 0xFF;

  // 2. 4-Node Cross-FM Intermodulation Loop
  // Calculate output of nodes
  uint8_t out1 = get_triangle(phase1);
  uint8_t out2 = get_triangle(phase2);
  uint8_t out3 = get_triangle(phase3);
  uint8_t out4 = get_triangle(phase4);

  // Cross-modulate frequencies: 1->2->3->4->1 (Ring topology)
  uint8_t depth = meltdown_mode ? 255 : fm_depth;
  int16_t mod1 = ((int16_t)out4 - 128) * depth >> 7;
  int16_t mod2 = ((int16_t)out1 - 128) * depth >> 7;
  int16_t mod3 = ((int16_t)out2 - 128) * depth >> 7;
  int16_t mod4 = ((int16_t)out3 - 128) * depth >> 7;

  if (drone_mode == 2) {
    // Geiger Mode: Inject random noise spikes into Node 1 & 4
    if (noise > 220) mod1 += 800;
    if (noise < 30)  mod4 -= 600;
  }

  int32_t f1 = (int32_t)base_freq1 + mod1;
  int32_t f2 = (int32_t)base_freq2 + mod2;
  int32_t f3 = (int32_t)base_freq3 + mod3;
  int32_t f4 = (int32_t)base_freq4 + mod4;

  if (f1 < 10) f1 = 10;
  if (f2 < 10) f2 = 10;
  if (f3 < 10) f3 = 10;
  if (f4 < 10) f4 = 10;

  phase1 += (uint16_t)f1;
  phase2 += (uint16_t)f2;
  phase3 += (uint16_t)f3;
  phase4 += (uint16_t)f4;

  // 3. Node Summation & Mode Coloration
  uint16_t sum = ((uint16_t)out1 + out2 + out3 + out4) >> 2;

  if (drone_mode == 1) {
    // Harsh Industrial: Hard wavefold & diode clipping
    sum = sum ^ (out2 >> 1);
    if (sum > 190) sum = 255 - sum;
  } else if (drone_mode == 3) {
    // Seismic Rumble: Sub-octave division & heavy low-end bias
    sum = ((sum << 1) + out1) / 3;
  }

  // 4. Overdrive & Non-Linear Saturation
  uint8_t drive = meltdown_mode ? 240 : drive_amt;
  int16_t sat = ((int16_t)sum - 128);
  sat = (sat * (128 + drive)) >> 7;
  if (sat > 115) sat = 115;
  else if (sat < -115) sat = -115;
  uint8_t driven = (uint8_t)(sat + 128);

  // 5. Screaming Resonant State Variable Filter
  uint8_t cut = meltdown_mode ? 240 : filter_cutoff;
  uint8_t res = meltdown_mode ? 130 : filter_res;
  int16_t in = (int16_t)driven - 128;
  int16_t res_in = in - (((int32_t)flt_bp * res) >> 8);
  flt_lp += (((int32_t)(res_in - flt_lp)) * cut) >> 8;
  flt_bp = res_in - flt_lp;

  int16_t final_sig = (drone_mode == 1) ? (flt_bp + 128) : (flt_lp + 128);
  if (final_sig < 0) final_sig = 0;
  else if (final_sig > 255) final_sig = 255;

  OCR1A = (uint8_t)final_sig;
}

// 5x4 Nuclear Trefoil Symbol Bitmap
const uint8_t nuclear_trefoil_bmp[20] PROGMEM = {
  1, 0, 1, 0, 1,
  0, 1, 1, 1, 0,
  1, 1, 1, 1, 1,
  0, 1, 0, 1, 0
};

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (meltdown_mode) {
    // Nuclear Meltdown Strobe: Emergency Red / Warning Yellow siren strobe!
    uint32_t alert = (ticks & 32) ? pixels.Color(255, 0, 0) : pixels.Color(255, 220, 0);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, alert);
    }
    pixels.show();
    return;
  }

  if (visual_mode == 0) {
    // Mode 0: Nuclear Hazard Trefoil Symbol
    // Pulsing nuclear yellow & black trefoil reflecting the drone amplitude!
    uint8_t pulse = 180 + ((ticks >> 5) & 63);
    uint32_t nuke_yellow = pixels.Color(pulse, pulse - 30, 0);
    uint32_t dark_bg     = pixels.Color(15, 8, 0);

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t is_part = pgm_read_byte(&nuclear_trefoil_bmp[y * 5 + x]);
        pixels.setPixelColor(idx, is_part ? nuke_yellow : dark_bg);
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Geiger Counter Ionizing Particle Sparks
    // Green radiation field with chaotic amber ionization strikes!
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t p = (lfsr + x * 41 + y * 67) & 255;
        if (p > 230) {
          pixels.setPixelColor(idx, pixels.Color(255, 220, 100)); // Ionization spark
        } else if (p > 170) {
          pixels.setPixelColor(idx, pixels.Color(0, 255, 50));   // Radiation green
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 15, 5));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Cyber Plasma Vortex (Swirling FM Attractor Orbit)
    uint8_t cx = (phase1 >> 13) % 5;
    uint8_t cy = (phase2 >> 14) % 4;

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t dist = abs((int8_t)x - (int8_t)cx) + abs((int8_t)y - (int8_t)cy);
        if (dist == 0) {
          pixels.setPixelColor(idx, pixels.Color(255, 255, 255)); // Vortex Core
        } else if (dist == 1) {
          pixels.setPixelColor(idx, pixels.Color(255, 0, 180));   // Plasma Ring
        } else {
          pixels.setPixelColor(idx, pixels.Color(30, 0, 60));
        }
      }
    }
  } else {
    // Mode 3: Seismic Fault Line
    // Shaking tectonic fracture line reflecting sub-drone frequencies
    uint8_t fault_y = (phase3 >> 14) % 4;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (y == fault_y) {
          pixels.setPixelColor(idx, pixels.Color(255, 40, 0)); // Molten magma fault
        } else {
          pixels.setPixelColor(idx, pixels.Color(20, 5, 0));
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
  uint16_t pot_l = analogRead(POTI_LEFT);  // FM Pitch Cluster & Modulation Spread
  uint16_t pot_r = analogRead(POTI_RIGHT); // Overdrive Saturation & Resonant Filter

  // Master Pitch: low seismic drone (~30 Hz) up to shrieking industrial whine (~1.6 kHz)
  uint32_t p_sq = ((uint32_t)pot_l * pot_l) >> 10;
  uint16_t master_p = 120 + (p_sq * 11) / 1;

  base_freq1 = master_p;
  base_freq2 = (master_p * 13) / 10;
  base_freq3 = (master_p * 17) / 10;
  base_freq4 = (master_p * 22) / 10;

  fm_depth      = 20 + ((uint32_t)pot_l * 200) / 1023;
  drive_amt     = 30 + ((uint32_t)pot_r * 190) / 1023;
  filter_cutoff = 70 + ((uint32_t)pot_r * 160) / 1023;
  filter_res    = 30 + ((uint32_t)pot_r * 75) / 1023;

  // 2. Read Button Resistor Ladder (PB3 / A3)
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons: Nuclear Meltdown Overdrive!
    meltdown_mode = 1;
  } else {
    meltdown_mode = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Drone Modes
      drone_mode = (drone_mode + 1) & 3;
      uint32_t badge = (drone_mode == 0) ? pixels.Color(0, 220, 255) : // Ambient (Cyan)
                       (drone_mode == 1) ? pixels.Color(255, 0, 0) :   // Industrial (Red)
                       (drone_mode == 2) ? pixels.Color(255, 220, 0) : // Geiger (Gold)
                                           pixels.Color(255, 0, 180);  // Seismic (Magenta)
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
