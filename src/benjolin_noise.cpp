/* ---------------------------------------------------------------------------------------
    BENJOLIN / RUNGLER CHAOTIC NOISE SYNTHESIZER
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic digital tribute to Rob Hordijk's legendary "Benjolin" chaotic
    modular instrument:
      - Dual cross-modulating oscillators (VCO1 and VCO2)
      - The "Rungler": 8-bit shift register clocked by VCO1 and fed by VCO2
      - 3-bit Rungler DAC producing stepped chaotic feedback CV
      - Resonant low-pass / band-pass filter modulated by the Rungler
      - Generative rhythm loops, hypnotic alien chatter, and total noise meltdown!

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (VCO1 Frequency / Master Clock: 2 Hz - 2.5 kHz)
      PB2 (A1): Right Pot (VCO2 Frequency / Cross-Modulation & Filter Cutoff)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Synthesis Modes (Classic Rungler, Industrial Drone, Screaming Filter, Bit-Slicer)
                - Right Button: Cycle Visualizers (Shift Register, Cellular Automaton, Lissajous, Chaos Radar)
                - Both Buttons: Entropy Injection & Maximum Feedback Meltdown!
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

// --- Synth State (Benjolin DDS & Rungler) ---
// Sample rate = 10,000 Hz
volatile uint16_t phase1        = 0;
volatile uint16_t phase2        = 0;
volatile uint16_t base_freq1    = 300;
volatile uint16_t base_freq2    = 450;

// The Rungler 8-bit shift register & 3-bit DAC
volatile uint8_t  rungler_sr    = 0xAC; // Initial non-zero seed
volatile uint8_t  rungler_val   = 3;    // 3-bit stepped CV (0..7)
volatile uint8_t  rungler_fb    = 120;  // Feedback modulation depth

// Resonant filter state
volatile uint8_t  filter_cutoff = 180;
volatile uint8_t  filter_res    = 60;
volatile int16_t  flt_lp        = 0;
volatile int16_t  flt_bp        = 0;

// Modes & Controls
volatile uint8_t  synth_mode    = 0; // 0=Classic, 1=Drone, 2=Screaming, 3=Slicer
volatile uint8_t  visual_mode   = 0; // 0=ShiftReg, 1=Cellular, 2=Lissajous, 3=Radar
volatile uint8_t  entropy_inject= 0; // 1=Chaos button held
volatile uint16_t ticks         = 0;
volatile uint16_t step_count    = 0;

// Fast pseudo-noise generator
uint16_t lfsr = 0xA513;

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

  // 1. Calculate Rungler Modulation CV
  // 3-bit DAC decoding: Bit 0 (weight 4) + Bit 1 (weight 2) + Bit 2 (weight 1)
  uint8_t r_bits = ((rungler_sr & 0x01) << 2) | (rungler_sr & 0x02) | ((rungler_sr & 0x04) >> 2);
  rungler_val = r_bits; // 0..7

  int16_t cv_mod = ((int16_t)r_bits - 3) * (int16_t)rungler_fb;

  // 2. Frequency Cross-Modulation
  int32_t f1 = (int32_t)base_freq1 + cv_mod;
  if (f1 < 20) f1 = 20;

  int32_t f2 = (int32_t)base_freq2 + (cv_mod >> 1);
  if (synth_mode == 1) {
    // Industrial Drone: Extreme cross-frequency intermodulation
    f2 += ((int16_t)get_triangle(phase1) - 128) * 8;
  }
  if (f2 < 20) f2 = 20;

  // 3. Oscillator Phase Accumulation & Rungler Clock Trigger
  uint16_t prev_p1 = phase1;
  phase1 += (uint16_t)f1;
  bool vco1_clock = (phase1 < prev_p1); // VCO1 overflow clock edge!

  phase2 += (uint16_t)f2;

  // 4. The Rungler Shift Register Operation
  if (vco1_clock) {
    step_count++;
    // Data bit taken from VCO2 comparator (XORed with entropy if active)
    uint8_t data_bit = (phase2 < 32768) ? 1 : 0;
    if (entropy_inject) {
      if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
      else { lfsr >>= 1; }
      data_bit ^= (lfsr & 1);
    }
    // Shift left and insert data bit into LSB
    rungler_sr = (rungler_sr << 1) | data_bit;
  }

  // 5. Waveform Synthesis
  uint8_t tri1 = get_triangle(phase1);
  uint8_t tri2 = get_triangle(phase2);
  uint8_t sq1  = (phase1 < 32768) ? 240 : 15;
  uint8_t sq2  = (phase2 < 32768) ? 240 : 15;

  uint8_t raw = 128;
  if (synth_mode == 0) {
    // Classic Benjolin: Pulse & Triangle mix with XOR ring modulation
    raw = ((tri1 + tri2) >> 1) ^ (rungler_sr & 0x80 ? 0x20 : 0x00);
  } else if (synth_mode == 1) {
    // Industrial Drone: Heavy wavefolded saturation
    raw = (tri1 ^ sq2) + (r_bits << 4);
  } else if (synth_mode == 2) {
    // Screaming Resonant: Triangle through high-resonance filter
    raw = (tri1 ^ tri2);
  } else {
    // Bit-Slicer / S&H Glitch
    uint8_t slice = (r_bits * 36);
    raw = (sq1 & slice) | (sq2 & ~slice);
  }

  // 6. Resonant Filter (MOS/Buchla State Variable Simulation)
  uint8_t cut = filter_cutoff + (r_bits << 3);
  int16_t in = (int16_t)raw - 128;
  int16_t res_in = in - (((int32_t)flt_bp * filter_res) >> 8);
  flt_lp += (((int32_t)(res_in - flt_lp)) * cut) >> 8;
  flt_bp = res_in - flt_lp;

  int16_t out_sig;
  if (synth_mode == 2) {
    out_sig = flt_bp + 128; // Band-pass screaming bite
  } else {
    out_sig = flt_lp + 128; // Low-pass warm body
  }

  if (out_sig < 0) out_sig = 0;
  else if (out_sig > 255) out_sig = 255;

  OCR1A = (uint8_t)out_sig;
}

// --- NeoPixel Visualizers ---
void update_visuals() {
  if (entropy_inject) {
    // Entropy Meltdown: White/Gold lightning noise storm!
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      uint8_t spark = (lfsr + i * 37) & 255;
      pixels.setPixelColor(i, (spark > 180) ? pixels.Color(255, 240, 200) : pixels.Color(spark, 50, 0));
    }
    pixels.show();
    return;
  }

  if (visual_mode == 0) {
    // Mode 0: The Rungler 8-Bit Shift Register Display
    // Top 2 rows show the 8 bits of the shift register in real-time!
    // Bottom 2 rows show the 3-bit DAC stepped level (0..7) as an analog bar
    uint8_t sr = rungler_sr;
    uint8_t dac = rungler_val;

    for (uint8_t i = 0; i < 8; i++) {
      uint8_t x = i % 4 + (i >= 4 ? 1 : 0);
      uint8_t y = i / 4;
      uint8_t idx = matrixIndex(x, y);
      bool bit = (sr >> (7 - i)) & 1;
      pixels.setPixelColor(idx, bit ? pixels.Color(0, 255, 200) : pixels.Color(5, 20, 15));
    }
    // DAC stepped bar across bottom rows
    for (uint8_t x = 0; x < 5; x++) {
      for (uint8_t y = 2; y < 4; y++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t bar_pos = x + (y == 3 ? 0 : 5);
        if (bar_pos <= (dac + 1)) {
          pixels.setPixelColor(idx, pixels.Color(255, 120, 0));
        } else {
          pixels.setPixelColor(idx, pixels.Color(15, 8, 0));
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: 1D Cellular Automaton (Rule 30 chaotic growth)
    static uint8_t ca_cells[5] = {0, 0, 1, 0, 0};
    static uint8_t ca_history[4][5];
    static uint8_t last_step = 0;

    if ((uint8_t)step_count != last_step) {
      last_step = (uint8_t)step_count;
      // Scroll history downwards
      for (uint8_t y = 3; y > 0; y--) {
        for (uint8_t x = 0; x < 5; x++) ca_history[y][x] = ca_history[y - 1][x];
      }
      // Compute Rule 30 for next row
      uint8_t next[5];
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t left  = (x > 0) ? ca_cells[x - 1] : ca_cells[4];
        uint8_t mid   = ca_cells[x];
        uint8_t right = (x < 4) ? ca_cells[x + 1] : ca_cells[0];
        uint8_t rule30 = left ^ (mid | right); // Rule 30 logic
        next[x] = rule30 ^ (rungler_sr & 1);   // Perturbed by Rungler!
      }
      for (uint8_t x = 0; x < 5; x++) {
        ca_cells[x] = next[x];
        ca_history[0][x] = next[x];
      }
    }

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        pixels.setPixelColor(idx, ca_history[y][x] ? pixels.Color(255, 0, 100) : pixels.Color(10, 0, 20));
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Lissajous Phase Plot (VCO1 vs VCO2 chaotic orbit)
    uint8_t px = (phase1 >> 13) % 5;
    uint8_t py = (phase2 >> 14) % 4;

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == px && y == py) {
          pixels.setPixelColor(idx, pixels.Color(255, 255, 255)); // Glowing attractor cursor
        } else if (abs((int8_t)x - (int8_t)px) <= 1 && abs((int8_t)y - (int8_t)py) <= 1) {
          pixels.setPixelColor(idx, pixels.Color(0, 150, 255));   // Electron halo
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 10, 25));
        }
      }
    }
  } else {
    // Mode 3: Chaos Radar Scanner
    uint8_t angle = (ticks >> 6) % 5;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        if (x == angle) {
          pixels.setPixelColor(idx, pixels.Color(0, 255, 50)); // Phosphor green sweep
        } else if ((rungler_sr >> (x + y)) & 1) {
          pixels.setPixelColor(idx, pixels.Color(0, 80, 20));  // Blip
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 15, 5));
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
  uint16_t pot_l = analogRead(POTI_LEFT);  // VCO1 Frequency / Master Clock
  uint16_t pot_r = analogRead(POTI_RIGHT); // VCO2 Frequency / Filter & Feedback

  // VCO1 Frequency: low rhythmic tick (~2 Hz, inc=13) up to ~2.5 kHz (inc=16384)
  uint32_t p1_sq = ((uint32_t)pot_l * pot_l) >> 10;
  base_freq1 = 15 + (p1_sq * 16) / 1;

  // VCO2 Frequency & Feedback
  uint32_t p2_sq = ((uint32_t)pot_r * pot_r) >> 10;
  base_freq2 = 40 + (p2_sq * 20) / 1;

  rungler_fb    = 30  + ((uint32_t)pot_r * 220) / 1023;
  filter_cutoff = 80  + ((uint32_t)pot_r * 140) / 1023;
  filter_res    = 40  + ((uint32_t)pot_l * 70) / 1023;

  // 2. Read Button Resistor Ladder (PB3 / A3)
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons: Entropy Injection & Feedback Overdrive!
    entropy_inject = 1;
    rungler_fb = 250;
    filter_res = 120;
  } else {
    entropy_inject = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Synthesis Modes
      synth_mode = (synth_mode + 1) & 3;
      uint32_t badge = (synth_mode == 0) ? pixels.Color(0, 255, 200) : // Classic Rungler (Cyan)
                       (synth_mode == 1) ? pixels.Color(255, 100, 0) : // Industrial Drone (Amber)
                       (synth_mode == 2) ? pixels.Color(255, 0, 150) : // Screaming Filter (Magenta)
                                           pixels.Color(0, 255, 50);   // Bit-Slicer (Green)
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
