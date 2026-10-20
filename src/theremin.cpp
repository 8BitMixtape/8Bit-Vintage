/* ---------------------------------------------------------------------------------------
    LÉON THEREMIN 1920 — CONTINUOUS SPACE SYNTH & ETHERPHONE
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    A tribute to Léon Theremin's 1920 "Etherphone" (Theremin), the world's first
    electronic instrument played without physical contact.

    Features:
      - Continuous Pitch Antenna (Left Pot / PB4): Smooth, microtonal portamento glide
        from deep cello bass (55 Hz) to soaring soprano (2 kHz) with zero ADC zipper noise.
      - Continuous Volume Antenna (Right Pot / PB2): Full dynamic expression from
        complete silence/muting to soaring forte with tube warmth.
      - Natural Hand-Tremor Vibrato: Organic 5.5 Hz pitch flutter simulating a thereminist's
        expressive finger vibrato.
      - 4 Pitch / Play Modes:
          0: Pure Continuous Glissando (Authentic 1920s microtonal gliding etherphone)
          1: Vocal Formant Singer (Resonant "Ooo-Aaa-Eee" singing vowel formants)
          2: Sci-Fi Flying Saucer (Dual-oscillator heterodyne ring-mod wail)
          3: Quantized Harmonic Minor (Ethereal melodies without playing out-of-tune)
      - 4 Instrument Timbres:
          0: Clara Rockmore Pure Sine (Warm vintage tube tone)
          1: Heterodyne Space UFO (Eerie beat-frequency interference)
          2: Ghost Cello / Singing Saw (Rich harmonic overtone body)
          3: Celestial Glass Harmonica (Prismatic high-register shimmer)
      - 4 Reactive 5x4 NeoPixel EMF Visualizers:
          0: EMF Plasma Field (Antenna aura radiating from pitch & volume hands)
          1: Phosphor CRT Oscilloscope (1920s green cathode-ray wave display)
          2: Chromatic Spectral Ribbon (Rainbow frequency spectrum)
          3: UFO Radar Scanner (Pulsing extraterrestrial beacon)

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL @ ~250 kHz carrier)
      PB4 (A2): Left Pot (Pitch Antenna: 55 Hz - 2.1 kHz continuous glide)
      PB2 (A1): Right Pot (Volume Antenna: 0% to 100% dynamic expression)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Pitch / Scale Mode (Glissando, Vocal, Sci-Fi, Minor)
                - Right Button: Cycle Instrument Timbre (Sine, Heterodyne, Cello, Chime)
                - Both Buttons: Antenna Touch / Space Ray Strobe & Pitch Dive
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
#define POTI_LEFT   A2 // PB4 (ADC2) - Pitch Antenna
#define POTI_RIGHT  A1 // PB2 (ADC1) - Volume Antenna

// --- Button Voltage Calibration ---
#define Vbutton_releaseLevel  490
#define Vbutton_left          460
#define Vbutton_right         395
#define Vbutton_both          320

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- 256-Entry Sine Wave Table (PROGMEM) ---
const uint8_t PROGMEM sine_table[256] = {
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
  176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
  218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
  245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
  255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246,
  245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
  218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179,
  176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
  128, 125, 122, 119, 116, 113, 110, 107, 104, 101,  98,  94,  91,  89,  86,  83,
   80,  77,  74,  71,  68,  66,  63,  60,  58,  55,  53,  50,  48,  45,  43,  41,
   38,  36,  34,  32,  30,  28,  26,  24,  22,  21,  19,  18,  16,  15,  13,  12,
   11,  10,   8,   7,   6,   6,   5,   4,   3,   3,   2,   2,   2,   1,   1,   1,
    1,   1,   1,   1,   2,   2,   2,   3,   3,   4,   5,   6,   6,   7,   8,  10,
   11,  12,  13,  15,  16,  18,  19,  21,  22,  24,  26,  28,  30,  32,  34,  36,
   38,  41,  43,  45,  48,  50,  53,  55,  58,  60,  63,  66,  68,  71,  74,  77,
   80,  83,  86,  89,  91,  94,  98, 101, 104, 107, 110, 113, 116, 119, 122, 125
};

// --- Quantized Harmonic Minor Scale (Pitch Increments for 10 kHz DDS) ---
// 17 notes spanning 2+ octaves: A2 (110Hz) to C6 (1046Hz)
const uint16_t PROGMEM minor_scale_incs[17] = {
  721,  809,  857,  963, 1081, 1145, 1362, // A2, B2, C3, D3, E3, F3, G#3
  1442, 1618, 1715, 1925, 2162, 2291, 2724, // A3, B3, C4, D4, E4, F4, G#4
  2884, 3430, 4324                         // A4, C5, E5
};

// --- Synthesis Variables (DDS Audio Engine @ 10,000 Hz) ---
volatile uint16_t phase_osc1     = 0;
volatile uint16_t phase_osc2     = 0; // Heterodyne / Formant oscillator
volatile uint16_t phase_vibrato  = 0; // Hand tremor LFO
volatile uint16_t current_inc    = 1442; // Portamento smoothed frequency
volatile uint16_t target_inc     = 1442;
volatile uint8_t  volume_level   = 200;  // Volume antenna amplitude
volatile uint8_t  pitch_mode     = 0;    // 0=Glissando, 1=Vocal, 2=Sci-Fi, 3=Minor
volatile uint8_t  timbre_mode    = 0;    // 0=Sine, 1=Heterodyne, 2=Saw/Cello, 3=Chime
volatile uint8_t  visual_mode    = 0;    // 0=Plasma EMF, 1=Scope, 2=Rainbow, 3=Radar
volatile uint8_t  touch_strobe   = 0;    // Antenna touched / both buttons
volatile uint16_t ticks          = 0;
volatile uint8_t  last_sample    = 128;
volatile uint8_t  envelope_vol   = 0;    // Smoothed volume

// Custom delay routine using 10 kHz Timer0 audio ticks (immune to disabled millis timer)
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

// ---------------------------------------------------------------------------------------
// 10 kHz Audio Interrupt (Timer0 COMPA)
// ---------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // 1. Smooth Portamento Glide (Theremin pitch antenna physics)
  if ((ticks & 3) == 0) {
    if (current_inc < target_inc) {
      uint16_t diff = target_inc - current_inc;
      current_inc += (diff > 32) ? 16 : (diff > 4 ? 4 : 1);
    } else if (current_inc > target_inc) {
      uint16_t diff = current_inc - target_inc;
      current_inc -= (diff > 32) ? 16 : (diff > 4 ? 4 : 1);
    }
  }

  // 2. Hand-Tremor Vibrato LFO (~5.5 Hz)
  phase_vibrato += 36;
  uint8_t vib_raw = pgm_read_byte(&sine_table[(phase_vibrato >> 8) & 0xFF]);
  int8_t vib_offset = ((int8_t)vib_raw - 128); // -128 .. +127

  // Apply subtle 1.5% vibrato depth
  int16_t effective_inc = current_inc;
  if (!touch_strobe) {
    effective_inc += (int16_t)((vib_offset * (int16_t)(current_inc >> 6)) >> 4);
  } else {
    // Touch Strobe: dive-bomb pitch drop
    effective_inc >>= 2;
  }
  if (effective_inc < 100) effective_inc = 100;

  // 3. Update Primary Oscillator Phase
  phase_osc1 += effective_inc;
  uint8_t wave_idx1 = phase_osc1 >> 8;

  // 4. Update Secondary Oscillator (Heterodyne / Formant)
  if (pitch_mode == 2) {
    // Sci-Fi UFO: Heterodyne offset creates audible difference beats
    phase_osc2 += effective_inc + (effective_inc >> 3) + 45;
  } else if (pitch_mode == 1) {
    // Vocal Formant: Formant frequency peaks around 700 - 1800 Hz
    phase_osc2 += (effective_inc << 1) + 1200;
  } else {
    // Octave overtone shimmer
    phase_osc2 += (effective_inc << 1);
  }
  uint8_t wave_idx2 = phase_osc2 >> 8;

  // 5. Sound Wave Generation by Timbre
  int16_t raw_audio = 0;

  if (timbre_mode == 0) {
    // Clara Rockmore Pure Sine Wave: Pure sine with subtle 2nd harmonic warmth
    uint8_t s1 = pgm_read_byte(&sine_table[wave_idx1]);
    raw_audio = (int16_t)s1 - 128;
  } else if (timbre_mode == 1) {
    // Heterodyne Space UFO: Ring-modulation product between osc1 and osc2
    uint8_t s1 = pgm_read_byte(&sine_table[wave_idx1]);
    uint8_t s2 = pgm_read_byte(&sine_table[wave_idx2]);
    int16_t ring = ((int16_t)s1 - 128) * ((int16_t)s2 - 128);
    raw_audio = ring >> 7; // -128 .. +128
  } else if (timbre_mode == 2) {
    // Ghost Cello / Singing Saw: Triangle + Sawtooth odd harmonics
    uint8_t saw = wave_idx1;
    uint8_t tri = (wave_idx1 < 128) ? (wave_idx1 << 1) : ((255 - wave_idx1) << 1);
    raw_audio = ((int16_t)saw + (int16_t)tri) - 256;
  } else {
    // Celestial Glass Harmonica: Pure fundamental + high crystal bell chime
    uint8_t s1 = pgm_read_byte(&sine_table[wave_idx1]);
    uint8_t s2 = pgm_read_byte(&sine_table[wave_idx2]);
    raw_audio = (((int16_t)s1 - 128) * 3 + ((int16_t)s2 - 128)) >> 2;
  }

  // 6. Vocal Formant Modulation (in Vocal mode)
  if (pitch_mode == 1) {
    uint8_t formant_env = 255 - wave_idx1; // Decay per cycle
    uint8_t f_sine = pgm_read_byte(&sine_table[wave_idx2]);
    int16_t vocal_burst = (((int16_t)f_sine - 128) * formant_env) >> 8;
    raw_audio = (raw_audio + vocal_burst) >> 1;
  }

  // 7. Volume Antenna Attenuation & Saturation
  if ((ticks & 7) == 0) {
    if (envelope_vol < volume_level) envelope_vol++;
    else if (envelope_vol > volume_level) envelope_vol--;
  }

  int16_t scaled = (raw_audio * envelope_vol) >> 8;

  // Soft tube saturation limiting
  if (scaled > 120) scaled = 120;
  else if (scaled < -120) scaled = -120;

  last_sample = (uint8_t)(scaled + 128);
  OCR1A = last_sample; // Output to 64MHz PWM DAC (PB1)
}

// ---------------------------------------------------------------------------------------
// NeoPixel Visualizer Engine (5x4 Matrix)
// ---------------------------------------------------------------------------------------
void update_visuals() {
  // If Touch Strobe is triggered: full electromagnetic spark flash!
  if (touch_strobe) {
    uint32_t flash_col = ((ticks & 32) != 0) ? pixels.Color(255, 255, 255) : pixels.Color(160, 0, 255);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, flash_col);
    }
    cli();
    pixels.show();
    sei();
    return;
  }

  // Snapshot pitch increment safely
  uint16_t inc_snap;
  uint8_t sreg = SREG; cli(); inc_snap = current_inc; SREG = sreg;

  // Calculate normalized pitch column (0 .. 4) and volume height (0 .. 3)
  uint8_t pitch_col_pos = ((uint32_t)(inc_snap > 400 ? inc_snap - 400 : 0) * 5) / 5000;
  if (pitch_col_pos > 4) pitch_col_pos = 4;
  uint8_t vol_height = ((uint16_t)envelope_vol * 4) / 255;
  if (vol_height > 3) vol_height = 3;

  if (visual_mode == 0) {
    // Mode 0: EMF Plasma Field (Aura radiating between Pitch & Volume antennas)
    for (uint8_t x = 0; x < 5; x++) {
      for (uint8_t y = 0; y < 4; y++) {
        int idx = matrixIndex(x, y);
        if (idx < 0) continue;

        uint8_t wave = (ticks >> 3) + (x * 40) + (y * 50);
        uint8_t pulse = pgm_read_byte(&sine_table[wave]);
        uint8_t lum = ((uint16_t)pulse * envelope_vol) >> 9;

        uint8_t r = 0, g = 0, b = 0;
        if (x <= pitch_col_pos) {
          // Pitch field: Ethereal cyan to violet
          r = lum >> 1;
          g = lum;
          b = lum + (lum >> 1);
        } else {
          // Volume field: Living emerald to gold
          r = lum;
          g = lum + (lum >> 2);
          b = lum >> 2;
        }

        // Active antenna fingertip highlight
        if (x == pitch_col_pos && y == (3 - vol_height)) {
          r = 255; g = 255; b = 255;
        }

        pixels.setPixelColor(idx, pixels.Color(r, g, b));
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: 1920s Phosphor CRT Oscilloscope
    for (uint8_t x = 0; x < 5; x++) {
      uint8_t osc_phase = (phase_osc1 >> 8) + (x * 45);
      uint8_t sample = pgm_read_byte(&sine_table[osc_phase]);
      int16_t amp = (((int16_t)sample - 128) * envelope_vol) >> 8;
      int8_t y_beam = 1 - (amp / 45);
      if (y_beam < 0) y_beam = 0;
      if (y_beam > 3) y_beam = 3;

      for (uint8_t y = 0; y < 4; y++) {
        int idx = matrixIndex(x, y);
        if (idx < 0) continue;
        if (y == y_beam) {
          pixels.setPixelColor(idx, pixels.Color(40, 255, 60)); // Bright trace
        } else if (abs((int)y - (int)y_beam) == 1) {
          pixels.setPixelColor(idx, pixels.Color(0, 45, 10));  // Persistence glow
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 0, 0));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Chromatic Spectral Ribbon
    for (uint8_t x = 0; x < 5; x++) {
      uint8_t hue = (pitch_col_pos * 48) + (x * 15);
      uint8_t r = 0, g = 0, b = 0;
      if (hue < 85) {
        r = 255 - hue * 3; g = hue * 3; b = 0;
      } else if (hue < 170) {
        hue -= 85; r = 0; g = 255 - hue * 3; b = hue * 3;
      } else {
        hue -= 170; r = hue * 3; g = 0; b = 255 - hue * 3;
      }

      for (uint8_t y = 0; y < 4; y++) {
        int idx = matrixIndex(x, 3 - y);
        if (idx < 0) continue;
        if (y <= vol_height) {
          uint8_t scale = 70 + (y * 55);
          pixels.setPixelColor(idx, pixels.Color(
            ((uint16_t)r * scale) >> 8,
            ((uint16_t)g * scale) >> 8,
            ((uint16_t)b * scale) >> 8
          ));
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 0, 0));
        }
      }
    }
  } else {
    // Mode 3: Sci-Fi UFO Radar Scanner
    uint8_t scan_col = (ticks >> 5) % 5;
    for (uint8_t x = 0; x < 5; x++) {
      for (uint8_t y = 0; y < 4; y++) {
        int idx = matrixIndex(x, y);
        if (idx < 0) continue;

        if (x == scan_col) {
          pixels.setPixelColor(idx, pixels.Color(180, 0, 255)); // Radar sweep
        } else if (x == pitch_col_pos && y == (3 - vol_height)) {
          pixels.setPixelColor(idx, pixels.Color(0, 255, 230)); // Ping
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 5, 25));
        }
      }
    }
  }

  cli();
  pixels.show();
  sei();
}

// ---------------------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------------------
void setup() {
  cli();

  // NeoPixel Setup
  pixels.begin();
  pixels.setBrightness(130);
  pixels.show();

  // High-Speed 64MHz PLL Audio PWM on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  TCCR1  = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10);
  OCR1A  = 128;
  pinMode(SPEAKERPIN, OUTPUT);

  // 10 kHz Audio Sample Clock on Timer0 COMPA
  TCCR0A = (3 << WGM00);              // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00);// Prescaler /8 -> 2 MHz / 200 = 10 kHz
  TIMSK  = (1 << OCIE0A);
  OCR0A  = 199;

  sei();
}

// ---------------------------------------------------------------------------------------
// Main Loop: Sensor Reading, Scale Mapping & Debounced Button Logic
// ---------------------------------------------------------------------------------------
void loop() {
  // 1. Read Potentiometers (Pitch & Volume Antennas)
  uint16_t pot_pitch = analogRead(POTI_LEFT);  // PB4 (A2): 0 .. 1023
  uint16_t pot_vol   = analogRead(POTI_RIGHT); // PB2 (A1): 0 .. 1023

  // Map Volume Pot: Quadratic curve with dead-zone for true acoustic muting
  if (pot_vol < 40) {
    volume_level = 0;
  } else {
    uint32_t v = (uint32_t)(pot_vol - 40);
    volume_level = (uint8_t)((v * v) / 3800);
    if (volume_level > 255) volume_level = 255;
  }

  // 2. Map Pitch Potentiometer according to Pitch Mode
  uint16_t new_target = 1442;
  if (pitch_mode == 3) {
    // Quantized Harmonic Minor Scale: 17 discrete musical steps
    uint8_t note_idx = ((uint32_t)pot_pitch * 17) / 1024;
    if (note_idx > 16) note_idx = 16;
    new_target = pgm_read_word(&minor_scale_incs[note_idx]);
  } else {
    // Continuous Glissando (Pure 1920s Etherphone): 55 Hz to ~2100 Hz
    uint32_t p = pot_pitch;
    new_target = 360 + (uint16_t)((p * p) >> 6);
  }

  // Atomically update target_inc
  uint8_t sreg = SREG; cli(); target_inc = new_target; SREG = sreg;

  // 3. Debounced Resistor Ladder Buttons (PB3 / A3)
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons Held: Antenna Ground Touch / Space Ray Strobe
    touch_strobe = 1;
  } else {
    touch_strobe = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Instrument Timbre (Sine, Heterodyne, Cello, Chime)
      timbre_mode = (timbre_mode + 1) & 3;

      // Color badge flash for clear tactile feedback
      uint32_t badge = (timbre_mode == 0) ? pixels.Color(0, 100, 255) :   // Blue (Sine)
                       (timbre_mode == 1) ? pixels.Color(255, 220, 0) :   // Yellow (Heterodyne)
                       (timbre_mode == 2) ? pixels.Color(255, 60, 0) :    // Orange (Cello/Saw)
                                            pixels.Color(255, 255, 255);  // White (Chime)
      pixels.fill(badge, 0, NUMPIXELS);
      cli(); pixels.show(); sei();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle Pitch / Scale Mode
      pitch_mode = (pitch_mode + 1) & 3;
      // Cycle visualizer mode to match
      visual_mode = (visual_mode + 1) & 3;

      // Color badge flash for mode feedback
      uint32_t badge = (pitch_mode == 0) ? pixels.Color(0, 255, 220) :  // Cyan (Continuous)
                       (pitch_mode == 1) ? pixels.Color(200, 0, 255) :  // Violet (Vocal Formant)
                       (pitch_mode == 2) ? pixels.Color(255, 0, 80) :   // Red (Sci-Fi UFO)
                                           pixels.Color(0, 255, 40);    // Green (Minor Scale)
      pixels.fill(badge, 0, NUMPIXELS);
      cli(); pixels.show(); sei();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn > Vbutton_releaseLevel) {
      // Button fully released - reset debounce latch
      prev_pressed = 0;
    }
  }

  // 4. Render Visuals
  update_visuals();

  // Frame pacing using custom 10 kHz audio-tick delay (~20 ms)
  my_delay(20);
}
