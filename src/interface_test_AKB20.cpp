/* ---------------------------------------------------------------------------------------
    ABK-20 HARDWARE & INTERFACE TEST
    Comprehensive visual & audio diagnostics for the ABK-20 Synthesizer

    Inputs Tested:
      1. Rotary Potentiometer POT2 (PB4 / A2):
         - Visual: Columns 0 & 1 dual vertical bar graph (Green -> Cyan)
         - Audio:  Continuous pitch sweep (~80 Hz to 880 Hz)
      2. Slide Fader RV1 (PB2 / A1):
         - Visual: Columns 3 & 4 dual vertical bar graph (Amber -> Red)
         - Audio:  Waveform Timbre (Triangle -> Sawtooth -> Pulse PWM)
      3. SPDT Toggle Switch SW3 (PB3 / A3):
         - Visual: Center Column top pixel (UP) / bottom pixel (DOWN)
         - Audio:  
             - UP:     Fast Major Acid Arpeggio (Root -> 3rd -> 5th -> Octave)
             - DOWN:   Sub-Bass Heavy Drone (-1 Octave Sub-Oscillator)
             - CENTER: Pure Continuous Lead Tone
      4. Button 3 SW4 (PB3 / A3):
         - Visual: Center Column middle pixels flash brilliant White / Magenta
         - Audio:  Instant +1 Octave Boost, High Resonance Grit, & Click Chirp
 --------------------------------------------------------------------------------------- */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// --- Hardware Pin Definitions ---
#define NUMPIXELS   20
#define NEOPIXELPIN 0  // PB0: NeoPixel Matrix
#define SPEAKERPIN  1  // PB1: High-Speed PWM Audio (OC1A)
#define BUTTON_PIN  A3 // PB3: Analog Ladder (Toggle + Button 3)
#define POTI_LEFT   A2 // PB4: Rotary Pot (Filter/Pitch)
#define POTI_RIGHT  A1 // PB2: Slide Fader (Timbre/Wave)

// --- PB3 Button Ladder States ---
#define BTN_STATE_IDLE        0 // Center toggle, no button (> 450, nominal ~536)
#define BTN_STATE_TOGGLE_UP   1 // Toggle UP (320-420, nominal ~356)
#define BTN_STATE_TOGGLE_DOWN 2 // Toggle DOWN (240-310, nominal ~278)
#define BTN_STATE_BTN3_SOLO   3 // Button 3 solo (185-235, nominal ~205)
#define BTN_STATE_BTN3_UP     4 // Button 3 + Toggle UP (155-180, nominal ~170)
#define BTN_STATE_BTN3_DOWN   5 // Button 3 + Toggle DOWN (< 155, nominal ~147)

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Synth Engine State ---
volatile uint16_t phase_acc = 0;
volatile uint16_t phase_inc = 200;
volatile uint8_t  synth_mode = 0;       // 0=Normal, 1=Arp (Up), 2=Bunny Hop (Down)
volatile uint8_t  synth_wave = 0;       // 0=Tri, 1=Saw, 2=Pulse
volatile uint8_t  pulse_width = 128;
volatile uint8_t  fader_val = 128;      // Slide fader value passed to ISR
volatile uint8_t  hop_vis = 0;          // Bouncing hop level for visual indicator (0..3)
volatile uint8_t  hop_beat_env = 0;     // Beat envelope (240..0) for full-matrix pulse
volatile uint8_t  btn3_held = 0;
volatile uint8_t  click_countdown = 0;

// Arpeggio multipliers (1.0, 1.25, 1.5, 2.0 mapped to integers)
const uint8_t arp_mult[4] = { 4, 5, 6, 8 }; // Divide by 4: 1x, 1.25x, 1.5x, 2x

void setup() {
  pinMode(SPEAKERPIN, OUTPUT);

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
  GTCCR = 0; // Ensure PB4 is free for analog input (POTI_LEFT)

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
  
  if (synth_mode == 1) { // Arpeggio mode (Toggle UP)
    uint8_t step = (arp_tick >> 9) & 3; // Change note every ~32ms
    cur_inc = (cur_inc * arp_mult[step]) >> 2;
  }

  // --- Bunny Hop Engine (Toggle DOWN: synth_mode == 2) ---
  static uint16_t hop_timer = 0;
  static uint8_t  hop_env = 0;

  if (synth_mode == 2) {
    hop_timer++;
    // Hop rate controlled by slide fader: slow lazy boings (fader down) to rapid bunny flutter (fader up)
    uint16_t hop_interval = 3200 - (fader_val * 9); // ~200ms down to ~60ms
    if (hop_timer >= hop_interval) {
      hop_timer = 0;
      hop_env = 240; // Trigger new cartoon boing hop!
    }
    if ((hop_timer & 7) == 0 && hop_env > 2) {
      hop_env -= 3; // Snappy bouncy decay
    }
    hop_vis = hop_env >> 6; // 0..3 for matrix visual bounce
    hop_beat_env = hop_env; // Full-resolution envelope for matrix strobe pulse

    // Bouncy cartoon pitch curve: leaps up on hop and slides down to root
    cur_inc += (cur_inc * hop_env) >> 7;
  }

  if (btn3_held) {
    if (synth_mode == 2) cur_inc <<= 2; // Mega Kangaroo Squeak (+2 Octaves!)
    else cur_inc <<= 1;                 // Transpose +1 Octave up
  }

  phase_acc += cur_inc;
  uint8_t saw = phase_acc >> 8;
  int16_t sample = 0;

  if (synth_mode == 2) {
    // Springy resonant rubber boing waveform
    uint8_t boing = (saw * hop_env) >> 8;
    saw = (saw + boing) ^ (boing >> 1);
    sample = saw;
  } else {
    // Waveform generation based on slide fader
    if (synth_wave == 0) {
      // Triangle wave
      sample = (saw < 128) ? (saw << 1) : ((255 - saw) << 1);
    } else if (synth_wave == 1) {
      // Sawtooth wave
      sample = saw;
    } else {
      // Pulse wave with variable pulse width
      sample = (saw > pulse_width) ? 220 : 35;
    }
  }

  // Button 3 overdrive/clip
  if (btn3_held) {
    if (sample > 200) sample = 255;
    else if (sample < 55) sample = 0;
  }

  OCR1A = (uint8_t)sample;
}

// Rotary knob (POT2): CW (turn up) = GND (~0), CCW (turn down) = 4.7k (~670) -> Inverted
static inline uint8_t readRotary(uint8_t pin) {
  int16_t raw = analogRead(pin);
  const int16_t ADC_TOP = 670;
  const int16_t ADC_BOT = 15;
  if (raw > ADC_TOP) raw = ADC_TOP;
  if (raw < ADC_BOT) raw = ADC_BOT;
  uint16_t diff = ADC_TOP - raw;
  uint16_t val = (diff * 100) >> 8;
  if (val > 255) val = 255;
  return (uint8_t)val;
}

// Slide fader (RV1): Slider UP = 4.7k (~670), Slider DOWN = GND (~0) -> Direct (non-inverted)
static inline uint8_t readSlider(uint8_t pin) {
  int16_t raw = analogRead(pin);
  const int16_t ADC_TOP = 670;
  const int16_t ADC_BOT = 15;
  if (raw > ADC_TOP) raw = ADC_TOP;
  if (raw < ADC_BOT) raw = ADC_BOT;
  uint16_t diff = raw - ADC_BOT;
  uint16_t val = (diff * 100) >> 8;
  if (val > 255) val = 255;
  return (uint8_t)val;
}

// PB3 Resistor Ladder Decoder with 3-sample oversampling and 2-frame debouncing
// to eliminate ground bounce noise spikes and toggle flickering
static inline uint8_t readButtonState() {
  uint16_t s1 = analogRead(BUTTON_PIN);
  uint16_t s2 = analogRead(BUTTON_PIN);
  uint16_t s3 = analogRead(BUTTON_PIN);
  uint16_t adc = (s1 + s2 + s3) / 3;

  uint8_t raw_state;
  if (adc > 445) raw_state = BTN_STATE_IDLE;
  else if (adc >= 315) raw_state = BTN_STATE_TOGGLE_DOWN; // 1.74V (ADC ~356) = Physical DOWN
  else if (adc >= 240) raw_state = BTN_STATE_TOGGLE_UP;   // 1.36V (ADC ~278) = Physical UP
  else if (adc >= 185) raw_state = BTN_STATE_BTN3_SOLO;   // 1.00V (ADC ~205) = Button 3 solo
  else if (adc >= 155) raw_state = BTN_STATE_BTN3_DOWN;   // 0.83V (ADC ~170) = Button 3 + DOWN
  else raw_state = BTN_STATE_BTN3_UP;                     // 0.72V (ADC ~147) = Button 3 + UP

  static uint8_t debounced_state = BTN_STATE_IDLE;
  static uint8_t candidate_state = BTN_STATE_IDLE;
  static uint8_t candidate_count = 0;

  if (raw_state == candidate_state) {
    if (++candidate_count >= 2) {
      debounced_state = raw_state;
    }
  } else {
    candidate_state = raw_state;
    candidate_count = 1;
  }

  return debounced_state;
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
  // 1. Read Inputs
  uint8_t left_pot  = readRotary(POTI_LEFT);  // 0 to 255 (Rotary Knob, inverted)
  uint8_t right_pot = readSlider(POTI_RIGHT); // 0 to 255 (Slide Fader, direct)
  uint8_t btn_state = readButtonState();

  static uint8_t prev_btn_state = 0xFF;
  if (btn_state != prev_btn_state) {
    click_countdown = 20; // Trigger instant audio chirp on transition
    prev_btn_state = btn_state;
  }

  bool btn3 = (btn_state >= BTN_STATE_BTN3_SOLO);
  btn3_held = btn3;

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
  if (btn_state == BTN_STATE_TOGGLE_UP || btn_state == BTN_STATE_BTN3_UP) {
    synth_mode = 1; // Arpeggio mode
  } else if (btn_state == BTN_STATE_TOGGLE_DOWN || btn_state == BTN_STATE_BTN3_DOWN) {
    synth_mode = 2; // Bunny Hop mode
  } else {
    synth_mode = 0; // Normal continuous tone
  }

  // 3. Render 5x4 Matrix Display
  pixels.clear();

  // SPECIAL FEATURE: When Toggle is DOWN and Button 3 is pressed,
  // the entire matrix illuminates in a unified color that pulses in the beat!
  // Pot 1 (left_pot) sweeps smoothly through all rainbow colors.
  if (btn_state == BTN_STATE_BTN3_DOWN) {
    uint8_t beat_bright = 45 + ((hop_beat_env * 210) >> 8); // Pulses 45 -> 255 on each hop beat
    uint32_t col = wheel(left_pot, beat_bright);
    for (uint8_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, col);
    }
  } else {
    // --- Normal Interface Display (Meters + Dedicated Indicators) ---
    // --- Left Pot Bar Graph (Columns 0 & 1): 4 vertical steps ---
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

    // --- Right Fader Bar Graph (Columns 3 & 4): 4 vertical steps ---
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

    // --- Center Column 2: Status Indicators with Elegant Dim Standby Backlight ---
    // 1. Pixel 2 (Row 0, Top): Dedicated Toggle UP (Bright Cyan active / Dim Cyan standby)
    if (btn_state == BTN_STATE_TOGGLE_UP || btn_state == BTN_STATE_BTN3_UP) {
      pixels.setPixelColor(mIdx(2, 0), pixels.Color(0, 255, 255)); // Bright Cyan UP
    } else {
      pixels.setPixelColor(mIdx(2, 0), pixels.Color(0, 18, 18));   // Dimly lit standby
    }

    // 2. Pixel 17 (Row 3, Bottom): Dedicated Toggle DOWN (Bright Yellow active / Dim Amber standby)
    if (btn_state == BTN_STATE_TOGGLE_DOWN) {
      pixels.setPixelColor(mIdx(2, 3), pixels.Color(255, 200, 0)); // Bright Yellow DOWN
    } else {
      pixels.setPixelColor(mIdx(2, 3), pixels.Color(22, 16, 0));   // Dimly lit standby
    }

    // 3. Pixels 7 & 12 (Rows 1 & 2, Middle): Always softly lit, strobe on Button 3
    if (btn3) {
      // Brilliant Magenta / White strobe ONLY when Button 3 is physically pressed!
      pixels.setPixelColor(mIdx(2, 1), pixels.Color(255, 255, 255));
      pixels.setPixelColor(mIdx(2, 2), pixels.Color(255, 0, 180));
    } else {
      // Always softly/dimly lit with gentle breathing purple glow
      static uint8_t pulse = 0;
      pulse += 8;
      uint8_t p = (pulse > 127) ? (255 - pulse) : pulse;
      uint8_t glow = 15 + (p >> 3); // 15 to 30 brightness (soft dim standby)
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

