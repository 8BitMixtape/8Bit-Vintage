/* ---------------------------------------------------------------------------------------
    ABK-20 HARDWARE & INTERFACE TEST (Rev 0.3)
    Comprehensive visual & audio diagnostics for the ABK-20 Synthesizer

    Inputs Tested:
      1. Rotary Potentiometer POT2 (PB4 / A2 - CW Non-Inverted):
         - Visual: Columns 0 & 1 dual vertical bar graph (Green -> Cyan)
         - Audio:  Continuous pitch sweep (~65 Hz to 880 Hz)
      2. Slide Fader RV1 (PB2 / A1 - Upward):
         - Visual: Columns 3 & 4 dual vertical bar graph (Amber -> Red)
         - Audio:  Waveform Timbre (Triangle -> Sawtooth -> Pulse PWM)
      3. SPDT Toggle Switch SW1 (PB3 / A3):
         - Visual: Center Column top pixel (UP / Pos 2) / bottom pixel (DOWN / Pos 1)
         - Audio:  
             - UP (Pos 2):    Fast Major Acid Arpeggio (Root -> 3rd -> 5th -> Octave)
             - DOWN (Pos 1):  Sub-Bass Heavy Drone (-1 Octave Sub-Oscillator)
             - CENTER:        Pure Continuous Lead Tone
      4. Button SW2 (PB3 / A3):
         - Visual: Center Column middle pixels flash brilliant White / Magenta
         - Audio:  Instant +1 Octave Boost, High Resonance Grit, & Click Chirp
         - Physical: Bi-color LED inside the button illuminates on press!
   --------------------------------------------------------------------------------------- */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "abk20_hardware.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_MATRIX_LEDS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// --- Synth Engine State ---
volatile uint16_t phase_acc = 0;
volatile uint16_t phase_inc = 200;
volatile uint8_t  synth_mode = 0;       // 0=Normal, 1=Arp (Up), 2=Bunny Hop (Down)
volatile uint8_t  synth_wave = 0;       // 0=Tri, 1=Saw, 2=Pulse
volatile uint8_t  pulse_width = 128;
volatile uint8_t  fader_val = 128;      // Slide fader value passed to ISR
volatile uint8_t  hop_vis = 0;          // Bouncing hop level for visual indicator (0..3)
volatile uint8_t  hop_beat_env = 0;     // Beat envelope (240..0) for full-matrix pulse
volatile uint8_t  sw2_held = 0;
volatile uint8_t  click_countdown = 0;

// Arpeggio multipliers (1.0, 1.25, 1.5, 2.0 mapped to integers)
const uint8_t arp_mult[4] = { 4, 5, 6, 8 }; // Divide by 4: 1x, 1.25x, 1.5x, 2x

void setup() {
  pinMode(PIN_SPEAKER, OUTPUT);

  pixels.begin();
  pixels.setBrightness(75); // Balanced brightness for clear testing
  pixels.clear();
  pixels.show();

  // High-Speed 64 MHz PLL Timer1 PWM Setup on PB1 (OC1A)
  PLLCSR |= (1 << PLLE);
  delayMicroseconds(100);
  while (!(PLLCSR & (1 << PLOCK)));
  PLLCSR |= (1 << PCKE);

  // Enable PWM on A (PB1), Clear on match, Prescaler 8 (~31.25 kHz carrier)
  TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS12);
  GTCCR = 0; // Ensure PB4 is free for analog input (PIN_POTI_ADC)

  TIMSK |= (1 << TOIE1);
  sei();
}

// Fast Timer1 Audio Interrupt (~15.6 kHz synthesis rate)
ISR(TIMER1_OVF_vect) {
  static uint8_t div = 0;
  div = !div;
  if (div) return;

  static uint16_t arp_tick = 0;
  arp_tick++;

  // Handle transient confirmation chirp on button/switch changes
  if (click_countdown > 0) {
    click_countdown--;
    OCR1A = (click_countdown & 1) ? 230 : 25;
    return;
  }

  // Calculate actual step pitch
  uint16_t cur_inc = phase_inc;
  
  if (synth_mode == 1) { // Arpeggio mode (Toggle UP / Pos 2)
    uint8_t step = (arp_tick >> 10) & 0x03; // ~15 Hz arp rate
    cur_inc = ((uint32_t)phase_inc * arp_mult[step]) >> 2;
  } else if (synth_mode == 2) { // Bunny Hop / Acid Sub-Bass (Toggle DOWN / Pos 1)
    // Slide fader (fader_val) controls hop tempo (5 Hz to ~30 Hz)
    uint8_t hop_speed = 3 + (fader_val >> 4); // 3 to 18
    static uint16_t hop_phase = 0;
    hop_phase += hop_speed;

    uint8_t hop_cycle = (hop_phase >> 8) & 0x07; // 8-step hop loop (0..7)
    uint8_t hop_sub = hop_phase & 0xFF;          // Sub-step ramp (0..255)

    // Visual envelope decay
    if (hop_beat_env > 4) hop_beat_env -= 4; else hop_beat_env = 0;

    // Geometric bounce offsets with per-beat trigger
    uint8_t bounce = 0;
    switch (hop_cycle) {
      case 0: bounce = 0;  if (hop_sub < 16) hop_beat_env = 240; break;
      case 1: bounce = 7;  break;
      case 2: bounce = 2;  break;
      case 3: bounce = 12; if (hop_sub < 16) hop_beat_env = 200; break;
      case 4: bounce = 4;  break;
      case 5: bounce = 16; if (hop_sub < 16) hop_beat_env = 255; break;
      case 6: bounce = 8;  break;
      case 7: bounce = 1;  break;
    }
    hop_vis = bounce >> 2; // (0..4 for visual column levels)

    // Sub-oscillator pitch modulation: base pitch / 2 (-1 Octave) + energetic bounce
    cur_inc = (phase_inc >> 1) + ((uint32_t)phase_inc * bounce >> 6);
  }

  // Button SW2 Pitch Mod: +1 Octave boost when held
  if (sw2_held) {
    cur_inc <<= 1;
  }

  phase_acc += cur_inc;
  uint8_t saw = phase_acc >> 8;

  int16_t sample = 128;
  if (synth_wave == 0) {
    // Triangle wave (sweet & mellow)
    sample = (saw < 128) ? (saw << 1) : ((255 - saw) << 1);
  } else if (synth_wave == 1) {
    // Sawtooth wave (bright & rich)
    sample = saw;
  } else {
    // Pulse wave with variable pulse width
    sample = (saw > pulse_width) ? 220 : 35;
  }

  // Button SW2 overdrive/clip
  if (sw2_held) {
    if (sample > 200) sample = 255;
    else if (sample < 55) sample = 0;
  }

  OCR1A = (uint8_t)sample;
}

// Smooth 8-bit rainbow color wheel with brightness scaling
static inline uint32_t wheel(uint8_t pos, uint8_t b) {
  pos = 255 - pos;
  uint8_t r, g, bl;
  if (pos < 85) {
    r = 255 - pos * 3;
    g = 0;
    bl = pos * 3;
  } else if (pos < 170) {
    pos -= 85;
    r = 0;
    g = pos * 3;
    bl = 255 - pos * 3;
  } else {
    pos -= 170;
    r = pos * 3;
    g = 255 - pos * 3;
    bl = 0;
  }
  if (b < 255) {
    r = ((uint16_t)r * b) >> 8;
    g = ((uint16_t)g * b) >> 8;
    bl = ((uint16_t)bl * b) >> 8;
  }
  return pixels.Color(r, g, bl);
}

// Matrix indexing: 5 columns (x: 0..4), 4 rows (y: 0..3)
static inline uint8_t mIdx(uint8_t x, uint8_t y) {
  return (y * 5) + x;
}

void loop() {
  // 1. Read Inputs using calibrated ABK-20 hardware decoders
  uint8_t left_pot  = readRotaryPot();  // 0 to 255 (Clockwise increases)
  uint8_t right_pot = readSlideFader(); // 0 to 255 (Sliding UP increases)
  LadderState btn_state = readLadderState();

  static LadderState prev_btn_state = (LadderState)0xFF;
  if (btn_state != prev_btn_state) {
    click_countdown = 20; // Trigger instant audio chirp on transition
    prev_btn_state = btn_state;
  }

  bool btn_pressed = isButtonSW2Pressed(btn_state);
  sw2_held = btn_pressed;

  // 2. Map Audio Engine Parameters
  // Rotary Pot: Smooth pitch from ~65 Hz (C2) to ~880 Hz (A5)
  phase_inc = 50 + (left_pot * 3);

  // Slide Fader: Waveform selection & Pulse Width
  fader_val = right_pot; // Feed to bunny hop generator
  if (right_pot < 85) {
    synth_wave = 0; // Triangle (warm)
  } else if (right_pot < 170) {
    synth_wave = 1; // Sawtooth (bright)
  } else {
    synth_wave = 2; // Pulse / PWM (edgy)
    pulse_width = 60 + ((right_pot - 170) * 135 / 85);
  }

  // Toggle switch mode
  uint8_t toggle_pos = getTogglePosition(btn_state);
  if (toggle_pos == 2) {
    synth_mode = 1; // Pos 2 (UP) = Arpeggio mode
  } else if (toggle_pos == 1) {
    synth_mode = 2; // Pos 1 (DOWN) = Bunny Hop / Sub-Bass mode
  } else {
    synth_mode = 0; // Center = Normal continuous lead tone
  }

  // 3. Render 5x4 Matrix Display
  pixels.clear();

  // SPECIAL FEATURE: When Toggle is DOWN and Button SW2 is pressed,
  // the entire matrix illuminates in a unified color that pulses with the beat!
  if (btn_state == LADDER_SW2_TOGGLE_1) {
    uint8_t beat_bright = 45 + ((hop_beat_env * 210) >> 8); // Pulses 45 -> 255 on each hop beat
    uint32_t col = wheel(left_pot, beat_bright);
    for (uint8_t i = 0; i < NUM_MATRIX_LEDS; i++) {
      pixels.setPixelColor(i, col);
    }
  } else {
    // --- Normal Interface Display (Meters + Dedicated Indicators) ---

    // Left Pot Bar Graph (Columns 0 & 1): 4 vertical steps
    uint8_t left_bars = (left_pot + 25) / 55; // 0 to 4
    for (uint8_t y = 0; y < 4; y++) {
      uint8_t row_from_bot = 3 - y;
      if (row_from_bot < left_bars) {
        // Color gradient: Deep Green at bottom to Electric Cyan at top
        uint32_t col = pixels.Color(0, 150 + (row_from_bot * 25), row_from_bot * 65);
        pixels.setPixelColor(mIdx(0, y), col);
        pixels.setPixelColor(mIdx(1, y), col);
      }
    }

    // Right Fader Bar Graph (Columns 3 & 4): 4 vertical steps
    uint8_t right_bars = (right_pot + 25) / 55; // 0 to 4
    for (uint8_t y = 0; y < 4; y++) {
      uint8_t row_from_bot = 3 - y;
      if (row_from_bot < right_bars) {
        // Color gradient: Warm Amber at bottom to Fiery Red/White at top
        uint32_t col = pixels.Color(160 + (row_from_bot * 25), (3 - row_from_bot) * 35, 0);
        pixels.setPixelColor(mIdx(3, y), col);
        pixels.setPixelColor(mIdx(4, y), col);
      }
    }

    // --- Center Column 2: Status Indicators with Soft Standby Backlight ---
    // 1. Pixel 2 (Row 0, Top): Dedicated Toggle UP / Pos 2 (Bright Cyan active / Dim Cyan standby)
    if (toggle_pos == 2) {
      pixels.setPixelColor(mIdx(2, 0), pixels.Color(0, 255, 255)); // Bright Cyan UP
    } else {
      pixels.setPixelColor(mIdx(2, 0), pixels.Color(0, 18, 18));   // Dimly lit standby
    }

    // 2. Pixel 17 (Row 3, Bottom): Dedicated Toggle DOWN / Pos 1 (Bright Yellow active / Dim Amber standby)
    if (toggle_pos == 1) {
      pixels.setPixelColor(mIdx(2, 3), pixels.Color(255, 200, 0)); // Bright Yellow DOWN
    } else {
      pixels.setPixelColor(mIdx(2, 3), pixels.Color(22, 16, 0));   // Dimly lit standby
    }

    // 3. Pixels 7 & 12 (Rows 1 & 2, Middle): Strobe on Button SW2, breathing standby glow
    if (btn_pressed) {
      // Brilliant Magenta / White strobe ONLY when Button SW2 is pressed!
      pixels.setPixelColor(mIdx(2, 1), pixels.Color(255, 255, 255));
      pixels.setPixelColor(mIdx(2, 2), pixels.Color(255, 0, 180));
    } else {
      // Soft breathing purple glow in standby
      static uint8_t pulse = 0;
      pulse += 8;
      uint8_t p = (pulse > 127) ? (255 - pulse) : pulse;
      uint8_t glow = 15 + (p >> 3);
      pixels.setPixelColor(mIdx(2, 1), pixels.Color(glow, 0, glow));
      pixels.setPixelColor(mIdx(2, 2), pixels.Color(glow, 0, glow));
    }
  }

  // Update display (disable interrupts during WS2812 bitbang)
  cli();
  pixels.show();
  sei();

  delay(20); // ~50 FPS responsive UI
}
