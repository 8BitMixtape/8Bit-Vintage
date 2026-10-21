/* ---------------------------------------------------------------------------------------
        ATtiny85 ACID SYNTHESIZER - ABK-20 HARDWARE EDITION
        Outputs mixed Bass (303 SVF) + Drums (Kick & Hat) on PB1 (OC1A)
        Hardware mapping:
          PB0: 20x NeoPixel 5x4 matrix
          PB1: Audio PWM out (Timer1 PLL @ 250 kHz carrier)
          PB2: Slide Fader RV1 (Pattern Selector, 4.7k series resistor)
          PB3: Analog Resistor Ladder (Toggle switch & Button 3)
          PB4: Rotary Pot POT2 (Filter Cutoff, 4.7k series resistor)
---------------------------------------------------------------------------------------
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// --- Hardware Definitions ---
#define NUMPIXELS 20
#define NEOPIXELPIN 0 // PB0
#define SPEAKERPIN 1  // PB1 (OC1A)
#define BUTTON_PIN A3 // PB3 (ADC3)
#define POTI_LEFT  A2 // PB4 (ADC2, Rotary Knob)
#define POTI_RIGHT A1 // PB2 (ADC1, Slide Fader)

// --- PB3 Analog Resistor Ladder Decoding ---
// Calibration measured at VCC = 5.0V:
//   Idle (Center toggle, no btn): 2.62V -> ADC ~536
//   Toggle UP:                   1.74V -> ADC ~356
//   Toggle DOWN:                 1.36V -> ADC ~278
//   Button 3 (SW4) solo:         1.00V -> ADC ~205
//   Button 3 + Toggle UP:        0.83V -> ADC ~170
//   Button 3 + Toggle DOWN:      0.72V -> ADC ~147
#define BTN_STATE_IDLE        0
#define BTN_STATE_TOGGLE_UP   1
#define BTN_STATE_TOGGLE_DOWN 2
#define BTN_STATE_BTN3_SOLO   3
#define BTN_STATE_BTN3_UP     4
#define BTN_STATE_BTN3_DOWN   5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Synth Variables ---
volatile uint16_t bass_phase = 0;
volatile uint16_t bass_pitch = 0;
volatile uint16_t bass_target_pitch = 0;
volatile uint8_t  bass_env = 0;
volatile uint8_t  filter_mod = 0;

volatile uint16_t drum_phase = 0;
volatile uint16_t drum_pitch = 0;
volatile uint8_t  drum_env = 0;
volatile uint8_t  drum_type = 0; // 0=kick, 1=hat

uint16_t lfsr = 0xACE1;
volatile uint8_t is_playing = 0; // Global flag for interrupt

void setup() {
  pinMode(SPEAKERPIN, OUTPUT);

  pixels.begin();
  pixels.setBrightness(80); // Clear, vibrant matrix without excessive rail noise
  
  // High-Speed PWM on Timer1 for PB1 (OC1A)
  PLLCSR |= (1 << PLLE);               
  delayMicroseconds(100);
  while (!(PLLCSR & (1 << PLOCK)));    
  PLLCSR |= (1 << PCKE);               

  // Enable PWM on A (PB1), Clear on match, Prescaler 8 (31.25 kHz)
  TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS12); 
  GTCCR = 0; // Disable Timer1 B output on PB4 completely!
  
  TIMSK |= (1 << TOIE1);
  sei();
}

ISR(TIMER1_OVF_vect) {
  static uint8_t math_div = 0;
  math_div = !math_div;
  if (math_div) return; // Only process heavy math on even interrupts (15.6 kHz) to save CPU!

  static uint8_t tick = 0;
  tick++;
  
  // Random noise generator (LFSR)
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  if (!is_playing) {
    OCR1A = 128; // Silence
    return;
  }

  // --- Bassline Envelope & Slides ---
  if ((tick & 7) == 0) { // Every ~0.5ms
    if (bass_env > 1) bass_env -= 2; else bass_env = 0;
    
    if (drum_type == 0) {
      if (drum_env > 0) drum_env -= 1; // Kick decay
    } else {
      if (drum_env > 7) drum_env -= 8; else drum_env = 0; // Hat decay
    }
  }
  
  if ((tick & 15) == 0) { // Every ~1ms
    if (bass_pitch < bass_target_pitch) bass_pitch += 2; // Slide twice as fast per tick
    else if (bass_pitch > bass_target_pitch) bass_pitch -= 2;
  }

  // --- Bassline Synthesis ---
  static int16_t filter_val = 0;
  static int16_t filter_vel = 0;
  
  bass_phase += (bass_pitch << 1); // DOUBLE PITCH increment since running half as fast!
  int16_t raw_wave = (bass_phase >> 8) - 128; // Sawtooth
  
  // Filter Cutoff: scaled envelope modulation so low-end gets ultra dark & sub-heavy
  int16_t env_mod = ((uint16_t)bass_env * (filter_mod + 25)) >> 8;
  int16_t cutoff = filter_mod + env_mod;
  if (cutoff < 4) cutoff = 4;
  if (cutoff > 215) cutoff = 215;
  
  int16_t diff = raw_wave - filter_val;
  
  // SVF math (using 16-bit to save CPU cycles inside the interrupt!)
  int16_t drive = (diff * (cutoff >> 1)) >> 7;
  filter_vel += drive;
  
  // Resonance dampening
  filter_vel -= (filter_vel * 25) >> 8; 
  filter_val += filter_vel;
  
  // Clip
  if (filter_val > 300) filter_val = 300;
  if (filter_val < -300) filter_val = -300;
  
  int16_t outA = filter_val + 128;
  if (outA < 0) outA = 0;
  if (outA > 255) outA = 255;

  // --- Drum Synthesis ---
  uint8_t outB = 0;
  if (drum_type == 0) { // Kick
    drum_phase += (drum_pitch << 1); // DOUBLE PITCH increment
    if (drum_pitch > 30) drum_pitch -= 2; // Decay pitch twice as fast per tick
    
    uint8_t kick_wave = drum_phase >> 8;
    if (drum_phase & 0x8000) kick_wave = 255 - kick_wave; // Triangle
    outB = (kick_wave * drum_env) >> 8;
  } else { // Hat
    outB = ((lfsr & 0xFF) * drum_env) >> 8;
  }

  // --- MIXING (Mono on PB1) ---
  int16_t sA = (int16_t)outA - 128; // -128 to +127
  int16_t sB = (int16_t)outB;       // 0 to 255

  // Mix bass and drum
  int16_t mixed = sA + sB; 
  
  // Hard clip
  if (mixed > 127) mixed = 127;
  if (mixed < -128) mixed = -128;
  
  // Convert back to 0-255 centered at 128
  OCR1A = (uint8_t)(mixed + 128);
}

// Frequencies for acid bassline
const uint16_t P_C2  = 137;
const uint16_t P_Eb2 = 163;
const uint16_t P_F2  = 183;
const uint16_t P_G2  = 205;
const uint16_t P_Bb2 = 244;
const uint16_t P_C3  = 274;
const uint16_t P_D3  = 308;
const uint16_t P_Eb3 = 326;
const uint16_t P_F3  = 366;
const uint16_t P_C4  = 548;
const uint16_t P_Eb4 = 652;

const uint16_t bass_seq_pitches[4][16] PROGMEM = {
  { P_C2, P_C3, P_C2, P_C2, P_C3, P_C2, P_G2, P_Bb2, P_C2, P_Eb3, P_C2, P_F2, P_C3, P_C2, P_Eb3, P_C4 }, // 0: Classic Acid
  { P_C2, P_Eb2, P_C2, P_F2, P_C2, P_G2, P_C2, P_Bb2, P_C3, P_C2, P_Eb3, P_C2, P_F3, P_C2, P_C4, P_Eb4 }, // 1: Arpeggio
  { P_G2, P_G2, P_Bb2, P_G2, P_C3, P_G2, P_Eb3, P_F3, P_G2, P_G2, P_Bb2, P_G2, P_Eb3, P_D3, P_C3, P_Bb2 }, // 2: G Minor Groove
  { P_C2, P_C2, P_C3, P_C3, P_C4, P_C4, P_Eb3, P_F3, P_C2, P_C2, P_Bb2, P_Bb2, P_C3, P_Eb3, P_C4, P_C4 } // 3: Octave Jumps
};

const uint8_t bass_seq_slides[4][16] PROGMEM = { 
  { 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
  { 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 }
};

const uint8_t bass_seq_accents[4][16] PROGMEM = { 
  { 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1 },
  { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1 },
  { 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 },
  { 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1 }
};

const uint8_t drum_seq[4][16] PROGMEM = {
  { 1, 0, 2, 0, 1, 0, 2, 0, 1, 2, 2, 1, 1, 0, 2, 2 }, // 0: Classic 4-on-the-floor
  { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 }, // 1: Fast Techno
  { 1, 0, 2, 0, 0, 0, 1, 0, 1, 0, 2, 0, 0, 2, 2, 0 }, // 2: Breakbeat
  { 1, 0, 0, 0, 2, 0, 0, 0, 1, 0, 1, 0, 2, 0, 2, 2 }  // 3: Electro
}; // 1 = Kick, 2 = Hat, 0 = Rest

void drawIndicator(uint8_t indicator_y) {
  const uint8_t rightColIndices[4] = {4, 9, 14, 19};
  for(uint8_t i=0; i<4; i++) {
    pixels.setPixelColor(rightColIndices[i], pixels.Color(20, 0, 0));
  }
  pixels.setPixelColor(rightColIndices[indicator_y], pixels.Color(255, 255, 255));
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

// Decode PB3 analog resistor ladder with 3-sample oversampling and 2-frame debouncing
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

static inline bool isButton3Pressed(uint8_t state) {
  return (state >= BTN_STATE_BTN3_SOLO); // Any state with Button 3 pressed (adc < 235)
}

void loop() {
  uint8_t btn_state = readButtonState();
  
  if (btn_state != BTN_STATE_IDLE) { // Playing! (Toggle UP/DOWN or Button 3 pressed)
    is_playing = 1;
    
    static uint8_t step = 0;
    
    // Read Potentiometers (calibrated for 4.7k series resistor)
    uint8_t raw_p1 = readRotary(POTI_LEFT);  // 0 to 255
    // Quadratic audio-taper curve: gives expansive, extreme low-end range
    filter_mod = ((uint16_t)raw_p1 * raw_p1) >> 8;
    
    uint8_t p2 = readSlider(POTI_RIGHT); // Pattern mapping: 0 to 3
    uint8_t current_pattern = p2 >> 6; // 0 to 3
    if (current_pattern > 3) current_pattern = 3;
    
    // Tempo control via Toggle switch:
    //   Toggle DOWN: Fast Techno (90ms)
    //   Toggle UP or Button 3 solo: Classic Acid (115ms)
    int tempo_delay = (btn_state == BTN_STATE_TOGGLE_DOWN || btn_state == BTN_STATE_BTN3_DOWN) ? 90 : 115;
    
    bool btn3 = isButton3Pressed(btn_state);
    
    // Trigger Drum (read from PROGMEM!)
    uint8_t d = pgm_read_byte(&drum_seq[current_pattern][step]);
    if (d == 1) {
      drum_type = 0;
      drum_pitch = 1000; 
      drum_env = 255;
    } else if (d == 2) {
      drum_type = 1;
      drum_env = 120; 
    }
    
    // Trigger Bass (read from PROGMEM!)
    uint16_t next_pitch = pgm_read_word(&bass_seq_pitches[current_pattern][step]);
    
    // --- BUTTON 3 FEATURE: +1 Octave Shift & Extreme Acid Scream ---
    if (btn3) {
      next_pitch <<= 1; // Double frequency: transpose up by 1 full octave!
      bass_target_pitch = next_pitch;
      bass_pitch = next_pitch;
      bass_env = 255;   // Maximum accent resonance punch!
    } else {
      if (pgm_read_byte(&bass_seq_slides[current_pattern][step])) {
        bass_target_pitch = next_pitch; // Slide smoothly
      } else {
        bass_pitch = next_pitch;
        bass_target_pitch = next_pitch;
        bass_env = pgm_read_byte(&bass_seq_accents[current_pattern][step]) ? 255 : 100; // Accents hit harder
      }
    }
    
    // Visuals!
    uint8_t bass_vis = bass_env >> 1; // Red for bass
    uint8_t drum_vis = drum_env >> 1; // White for drums
    
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      if (btn3) {
        // Neon Acid Lime Flash when Button 3 octave boost is pressed!
        pixels.setPixelColor(i, pixels.Color(bass_vis, 255, bass_vis >> 1));
      } else {
        pixels.setPixelColor(i, pixels.Color(bass_vis + drum_vis, drum_vis, drum_vis)); 
      }
    }
    drawIndicator(current_pattern); // Show pattern selection on the right strip (0-3)
    cli(); pixels.show(); sei(); // Disable interrupts during NeoPixel bit-bang!
    
    step = (step + 1) & 15;
    delay(tempo_delay);
    
  } else {
    // Idle Mode (Toggle Center, no buttons pressed)
    is_playing = 0;
    
    uint8_t p1 = readRotary(POTI_LEFT);  // 0-255
    uint8_t p2 = readSlider(POTI_RIGHT); // 0-255
    
    uint8_t val = (millis() / 20) % 255;
    if (val > 127) val = 255 - val; // 0 to 127
    if (val < 15) val = 15;
    
    uint8_t r = (p1 * val) >> 7;
    uint8_t g = (p2 * val) >> 7;
    uint8_t b = ((255 - p1) * val) >> 7;
    
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(r, g, b)); 
    }
    drawIndicator(p2 >> 6); // Show pattern selection in idle
    cli(); pixels.show(); sei(); // Disable interrupts during NeoPixel bit-bang!
    delay(10);
  }
}

