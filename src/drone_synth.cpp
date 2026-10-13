#include "neolib.h"
#include "matrix_helpers.h"
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/power.h>

// --- Audio ---
volatile uint16_t phase1 = 0;
volatile uint16_t phase2 = 0;
volatile uint16_t phase_inc1 = 0;
volatile uint16_t phase_inc2 = 0;
volatile uint16_t ticks = 0;

// Custom delay using ISR ticks (10kHz)
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

// Trickster Plasma Variables
float p1 = 0.f, p2 = 0.f, p3 = 0.f;

// Linked Visual Theme & Audio Mode
uint8_t currentTheme = 0;
volatile uint8_t currentMode = 0; // 0=Triangle, 1=Fuzz, 2=Bitcrush, 3=RingMod

// Portamento filter states & targets
float smoothPitch = 500.f;
float smoothDetune = 0.f;
float smoothSpeed = 1.0f;
float smoothZoom = 1.5f;

float targetPitch = 500.f;
float targetDetune = 0.f;
float targetSpeed = 1.0f;
float targetZoom = 1.5f;

bool isLongPress = false;

// Button logic
#define BUTTON_LEFT  1
#define BUTTON_RIGHT 2

void setup() {
  neobegin();
  pixels.setBrightness(100);

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // PLL for 64MHz Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Timer1 PWM (Audio Output)
  TIMSK = 0;
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10; 
  pinMode(SPEAKERPIN, OUTPUT);

  // Timer0 10kHz Interrupt
  TCCR0A = 3 << WGM00; // WGM01 | WGM00
  TCCR0B = 1 << WGM02 | 2 << CS00; // WGM02 | Prescaler 8
  TIMSK = 1 << OCIE0A;
  OCR0A = 199; // (16MHz / 8) / (199+1) = 10kHz
}

// Helper for plasma colors
uint32_t lerpCol(uint8_t a) {
  uint32_t COL_DARK;
  uint32_t COL_BRIGHT;
  
  if (currentTheme == 0) {
    COL_DARK   = pixels.Color(30, 15, 0);
    COL_BRIGHT = pixels.Color(255, 220, 60); // Yellow/Brown
  } else if (currentTheme == 1) {
    COL_DARK   = pixels.Color(40, 0, 0);
    COL_BRIGHT = pixels.Color(255, 50, 0);   // Deep Red/Orange
  } else if (currentTheme == 2) {
    COL_DARK   = pixels.Color(20, 10, 0);
    COL_BRIGHT = pixels.Color(200, 100, 0);  // Burnt Orange
  } else {
    COL_DARK   = pixels.Color(30, 0, 10);
    COL_BRIGHT = pixels.Color(255, 20, 60);  // Crimson/Warm Red
  }

  uint8_t dr = (uint8_t)((COL_DARK   >> 16) & 0xFF);
  uint8_t dg = (uint8_t)((COL_DARK   >>  8) & 0xFF);
  uint8_t db = (uint8_t)( COL_DARK         & 0xFF);
  uint8_t br = (uint8_t)((COL_BRIGHT >> 16) & 0xFF);
  uint8_t bg = (uint8_t)((COL_BRIGHT >>  8) & 0xFF);
  uint8_t bb = (uint8_t)( COL_BRIGHT       & 0xFF);
  uint8_t r = (uint8_t)(dr + ((uint16_t)(br - dr) * a >> 8));
  uint8_t g = (uint8_t)(dg + ((uint16_t)(bg - dg) * a >> 8));
  uint8_t b = (uint8_t)(db + ((uint16_t)(bb - db) * a >> 8));
  return pixels.Color(r, g, b);
}

void loop() {
  float rawVal1 = (float)analogRead(POTI_LEFT);
  float rawVal2 = (float)analogRead(POTI_RIGHT);
  
  static uint8_t holdFrames = 0;
  uint8_t btnState = getButton();
  
  if (btnState != BUTTON_NONE) {
    if (holdFrames < 10) holdFrames++;
    
    if (holdFrames >= 6) { // ~300ms hold time
      // SHIFT MODE: Adjust Visuals
      isLongPress = true; // Treat as a long press; suppress mode change on release
      
      // Knob -> Zoom (Plasma complexity)
      targetZoom = 0.5f + (rawVal1 / 1023.0f) * 2.0f;
      // Slider -> Speed (Very slow to high: 0.1x to 5.0x)
      targetSpeed = 0.1f + (rawVal2 / 1023.0f) * 4.9f;
    }
    // If holdFrames < 6, we temporarily freeze parameters to ignore accidental shifts during clicks
  } else {
    holdFrames = 0;
    // NORMAL MODE: Adjust Audio
    targetPitch = 20.0f + (rawVal1 / 1023.0f) * 1980.0f;
    targetDetune = -(targetPitch / 2.0f) + (rawVal2 / 1023.0f) * (targetPitch * 1.5f);
  }

  // Smooth all parameters (Portamento)
  smoothPitch += (targetPitch - smoothPitch) * 0.1f;
  smoothDetune += (targetDetune - smoothDetune) * 0.1f;
  smoothSpeed += (targetSpeed - smoothSpeed) * 0.1f;
  smoothZoom += (targetZoom - smoothZoom) * 0.1f;

  // Read Buttons: Change mode ONLY if it was a short click (not a long press)
  uint8_t btn = wasButtonPressed();
  if (btn != BUTTON_NONE) {
    if (!isLongPress) {
      if (btn == BUTTON_LEFT) {
        currentMode = (currentMode + 3) % 4; // Down
      } else if (btn == BUTTON_RIGHT) {
        currentMode = (currentMode + 1) % 4; // Up
      }
      currentTheme = currentMode;
    }
    isLongPress = false; // Reset shift state on release
  }
  
  // Advance plasma phases with smoothed speed
  p1 += 0.18f * smoothSpeed;
  p2 += 0.11f * smoothSpeed;
  p3 += 0.08f * smoothSpeed;

  uint16_t sum_a = 0;
  for (uint8_t y = 0; y < MATRIX_H; ++y) {
    for (uint8_t x = 0; x < MATRIX_W; ++x) {
      float zx = (float)x * smoothZoom;
      float zy = (float)y * smoothZoom;
      
      float v = 0.0f;
      v += sin((zx + p1) * 0.8f);
      v += cos((zy - p2) * 1.1f);
      v += sin((zx + zy + p3) * 0.6f);
      v += 0.6f * sin((zx * 0.5f - zy * 0.7f) + p2 * 0.9f);

      v = (v * (1.0f / 7.2f)) + 0.5f; // normalize ~[-3.6,3.6] to ~[0,1]
      if (v < 0.f) v = 0.f;
      if (v > 1.f) v = 1.f;

      float pulse = (sin(p1 * 0.25f) * 0.15f);
      float vv = v + pulse;
      if (vv < 0.f) vv = 0.f;
      if (vv > 1.f) vv = 1.f;

      vv = vv * 1.05f;
      if (vv > 1.f) vv = 1.f;
      if (vv < 0.f) vv = 0.f;
      vv = pow(vv, 2.4f);
      if (vv < 0.06f) vv = 0.f;

      uint8_t a = (uint8_t)(vv * 255.0f + 0.5f);
      sum_a += a;
      uint32_t c = lerpCol(a);
      matrixSet(x, y, c);
    }
  }

  pixels.show();

  // Plasma-based modulation
  uint8_t avg_a = sum_a / 20; 
  // More modulation when darker: depth ranges from 0 to 255
  float mod_depth = 255.0f - (float)avg_a;
  
  // Slow LFO based on plasma time
  float lfo = sin(p1 * 0.4f); 
  int16_t mod_val = (int16_t)(lfo * mod_depth * 1.5f); // Scale up for noticeable pitch wobble

  // Calculate Frequencies & Detune
  int32_t base_f = (int32_t)smoothPitch;
  int32_t detune = (int32_t)smoothDetune; 
  
  int16_t f1 = base_f + mod_val;
  int16_t f2 = base_f + detune - mod_val; // Invert mod on f2 for stereo phasing effect!

  if (f1 < 10) f1 = 10;
  if (f2 < 10) f2 = 10;
  
  phase_inc1 = f1;
  phase_inc2 = f2;

  my_delay(50); // Frame rate limit
}

ISR(TIMER0_COMPA_vect) {
  ticks++;
  
  phase1 += phase_inc1;
  phase2 += phase_inc2;
  
  // Generate Base Triangles
  uint8_t t1 = phase1 >> 8;
  if (t1 > 127) t1 = 255 - t1;
  uint8_t out1 = t1 << 1;
  
  uint8_t t2 = phase2 >> 8;
  if (t2 > 127) t2 = 255 - t2;
  uint8_t out2 = t2 << 1;
  
  if (currentMode == 0) {
    // Classic Dual Triangle
    OCR1A = (out1 + out2) >> 1;
  } else if (currentMode == 1) {
    // Overdriven/Fuzzed Triangle (Hard clipping)
    uint16_t mix = (uint16_t)out1 + (uint16_t)out2;
    if (mix > 255) mix = 255;
    OCR1A = mix;
  } else if (currentMode == 2) {
    // Bitcrushed Triangle (Drop lower 3 bits)
    OCR1A = ((out1 & 0xE0) + (out2 & 0xE0)) >> 1;
  } else {
    // Ring Modulation
    OCR1A = (uint8_t)(((uint16_t)out1 * (uint16_t)out2) >> 8);
  }
}
