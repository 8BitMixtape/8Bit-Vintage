#ifndef ABK20_HARDWARE_H
#define ABK20_HARDWARE_H

#include <Arduino.h>

/* =======================================================================================
   ABK-20 HARDWARE DEFINITIONS & SENSOR DECODERS (Rev 0.3)
   
   Hardware Pinout on ATtiny85:
     PB0 (Pin 5) - NeoPixel 5x4 Matrix Data (DIN) / Bi-directional LED Cathode
     PB1 (Pin 6) - Audio PWM Out (Timer 1 OC1A) / Bi-directional LED Anode (via 1k R26)
     PB2 (Pin 7) - Slide Fader RV1 (ADC1 / A1)
     PB3 (Pin 2) - Audio In (100nF AC coupled) & Analog Switch Ladder (ADC3 / A3)
     PB4 (Pin 3) - Rotary Potentiometer POT2 (ADC2 / A2)
     PB5 (Pin 1) - RESET (Active LOW / ISP Reset)

   Measured Voltages on PB3 (ADC3) @ VCC = 5.0V:
     1. Idle (Center Toggle, no button):  2.63 V  -> ADC ~538 (ADCH ~134)
     2. Toggle 1 (SPDT Pos 1 / DOWN):     1.72 V  -> ADC ~352 (ADCH ~88)
     3. Toggle 2 (SPDT Pos 2 / UP):       1.29 V  -> ADC ~264 (ADCH ~66)
     4. Button SW2 (Solo Press):          0.99 V  -> ADC ~202 (ADCH ~51)
     5. SW2 + Toggle 1:                   0.82 V  -> ADC ~168 (ADCH ~42)
     6. SW2 + Toggle 2:                   0.70 V  -> ADC ~143 (ADCH ~36)

   Potentiometers (Rev 0.3 Routing & Measured Voltages @ VCC = 5.0V):
     - Voltage range: 0.00 V (bottom / CCW) to 3.58 V (top / CW)
     - ADC range (10-bit): ~0 (0.00V) to ~732 (3.58V)
     - Rotary Pot (PB4 / A2):  CCW (bottom) = 0.00V (~0), CW (top) = 3.58V (~732)
     - Slide Fader (PB2 / A1): Bottom = 0.00V (~0), Top = 3.58V (~732)
   ======================================================================================= */

// --- Physical Pin Assignments ---
#define PIN_NEOPIXEL     0   // PB0: NeoPixel Data & Bi-color LED K
#define PIN_SPEAKER      1   // PB1: Audio Output & Bi-color LED A
#define PIN_FADER_ADC    A1  // PB2: Slide Fader (RV1)
#define PIN_LADDER_ADC   A3  // PB3: Toggle SW1 + Button SW2 Ladder & Audio In
#define PIN_POTI_ADC     A2  // PB4: Rotary Pot (POT2)

#define NUM_MATRIX_LEDS  20  // 5 columns x 4 rows matrix

// --- Potentiometer Hardware Calibration ---
// Physical wiper voltage swing: 0.00V to 3.58V (3.58V / 5.0V * 1023 ~= 732)
#define ABK_POT_ADC_BOT  10   // Noise / ground margin to ensure clean zero
#define ABK_POT_ADC_TOP  725  // Calibrated top threshold (~3.54V to ensure 255 is reached)

// --- PB3 Switch Ladder States ---
enum LadderState : uint8_t {
  LADDER_IDLE        = 0, // Toggle Center, Button Released (nominal 2.63V / ADC ~538)
  LADDER_TOGGLE_1     = 1, // Toggle 1 (DOWN) alone          (nominal 1.72V / ADC ~352)
  LADDER_TOGGLE_2     = 2, // Toggle 2 (UP) alone            (nominal 1.29V / ADC ~264)
  LADDER_SW2_SOLO     = 3, // Button SW2 pressed alone       (nominal 0.99V / ADC ~202)
  LADDER_SW2_TOGGLE_1 = 4, // SW2 pressed + Toggle 1 (DOWN)  (nominal 0.82V / ADC ~168)
  LADDER_SW2_TOGGLE_2 = 5  // SW2 pressed + Toggle 2 (UP)    (nominal 0.70V / ADC ~143)
};

// Midpoint Decision Thresholds (10-bit ADC, 0-1023)
#define THRESH_IDLE_TOGGLE1  445  // Midpoint between 538 and 352
#define THRESH_TOGGLE1_2     308  // Midpoint between 352 and 264
#define THRESH_TOGGLE2_SW2   233  // Midpoint between 264 and 202
#define THRESH_SW2_BOTH1     185  // Midpoint between 202 and 168
#define THRESH_BOTH1_BOTH2   155  // Midpoint between 168 and 143

// --- Ladder Reading with 3-Sample Averaging & Debouncing ---
static inline LadderState readLadderState() {
  uint16_t s1 = analogRead(PIN_LADDER_ADC);
  uint16_t s2 = analogRead(PIN_LADDER_ADC);
  uint16_t s3 = analogRead(PIN_LADDER_ADC);
  uint16_t adc = (s1 + s2 + s3) / 3;

  LadderState raw;
  if (adc > THRESH_IDLE_TOGGLE1)        raw = LADDER_IDLE;
  else if (adc > THRESH_TOGGLE1_2)      raw = LADDER_TOGGLE_1;
  else if (adc > THRESH_TOGGLE2_SW2)    raw = LADDER_TOGGLE_2;
  else if (adc > THRESH_SW2_BOTH1)      raw = LADDER_SW2_SOLO;
  else if (adc > THRESH_BOTH1_BOTH2)    raw = LADDER_SW2_TOGGLE_1;
  else                                  raw = LADDER_SW2_TOGGLE_2;

  // 2-frame debouncing filter
  static LadderState debounced = LADDER_IDLE;
  static LadderState candidate = LADDER_IDLE;
  static uint8_t count = 0;

  if (raw == candidate) {
    if (++count >= 2) debounced = raw;
  } else {
    candidate = raw;
    count = 1;
  }

  return debounced;
}

// Helper: Is Button SW2 held down?
static inline bool isButtonSW2Pressed(LadderState state) {
  return (state >= LADDER_SW2_SOLO);
}

// Helper: Get SPDT Toggle Switch Position: 0 = Center (Off), 1 = Pos 1 (DOWN), 2 = Pos 2 (UP)
static inline uint8_t getTogglePosition(LadderState state) {
  if (state == LADDER_TOGGLE_1 || state == LADDER_SW2_TOGGLE_1) return 1;
  if (state == LADDER_TOGGLE_2 || state == LADDER_SW2_TOGGLE_2) return 2;
  return 0; // Center / Off
}

// --- Potentiometers Reading (Rev 0.3: 0 at GND/bottom, 255 at Top/CW) ---
// Scaled 0..255 using measured voltage swing (0.00V to 3.58V)

// Reads Rotary Pot (PB4 / A2) returning scaled 0..255 (Turning CW increases)
static inline uint8_t readRotaryPot() {
  int16_t raw = analogRead(PIN_POTI_ADC);
  if (raw < ABK_POT_ADC_BOT) raw = ABK_POT_ADC_BOT;
  if (raw > ABK_POT_ADC_TOP) raw = ABK_POT_ADC_TOP;
  uint32_t val = (uint32_t)(raw - ABK_POT_ADC_BOT) * 255 / (ABK_POT_ADC_TOP - ABK_POT_ADC_BOT);
  return (uint8_t)(val > 255 ? 255 : val);
}

// Reads Slide Fader (PB2 / A1) returning scaled 0..255 (Sliding UP increases)
static inline uint8_t readSlideFader() {
  int16_t raw = analogRead(PIN_FADER_ADC);
  if (raw < ABK_POT_ADC_BOT) raw = ABK_POT_ADC_BOT;
  if (raw > ABK_POT_ADC_TOP) raw = ABK_POT_ADC_TOP;
  uint32_t val = (uint32_t)(raw - ABK_POT_ADC_BOT) * 255 / (ABK_POT_ADC_TOP - ABK_POT_ADC_BOT);
  return (uint8_t)(val > 255 ? 255 : val);
}

// --- Bi-directional LED Helpers (KS01 Button LED between PB1 and PB0) ---
// Note: Direct port writes to PB1/PB0 should be used when NeoPixels/Speaker are not actively driving.
static inline void abkLedGreen() {
  PORTB = (PORTB & ~(1 << PIN_NEOPIXEL)) | (1 << PIN_SPEAKER);
}

static inline void abkLedRed() {
  PORTB = (PORTB & ~(1 << PIN_SPEAKER)) | (1 << PIN_NEOPIXEL);
}

static inline void abkLedOff() {
  PORTB &= ~((1 << PIN_SPEAKER) | (1 << PIN_NEOPIXEL));
}

#endif // ABK20_HARDWARE_H

