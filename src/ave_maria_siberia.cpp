/* ---------------------------------------------------------------------------------------
    AVE MARIA — SIBERIAN LITURGICAL EDITION (HARDWARE FIX)
    For ATtiny85 / 8Bit Mixtape (AudioBoot)
    
    A liturgical arrangement of Schubert's Ave Maria with deep
    Russian Orthodox bass profundo pedal drones, sacred arpeggios, and
    Soviet-constructivist LED visuals.

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 PLL)
      PB4 (A2): Left Pot (Tempo & Tone)
      PB2 (A1): Right Pot (Sub Drone Profundis Depth)
      PB3 (A3): Resistor Ladder Buttons (Visual & Voice Modes)
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

// --- Audio Engine State ---
volatile uint16_t phase_bass = 0;
volatile uint16_t inc_bass   = 481; // D2 (73.4 Hz)
volatile uint16_t phase_arp  = 0;
volatile uint16_t inc_arp   = 962; // D3
volatile uint16_t phase_lead = 0;
volatile uint16_t inc_lead   = 1442; // A3

volatile uint8_t env_lead    = 250;
volatile uint8_t env_arp     = 180;
volatile uint8_t sub_depth   = 160;
volatile uint8_t voice_mode  = 0; // 0=Cathedral Choir, 1=Siberian Chimes, 2=Orthodox Profundo

volatile uint16_t ticks      = 0;
volatile uint16_t step_ticks = 1100; // 110ms per arpeggio step at 10kHz

// Note Frequencies (D Minor / Dorian Sacred Scale, 10kHz scale)
#define N_REST 0
#define N_D2   481
#define N_A2   721
#define N_D3   962
#define N_F3   1144
#define N_G3   1285
#define N_A3   1442
#define N_Bb3  1528
#define N_C4   1715
#define N_Cs4  1817
#define N_D4   1925
#define N_E4   2160
#define N_F4   2289
#define N_G4   2569
#define N_A4   2884

// Schubert's Ave Maria Melody (Liturgical D minor)
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

// 16-step Sacred Arpeggio Pattern
const uint16_t arp_notes[16] PROGMEM = {
  N_D3, N_F3, N_A3, N_D4, N_F4, N_D4, N_A3, N_F3,
  N_A2, N_D3, N_F3, N_A3, N_D4, N_A3, N_F3, N_D3
};

// Visual Modes: 0=Red Star, 1=Kremlin Spire, 2=Siberian Blizzard, 3=Constructivist Rays
uint8_t visual_mode = 0;
volatile uint8_t current_arp_step = 0;

// 5x4 Soviet Star Bitmap
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

// Custom hardware delay based on 10kHz ISR ticks
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

// 10kHz Audio Sample & Sequencer ISR
ISR(TIMER0_COMPA_vect) {
  ticks++;

  // --- Hardware Sequencer: Advances notes accurately in background ---
  static uint16_t tick_acc = 0;
  static uint8_t arp_idx = 0;
  static uint8_t mel_idx = 0;
  static uint8_t mel_hold = 0;

  tick_acc++;
  if (tick_acc >= step_ticks) {
    tick_acc = 0;

    // Advance Arpeggio
    arp_idx = (arp_idx + 1) & 15;
    current_arp_step = arp_idx;
    inc_arp = pgm_read_word(&arp_notes[arp_idx]);
    env_arp = 210;

    // Advance Melody
    if (mel_hold == 0) {
      inc_lead = pgm_read_word(&melody_notes[mel_idx]);
      mel_hold = pgm_read_byte(&melody_dur[mel_idx]);
      env_lead = 250;
      mel_idx = (mel_idx + 1) % 32;
    } else {
      mel_hold--;
    }
  }

  // Melodic decay & vibrato
  if ((ticks & 31) == 0 && env_lead > 40) {
    env_lead -= 2;
  }

  // --- Voice 1: Deep Siberian Bass Drone (D2) ---
  phase_bass += inc_bass;
  uint8_t tb = (phase_bass >> 8);
  if (tb > 127) tb = 255 - tb;
  uint8_t s_bass = ((uint16_t)(tb << 1) * sub_depth) >> 8;

  // --- Voice 2: Sacred Arpeggio ---
  phase_arp += inc_arp;
  uint8_t ta = (phase_arp >> 8);
  if (ta > 127) ta = 255 - ta;
  uint8_t s_arp = ((uint16_t)(ta << 1) * env_arp) >> 8;

  // --- Voice 3: Sacred Lead Melody ---
  // Subtle liturgical choral vibrato
  int8_t vib = (ticks & 0x40) ? 6 : -6;
  phase_lead += (inc_lead + vib);

  uint8_t s_lead = 0;
  if (voice_mode == 0) {
    // Cathedral Choir (smooth triangle)
    uint8_t tl = (phase_lead >> 8);
    if (tl > 127) tl = 255 - tl;
    s_lead = ((uint16_t)(tl << 1) * env_lead) >> 8;
  } else if (voice_mode == 1) {
    // Siberian Bell (pulse wave)
    uint8_t pulse = (phase_lead & 0x8000) ? 220 : 35;
    s_lead = ((uint16_t)pulse * env_lead) >> 8;
  } else {
    // Orthodox Profundo (resonant saw)
    uint8_t saw = phase_lead >> 8;
    s_lead = ((uint16_t)saw * env_lead) >> 8;
  }

  // Clean 3-voice 8-bit PWM mix
  uint16_t sum = (uint16_t)s_bass + (uint16_t)s_arp + ((uint16_t)s_lead << 1);
  OCR1A = sum >> 2;
}

void setup() {
  pixels.begin();
  pixels.setBrightness(120);

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // PLL for 64MHz Timer1 high-speed PWM audio on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  // CRITICAL FIX: 2 << COM1A0 is COM1A1 (Clear on match = real PWM!)
  TCCR1  = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10);
  pinMode(SPEAKERPIN, OUTPUT);

  // Timer0 10kHz Interrupt for audio & sequencer
  TCCR0A = (3 << WGM00);            // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00); // Prescaler 8
  TIMSK  = (1 << OCIE0A);
  OCR0A  = 199; // (16MHz / 8) / (199 + 1) = 10kHz

  sei();
}

void loop() {
  static uint8_t frame = 0;

  // Read Potentiometers
  uint16_t p_left  = analogRead(POTI_LEFT);  // Tempo & Tone
  uint16_t p_right = analogRead(POTI_RIGHT); // Sub-bass Depth

  // Update parameters
  sub_depth = map(p_right, 0, 1023, 70, 240);
  // Map step_ticks from 600 (60ms = fast) to 1800 (180ms = majestic slow)
  step_ticks = map(p_left, 0, 1023, 650, 1600);

  // Check Buttons
  uint16_t btn = analogRead(BUTTON_PIN);
  if (btn < Vbutton_releaseLevel) {
    if (btn < Vbutton_midLevel) {
      voice_mode = (voice_mode + 1) % 3;
    } else {
      visual_mode = (visual_mode + 1) % 4;
    }
    my_delay(250); // Debounce using hardware tick delay!
  }

  // --- Visuals (5x4 NeoPixel Matrix) ---
  cli(); // Briefly protect timing
  uint32_t ruby_red = pixels.Color(160, 0, 0);
  uint32_t gold     = pixels.Color(255, 195, 0);
  uint32_t dark_red = pixels.Color(20, 0, 0);
  uint32_t white    = pixels.Color(230, 240, 255);

  if (visual_mode == 0) {
    // Mode 0: Soviet Red Star pulsing to the melody
    uint8_t pulse = 180 + (env_lead >> 2);
    uint32_t active_gold = pixels.Color(pulse, (pulse * 3) >> 2, 0);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_star = pgm_read_byte(&soviet_star[y * MATRIX_W + x]);
        matrixSet(x, y, is_star ? active_gold : dark_red);
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Kremlin Spire with rising light beacon
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
    // Mode 2: Siberian Blizzard across red banner
    matrixFill(dark_red);
    uint8_t snow_x = (frame * 3 + current_arp_step) % MATRIX_W;
    uint8_t snow_y = (frame) % MATRIX_H;
    matrixSet(snow_x, snow_y, white);
    matrixSet((snow_x + 2) % MATRIX_W, (snow_y + 2) % MATRIX_H, pixels.Color(100, 200, 255));
    matrixSet(2, 1, gold);
  } else {
    // Mode 3: Constructivist Angular Rays in sync with arpeggio
    matrixFill(pixels.Color(20, 0, 0));
    uint8_t bar_col = current_arp_step % MATRIX_W;
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      matrixSet(bar_col, y, (y % 2 == 0) ? gold : ruby_red);
    }
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  my_delay(50); // Clean 50ms frame update, never freezes!
}
