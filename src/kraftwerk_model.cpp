/* ---------------------------------------------------------------------------------------
    KRAFTWERK — THE MODEL ("DAS MODEL")
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic 8-bit multi-voice synthesizer tribute to Kraftwerk's 1978 electro
    pop masterpiece "Das Model" (Die Mensch-Maschine). Features the iconic Minimoog
    lead melody, driving 16th-note analog bass sequence, electronic drum machine,
    and Kling Klang studio oscilloscope & mannequin visuals.

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Kling Klang Clock Tempo: 90 - 145 BPM)
      PB2 (A1): Right Pot (Minimoog VCF Filter Cutoff, Resonance & Sub-Bass)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Synth Voices (Minimoog Lead, Computerwelt Bleeps, Vocoder, Saw)
                - Right Button: Cycle Visualizers (Mensch-Maschine, Oscilloscope, Die Roboter, Radioaktivität)
                - Both Buttons: Kling Klang Computer Glitch & Strobe Break!
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
#define N_A2    721
#define N_B2    809
#define N_C3    857
#define N_D3    962
#define N_E3    1080
#define N_F3    1144
#define N_G3    1285
#define N_Gs3   1361
#define N_A3    1442
#define N_B3    1618
#define N_C4    1715
#define N_D4    1925
#define N_E4    2160
#define N_F4    2289
#define N_G4    2569
#define N_Gs4   2722
#define N_A4    2884
#define N_B4    3237
#define N_C5    3429
#define N_D5    3849
#define N_E5    4320
#define N_F5    4577
#define N_A5    5767

// Lead melody note lookup table (8 notes)
const uint16_t note_table[8] PROGMEM = {
  N_REST, // 0
  N_Gs4,  // 1
  N_A4,   // 2
  N_B4,   // 3
  N_C5,   // 4
  N_D5,   // 5
  N_E5,   // 6
  N_F5    // 7
};

// --- Complete 256-Step (16 Bar) "Das Model" Arrangement ---
// Bars 1-8:   The iconic Main Synth Hook & Variation
// Bars 9-12:  The Vocal Theme ("Sie ist ein Model und sie sieht gut aus...")
// Bars 13-16: Main Hook Climax and Cadence
const uint8_t kraftwerk_model_lead[256] PROGMEM = {
  // Bar 1: E5 C5 A4 E5
   6,  6,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  6,  6,  6,  6,
  // Bar 2: F5 D5 A4 F5
   7,  7,  7,  7,  5,  5,  5,  5,  2,  2,  2,  2,  7,  7,  7,  7,
  // Bar 3: E5 B4 G#4 E5
   6,  6,  6,  6,  3,  3,  3,  3,  1,  1,  1,  1,  6,  6,  6,  6,
  // Bar 4: E5 C5 A4 hold
   6,  6,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  2,  2,  2,  2,
  // Bar 5: E5 C5 A4 E5
   6,  6,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  6,  6,  6,  6,
  // Bar 6: F5 D5 A4 F5
   7,  7,  7,  7,  5,  5,  5,  5,  2,  2,  2,  2,  7,  7,  7,  7,
  // Bar 7: E5 B4 G#4 B4
   6,  6,  6,  6,  3,  3,  3,  3,  1,  1,  1,  1,  3,  3,  3,  3,
  // Bar 8: A4 hold ... turnaround
   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,
  // Bar 9: "Sie ist ein Model und sie sieht gut aus"
   2,  2,  2,  4,  4,  4,  3,  2,  1,  1,  1,  3,  3,  3,  2,  0,
  // Bar 10: "Ich nehm sie heute gerne mit nach Haus"
   2,  2,  2,  4,  4,  4,  3,  2,  1,  1,  1,  3,  3,  3,  2,  0,
  // Bar 11: "Sie trinkt im Nachtclub taglich Champagner"
   4,  4,  4,  6,  6,  6,  5,  4,  3,  3,  3,  5,  5,  5,  4,  0,
  // Bar 12: "Und hat hier alle Manner schon geschafft"
   4,  4,  4,  6,  6,  6,  5,  4,  3,  3,  1,  1,  2,  2,  2,  0,
  // Bar 13: Hook Climax E5 C5 A4 E5
   6,  6,  6,  6,  4,  4,  4,  4,  2,  2,  2,  2,  6,  6,  6,  6,
  // Bar 14: F5 D5 A4 F5
   7,  7,  7,  7,  5,  5,  5,  5,  2,  2,  2,  2,  7,  7,  7,  7,
  // Bar 15: E5 B4 G#4 B4
   6,  6,  6,  6,  3,  3,  3,  3,  1,  1,  1,  1,  3,  3,  3,  3,
  // Bar 16: Final resolution
   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0
};

// Root notes for the 16 bars (Am, Dm, E, Am...)
const uint16_t bar_roots[16] PROGMEM = {
  N_A2, N_D3, N_E3, N_A2,
  N_A2, N_D3, N_E3, N_A2,
  N_A2, N_A2, N_C3, N_E3,
  N_A2, N_D3, N_E3, N_A2
};

// Fast Arpeggiator Chords for Voice 2 (Am, Dm, E, Am)
const uint16_t arp_triads[4][3] PROGMEM = {
  { N_A3, N_C4,  N_E4  }, // Am
  { N_D3, N_F3,  N_A3  }, // Dm
  { N_E3, N_Gs3, N_B3  }, // E
  { N_A3, N_C4,  N_E4  }  // Am
};

// --- Synth Engine State ---
volatile uint16_t phase_lead = 0;
volatile uint16_t inc_lead   = 0;
volatile uint8_t  env_lead   = 0;

volatile uint16_t phase_arp  = 0;
volatile uint16_t inc_arp   = 0;
volatile uint8_t  env_arp    = 0;
volatile uint8_t  arp_tick   = 0;

volatile uint16_t phase_bass = 0;
volatile uint16_t inc_bass   = 0;
volatile uint8_t  env_bass   = 0;

volatile uint16_t phase_drum = 0;
volatile uint16_t drum_pitch = 0;
volatile uint8_t  env_drum   = 0;
volatile uint8_t  drum_type  = 0; // 0=None, 1=Electronic Kick, 2=Electro Clap/Snare, 3=Electronic Hat

uint16_t lfsr = 0x8351; // 16-bit noise generator

// --- Sequencer State ---
volatile uint16_t ticks       = 0;
volatile uint16_t step_ticks  = 1210; // ~124 BPM
volatile uint8_t  curr_step   = 0;
volatile uint8_t  curr_bar    = 0;
volatile uint8_t  turbo_roll  = 0;

// Controls
volatile uint8_t  voice_mode  = 0; // 0=Minimoog Lead, 1=Computer Bleeps, 2=Vocoder Pulse, 3=Saw
volatile uint8_t  visual_mode = 0; // 0=Mensch-Maschine, 1=Oscilloscope, 2=Roboter, 3=Radioaktivität
volatile uint8_t  moog_cutoff = 210;
volatile uint8_t  sub_depth   = 220;

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
  static uint8_t prev_lead_idx = 0;

  // 16th-Note Sequencer Clock
  tick_acc++;
  uint16_t target_ticks = turbo_roll ? (step_ticks >> 2) : step_ticks;
  if (target_ticks < 120) target_ticks = 120;

  if (tick_acc >= target_ticks) {
    tick_acc = 0;

    if (turbo_roll) {
      // Kraftwerk Tape Glitch Stutter
      drum_type = 2;
      env_drum = 240;
      env_lead = 250;
      inc_lead = N_A4;
    } else {
      curr_step = (curr_step + 1) & 255;
      curr_bar  = curr_step >> 4;
      uint8_t bar_step = curr_step & 15;
      uint8_t chord_idx = curr_bar & 3;

      // 1. Lead Melody Note
      uint8_t note_idx = pgm_read_byte(&kraftwerk_model_lead[curr_step]);
      if (note_idx > 0) {
        inc_lead = pgm_read_word(&note_table[note_idx]);
        if (note_idx != prev_lead_idx || (bar_step & 3) == 0) {
          env_lead = 255; // Precise electronic key trigger
        }
      } else {
        inc_lead = 0;
      }
      prev_lead_idx = note_idx;

      // 2. 16th-note Kling Klang Arpeggiator Clock
      arp_tick = (arp_tick + 1) % 3;
      inc_arp = pgm_read_word(&arp_triads[chord_idx][arp_tick]);
      env_arp = 150;

      // 3. Driving 16th-note Kraftwerk Bassline
      uint16_t root = pgm_read_word(&bar_roots[curr_bar]);
      if (bar_step == 0 || bar_step == 3 || bar_step == 6 || bar_step == 8 ||
          bar_step == 11 || bar_step == 14) {
        inc_bass = root;
        env_bass = 250;
      }

      // 4. Electronic Drum Machine (Bohm / Simmons style)
      if (bar_step == 0 || bar_step == 8) {
        // Electronic Kick
        drum_type  = 1;
        drum_pitch = 950;
        env_drum   = 250;
      } else if (bar_step == 4 || bar_step == 12) {
        // Electronic Snare / Clap
        drum_type  = 2;
        env_drum   = 230;
      } else if ((bar_step & 1) == 0) {
        // Electro Hat
        drum_type  = 3;
        env_drum   = 90;
      } else {
        drum_type  = 3;
        env_drum   = 40;
      }
    }
  }

  // --- Voice Envelopes & Noise ---
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  // Precise electronic decays
  if ((ticks & 7) == 0) {
    if (env_drum > 14) env_drum -= 14; else env_drum = 0;
    if (env_bass > 10) env_bass -= 10; else env_bass = 0;
    if (env_arp > 12)  env_arp -= 12;  else env_arp = 0;
  }
  if ((ticks & 15) == 0) {
    if (env_lead > 16) env_lead -= 16; else env_lead = 0;
  }

  // --- Voice 1: Lead Synthesis ---
  uint8_t s_lead = 0;
  if (inc_lead > 0 && env_lead > 0) {
    phase_lead += inc_lead;
    uint8_t p = phase_lead >> 8;

    if (voice_mode == 0) {
      // Minimoog Lead (Warm analog pulse with punchy filter decay)
      uint8_t pulse = (p < 120) ? 230 : 25;
      s_lead = ((uint16_t)pulse * env_lead) >> 8;
    } else if (voice_mode == 1) {
      // Computerwelt Bleeps (Pure electronic triangle)
      uint8_t tri = p;
      if (tri > 127) tri = 255 - tri;
      s_lead = ((uint16_t)(tri << 1) * env_lead) >> 8;
    } else if (voice_mode == 2) {
      // Robot Vocoder Pulse (Narrow 20% pulse wave)
      uint8_t voc = (p < 50) ? 245 : 10;
      s_lead = ((uint16_t)voc * env_lead) >> 8;
    } else {
      // Sawtooth
      s_lead = ((uint16_t)p * env_lead) >> 8;
    }
    s_lead = ((uint16_t)s_lead * moog_cutoff) >> 8;
  }

  // --- Voice 2: Arpeggiator ---
  uint8_t s_arp = 0;
  if (inc_arp > 0 && env_arp > 0) {
    phase_arp += inc_arp;
    uint8_t ap = (phase_arp >> 8) < 64 ? 220 : 35;
    s_arp = ((uint16_t)ap * env_arp) >> 8;
  }

  // --- Voice 3: Bass & Electronic Drums ---
  uint8_t s_bass = 0;
  if (inc_bass > 0 && env_bass > 0) {
    phase_bass += inc_bass;
    uint8_t btri = phase_bass >> 8;
    if (btri > 127) btri = 255 - btri;
    s_bass = ((uint16_t)(btri << 1) * env_bass) >> 8;
    s_bass = ((uint16_t)s_bass * sub_depth) >> 8;
  }

  uint8_t s_drum = 0;
  if (env_drum > 0) {
    if (drum_type == 1) {
      // Kick: pitch drop
      phase_drum += (drum_pitch << 1);
      if (drum_pitch > 35) drum_pitch -= 3;
      uint8_t kw = phase_drum >> 8;
      if (kw > 127) kw = 255 - kw;
      s_drum = ((uint16_t)(kw << 1) * env_drum) >> 8;
    } else if (drum_type == 2) {
      // Electronic Clap / Snare: noise burst
      uint8_t noise = lfsr & 0xFF;
      s_drum = ((uint16_t)noise * env_drum) >> 8;
    } else {
      // Hat: short noise
      uint8_t noise = lfsr & 0xFF;
      s_drum = ((uint16_t)noise * env_drum) >> 8;
    }
  }

  // 3-Voice Mix to 8-bit PWM (PB1)
  uint16_t mix = (uint16_t)s_bass + ((uint16_t)s_lead << 1) + (uint16_t)s_arp + (uint16_t)s_drum;
  uint8_t out = mix >> 2;
  OCR1A = out;
}

// 5x4 Mannequin Silhouette Bitmap (Kraftwerk Red Shirt / Black Tie)
const uint8_t mannequin_bmp[20] PROGMEM = {
  0, 1, 1, 1, 0, // Head
  1, 1, 0, 1, 1, // Red shirt with black tie center
  1, 1, 0, 1, 1, // Torso
  0, 1, 0, 1, 0  // Legs
};

// 5x4 Radioactive Hazard Symbol Bitmap
const uint8_t radioaktiv_bmp[20] PROGMEM = {
  1, 0, 1, 0, 1,
  0, 1, 1, 1, 0,
  0, 1, 1, 1, 0,
  1, 0, 1, 0, 1
};

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
  uint16_t p_left  = analogRead(POTI_LEFT);  // Left Pot: Sequencer Tempo (BPM)
  uint16_t p_right = analogRead(POTI_RIGHT); // Right Pot: Moog VCF Filter & Bass

  // Map Left Pot to tempo: 850 (145 BPM) to 1800 (68 BPM)
  step_ticks = map(p_left, 0, 1023, 850, 1800);

  // Map Right Pot to Moog filter cutoff and sub-bass depth
  moog_cutoff = map(p_right, 0, 1023, 110, 255);
  sub_depth   = map(p_right, 0, 1023, 110, 255);

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

  // Both buttons: Computer Glitch Stutter while held!
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
      cli();
      uint32_t col = (voice_mode == 0) ? pixels.Color(255, 30, 20) :  // Moog Red
                     (voice_mode == 1) ? pixels.Color(0, 255, 120) :  // Computerwelt Green
                     (voice_mode == 2) ? pixels.Color(0, 180, 255) :  // Vocoder Cyan
                                         pixels.Color(255, 220, 0);   // Saw Yellow
      matrixFill(col);
      pixels.show();
      sei();
      my_delay(150);
    } else if (curr_btn_state == 2) {
      // Right Button: Cycle Visualizers
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
  cli(); // Protect timing during WS2812B bit-bang

  if (turbo_roll) {
    // Glitch Mode: Strobe white/red
    uint32_t flash = (frame & 1) ? pixels.Color(255, 0, 0) : pixels.Color(255, 255, 255);
    matrixFill(flash);
  } else if (visual_mode == 0) {
    // Mode 0: Die Mensch-Maschine (Kraftwerk Red Shirt & Black Tie)
    uint8_t red_level = 160 + (env_lead >> 2);
    uint32_t red_shirt = pixels.Color(red_level, 0, 0);
    uint32_t black_tie = pixels.Color(10, 10, 15);
    uint32_t head_col  = pixels.Color(240, 200, 170); // Mannequin head

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_on = pgm_read_byte(&mannequin_bmp[y * MATRIX_W + x]);
        if (y == 0 && is_on) {
          matrixSet(x, y, head_col);
        } else if (is_on) {
          matrixSet(x, y, (x == 2) ? black_tie : red_shirt);
        } else {
          matrixSet(x, y, black_tie);
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Kling Klang Oscilloscope (Green Phosphor CRT Wave)
    matrixFade(80); // Phosphor persistence decay
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      // Calculate sine wave offset modulated by lead pitch & frame
      int8_t wave_y = 1 + (((int8_t)((frame * 2 + x * 3 + (curr_step >> 1)) & 7) - 4) >> 1);
      if (wave_y < 0) wave_y = 0;
      if (wave_y >= MATRIX_H) wave_y = MATRIX_H - 1;

      uint32_t green_crt = (env_lead > 160) ? pixels.Color(120, 255, 140) : pixels.Color(0, 220, 40);
      matrixSet(x, wave_y, green_crt);
    }
  } else if (visual_mode == 2) {
    // Mode 2: Die Roboter (Rigid Machine Steps)
    uint8_t step_x = (curr_step >> 2) % MATRIX_W;
    matrixFill(pixels.Color(15, 15, 20));

    uint32_t robot_cyan = pixels.Color(0, 220, 255);
    uint32_t robot_gold = pixels.Color(255, 215, 0);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      matrixSet(step_x, y, (y % 2 == 0) ? robot_cyan : robot_gold);
    }
    if (drum_type == 1 && env_drum > 100) {
      matrixSet(0, 3, pixels.Color(255, 40, 0));
      matrixSet(4, 3, pixels.Color(255, 40, 0));
    }
  } else {
    // Mode 3: Radioaktivität (Radioactive Hazard & Geiger Sparks)
    uint32_t hazard_gold = pixels.Color(240, 200, 0);
    uint32_t dark_bg     = pixels.Color(15, 15, 15);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_haz = pgm_read_byte(&radioaktiv_bmp[y * MATRIX_W + x]);
        matrixSet(x, y, is_haz ? hazard_gold : dark_bg);
      }
    }
    // Geiger clicks flashing center
    if (env_drum > 160) {
      matrixSet(2, 1, pixels.Color(255, 255, 255));
      matrixSet(2, 2, pixels.Color(255, 255, 255));
    }
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  my_delay(40); // 25 FPS visual update
}
