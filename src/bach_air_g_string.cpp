/* ---------------------------------------------------------------------------------------
    J.S. BACH: AIR ON THE G STRING & CLASSICAL BAROQUE SUITE
    For ATtiny85 / 8Bit Mixtape (AudioBoot)
    
    A 3-voice polyphonic classical synthesizer engine featuring:
      - Movement 0: J.S. Bach — Air on the G String (BWV 1068)
      - Movement 1: Johann Pachelbel — Canon in D Major
      - Movement 2: Antonio Vivaldi — Winter (L'Inverno - Largo)
      - Movement 3: Ludwig van Beethoven — Moonlight Sonata (Adagio sostenuto)

    Sound Engine:
      - Voice 1 (Bass): Basso Continuo / Cello pedal drone with warm sub-harmonics
      - Voice 2 (Arp): Baroque Continuo / Plucked Harpsichord / Organ Flute
      - Voice 3 (Lead): Cantabile Soloist with delayed expressive classical vibrato
      - 4 Instrument Timbres: Baroque Chamber, Cathedral Organ, Glass Harmonica, Switched-On Moog
      - 4 NeoPixel Visualizer Modes on 5x4 Matrix (Golden Candelabra, Stained Glass, Polyphonic Harp, Moonlight Serenade)

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL @ 250kHz carrier)
      PB4 (A2): Left Pot (Tempo & Agogic Timing: 70ms - 220ms per step)
      PB2 (A1): Right Pot (Acoustic Reverb Tail & Harmony / Continuous Balance)
      PB3 (A3): Resistor Ladder Buttons
                - Short Press Left:  Cycle Classical Piece / Movement
                - Short Press Right: Cycle Instrument Timbre
                - Both Buttons:      Cycle NeoPixel Visualizer Theme
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

// Button Voltage Ladder Calibration
#define Vbutton_releaseLevel  490
#define Vbutton_left          460
#define Vbutton_right         395
#define Vbutton_both          320

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Note Frequencies (10kHz sample rate: inc = f_Hz * 65536 / 10000) ---
#define N_REST 0

// Octave 1 & 2 (Bass Continuo)
#define N_C2   429
#define N_Cs2  454
#define N_D2   481
#define N_Eb2  510
#define N_Ds2  510
#define N_E2   540
#define N_F2   572
#define N_Fs2  606
#define N_G2   642
#define N_Gs2  680
#define N_A2   721
#define N_Bb2  764
#define N_B2   809

// Octave 3 (Tenor / Inner Voice)
#define N_C3   857
#define N_Cs3  908
#define N_D3   962
#define N_Eb3  1020
#define N_Ds3  1020
#define N_E3   1080
#define N_F3   1144
#define N_Fs3  1212
#define N_G3   1285
#define N_Gs3  1361
#define N_A3   1442
#define N_Bb3  1528
#define N_B3   1618

// Octave 4 (Soprano / Melody)
#define N_C4   1715
#define N_Cs4  1817
#define N_D4   1925
#define N_Eb4  2039
#define N_Ds4  2039
#define N_E4   2160
#define N_F4   2289
#define N_Fs4  2425
#define N_G4   2569
#define N_Gs4  2722
#define N_A4   2884
#define N_Bb4  3055
#define N_B4   3237

// Octave 5 (High Cantabile)
#define N_C5   3429
#define N_Cs5  3633
#define N_D5   3849
#define N_Eb5  4078
#define N_Ds5  4078
#define N_E5   4321
#define N_F5   4577
#define N_Fs5  4850
#define N_G5   5138
#define N_A5   5767

// --- 64-step Musical Scores in Flash (PROGMEM) ---

// --- Movement 0: J.S. Bach — Air on the G String (BWV 1068) ---
const uint16_t bach_melody[64] PROGMEM = {
  // M1: F#4 held, then G4, A4
  N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_G4,  N_A4,
  // M2: B4 held, then A4, G4, F#4, E4, C#4
  N_B4,  N_B4,  N_B4,  N_A4,  N_G4,  N_Fs4, N_E4,  N_Cs4,
  // M3: D4 held, then E4, F#4, G4, A4, B4
  N_D4,  N_D4,  N_D4,  N_E4,  N_Fs4, N_G4,  N_A4,  N_B4,
  // M4: C#5 held, then B4, A4, G#4, B4, G#4
  N_Cs5, N_Cs5, N_Cs5, N_B4,  N_A4,  N_Gs4, N_B4,  N_Gs4,
  // M5: A4 held, then B4, C#5
  N_A4,  N_A4,  N_A4,  N_A4,  N_A4,  N_A4,  N_B4,  N_Cs5,
  // M6: D5 held, C#5, B4
  N_D5,  N_D5,  N_D5,  N_D5,  N_Cs5, N_Cs5, N_B4,  N_B4,
  // M7: A4, G4, F#4, E4
  N_A4,  N_A4,  N_G4,  N_G4,  N_Fs4, N_Fs4, N_E4,  N_E4,
  // M8: Cadence in D Major
  N_D4,  N_D4,  N_D4,  N_D4,  N_D4,  N_D4,  N_D4,  N_D4
};

const uint16_t bach_bass[64] PROGMEM = {
  // Iconic walking bass with octave drop
  N_D3,  N_D2,  N_Cs3, N_Cs2, N_B2,  N_B2,  N_A2,  N_A2,
  N_G2,  N_G2,  N_Fs2, N_Fs2, N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D3,  N_Cs3, N_B2,  N_A2,  N_G2,  N_Fs2, N_E2,
  N_Fs2, N_G2,  N_Gs2, N_A2,  N_A2,  N_G2,  N_Fs2, N_E2,
  N_A2,  N_A2,  N_G2,  N_Fs2, N_E2,  N_D2,  N_Cs2, N_A2,
  N_D3,  N_D2,  N_Cs3, N_Cs2, N_B2,  N_B2,  N_A2,  N_A2,
  N_G2,  N_Fs2, N_E2,  N_D2,  N_A2,  N_A2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_A2,  N_A2,  N_D2,  N_D2,  N_D2,  N_D2
};

const uint16_t bach_arp[64] PROGMEM = {
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
const uint16_t pachelbel_melody[64] PROGMEM = {
  // Theme A: Serene hymn
  N_Fs4, N_Fs4, N_Fs4, N_Fs4, N_E4,  N_E4,  N_E4,  N_E4,
  N_D4,  N_D4,  N_D4,  N_D4,  N_Cs4, N_Cs4, N_Cs4, N_Cs4,
  N_B3,  N_B3,  N_B3,  N_B3,  N_A3,  N_A3,  N_A3,  N_A3,
  N_B3,  N_B3,  N_B3,  N_B3,  N_Cs4, N_Cs4, N_Cs4, N_Cs4,
  // Theme B: Dancing quarter variations
  N_D4,  N_D4,  N_Fs4, N_Fs4, N_A4,  N_A4,  N_G4,  N_G4,
  N_Fs4, N_Fs4, N_D4,  N_D4,  N_Fs4, N_Fs4, N_E4,  N_E4,
  N_D4,  N_D4,  N_B3,  N_B3,  N_D4,  N_D4,  N_A3,  N_A3,
  N_G3,  N_G3,  N_B3,  N_B3,  N_A3,  N_A3,  N_G3,  N_G3
};

const uint16_t pachelbel_bass[64] PROGMEM = {
  // Pachelbel 8-chord ground bass (repeated twice)
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2,
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2,
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2,
  N_D3,  N_D2,  N_A2,  N_A2,  N_B2,  N_B2,  N_Fs2, N_Fs2,
  N_G2,  N_G2,  N_D2,  N_D2,  N_G2,  N_G2,  N_A2,  N_A2
};

const uint16_t pachelbel_arp[64] PROGMEM = {
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
const uint16_t vivaldi_melody[64] PROGMEM = {
  N_Fs4, N_Fs4, N_Fs4, N_G4,  N_A4,  N_A4,  N_A4,  N_A4,
  N_D5,  N_D5,  N_D5,  N_D5,  N_Cs5, N_Cs5, N_B4,  N_B4,
  N_A4,  N_A4,  N_A4,  N_A4,  N_B4,  N_A4,  N_G4,  N_Fs4,
  N_E4,  N_E4,  N_Fs4, N_G4,  N_A4,  N_A4,  N_A4,  N_A4,
  N_Fs4, N_Fs4, N_Fs4, N_D4,  N_E4,  N_Fs4, N_G4,  N_A4,
  N_B4,  N_B4,  N_B4,  N_Cs5, N_D5,  N_D5,  N_D5,  N_D5,
  N_Cs5, N_Cs5, N_B4,  N_B4,  N_A4,  N_A4,  N_G4,  N_G4,
  N_Fs4, N_Fs4, N_E4,  N_E4,  N_D4,  N_D4,  N_D4,  N_D4
};

const uint16_t vivaldi_bass[64] PROGMEM = {
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_Fs2, N_Fs2, N_G2,  N_G2,  N_A2,  N_A2,
  N_D2,  N_D2,  N_B2,  N_B2,  N_E2,  N_E2,  N_D2,  N_D2
};

const uint16_t vivaldi_arp[64] PROGMEM = {
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
const uint16_t moonlight_melody[64] PROGMEM = {
  N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST,
  N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST, N_REST,
  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,
  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,
  N_A3,   N_A3,   N_A3,   N_A3,   N_Gs3,  N_Gs3,  N_Fs3,  N_Fs3,
  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,  N_Gs3,
  N_E3,   N_E3,   N_E3,   N_E3,   N_Ds3,  N_Ds3,  N_Ds3,  N_Ds3,
  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3,  N_Cs3
};

const uint16_t moonlight_bass[64] PROGMEM = {
  N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_B2,  N_B2,  N_B2,  N_B2,
  N_A2,  N_A2,  N_A2,  N_A2,  N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_B2,  N_B2,  N_B2,  N_B2,
  N_A2,  N_A2,  N_A2,  N_A2,  N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Fs2, N_Fs2, N_Fs2, N_Fs2, N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Gs2, N_Gs2, N_Gs2, N_Gs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2,
  N_Fs2, N_Fs2, N_Fs2, N_Fs2, N_Gs2, N_Gs2, N_Gs2, N_Gs2,
  N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2, N_Cs2
};

const uint16_t moonlight_arp[64] PROGMEM = {
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_A3,  N_Cs4, N_E4,  N_Cs4, N_A3,  N_D4,  N_Fs4, N_D4,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_B3,  N_Ds4, N_B3,
  N_A3,  N_Cs4, N_Fs4, N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_Fs3, N_C4,  N_Ds4, N_C4,  N_Gs3, N_B3,  N_Ds4, N_B3,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4,
  N_Gs3, N_Cs4, N_E4,  N_Cs4, N_Gs3, N_Cs4, N_E4,  N_Cs4
};

// --- Audio Engine State Variables ---
volatile uint16_t phase_bass = 0;
volatile uint16_t inc_bass   = N_D2;
volatile uint16_t phase_arp  = 0;
volatile uint16_t inc_arp    = N_A3;
volatile uint16_t phase_lead = 0;
volatile uint16_t inc_lead   = N_Fs4;

volatile uint8_t env_lead    = 240;
volatile uint8_t env_arp     = 180;
volatile uint8_t env_bass    = 200;
volatile uint8_t note_age    = 0;

volatile uint8_t current_piece  = 0; // 0=Bach Air, 1=Pachelbel Canon, 2=Vivaldi Winter, 3=Beethoven Moonlight
volatile uint8_t timbre_mode    = 0; // 0=Chamber Strings, 1=Cathedral Organ, 2=Glass Harmonica, 3=Switched-On Moog
uint8_t visual_mode             = 0; // 0=Golden Candelabra, 1=Stained Glass, 2=Harp Strings, 3=Moonlight Serenade

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

  // --- Real-time Hardware Sequencer ---
  static uint16_t tick_acc = 0;
  static uint16_t last_lead_note = 0;

  tick_acc++;
  if (tick_acc >= step_ticks) {
    tick_acc = 0;
    current_step = (current_step + 1) & 63; // Clean 64-step loop

    // Read notes from active movement in Flash
    uint16_t n_lead = 0;
    uint16_t n_bass = 0;
    uint16_t n_arp  = 0;

    switch (current_piece) {
      case 0: // Bach Air
        n_lead = pgm_read_word(&bach_melody[current_step]);
        n_bass = pgm_read_word(&bach_bass[current_step]);
        n_arp  = pgm_read_word(&bach_arp[current_step]);
        break;
      case 1: // Pachelbel Canon
        n_lead = pgm_read_word(&pachelbel_melody[current_step]);
        n_bass = pgm_read_word(&pachelbel_bass[current_step]);
        n_arp  = pgm_read_word(&pachelbel_arp[current_step]);
        break;
      case 2: // Vivaldi Winter
        n_lead = pgm_read_word(&vivaldi_melody[current_step]);
        n_bass = pgm_read_word(&vivaldi_bass[current_step]);
        n_arp  = pgm_read_word(&vivaldi_arp[current_step]);
        break;
      default: // Beethoven Moonlight
        n_lead = pgm_read_word(&moonlight_melody[current_step]);
        n_bass = pgm_read_word(&moonlight_bass[current_step]);
        n_arp  = pgm_read_word(&moonlight_arp[current_step]);
        break;
    }

    // Lead voice articulation & legato check
    if (n_lead != last_lead_note) {
      inc_lead = n_lead;
      last_lead_note = n_lead;
      env_lead = (n_lead == N_REST) ? 0 : 250;
      note_age = 0;
    }

    // Arpeggio / Continuo pluck trigger
    inc_arp = n_arp;
    env_arp = 220;

    // Bass note step
    inc_bass = n_bass;
    env_bass = 210;
  }

  // Envelope decay & articulation
  if ((ticks & 15) == 0) {
    if (note_age < 255) note_age++;
    // Subtle decay on lead
    if (env_lead > 130) env_lead -= 1;
    // Faster decay on plucked harpsichord arpeggio
    if (env_arp > 40) env_arp -= (timbre_mode == 0) ? 4 : 2;
    // Warm cello bass decay
    if (env_bass > 100) env_bass -= 1;
  }

  // --- Voice 1: Basso Continuo (Cello / Violone) ---
  phase_bass += inc_bass;
  uint8_t tb = (phase_bass >> 8);
  if (tb > 127) tb = 255 - tb; // Triangle (0..127)
  // Sub-octave harmonic warmth
  uint8_t sub = (phase_bass >> 9);
  if (sub > 127) sub = 255 - sub;
  uint8_t raw_bass = (tb + (sub >> 1));
  uint8_t s_bass = ((uint16_t)raw_bass * env_bass) >> 8;
  s_bass = ((uint16_t)s_bass * sub_bass_depth) >> 8;

  // --- Voice 2: Arpeggiated Continuo / Inner Voice ---
  phase_arp += inc_arp;
  uint8_t s_arp = 0;
  if (timbre_mode == 0) {
    // Harpsichord / Lute: bright plucked attack
    uint8_t saw = phase_arp >> 8;
    uint8_t pluck = (saw >> 1) + ((phase_arp & 0x4000) ? 64 : 0);
    s_arp = ((uint16_t)pluck * env_arp) >> 8;
  } else if (timbre_mode == 1) {
    // Pipe Organ Flute: soft rounded triangle
    uint8_t ta = (phase_arp >> 8);
    if (ta > 127) ta = 255 - ta;
    s_arp = ((uint16_t)(ta << 1) * env_arp) >> 8;
  } else if (timbre_mode == 2) {
    // Glass Harmonica / Music Box: celestial crystalline shimmer
    uint8_t ta = (phase_arp >> 8);
    if (ta > 127) ta = 255 - ta;
    uint8_t shimmer = (phase_arp >> 7);
    s_arp = ((uint16_t)(ta ^ (shimmer & 0x1F)) * env_arp) >> 8;
  } else {
    // Switched-On Moog: resonant saw
    uint8_t saw = phase_arp >> 8;
    s_arp = ((uint16_t)saw * env_arp) >> 8;
  }

  // --- Voice 3: Solo Cantabile (Violin / Solo Organ / Flute) ---
  // Delayed Classical Vibrato: swells gracefully after note onset
  int8_t vib = 0;
  if (note_age > 16) {
    uint8_t vib_step = (ticks >> 4) & 31;
    int8_t vib_shape = (vib_step < 16) ? (vib_step - 8) : (24 - vib_step);
    // Vibrato depth swells naturally with note duration
    uint8_t v_depth = (note_age > 60) ? 8 : 4;
    vib = (vib_shape * v_depth) >> 3;
  }
  phase_lead += (inc_lead + vib);

  uint8_t s_lead = 0;
  if (inc_lead != N_REST) {
    if (timbre_mode == 0) {
      // Baroque Cantabile Violin: smooth triangle with singing tone
      uint8_t tl = (phase_lead >> 8);
      if (tl > 127) tl = 255 - tl;
      s_lead = ((uint16_t)(tl << 1) * env_lead) >> 8;
    } else if (timbre_mode == 1) {
      // Cathedral Organ Principal (8' + 4' dual stop)
      uint8_t tl1 = (phase_lead >> 8);
      if (tl1 > 127) tl1 = 255 - tl1;
      uint8_t tl2 = (phase_lead >> 7);
      if (tl2 > 127) tl2 = 255 - tl2;
      uint8_t organ = (tl1 * 3 + tl2) >> 2;
      s_lead = ((uint16_t)(organ << 1) * env_lead) >> 8;
    } else if (timbre_mode == 2) {
      // Glass Harmonica: pure crystalline flute
      uint8_t tl = (phase_lead >> 8);
      if (tl > 127) tl = 255 - tl;
      s_lead = ((uint16_t)(tl << 1) * env_lead) >> 8;
    } else {
      // Switched-On Moog: PWM Brass
      static uint8_t pwm_width = 128;
      uint8_t pulse = ((phase_lead >> 8) > pwm_width) ? 220 : 35;
      s_lead = ((uint16_t)pulse * env_lead) >> 8;
    }
  }

  // --- 3-Voice Polyphonic Master Mix with Reverb Headroom ---
  uint16_t mix = ((uint16_t)s_bass * 160) >> 8;
  mix += ((uint16_t)s_arp * (140 + (reverb_tail >> 2))) >> 8;
  mix += ((uint16_t)s_lead * 240) >> 8;

  if (mix > 255) mix = 255;
  OCR1A = mix;
}

void setup() {
  pixels.begin();
  pixels.setBrightness(110);

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

  // Read Potentiometers
  uint16_t p_tempo  = analogRead(POTI_LEFT);  // PB4: Tempo / Rubato
  uint16_t p_reverb = analogRead(POTI_RIGHT); // PB2: Reverb Tail & Sub Depth

  // Update real-time musical parameters
  // Map step_ticks: 700 (70ms = lively allegro) to 2200 (220ms = majestic largo)
  step_ticks = map(p_tempo, 0, 1023, 700, 2200);
  reverb_tail = map(p_reverb, 0, 1023, 40, 240);
  sub_bass_depth = map(p_reverb, 0, 1023, 110, 250);

  // Read Navigation Buttons
  uint16_t btn = analogRead(BUTTON_PIN);
  if (btn < Vbutton_releaseLevel) {
    if (btn < Vbutton_both) {
      // Both Buttons: Cycle Visualizer Mode
      visual_mode = (visual_mode + 1) & 3;
    } else if (btn < Vbutton_right) {
      // Right Button: Cycle Instrument Timbre Mode
      timbre_mode = (timbre_mode + 1) & 3;
    } else if (btn < Vbutton_left) {
      // Left Button: Cycle Classical Piece / Movement
      current_piece = (current_piece + 1) & 3;
      current_step = 0; // Restart at beginning of piece
    }
    my_delay(260); // Clean hardware tick debounce
  }

  // --- Classical Visualizer on 5x4 NeoPixel Matrix ---
  cli(); // Protect WS2812B 800kHz protocol timing

  if (visual_mode == 0) {
    // Mode 0: Golden Baroque Candelabra & Warm Candlelight
    // Warm golds, amber, and gentle dynamic glow
    uint8_t flicker = (frame * 7 + current_step * 5) & 31;
    uint32_t c_gold   = pixels.Color(255 - flicker, 175 - (flicker >> 1), 10);
    uint32_t c_candle = pixels.Color(180, 70, 5);
    uint32_t c_bg     = pixels.Color(12, 4, 1);

    matrixFill(c_bg);
    // Draw 3 candelabra candles at cols 1, 2, 3
    for (uint8_t x = 1; x <= 3; x++) {
      matrixSet(x, 3, c_candle);
      matrixSet(x, 2, c_candle);
    }
    // Animated flickering candle flame heights based on melody and arpeggio
    uint8_t flame_h = (env_lead >> 6) + 1;
    if (flame_h > 3) flame_h = 3;
    matrixSet(2, 3 - flame_h, c_gold);
    matrixSet(1, (current_step & 1) ? 1 : 0, c_gold);
    matrixSet(3, (current_step & 2) ? 0 : 1, c_gold);

  } else if (visual_mode == 1) {
    // Mode 1: Cathedral Stained Glass Rose Window
    // Majestic jewel tones: sapphire, ruby, emerald, amethyst
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t pattern = (x + y + (current_step >> 2)) & 3;
        uint32_t glass_color;
        switch (pattern) {
          case 0: glass_color = pixels.Color(0, 40, 220); break;   // Sapphire Blue
          case 1: glass_color = pixels.Color(210, 10, 30); break;  // Ruby Red
          case 2: glass_color = pixels.Color(10, 190, 60); break;  // Emerald Green
          default: glass_color = pixels.Color(170, 20, 200); break;// Amethyst Purple
        }
        // Pulse brightness with active voice dynamics
        uint8_t scale = 110 + (env_lead >> 2);
        matrixSet(x, y, dimColor(glass_color, scale));
      }
    }

  } else if (visual_mode == 2) {
    // Mode 2: Polyphonic Counterpoint Harp Strings
    // Col 0 = Basso Continuo, Cols 1-2 = Arpeggios, Cols 3-4 = Soaring Melody
    matrixFill(pixels.Color(5, 5, 12));

    // Bass string (Warm Amber)
    uint8_t b_height = (env_bass >> 6);
    for (uint8_t y = 0; y <= b_height && y < MATRIX_H; y++) {
      matrixSet(0, 3 - y, pixels.Color(240, 130, 20));
    }

    // Arpeggio waves (Verdant Emerald & Jade)
    uint8_t a_col = 1 + ((current_step >> 1) & 1);
    uint8_t a_row = current_step % MATRIX_H;
    matrixSet(a_col, a_row, pixels.Color(30, 220, 140));

    // Soaring Cantabile Melody (Celestial Gold & Brilliant White)
    uint8_t m_col = 3 + (current_step & 1);
    uint8_t m_height = (env_lead >> 6);
    for (uint8_t y = 0; y <= m_height && y < MATRIX_H; y++) {
      matrixSet(m_col, 3 - y, pixels.Color(255, 230, 160));
    }

  } else {
    // Mode 3: Moonlight Serenade (Deep Lake Azure & Silver Reflection)
    uint32_t water_blue = pixels.Color(3, 15, 60);
    matrixFill(water_blue);

    // Moonlight reflection shifting gently across columns
    uint8_t moon_x = 2;
    matrixSet(moon_x, 0, pixels.Color(240, 250, 255)); // Full Moon

    // Water ripple reflections responding to the arpeggio
    uint8_t ripple_y = 1 + (frame % 3);
    uint8_t ripple_x = (moon_x + (current_step & 1) - ((current_step >> 1) & 1)) % MATRIX_W;
    matrixSet(ripple_x, ripple_y, pixels.Color(120, 200, 255));
    matrixSet((ripple_x + 1) % MATRIX_W, ripple_y, pixels.Color(40, 90, 180));
  }

  pixels.show();
  sei(); // Restore audio interrupt immediately

  frame++;
  my_delay(40); // 25 FPS display update, crystal clear audio
}
