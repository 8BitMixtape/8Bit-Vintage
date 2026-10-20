/* ---------------------------------------------------------------------------------------
    SPAGHETTI! — THE TALKING CHIP (ITALIAN VOICE SAMPLER)
    For ATtiny85 / 8Bit Mixtape (AudioBoot)

    Plays back a pre-recorded digitized speech sample saying:
    "Spaghetti!"
    
    Controls & Effects:
      - Left Pot (PB4 / A2): Playback Speed & Pitch (Low-pitched monster to hyper chipmunk)
      - Right Pot (PB2 / A1): Bitcrush / Lo-Fi Resolution Crusher (1 to 8-bit dynamic quantization)
      - Buttons (PB3 / A3):
          - Left Button: Clean Pitch-Shifted Voice
          - Right Button: Bitcrushed Cyber-Spaghetti
          - Both Buttons: Rapid-fire "Spag-Spag-Spaghetti!" stutter loop with party strobe
      - 5x4 NeoPixel Matrix (PB0):
          - Italian Tricolore (Green, White, Red) pulsing dynamically with voice volume
          - Right-column position indicator showing bitcrush depth
          - Idle breathing ambient glow mapped to potentiometers
 --------------------------------------------------------------------------------------- */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "spageti.h"

// --- Hardware Definitions ---
#define NUMPIXELS 20
#define NEOPIXELPIN 0 // PB0
#define SPEAKERPIN  1 // PB1 (OC1A Timer1)
#define BUTTON_PIN  A3 // PB3 (ADC3)
#define POTI_LEFT   A2 // PB4 (ADC2)
#define POTI_RIGHT  A1 // PB2 (ADC1)

// --- Button Voltage Calibration ---
#define Vbutton_releaseLevel  490
#define Vbutton_left          460
#define Vbutton_right         395
#define Vbutton_both          320

// --- NeoPixel Object ---
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

void drawIndicator(uint8_t indicator_y) {
  const uint8_t rightColIndices[4] = {4, 9, 14, 19};
  if (indicator_y > 3) indicator_y = 3;
  for(uint8_t i = 0; i < 4; i++) {
    pixels.setPixelColor(rightColIndices[i], pixels.Color(20, 0, 0));
  }
  pixels.setPixelColor(rightColIndices[indicator_y], pixels.Color(255, 255, 255));
}

void playSpeech(int effectType) {
  int speedDelay = 300; 
  uint8_t shiftAmount = 0;

  unsigned int start_idx = 0;
  unsigned int end_idx = sample_len;

  // Stutter mode: play only first half repeated
  if (effectType == 3) {
    end_idx = sample_len / 2;
  }

  for (unsigned int i = start_idx; i < end_idx; i++) {
    
    // Update pots and NeoPixels every 128 samples to allow live tweaking 
    if ((i & 0x7F) == 0) {
      uint16_t p1 = analogRead(POTI_LEFT);
      // Map 0-1023 to ~50-818 us delay for dynamic pitch/speed
      speedDelay = 50 + ((p1 * 3) >> 2); 
      
      uint16_t p2 = analogRead(POTI_RIGHT);
      
      if (effectType == 1) {
        // Mode 1: Clean pitch-shifted voice
        shiftAmount = 0;
      } else {
        // Mode 2 or 3: Bitcrush / robotic crunch (0 to 7 bits shifted)
        shiftAmount = p2 >> 7; 
      }
      
      // Visual amplitude
      uint8_t currentSample = pgm_read_byte(&sample_data[i]);
      int16_t ampDiff = (int16_t)currentSample - 128;
      if (ampDiff < 0) ampDiff = -ampDiff;
      uint16_t amplitude = ampDiff << 1; 
      if (amplitude > 255) amplitude = 255;

      // Render Italian Tricolore themed amplitude display:
      // Left 2 cols = Green, Center 1 col = White, Right 2 cols = Red
      for (uint8_t x = 0; x < 5; x++) {
        for (uint8_t y = 0; y < 4; y++) {
          uint8_t p = y * 5 + x;
          if (x < 2) {
            // Emerald Green (Basil)
            pixels.setPixelColor(p, pixels.Color(0, amplitude, (amplitude >> 3)));
          } else if (x == 2) {
            // Warm White / Garlic Mozzarella
            pixels.setPixelColor(p, pixels.Color(amplitude, amplitude, amplitude));
          } else {
            // Tomato Red (Pomodoro)
            pixels.setPixelColor(p, pixels.Color(amplitude, 0, 0));
          }
        }
      }
      
      // Override the rightmost column to show effect pot position
      drawIndicator(p2 >> 8); // p2 is 0-1023 -> 0-3
      
      pixels.show();
    }
    
    uint8_t sample1 = pgm_read_byte(&sample_data[i]);
    
    // Apply bitcrush if active
    if (effectType >= 2 && shiftAmount > 0) {
      sample1 = (sample1 >> shiftAmount) << shiftAmount;
    }
    
    // Output sample to High-Speed PWM (PB1)
    OCR1A = sample1;
    delayMicroseconds(speedDelay);
  }
  
  // Center of 8-bit waveform (silence)
  OCR1A = 128;
}

void setup() {
  pinMode(SPEAKERPIN, OUTPUT);

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  
  // Initialize NeoPixels
  pixels.begin();
  pixels.setBrightness(70);
  pixels.clear();
  pixels.show();
  
  // Setup High-Speed PWM on Timer1 for PB1 (OC1A) via 64MHz PLL
  PLLCSR = (1 << PCKE) | (1 << PLLE);
  TCCR1  = (1 << PWM1A) | (2 << COM1A0) | (1 << CS10); // 250 kHz carrier on PB1
  GTCCR  = 0;

  // Initial silence (DC center)
  OCR1A = 128;
}

void loop() {
  // Read button
  uint16_t btn = analogRead(BUTTON_PIN);
  
  if (btn < Vbutton_both) {
    // Both buttons: Machine-gun "Spag-Spag-Spaghetti!" stutter loop
    playSpeech(3);
    delay(50);
  } else if (btn < Vbutton_right) {
    // Right button: Bitcrushed cyber voice
    playSpeech(2);
    pixels.clear();
    pixels.show();
    delay(350);
  } else if (btn < Vbutton_left) {
    // Left button: Clean pitch-shifted voice
    playSpeech(1);
    pixels.clear();
    pixels.show();
    delay(350);
  } else {
    // Idle animation: breathing ambient pasta glow mapped to potentiometers
    uint16_t p1 = analogRead(POTI_LEFT) >> 2;  // 0-255
    uint16_t p2 = analogRead(POTI_RIGHT) >> 2; // 0-255
    
    uint8_t val = (millis() / 20) % 255;
    if (val > 127) val = 255 - val; // 0 to 127
    if (val < 15) val = 15;
    
    // Warm Italian tricolor idle breathing
    for (uint8_t x = 0; x < 5; x++) {
      for (uint8_t y = 0; y < 4; y++) {
        uint8_t p = y * 5 + x;
        if (x < 2) {
          pixels.setPixelColor(p, pixels.Color(0, (val * p2) >> 8, 0));
        } else if (x == 2) {
          uint8_t w = (val * 160) >> 8;
          pixels.setPixelColor(p, pixels.Color(w, w, w));
        } else {
          pixels.setPixelColor(p, pixels.Color((val * p1) >> 8, 0, 0));
        }
      }
    }
    
    // Draw effect indicator on the right side
    drawIndicator(p2 >> 6); // 0-255 -> 0-3
    
    pixels.show();
    delay(15);
  }
}
