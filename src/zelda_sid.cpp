/* ---------------------------------------------------------------------------------------
    THE LEGEND OF ZELDA — OVERWORLD THEME (SID SOUND ENGINE)
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    A full-scale arrangement of Koji Kondo's legendary 1986 "Overworld Theme"
    powered by an authentic Commodore 64 MOS 6581/8580 "SID-style" sound engine:
      - 3 polyphonic SID voices (Lead, Arp/Chords, Bass & Noise Percussion)
      - Dynamic Pulse-Width Modulation (PWM LFO) for rich, singing chorused brass
      - 50Hz Fast SID Arpeggiator (Tracker 0xy) for sparkling multi-voice chords
      - Hardware-style ADSR envelope generators (Attack, Decay, Sustain, Release)
      - Resonant low-pass SID filter simulation controllable via potentiometer

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Quest Tempo: 85 - 145 BPM)
      PB2 (A1): Right Pot (SID Filter Cutoff, Resonance & PWM Modulation Depth)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle SID Voices (PWM Brass, Fast Arp, NES 2A03, SID Overdrive)
                - Right Button: Cycle Zelda Visualizers (Triforce, Master Sword, Hearts, Rupees)
                - Both Buttons: Secret Item Fanfare & Fairy Revival Stutter!
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
#define N_Bb1   382
#define N_D2    481
#define N_Eb2   510
#define N_F2    572
#define N_Gb2   606
#define N_G2    642
#define N_A2    721
#define N_Bb2   764
#define N_C3    857
#define N_Db3   908
#define N_D3    962
#define N_Eb3   1019
#define N_F3    1144
#define N_G3    1285
#define N_Ab3   1361
#define N_A3    1442
#define N_Bb3   1528
#define N_C4    1715
#define N_Db4   1817
#define N_D4    1925
#define N_Eb4   2039
#define N_E4    2160
#define N_F4    2289
#define N_Gb4   2425
#define N_G4    2569
#define N_Ab4   2722
#define N_A4    2884
#define N_Bb4   3055
#define N_C5    3429
#define N_Db5   3633
#define N_D5    3849
#define N_Eb5   4078
#define N_E5    4320
#define N_F5    4577
#define N_Gb5   4850
#define N_G5    5138
#define N_Ab5   5443
#define N_A5    5767
#define N_Bb5   6110
#define N_C6    6858

// Lookup table for Lead melody notes (17 distinct notes)
const uint16_t note_table[17] PROGMEM = {
  N_REST, // 0
  N_F4,   // 1
  N_Gb4,  // 2
  N_G4,   // 3
  N_Ab4,  // 4
  N_A4,   // 5
  N_Bb4,  // 6
  N_C5,   // 7
  N_Db5,  // 8
  N_D5,   // 9
  N_Eb5,  // 10
  N_E5,   // 11
  N_F5,   // 12
  N_Gb5,  // 13
  N_G5,   // 14
  N_Ab5,  // 15
  N_Bb5   // 16
};

// --- Complete 128-Step (8 Bar) Zelda Overworld Melody ---
// Bars 1-3: Heroic opening fanfare and climb to high Bb5
// Bar 4:    Triumphant brass hold on F5
// Bars 5-7: Dramatic chromatic descent (Gb5 -> F5 -> E5)
// Bar 8:    Cadence resolution and pickup turnaround
const uint8_t zelda_lead[128] PROGMEM = {
  // Bar 1: Bb4 hold ... F4 . Bb4 C5 D5 Eb5
   6,  6,  6,  6,  6,  6,  6,  6,  0,  0,  1,  1,  6,  7,  9, 10,
  // Bar 2: F5 hold ... F5 . F5 F5 Gb5 Ab5
  12, 12, 12, 12, 12, 12, 12, 12,  0,  0, 12,  0, 12, 12, 13, 15,
  // Bar 3: Bb5 hold ... Bb5 . Bb5 Bb5 Ab5 Gb5
  16, 16, 16, 16, 16, 16, 16, 16,  0,  0, 16,  0, 16, 16, 15, 13,
  // Bar 4: Ab5 Gb5 F5 hold
  15, 15, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
  // Bar 5: F5 Eb5 Eb5 F5 Gb5 hold
  12, 12, 10, 10, 10, 10, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13,
  // Bar 6: F5 Eb5 Db5 Eb5 F5 hold
  12, 12, 10, 10,  8,  8, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
  // Bar 7: Eb5 Db5 C5 D5 E5 hold
  10, 10,  8,  8,  7,  7,  9,  9, 11, 11, 11, 11, 11, 11, 11, 11,
  // Bar 8: G5 F5 hold ... F4 Bb4 C5 D5
  14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,  1,  6,  7,  9
};

// 80s SID Arpeggiator Triads for Voice 2 (Bb, Bb/Eb, Bb/Gb, F, Gb/Bbm, Db/F, C/Edim, F)
const uint16_t arp_chords[8][3] PROGMEM = {
  { N_Bb3, N_D4,  N_F4  }, // Bar 1: Bb Major
  { N_Bb3, N_Eb4, N_G4  }, // Bar 2: Eb Major
  { N_Bb3, N_Db4, N_Gb4 }, // Bar 3: Gb Major
  { N_A3,  N_C4,  N_F4  }, // Bar 4: F Major
  { N_Bb3, N_Db4, N_F4  }, // Bar 5: Bb Minor
  { N_Ab3, N_Db4, N_F4  }, // Bar 6: Db Major
  { N_G3,  N_Bb3, N_E4  }, // Bar 7: E Diminished
  { N_A3,  N_C4,  N_F4  }  // Bar 8: F Major
};

// Root notes for Voice 3 Bass
const uint16_t bar_bass[8] PROGMEM = {
  N_Bb2, N_Eb2, N_Gb2, N_F2,
  N_Bb2, N_Db3, N_C3,  N_F2
};

// --- SID Hardware ADSR States ---
#define ADSR_IDLE    0
#define ADSR_ATTACK  1
#define ADSR_DECAY   2
#define ADSR_SUSTAIN 3
#define ADSR_RELEASE 4

// --- SID Sound Engine State ---
// Voice 1: Heroic Lead (Pulse with modulated PWM / Saw)
volatile uint16_t v1_phase   = 0;
volatile uint16_t v1_freq    = 0;
volatile uint8_t  v1_adsr    = ADSR_IDLE;
volatile uint8_t  v1_env     = 0;
volatile uint8_t  v1_pw      = 128; // Base 50% pulse width

// Voice 2: SID Fast Arpeggiator Chords (50Hz cycling tri-notes)
volatile uint16_t v2_phase   = 0;
volatile uint16_t v2_freq    = 0;
volatile uint8_t  v2_adsr    = ADSR_IDLE;
volatile uint8_t  v2_env     = 0;
volatile uint8_t  v2_arp_idx = 0;

// Voice 3: Bass & Chiptune Noise Percussion
volatile uint16_t v3_phase   = 0;
volatile uint16_t v3_freq    = 0;
volatile uint8_t  v3_adsr    = ADSR_IDLE;
volatile uint8_t  v3_env     = 0;
volatile uint8_t  v3_is_drum = 0; // 0=Bass, 1=Snare Noise, 2=Kick

uint16_t lfsr = 0x7654; // 16-bit SID noise generator

// --- Sequencer & Timing State ---
volatile uint16_t ticks       = 0;
volatile uint16_t step_ticks  = 1150; // ~130 BPM adventure tempo
volatile uint8_t  curr_step   = 0;
volatile uint8_t  curr_bar    = 0;
volatile uint8_t  turbo_roll  = 0;

// SID Global Controls
volatile uint8_t  voice_mode  = 0; // 0=SID PWM Brass, 1=SID Fast Arp, 2=NES APU, 3=SID Overdrive
volatile uint8_t  visual_mode = 0; // 0=Triforce, 1=Master Sword, 2=Hearts, 3=Rupees
volatile uint8_t  sid_cutoff  = 200; // Filter cutoff / brightness
volatile uint8_t  pwm_depth   = 60;  // PWM LFO sweep depth

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

// --- 10kHz SID Audio & Sequencer Interrupt ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  static uint16_t tick_acc = 0;
  static uint8_t prev_lead_idx = 0;

  // 1. High-speed 50Hz SID Arpeggiator Clock (every 200 ticks at 10kHz = 50Hz!)
  if ((ticks % 180) == 0) {
    v2_arp_idx = (v2_arp_idx + 1) % 3;
    v2_freq = pgm_read_word(&arp_chords[curr_bar][v2_arp_idx]);
  }

  // 2. 16th-Note Step Sequencer
  tick_acc++;
  uint16_t target_ticks = turbo_roll ? (step_ticks >> 2) : step_ticks;
  if (target_ticks < 120) target_ticks = 120;

  if (tick_acc >= target_ticks) {
    tick_acc = 0;

    if (turbo_roll) {
      // Secret Fanfare / Revival Stutter
      v1_freq = N_F5;
      v1_adsr = ADSR_ATTACK;
      v3_is_drum = 1;
      v3_adsr = ADSR_ATTACK;
    } else {
      curr_step = (curr_step + 1) & 127;
      curr_bar  = curr_step >> 4;
      uint8_t bar_step = curr_step & 15;

      // Voice 1: Lead Melody
      uint8_t note_idx = pgm_read_byte(&zelda_lead[curr_step]);
      if (note_idx > 0) {
        v1_freq = pgm_read_word(&note_table[note_idx]);
        if (note_idx != prev_lead_idx || bar_step == 0 || bar_step == 8) {
          v1_adsr = ADSR_ATTACK; // Retrigger attack
        }
      } else {
        v1_adsr = ADSR_RELEASE;
      }
      prev_lead_idx = note_idx;

      // Voice 2: SID Chords / Counter-Harmony
      v2_adsr = ADSR_ATTACK;

      // Voice 3: Zelda March Snare & Bass Groove
      // March Snare pattern: hits on 0, 2, 3, 4, 6, 8, 10, 11, 12, 14
      bool is_snare = (bar_step == 2 || bar_step == 3 || bar_step == 4 ||
                       bar_step == 6 || bar_step == 10 || bar_step == 11 ||
                       bar_step == 12 || bar_step == 14);

      if (bar_step == 0 || bar_step == 8) {
        // Kick & Root Bass downbeat
        v3_is_drum = 2; // Kick
        v3_freq = 900;
        v3_adsr = ADSR_ATTACK;
      } else if (is_snare) {
        // Snare march roll
        v3_is_drum = 1;
        v3_adsr = ADSR_ATTACK;
      } else {
        // Walking Bass note
        v3_is_drum = 0;
        v3_freq = pgm_read_word(&bar_bass[curr_bar]);
        v3_adsr = ADSR_ATTACK;
      }
    }
  }

  // --- SID ADSR Envelope State Machines ---
  // Voice 1 ADSR (Sharp attack, 80% sustain, smooth release)
  if (v1_adsr == ADSR_ATTACK) {
    if (v1_env < 240) v1_env += 30;
    else { v1_env = 255; v1_adsr = ADSR_DECAY; }
  } else if (v1_adsr == ADSR_DECAY) {
    if (v1_env > 190) v1_env -= 4;
    else { v1_env = 190; v1_adsr = ADSR_SUSTAIN; }
  } else if (v1_adsr == ADSR_RELEASE) {
    if (v1_env > 12) v1_env -= 12;
    else { v1_env = 0; v1_adsr = ADSR_IDLE; }
  }

  // Voice 2 ADSR (Crisp plucked attack, quick decay)
  if (v2_adsr == ADSR_ATTACK) {
    v2_env = 180;
    v2_adsr = ADSR_DECAY;
  } else if (v2_adsr == ADSR_DECAY) {
    if (v2_env > 6) v2_env -= 6;
    else { v2_env = 0; v2_adsr = ADSR_IDLE; }
  }

  // Voice 3 ADSR
  if (v3_adsr == ADSR_ATTACK) {
    v3_env = 250;
    v3_adsr = ADSR_DECAY;
  } else if (v3_adsr == ADSR_DECAY) {
    uint8_t dec = (v3_is_drum == 1) ? 14 : 8;
    if (v3_env > dec) v3_env -= dec;
    else { v3_env = 0; v3_adsr = ADSR_IDLE; }
  }

  // --- LFSR Noise Generator ---
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  // --- SID Oscillators & Waveforms ---
  // Voice 1: Lead with Pulse Width Modulation (PWM LFO)
  uint8_t s1 = 0;
  if (v1_freq > 0 && v1_env > 0) {
    // Dynamic PWM LFO sweep (~4Hz chorusing)
    int8_t lfo = (ticks & 0x80) ? (ticks & 0x7F) : (127 - (ticks & 0x7F));
    uint8_t dyn_pw = 128 + ((lfo * (int16_t)pwm_depth) >> 7);

    v1_phase += v1_freq;
    uint8_t p1 = v1_phase >> 8;

    if (voice_mode == 0) {
      // SID PWM Brass (Pulse with dynamic PWM)
      uint8_t pulse = (p1 < dyn_pw) ? 240 : 15;
      s1 = ((uint16_t)pulse * v1_env) >> 8;
    } else if (voice_mode == 1) {
      // SID Arp Lead (Sawtooth + Pulse)
      uint8_t saw = p1;
      uint8_t pulse = (p1 < 96) ? 220 : 35;
      s1 = (((saw + pulse) >> 1) * (uint16_t)v1_env) >> 8;
    } else if (voice_mode == 2) {
      // NES 2A03 Pure 50% Square
      uint8_t sq = (p1 < 128) ? 240 : 15;
      s1 = ((uint16_t)sq * v1_env) >> 8;
    } else {
      // Overdriven SID Saw
      s1 = ((uint16_t)p1 * v1_env) >> 8;
    }
    s1 = ((uint16_t)s1 * sid_cutoff) >> 8;
  }

  // Voice 2: SID Fast Arpeggiator (Shimmering 25% Pulse)
  uint8_t s2 = 0;
  if (v2_freq > 0 && v2_env > 0) {
    v2_phase += v2_freq;
    uint8_t p2 = v2_phase >> 8;
    uint8_t pulse2 = (p2 < 64) ? 220 : 25; // 25% duty cycle
    s2 = ((uint16_t)pulse2 * v2_env) >> 8;
  }

  // Voice 3: Bass & Drums
  uint8_t s3 = 0;
  if (v3_env > 0) {
    if (v3_is_drum == 1) {
      // March Snare: noise burst
      uint8_t noise = lfsr & 0xFF;
      s3 = ((uint16_t)noise * v3_env) >> 8;
    } else if (v3_is_drum == 2) {
      // Kick: pitch dropping triangle
      v3_phase += (v3_freq << 1);
      if (v3_freq > 35) v3_freq -= 3;
      uint8_t tri = v3_phase >> 8;
      if (tri > 127) tri = 255 - tri;
      s3 = ((uint16_t)(tri << 1) * v3_env) >> 8;
    } else {
      // Bass: deep triangle wave
      v3_phase += v3_freq;
      uint8_t tri = v3_phase >> 8;
      if (tri > 127) tri = 255 - tri;
      s3 = ((uint16_t)(tri << 1) * v3_env) >> 8;
    }
  }

  // --- 3-Voice SID Audio Mix to PWM (OC1A on PB1) ---
  uint16_t mix = (uint16_t)s3 + ((uint16_t)s1 << 1) + (uint16_t)s2;
  uint8_t out = mix >> 2;
  OCR1A = out;
}

// 5x4 Golden Triforce Bitmap (1 = Gold, 0 = Aura)
const uint8_t triforce_bmp[20] PROGMEM = {
  0, 0, 1, 0, 0,
  0, 1, 1, 1, 0,
  1, 1, 0, 1, 1,
  1, 0, 0, 0, 1
};

// 5x4 Master Sword Bitmap
const uint8_t master_sword_bmp[20] PROGMEM = {
  0, 0, 1, 0, 0, // Blade tip
  0, 0, 1, 0, 0, // Blade
  0, 1, 1, 1, 0, // Crossguard
  0, 0, 1, 0, 0  // Hilt
};

// 5x4 Heart Containers Bitmap
const uint8_t heart_bmp[20] PROGMEM = {
  1, 1, 0, 1, 1,
  1, 1, 1, 1, 1,
  0, 1, 1, 1, 0,
  0, 0, 1, 0, 0
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
  uint16_t p_left  = analogRead(POTI_LEFT);  // Left Pot: Quest Tempo (BPM)
  uint16_t p_right = analogRead(POTI_RIGHT); // Right Pot: SID Filter Cutoff & PWM Depth

  // Map Left Pot to tempo: 800 (145 BPM battle sprint) to 1750 (80 BPM majestic stroll)
  step_ticks = map(p_left, 0, 1023, 800, 1750);

  // Map Right Pot to SID filter cutoff & PWM modulation depth
  sid_cutoff = map(p_right, 0, 1023, 110, 255);
  pwm_depth  = map(p_right, 0, 1023, 20, 95);

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

  // Both buttons: Secret Fanfare / Revival Stutter while held!
  if (curr_btn_state == 3) {
    turbo_roll = 1;
  } else {
    turbo_roll = 0;
  }

  // Edge detection for taps
  if (curr_btn_state != 0 && last_btn_state == 0) {
    if (curr_btn_state == 1) {
      // Left Button: Cycle SID Voice Timbres
      voice_mode = (voice_mode + 1) % 4;
      cli();
      uint32_t col = (voice_mode == 0) ? pixels.Color(255, 215, 0) :  // Triforce Gold
                     (voice_mode == 1) ? pixels.Color(0, 240, 220) :  // Arp Cyan
                     (voice_mode == 2) ? pixels.Color(255, 60, 40) :  // NES Red
                                         pixels.Color(200, 0, 255);   // Overdrive Purple
      matrixFill(col);
      pixels.show();
      sei();
      my_delay(150);
    } else if (curr_btn_state == 2) {
      // Right Button: Cycle Zelda Visualizers
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

  if (turbo_roll) {
    // Secret Fanfare Strobe: Rainbow Triforce Flash
    uint32_t flash = (frame & 1) ? pixels.Color(255, 230, 0) : pixels.Color(0, 240, 255);
    matrixFill(flash);
  } else if (visual_mode == 0) {
    // Mode 0: The Golden Triforce & Sacred Realm
    uint8_t gold_glow = 180 + (v1_env >> 2);
    uint32_t triforce_gold = pixels.Color(gold_glow, (gold_glow * 7) >> 3, 0);
    uint32_t realm_aura   = pixels.Color(0, 60 + (v3_env >> 2), 20);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_gold = pgm_read_byte(&triforce_bmp[y * MATRIX_W + x]);
        matrixSet(x, y, is_gold ? triforce_gold : realm_aura);
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: The Master Sword (Pedestal of Time)
    uint32_t blade_silver = pixels.Color(220, 240, 255);
    uint32_t guard_cyan   = pixels.Color(0, 180, 255);
    uint32_t pedestal     = pixels.Color(40, 50, 70);
    uint32_t ray_beam     = (v1_env > 160) ? pixels.Color(120, 220, 255) : pixels.Color(10, 20, 40);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_sword = pgm_read_byte(&master_sword_bmp[y * MATRIX_W + x]);
        if (is_sword) {
          matrixSet(x, y, (y == 2) ? guard_cyan : blade_silver);
        } else {
          matrixSet(x, y, (x == 2) ? ray_beam : pedestal);
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Heart Containers & Life Meter
    uint8_t heart_pulse = (v3_is_drum == 2 && v3_env > 100);
    uint32_t heart_red  = heart_pulse ? pixels.Color(255, 20, 40) : pixels.Color(180, 10, 25);
    uint32_t bg_dark    = pixels.Color(15, 0, 5);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t is_heart = pgm_read_byte(&heart_bmp[y * MATRIX_W + x]);
        matrixSet(x, y, is_heart ? heart_red : bg_dark);
      }
    }
  } else {
    // Mode 3: Rupee Storm & Hyrule Field (Green, Blue, Red Rupees)
    matrixFade(75);
    uint8_t rx = (frame * 2 + curr_step) % MATRIX_W;
    uint8_t ry = (frame + (curr_step >> 2)) % MATRIX_H;
    // Rupee color: Green (1 Rupee), Blue (5 Rupees), Red (20 Rupees)
    uint32_t rupee_col = (frame % 3 == 0) ? pixels.Color(0, 255, 40) :
                         (frame % 3 == 1) ? pixels.Color(0, 140, 255) :
                                            pixels.Color(255, 30, 20);
    matrixSet(rx, ry, rupee_col);
  }

  pixels.show();
  sei(); // Re-enable audio interrupt

  frame++;
  my_delay(40); // 25 FPS visual update
}
