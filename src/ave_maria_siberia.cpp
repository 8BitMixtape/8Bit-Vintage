/* ---------------------------------------------------------------------------------------
    AVE MARIA — SIBERIAN LITURGICAL EDITION
    For ATtiny85 / 8Bit Mixtape (AudioBoot)
    
    A haunting liturgical arrangement of Schubert's Ave Maria with deep
    Russian Orthodox bass profundo pedal drones, sacred arpeggios, and
    vibrant Soviet-constructivist LED visuals.

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 PLL)
      PB4 (A2): Left Pot (Icy Detune / Cathedral Tone)
      PB2 (A1): Right Pot (Tempo & Sub Drone Depth)
      PB3 (A3): Resistor Ladder Buttons (Visual & Choral Modes)
--------------------------------------------------------------------------------------- */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"

#define NUMPIXELS 20
#define NEOPIXELPIN 0
#define SPEAKERPIN  1
#define BUTTON_PIN  A3
#define POTI_LEFT   A2 // PB4 (ADC2)
#define POTI_RIGHT  A1 // PB2 (ADC1)

#define Vbutton_releaseLevel  490
#define Vbutton_midLevel      410

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Audio Engine Variables ---
volatile uint16_t phase_bass = 0;
volatile uint16_t inc_bass   = 0;
volatile uint16_t phase_arp  = 0;
volatile uint16_t inc_arp   = 0;
volatile uint16_t phase_lead = 0;
volatile uint16_t inc_lead   = 0;

volatile uint8_t env_lead    = 0;
volatile uint8_t env_arp     = 0;
volatile uint8_t sub_depth   = 160;
volatile uint8_t voice_mode  = 0; // 0=Cathedral Choir, 1=Frozen Siberian Bells, 2=Orthodox Profundo

// Note Frequencies (D Minor / Dorian Sacred Scale, 10kHz sample rate scale: freq * 65536 / 10000)
// D2=73Hz, A2=110Hz, D3=147Hz, F3=175Hz, G3=196Hz, A3=220Hz, Bb3=233Hz, C4=262Hz, C#4=277Hz, D4=294Hz, E4=330Hz, F4=349Hz, G4=392Hz, A4=440Hz
#define N_REST 0
#define N_D2   480
#define N_A2   721
#define N_D3   962
#define N_F3   1145
#define N_G3   1285
#define N_A3   1442
#define N_Bb3  1528
#define N_C4   1718
#define N_Cs4  1816
#define N_D4   1925
#define N_E4   2163
#define N_F4   2289
#define N_G4   2570
#define N_A4   2884

// Ave Maria Melody (Liturgical phrase in D minor)
const uint16_t melody_notes[32] PROGMEM = {
  // Phrase 1: "A-ve..."
  N_A3, N_A3, N_D4, N_D4, N_D4, N_Cs4, N_Bb3, N_A3,
  N_A3, N_F4, N_E4, N_D4, N_Cs4, N_D4, N_E4, N_A3,
  // Phrase 2: "Ma-ri-a..."
  N_A3, N_A3, N_F4, N_F4, N_F4, N_E4, N_D4, N_Cs4,
  N_D4, N_F4, N_A4, N_G4, N_F4, N_E4, N_D4, N_D4
};

const uint8_t melody_dur[32] PROGMEM = {
  4, 4, 6, 2, 3, 1, 2, 6,
  2, 4, 3, 1, 2, 4, 4, 6,
  4, 4, 6, 2, 3, 1, 2, 6,
  2, 3, 3, 2, 2, 2, 6, 6
};

// 16-step Sacred Arpeggio Pattern (Choral accompaniment)
const uint16_t arp_notes[16] PROGMEM = {
  N_D3, N_F3, N_A3, N_D4, N_F4, N_D4, N_A3, N_F3,
  N_A2, N_D3, N_F3, N_A3, N_D4, N_A3, N_F3, N_D3
};

// Visual Modes: 0=Red Star, 1=Kremlin Spire, 2=Siberian Blizzard, 3=Constructivist Rays
uint8_t visual_mode = 0;

// 5x4 Soviet Star Bitmap (1 = Star Gold, 0 = Soviet Ruby Red)
const uint8_t soviet_star[20] PROGMEM = {
  0, 0, 1, 0, 0,
  0, 1, 1, 1, 0,
  1, 1, 1, 1, 1,
  0, 1, 0, 1, 0
};

// Kremlin Spire Bitmap
const uint8_t kremlin_spire[20] PROGMEM = {
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 1, 1, 1, 0,
  1, 1, 1, 1, 1
};

// 10kHz Audio Interrupt
ISR(TIMER0_COMPA_vect) {
  // Voice 1: Deep Siberian Bass Drone (Sub-bass triangle / organ pipe)
  phase_bass += inc_bass;
  uint8_t tb = (phase_bass >> 8);
  if (tb > 127) tb = 255 - tb;
  uint8_t s_bass = tb << 1;

  // Voice 2: Sacred Arpeggio
  phase_arp += inc_arp;
  uint8_t ta = (phase_arp >> 8);
  if (ta > 127) ta = 255 - ta;
  uint8_t s_arp = (uint16_t)(ta << 1) * env_arp >> 8;

  // Voice 3: Sacred Lead Melody
  phase_lead += inc_lead;
  uint8_t s_lead = 0;
  if (voice_mode == 0) {
    // Cathedral choir (warm triangle)
    uint8_t tl = (phase_lead >> 8);
    if (tl > 127) tl = 255 - tl;
    s_lead = (uint16_t)(tl << 1) * env_lead >> 8;
  } else if (voice_mode == 1) {
    // Siberian Chime / Bell (pulse wave with ring overtone)
    uint8_t pulse = (phase_lead & 0x8000) ? 220 : 35;
    s_lead = (uint16_t)pulse * env_lead >> 8;
  } else {
    // Orthodox Profundo (Sawtooth resonant organ)
    uint8_t saw = phase_lead >> 8;
    s_lead = (uint16_t)saw * env_lead >> 8;
  }

  // Sub drone level control
  uint8_t scaled_bass = ((uint16_t)s_bass * sub_depth) >> 8;

  // Mix 3 voices cleanly centered at 128
  int16_t mix = (int16_t)scaled_bass + s_arp + (s_lead << 1);
  mix = mix >> 2;
  if (mix > 255) mix = 255;
  OCR1A = (uint8_t)mix;
}

void setup() {
  pixels.begin();
  pixels.setBrightness(120);

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // PLL for 64MHz Timer1 high-speed PWM audio on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  TCCR1  = (1 << PWM1A) | (1 << COM1A0) | (1 << CS10);
  pinMode(SPEAKERPIN, OUTPUT);

  // Timer0 10kHz Interrupt for sample generation
  TCCR0A = (3 << WGM00);            // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00); // Prescaler 8
  TIMSK  = (1 << OCIE0A);
  OCR0A  = 199; // (16MHz / 8) / (199 + 1) = 10kHz

  inc_bass = N_D2; // Deep D2 pedal bass
  sei();
}

void loop() {
  static uint8_t mel_idx = 0;
  static uint8_t arp_idx = 0;
  static uint16_t step_timer = 0;
  static uint16_t mel_hold = 0;
  static uint8_t frame = 0;

  // Read Potentiometers
  uint16_t p_left  = analogRead(POTI_LEFT);  // Detune / Frost
  uint16_t p_right = analogRead(POTI_RIGHT); // Tempo & Sub Drone

  // Map sub drone depth
  sub_depth = map(p_right, 0, 1023, 80, 255);
  uint16_t tempo_ms = map(p_left, 0, 1023, 70, 150);

  // Check Buttons
  uint16_t btn = analogRead(BUTTON_PIN);
  if (btn < Vbutton_releaseLevel) {
    if (btn < Vbutton_midLevel) {
      // Right button: Cycle voice mode
      voice_mode = (voice_mode + 1) % 3;
    } else {
      // Left button: Cycle visual mode
      visual_mode = (visual_mode + 1) % 4;
    }
    delay(200); // Debounce
  }

  // --- Step Sequencer: Arpeggio & Melody ---
  // Advance arpeggio every step
  inc_arp = pgm_read_word(&arp_notes[arp_idx]);
  env_arp = 220;
  arp_idx = (arp_idx + 1) & 15;

  // Advance melody note when hold expires
  if (mel_hold == 0) {
    uint16_t note = pgm_read_word(&melody_notes[mel_idx]);
    uint8_t dur   = pgm_read_byte(&melody_dur[mel_idx]);
    inc_lead = note;
    env_lead = 250;
    mel_hold = dur;
    mel_idx = (mel_idx + 1) % 32;
  } else {
    mel_hold--;
    // Gentle melodic decay
    if (env_lead > 10) env_lead -= 6;
  }

  // --- Visual Rendering (5x4 NeoPixel Matrix) ---
  cli(); // Briefly protect NeoPixel timing
  uint32_t ruby_red = pixels.Color(160, 0, 0);
  uint32_t gold     = pixels.Color(255, 195, 0);
  uint32_t dark_red = pixels.Color(25, 0, 0);
  uint32_t white    = pixels.Color(230, 240, 255);

  if (visual_mode == 0) {
    // Soviet Red Star: Gold star pulsing against Soviet Ruby Red
    uint8_t pulse = 180 + (env_lead >> 2);
    uint32_t active_gold = pixels.Color(pulse, (pulse * 3) >> 2, 0);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_star = pgm_read_byte(&soviet_star[y * MATRIX_W + x]);
        matrixSet(x, y, is_star ? active_gold : dark_red);
      }
    }
  } else if (visual_mode == 1) {
    // Kremlin Spire: Beacon light ascending
    uint8_t beacon_y = 3 - (frame % 4);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_spire = pgm_read_byte(&kremlin_spire[y * MATRIX_W + x]);
        if (is_spire) {
          matrixSet(x, y, (y == beacon_y) ? white : ruby_red);
        } else {
          matrixSet(x, y, pixels.Color(10, 0, 0));
        }
      }
    }
  } else if (visual_mode == 2) {
    // Siberian Blizzard: Red banner with falling ice crystals
    matrixFill(dark_red);
    uint8_t snow_x = (frame * 3 + arp_idx) % MATRIX_W;
    uint8_t snow_y = (frame) % MATRIX_H;
    matrixSet(snow_x, snow_y, white);
    matrixSet((snow_x + 2) % MATRIX_W, (snow_y + 2) % MATRIX_H, pixels.Color(100, 200, 255));
    // Center gold accent
    matrixSet(2, 1, gold);
  } else {
    // Constructivist Rays: Sweeping red & gold angular bars
    matrixFill(pixels.Color(20, 0, 0));
    uint8_t bar_col = arp_idx % MATRIX_W;
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      matrixSet(bar_col, y, (y % 2 == 0) ? gold : ruby_red);
    }
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  delay(tempo_ms);
}
