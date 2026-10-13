/* ---------------------------------------------------------------------------------------
        ATtiny85 DEEP MINIMAL TECHNO
        A specialized groovebox for deep, plucky, sub-heavy minimal techno.
        Outputs mixed Bass + Drums on PB1 (OC1A)
        
        Interface:
        Hold Left Button  -> Play Mode
                             Left Pot: Macro (Filter Cutoff + Bass Decay)
                             Right Pot: Select Pattern (0-3)
        Hold Right Button -> Shift Mode
                             Right Pot: Adjust BPM
---------------------------------------------------------------------------------------
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 20
#define NEOPIXELPIN 0
#define SPEAKERPIN 1 
#define BUTTON_PIN A3 
#define POTI_LEFT  A2 
#define POTI_RIGHT A1 

#define Vbutton_releaseLevel  490
#define Vbutton_midLevel      410

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Synth Variables ---
volatile uint16_t bass_phase = 0;
volatile uint16_t bass_pitch = 0;
volatile uint8_t  bass_env = 0;
volatile uint8_t  filter_mod = 0;

volatile uint16_t drum_phase = 0;
volatile uint16_t drum_pitch = 0;
volatile uint8_t  drum_env = 0;
volatile uint8_t  drum_type = 0; // 0=kick, 1=hat

uint16_t lfsr = 0xACE1;
volatile uint8_t is_playing = 0;

void setup() {
  pinMode(SPEAKERPIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(50);
  
  // High-Speed PWM on Timer1 for PB1 (OC1A)
  PLLCSR |= (1 << PLLE);               
  delayMicroseconds(100);
  while (!(PLLCSR & (1 << PLOCK)));    
  PLLCSR |= (1 << PCKE);               

  TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS12); 
  GTCCR = 0;
  
  TIMSK |= (1 << TOIE1);
  sei();
}

ISR(TIMER1_OVF_vect) {
  static uint8_t math_div = 0;
  math_div = !math_div;
  if (math_div) return; // 15.6 kHz to save CPU

  static uint8_t tick = 0;
  tick++;
  
  // LFSR for Noise
  if (lfsr & 1) { lfsr >>= 1; lfsr ^= 0xB400u; }
  else { lfsr >>= 1; }

  if (!is_playing) {
    OCR1A = 128;
    return;
  }

  // --- Envelopes (Every ~0.5ms) ---
  if ((tick & 7) == 0) { 
    // Macro Knob: Left Pot controls Bass Decay!
    uint8_t bass_decay = 12 - (filter_mod >> 4); 
    if (bass_decay < 2) bass_decay = 2; // slow decay
    
    if (bass_env > bass_decay) bass_env -= bass_decay; 
    else bass_env = 0;
    
    if (drum_type == 0) {
      if (drum_env > 1) drum_env -= 2; // Kick envelope
    } else {
      if (drum_env > 15) drum_env -= 16; else drum_env = 0; // Super tight hat
    }
  }

  // --- Bassline Synthesis (Deep Triangle + 1-Pole Lowpass) ---
  static int16_t filter_val = 0;
  
  bass_phase += (bass_pitch << 1); 
  uint8_t bw = bass_phase >> 8;
  if (bass_phase & 0x8000) bw = 255 - bw; // Triangle wave (deep sub)
  int16_t raw_bass = (int16_t)bw - 128; 
  
  // 1-Pole Lowpass Filter
  int16_t cutoff = 10 + filter_mod; // 10 to 265
  if (cutoff > 255) cutoff = 255;
  
  filter_val += ((raw_bass - filter_val) * cutoff) >> 8;
  
  int16_t outA = (filter_val * bass_env) >> 8;

  // --- Drum Synthesis ---
  uint8_t outB = 0;
  if (drum_type == 0) { // Kick
    drum_phase += (drum_pitch << 1);
    if (drum_pitch > 20) drum_pitch -= 2; // Punchy pitch drop
    
    uint8_t kick_wave = drum_phase >> 8;
    if (drum_phase & 0x8000) kick_wave = 255 - kick_wave; // Triangle Kick
    outB = (kick_wave * drum_env) >> 8;
  } else { // Hat
    outB = ((lfsr & 0xFF) * drum_env) >> 8;
  }

  // --- MIXING ---
  int16_t sA = outA; // -128 to +127
  int16_t sB = (int16_t)outB; // 0 to 255

  // Drums hit slightly harder than bass
  int16_t mixed = sA + sB; 
  if (mixed > 127) mixed = 127;
  if (mixed < -128) mixed = -128;
  
  OCR1A = (uint8_t)(mixed + 128);
}

// Deep Sub Frequencies
const uint16_t P_G1  = 102; // ~49 Hz
const uint16_t P_Bb1 = 122; // ~58 Hz
const uint16_t P_C2  = 137; // ~65 Hz
const uint16_t P_Eb2 = 163; // ~77 Hz
const uint16_t P_F2  = 183; // ~87 Hz
const uint16_t P_G2  = 205; // ~98 Hz

const uint16_t bass_seq_pitches[4][16] PROGMEM = {
  // 0: Deep Off-beat Plucks
  { 0, 0, P_C2, 0, 0, 0, P_C2, 0, 0, 0, P_Eb2, 0, 0, 0, P_C2, 0 },
  // 1: Rolling Sub
  { 0, P_C2, 0, 0, 0, P_C2, 0, P_C2, 0, P_Eb2, 0, 0, 0, P_F2, 0, 0 },
  // 2: Sparse G-Minor
  { P_C2, 0, 0, 0, P_C2, 0, 0, 0, P_G1, 0, 0, 0, P_Bb1, 0, 0, 0 },
  // 3: Detroit Style
  { 0, P_C2, P_C2, 0, 0, P_Eb2, P_C2, 0, 0, P_C2, P_C2, 0, 0, P_F2, P_C2, 0 }
};

const uint8_t bass_seq_accents[4][16] PROGMEM = { 
  { 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0 },
  { 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0 },
  { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 }
};

const uint8_t drum_seq[4][16] PROGMEM = {
  { 1, 0, 2, 0, 1, 0, 2, 0, 1, 0, 2, 0, 1, 0, 2, 2 }, // 0: Standard 4x4
  { 1, 0, 0, 2, 1, 0, 0, 2, 1, 0, 0, 2, 1, 0, 0, 2 }, // 1: Dubby
  { 1, 0, 2, 0, 0, 0, 2, 0, 1, 0, 2, 0, 0, 0, 2, 2 }, // 2: Syncopated
  { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 }  // 3: Driving
}; // 1 = Kick, 2 = Hat, 0 = Rest

void drawIndicator(uint8_t indicator_y) {
  const uint8_t rightColIndices[4] = {4, 9, 14, 19};
  for(uint8_t i=0; i<4; i++) {
    pixels.setPixelColor(rightColIndices[i], pixels.Color(20, 0, 0));
  }
  pixels.setPixelColor(rightColIndices[indicator_y], pixels.Color(255, 255, 255));
}

void loop() {
  uint16_t btn = analogRead(BUTTON_PIN);
  
  if (btn < Vbutton_releaseLevel) { // Playing
    is_playing = 1;
    
    static uint8_t step = 0;
    static uint8_t current_pattern = 0;
    static int tempo_delay = 120; // ~125 BPM
    
    // Left Pot ALWAYS controls the Macro (Filter + Decay)
    uint16_t p1 = analogRead(POTI_LEFT); 
    filter_mod = p1 >> 2; // 0-255
    
    if (btn < Vbutton_midLevel) {
      // Right Button Held: Shift Mode (Right Pot changes BPM)
      uint16_t p2 = analogRead(POTI_RIGHT);
      tempo_delay = 60 + ((1023L - p2) * 140L) / 1023L; // 60ms to 200ms
    } else {
      // Left Button Held: Normal Mode (Right Pot changes Pattern)
      uint16_t p2 = analogRead(POTI_RIGHT); 
      current_pattern = p2 >> 8; // 0 to 3
    }
    
    // Trigger Drum
    uint8_t d = pgm_read_byte(&drum_seq[current_pattern][step]);
    if (d == 1) {
      drum_type = 0;
      drum_pitch = 1000; 
      drum_env = 255;
    } else if (d == 2) {
      drum_type = 1;
      drum_env = 100; 
    }
    
    // Trigger Bass
    uint16_t next_pitch = pgm_read_word(&bass_seq_pitches[current_pattern][step]);
    if (pgm_read_byte(&bass_seq_accents[current_pattern][step])) {
      bass_pitch = next_pitch;
      bass_env = 255;
    }
    
    // Visuals
    uint8_t bass_vis = bass_env >> 1; 
    uint8_t drum_vis = drum_env >> 1; 
    
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(bass_vis + drum_vis, drum_vis, drum_vis)); 
    }
    drawIndicator(current_pattern);
    cli(); pixels.show(); sei(); 
    
    step = (step + 1) & 15;
    delay(tempo_delay);
    
  } else {
    // Idle Mode
    is_playing = 0;
    
    uint16_t p1 = analogRead(POTI_LEFT) >> 2; 
    uint16_t p2 = analogRead(POTI_RIGHT) >> 2; 
    
    uint8_t val = (millis() / 20) % 255;
    if (val > 127) val = 255 - val; 
    if (val < 15) val = 15;
    
    uint8_t r = (p1 * val) >> 7;
    uint8_t g = (p2 * val) >> 7;
    uint8_t b = ((255 - p1) * val) >> 7;
    
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(r, g, b)); 
    }
    drawIndicator(p2 >> 6); 
    cli(); pixels.show(); sei(); 
    
    delay(10);
  }
}
