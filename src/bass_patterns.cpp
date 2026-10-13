#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>

// --- Configuration ---
// Uncomment the following line to use Square Wave, comment out for Sine Wave
#define USE_SQUARE_WAVE 1 

// --- Hardware Definitions ---
#define NUMPIXELS 20
#define NEOPIXELPIN 0
#define SPEAKERPIN 1
#define BUTTON_PIN A3
#define POTI_LEFT  A2
#define POTI_RIGHT A1

// Button Calibration & States
#define BUTTON_NONE         0
#define BUTTON_LEFT         1
#define BUTTON_RIGHT        2

#define Vbutton_releaseLevel  490
#define Vbutton_left          470
#define Vbutton_right         400
#define Vbutton_both          330
#define Vbutton_pressedLevel  Vbutton_left 

#define Vcc                    37 // 3.7 V for LiPo
#define Vdiv                   26 // measure max Voltage on Analog In

#define BUTTON_NOTPRESSED   0
#define BUTTON_PRESSED      1

// --- Matrix Definitions ---
#define MATRIX_W 5
#define MATRIX_H 4
#define MATRIX_SERPENTINE 0   // 1 = serpentine wiring per row, 0 = progressive
#define MATRIX_ORIGIN_TOPLEFT 1 // assume pixel 0 is top-left

// --- NeoPixel Object ---
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Helpers ---
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

uint32_t colorWheel(uint8_t colorNumber) {
   return Wheel(colorNumber);
}

void setColorAllPixel(uint32_t color) {
  for (uint8_t n = 0; n < NUMPIXELS; n++) {
    pixels.setPixelColor(n, color);
  }
}

int matrixIndex(uint8_t x, uint8_t y) {
  if (x >= MATRIX_W || y >= MATRIX_H) return -1;
  uint8_t row = y;
  uint8_t col = x;
  if (!MATRIX_ORIGIN_TOPLEFT) {
    row = (MATRIX_H - 1) - y;
  }
  if (MATRIX_SERPENTINE) {
    if (row % 2 == 0) return row * MATRIX_W + col;
    else return row * MATRIX_W + (MATRIX_W - 1 - col);
  } else {
    return row * MATRIX_W + col;
  }
}

void matrixSet(uint8_t x, uint8_t y, uint32_t color) {
  int idx = matrixIndex(x, y);
  if (idx >= 0 && idx < NUMPIXELS) {
    pixels.setPixelColor(idx, color);
  }
}

// --- Sine Wavetable ---
#ifndef USE_SQUARE_WAVE
const byte sine256[] PROGMEM = {
  0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 128,
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246,
  248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211,
  208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65,
  62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0
};
#endif

// --- Synthesis Variables ---
volatile uint16_t phase = 0;
volatile uint16_t phase_increment = 0;
volatile uint8_t amplitude = 0;
volatile bool trigger_kick = false;
volatile uint16_t ticks = 0;

// Noise / Snare Variables
volatile uint16_t lfsr = 1;
volatile uint8_t noise_amp = 0;
volatile bool trigger_noise = false;
volatile uint8_t noise_decay_rate = 10; // Adjust for length of snare

// Parameters for the kick drum
volatile uint16_t kick_start_freq = 1500;
volatile uint16_t kick_end_freq = 350;
volatile uint8_t  kick_pitch_decay = 2;
volatile uint8_t  kick_amp_decay = 8;

// Sequencer Speed
// const uint16_t BPM = 120; // controlled by pot now
uint16_t STEP_MS = 125;      // 16th notes (125ms at 120 BPM)
const uint16_t FLASH_MS = 25;               // LED Flash duration

struct KickStep {
  bool active;
  uint16_t start_freq;
  uint16_t end_freq;
  uint8_t pitch_decay;
  uint8_t amp_decay;
};

struct SnareStep {
  bool active;
  uint8_t decay_rate;
};

KickStep kickPattern[16];
SnareStep snarePattern[16];
uint8_t current_step = 0;

uint16_t analogReadScaled(uint8_t channel) {
  uint32_t value = analogRead(channel);
  value = value * Vcc / Vdiv;
  if (value > 1023) value = 1023;
  return (uint16_t)value;
}

uint8_t wasButtonPressed()
{
  static uint8_t buttonPressed    = false;
  static uint8_t buttonState      = 0;
  static uint8_t buttonValue      = BUTTON_NONE;
  static uint8_t buttonMaxValue   = 0;

  uint8_t        buttonReturnValue;  
  uint16_t       pinVoltage;
    
  pinVoltage = analogRead( BUTTON_PIN );
  
  // hysteresis switch
  if( pinVoltage > Vbutton_releaseLevel ) buttonPressed = false;
  if( pinVoltage < Vbutton_pressedLevel ) buttonPressed = true;

  buttonReturnValue = BUTTON_NONE;
  
  switch( buttonState )
  {
    case BUTTON_NOTPRESSED:
    {
      buttonMaxValue = 0;      
      
      if( buttonPressed )
      {      
        buttonState = BUTTON_PRESSED;
      }
    };break;
    
    case BUTTON_PRESSED:
    {
      if( buttonPressed ) // find minimum volage level during button pressed period
      {
        buttonValue = BUTTON_NONE; 
        
             if ( pinVoltage < Vbutton_both  ) buttonValue = BUTTON_LEFT + BUTTON_RIGHT;
        else if ( pinVoltage < Vbutton_right ) buttonValue =               BUTTON_RIGHT;
        else if ( pinVoltage < Vbutton_left  ) buttonValue = BUTTON_LEFT               ;      
        
        if( buttonValue > buttonMaxValue ) buttonMaxValue = buttonValue;   
                                     
      }else
      {
        buttonState = BUTTON_NOTPRESSED;
        buttonReturnValue = buttonMaxValue; 
      }
      ;break;

    }
  }
  
  return buttonReturnValue; 
}

uint8_t current_pattern_id = 0;

void generatePatterns() {
  for (int i = 0; i < 16; i++) {
    kickPattern[i].active = false;
    snarePattern[i].active = false;
    
    // Default synth parameters
    kickPattern[i].start_freq = 900;
    kickPattern[i].end_freq = 150;
    kickPattern[i].pitch_decay = 8;
    kickPattern[i].amp_decay = 10;
    snarePattern[i].decay_rate = 10;
  }

  if (current_pattern_id == 0) {
    // 0: Techno (4 on the floor)
    kickPattern[0].active = true;
    kickPattern[4].active = true;
    kickPattern[8].active = true;
    kickPattern[12].active = true;
    
    snarePattern[4].active = true;
    snarePattern[12].active = true;
  }
  else if (current_pattern_id == 1) {
    // 1: Breaks
    kickPattern[0].active = true;
    kickPattern[5].active = true;
    kickPattern[10].active = true;
    
    snarePattern[4].active = true;
    snarePattern[12].active = true;
    snarePattern[14].active = true; // ghost snare
  }
  else if (current_pattern_id == 2) {
    // 2: Dembow / Reggaeton
    kickPattern[0].active = true;
    kickPattern[4].active = true;
    kickPattern[8].active = true;
    kickPattern[12].active = true;
    
    snarePattern[3].active = true;
    snarePattern[6].active = true;
    snarePattern[11].active = true;
    snarePattern[14].active = true;
  }
  else if (current_pattern_id == 3) {
    // 3: Trap
    kickPattern[0].active = true;
    kickPattern[8].active = true;
    kickPattern[11].active = true;
    
    snarePattern[4].active = true;
    snarePattern[12].active = true;
    snarePattern[14].active = true;
    snarePattern[15].active = true;
  }
  else if (current_pattern_id == 4) {
    // 4: UK Garage / 2-step
    kickPattern[0].active = true;
    kickPattern[7].active = true;
    kickPattern[14].active = true;
    
    snarePattern[4].active = true;
    snarePattern[12].active = true;
  }
  else if (current_pattern_id == 5) {
    // 5: Electro
    kickPattern[0].active = true;
    kickPattern[3].active = true;
    kickPattern[8].active = true;
    
    snarePattern[4].active = true;
    snarePattern[12].active = true;
  }
  else if (current_pattern_id == 6) {
    // 6: Random
    for (int i = 0; i < 16; i++) {
      kickPattern[i].active = (i % 4 == 0) || (random(0, 4) == 0);
      kickPattern[i].start_freq = random(800, 1000);
      kickPattern[i].end_freq = random(100, 200);
      kickPattern[i].pitch_decay = random(6, 10);
      kickPattern[i].amp_decay = random(7, 15);
  
      snarePattern[i].active = (i == 4) || (i == 12) || (random(0, 5) == 0);
      snarePattern[i].decay_rate = random(5, 15); 
    }
  }
}

void setup() {
  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  OSCCAL += 3; // Oscillator calibration

  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                          
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;       
  pinMode(SPEAKERPIN, OUTPUT);                                 

  // Set up Timer/Counter0 for interrupt (10kHz)
  TCCR0A = 3 << WGM00;                                
  TCCR0B = 1 << WGM02 | 2 << CS00;                    
  TIMSK = 1 << OCIE0A;                                
  OCR0A = 199;                                         
  pinMode(NEOPIXELPIN, OUTPUT);
  
  pixels.begin();
  pixels.setBrightness(255);
  randomSeed(analogRead(A1));

  generatePatterns();
}

// Custom delay using ISR ticks (10kHz) because standard delay() is broken by Timer0 usage
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

void loop() {
  // Read Speed Potentiometer (Right Pot)
  uint16_t potVal = analogRead(POTI_RIGHT);
  // Map potVal (0-1023) to BPM (40 to 600)
  uint16_t bpm = map(potVal, 0, 1023, 600, 40); 
  
  // Calculate 16th note step duration in ms: 60000ms / (BPM * 4) = 15000 / BPM
  STEP_MS = 15000 / bpm; 
  if (STEP_MS < 26) STEP_MS = 26; // safety clamp for FLASH_MS

  // Read Bass Modifier Potentiometer (Left Pot)
  uint16_t potLeft = analogRead(POTI_LEFT);
  
  // Piece-wise mapping: 50% of the physical dial is completely dedicated to fine-tuning the low-bass.
  // The upper 50% morphs it into high-pitched lasers.
  int16_t sf_mod, ef_mod, pd_mod, ad_mod;
  
  if (potLeft < 512) {
    // LOWER HALF: Deep sub-bass tuning
    // Reduced the extreme ranges so the dial feels smoother and much more musical
    sf_mod = map(potLeft, 0, 511, -400, 0);    // Start freq drops gently, not all the way to zero
    ef_mod = map(potLeft, 0, 511, -70, 0);     // End freq stays punchy
    pd_mod = map(potLeft, 0, 511, -4, 0);      // Pitch decay slows slightly
    ad_mod = map(potLeft, 0, 511, -2, 6);      // Length ranges from tight kick to smooth 808, avoiding clicks
  } else {
    // UPPER HALF: Laser zaps and toms
    sf_mod = map(potLeft, 512, 1023, 0, 8000); 
    ef_mod = map(potLeft, 512, 1023, 0, 1500); 
    pd_mod = map(potLeft, 512, 1023, 0, 5);    
    ad_mod = map(potLeft, 512, 1023, 6, -5);  // Blend from smooth into tight zap
  }

  // Check Buttons
  uint8_t button = wasButtonPressed();
  
  if (button == BUTTON_RIGHT) {
    // Button Down: cycle through predefined rhythms (0 to 5)
    if (current_pattern_id >= 5) {
      current_pattern_id = 0;
    } else {
      current_pattern_id++;
    }
    generatePatterns();
    
    // Visual feedback (Green = Predefined cycle)
    setColorAllPixel(pixels.Color(0, 255, 0)); 
    pixels.show();
    my_delay(500);
  }
  else if (button == BUTTON_LEFT) {
    // Button Up: generate a new random pattern
    current_pattern_id = 6;
    generatePatterns();
    
    // Visual feedback (Blue/Purple = Randomizer)
    setColorAllPixel(pixels.Color(128, 0, 255)); 
    pixels.show();
    my_delay(500);
  }

  KickStep &k = kickPattern[current_step];
  SnareStep &s = snarePattern[current_step];

  // Prepare synthesis triggers
  if (k.active) {
    int16_t sf = k.start_freq + sf_mod;
    kick_start_freq = (sf < 40) ? 40 : sf; // Deep sub bass floor
    
    int16_t ef = k.end_freq + ef_mod;
    kick_end_freq = (ef < 20) ? 20 : ef;   // Deepest resting pitch
    
    int16_t pd = k.pitch_decay + pd_mod;
    kick_pitch_decay = (pd < 1) ? 1 : pd;
    
    int16_t ad = k.amp_decay + ad_mod;
    kick_amp_decay = (ad < 2) ? 2 : ad;    // 2 = sharp click, high = long 808
    
    trigger_kick = true;
  }
  
  if (s.active) {
      trigger_noise = true;
      noise_decay_rate = s.decay_rate;
  }

  // Visualization
  setColorAllPixel(0); // Clear buffer

  if (k.active) {
    // Bass Visualization (Center Out)
    uint8_t color_pos = map(kick_start_freq, 800, 1000, 190, 220);
    uint32_t color_center = colorWheel(color_pos);
    uint32_t color_mid    = colorWheel(color_pos + 20);
    uint32_t color_outer  = colorWheel(color_pos + 40);
    
    uint8_t num_pixels = map(kick_amp_decay, 7, 15, 4, 20);
    if (num_pixels > 20) num_pixels = 20;

    // 1. Center
    for (int y = 0; y < 4; y++) {
      if (num_pixels > 0) { matrixSet(2, 3 - y, color_center); num_pixels--; }
    }
    // 2. Mid
    for (int y = 0; y < 4; y++) {
      if (num_pixels >= 2) { matrixSet(1, 3 - y, color_mid); matrixSet(3, 3 - y, color_mid); num_pixels -= 2; }
    }
    // 3. Outer
    for (int y = 0; y < 4; y++) {
      if (num_pixels >= 2) { matrixSet(0, 3 - y, color_outer); matrixSet(4, 3 - y, color_outer); num_pixels -= 2; }
    }
  }

  if (s.active) {
      // Snare Visualization: White Flash on top corners
      uint32_t white = pixels.Color(200, 200, 200);
      matrixSet(0, 0, white);
      matrixSet(4, 0, white);
      matrixSet(1, 0, white);
      matrixSet(3, 0, white);
  }

  pixels.show();
  
  // Flash duration logic
  // If either is active, we flash. If silent, just wait.
  if (k.active || s.active) {
      my_delay(FLASH_MS);
      setColorAllPixel(0);
      pixels.show();
      my_delay(STEP_MS - FLASH_MS);
  } else {
      // Idle cursor
      // pixels.setPixelColor(current_step, 5, 5, 5);
      pixels.show();
      my_delay(STEP_MS);
  }

  current_step = (current_step + 1) % 16;
}

ISR(TIMER0_COMPA_vect) {
  ticks++;
  
  uint16_t bass_sample = 0;
  uint16_t noise_sample = 0;

  // --- Bass Voice ---
  if (trigger_kick) {
    phase_increment = kick_start_freq;
    amplitude = 255;
    trigger_kick = false;
  }

  if (amplitude > 0) {
    phase += phase_increment;
    if (phase_increment > kick_end_freq) {
      phase_increment -= kick_pitch_decay;
    }
    
    static uint8_t decay_counter = 0;
    decay_counter++;
    if (decay_counter >= kick_amp_decay) {
        if (amplitude > 0) amplitude--;
        decay_counter = 0;
    }

    #ifdef USE_SQUARE_WAVE
      if (phase & 0x8000) bass_sample = amplitude; 
    #else
      uint8_t s = pgm_read_byte(&sine256[phase >> 8]);
      bass_sample = (s * amplitude) >> 8;
    #endif
  }

  // --- Noise Voice (LFSR) ---
  if (trigger_noise) {
      noise_amp = 140; // Reduced from 255 to mix better with kick
      trigger_noise = false;
  }
  
  if (noise_amp > 0) {
      static uint8_t noise_decay_cnt = 0;
      noise_decay_cnt++;
      if (noise_decay_cnt >= noise_decay_rate) {
          if (noise_amp > 0) noise_amp--;
          noise_decay_cnt = 0;
      }
      
      // Galois LFSR 16-bit
      // Taps: 16, 14, 13, 11 -> Mask 0xB400
      uint8_t lsb = lfsr & 1;
      lfsr >>= 1;
      if (lsb) lfsr ^= 0xB400u;
      
      if (lsb) noise_sample = noise_amp;
  }
  
  // --- Mixing ---
  // Simple average to avoid overflow, or saturating add.
  // (bass + noise) >> 1
  OCR1A = (bass_sample + noise_sample) >> 1;
}
