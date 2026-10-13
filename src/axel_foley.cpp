/* ---------------------------------------------------------------------------------------
    AXEL FOLEY ("AXEL F") — 80s RETRO CHIPTUNE SYNTH
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic 8-bit multi-voice synthesizer arrangement of Harold Faltermeyer's
    legendary 1984 theme "Axel F" from Beverly Hills Cop. Features the iconic
    staccato synth lead, driving Moog bassline, and 80s LinnDrum electro beats
    with interactive pot & button controls and retro police/synthwave visuals.

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Tempo / BPM Control: 90 - 150 BPM)
      PB2 (A1): Right Pot (80s Timbre & Filter: Moog Cutoff, Resonance, Sub-Bass)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle 80s Synth Voices (Jupiter-8, DX7, SID 8-Bit, Moog Saw)
                - Right Button: Cycle Visualizers (Police Siren, Outrun Grid, Boombox EQ, Neon Chase)
                - Both Buttons: Turbo 80s Machine-Gun Stutter & LinnDrum Roll!
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

// --- Note Frequencies (10kHz Sample Rate increments: inc = f_Hz * 65536 / 10000) ---
#define N_REST  0
#define N_F2    572
#define N_Ab2   680
#define N_Bb2   764
#define N_C3    857
#define N_Db3   908
#define N_Eb3   1019
#define N_F3    1144
#define N_C4    1715
#define N_Eb4   2039
#define N_F4    2289
#define N_G4    2569
#define N_Ab4   2722
#define N_Bb4   3055
#define N_C5    3429
#define N_Db5   3633
#define N_Eb5   4078
#define N_F5    4577

// Lookup table for Axel F notes (11 distinct note values)
const uint16_t note_table[11] PROGMEM = {
  N_REST, // 0
  N_C4,   // 1
  N_Eb4,  // 2
  N_F4,   // 3
  N_G4,   // 4
  N_Ab4,  // 5
  N_Bb4,  // 6
  N_C5,   // 7
  N_Db5,  // 8
  N_Eb5,  // 9
  N_F5    // 10
};

// --- Complete 128-Step (8 Bar) Axel F Arrangement in F Minor ---
// Exactly 16 sixteenth-notes per bar:
// Bars 1-3: The iconic main motif
// Bar 4:    Bass breakdown turnaround
// Bars 5-7: Theme variation
// Bar 8:    Ascending 80s synth run (F4 -> Ab4 -> Bb4 -> C5 -> Db5 -> Eb5 -> F5)
const uint8_t axel_melody[128] PROGMEM = {
  // Bar 1: F4 . . F4 . Ab4 . F4 | F4 . Bb4 F4 F4 . Eb4 .
   3,  0,  0,  3,  0,  5,  0,  3,  3,  0,  6,  3,  3,  0,  2,  0,
  // Bar 2: F4 . . C5 . F4 F4 . | Db5 . C5 . Ab4 . . .
   3,  0,  0,  7,  0,  3,  3,  0,  8,  0,  7,  0,  5,  0,  0,  0,
  // Bar 3: F4 . C5 . F5 . F4 . | Eb4 . Eb4 C4 G4 . F4 .
   3,  0,  7,  0, 10,  0,  3,  0,  2,  0,  2,  1,  4,  0,  3,  0,
  // Bar 4: Turnaround bass break
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,

  // Bar 5: Repeat Phrase 1
   3,  0,  0,  3,  0,  5,  0,  3,  3,  0,  6,  3,  3,  0,  2,  0,
  // Bar 6: Repeat Phrase 2
   3,  0,  0,  7,  0,  3,  3,  0,  8,  0,  7,  0,  5,  0,  0,  0,
  // Bar 7: Repeat Phrase 3
   3,  0,  7,  0, 10,  0,  3,  0,  2,  0,  2,  1,  4,  0,  3,  0,
  // Bar 8: Ascending synth lead climax!
   0,  0,  3,  0,  5,  0,  6,  0,  7,  0,  8,  0,  9,  0, 10,  0
};

// Root chord notes for each of the 8 bars
const uint16_t bar_roots[8] PROGMEM = {
  N_F2, N_F2, N_Db3, N_C3,
  N_F2, N_F2, N_Db3, N_C3
};

// --- Synth Engine State ---
volatile uint16_t phase_lead = 0;
volatile uint16_t inc_lead   = 0;
volatile uint8_t  env_lead   = 0;

volatile uint16_t phase_bass = 0;
volatile uint16_t inc_bass   = 0;
volatile uint8_t  env_bass   = 0;

volatile uint16_t phase_drum = 0;
volatile uint16_t drum_pitch = 0;
volatile uint8_t  env_drum   = 0;
volatile uint8_t  drum_type  = 0; // 0=None, 1=Linn Kick, 2=80s Snare/Clap, 3=Electro Hat

uint16_t lfsr = 0x5432; // 16-bit noise LFSR

// --- Sequencer State ---
volatile uint16_t ticks       = 0;
volatile uint16_t step_ticks  = 1280; // Default 117 BPM (~128ms per 16th note)
volatile uint8_t  curr_step   = 0;
volatile uint8_t  curr_bar    = 0;
volatile uint8_t  turbo_roll  = 0;

// Control Parameters
volatile uint8_t  voice_mode  = 0; // 0=Jupiter-8 Brass, 1=DX7 Bell, 2=C64 SID, 3=Moog Saw
volatile uint8_t  visual_mode = 0; // 0=Police Siren, 1=Outrun Grid, 2=Boombox EQ, 3=Neon Chase
volatile uint8_t  sub_depth   = 210;
volatile uint8_t  lead_bright = 210;

// Custom delay based on 10kHz ISR ticks
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

// --- 10kHz Audio & Sequencer Interrupt ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  static uint16_t tick_acc = 0;

  tick_acc++;
  uint16_t target_ticks = turbo_roll ? (step_ticks >> 2) : step_ticks;
  if (target_ticks < 140) target_ticks = 140;

  if (tick_acc >= target_ticks) {
    tick_acc = 0;

    if (turbo_roll) {
      // Machine-gun electro snare roll
      drum_type = 2;
      env_drum = 240;
      env_lead = 250;
      inc_lead = N_F5;
    } else {
      // Advance 128-step sequencer
      curr_step = (curr_step + 1) & 127;
      curr_bar  = curr_step >> 4;
      uint8_t bar_step = curr_step & 15;

      // 1. Lead Melody Note
      uint8_t note_idx = pgm_read_byte(&axel_melody[curr_step]);
      if (note_idx > 0) {
        inc_lead = pgm_read_word(&note_table[note_idx]);
        env_lead = 255; // Crisp staccato attack!
      } else {
        inc_lead = 0;
      }

      // 2. Driving 16th-Note Moog Synth Bass
      uint16_t root = pgm_read_word(&bar_roots[curr_bar]);
      if (bar_step == 0 || bar_step == 4 || bar_step == 8 || bar_step == 12) {
        // Driving downbeat pulse
        inc_bass = root;
        env_bass = 250;
      } else if (bar_step == 2 || bar_step == 6 || bar_step == 10 || bar_step == 14) {
        // Offbeat bounce (octave jump on 14)
        inc_bass = (bar_step == 14) ? (root << 1) : root;
        env_bass = 200;
      }

      // 3. 80s LinnDrum Electro Beat
      if (bar_step == 0 || bar_step == 8) {
        // Punchy LinnDrum Kick
        drum_type  = 1;
        drum_pitch = 1000;
        env_drum   = 255;
      } else if (bar_step == 4 || bar_step == 12) {
        // Gated Snare / Clap on 2 & 4
        drum_type  = 2;
        env_drum   = 240;
      } else if (bar_step == 2 || bar_step == 6 || bar_step == 10 || bar_step == 14) {
        // Crisp electro hat
        drum_type  = 3;
        env_drum   = 150;
      } else {
        drum_type  = 3;
        env_drum   = 60;
      }
    }
  }

  // --- Voice Envelopes & Noise ---
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  // Rapid staccato decays (giving that famous Harold Faltermeyer bite)
  if ((ticks & 7) == 0) {
    if (env_drum > 14) env_drum -= 14; else env_drum = 0;
    if (env_bass > 8)  env_bass -= 8;  else env_bass = 0;
  }
  if ((ticks & 15) == 0) {
    if (env_lead > 24) env_lead -= 24; // Fast staccato decay
    else env_lead = 0;
  }

  // --- Voice 1: 80s Lead Synthesis ---
  uint8_t s_lead = 0;
  if (inc_lead > 0 && env_lead > 0) {
    if (voice_mode == 0) {
      // Jupiter-8 Synth Brass (brassy saw with pulse width)
      phase_lead += inc_lead;
      uint8_t saw = phase_lead >> 8;
      uint8_t pulse = (phase_lead & 0x6000) ? 230 : 25;
      uint8_t brass = (saw + pulse) >> 1;
      s_lead = ((uint16_t)brass * env_lead) >> 8;
    } else if (voice_mode == 1) {
      // Yamaha DX7 FM Chime / Electric Bell
      phase_lead += inc_lead;
      uint8_t bell = phase_lead >> 8;
      if (bell > 127) bell = 255 - bell;
      // Add octave harmonic
      uint8_t h2 = (phase_lead * 2) >> 8;
      uint8_t dx = (bell + h2) >> 1;
      s_lead = ((uint16_t)dx * env_lead) >> 8;
    } else if (voice_mode == 2) {
      // Commodore 64 SID Lead (classic 8-bit pulse with vibrato)
      int8_t vib = (ticks & 0x20) ? 8 : -8;
      phase_lead += (inc_lead + vib);
      uint8_t sid = ((phase_lead >> 8) < 80) ? 245 : 10;
      s_lead = ((uint16_t)sid * env_lead) >> 8;
    } else {
      // Moog Resonant Saw
      phase_lead += inc_lead;
      uint8_t saw = phase_lead >> 8;
      s_lead = ((uint16_t)saw * env_lead) >> 8;
    }
    s_lead = ((uint16_t)s_lead * lead_bright) >> 8;
  }

  // --- Voice 2: Moog Bass ---
  uint8_t s_bass = 0;
  if (inc_bass > 0 && env_bass > 0) {
    phase_bass += inc_bass;
    uint8_t b_tri = phase_bass >> 8;
    if (b_tri > 127) b_tri = 255 - b_tri;
    uint8_t b_raw = b_tri << 1;
    s_bass = ((uint16_t)b_raw * env_bass) >> 8;
    s_bass = ((uint16_t)s_bass * sub_depth) >> 8;
  }

  // --- Voice 3: 80s LinnDrum Percussion ---
  uint8_t s_drum = 0;
  if (env_drum > 0) {
    if (drum_type == 1) {
      // Linn Kick: pitch-dropping punch
      phase_drum += (drum_pitch << 1);
      if (drum_pitch > 40) drum_pitch -= 2;
      uint8_t kw = phase_drum >> 8;
      if (kw > 127) kw = 255 - kw;
      s_drum = ((uint16_t)(kw << 1) * env_drum) >> 8;
    } else if (drum_type == 2) {
      // Gated 80s Snare / Clap: noise burst + mid crack
      phase_drum += 450;
      uint8_t crack = (phase_drum & 0x8000) ? 200 : 50;
      uint8_t noise = lfsr & 0xFF;
      uint8_t sn = (crack + noise) >> 1;
      s_drum = ((uint16_t)sn * env_drum) >> 8;
    } else {
      // Electro Hi-Hat
      uint8_t noise = lfsr & 0xFF;
      s_drum = ((uint16_t)noise * env_drum) >> 8;
    }
  }

  // --- 8-Bit PWM Mix Output (OC1A on PB1) ---
  uint16_t mix = (uint16_t)s_bass + ((uint16_t)s_lead << 1) + (uint16_t)s_drum;
  uint8_t out = mix >> 2;
  OCR1A = out;
}

void setup() {
  pixels.begin();
  pixels.setBrightness(100);

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // PLL for 64MHz Timer1 high-speed PWM audio on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
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
  static uint8_t last_btn_state = 0;

  // --- 1. Read Potentiometers ---
  uint16_t p_left  = analogRead(POTI_LEFT);  // Left Pot: Tempo (BPM)
  uint16_t p_right = analogRead(POTI_RIGHT); // Right Pot: 80s Filter / Timbre

  // Map Left Pot to tempo: 850 (147 BPM car chase) to 1800 (69 BPM chill synthwave)
  step_ticks = map(p_left, 0, 1023, 850, 1800);

  // Map Right Pot to 80s filter cutoff & bass punch
  sub_depth   = map(p_right, 0, 1023, 110, 255);
  lead_bright = map(p_right, 0, 1023, 130, 255);

  // --- 2. Read Buttons (Resistor Ladder on A3) ---
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

  // Both buttons: Machine-gun stutter roll!
  if (curr_btn_state == 3) {
    turbo_roll = 1;
  } else {
    turbo_roll = 0;
  }

  // Edge detection for taps
  if (curr_btn_state != 0 && last_btn_state == 0) {
    if (curr_btn_state == 1) {
      // Left Button: Cycle 80s Synth Voices
      voice_mode = (voice_mode + 1) % 4;
      cli();
      uint32_t col = (voice_mode == 0) ? pixels.Color(255, 120, 0) :  // Jupiter-8 Gold
                     (voice_mode == 1) ? pixels.Color(0, 220, 255) :  // DX7 Electric Cyan
                     (voice_mode == 2) ? pixels.Color(0, 255, 100) :  // C64 Green
                                         pixels.Color(255, 0, 180);   // Moog Magenta
      matrixFill(col);
      pixels.show();
      sei();
      my_delay(150);
    } else if (curr_btn_state == 2) {
      // Right Button: Cycle Retro Visualizers
      visual_mode = (visual_mode + 1) % 4;
      cli();
      matrixFill(pixels.Color(255, 255, 255));
      pixels.show();
      sei();
      my_delay(120);
    }
  }
  last_btn_state = curr_btn_state;

  // --- 3. Dynamic NeoPixel Visualizers (5x4 Matrix) ---
  cli(); // Protect timing during bit-bang

  if (turbo_roll) {
    // Turbo Mode: Police strobe siren flash
    uint32_t siren = ((frame & 1) == 0) ? pixels.Color(255, 0, 0) : pixels.Color(0, 0, 255);
    matrixFill(siren);
  } else if (visual_mode == 0) {
    // Mode 0: Beverly Hills Police Cruiser Beacon
    // Left side: Red emergency light; Right side: Blue emergency light
    uint8_t strobe_phase = (frame * 2 + (curr_step >> 1)) & 7;
    uint32_t red_col  = (strobe_phase < 4) ? pixels.Color(255, 0, 0) : pixels.Color(30, 0, 0);
    uint32_t blue_col = (strobe_phase >= 4) ? pixels.Color(0, 40, 255) : pixels.Color(0, 5, 30);
    uint32_t white_flash = (env_drum > 160) ? pixels.Color(255, 255, 255) : pixels.Color(20, 20, 20);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      matrixSet(0, y, red_col);
      matrixSet(1, y, red_col);
      matrixSet(2, y, white_flash);
      matrixSet(3, y, blue_col);
      matrixSet(4, y, blue_col);
    }
  } else if (visual_mode == 1) {
    // Mode 1: Outrun 80s Synthwave Sunset & Moving Grid
    // Top 2 rows: Neon Synthwave Sun & Dusk
    uint8_t sun_glow = 130 + (env_lead >> 1);
    uint32_t sun_col = pixels.Color(sun_glow, 20, 160); // Neon Magenta/Pink
    matrixSet(1, 0, sun_col);
    matrixSet(2, 0, pixels.Color(255, 220, 0)); // Golden core
    matrixSet(3, 0, sun_col);
    matrixSet(0, 0, pixels.Color(30, 0, 60));
    matrixSet(4, 0, pixels.Color(30, 0, 60));

    for (uint8_t x = 0; x < MATRIX_W; x++) {
      matrixSet(x, 1, pixels.Color(80, 0, 120)); // Dusk horizon
    }

    // Bottom 2 rows: Moving Cyber Grid
    uint8_t grid_offset = (frame + curr_step) & 3;
    uint32_t cyan_grid = pixels.Color(0, 220, 255);
    uint32_t dark_grid = pixels.Color(0, 20, 40);

    for (uint8_t x = 0; x < MATRIX_W; x++) {
      bool is_line = ((x + grid_offset) % 2 == 0);
      matrixSet(x, 2, is_line ? cyan_grid : dark_grid);
      matrixSet(x, 3, (env_bass > 100 && is_line) ? pixels.Color(255, 0, 180) : dark_grid);
    }
  } else if (visual_mode == 2) {
    // Mode 2: 80s Boombox Graphic Equalizer
    // Col 0: Kick, Col 1: Bass, Col 2: Snare, Col 3: Lead, Col 4: Timbre
    uint8_t h_kick  = (env_drum > 150) ? 4 : ((env_drum > 60) ? 2 : 0);
    uint8_t h_bass  = (env_bass > 180) ? 4 : ((env_bass > 90) ? 2 : ((env_bass > 20) ? 1 : 0));
    uint8_t h_snare = (drum_type == 2 && env_drum > 50) ? 4 : 0;
    uint8_t h_lead  = (env_lead > 160) ? 4 : ((env_lead > 80) ? 2 : ((env_lead > 20) ? 1 : 0));
    uint8_t h_tim   = (sub_depth >> 6) + 1;

    uint8_t heights[5] = { h_kick, h_bass, h_snare, h_lead, h_tim };
    // Retro boombox LED colors: Green at bottom, Yellow at mid, Red at top
    uint32_t row_colors[4] = {
      pixels.Color(255, 0, 0),    // Top: Red peak
      pixels.Color(255, 140, 0),  // Row 1: Orange
      pixels.Color(255, 230, 0),  // Row 2: Yellow
      pixels.Color(0, 255, 40)    // Row 3: Green base
    };

    for (uint8_t x = 0; x < MATRIX_W; x++) {
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t bar_y = 3 - y;
        if (bar_y < heights[x]) {
          matrixSet(x, y, row_colors[y]);
        } else {
          matrixSet(x, y, pixels.Color(5, 5, 10));
        }
      }
    }
  } else {
    // Mode 3: Miami Vice Neon Chase
    matrixFade(75);
    uint8_t chase_x = (frame * 2 + curr_step) % MATRIX_W;
    uint8_t chase_y = (frame + (curr_step >> 2)) % MATRIX_H;
    uint32_t neon_col = (frame & 1) ? pixels.Color(0, 255, 230) : pixels.Color(255, 0, 180);
    matrixSet(chase_x, chase_y, neon_col);

    if (env_drum > 160 && drum_type == 2) {
      // Snare flash corner flares
      matrixSet(0, 0, pixels.Color(255, 255, 255));
      matrixSet(4, 0, pixels.Color(255, 255, 255));
    }
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  my_delay(40); // 25 FPS visual update
}
