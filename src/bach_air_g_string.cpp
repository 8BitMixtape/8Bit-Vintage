/* ---------------------------------------------------------------------------------------
    J.S. BACH: AIR ON THE G STRING & CLASSICAL BAROQUE SUITE (OPTIMIZED EDITION)
    For ATtiny85 / 8Bit Mixtape (AudioBoot)
    
    A 3-voice polyphonic classical synthesizer engine featuring:
      - Movement 0: J.S. Bach — Air on the G String (BWV 1068)
      - Movement 1: Johann Pachelbel — Canon in D Major
      - Movement 2: Antonio Vivaldi — Winter (L'Inverno - Largo)
      - Movement 3: Ludwig van Beethoven — Moonlight Sonata (Adagio sostenuto)

    Visuals:
      - 6 Generative Classical Visualizers on 5x4 NeoPixel Matrix
      - Automatic random visual mode change every time a new song is selected!

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL @ 250kHz carrier)
      PB4 (A2): Left Pot (Tempo & Agogic Timing: 70ms - 220ms per step)
      PB2 (A1): Right Pot (Acoustic Reverb Tail & Sub-Bass Depth)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Classical Piece / Movement (Changes visual randomly!)
                - Right Button: Cycle Instrument Timbre (Chamber, Organ, Glass, Moog)
                - Both Buttons: Cycle / Randomize Visualizer Mode manually
 --------------------------------------------------------------------------------------- */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"

// Hardware Pin Definitions
#define NUMPIXELS   20
#define NEOPIXELPIN 0
#define SPEAKERPIN  1
#define BUTTON_PIN  A3
#define POTI_LEFT   A2 // PB4 (ADC2)
#define POTI_RIGHT  A1 // PB2 (ADC1)

// Button Calibration
#define Vbutton_releaseLevel  490
#define Vbutton_left          460
#define Vbutton_right         395
#define Vbutton_both          320

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// Compact Note Indices (1-byte per note to keep Flash comfortably below 6KB!)
enum NoteName : uint8_t {
  N_REST = 0,
  N_C2, N_Cs2, N_D2, N_Ds2, N_E2, N_F2, N_Fs2, N_G2, N_Gs2, N_A2, N_Bb2, N_B2,
  N_C3, N_Cs3, N_D3, N_Ds3, N_E3, N_F3, N_Fs3, N_G3, N_Gs3, N_A3, N_Bb3, N_B3,
  N_C4, N_Cs4, N_D4, N_Ds4, N_E4, N_F4, N_Fs4, N_G4, N_Gs4, N_A4, N_Bb4, N_B4,
  N_C5, N_Cs5, N_D5, N_Ds5, N_E5, N_F5, N_Fs5, N_G5, N_A5
};

// 10kHz Phase Increments Table (inc = f_Hz * 65536 / 10000)
const uint16_t note_freqs[] PROGMEM = {
  0,    // REST
  429,  // C2
  454,  // Cs2
  481,  // D2
  510,  // Ds2 / Eb2
  540,  // E2
  572,  // F2
  606,  // Fs2
  642,  // G2
  680,  // Gs2
  721,  // A2
  764,  // Bb2
  809,  // B2

  857,  // C3
  908,  // Cs3
  962,  // D3
  1020, // Ds3 / Eb3
  1080, // E3
  1144, // F3
  1212, // Fs3
  1285, // G3
  1361, // Gs3
  1442, // A3
  1528, // Bb3
  1618, // B3

  1715, // C4
  1817, // Cs4
  1925, // D4
  2039, // Ds4 / Eb4
  2160, // E4
  2289, // F4
  2425, // Fs4
  2569, // G4
  2722, // Gs4
  2884, // A4
  3055, // Bb4
  3237, // B4

  3429, // C5
  3633, // Cs5
  3849, // D5
  4078, // Ds5 / Eb5
  4321, // E5
  4577, // F5
  4850, // Fs5
  5138, // G5
  5767  // A5
};

static inline uint16_t get_note_inc(uint8_t idx) {
  if (idx >= sizeof(note_freqs)/sizeof(note_freqs[0])) return 0;
  return pgm_read_word(&note_freqs[idx]);
}

// --- 64-step Musical Scores in Flash (Compact 1-byte per note!) ---

// --- Movement 0: J.S. Bach — Air on the G String (BWV 1068) ---
const uint8_t bach_melody[64] PROGMEM = {
  N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_G4,  N_A4,
  N_B4,  N_B4,  N_B4,  N_A4,  N_G4,  N_Fs4, N_E4,  N_Cs4,
  N_D4,  N_D4,  N_D4,  N_E4,  N_Fs4, N_G4,  N_A4,  N_B4,
  N_Cs5, N_Cs5, N_Cs5, N_B4,  N_A4,  N_Gs4, N_B4,  N_Gs4,
  N_A4,  N_A4,  N_A4,  N_A4,  N_A4,  N_A4,  N_B4,  N_Cs5,
  N_D5,  N_D5,  N_D5,  N_D5,  N_Cs5, N_Cs5, N_B4,  N_B4,
  N_A4,  N_A4,  N_G4,  N_G4,  N_Fs4, N_Fs4, N_E4,  N_E4,
  N_D4,  N_D4,  N_D4,  N_D4,  N_D4,  N_D4,  N_D4,  N_D4
};

const uint8_t bach_bass[64] PROGMEM = {
  N_D3,  N_D2,  N_Cs3, N_Cs2, N_B2,  N_B2,  N_A2,  N_A2,
  N_G2,  N_G2,  N_Fs2, N_Fs2, N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D3,  N_Cs3, N_B2,  N_A2,  N_G2,  N_Fs2, N_E2,
  N_Fs2, N_G2,  N_Gs2, N_A2,  N_A2,  N_G2,  N_Fs2, N_E2,
  N_A2,  N_A2,  N_G2,  N_Fs2, N_E2,  N_D2,  N_Cs2, N_A2,
  N_D3,  N_D2,  N_Cs3, N_Cs2, N_B2,  N_B2,  N_A2,  N_A2,
  N_G2,  N_Fs2, N_E2,  N_D2,  N_A2,  N_A2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_A2,  N_A2,  N_D2,  N_D2,  N_D2,  N_D2
};

const uint8_t bach_arp[64] PROGMEM = {
  N_A3,  N_D4,  N_Fs4, N_D4,  N_A3,  N_Cs4, N_E4,  N_Cs4,
  N_D3,  N_G3,  N_B3,  N_G3,  N_D3,  N_Fs3, N_A3,  N_E3,
  N_Fs3, N_A3,  N_D4,  N_A3,  N_E3,  N_A3,  N_Cs4, N_Fs3,
  N_Cs3, N_E3,  N_A3,  N_E3,  N_D3,  N_Gs3, N_B3,  N_Gs3,
  N_E3,  N_A3,  N_Cs4, N_A3,  N_E3,  N_A3,  N_Cs4, N_A3,
  N_Fs3, N_A3,  N_D4,  N_A3,  N_E3,  N_G3,  N_Cs4, N_G3,
  N_D3,  N_G3,  N_B3,  N_G3,  N_Cs3, N_E3,  N_A3,  N_E3,
  N_D3,  N_Fs3, N_A3,  N_Fs3, N_D3,  N_Fs3, N_A3,  N_D4
};

// --- Movement 1: Johann Pachelbel — Canon in D Major ---
const uint8_t pachelbel_melody[64] PROGMEM = {
  N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_E4,  N_E4,  N_E4,  N_E4,
  N_D4,  N_D4,  N_D4,  N_D4,  N_Cs4, N_Cs4, N_Cs4, N_Cs4,
  N_B3,  N_B3,  N_B3,  N_B3,  N_A3,  N_A3,  N_A3,  N_A3,
  N_B3,  N_B3,  N_B3,  N_B3,  N_Cs4, N_Cs4, N_Cs4, N_Cs4,
  N_D4,  N_D4,  N_Fs4, N_Fs4, N_A4,  N_A4,  N_G4,  N_G4,
  N_Fs4, N_Fs4, N_D4,  N_D4,  N_Fs4, N_Fs4, N_E4,  N_E4,
  N_D4,  N_D4,  N_B3,  N_B3,  N_D4,  N_D4,  N_A3,  N_A3,
  N_G3,  N_G3,  N_B3,  N_B3,  N_A3,  N_A3,  N_G3,  N_G3
};

const uint8_t pachelbel_bass[64] PROGMEM = {
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2,
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2,
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2,
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2
};

const uint8_t pachelbel_arp[64] PROGMEM = {
  N_D4,  N_Fs4, N_A4,  N_Fs4, N_Cs4, N_E4,  N_A4,  N_E4,
  N_B3,  N_D4,  N_Fs4, N_D4,  N_A3,  N_Cs4, N_Fs4, N_Cs4,
  N_G3,  N_B3,  N_D4,  N_B3,  N_Fs3, N_A3,  N_D4,  N_A3,
  N_G3,  N_B3,  N_D4,  N_B3,  N_A3,  N_Cs4, N_E4,  N_Cs4,
  N_D4,  N_Fs4, N_A4,  N_Fs4, N_Cs4, N_E4,  N_A4,  N_E4,
  N_B3,  N_D4,  N_Fs4, N_D4,  N_A3,  N_Cs4, N_Fs4, N_Cs4,
  N_G3,  N_B3,  N_D4,  N_B3,  N_Fs3, N_A3,  N_D4,  N_A3,
  N_G3,  N_B3,  N_D4,  N_B3,  N_A3,  N_Cs4, N_E4,  N_Cs4
};

// --- Movement 2: Antonio Vivaldi — Winter (Largo) ---
const uint8_t vivaldi_melody[64] PROGMEM = {
  N_Fs4, N_Fs4, N_Fs4, N_G4,  N_A4,  N_A4,  N_A4,  N_A4,
  N_D5,  N_D5,  N_D5,  N_D5,  N_Cs5, N_Cs5, N_B4,  N_B4,
  N_A4,  N_A4,  N_A4,  N_A4,  N_B4,  N_A4,  N_G4,  N_Fs4,
  N_E4,  N_E4,  N_Fs4, N_G4,  N_A4,  N_A4,  N_A4,  N_A4,
  N_Fs4, N_Fs4, N_Fs4, N_D4,  N_E4,  N_Fs4, N_G4,  N_A4,
  N_B4,  N_B4,  N_B4,  N_Cs5, N_D5,  N_D5,  N_D5,  N_D5,
  N_Cs5, N_Cs5, N_B4,  N_B4,  N_A4,  N_A4,  N_G4,  N_G4,
  N_Fs4, N_Fs4, N_E4,  N_E4,  N_D4,  N_D4,  N_D4,  N_D4
};

const uint8_t vivaldi_bass[64] PROGMEM = {
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_D2,  N_D2
};

const uint8_t vivaldi_arp[64] PROGMEM = {
  N_Fs3, N_A3,  N_Fs3, N_A3,  N_A3,  N_D4,  N_A3,  N_D4,
  N_B3,  N_D4,  N_B3,  N_D4,  N_Cs4, N_E4,  N_Cs4, N_E4,
  N_Fs3, N_A3,  N_Fs3, N_A3,  N_G3,  N_B3,  N_G3,  N_B3,
  N_G3,  N_B3,  N_G3,  N_B3,  N_A3,  N_Cs4, N_A3,  N_Cs4,
  N_Fs3, N_A3,  N_Fs3, N_A3,  N_A3,  N_D4,  N_A3,  N_D4,
  N_B3,  N_D4,  N_B3,  N_D4,  N_Cs4, N_E4,  N_Cs4, N_E4,
  N_Fs3, N_A3,  N_Fs3, N_A3,  N_G3,  N_B3,  N_G3,  N_B3,
  N_G3,  N_B3,  N_G3,  N_B3,  N_Fs3, N_A3,  N_D4,  N_A3
};

// --- Movement 3: Ludwig van Beethoven — Moonlight Sonata (Adagio) ---
const uint8_t moonlight_melody[64] PROGMEM = {
  N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST,
  N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST,
  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,
  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,
  N_A3,   N_A3,   N_A3,   N_A3,   N_Gs3,  N_Gs3,  N_Fs3,  N_Fs3,
  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,
  N_E3,   N_E3,   N_E3,   N_E3,   N_Ds3,  N_Ds3,  N_Ds3,  N_Ds3,
  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3
};

const uint8_t moonlight_bass[64] PROGMEM = {
  N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_B2,  N_B2,  N_B2,  N_B2,
  N_A2,  N_A2,  N_A2,  N_A2,  N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_B2,  N_B2,  N_B2,  N_B2,
  N_A2,  N_A2,  N_A2,  N_A2,  N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Fs2, N_Fs2, N_Fs2, N_Fs2, N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Gs2, N_Gs2, N_Gs2, N_Gs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2,
  N_Fs2, N_Fs2, N_Fs2, N_Fs2, N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2
};

const uint8_t moonlight_arp[64] PROGMEM = {
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_A3,  N_Cs4, N_E4,  N_Cs4, N_A3,  N_D4,  N_Fs4, N_D4,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_B3,  N_Ds4, N_B3,
  N_A3,  N_Cs4, N_Fs4, N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_Fs3, N_C4,  N_Ds4, N_C4,  N_Gs3, N_B3,  N_Ds4, N_B3,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4
};

// Audio Engine State Variables
volatile uint16_t phase_bass = 0;
volatile uint16_t inc_bass   = 481;
volatile uint16_t phase_arp  = 0;
volatile uint16_t inc_arp    = 1442;
volatile uint16_t phase_lead = 0;
volatile uint16_t inc_lead   = 2425;

volatile uint8_t env_lead    = 240;
volatile uint8_t env_arp     = 180;
volatile uint8_t env_bass    = 200;
volatile uint8_t note_age    = 0;

volatile uint8_t current_piece  = 0; // 0=Bach Air, 1=Pachelbel Canon, 2=Vivaldi Winter, 3=Beethoven Moonlight
volatile uint8_t timbre_mode    = 0; // 0=Chamber Strings, 1=Cathedral Organ, 2=Glass Harmonica, 3=Switched-On Moog

#define NUM_VISUAL_MODES 6
uint8_t visual_mode             = 0; 

volatile uint16_t ticks         = 0;
volatile uint16_t step_ticks    = 1200; // Tempo tick count per step (~120ms default)
volatile uint8_t current_step   = 0;

volatile uint8_t sub_bass_depth = 160;
volatile uint8_t reverb_tail    = 120;

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

  static uint16_t tick_acc = 0;
  static uint8_t last_lead_note = 255;

  tick_acc++;
  if (tick_acc >= step_ticks) {
    tick_acc = 0;
    current_step = (current_step + 1) & 63; // 64-step loop

    uint8_t n_lead_idx = 0;
    uint8_t n_bass_idx = 0;
    uint8_t n_arp_idx  = 0;

    switch (current_piece) {
      case 0:
        n_lead_idx = pgm_read_byte(&bach_melody[current_step]);
        n_bass_idx = pgm_read_byte(&bach_bass[current_step]);
        n_arp_idx  = pgm_read_byte(&bach_arp[current_step]);
        break;
      case 1:
        n_lead_idx = pgm_read_byte(&pachelbel_melody[current_step]);
        n_bass_idx = pgm_read_byte(&pachelbel_bass[current_step]);
        n_arp_idx  = pgm_read_byte(&pachelbel_arp[current_step]);
        break;
      case 2:
        n_lead_idx = pgm_read_byte(&vivaldi_melody[current_step]);
        n_bass_idx = pgm_read_byte(&vivaldi_bass[current_step]);
        n_arp_idx  = pgm_read_byte(&vivaldi_arp[current_step]);
        break;
      default:
        n_lead_idx = pgm_read_byte(&moonlight_melody[current_step]);
        n_bass_idx = pgm_read_byte(&moonlight_bass[current_step]);
        n_arp_idx  = pgm_read_byte(&moonlight_arp[current_step]);
        break;
    }

    if (n_lead_idx != last_lead_note) {
      inc_lead = get_note_inc(n_lead_idx);
      last_lead_note = n_lead_idx;
      env_lead = (n_lead_idx == N_REST) ? 0 : 250;
      note_age = 0;
    }

    inc_arp  = get_note_inc(n_arp_idx);
    env_arp  = 220;

    inc_bass = get_note_inc(n_bass_idx);
    env_bass = 210;
  }

  // Envelope decay & articulation
  if ((ticks & 15) == 0) {
    if (note_age < 255) note_age++;
    if (env_lead > 130) env_lead -= 1;
    if (env_arp > 40) env_arp -= (timbre_mode == 0) ? 4 : 2;
    if (env_bass > 100) env_bass -= 1;
  }

  // Voice 1: Basso Continuo (Cello)
  phase_bass += inc_bass;
  uint8_t tb = (phase_bass >> 8);
  if (tb > 127) tb = 255 - tb;
  uint8_t sub = (phase_bass >> 9);
  if (sub > 127) sub = 255 - sub;
  uint8_t raw_bass = tb + (sub >> 1);
  uint8_t s_bass = ((uint16_t)raw_bass * env_bass) >> 8;
  s_bass = ((uint16_t)s_bass * sub_bass_depth) >> 8;

  // Voice 2: Arpeggiated Continuo / Harpsichord
  phase_arp += inc_arp;
  uint8_t s_arp = 0;
  if (timbre_mode == 0) {
    uint8_t saw = phase_arp >> 8;
    uint8_t pluck = (saw >> 1) + ((phase_arp & 0x4000) ? 64 : 0);
    s_arp = ((uint16_t)pluck * env_arp) >> 8;
  } else if (timbre_mode == 1) {
    uint8_t ta = (phase_arp >> 8);
    if (ta > 127) ta = 255 - ta;
    s_arp = ((uint16_t)(ta << 1) * env_arp) >> 8;
  } else if (timbre_mode == 2) {
    uint8_t ta = (phase_arp >> 8);
    if (ta > 127) ta = 255 - ta;
    uint8_t shimmer = (phase_arp >> 7);
    s_arp = ((uint16_t)(ta ^ (shimmer & 0x1F)) * env_arp) >> 8;
  } else {
    uint8_t saw = phase_arp >> 8;
    s_arp = ((uint16_t)saw * env_arp) >> 8;
  }

  // Voice 3: Solo Cantabile (Violin with delayed vibrato)
  int8_t vib = 0;
  if (note_age > 16) {
    uint8_t vib_step = (ticks >> 4) & 31;
    int8_t vib_shape = (vib_step < 16) ? (vib_step - 8) : (24 - vib_step);
    uint8_t v_depth = (note_age > 60) ? 8 : 4;
    vib = (vib_shape * v_depth) >> 3;
  }
  phase_lead += (inc_lead + vib);

  uint8_t s_lead = 0;
  if (inc_lead != 0) {
    if (timbre_mode == 0) {
      uint8_t tl = (phase_lead >> 8);
      if (tl > 127) tl = 255 - tl;
      s_lead = ((uint16_t)(tl << 1) * env_lead) >> 8;
    } else if (timbre_mode == 1) {
      uint8_t tl1 = (phase_lead >> 8);
      if (tl1 > 127) tl1 = 255 - tl1;
      uint8_t tl2 = (phase_lead >> 7);
      if (tl2 > 127) tl2 = 255 - tl2;
      uint8_t organ = (tl1 * 3 + tl2) >> 2;
      s_lead = ((uint16_t)(organ << 1) * env_lead) >> 8;
    } else if (timbre_mode == 2) {
      uint8_t tl = (phase_lead >> 8);
      if (tl > 127) tl = 255 - tl;
      s_lead = ((uint16_t)(tl << 1) * env_lead) >> 8;
    } else {
      static uint8_t pwm_width = 128;
      uint8_t pulse = ((phase_lead >> 8) > pwm_width) ? 220 : 35;
      s_lead = ((uint16_t)pulse * env_lead) >> 8;
    }
  }

  // Master mix
  uint16_t mix = ((uint16_t)s_bass * 160) >> 8;
  mix += ((uint16_t)s_arp * (140 + (reverb_tail >> 2))) >> 8;
  mix += ((uint16_t)s_lead * 240) >> 8;

  if (mix > 255) mix = 255;
  OCR1A = mix;
}

void setup() {
  pixels.begin();
  pixels.setBrightness(140); // Bright & vibrant on hardware NeoPixels!
  pixels.clear();
  pixels.show();

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // PLL for 64MHz Timer1 high-speed PWM audio on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  TCCR1  = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10);
  pinMode(SPEAKERPIN, OUTPUT);

  // Timer0 10kHz Interrupt for audio & sequencer
  TCCR0A = (3 << WGM00);               // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00); // Prescaler 8
  TIMSK  = (1 << OCIE0A);
  OCR0A  = 199; // (16MHz / 8) / (199 + 1) = 10,000 Hz

  sei();
}

void loop() {
  static uint8_t frame = 0;
  static uint8_t last_btn_state = 0;

  // Read Potentiometers
  uint16_t p_tempo  = analogRead(POTI_LEFT);  // PB4: Tempo / Rubato
  uint16_t p_reverb = analogRead(POTI_RIGHT); // PB2: Reverb Tail & Sub Depth

  step_ticks = map(p_tempo, 0, 1023, 700, 2200);
  reverb_tail = map(p_reverb, 0, 1023, 40, 240);
  sub_bass_depth = map(p_reverb, 0, 1023, 110, 250);

  // Read Navigation Buttons with robust Edge Detection
  uint16_t btn = analogRead(BUTTON_PIN);
  uint8_t curr_btn_state = 0;

  if (btn < Vbutton_both) {
    curr_btn_state = 3; // Both buttons pressed
  } else if (btn < Vbutton_right) {
    curr_btn_state = 2; // Right button pressed
  } else if (btn < Vbutton_left) {
    curr_btn_state = 1; // Left button pressed
  } else {
    curr_btn_state = 0; // Released
  }

  // Trigger action only on initial button press!
  if (curr_btn_state != last_btn_state && curr_btn_state != 0) {
    if (curr_btn_state == 1) {
      // Left Button: Cycle Classical Piece / Movement
      current_piece = (current_piece + 1) & 3;
      current_step = 0; // Restart at start of piece

      // Randomly change to a different visual mode every time song changes!
      uint8_t rand_offset = 1 + (((uint8_t)ticks ^ frame) % (NUM_VISUAL_MODES - 1));
      visual_mode = (visual_mode + rand_offset) % NUM_VISUAL_MODES;
    } else if (curr_btn_state == 2) {
      // Right Button: Cycle Instrument Timbre Mode
      timbre_mode = (timbre_mode + 1) & 3;
    } else if (curr_btn_state == 3) {
      // Both Buttons: Cycle visual mode manually
      visual_mode = (visual_mode + 1) % NUM_VISUAL_MODES;
    }
    my_delay(150);
  }
  last_btn_state = curr_btn_state;

  // --- Classical Visualizer on 5x4 NeoPixel Matrix ---
  // (Compute pixel colors with interrupts ENABLED so audio timing is never interrupted!)

  if (visual_mode == 0) {
    // Mode 0: Golden Baroque Candelabra & Warm Living Candlelight
    uint8_t flicker = (frame * 9 + current_step * 7) & 31;
    uint32_t c_gold   = pixels.Color(255 - flicker, 155 - (flicker >> 1), 10);
    uint32_t c_candle = pixels.Color(160, 55, 5);
    uint32_t c_hearth = pixels.Color(15, 4, 1);

    matrixFill(c_hearth);
    // 3 candelabra candles on columns 1, 2, 3
    for (uint8_t x = 1; x <= 3; x++) {
      matrixSet(x, 3, c_candle);
      matrixSet(x, 2, c_candle);
    }
    // Dynamic flame tops leaping with melody amplitude
    uint8_t flame_h = (env_lead >> 6) + 1;
    if (flame_h > 3) flame_h = 3;
    matrixSet(2, 3 - flame_h, c_gold);
    matrixSet(1, (current_step & 1) ? 1 : 0, c_gold);
    matrixSet(3, (current_step & 2) ? 0 : 1, c_gold);

  } else if (visual_mode == 1) {
    // Mode 1: Cathedral Stained Glass Rose Window
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t pattern = (x + y + (current_step >> 2)) & 3;
        uint32_t glass_color;
        switch (pattern) {
          case 0: glass_color = pixels.Color(10, 60, 255); break;  // Sapphire Blue
          case 1: glass_color = pixels.Color(240, 10, 35); break;  // Ruby Red
          case 2: glass_color = pixels.Color(10, 210, 60); break;  // Emerald Green
          default: glass_color = pixels.Color(180, 20, 220); break;// Imperial Amethyst
        }
        // Center cross is radiant gold
        if ((x == 2 && (y == 1 || y == 2)) || (y == 1 && (x == 1 || x == 3))) {
          glass_color = pixels.Color(255, 195, 20);
        }
        uint8_t scale = 120 + (env_lead >> 2);
        matrixSet(x, y, dimColor(glass_color, scale));
      }
    }

  } else if (visual_mode == 2) {
    // Mode 2: Polyphonic Counterpoint Harp (Independent Voice Spectrum)
    matrixFill(pixels.Color(5, 5, 15));

    // Voice 1 (Bass Continuo): Deep amber pillar on Column 0
    uint8_t b_h = (env_bass >> 6);
    for (uint8_t y = 0; y <= b_h && y < MATRIX_H; y++) {
      matrixSet(0, 3 - y, pixels.Color(255, 120, 15));
    }

    // Voice 2 (Arpeggio): Emerald & Cyan wave dancing across Columns 1 & 2
    uint8_t a_col = 1 + ((current_step >> 1) & 1);
    uint8_t a_row = (current_step + (frame >> 2)) % MATRIX_H;
    matrixSet(a_col, a_row, pixels.Color(20, 230, 140));
    matrixSet(3 - a_col, 3 - a_row, pixels.Color(15, 140, 240));

    // Voice 3 (Solo Cantabile): Brilliant gold & white soaring ribbon on Columns 3 & 4
    uint8_t m_col = 3 + (current_step & 1);
    uint8_t m_h = (env_lead >> 6);
    for (uint8_t y = 0; y <= m_h && y < MATRIX_H; y++) {
      matrixSet(m_col, 3 - y, pixels.Color(255, 230, 160));
    }

  } else if (visual_mode == 3) {
    // Mode 3: Moonlight on Lake Lucerne (Beethoven Nocturne)
    matrixFill(pixels.Color(3, 8, 35));

    // Full silver moon centered at (2, 0)
    matrixSet(2, 0, pixels.Color(240, 250, 255));
    matrixSet(1, 0, pixels.Color(50, 100, 190));
    matrixSet(3, 0, pixels.Color(50, 100, 190));

    // Moonlight ripples shimmering across the lake water surface
    uint8_t ripple_y = 1 + (frame % 3);
    uint8_t ripple_x = (2 + (current_step & 1) - ((current_step >> 1) & 1)) % MATRIX_W;
    matrixSet(ripple_x, ripple_y, pixels.Color(130, 215, 255));
    matrixSet((ripple_x + 1) % MATRIX_W, ripple_y, pixels.Color(40, 95, 180));

  } else if (visual_mode == 4) {
    // Mode 4: Royal Velvet & Imperial Gold Fleur-de-lis
    matrixFill(pixels.Color(28, 2, 16));
    uint8_t gold_pulse = 180 + (env_lead >> 2);
    uint32_t royal_gold = pixels.Color(gold_pulse, (gold_pulse * 3) >> 2, 10);
    matrixSet(2, 0, royal_gold);
    matrixSet(1, 1, royal_gold);
    matrixSet(2, 1, royal_gold);
    matrixSet(3, 1, royal_gold);
    matrixSet(2, 2, royal_gold);
    matrixSet(0, 3, royal_gold);
    matrixSet(4, 3, royal_gold);

  } else {
    // Mode 5: Prismatic Aurora Cantabile
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t wave = (frame * 4 + x * 25 + y * 35 + current_step * 5) & 0xFF;
        uint8_t r = (wave < 85) ? (wave * 3) : ((wave < 170) ? (255 - (wave - 85) * 3) : 0);
        uint8_t g = (wave < 85) ? (255 - wave * 3) : ((wave < 170) ? 0 : ((wave - 170) * 3));
        uint8_t b = (wave < 85) ? 0 : ((wave < 170) ? ((wave - 85) * 3) : (255 - (wave - 170) * 3));
        matrixSet(x, y, pixels.Color(r, g, b));
      }
    }
  }

  // Atomic, timing-protected LED refresh (only disables interrupts during ~600us pixel send!)
  cli();
  pixels.show();
  sei();

  frame++;
  my_delay(35); // Smooth ~28 FPS display update
}
