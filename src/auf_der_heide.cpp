/* ---------------------------------------------------------------------------------------
    AUF DER HEIDE ("ERIKA") — VINTAGE GERMAN BRASS & CHIPTUNE MARSCH
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    A traditional 8-bit multi-voice arrangement of the classic 1938 German folk
    and marching tune "Auf der Heide blüht ein kleines Blümelein" (Herms Niel).
    Features the iconic brass lead, alternating Oom-Pah tuba bassline, and
    the famous 3 heavy stomps ("BUM! BUM! BUM!") with interactive controls.

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Marsch-Tempo: 75 - 140 BPM)
      PB2 (A1): Right Pot (Tuba Depth, Brass Cutoff & Stomp Punch)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle Voices (Oom-Pah Brass, Accordion, 8-Bit, Glockenspiel)
                - Right Button: Cycle Visualizers (Heidekraut, Alpine March, Brass EQ, Stomp Pulse)
                - Both Buttons: Trommelwirbel Snare Roll / Stomp Hold!
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
#define N_G2    642
#define N_B2    809
#define N_D3    962
#define N_G3    1285
#define N_A3    1442
#define N_B3    1618
#define N_C4    1715
#define N_D4    1925
#define N_E4    2160
#define N_Fs4   2425
#define N_G4    2569
#define N_A4    2884
#define N_B4    3237
#define N_C5    3429
#define N_D5    3849
#define N_E5    4320

// Lookup table for notes (14 distinct note values)
const uint16_t note_table[14] PROGMEM = {
  N_REST, // 0
  N_G3,   // 1
  N_A3,   // 2
  N_B3,   // 3
  N_C4,   // 4
  N_D4,   // 5
  N_E4,   // 6
  N_Fs4,  // 7
  N_G4,   // 8
  N_A4,   // 9
  N_B4,   // 10
  N_C5,   // 11
  N_D5,   // 12
  N_E5    // 13
};

// --- Complete 128-Step (8 Bar) "Auf der Heide" Melody ---
// Bars 1-4: Phrase 1 ("Auf der Heide blüht ein kleines Blümelein... BUM BUM BUM ... Erika!")
// Bars 5-8: Phrase 2 ("Heiß von hunderttausend kleinen Bienelein... BUM BUM BUM ... Erika!")
const uint8_t erika_melody[128] PROGMEM = {
  // Bar 1: Auf der Heide blüht ein kleines
   5,  5,  8,  8, 10, 10, 12, 12, 12, 12, 10, 10,  8,  8,  0,  0,
  // Bar 2: Blümelein ... [3 STOMPS on 6, 8, 10] ... und das
  11, 11,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  5,
  // Bar 3: heißt: ... [3 STOMPS on 7, 9, 11]
  12, 12, 11, 11,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // Bar 4: E - ri - ka!
   8,  8, 10, 10,  8,  8,  8,  8,  0,  0,  0,  0,  0,  0,  0,  0,

  // Bar 5: Heiß von hunderttausend kleinen
   5,  5,  8,  8, 10, 10, 12, 12, 12, 12, 10, 10,  8,  8,  0,  0,
  // Bar 6: Bienelein ... [3 STOMPS on 6, 8, 10] ... wird um-
  11, 11,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  5,
  // Bar 7: schwärmt: ... [3 STOMPS on 7, 9, 11]
  12, 12, 11, 11,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // Bar 8: E - ri - ka!
   8,  8, 10, 10,  8,  8,  8,  8,  0,  0,  0,  0,  0,  0,  0,  0
};

// Root chord notes for each of the 8 bars (G, D7, D7, G, G, D7, D7, G)
const uint16_t bar_roots[8] PROGMEM = {
  N_G2, N_D3, N_D3, N_G2,
  N_G2, N_D3, N_D3, N_G2
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
volatile uint8_t  drum_type  = 0; // 0=None, 1=Heavy Stomp, 2=Snare Crack, 3=March Hat

uint16_t lfsr = 0x9876; // 16-bit noise LFSR

// --- Sequencer State ---
volatile uint16_t ticks       = 0;
volatile uint16_t step_ticks  = 1300; // ~115 BPM marching tempo
volatile uint8_t  curr_step   = 0;
volatile uint8_t  curr_bar    = 0;
volatile uint8_t  is_stomp    = 0; // Flag indicating one of the 3 iconic stomps
volatile uint8_t  turbo_roll  = 0;

// Control Parameters
volatile uint8_t  voice_mode  = 0; // 0=Brass Horn, 1=Alpine Accordion, 2=8-Bit Marsch, 3=Glockenspiel
volatile uint8_t  visual_mode = 0; // 0=Heidekraut, 1=Alpine March, 2=Brass EQ, 3=Stomp Pulse
volatile uint8_t  sub_depth   = 220;
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
  static uint8_t prev_note_idx = 0;

  tick_acc++;
  uint16_t target_ticks = turbo_roll ? (step_ticks >> 2) : step_ticks;
  if (target_ticks < 140) target_ticks = 140;

  if (tick_acc >= target_ticks) {
    tick_acc = 0;

    if (turbo_roll) {
      // Rapid snare drum roll / Trommelwirbel
      drum_type = 2;
      env_drum = 240;
      env_lead = 250;
      inc_lead = N_G4;
      is_stomp = 1;
    } else {
      is_stomp = 0;

      // Advance 128-step sequencer
      curr_step = (curr_step + 1) & 127;
      curr_bar  = curr_step >> 4;
      uint8_t bar_step = curr_step & 15;

      // 1. Lead Melody Note
      uint8_t note_idx = pgm_read_byte(&erika_melody[curr_step]);
      if (note_idx > 0) {
        inc_lead = pgm_read_word(&note_table[note_idx]);
        if (note_idx != prev_note_idx || (bar_step & 1) == 0) {
          env_lead = 250; // New attack
        }
      } else {
        inc_lead = 0;
      }
      prev_note_idx = note_idx;

      // 2. Alternating Oom-Pah Tuba Bassline (Root on 0, 8; Fifth on 4, 12)
      uint16_t root = pgm_read_word(&bar_roots[curr_bar]);
      if (bar_step == 0 || bar_step == 8) {
        // "OOM" - Deep Root Tuba
        inc_bass = root;
        env_bass = 255;
      } else if (bar_step == 4 || bar_step == 12) {
        // "PAH" - Fifth harmonic bounce
        inc_bass = (root == N_G2) ? N_D3 : N_A3;
        env_bass = 210;
      }

      // 3. Marching Percussion & The Iconic 3 Stomps ("BUM! BUM! BUM!")
      bool stomp_now = false;
      if (curr_bar == 1 || curr_bar == 5) {
        // Bar 2 & 6: 3 Stomps on steps 6, 8, 10
        if (bar_step == 6 || bar_step == 8 || bar_step == 10) stomp_now = true;
      } else if (curr_bar == 2 || curr_bar == 6) {
        // Bar 3 & 7: 3 Stomps on steps 7, 9, 11
        if (bar_step == 7 || bar_step == 9 || bar_step == 11) stomp_now = true;
      }

      if (stomp_now) {
        // Massive Stomp (Heavy deep bass drum + snare crack)
        drum_type  = 1;
        drum_pitch = 1100;
        env_drum   = 255;
        is_stomp   = 1;
      } else if (bar_step == 0 || bar_step == 8) {
        // Normal March Downbeat
        drum_type  = 1;
        drum_pitch = 850;
        env_drum   = 200;
      } else if (bar_step == 4 || bar_step == 12) {
        // March Snare
        drum_type  = 2;
        env_drum   = 190;
      } else if ((bar_step & 1) == 0) {
        // Light march hat
        drum_type  = 3;
        env_drum   = 80;
      }
    }
  }

  // --- Voice Envelopes & Noise ---
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  // Envelopes decay
  if ((ticks & 7) == 0) {
    if (env_drum > 10) env_drum -= 10; else env_drum = 0;
    if (env_bass > 5)  env_bass -= 5;  else env_bass = 0;
  }
  if ((ticks & 15) == 0) {
    if (env_lead > 16) env_lead -= 16; else env_lead = 0;
  }

  // --- Voice 1: Brass & Melody Synthesis ---
  uint8_t s_lead = 0;
  if (inc_lead > 0 && env_lead > 0) {
    if (voice_mode == 0) {
      // Oom-Pah Brass Horn (Saw + Pulse mixture)
      phase_lead += inc_lead;
      uint8_t saw = phase_lead >> 8;
      uint8_t pulse = (phase_lead & 0x7000) ? 220 : 35;
      uint8_t brass = (saw + pulse) >> 1;
      s_lead = ((uint16_t)brass * env_lead) >> 8;
    } else if (voice_mode == 1) {
      // Hohner Alpine Accordion (Dual reed with slight vibrato)
      int8_t vib = (ticks & 0x40) ? 8 : -8;
      phase_lead += (inc_lead + vib);
      uint8_t saw = phase_lead >> 8;
      uint8_t reed2 = ((phase_lead * 2) & 0x8000) ? 220 : 35;
      uint8_t acc = (saw + reed2) >> 1;
      s_lead = ((uint16_t)acc * env_lead) >> 8;
    } else if (voice_mode == 2) {
      // 8-Bit Chiptune Marsch (50% Square Wave)
      phase_lead += inc_lead;
      uint8_t sq = (phase_lead & 0x8000) ? 240 : 15;
      s_lead = ((uint16_t)sq * env_lead) >> 8;
    } else {
      // Alpine Glockenspiel / Carillon (High Triangle chime)
      phase_lead += inc_lead;
      uint8_t tri = phase_lead >> 8;
      if (tri > 127) tri = 255 - tri;
      s_lead = ((uint16_t)(tri << 1) * env_lead) >> 8;
    }
    s_lead = ((uint16_t)s_lead * lead_bright) >> 8;
  }

  // --- Voice 2: Tuba Oom-Pah Bass ---
  uint8_t s_bass = 0;
  if (inc_bass > 0 && env_bass > 0) {
    phase_bass += inc_bass;
    uint8_t b_tri = phase_bass >> 8;
    if (b_tri > 127) b_tri = 255 - b_tri;
    uint8_t raw_tuba = b_tri << 1;
    s_bass = ((uint16_t)raw_tuba * env_bass) >> 8;
    s_bass = ((uint16_t)s_bass * sub_depth) >> 8;
  }

  // --- Voice 3: Stomp / Drum Percussion ---
  uint8_t s_drum = 0;
  if (env_drum > 0) {
    if (drum_type == 1) {
      // Stomp: massive pitch drop + deep resonance
      phase_drum += (drum_pitch << 1);
      if (drum_pitch > 35) drum_pitch -= 2;
      uint8_t kw = phase_drum >> 8;
      if (kw > 127) kw = 255 - kw;
      // Add slight noise crunch to stomp
      uint8_t stomp_sound = (kw + ((lfsr & 0x3F) >> 1));
      s_drum = ((uint16_t)stomp_sound * env_drum) >> 8;
    } else if (drum_type == 2) {
      // Snare: crack + noise
      phase_drum += 400;
      uint8_t tone = (phase_drum & 0x8000) ? 190 : 60;
      uint8_t noise = lfsr & 0xFF;
      uint8_t sn = (tone + noise) >> 1;
      s_drum = ((uint16_t)sn * env_drum) >> 8;
    } else {
      // Marching Hat
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
  uint16_t p_left  = analogRead(POTI_LEFT);  // Left Pot: Marsch-Tempo (BPM)
  uint16_t p_right = analogRead(POTI_RIGHT); // Right Pot: Tuba & Stomp Depth

  // Map Left Pot to tempo: 850 (140 BPM brisk march) to 1800 (68 BPM slow stroll)
  step_ticks = map(p_left, 0, 1023, 850, 1800);

  // Map Right Pot to Tuba sub-bass & brass brightness
  sub_depth   = map(p_right, 0, 1023, 110, 255);
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

  // Both buttons: Snare roll / Trommelwirbel while held!
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
      uint32_t col = (voice_mode == 0) ? pixels.Color(255, 180, 0) :  // Brass Gold
                     (voice_mode == 1) ? pixels.Color(0, 240, 180) :  // Accordion Teal
                     (voice_mode == 2) ? pixels.Color(255, 60, 0) :   // Chiptune Orange
                                         pixels.Color(200, 230, 255); // Glockenspiel Silver
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
  cli(); // Protect timing for WS2812B bit-bang

  if (is_stomp || turbo_roll) {
    // Stomp Pulse: Golden explosion on the 3 stomps!
    matrixFill(pixels.Color(255, 210, 0));
  } else if (visual_mode == 0) {
    // Mode 0: Blühendes Heidekraut (Blooming Heather / Erika Flowers)
    // Row 0: Alpine Blue Sky
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      matrixSet(x, 0, pixels.Color(20, 80, 220));
    }
    // Rows 1-3: Heather Flowers (Erika Violet & Green Heath)
    uint32_t erika_violet = pixels.Color(200, 30, 160);
    uint32_t heath_green  = pixels.Color(20, 140, 30);
    uint32_t blossom_gold = pixels.Color(255, 230, 80);

    for (uint8_t y = 1; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_flower = ((x + y + (curr_step >> 2)) % 3 == 0);
        if (is_flower) {
          matrixSet(x, y, (env_lead > 160) ? blossom_gold : erika_violet);
        } else {
          matrixSet(x, y, heath_green);
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Alpine March / Bergwanderer (Mountain Silhouette & Clouds)
    // Mountain peaks moving across horizon
    uint8_t offset = (frame + curr_step) & 7;
    uint32_t sky_col   = pixels.Color(10, 40, 160);
    uint32_t peak_snow = pixels.Color(240, 240, 255);
    uint32_t mountain  = pixels.Color(60, 80, 100);
    uint32_t valley    = (env_bass > 100) ? pixels.Color(40, 160, 40) : pixels.Color(20, 80, 20);

    for (uint8_t x = 0; x < MATRIX_W; x++) {
      matrixSet(x, 0, sky_col);
      bool is_peak = ((x + offset) % 3 == 0);
      matrixSet(x, 1, is_peak ? peak_snow : mountain);
      matrixSet(x, 2, mountain);
      matrixSet(x, 3, valley);
    }
  } else if (visual_mode == 2) {
    // Mode 2: Vintage Brass Oom-Pah Equalizer
    // Col 0: Tuba Root, Col 1: Tuba Fifth, Col 2: Snare, Col 3: Brass Lead, Col 4: Timbre
    uint8_t h_tuba1 = (env_bass > 180) ? 4 : ((env_bass > 80) ? 2 : 0);
    uint8_t h_tuba2 = (env_bass > 140) ? 3 : 0;
    uint8_t h_snare = (drum_type == 2 && env_drum > 60) ? 4 : 0;
    uint8_t h_lead  = (env_lead > 160) ? 4 : ((env_lead > 80) ? 2 : ((env_lead > 20) ? 1 : 0));
    uint8_t h_tim   = (sub_depth >> 6) + 1;

    uint8_t heights[5] = { h_tuba1, h_tuba2, h_snare, h_lead, h_tim };
    uint32_t col_colors[5] = {
      pixels.Color(255, 140, 0),  // Deep Tuba Gold
      pixels.Color(255, 200, 0),  // Tuba Fifth
      pixels.Color(255, 40, 20),   // Snare Red
      pixels.Color(255, 230, 80),  // Brass Horn
      pixels.Color(0, 220, 180)    // Timbre
    };

    for (uint8_t x = 0; x < MATRIX_W; x++) {
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t bar_y = 3 - y;
        if (bar_y < heights[x]) {
          matrixSet(x, y, col_colors[x]);
        } else {
          matrixSet(x, y, pixels.Color(6, 6, 10));
        }
      }
    }
  } else {
    // Mode 3: The 3 Stomps Drum Visualizer (Concentric Stomp Rings)
    matrixFade(75);
    uint8_t cx = 2;
    uint8_t cy = 1;
    // Pulse on beats
    uint32_t ring_col = (env_drum > 160) ? pixels.Color(255, 220, 0) : pixels.Color(200, 30, 80);
    matrixSet(cx, cy, ring_col);
    if (env_drum > 100) {
      matrixSet(cx - 1, cy, ring_col);
      matrixSet(cx + 1, cy, ring_col);
      matrixSet(cx, cy + 1, ring_col);
    }
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  my_delay(40); // 25 FPS visual update
}
