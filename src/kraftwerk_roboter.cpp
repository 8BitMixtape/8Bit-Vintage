/* ---------------------------------------------------------------------------------------
    KRAFTWERK — DIE ROBOTER ("THE ROBOTS") [FULL MOS 6581 SID SOUND ENGINE]
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    An authentic 8-bit multi-voice synthesizer tribute to Kraftwerk's 1978 electro
    anthem "Die Roboter" / "The Robots" (Die Mensch-Maschine) powered by a true
    Commodore 64 MOS 6581/8580 "SID-style" Direct Digital Synthesis (DDS) sound engine:
      - 3 polyphonic SID voices with phase-accumulator Direct Digital Synthesis (DDS)
      - Dynamic Pulse-Width Modulation (PWM LFO) for fat chorused singing brass
      - Authentic SID Ring Modulation (Voice 1 triangle MSB modulated by Voice 3)
      - Authentic SID Hard Sync (Voice 1 forced phase reset on Voice 3 overflow)
      - 50Hz Fast SID Arpeggiator (Tracker 0xy) for shimmering robotic chords
      - Hardware-style ADSR envelope state machines (Attack, Decay, Sustain, Release)
      - Resonant low-pass SID filter simulation with potentiometer cutoff & Q control

    Hardware Mapping:
      PB0: 20x NeoPixels (5x4 Matrix)
      PB1: 8-bit Audio PWM (OC1A Timer1 64MHz PLL)
      PB4 (A2): Left Pot (Robot Clock Tempo: 85 - 140 BPM)
      PB2 (A1): Right Pot (SID Filter Cutoff, Resonance & PWM Modulation Depth)
      PB3 (A3): Resistor Ladder Buttons
                - Left Button:  Cycle SID Voicings (PWM Chorus, Ring Mod, Hard Sync, Raw Saw)
                - Right Button: Cycle Visualizers (Robot Head, Walking Automaton, Spectrum, Teletype)
                - Both Buttons: Emergency Robot Overload / Malfunction Laser Stutter!
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
#define N_E2    540
#define N_G2    642
#define N_A2    721
#define N_B2    809
#define N_C3    857
#define N_D3    962
#define N_E3    1080
#define N_Fs3   1212
#define N_G3    1285
#define N_A3    1442
#define N_B3    1618
#define N_C4    1715
#define N_D4    1925
#define N_Ds4   2039
#define N_E4    2160
#define N_Fs4   2425
#define N_G4    2569
#define N_A4    2884
#define N_B4    3237
#define N_C5    3429
#define N_D5    3849
#define N_E5    4320
#define N_Fs5   4850
#define N_G5    5138

// Lookup table for Lead melody notes (11 distinct notes)
const uint16_t roboter_notes[11] PROGMEM = {
  N_REST, // 0
  N_B3,   // 1
  N_D4,   // 2
  N_E4,   // 3
  N_Fs4,  // 4
  N_G4,   // 5
  N_A4,   // 6
  N_B4,   // 7
  N_D5,   // 8
  N_E5,   // 9
  N_G5    // 10
};

// --- Complete 256-Step (16 Bar) "Die Roboter" Arrangement ---
const uint8_t kraftwerk_roboter_lead[256] PROGMEM = {
  // Bar 1: E5 B4 G4 E4 D4 G4 B4 D5
   9,  9,  7,  7,  5,  5,  3,  3,  2,  2,  5,  5,  7,  7,  8,  8,
  // Bar 2: E5 B4 G4 E4 D4 G4 B4 D5
   9,  9,  7,  7,  5,  5,  3,  3,  2,  2,  5,  5,  7,  7,  8,  8,
  // Bar 3: D5 B4 F#4 D4 C4 E4 G4 B4
   8,  8,  7,  7,  4,  4,  2,  2,  3,  3,  5,  5,  7,  7,  8,  8,
  // Bar 4: E5 B4 G4 E4 turnaround
   9,  9,  7,  7,  5,  5,  3,  3,  9,  9,  7,  7,  0,  0,  0,  0,

  // Bar 5: "Wir sind die Ro-bo-ter" (E4 G4 B4 E5 E5 D5 B4)
   3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
  // Bar 6: "Wir sind die Ro-bo-ter"
   3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
  // Bar 7: "Ja tvoi sluga, ja tvoi rabotnik"
   1,  1,  2,  2,  3,  3,  5,  5,  5,  4,  3,  3,  2,  2,  3,  0,
  // Bar 8: "Wir sind die Ro-bo-ter"
   3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,

  // Bar 9: Arpeggiator Interlude (Em: E4 G4 B4 E5 G5 E5 B4 G4)
   3,  3,  5,  5,  7,  7,  9,  9, 10, 10,  9,  9,  7,  7,  5,  5,
  // Bar 10: Em
   3,  3,  5,  5,  7,  7,  9,  9, 10, 10,  9,  9,  7,  7,  5,  5,
  // Bar 11: D: D4 F#4 A4 D5
   2,  2,  4,  4,  6,  6,  8,  8,  8,  8,  6,  6,  4,  4,  2,  2,
  // Bar 12: C -> B cadence
   3,  3,  5,  5,  7,  7,  8,  8,  7,  7,  5,  5,  4,  4,  0,  0,

  // Bar 13: "Wir sind die Ro-bo-ter"
   3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
  // Bar 14: "Wir sind die Ro-bo-ter"
   3,  3,  5,  5,  7,  7,  9,  9,  9,  9,  8,  8,  7,  7,  0,  0,
  // Bar 15: "Ja tvoi sluga, ja tvoi rabotnik"
   1,  1,  2,  2,  3,  3,  5,  5,  5,  4,  3,  3,  2,  2,  3,  0,
  // Bar 16: Final mechanical resolution
   9,  9,  9,  9,  8,  8,  7,  7,  5,  5,  4,  4,  3,  3,  0,  0
};

// Root bass notes for the 16 bars (Em, Em, D, Em, etc.)
const uint16_t bar_roots[16] PROGMEM = {
  N_E2, N_E2, N_D3, N_E2,
  N_E2, N_E2, N_C3, N_E2,
  N_E2, N_E2, N_D3, N_C3,
  N_E2, N_E2, N_C3, N_E2
};

// 50Hz Fast SID Arp Chords (Em, D, C, B)
const uint16_t sid_arp_chords[4][3] PROGMEM = {
  { N_E4, N_G4,  N_B4 },  // Em
  { N_D4, N_Fs4, N_A4 },  // D
  { N_C4, N_E4,  N_G4 },  // C
  { N_B3, N_Ds4, N_Fs4 }  // B
};

// --- SID ADSR Envelope States ---
#define ADSR_ATTACK  0
#define ADSR_DECAY   1
#define ADSR_SUSTAIN 2
#define ADSR_RELEASE 3
#define ADSR_IDLE    4

// --- SID Voice Direct Digital Synthesis Structure ---
struct SIDVoice {
  uint16_t freq;         // Phase increment (16-bit DDS)
  uint16_t phase;        // Phase accumulator (16-bit DDS)
  uint16_t pulse_width;  // 16-bit pulse threshold
  uint8_t  adsr_state;   // Current ADSR stage
  uint8_t  env;          // Current 8-bit envelope volume (0-255)
  uint8_t  att_rate;     // Attack increment per tick
  uint8_t  dec_rate;     // Decay decrement per tick
  uint8_t  sus_level;    // Sustain volume level
  uint8_t  rel_rate;     // Release decrement per tick
};

volatile SIDVoice v1; // Voice 1: Lead / Vocoder / Hard Sync / Ring Mod
volatile SIDVoice v2; // Voice 2: 50Hz Fast SID Tracker Arp Chords
volatile SIDVoice v3; // Voice 3: Bass / LFSR Noise Drums (Master Sync/Mod Oscillator)

volatile uint8_t  v3_is_drum    = 0; // 0=Bass, 1=Snare/Clap, 2=Kick
volatile uint8_t  v2_arp_idx    = 0;
uint16_t          lfsr          = 0x5183; // 16-bit Galois LFSR noise generator

// --- Sequencer & Control State ---
volatile uint16_t ticks         = 0;
volatile uint16_t step_ticks    = 1300; // ~115 BPM
volatile uint8_t  curr_step     = 0;
volatile uint8_t  curr_bar      = 0;
volatile uint8_t  turbo_roll    = 0;

// SID Knobs & Modes
volatile uint8_t  sid_mode      = 0; // 0=PWM Brass, 1=Ring Modulation, 2=Hard Sync, 3=Raw Saw
volatile uint8_t  visual_mode   = 0; // 0=Robot Head, 1=Walking Automaton, 2=Spectrum, 3=Teletype
volatile uint8_t  sid_cutoff    = 210;
volatile uint8_t  sid_res       = 50;
volatile uint8_t  pwm_depth     = 90;

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

// Inline triangle waveform calculator
static inline uint8_t get_triangle(uint16_t phase) {
  uint8_t t = phase >> 8;
  return (t < 128) ? (t << 1) : ((255 - t) << 1);
}

// --- 10kHz SID Direct Digital Synthesis Interrupt ---
ISR(TIMER0_COMPA_vect) {
  ticks++;

  static uint16_t tick_acc = 0;
  static uint8_t  prev_lead_idx = 0;

  // 1. 50Hz Fast SID Tracker Arpeggiator Clock (Tracker 0xy command, every 200 ticks)
  if ((ticks % 180) == 0) {
    v2_arp_idx = (v2_arp_idx + 1) % 3;
    uint8_t chord_bar = curr_bar & 3;
    v2.freq = pgm_read_word(&sid_arp_chords[chord_bar][v2_arp_idx]);
  }

  // 2. 16th-Note Step Sequencer
  tick_acc++;
  uint16_t target_ticks = turbo_roll ? (step_ticks >> 2) : step_ticks;
  if (target_ticks < 120) target_ticks = 120;

  if (tick_acc >= target_ticks) {
    tick_acc = 0;

    if (turbo_roll) {
      // Robot Malfunction Emergency Stutter!
      v1.freq = N_E5;
      v1.adsr_state = ADSR_ATTACK;
      v3_is_drum = 1;
      v3.adsr_state = ADSR_ATTACK;
    } else {
      curr_step = (curr_step + 1) & 255;
      curr_bar  = curr_step >> 4;
      uint8_t bar_step = curr_step & 15;

      // Voice 1: Lead Melody Note
      uint8_t note_idx = pgm_read_byte(&kraftwerk_roboter_lead[curr_step]);
      if (note_idx > 0) {
        v1.freq = pgm_read_word(&roboter_notes[note_idx]);
        if (note_idx != prev_lead_idx || (bar_step & 1) == 0) {
          v1.adsr_state = ADSR_ATTACK; // Trigger new key attack
        }
      } else {
        v1.adsr_state = ADSR_RELEASE;
      }
      prev_lead_idx = note_idx;

      // Voice 2: SID Tracker Arp Trigger
      v2.adsr_state = ADSR_ATTACK;

      // Voice 3: Bassline & Percussive Noise Drums
      uint16_t root = pgm_read_word(&bar_roots[curr_bar]);
      if (bar_step == 0 || bar_step == 8) {
        // Electronic Kick
        v3_is_drum = 2;
        v3.freq = 980;
        v3.adsr_state = ADSR_ATTACK;
      } else if (bar_step == 4 || bar_step == 12) {
        // Electronic Snare / Simmons Clap
        v3_is_drum = 1;
        v3.adsr_state = ADSR_ATTACK;
      } else {
        // Driving Octave-Jumping Bass (E2 <-> E3 bounce)
        v3_is_drum = 0;
        v3.freq = (bar_step & 1) ? (root << 1) : root;
        v3.adsr_state = ADSR_ATTACK;
      }
    }
  }

  // 3. Hardware-Style ADSR Envelope Generators
  // Voice 1 ADSR (Fast Attack, Snappy Decay, 80% Sustain)
  if (v1.adsr_state == ADSR_ATTACK) {
    if (v1.env < 240) v1.env += v1.att_rate;
    else { v1.env = 255; v1.adsr_state = ADSR_DECAY; }
  } else if (v1.adsr_state == ADSR_DECAY) {
    if (v1.env > v1.sus_level + v1.dec_rate) v1.env -= v1.dec_rate;
    else { v1.env = v1.sus_level; v1.adsr_state = ADSR_SUSTAIN; }
  } else if (v1.adsr_state == ADSR_RELEASE) {
    if (v1.env > v1.rel_rate) v1.env -= v1.rel_rate;
    else { v1.env = 0; v1.adsr_state = ADSR_IDLE; }
  }

  // Voice 2 ADSR (Tracker Arp: Crisp Plucked Attack, Quick Decay)
  if (v2.adsr_state == ADSR_ATTACK) {
    v2.env = 190;
    v2.adsr_state = ADSR_DECAY;
  } else if (v2.adsr_state == ADSR_DECAY) {
    if (v2.env > v2.dec_rate) v2.env -= v2.dec_rate;
    else { v2.env = 0; v2.adsr_state = ADSR_IDLE; }
  }

  // Voice 3 ADSR (Bass & Drums)
  if (v3.adsr_state == ADSR_ATTACK) {
    v3.env = 255;
    v3.adsr_state = ADSR_DECAY;
  } else if (v3.adsr_state == ADSR_DECAY) {
    uint8_t d = (v3_is_drum == 1) ? 16 : (v3_is_drum == 2 ? 14 : 10);
    if (v3.env > d) v3.env -= d;
    else { v3.env = 0; v3.adsr_state = ADSR_IDLE; }
  }

  // 4. LFSR Noise Generator (Galois 16-bit)
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  // 5. Voice 3 DDS (Master Oscillator, Bass & Drums)
  uint16_t prev_p3 = v3.phase;
  v3.phase += v3.freq;
  bool v3_overflow = (v3.phase < prev_p3); // Overflow trigger for Hard Sync!

  uint8_t s3 = 0;
  if (v3.env > 0) {
    if (v3_is_drum == 1) {
      // Snare / Clap: Noise burst
      uint8_t noise = lfsr & 0xFF;
      s3 = ((uint16_t)noise * v3.env) >> 8;
    } else if (v3_is_drum == 2) {
      // Kick: Pitch-dropping triangle
      if (v3.freq > 40) v3.freq -= 4;
      uint8_t tri = get_triangle(v3.phase);
      s3 = ((uint16_t)tri * v3.env) >> 8;
    } else {
      // Bass: Deep resonant triangle
      uint8_t tri = get_triangle(v3.phase);
      s3 = ((uint16_t)tri * v3.env) >> 8;
    }
  }

  // 6. Voice 1 DDS (Lead with PWM, Ring Mod, Hard Sync & Saw)
  uint8_t s1 = 0;
  if (v1.freq > 0 && v1.env > 0) {
    if (sid_mode == 2 && v3_overflow) {
      // Authentic SID Hard Sync: Reset Voice 1 phase when Voice 3 overflows!
      v1.phase = 0;
    } else {
      v1.phase += v1.freq;
    }

    if (sid_mode == 0) {
      // SID PWM Brass: Dynamic LFO-swept pulse width (~4Hz chorusing)
      int8_t lfo = (ticks & 0x80) ? (ticks & 0x7F) : (127 - (ticks & 0x7F));
      uint16_t dyn_pw = v1.pulse_width + ((int16_t)lfo * (pwm_depth << 1));
      uint8_t pulse = (v1.phase < dyn_pw) ? 245 : 15;
      s1 = ((uint16_t)pulse * v1.env) >> 8;
    } else if (sid_mode == 1) {
      // Authentic SID Ring Modulation: Triangle MSB modulated by Voice 3 MSB!
      uint8_t tri1 = get_triangle(v1.phase);
      uint8_t ring = (v3.phase & 0x8000) ? (255 - tri1) : tri1;
      s1 = ((uint16_t)ring * v1.env) >> 8;
    } else if (sid_mode == 2) {
      // SID Hard Sync: Screaming synced sawtooth sweep!
      uint8_t saw = v1.phase >> 8;
      s1 = ((uint16_t)saw * v1.env) >> 8;
    } else {
      // Raw MOS 6581 Sawtooth
      uint8_t saw = v1.phase >> 8;
      s1 = ((uint16_t)saw * v1.env) >> 8;
    }
  }

  // 7. Voice 2 DDS (50Hz Fast SID Arpeggiator: 25% Duty Cycle Pulse Wave)
  uint8_t s2 = 0;
  if (v2.freq > 0 && v2.env > 0) {
    v2.phase += v2.freq;
    uint8_t pulse2 = ((v2.phase >> 8) < 64) ? 230 : 25;
    s2 = ((uint16_t)pulse2 * v2.env) >> 8;
  }

  // 8. 3-Voice Mix & Resonant Low-Pass Filter Simulation (MOS 6581 VCF)
  uint16_t dry_mix = (s3 + (s1 << 1) + s2) >> 2;

  // 1-Pole Resonant Filter with Feedback Resonance
  static int16_t flt_lp = 0;
  static int16_t flt_bp = 0;
  int16_t in = (int16_t)dry_mix;
  int16_t res_in = in - (((int32_t)flt_bp * sid_res) >> 8);
  flt_lp += (((int32_t)(res_in - flt_lp)) * sid_cutoff) >> 8;
  flt_bp = res_in - flt_lp;

  int16_t out = flt_lp;
  if (out < 0) out = 0;
  else if (out > 255) out = 255;

  OCR1A = (uint8_t)out;
}

// 5x4 Robot Head Silhouette Bitmap
const uint8_t robot_head_bmp[20] PROGMEM = {
  0, 1, 1, 1, 0, // Antenna & Crown
  1, 0, 1, 0, 1, // Glowing Visor Eyes (Left & Right)
  1, 1, 1, 1, 1, // Metal Cheekplates
  0, 1, 1, 1, 0  // Vocoder Speaker Grille
};

// 5x4 Walking Automaton Frame A
const uint8_t automaton_a_bmp[20] PROGMEM = {
  0, 1, 1, 1, 0,
  0, 1, 1, 1, 0,
  1, 1, 0, 1, 0,
  1, 0, 0, 0, 1
};

// 5x4 Walking Automaton Frame B
const uint8_t automaton_b_bmp[20] PROGMEM = {
  0, 1, 1, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 0, 1, 1,
  0, 1, 0, 1, 0
};

// --- NeoPixel Visualizers ---
void update_visuals() {
  uint8_t step = curr_step;
  uint8_t bar_step = step & 15;

  if (turbo_roll) {
    // Malfunction strobe break: Red alert flashing!
    uint32_t alert = (ticks & 64) ? pixels.Color(255, 0, 0) : pixels.Color(255, 255, 255);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, alert);
    }
    pixels.show();
    return;
  }

  if (visual_mode == 0) {
    // Mode 0: Die Roboter Head & Glowing Cyan Visor Eyes
    uint8_t eye_pulse = (v1.env > 0) ? v1.env : 70;
    uint32_t c_head = pixels.Color(180, 0, 0); // Kraftwerk Red casing
    uint32_t c_eye  = pixels.Color(0, eye_pulse, eye_pulse); // Cyan glowing robot eyes
    uint32_t c_mouth= (bar_step & 2) ? pixels.Color(255, 255, 255) : pixels.Color(40, 40, 40);

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t is_part = pgm_read_byte(&robot_head_bmp[y * 5 + x]);
        if (!is_part) {
          pixels.setPixelColor(idx, 0);
        } else if (y == 1 && (x == 1 || x == 3)) {
          pixels.setPixelColor(idx, c_eye);
        } else if (y == 3) {
          pixels.setPixelColor(idx, c_mouth);
        } else {
          pixels.setPixelColor(idx, c_head);
        }
      }
    }
  } else if (visual_mode == 1) {
    // Mode 1: Walking Mechanical Automaton (Steps to the 16th beat)
    const uint8_t* frame = (bar_step & 4) ? automaton_a_bmp : automaton_b_bmp;
    uint32_t metal_body = pixels.Color(0, 180, 220); // Cyan steel
    uint32_t led_eye    = pixels.Color(255, 30, 30);  // Red sensor

    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t is_part = pgm_read_byte(&frame[y * 5 + x]);
        if (!is_part) {
          pixels.setPixelColor(idx, 0);
        } else if (y == 0 && x == 2) {
          pixels.setPixelColor(idx, led_eye);
        } else {
          pixels.setPixelColor(idx, metal_body);
        }
      }
    }
  } else if (visual_mode == 2) {
    // Mode 2: Kling Klang 5-Band Vocoder Spectrum Analyzer
    uint8_t bands[5];
    bands[0] = v3.env >> 5;                  // Sub Bass
    bands[1] = (v3_is_drum ? v3.env : 0) >> 4; // Drums
    bands[2] = v1.env >> 5;                  // Vocoder / Lead
    bands[3] = (v1.env + v2.env) >> 6;       // Mid
    bands[4] = v2.env >> 5;                  // High Arp

    for (uint8_t x = 0; x < 5; x++) {
      uint8_t height = bands[x];
      if (height > 4) height = 4;
      for (uint8_t y = 0; y < 4; y++) {
        uint8_t idx = matrixIndex(x, 3 - y);
        if (y < height) {
          uint32_t col = (y == 3) ? pixels.Color(255, 0, 0) :
                         (y == 2) ? pixels.Color(255, 200, 0) :
                                    pixels.Color(0, 220, 50);
          pixels.setPixelColor(idx, col);
        } else {
          pixels.setPixelColor(idx, pixels.Color(5, 10, 5));
        }
      }
    }
  } else {
    // Mode 3: Cyrillic Teletype / Binary Data Cascade
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t idx = matrixIndex(x, y);
        uint8_t bit = ((step + x * 3 + y * 7) ^ (ticks >> 7)) & 1;
        if (bit) {
          pixels.setPixelColor(idx, pixels.Color(0, 255, 80)); // Matrix Green
        } else {
          pixels.setPixelColor(idx, pixels.Color(0, 25, 10));
        }
      }
    }
  }

  pixels.show();
}

// --- Setup ---
void setup() {
  cli();

  // NeoPixel Setup
  pixels.begin();
  pixels.setBrightness(45);
  pixels.show();

  // Initialize SID Voices
  v1.freq        = 0;
  v1.phase       = 0;
  v1.pulse_width = 32768; // 50% initial duty cycle
  v1.adsr_state  = ADSR_IDLE;
  v1.env         = 0;
  v1.att_rate    = 35;   // Fast attack
  v1.dec_rate    = 4;    // Smooth decay
  v1.sus_level   = 190;  // 75% sustain
  v1.rel_rate    = 14;   // Musical release

  v2.freq        = 0;
  v2.phase       = 0;
  v2.pulse_width = 16384; // 25% duty cycle
  v2.adsr_state  = ADSR_IDLE;
  v2.env         = 0;
  v2.att_rate    = 190;
  v2.dec_rate    = 8;
  v2.sus_level   = 0;
  v2.rel_rate    = 15;

  v3.freq        = 0;
  v3.phase       = 0;
  v3.pulse_width = 32768;
  v3.adsr_state  = ADSR_IDLE;
  v3.env         = 0;
  v3.att_rate    = 255;
  v3.dec_rate    = 10;
  v3.sus_level   = 0;
  v3.rel_rate    = 12;

  // High-Speed 64MHz PLL Audio PWM on PB1 (OC1A)
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  TCCR1  = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10);
  OCR1A  = 128;
  pinMode(SPEAKERPIN, OUTPUT);

  // 10kHz Audio Sample Clock on Timer0
  TCCR0A = (3 << WGM00);
  TCCR0B = (1 << WGM02) | (2 << CS00);
  TIMSK  = (1 << OCIE0A);
  OCR0A  = 199;

  sei();
}

// --- Main Loop ---
void loop() {
  // 1. Read Potentiometers
  uint16_t pot_l = analogRead(POTI_LEFT);  // 0 - 1023: Tempo
  uint16_t pot_r = analogRead(POTI_RIGHT); // 0 - 1023: SID Filter Cutoff & PWM Depth

  // Tempo range: ~85 BPM (1760 ticks) down to ~145 BPM (1030 ticks)
  step_ticks = 1760 - ((uint32_t)pot_l * 730) / 1023;

  // SID Filter Cutoff & PWM Modulation Depth
  sid_cutoff = 130 + ((uint32_t)pot_r * 120) / 1023;
  sid_res    = 30  + ((uint32_t)pot_r * 80) / 1023;
  pwm_depth  = 30  + ((uint32_t)pot_r * 90) / 1023;

  // 2. Read Button Resistor Ladder (PB3 / A3)
  uint16_t btn = analogRead(BUTTON_PIN);
  static uint8_t prev_pressed = 0;

  if (btn < Vbutton_both) {
    // Both Buttons: Robot Malfunction Overload!
    turbo_roll = 1;
  } else {
    turbo_roll = 0;

    if (btn < Vbutton_right && prev_pressed == 0) {
      // Right Button: Cycle Visualizers
      visual_mode = (visual_mode + 1) & 3;
      pixels.fill(pixels.Color(255, 255, 255), 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn < Vbutton_left && prev_pressed == 0) {
      // Left Button: Cycle SID Voice Modes
      sid_mode = (sid_mode + 1) & 3;
      uint32_t badge = (sid_mode == 0) ? pixels.Color(0, 255, 255) : // PWM Brass (Cyan)
                       (sid_mode == 1) ? pixels.Color(255, 120, 0) : // Ring Mod (Orange)
                       (sid_mode == 2) ? pixels.Color(255, 0, 150) : // Hard Sync (Magenta)
                                         pixels.Color(0, 255, 50);   // Raw Saw (Green)
      pixels.fill(badge, 0, NUMPIXELS);
      pixels.show();
      my_delay(50);
      prev_pressed = 1;
    } else if (btn > Vbutton_releaseLevel) {
      prev_pressed = 0;
    }
  }

  // 3. Render Visuals
  update_visuals();
  my_delay(20);
}
