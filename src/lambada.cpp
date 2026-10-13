/* ---------------------------------------------------------------------------------------
    LAMBADA ("CHORANDO SE FOI") — TROPICAL CARNIVAL SYNTH
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic 8-bit multi-voice synthesizer arrangement of Kaoma's "Lambada"
    featuring the legendary accordion hook, syncopated Carimbó bassline,
    and Latin Zouk percussion groove with interactive button & pot controls.

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Tempo / BPM Control: 70 - 160 BPM)
      PB2 (A1): Right Pot (Groove & Timbre: Bass Punch, Filter & Brightness)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Synth Voices (Accordion, Marimba, 8-Bit, Saw)
                - Right Button: Cycle Visualizers (Copacabana, Dancers, EQ, Fire)
                - Both Buttons: Carnival Turbo Stutter & Snare Roll!
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
#define N_D2    481
#define N_F2    572
#define N_G2    642
#define N_A2    721
#define N_Bb2   764
#define N_C3    857
#define N_Cs3   908
#define N_D3    962
#define N_E3    1080
#define N_F3    1144
#define N_G3    1285
#define N_A3    1442
#define N_Bb3   1528
#define N_C4    1715
#define N_Cs4   1817
#define N_D4    1925
#define N_E4    2160
#define N_F4    2289
#define N_G4    2569
#define N_A4    2884
#define N_Bb4   3055
#define N_C5    3429
#define N_Cs5   3633
#define N_D5    3849
#define N_E5    4320
#define N_F5    4577
#define N_G5    5138
#define N_A5    5767
#define N_Bb5   6110
#define N_C6    6858
#define N_D6    7698

// Lookup table for Lambada notes (14 distinct note values)
const uint16_t note_table[14] PROGMEM = {
  N_REST, // 0
  N_G4,   // 1
  N_A4,   // 2
  N_Bb4,  // 3
  N_C5,   // 4
  N_Cs5,  // 5
  N_D5,   // 6
  N_E5,   // 7
  N_F5,   // 8
  N_G5,   // 9
  N_A5,   // 10
  N_Bb5,  // 11
  N_C6,   // 12
  N_D6    // 13
};

// --- Complete 256-Step (16 Bar) Lambada Melody ---
// Exactly 16 sixteenth-notes per bar:
// Bars 1-4:  Iconic Accordion Hook Part 1
// Bars 5-8:  Accordion Hook Part 2 (G5 variation + pickup)
// Bars 9-12: Verse Phrase 1 ("Chorando se foi quem um dia so me fez chorar...")
// Bars 13-16: Verse Phrase 2 ("Chorando estara ao lembrar de um amor...")
const uint8_t lambada_melody[256] PROGMEM = {
  // Bar 1: Hook opening: E F E D | C# D E - | - - A A | F E
   7,  8,  7,  6,  5,  6,  7,  7,  0,  0, 10, 10, 10, 10,  8,  7,
  // Bar 2: D hold | - - - - | E F E D | C# D C# A
   6,  6,  6,  6,  0,  0,  0,  0,  7,  8,  7,  6,  5,  6,  5,  2,
  // Bar 3: C# D E F | E D C# D | D hold | - - - -
   5,  6,  7,  8,  7,  6,  5,  6,  6,  6,  6,  6,  0,  0,  0,  0,
  // Bar 4: Turnaround / pickup
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  6,
  // Bar 5: Hook variation opening
   7,  8,  7,  6,  5,  6,  7,  7,  0,  0, 10, 10, 10, 10,  8,  7,
  // Bar 6: D hold | - - - - | E F E D | C# D C# A
   6,  6,  6,  6,  0,  0,  0,  0,  7,  8,  7,  6,  5,  6,  5,  2,
  // Bar 7: C# D E F | G F E F | D hold | - - - -
   5,  6,  7,  8,  9,  8,  7,  8,  6,  6,  6,  6,  0,  0,  0,  0,
  // Bar 8: Pickup to verse ("Cho-ran-do...")
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  6,  8,
  // Bar 9: Verse 1: "Cho - ran - do se foi..."
  10, 10, 10, 10, 10, 10,  9,  9,  8,  8,  9,  9, 10, 10, 10, 10,
  // Bar 10: "...quem um dia..."
   0,  0,  8,  8,  6,  6,  6,  6,  6,  6,  6,  6,  0,  0,  1,  1,
  // Bar 11: "...so me fez chorar..."
   4,  4,  7,  7,  9,  9,  9,  9,  9,  9,  8,  8,  7,  7,  8,  8,
  // Bar 12: Resolution
   9,  9,  9,  9,  0,  0,  7,  7,  6,  6,  6,  6,  0,  0,  2,  2,
  // Bar 13: Verse 2: "Chorando estara..."
   6,  6,  8,  8, 10, 10, 10, 10, 10, 10,  9,  9,  8,  8,  9,  9,
  // Bar 14: "...ao lembrar de um amor..."
  10, 10, 10, 10,  0,  0,  8,  8,  6,  6,  6,  6,  0,  0,  1,  1,
  // Bar 15: "...que um dia nao soube cuidar"
   4,  4,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  8,  8,  7,  7,
  // Bar 16: Final cadence resolution
   8,  8,  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,  0,  0,  0,  0
};

// Root notes for the 16 bars (Dm, Dm, Dm, Dm, Dm, Dm, Dm, Dm, Dm, Dm, C, Dm, Dm, Dm, C, Dm)
const uint16_t bar_roots[16] PROGMEM = {
  N_D2, N_D2, N_D2, N_D2,
  N_D2, N_D2, N_D2, N_D2,
  N_D2, N_D2, N_C3, N_D2,
  N_D2, N_D2, N_C3, N_D2
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
volatile uint8_t  drum_type  = 0; // 0=None, 1=Kick, 2=Snare, 3=Shaker

uint16_t lfsr = 0xACE1; // 16-bit noise LFSR

// --- Sequencer State ---
volatile uint16_t ticks       = 0;
volatile uint16_t step_ticks  = 1250; // Default 125ms per 16th note (120 BPM)
volatile uint8_t  curr_step   = 0;
volatile uint8_t  curr_bar    = 0;
volatile uint8_t  turbo_roll  = 0; // Stutter / Drum Roll Flag

// Control Parameters
volatile uint8_t  voice_mode  = 0; // 0=Accordion, 1=Marimba, 2=8-Bit Chiptune, 3=Analog Saw
volatile uint8_t  visual_mode = 0; // 0=Copacabana Sunset, 1=Dancing Couple, 2=Carnival EQ, 3=Samba Fire
volatile uint8_t  sub_depth   = 200;
volatile uint8_t  lead_bright = 200;

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

  // Step Sequencer Counter
  static uint16_t tick_acc = 0;
  static uint8_t prev_note_idx = 0;

  tick_acc++;
  uint16_t target_ticks = turbo_roll ? (step_ticks >> 2) : step_ticks;
  if (target_ticks < 150) target_ticks = 150;

  if (tick_acc >= target_ticks) {
    tick_acc = 0;

    if (turbo_roll) {
      // Rapid Carnival Drum Stutter Roll!
      drum_type = 2; // Snare
      env_drum = 240;
      env_lead = 250;
      inc_lead = N_D5;
    } else {
      // Advance 256-step Sequencer
      curr_step = (curr_step + 1) & 255;
      curr_bar  = curr_step >> 4;
      uint8_t bar_step = curr_step & 15;

      // 1. Lead Melody Note
      uint8_t note_idx = pgm_read_byte(&lambada_melody[curr_step]);
      if (note_idx > 0) {
        inc_lead = pgm_read_word(&note_table[note_idx]);
        if (note_idx != prev_note_idx || bar_step == 0 || bar_step == 8) {
          env_lead = 250; // New attack
        }
      } else {
        inc_lead = 0;
      }
      prev_note_idx = note_idx;

      // 2. Carimbó / Lambada Syncopated Bass (hits on 0, 4, 6, 8, 12, 14)
      uint16_t root = pgm_read_word(&bar_roots[curr_bar]);
      if (bar_step == 0 || bar_step == 8) {
        // Deep root downbeat hit
        inc_bass = root;
        env_bass = 255;
      } else if (bar_step == 4 || bar_step == 12) {
        // Soft mid bounce
        inc_bass = root;
        env_bass = 180;
      } else if (bar_step == 6 || bar_step == 14) {
        // Syncopated octave pop!
        inc_bass = root << 1;
        env_bass = 230;
      }

      // 3. Latin Zouk / Lambada Drum Pattern
      if (bar_step == 0 || bar_step == 8) {
        // Kick on beats 1 and 3
        drum_type  = 1;
        drum_pitch = 950;
        env_drum   = 255;
      } else if (bar_step == 6 || bar_step == 14) {
        // Syncopated Snare / Rimshot!
        drum_type  = 2;
        env_drum   = 230;
      } else if (bar_step == 3 || bar_step == 11) {
        // Syncopated Shaker accent
        drum_type  = 3;
        env_drum   = 160;
      } else {
        // Background Shaker groove
        drum_type  = 3;
        env_drum   = 70;
      }
    }
  }

  // --- Voice Envelopes & LFSR Noise ---
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  // Envelopes decay
  if ((ticks & 7) == 0) {
    if (env_drum > 12) env_drum -= 12; else env_drum = 0;
    if (env_bass > 4)  env_bass -= 4;  else env_bass = 0;
  }
  if ((ticks & 15) == 0) {
    if (env_lead > 160) env_lead -= 2; // Warm sustained accordion decay
    else if (env_lead > 20) env_lead -= 1;
  }

  // --- Voice 1: Lead Melody Synthesis ---
  uint8_t s_lead = 0;
  if (inc_lead > 0 && env_lead > 0) {
    if (voice_mode == 0) {
      // Accordion (Dual Reeds with Vibrato / Chorus)
      int8_t vib = (ticks & 0x40) ? 9 : -9;
      phase_lead += (inc_lead + vib);
      uint8_t saw = phase_lead >> 8;
      uint8_t reed2 = ((phase_lead * 2) & 0x8000) ? 220 : 35;
      uint8_t acc = (saw + reed2) >> 1;
      s_lead = ((uint16_t)acc * env_lead) >> 8;
    } else if (voice_mode == 1) {
      // Tropical Marimba / Steel Pan (Triangle with bright pluck)
      phase_lead += inc_lead;
      uint8_t tri = phase_lead >> 8;
      if (tri > 127) tri = 255 - tri;
      s_lead = ((uint16_t)(tri << 1) * env_lead) >> 8;
    } else if (voice_mode == 2) {
      // 8-Bit Chiptune (25% Pulse Wave)
      phase_lead += inc_lead;
      uint8_t sq = ((phase_lead >> 8) < 64) ? 240 : 15;
      s_lead = ((uint16_t)sq * env_lead) >> 8;
    } else {
      // Analog Sizzle Saw
      phase_lead += inc_lead;
      uint8_t saw = phase_lead >> 8;
      s_lead = ((uint16_t)saw * env_lead) >> 8;
    }
    // Scale lead brightness
    s_lead = ((uint16_t)s_lead * lead_bright) >> 8;
  }

  // --- Voice 2: Carimbó Sub-Bass ---
  uint8_t s_bass = 0;
  if (inc_bass > 0 && env_bass > 0) {
    phase_bass += inc_bass;
    uint8_t b_tri = phase_bass >> 8;
    if (b_tri > 127) b_tri = 255 - b_tri;
    uint8_t raw_bass = b_tri << 1;
    s_bass = ((uint16_t)raw_bass * env_bass) >> 8;
    s_bass = ((uint16_t)s_bass * sub_depth) >> 8;
  }

  // --- Voice 3: Latin Drums & Shaker ---
  uint8_t s_drum = 0;
  if (env_drum > 0) {
    if (drum_type == 1) {
      // Kick: pitch-swept triangle
      phase_drum += (drum_pitch << 1);
      if (drum_pitch > 35) drum_pitch -= 2;
      uint8_t kw = phase_drum >> 8;
      if (kw > 127) kw = 255 - kw;
      s_drum = ((uint16_t)(kw << 1) * env_drum) >> 8;
    } else if (drum_type == 2) {
      // Snare: tonal pop + noise
      phase_drum += 400;
      uint8_t tone = (phase_drum & 0x8000) ? 180 : 70;
      uint8_t noise = lfsr & 0xFF;
      uint8_t sn = (tone + noise) >> 1;
      s_drum = ((uint16_t)sn * env_drum) >> 8;
    } else {
      // Shaker / Hi-Hat: pure crisp noise
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
  uint16_t p_right = analogRead(POTI_RIGHT); // Right Pot: Groove / Bass / Timbre

  // Map Left Pot to tempo: 800 (156 BPM carnival) to 1800 (69 BPM sensual slow)
  step_ticks = map(p_left, 0, 1023, 750, 1750);

  // Map Right Pot to Groove: Sub-bass punch & lead brightness
  sub_depth   = map(p_right, 0, 1023, 100, 255);
  lead_bright = map(p_right, 0, 1023, 120, 255);

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

  // Both buttons: Carnival Turbo Stutter Roll while held!
  if (curr_btn_state == 3) {
    turbo_roll = 1;
  } else {
    turbo_roll = 0;
  }

  // Edge detection for taps
  if (curr_btn_state != 0 && last_btn_state == 0) {
    if (curr_btn_state == 1) {
      // Left Button: Cycle Synth Voices
      voice_mode = (voice_mode + 1) % 4;
      // Visual feedback flash
      cli();
      uint32_t col = (voice_mode == 0) ? pixels.Color(255, 200, 0) :
                     (voice_mode == 1) ? pixels.Color(0, 255, 200) :
                     (voice_mode == 2) ? pixels.Color(0, 180, 255) :
                                         pixels.Color(255, 0, 150);
      matrixFill(col);
      pixels.show();
      sei();
      my_delay(150);
    } else if (curr_btn_state == 2) {
      // Right Button: Cycle Visualizer Modes
      visual_mode = (visual_mode + 1) % 4;
      // Visual feedback flash
      cli();
      matrixFill(pixels.Color(255, 255, 255));
      pixels.show();
      sei();
      my_delay(120);
    }
  }
  last_btn_state = curr_btn_state;

  // --- 3. Dynamic NeoPixel Visualizers (5x4 Matrix) ---
  cli(); // Briefly protect timing for WS2812B bit-bang

  if (turbo_roll) {
    // Turbo Mode: Brazilian Carnival strobe
    uint32_t turbo_col = ((frame & 1) == 0) ? pixels.Color(255, 240, 0) : pixels.Color(0, 220, 80);
    matrixFill(turbo_col);
  } else if (visual_mode == 0) {
    // Mode 0: Copacabana Sunset & Waves
    // Row 0: Golden Sun reacting to melody
    uint8_t sun_glow = 120 + (env_lead >> 1);
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      matrixSet(x, 0, pixels.Color(sun_glow, (sun_glow * 3) >> 2, 0));
    }
    // Rows 1-2: Tropical sky gradient with sunset hues
    uint8_t sky_step = (frame + curr_step) & 7;
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      uint8_t shift = (x + sky_step) % 5;
      matrixSet(x, 1, pixels.Color(200 - shift * 20, 20 + shift * 15, 80));
      matrixSet(x, 2, pixels.Color(120 - shift * 15, 10, 140 + shift * 10));
    }
    // Row 3: Ocean waves rolling across and leaping to the bass kick
    uint8_t wave_crest = (frame * 2 + (curr_step >> 1)) % MATRIX_W;
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      if (x == wave_crest && env_bass > 100) {
        matrixSet(x, 3, pixels.Color(180, 255, 255)); // Whitecap
      } else {
        matrixSet(x, 3, pixels.Color(0, 100 + (x * 25), 220)); // Aqua wave
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: The Dancing Lambada Couple (Swaying with the 3-3-2 rhythm)
    matrixFill(pixels.Color(20, 0, 30)); // Deep carnival night backdrop
    // Sway offset based on current step
    uint8_t sway = ((curr_step & 8) == 0) ? 1 : 2;
    uint32_t dancer_pink   = pixels.Color(255, 20, 120); // Dancer dress
    uint32_t partner_gold  = pixels.Color(255, 210, 0);  // Partner shirt
    uint32_t rhythm_accent = pixels.Color(0, 255, 220);  // Dancing feet
    
    // Heads
    matrixSet(sway, 0, dancer_pink);
    matrixSet(sway + 1, 0, partner_gold);
    // Torso / embrace
    matrixSet(sway, 1, dancer_pink);
    matrixSet(sway + 1, 1, partner_gold);
    // Swaying hips
    uint8_t hip_shift = ((curr_step & 4) == 0) ? sway - 1 : sway + 1;
    if (hip_shift < MATRIX_W) matrixSet(hip_shift, 2, dancer_pink);
    matrixSet(sway, 2, partner_gold);
    // Feet bouncing with kick & bass
    matrixSet(sway, 3, (env_bass > 120) ? rhythm_accent : pixels.Color(40, 10, 60));
    matrixSet(sway + 1, 3, (env_drum > 120) ? rhythm_accent : pixels.Color(40, 10, 60));
  } else if (visual_mode == 2) {
    // Mode 2: 5-Band Carnival Equalizer
    // Col 0: Kick Drum, Col 1: Bass, Col 2: Snare, Col 3: Lead Note, Col 4: Timbre
    uint8_t h_kick  = env_drum > 150 ? 4 : (env_drum > 70 ? 2 : 0);
    uint8_t h_bass  = env_bass > 180 ? 4 : (env_bass > 90 ? 2 : (env_bass > 30 ? 1 : 0));
    uint8_t h_snare = (drum_type == 2 && env_drum > 50) ? 4 : ((drum_type == 3 && env_drum > 50) ? 2 : 0);
    uint8_t h_lead  = env_lead > 180 ? 4 : (env_lead > 90 ? 2 : (env_lead > 30 ? 1 : 0));
    uint8_t h_tim   = (sub_depth >> 6) + 1;

    uint8_t heights[5] = { h_kick, h_bass, h_snare, h_lead, h_tim };
    uint32_t col_colors[5] = {
      pixels.Color(255, 30, 0),   // Kick: Fire Red
      pixels.Color(255, 180, 0),  // Bass: Warm Gold
      pixels.Color(0, 255, 100),  // Snare: Carnival Green
      pixels.Color(0, 180, 255),  // Lead: Tropical Blue
      pixels.Color(255, 0, 200)   // Timbre: Samba Magenta
    };

    for (uint8_t x = 0; x < MATRIX_W; x++) {
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t bar_y = 3 - y; // Bottom-up
        if (bar_y < heights[x]) {
          matrixSet(x, y, col_colors[x]);
        } else {
          matrixSet(x, y, pixels.Color(8, 8, 12));
        }
      }
    }
  } else {
    // Mode 3: Samba Fire & Rio Carnival Sparkles
    matrixFade(70); // Smooth phosphor trail
    // Fire sparks jumping on beats
    uint8_t spark_x = (frame * 3 + curr_step) % MATRIX_W;
    uint8_t spark_y = (frame + (curr_step >> 2)) % MATRIX_H;
    uint32_t spark_col = (frame & 1) ? pixels.Color(255, 230, 0) : pixels.Color(0, 255, 180);
    matrixSet(spark_x, spark_y, spark_col);

    if (env_bass > 150) {
      matrixSet(2, 3, pixels.Color(255, 50, 0)); // Center flame burst
      matrixSet(1, 3, pixels.Color(255, 140, 0));
      matrixSet(3, 3, pixels.Color(255, 140, 0));
    }
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  my_delay(40); // Smooth 25 FPS visual refresh
}
