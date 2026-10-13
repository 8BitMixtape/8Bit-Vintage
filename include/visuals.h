// Ensure dependencies are available before any implementations that use MATRIX_W/H
#include <stdint.h>
#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"
#include <math.h>
#include <string.h>

// [Note for contributors]
// Implementations of visuals are placed below in the "===== Implementations =====" section.
// When adding a new effect:
//  1) Declare its prototype in the Visuals API block below (void myVisual(...);
//  2) Implement it in the Implementations section alongside the others (group by theme if possible).
//  3) Add an entry in src/AetzLampliu_mini.ino's Visual Selector and (optionally) set it as default.
//  4) Keep RAM tight for ATtiny85; prefer small local arrays and reuse helpers (matrixFade, dimColor, etc.).

#pragma once
#include <stdint.h>
#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"

// Externs provided by the sketch/libs
extern Adafruit_NeoPixel pixels;

// Forward utility from sketch
uint32_t dimColor(uint32_t color, uint8_t scale);
uint32_t Wheel(byte WheelPos);
static inline uint32_t colorFromHeat(uint8_t heat);
void playSound(uint16_t freq, uint16_t dur);

// Visuals API (add more here as we migrate)
void matrixRainbowWaves(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixFire(uint16_t runtime_ms, uint8_t cooling, uint8_t sparking, uint8_t stepDelay_ms);
void matrixTetris(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixDnBDancer(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixKanjiScroll(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixLarsonScanner(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRain(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixTwinkle(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixWipe(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixShoggoth(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRainbowZoom(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixBouncingDot(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSpinner(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixPong(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSwitchToggle(uint16_t runtime_ms, uint16_t frameDelay_ms);
void matrixCometSweep(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixPinkSpiral(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixGalagaInvader(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixLightning(uint16_t runtime_ms);
void matrixBlueLightning(uint16_t runtime_ms);
void matrixFlowerBurst(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixAlpineStorm(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixCocktail(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixHeartbeatCalm(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixHeartbeatRelentless(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixPinkPlasmaBoom(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixPinkVelvet(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixBreathAndRush(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixTricksterPlasma(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixTricksterPlasmaLoop(uint16_t stepDelay_ms);
void matrixFunkySchachBratz(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRainbowWash(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixFlagsShow(uint16_t runtime_ms, uint16_t hold_ms);
void matrixFlagsShowFade(uint16_t runtime_ms, uint16_t hold_ms, uint16_t fade_ms, uint8_t steps);
void matrixFiveEightSeam(uint16_t runtime_ms, uint16_t bpm8, uint8_t col);
void matrixCoteAzur(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSunsetPickleSun(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixLarsonScannerDual(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixExplosion(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixDigitalRain(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixMonoBlink5s(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixDigitalRainAmber(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixWaterfall(uint16_t runtime_ms, uint16_t stepDelay_ms);
static inline void matrixNightStreet2000(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixChristmasTreeAura(uint16_t runtime_ms, uint16_t stepDelay_ms, uint8_t sparkleMask);
void matrixCCCRocket(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixAntifaFlag(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixBeachWave(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixBurningSauna(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixMagentaMiniSnake(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixCounter1to9(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixUniverseCreation(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSpear(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixCCCRocket(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixChaoticPink(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSpringStorm(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixBlueBouncingBall(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixGreenBouncingBall(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixThaiLogo(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRandomStrobe(uint16_t runtime_ms);
void matrixClappingMusic(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixYellowRedBouncing(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixTechnoOrbit(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRocketLiftoff(uint16_t runtime_ms, uint16_t stepDelay_ms);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Visuals Index and Guide
========================================================================================================================
    __     __           _       _ _       
    \ \   / /__ _ __ __| | __ _| (_)_ __  
     \ \ / / _ \ '__/ _` |/ _` | | | '_ \ 
      \ V /  __/ | | (_| | (_| | | | | | |
       \_/ \___|_|  \__,_|\__,_|_|_|_| |_|

  List of available 5x4 matrix effects (runtime_ms, speed params...)

  - matrixRainbowWaves(runtime, stepDelay): Flowing rainbow stripes across the matrix.
  - matrixFire(runtime, cooling, sparking, stepDelay): Fire effect with hotter center and breathing edges.
  - matrixTetris(runtime, stepDelay): Simple falling blocks stack and clear rows.
  - matrixDnBDancer(runtime, stepDelay): Two-frame dancer pulsing to a bass beat.
  - matrixKanjiScroll(runtime, stepDelay): Scrolls a 10x4 kanji-like glyph across the 5x4 matrix.
  - matrixLarsonScanner(runtime, stepDelay): Classic Cylon/Knight Rider scanner with tails.
  - matrixRain(runtime, stepDelay): Blue raindrops fall with fading tails.
  - matrixTwinkle(runtime, stepDelay): Soft random color twinkles that fade.
  - matrixWipe(runtime, stepDelay): Wipe a solid color across rows, back and forth.
  - matrixShoggoth(runtime, stepDelay): Amorphous wobbling blob with blinking eyes.
  - matrixRainbowZoom(runtime, stepDelay): Concentric rainbow rings that zoom.
  - matrixBouncingDot(runtime, stepDelay): Single dot bounces with fading trail.
  - matrixSpinner(runtime, stepDelay): Dot runs around the matrix perimeter.
  - matrixPong(runtime, stepDelay): Mini Pong with AI paddles and bouncing ball.
  - matrixSwitchToggle(runtime, frameDelay): Animated switch body with moving lever.
  - matrixCometSweep(runtime, stepDelay): Bright head sweeps all pixels with trail.
  - matrixPinkSpiral(runtime, stepDelay): Pink dot traces an inward spiral path.
  - matrixGalagaInvader(runtime, stepDelay): Classic alien sprite flaps and sweeps.
  - matrixLightning(runtime): Random lightning strikes with glow and flicker.
  - matrixFlagsShow(runtime, hold_ms): Cycle through several 5x4 national flags.
  - matrixFlagsShowFade(runtime, hold_ms, fade_ms, steps): Flags with cross-fades.
  - matrixDigitalRain(runtime, stepDelay): Matrix movie-style green code rain adapted to 5x4.

  Tip: For very small flash budgets, comment-out heavier effects or guard them with macros.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// ===== Implementations =====

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFiveEightSeam
   Illuminate one matrix column as a 5/8 rhythm (5 eighths per bar) with non-repeating accents.
   - Strong beats (group starts) are bright; other beats are dim.
   - Accents vary per bar using a tiny LFSR and rotating start offset so it practically never repeats.
   Params: runtime_ms, bpm8 (eighth-notes per minute), col (0..MATRIX_W-1)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFiveEightSeam(uint16_t runtime_ms, uint16_t bpm8, uint8_t col) {
  if (col >= MATRIX_W) col = MATRIX_W - 1;
  uint32_t start = millis();
  if (bpm8 < 10) bpm8 = 10; // safety lower bound
  uint16_t stepMs = (uint16_t)(60000UL / bpm8); // one 8th-note in ms

  // LFSR for pseudo-random accent variation (16-bit, taps 16,14,13,11)
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBEEF);
  auto nextLfsr = [&]() {
    uint16_t bit = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (bit << 15));
    return lfsr;
  };

  // Accent pattern per bar built from either 3+2 or 2+3 grouping; rotated by offset
  uint8_t bar = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Choose grouping: 0 => [3,2], 1 => [2,3]
    bool two_three = (nextLfsr() & 1);
    uint8_t accents[5] = {0,0,0,0,0};
    if (two_three) { accents[0] = 1; accents[2] = 1; accents[3] = 1; } // 2+3 strong at 0 and 2 (and optional extra at 3)
    else            { accents[0] = 1; accents[3] = 1; }                 // 3+2 strong at 0 and 3
    // Rotate start offset by a value derived from LFSR and bar index
    uint8_t rot = (uint8_t)((nextLfsr() + bar) % 5);
    auto accAt = [&](uint8_t i) -> uint8_t { return accents[(i + rot) % 5]; };

    for (uint8_t i = 0; i < 5; i++) {
      // Compute on-time and off-time fractions for visibility
      uint16_t onMs = (uint16_t)(stepMs * 3 / 5);   // 60% on for richer color
      uint16_t offMs = (uint16_t)(stepMs - onMs);   // 40% off
      uint16_t mainMs = (uint16_t)(onMs * 2 / 3);   // primary hit duration
      uint16_t echoMs = (uint16_t)(onMs - mainMs);  // echo duration

      // Dynamic hue per beat, slowly rotating per bar for variety
      uint8_t baseHue = (uint8_t)((bar * 37 + i * 21) & 0xFF);
      bool strong = accAt(i) != 0;
      uint8_t mainHue = strong ? baseHue : (uint8_t)(baseHue + 28);
      uint8_t mainLevel = strong ? 255 : 150;   // strong beats brighter

      // Draw main seam column with a slight vertical hue gradient
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t yHue = (uint8_t)(mainHue + y * 10);
        uint32_t c = dimColor(Wheel(yHue), mainLevel);
        matrixSet(col, y, c);
      }


      // Light adjacent columns with falloff for a wider, more interesting look
      if (col > 0) {
        uint8_t leftLevel = (uint8_t)(mainLevel / 3);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(mainHue + 8 + y * 10);
          matrixSet((uint8_t)(col - 1), y, dimColor(Wheel(yHue), leftLevel));
        }
      }
      if (col + 1 < MATRIX_W) {
        uint8_t rightLevel = (uint8_t)(mainLevel / 3);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(mainHue - 8 + y * 10);
          matrixSet((uint8_t)(col + 1), y, dimColor(Wheel(yHue), rightLevel));
        }
      }
      // Occasional sparkle on strong beats
      if (strong && ((nextLfsr() & 0x03) == 0)) {
        uint8_t sy = (uint8_t)(nextLfsr() % MATRIX_H);
        matrixSet(col, sy, pixels.Color(255, 255, 255));
      }
      pixels.show();
      delay(mainMs);

      // Echo on mirrored seam in second half of the beat
      uint8_t mirrorCol = (uint8_t)(MATRIX_W - 1 - col);
      uint8_t echoHue = (uint8_t)(mainHue + 96);           // complementary-ish
      uint8_t echoLevel = strong ? 160 : 220;              // weak beats respond stronger
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t yHue = (uint8_t)(echoHue + y * 12);
        matrixSet(mirrorCol, y, dimColor(Wheel(yHue), echoLevel));
      }
      if (mirrorCol > 0) {
        uint8_t ml = (uint8_t)(echoLevel / 3);
        uint8_t mcL = (uint8_t)(mirrorCol - 1);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(echoHue + 6 + y * 12);
          matrixSet(mcL, y, dimColor(Wheel(yHue), ml));
        }
      }
      if (mirrorCol + 1 < MATRIX_W) {
        uint8_t mr = (uint8_t)(echoLevel / 3);
        uint8_t mcR = (uint8_t)(mirrorCol + 1);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(echoHue - 6 + y * 12);
          matrixSet(mcR, y, dimColor(Wheel(yHue), mr));
        }
      }
      pixels.show();
      delay(echoMs);

      // Off phase: gentle fade to leave trailing glow
      matrixFade(140);
      pixels.show();
      delay(offMs);

      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
    bar++;
  }
}

// Small rainbow planet that flies across with vertical wobble and trail
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowPlanet
   A rotating rainbow planet that moves slightly across the display.
   - Creates a 5x4 spherical illusion by computing distances to a moving center.
   - Colors cycle through a vibrant 8-bit rainbow palette.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowPlanet(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;

  auto tri8 = [](uint8_t v) -> uint8_t {
    return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1);
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Soft fade, aber etwas länger sichtbar
    matrixFade(88);

    uint8_t phase = t;

    // Basiszentrum in der Mitte, etwas stärkere kreisförmige Wobble-Bewegung
    int8_t baseX = (int8_t)(W / 2);      // 2 bei 5x4
    int8_t baseY = (int8_t)(H / 2);      // 2 bei 5x4

    uint8_t wobH = tri8((uint8_t)(phase * 2));        // 0..255
    uint8_t wobV = tri8((uint8_t)(phase * 3 + 37));   // 0..255, phasenverschoben
    int8_t cx = (int8_t)(baseX + (int8_t)((int16_t)(wobH - 128) / 90));  // ca. -2..+2
    int8_t cy = (int8_t)(baseY + (int8_t)((int16_t)(wobV - 128) / 120)); // etwas kleinerer Hub
    if (cx < 0) cx = 0;
    if (cx >= (int8_t)W) cx = (int8_t)W - 1;
    if (cy < 0) cy = 0;
    if (cy >= (int8_t)H) cy = (int8_t)H - 1;

    // Planetfarben: heller Kern, bunte Regenbogen-Bänder über der Kugel, deutlich rotierend
    uint8_t bandPhase = (uint8_t)(phase * 2);   // schnellerer Band-Offset

    for (int8_t dy = -1; dy <= 1; dy++) {
      for (int8_t dx = -1; dx <= 1; dx++) {
        int8_t px = (int8_t)cx + dx;
        int8_t py = (int8_t)cy + dy;
        if (px < 0 || py < 0 || px >= (int8_t)W || py >= (int8_t)H) continue;

        int8_t adx = dx < 0 ? -dx : dx;
        int8_t ady = dy < 0 ? -dy : dy;
        uint8_t dist = (uint8_t)(adx + ady);
        if (dist > 2) continue; // 3x3 Scheibe maximal

        // Helligkeit radial: Mitte heller, Rand dunkler (Kugelgefühl)
        uint8_t radLevel = (uint8_t)(2 - dist); // 2,1,0
        uint8_t bright = (uint8_t)(180 + radLevel * 30); // 180..240

        // Regenbogen-Bänder: abhängig von "Breiten"-Position (dx), Höhe (dy) und Bandphase
        uint8_t band = (uint8_t)((dx + 1) * 40 + (dy + 1) * 16); // leichte Neigung über y
        uint8_t hue = (uint8_t)(bandPhase * 4 + band);
        uint32_t baseCol = Wheel(hue);
        uint32_t shaded = dimColor(baseCol, bright);
        matrixSet((uint8_t)px, (uint8_t)py, shaded);
      }
    }

    // Occasional tiny star in the background
    if ((t & 0x07) == 0) {
      uint8_t sx = (uint8_t)(random(W));
      uint8_t sy = (uint8_t)(random(H));
      if (!(sx == cx && sy == cy)) {
        matrixSet(sx, sy, dimColor(pixels.Color(200, 220, 255), 140));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixMonoBlink5s
   Every 5 seconds, blink exactly one random pixel briefly. All others remain off.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixMonoBlink5s(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  // Tiny LFSR for reproducible randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0x5123);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  matrixFill(0);
  pixels.show();

  uint32_t nextAt = start + 5000UL;
  bool lit = false;
  uint32_t offAt = 0;
  // Simple pentatonic scale (Hz)
  const uint16_t scale[5] = {262, 294, 330, 392, 440};
  uint8_t noteIdx = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint32_t now = millis();

    if (!lit && now >= nextAt) {
      matrixFill(0);
      uint8_t x = (uint8_t)(nextL() % W);
      uint8_t y = (uint8_t)(nextL() % H);
      matrixSet(x, y, pixels.Color(255, 255, 255));
      pixels.show();
      lit = true;
      offAt = now + 120UL;      // blink duration ~120ms
      nextAt = now + 5000UL;    // schedule next blink
      // Play tone for melody
      uint16_t f = scale[noteIdx % 5];
      // occasional octave up
      if ((nextL() & 0x07) == 0 && f <= 600) f = (uint16_t)(f * 2);
      playSound(f, (uint16_t)100);
      noteIdx++;
    }

    if (lit && now >= offAt) {
      matrixFill(0);
      pixels.show();
      lit = false;
    }

    delay(stepDelay_ms);
  }
}

// Simple starry sky with twinkling stars on a dark background
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixStarrySky
   A classic twinkling starry sky on a dark night background.
   - Smoothly fades stars in and out to simulate atmospheric shimmering.
   - Occasional brighter blue/white twinkles add depth.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixStarrySky(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // Per-pixel brightness 0..255
  uint8_t bri[20];
  for (uint8_t i = 0; i < W * H; i++) bri[i] = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Global gentle darkening
    matrixFade(96);

    // Randomly nudge brightness for each pixel
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t idx = (uint8_t)(y * W + x);
        uint8_t v = bri[idx];
        // Occasionally spawn a brighter star
        if (v == 0) {
          if ((uint8_t)random(64) == 0) {
            v = (uint8_t)(180 + (random(60))); // bright new star
          }
        } else {
          // Small random walk up/down
          int8_t delta = 0;
          uint8_t r = (uint8_t)random(4);
          if (r == 0) delta = 1;
          else if (r == 1) delta = -1;
          int16_t nv = (int16_t)v + delta;
          if (nv < 0) nv = 0;
          if (nv > 255) nv = 255;
          v = (uint8_t)nv;
          // Rarely let star fade out
          if ((uint8_t)random(80) == 0 && v > 0) v = (uint8_t)(v / 2);
        }
        bri[idx] = v;

        if (v > 0) {
          // Slightly bluish-white tint
          uint8_t b = v;
          uint8_t g = (uint8_t)(v * 3 / 4);
          uint8_t rch = (uint8_t)(v / 3);
          matrixSet(x, y, pixels.Color(rch, g, b));
        }
      }
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixChristmasTreeAura
   Small central green tree with a soft, slow color-changing "aura" of pixels
   on the border around the tree. Tree stays mostly stable; edge pixels fade
   in and out individually in warm/cool Christmas tones.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixChristmasTreeAura(uint16_t runtime_ms, uint16_t stepDelay_ms, uint8_t sparkleMask) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // 5x4 tree bitmap, row-major, 1 = tree, 2 = trunk
  const uint8_t treeBmp[20] = {
    // y = 0 (top)
    0,0,1,0,0,
    // y = 1
    0,1,1,1,0,
    // y = 2
    1,1,1,1,1,
    // y = 3 (bottom, trunk in center)
    0,0,2,0,0
  };

  uint16_t lfsr = (uint16_t)(millis() ^ 0xC123);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  auto treeVal = [&](uint8_t x, uint8_t y) -> uint8_t {
    if (x >= 5 || y >= 4) return 0;
    return treeBmp[y * 5 + x];
  };

  auto isTree = [&](uint8_t x, uint8_t y) -> bool {
    return treeVal(x, y) != 0;
  };

  auto isTreeEdge = [&](uint8_t x, uint8_t y) -> bool {
    if (isTree(x, y)) return false;
    // Any 4-neighbor that is tree -> considered edge pixel
    if (x > 0      && isTree((uint8_t)(x - 1), y)) return true;
    if (x + 1 < W  && isTree((uint8_t)(x + 1), y)) return true;
    if (y > 0      && isTree(x, (uint8_t)(y - 1))) return true;
    if (y + 1 < H  && isTree(x, (uint8_t)(y + 1))) return true;
    return false;
  };

  matrixFill(0);
  pixels.show();

  // Frame counter for slow color breathing and motion; keep across calls.
  // Use 16-bit so our derived 8-bit phase (via shifts) cycles smoothly.
  static uint16_t tPhase = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Global fade: slightly stronger decay so sparkles do not linger too long
    matrixFade(40);

    // Draw the tree body fresh each frame so it stays solid
    uint8_t breathPhase = (uint8_t)(tPhase >> 2); // slowish
    // Simple triangle 0..255 from phase
    uint8_t breath = (breathPhase & 0x80) ? (uint8_t)(255 - ((breathPhase & 0x7F) << 1))
                                          : (uint8_t)((breathPhase & 0x7F) << 1);
    uint8_t level = (uint8_t)(190 + (breath >> 3)); // 190..~221

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t v = treeVal(x, y);
        if (!v) continue;
        if (v == 2) {
          // Trunk: warm brown, mostly constant
          uint32_t trunk = pixels.Color(120, 60, 10);
          matrixSet(x, y, trunk);
        } else {
          // Needley color that shifts green <-> blue using a simple triangle wave
          // with no discontinuity. We blend directly between two RGB colors.

          // Slow phase for color motion
          uint8_t phase = (uint8_t)(tPhase >> 3);  // slower = smoother

          // Triangle wave 0..255: 0→255→0 with no jumps
          // Up-slope for 0..127, down-slope for 128..255
          uint8_t tri;
          if (phase < 128) {
            tri = (uint8_t)(phase << 1);             // 0..254
          } else {
            tri = (uint8_t)((255 - phase) << 1);     // 254..0
          }

          // Base colors for the tree: deep green and softer blue/teal
          uint8_t gR = 10,  gG = 190, gB = 40;   // deep green
          uint8_t bR = 25,  bG = 170, bB = 210;  // gentle blue/teal

          // Blend green <-> blue using tri (0..255)
          uint8_t r = (uint8_t)(((uint16_t)gR * (255 - tri) + (uint16_t)bR * tri) / 255);
          uint8_t g = (uint8_t)(((uint16_t)gG * (255 - tri) + (uint16_t)bG * tri) / 255);
          uint8_t b = (uint8_t)(((uint16_t)gB * (255 - tri) + (uint16_t)bB * tri) / 255);

          // Apply breathing brightness level on top
          uint32_t base = pixels.Color(r, g, b);
          uint32_t col  = dimColor(base, level);
          matrixSet(x, y, col);
        }
      }
    }

    // Occasionally spawn/refresh aura pixels on the tree edge.
    // Single new sparkle per frame (if any), so they feel more independent.
    if (sparkleMask && ((nextL() & sparkleMask) == 0)) {
      for (uint8_t tries = 0; tries < 6; tries++) {  // fewer attempts per frame
        uint8_t rx = (uint8_t)(nextL() % W);
        uint8_t ry = (uint8_t)(nextL() % H);
        if (!isTreeEdge(rx, ry)) continue;

        // Christmas color palette: warm white, red, gold, icy blue
        uint8_t pick = (uint8_t)(nextL() & 0x03);
        uint32_t c;
        switch (pick) {
          default:
          case 0: c = pixels.Color(255, 220, 180); break; // warm white
          case 1: c = pixels.Color(255, 40, 40);    break; // red
          case 2: c = pixels.Color(255, 200, 60);   break; // gold
          case 3: c = pixels.Color(120, 200, 255);  break; // icy blue
        }
        // Slight random brightness so not all are equal
        uint8_t bri = (uint8_t)(120 + (nextL() & 0x3F));
        c = dimColor(c, bri);

        // Fade-in more gently: blend new color with existing pixel instead of hard overwrite
        int idx = matrixIndex(rx, ry);
        uint32_t prev = pixels.getPixelColor(idx);
        uint8_t pr = (prev >> 16) & 0xFF;
        uint8_t pg = (prev >> 8) & 0xFF;
        uint8_t pb = prev & 0xFF;
        uint8_t tr = (c >> 16) & 0xFF;
        uint8_t tg = (c >> 8) & 0xFF;
        uint8_t tb = c & 0xFF;
        // Low alpha so new sparkles ramp up over multiple frames
        const uint8_t alpha = 50; // ~31% toward target
        uint8_t r = (uint16_t)pr * (255 - alpha) / 255 + (uint16_t)tr * alpha / 255;
        uint8_t g = (uint16_t)pg * (255 - alpha) / 255 + (uint16_t)tg * alpha / 255;
        uint8_t b = (uint16_t)pb * (255 - alpha) / 255 + (uint16_t)tb * alpha / 255;
        matrixSet(rx, ry, pixels.Color(r, g, b));
        break; // only one new sparkle this frame
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    tPhase++;
  }
}


// Tiled rainbow noise field inspired by Perlin_Tiled.cs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowTiledNoise
   A generative noise pattern mapping Perlin-like cellular noise to a rainbow palette.
   - Creates an organic, flowing lava-lamp or stained glass effect.
   - The palette is smoothly shifted over time for endless variation.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowTiledNoise(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;

  auto hash8 = [](int16_t x, int16_t y) -> uint8_t {
    uint16_t h = (uint16_t)(x * 374761 + y * 668265);
    h ^= (uint16_t)(h >> 7);
    h *= 28921U;
    h ^= (uint16_t)(h >> 9);
    return (uint8_t)h;
  };

  auto lerp8 = [](uint8_t a, uint8_t b, uint8_t w) -> uint8_t {
    return (uint8_t)(((uint16_t)a * (255 - w) + (uint16_t)b * w) / 255);
  };

  auto fade8 = [](uint8_t v) -> uint8_t {
    uint16_t t16 = v;
    uint16_t t2 = (uint16_t)((t16 * t16) >> 8);
    uint16_t t3 = (uint16_t)((t2 * t16) >> 8);
    uint16_t f = (uint16_t)(6 * t3 - 15 * t2 + 10 * t16);
    return (uint8_t)(f > 255 ? 255 : f);
  };

  auto noise2 = [&](uint16_t x, uint16_t y) -> uint8_t {
    // "Tile" by wrapping coordinates over a small grid
    uint16_t sx = (uint16_t)((x >> 4) & 0x07); // 0..7
    uint16_t sy = (uint16_t)((y >> 4) & 0x07);

    uint8_t fx = (uint8_t)(x & 0x0F) * 17;
    uint8_t fy = (uint8_t)(y & 0x0F) * 17;
    uint8_t ux = fade8(fx);
    uint8_t uy = fade8(fy);

    uint8_t c00 = hash8((int16_t)sx, (int16_t)sy);
    uint8_t c10 = hash8((int16_t)((sx + 1) & 0x07), (int16_t)sy);
    uint8_t c01 = hash8((int16_t)sx, (int16_t)((sy + 1) & 0x07));
    uint8_t c11 = hash8((int16_t)((sx + 1) & 0x07), (int16_t)((sy + 1) & 0x07));

    uint8_t x0 = lerp8(c00, c10, ux);
    uint8_t x1 = lerp8(c01, c11, ux);
    return lerp8(x0, x1, uy);
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);

    uint16_t scroll = (uint16_t)(t * 10);
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint16_t sx = (uint16_t)(x * 32 + (scroll & 0xFF));
        uint16_t sy = (uint16_t)(y * 40 + ((scroll >> 1) & 0xFF));
        uint8_t n = noise2(sx, sy); // 0..255

        // Use noise as hue offset and brightness
        uint8_t hue = (uint8_t)(n + t * 3);
        uint8_t bri = (uint8_t)(160 + (uint16_t)n * 80 / 255); // 160..~240
        uint32_t base = Wheel(hue);
        uint32_t col = dimColor(base, bri);
        matrixSet(x, y, col);
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

// Soft fog/cloud effect using a simple random-walk brightness field
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFogNoise
   A moody, thick atmospheric fog simulation using grayscale noise.
   - Fading and blending algorithm simulates dense shifting mist.
   - Subtle variations in brightness create a convincing volumetric effect.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFogNoise(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // Per-pixel fog intensity 0..255
  uint8_t bri[20];
  for (uint8_t i = 0; i < W * H; i++) bri[i] = 120; // start mid-gray

  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t idx = (uint8_t)(y * W + x);
        uint8_t v = bri[idx];

        // Gentle drift: small random step up/down
        int8_t delta = 0;
        uint8_t r = (uint8_t)random(5);
        if (r == 0) delta = 1;
        else if (r == 1) delta = -1;

        // Slight attraction toward average of neighbors for smoothness
        uint16_t sum = v;
        uint8_t cnt = 1;
        if (x > 0)         { sum += bri[idx - 1];     cnt++; }
        if (x + 1 < W)     { sum += bri[idx + 1];     cnt++; }
        if (y > 0)         { sum += bri[idx - W];     cnt++; }
        if (y + 1 < H)     { sum += bri[idx + W];     cnt++; }
        uint8_t avg = (uint8_t)(sum / cnt);

        int16_t nv = (int16_t)v + delta + (int16_t)((int8_t)(avg - v) / 6);
        if (nv < 70) nv = 70;       // avoid full black
        if (nv > 210) nv = 210;     // avoid full white
        v = (uint8_t)nv;
        bri[idx] = v;

        // cool gray-blue fog color
        uint8_t rch = (uint8_t)(v / 4);
        uint8_t gch = (uint8_t)(v * 3 / 4);
        uint8_t bch = v;
        matrixSet(x, y, pixels.Color(rch, gch, bch));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixDigitalRainAmber
   Matrix-style code rain in warm amber/brown/yellow tones.
   Same mechanics as matrixDigitalRain, with a warm palette and slightly softer trails.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixDigitalRainAmber(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0x5A3C);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  uint8_t headY[8];
  uint8_t speed[8];
  uint8_t tick[8];
  for (uint8_t x = 0; x < W; x++) {
    headY[x] = 0xFF; // inactive
    speed[x] = (uint8_t)(1 + (nextL() % 3));
    tick[x] = 0;
  }

  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Slightly softer trails than green version
    matrixFade(74);

    for (uint8_t x = 0; x < W; x++) {
      if (headY[x] == 0xFF) {
        if ((nextL() & 0x07) == 0) { // ~1/8 chance per frame per column
          headY[x] = 0; // top
          speed[x] = (uint8_t)(1 + (nextL() % 3));
          tick[x] = 0;
        }
      } else {
        tick[x]++;
        if ((tick[x] % speed[x]) == 0) {
          if (headY[x] + 1 < H) headY[x]++;
          else { headY[x] = 0xFF; continue; }
        }

        uint8_t y = headY[x];
        uint32_t headCol = pixels.Color(255, 240, 120); // bright warm yellow
        matrixSet(x, y, headCol);
        if (y > 0) {
          uint32_t bodyCol = pixels.Color(180, 110, 30); // brownish trail
          matrixSet(x, (uint8_t)(y - 1), dimColor(bodyCol, 200));
        }
      }
    }

    // Rare glyph flicker in warm amber
    if ((nextL() & 0x0F) == 0) {
      uint8_t rx = (uint8_t)(nextL() % W);
      uint8_t ry = (uint8_t)(nextL() % H);
      matrixSet(rx, ry, pixels.Color(255, 210, 90));
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBreathAndRush
   Alternates between energetic, lively phases and calm, breathing phases.
   - Rush: bright pink/orange pulses with sparkles and quick drift
   - Calm: slow magenta breathing wash with soft fade
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBreathAndRush(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t t = 0;
  uint16_t lfsr = (uint16_t)(millis() ^ 0x9B1E);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };
  auto tri8 = [](uint8_t v) -> uint8_t { return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1); };

  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint16_t e = (uint16_t)(millis() - start);
    bool rush = (((uint16_t)(e / 4000U)) & 1U) == 0; // 4s Rush, 4s Calm alternating

    if (rush) {
      // Energetic: stronger fade (short trails), bright warm palette, quick drift
      matrixFade(96);
      uint8_t ox = (uint8_t)(t * 2);
      uint8_t oy = (uint8_t)(t * 3);
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t w = (uint8_t)((tri8((uint8_t)(x * 41 + ox)) + tri8((uint8_t)(y * 59 + oy))) >> 1);
          // Warm punchy gradient: orange→pink
          uint8_t r0 = 255, g0 = 120, b0 = 20;
          uint8_t r1 = 255, g1 = 60,  b1 = 180;
          uint8_t r = (uint8_t)(((uint16_t)r0 * (255 - w) + (uint16_t)r1 * w) / 255);
          uint8_t g = (uint8_t)(((uint16_t)g0 * (255 - w) + (uint16_t)g1 * w) / 255);
          uint8_t b = (uint8_t)(((uint16_t)b0 * (255 - w) + (uint16_t)b1 * w) / 255);
          matrixSet(x, y, pixels.Color(r, g, b));
        }
      }
      // Sparkles
      if ((nextL() & 0x03) == 0) {
        uint8_t sx = (uint8_t)(nextL() % W);
        uint8_t sy = (uint8_t)(nextL() % H);
        matrixSet(sx, sy, pixels.Color(255, 255, 255));
      }
    } else {
      // Calm: softer fade (longer trails), breathing brightness
      matrixFade(70);
      uint8_t breath = tri8((uint8_t)(t)); // 0..255
      uint8_t base = (uint8_t)(120 + (breath >> 2)); // 120..~183
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          // Deep magenta base, modulated by breath
          uint32_t c = pixels.Color((uint8_t)(base + 80), (uint8_t)(breath >> 3), (uint8_t)(base / 2));
          matrixSet(x, y, dimColor(c, (uint8_t)(160 + (breath >> 3))));
        }
      }
      // Occasional soft twinkle in calm
      if ((nextL() & 0x1F) == 0) {
        uint8_t sx = (uint8_t)(nextL() % W);
        uint8_t sy = (uint8_t)(nextL() % H);
        matrixSet(sx, sy, dimColor(pixels.Color(255, 160, 220), 140));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}
 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixPinkPlasmaBoom
   Pink plasma field with gentle motion and occasional small explosion bursts.
   Lightweight integer math, fits 5x4.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixPinkPlasmaBoom(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t t = 0;
  // Up to two concurrent small explosion bursts (minimal RAM)
  bool active[2] = {false, false};
  uint8_t bx[2] = {2, 3}, by[2] = {2, 1};
  uint8_t rad[2] = {0, 0}, life[2] = {0, 0};
  // Tiny LFSR for pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0xC0DE);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Base fade to leave trails
    matrixFade(72);

    // Pink palette endpoints
    const uint32_t DARK = pixels.Color(60, 10, 30);
    const uint32_t LITE = pixels.Color(255, 100, 200);

    // Animate three integer phases for interference
    uint8_t p1 = (uint8_t)(t * 5);
    uint8_t p2 = (uint8_t)(t * 3 + 47);
    uint8_t p3 = (uint8_t)(t * 2 + 99);

    // Inline triangle function 0..255
    auto tri8 = [](uint8_t v) -> uint8_t { return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1); };

    // Draw plasma
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t a = tri8((uint8_t)(x * 37 + p1));
        uint8_t b = tri8((uint8_t)(y * 53 + p2));
        uint8_t c = tri8((uint8_t)((x + y) * 29 + p3));
        uint16_t mix = (uint16_t)a + (uint16_t)b + (uint16_t)c; // 0..(3*255)
        uint8_t w = (uint8_t)(mix / 3); // 0..255
        // Blend dark->light pink
        uint8_t rD = (DARK >> 16) & 0xFF, gD = (DARK >> 8) & 0xFF, bD = DARK & 0xFF;
        uint8_t rL = (LITE >> 16) & 0xFF, gL = (LITE >> 8) & 0xFF, bL = LITE & 0xFF;
        uint8_t r = (uint16_t)rD * (255 - w) / 255 + (uint16_t)rL * w / 255;
        uint8_t g = (uint16_t)gD * (255 - w) / 255 + (uint16_t)gL * w / 255;
        uint8_t bch = (uint16_t)bD * (255 - w) / 255 + (uint16_t)bL * w / 255;
        matrixSet(x, y, pixels.Color(r, g, bch));
      }
    }

    // More frequent booms; try to spawn into a free slot (~1/32 chance per frame)
    if ((nextL() & 0x1F) == 0) {
      for (uint8_t i = 0; i < 2; i++) {
        if (!active[i]) {
          bx[i] = (uint8_t)(nextL() % W);
          by[i] = (uint8_t)(nextL() % H);
          rad[i] = 0; life[i] = 12; active[i] = true;
          playSound((uint16_t)(190 + (nextL() & 0x3F)), (uint16_t)28);
          break;
        }
      }
    }

    // Render boom rings and decay for all active slots
    for (uint8_t i = 0; i < 2; i++) {
      if (!active[i] || life[i] == 0) continue;
      uint8_t r2 = (uint8_t)(rad[i] * rad[i]);
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          int8_t dx = (int8_t)x - (int8_t)bx[i];
          int8_t dy = (int8_t)y - (int8_t)by[i];
          uint16_t d2 = (uint16_t)((int16_t)dx * (int16_t)dx + (int16_t)dy * (int16_t)dy);
          int16_t diff = (int16_t)d2 - (int16_t)r2;
          if (diff < 0) diff = (int16_t)-diff;
          if (diff <= 1) {
            uint8_t heat = (uint8_t)(200 + (life[i] * 5));
            if (heat > 255) heat = 255;
            matrixSet(x, y, pixels.Color(255, heat, (uint8_t)(heat / 6)));
          }
        }
      }
      rad[i]++;
      if (life[i] > 0) life[i]--; else active[i] = false;
      if (life[i] == 0) active[i] = false;
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

// Soft velvet pink plasma with gentle shimmer
inline void matrixPinkVelvet(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t t = 0;
  uint16_t lfsr = (uint16_t)(millis() ^ 0xA55A);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  auto tri8 = [](uint8_t v) -> uint8_t {
    return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1);
  };

  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(78);
    uint8_t p1 = (uint8_t)(t * 3);
    uint8_t p2 = (uint8_t)(t * 2 + 37);

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t a = tri8((uint8_t)(x * 40 + p1));
        uint8_t b = tri8((uint8_t)(y * 58 + p2));
        uint8_t w = (uint8_t)(((uint16_t)a * 3 + (uint16_t)b * 2) / 5);
        uint8_t r0 = 80,  g0 = 0,   b0 = 40;   // deep magenta
        uint8_t r1 = 255, g1 = 80,  b1 = 180;  // hot pink
        uint8_t r = (uint8_t)(((uint16_t)r0 * (255 - w) + (uint16_t)r1 * w) / 255);
        uint8_t g = (uint8_t)(((uint16_t)g0 * (255 - w) + (uint16_t)g1 * w) / 255);
        uint8_t bl= (uint8_t)(((uint16_t)b0 * (255 - w) + (uint16_t)b1 * w) / 255);
        matrixSet(x, y, pixels.Color(r, g, bl));
      }
    }

    if ((nextL() & 0x0F) == 0) {
      uint8_t sx = (uint8_t)(nextL() % W);
      uint8_t sy = (uint8_t)(nextL() % H);
      matrixSet(sx, sy, pixels.Color(255, 180, 220));
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

// Wobbling pink slime at the bottom with bright sprouts
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixPinkSlimeSprouts
   An organic, generative simulation of creeping pink vines or slime molds.
   - Roots grow downwards and occasionally branch outwards.
   - Uses warm pink, magenta, and purple hues against a dark background.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixPinkSlimeSprouts(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;

  // Tiny LFSR for pseudo-random sprouts
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBEEF);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Per-column sprout age (0 = none)
  uint8_t sprAge[5];
  for (uint8_t i = 0; i < 5; i++) sprAge[i] = 0;

  auto tri8 = [](uint8_t v) -> uint8_t {
    return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1);
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Gentle fade so slime and sprouts smear a bit
    matrixFade(80);

    uint8_t baseY = (uint8_t)(H - 1); // bottom row

    // Draw wobbling slime band at the bottom
    for (uint8_t x = 0; x < W; x++) {
      uint8_t ph = (uint8_t)(t * 4 + x * 40);
      uint8_t w = tri8(ph); // 0..255
      // Height 1 or 2 depending on wave
      uint8_t h = (w > 140) ? 2 : 1;

      for (uint8_t iy = 0; iy < h; iy++) {
        int8_t yy = (int8_t)baseY - (int8_t)iy;
        if (yy < 0) continue;
        // Pink slime gradient: darker near bottom, lighter just above
        uint8_t r0 = 120, g0 = 10,  b0 = 60;   // deep magenta
        uint8_t r1 = 255, g1 = 80,  b1 = 200;  // hot pink
        uint8_t mix = (uint8_t)(200 - iy * 40);
        uint8_t r = (uint8_t)(((uint16_t)r0 * (255 - mix) + (uint16_t)r1 * mix) / 255);
        uint8_t g = (uint8_t)(((uint16_t)g0 * (255 - mix) + (uint16_t)g1 * mix) / 255);
        uint8_t bl= (uint8_t)(((uint16_t)b0 * (255 - mix) + (uint16_t)b1 * mix) / 255);
        matrixSet(x, (uint8_t)yy, pixels.Color(r, g, bl));
      }
    }

    // Update and render sprouts
    for (uint8_t x = 0; x < W; x++) {
      // Spawn new sprout occasionally if none active in this column
      if (sprAge[x] == 0) {
        if ((nextL() & 0x1F) == 0) {
          sprAge[x] = 10; // lifespan in frames
        }
      }

      if (sprAge[x] > 0) {
        // Map age to vertical position: start near slime and grow upward
        // Age 10 -> just above slime, Age 1 -> near top
        uint8_t maxRise = (uint8_t)((H > 1) ? (H - 1) : 0);
        uint8_t step = (uint8_t)((sprAge[x] > 0) ? (sprAge[x] - 1) : 0);
        uint8_t rise = (uint8_t)((uint16_t)step * maxRise / 9); // 0..maxRise
        int8_t yy = (int8_t)baseY - (int8_t)(1 + rise);
        if (yy >= 0 && yy < (int8_t)H) {
          // Bright pink/white sprout tip
          uint8_t fade = (uint8_t)(200 + sprAge[x] * 5); // 200..250
          if (fade > 255) fade = 255;
          uint32_t tip = pixels.Color(255, 180, 240);
          matrixSet(x, (uint8_t)yy, dimColor(tip, fade));
        }
        sprAge[x]--;
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixMilkySea
   A flowing, bio-luminescent milky sea simulation.
   - Slow, gentle noise fields mapped to cyan, blue, and white tones.
   - Mimics the glowing phenomena seen in noctiluca scintillans algae blooms.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixMilkySea(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;

  uint16_t lfsr = (uint16_t)(millis() ^ 0x1234);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  uint8_t dripAge[5];
  for (uint8_t i = 0; i < 5; i++) dripAge[i] = 0;

  auto tri8 = [](uint8_t v) -> uint8_t {
    return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1);
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(90);

    uint8_t baseY = (uint8_t)(H - 1);

    for (uint8_t x = 0; x < W; x++) {
      uint8_t ph = (uint8_t)(t * 3 + x * 32);
      uint8_t w = tri8(ph);
      uint8_t h = (w > 160) ? 2 : 1;

      for (uint8_t iy = 0; iy < h; iy++) {
        int8_t yy = (int8_t)baseY - (int8_t)iy;
        if (yy < 0) continue;
        uint8_t r0 = 220, g0 = 220, b0 = 235;
        uint8_t r1 = 255, g1 = 255, b1 = 255;
        uint8_t mix = (uint8_t)(210 - iy * 40);
        uint8_t r = (uint8_t)(((uint16_t)r0 * (255 - mix) + (uint16_t)r1 * mix) / 255);
        uint8_t g = (uint8_t)(((uint16_t)g0 * (255 - mix) + (uint16_t)g1 * mix) / 255);
        uint8_t bl= (uint8_t)(((uint16_t)b0 * (255 - mix) + (uint16_t)b1 * mix) / 255);
        matrixSet(x, (uint8_t)yy, pixels.Color(r, g, bl));
      }
    }

    for (uint8_t x = 0; x < W; x++) {
      if (dripAge[x] == 0) {
        if ((nextL() & 0x1F) == 0) {
          dripAge[x] = 8;
        }
      }

      if (dripAge[x] > 0) {
        uint8_t maxRise = (uint8_t)((H > 1) ? (H - 1) : 0);
        uint8_t step = (uint8_t)(dripAge[x] - 1);
        uint8_t rise = (uint8_t)((uint16_t)step * maxRise / 7);
        int8_t yy = (int8_t)baseY - (int8_t)(1 + rise);
        if (yy >= 0 && yy < (int8_t)H) {
          uint8_t fade = (uint8_t)(210 + dripAge[x] * 5);
          if (fade > 255) fade = 255;
          uint32_t tip = pixels.Color(255, 255, 255);
          matrixSet(x, (uint8_t)yy, dimColor(tip, fade));
        }
        dripAge[x]--;
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

// Rainbow half-donut that spins colors around the arc
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowHalfDonut
   A stylized, rotating half-donut arc sweeping across the matrix.
   - The arc displays a cross-section of a dynamic rainbow gradient.
   - Rotates rhythmically back and forth or fully around the center.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowHalfDonut(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint8_t t = 0;

  // Fixed geometry for a half-donut on 5x4:
  // Outer arc:  (0,1) (1,0) (2,0) (3,0) (4,1)
  // Inner arc:  (1,1) (2,1) (3,1)
  const uint8_t outerCount = 5;
  const uint8_t innerCount = 3;
  const uint8_t outerX[5] = {0,1,2,3,4};
  const uint8_t outerY[5] = {1,0,0,0,1};
  const uint8_t innerX[3] = {1,2,3};
  const uint8_t innerY[3] = {1,1,1};

  auto tri8 = [](uint8_t v) -> uint8_t {
    return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1);
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);

    // Slower phase for calmer rotation
    uint8_t phase = (uint8_t)(t >> 1); // half speed

    // Outer ring: brighter, with stronger 3D depth via brightness
    for (uint8_t i = 0; i < outerCount; i++) {
      uint8_t idx = (uint8_t)((i + phase) % outerCount);
      uint8_t x = outerX[idx];
      uint8_t y = outerY[idx];
      uint8_t hue = (uint8_t)(phase * 8 + i * 40);
      uint32_t c = Wheel(hue);
      // Depth: front of arc = brighter, back = dimmer
      uint8_t depth = tri8((uint8_t)(i * 64 + phase * 6)); // 0..255
      // Map depth to strong front/back contrast (~110..255)
      uint8_t dim = (uint8_t)(110 + (depth * 145) / 255);
      matrixSet(x, y, dimColor(c, dim));
    }

    // Inner ring: slightly dimmer than outer, also depth-shaded
    for (uint8_t i = 0; i < innerCount; i++) {
      uint8_t idx = (uint8_t)((i + phase / 2) % innerCount);
      uint8_t x = innerX[idx];
      uint8_t y = innerY[idx];
      uint8_t hue = (uint8_t)(phase * 8 + 20 + i * 40);
      uint32_t c = Wheel(hue);
      uint8_t depth = tri8((uint8_t)(i * 80 + phase * 5));
      // Inner ring range a bit lower (~80..210)
      uint8_t dim = (uint8_t)(80 + (depth * 130) / 255);
      matrixSet(x, y, dimColor(c, dim));
    }

    // Slight glow at the very top center as a subtle highlight
    if (MATRIX_H >= 3) {
      uint8_t hy = 0;
      uint8_t hx = MATRIX_W / 2;
      uint32_t hc = dimColor(Wheel((uint8_t)(phase * 8 + 160)), 120);
      matrixSet(hx, hy, hc);
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

inline void matrixHeartbeatRelentless(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  static const uint8_t PROGMEM HEART[20] = {
    0,1,0,1,0,
    1,1,1,1,1,
    0,1,1,1,0,
    0,0,1,0,0
  };
  uint16_t intervalMs = 480;
  uint16_t dubGapMs = 140;
  uint32_t nextBeatAt = millis();
  uint32_t nextDubAt = 0;
  uint32_t lastBeatAt = 0;
  uint32_t lastDubAt = 0;
  auto env = [&](uint32_t since) -> uint8_t {
    if (since > 160U) return 0;
    uint16_t s = (uint16_t)since;
    uint16_t up = (s < 45U) ? (uint16_t)(s * 6U) : 270U;
    uint16_t down = (s > 45U) ? (uint16_t)(270U - ((s - 45U) * 3U)) : up;
    return (down > 255U) ? 255U : (uint8_t)down;
  };
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint32_t now = millis();
    if (now >= nextBeatAt) { lastBeatAt = now; nextBeatAt = now + intervalMs; nextDubAt = now + dubGapMs; }
    if (nextDubAt && now >= nextDubAt) { lastDubAt = now; nextDubAt = 0; }
    matrixFade(80);
    uint8_t amp1 = (lastBeatAt ? env(now - lastBeatAt) : 0);
    uint8_t amp2 = (lastDubAt  ? env(now - lastDubAt)  : 0);
    uint8_t amp = (amp1 > amp2) ? amp1 : amp2;
    uint8_t briCore = amp;
    uint8_t briGlow = (uint8_t)(amp / 3);
    uint32_t coreCol = dimColor(pixels.Color(255, 60, 80), (uint8_t)(160 + (briCore >> 2)));
    uint32_t glowCol = dimColor(pixels.Color(255, 40, 60), (uint8_t)(80 + (briGlow >> 2)));
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t v = pgm_read_byte(&HEART[y * 5 + x]);
        if (v) {
          matrixSet(x, y, coreCol);
          if (x > 0) matrixSet((uint8_t)(x - 1), y, glowCol);
          if (x + 1 < W) matrixSet((uint8_t)(x + 1), y, glowCol);
          if (y > 0) matrixSet(x, (uint8_t)(y - 1), glowCol);
          if (y + 1 < H) matrixSet(x, (uint8_t)(y + 1), glowCol);
        }
      }
    }
    if (amp1 > 220) { playSound((uint16_t)(150 + (random(20))), (uint16_t)8); }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFunkySchachBratz
   Funky Checkerboard (Schachbrett) in frechen Bratz-Farben (Neon-Pink/Lila vs. Cyan/Lime),
   mit driftender Verzerrung, Puls, gelegentlichem Invert-Glitch und Sparkles.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFunkySchachBratz(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t t = 0;
  // LFSR für Glitches/Sparkles
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBADA);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  auto tri8 = [](uint8_t v) -> uint8_t { return (v & 0x80) ? (uint8_t)(255 - ((v & 0x7F) << 1)) : (uint8_t)((v & 0x7F) << 1); };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // sanfte Nachleuchten (etwas stärker für weniger harte Kanten)
    matrixFade(90);

    // driftende Offsets und Puls
    uint8_t ox = (uint8_t)(t >> 2);
    uint8_t oy = (uint8_t)(t >> 3);
    // flacherer Puls: kleinere Auslenkung
    uint8_t pulse = (uint8_t)(178 + (tri8((uint8_t)(t * 5)) >> 3)); // ~178..210, etwas lebendiger
    // zeitbasierte Funky-Palettenmorph (0..255)
    uint8_t funkW = tri8((uint8_t)(t * 3));

    // optionaler globaler Invert-Glitch
    bool invert = ((nextL() & 0x7F) == 0); // seltener

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        // leicht verzerrter Checker mit Mikro-Wobble: füge eine kleine zeit-/positionsabhängige Welle hinzu
        uint8_t wob = tri8((uint8_t)(x * 17 + t * 9));
        uint8_t warp = (uint8_t)(((x * 5 + y * 7 + (int8_t)(x - y)) + ox + (oy >> 1) + (wob >> 6)) & 0xFF);
        uint8_t cb = (uint8_t)(((x + y + ((warp >> 5) & 1) + ((wob >> 7) & 1)) & 1) ^ (invert ? 1 : 0));

        // zwei kräftige Grund-Paletten und Morph zwischen Varianten für mehr Funk
        uint32_t PAL_A1 = pixels.Color(255, 70, 180);   // pink
        uint32_t PAL_A2 = pixels.Color(200, 60, 220);   // pink→lila
        uint32_t PAL_B1 = pixels.Color(60, 230, 200);   // türkis
        uint32_t PAL_B2 = pixels.Color(180, 255, 80);   // lime
        // lineares Blend A1..A2, B1..B2 per funkW (8-bit, kanalweise)
        auto blend = [&](uint32_t a, uint32_t b, uint8_t w){
          uint8_t ar=(a>>16)&0xFF, ag=(a>>8)&0xFF, ab=a&0xFF;
          uint8_t br=(b>>16)&0xFF, bg=(b>>8)&0xFF, bb=b&0xFF;
          uint8_t r=(uint16_t)ar*(255-w)/255 + (uint16_t)br*w/255;
          uint8_t g=(uint16_t)ag*(255-w)/255 + (uint16_t)bg*w/255;
          uint8_t bl=(uint16_t)ab*(255-w)/255 + (uint16_t)bb*w/255;
          return pixels.Color(r,g,bl);
        };
        uint32_t colA = dimColor(blend(PAL_A1, PAL_A2, funkW), pulse);
        uint32_t colB = dimColor(blend(PAL_B1, PAL_B2, (uint8_t)(255 - funkW)), (uint8_t)(pulse - 18));
        // leichte Variation pro Zelle
        uint8_t jitter = (uint8_t)((x * 23 + y * 41 + t * 9) & 0x1F);
        if (jitter < 8) {
          // Lila Akzent
          colA = dimColor(pixels.Color(180, 40, 200), (uint8_t)(pulse + 10));
        } else if (jitter > 24) {
          // Lime Akzent
          colB = dimColor(pixels.Color(180, 255, 80), pulse);
        }

        // Zielfarbe und sanfte zeitliche Blendung gegen vorherigen Pixel
        uint32_t target = cb ? colA : colB;
        int idx = matrixIndex(x, y);
        uint32_t prev = pixels.getPixelColor(idx);
        // 8-bit Kanalweise-Blend
        uint8_t tr = (target >> 16) & 0xFF, tg = (target >> 8) & 0xFF, tb = target & 0xFF;
        uint8_t pr = (prev >> 16) & 0xFF, pg = (prev >> 8) & 0xFF, pb = prev & 0xFF;
        uint8_t alpha = 88; // ~34% Richtung Ziel, noch etwas smoother
        uint8_t r = (uint16_t)pr * (255 - alpha) / 255 + (uint16_t)tr * alpha / 255;
        uint8_t g = (uint16_t)pg * (255 - alpha) / 255 + (uint16_t)tg * alpha / 255;
        uint8_t b = (uint16_t)pb * (255 - alpha) / 255 + (uint16_t)tb * alpha / 255;
        matrixSet(x, y, pixels.Color(r, g, b));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixHeartbeatCalm
   Heartbeat starts fast/intense (stress) with double-beat "lub-dub" pulses and slowly calms down
   to a slower, softer heartbeat. On a 5x4 display using a tiny heart sprite in red shades.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixHeartbeatCalm(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H; // expected 5x4
  uint32_t start = millis();
  // Tiny 5x4 heart mask (1 = heart pixel)
  static const uint8_t PROGMEM HEART[20] = {
    0,1,0,1,0,
    1,1,1,1,1,
    0,1,1,1,0,
    0,0,1,0,0
  };

  auto lerp16 = [](uint16_t a, uint16_t b, uint16_t t, uint16_t tmax) -> uint16_t {
    // linear interpolate a..b with t in [0..tmax]
    return (uint16_t)(a + ((uint32_t)(b - a) * t) / tmax);
  };

  uint32_t nextBeatAt = millis();
  uint32_t nextDubAt  = 0;
  uint32_t lastBeatAt = 0;
  uint32_t lastDubAt  = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint32_t now = millis();
    uint16_t elapsed = (uint16_t)(now - start);
    // Progress 0..65535 for smooth mapping regardless of runtime
    uint16_t prog = (uint16_t)((uint32_t)elapsed * 65535UL / (uint32_t)(runtime_ms ? runtime_ms : 1));

    // Map heartbeat interval from fast (~380ms) to calm (~900ms)
    uint16_t intervalMs = lerp16(380, 900, prog, 65535);
    // Lub-dub gap ~1/3 of interval, capped
    uint16_t dubGapMs = (uint16_t)((intervalMs < 90) ? 90 : (intervalMs / 3));

    // Trigger main beat
    if (now >= nextBeatAt) {
      lastBeatAt = now;
      nextBeatAt = now + intervalMs;
      nextDubAt  = now + dubGapMs;
    }
    // Trigger dub
    if (nextDubAt && now >= nextDubAt) {
      lastDubAt = now;
      nextDubAt = 0; // fire once per cycle
    }

    // Envelope: quick rise/decay per pulse
    auto env = [&](uint32_t since) -> uint8_t {
      if (since > 140U) return 0; // 140ms envelope window
      // triangle shape 0..255: peak at ~40ms
      uint16_t s = (uint16_t)since;
      uint16_t up = (s < 40U) ? (uint16_t)(s * 6U) : 240U; // rise to 240 at 40ms
      uint16_t down = (s > 40U) ? (uint16_t)(240U - ((s - 40U) * 3U)) : up; // decay
      return (down > 255U) ? 0 : (uint8_t)down;
    };

    uint8_t amp1 = (lastBeatAt ? env(now - lastBeatAt) : 0);
    uint8_t amp2 = (lastDubAt  ? env(now - lastDubAt)  : 0);
    uint8_t amp  = (amp1 > amp2) ? amp1 : amp2;

    // Overall intensity also eases down with time (stress -> calm)
    uint8_t globalScale = (uint8_t)(255 - (prog >> 8)); // 255..0 over runtime
    // Keep a minimum so it doesn't vanish
    if (globalScale < 60) globalScale = 60;

    // Slight jitter early on to simulate stress, fades out
    int8_t jitter = (int8_t)((prog < 20000) ? 1 : (prog < 45000 ? 0 : 0));
    int8_t jx = 0, jy = 0;
    if (jitter) { jx = (random(3) - 1); jy = (random(3) - 1); }

    // Background subtle dark red that also calms
    matrixFade(80);

    // Draw the heart with brightness = amp blended with globalScale
    uint16_t mix = (uint16_t)((uint16_t)amp * (uint16_t)globalScale) / 255U; // 0..255
    uint8_t briCore = (uint8_t)(mix);
    uint8_t briGlow = (uint8_t)(mix / 3);
    uint32_t coreCol = dimColor(pixels.Color(255, 60, 80), (uint8_t)(160 + (briCore >> 2))); // intense red
    uint32_t glowCol = dimColor(pixels.Color(255, 40, 60), (uint8_t)(80 + (briGlow >> 2)));

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t v = pgm_read_byte(&HEART[y * 5 + x]);
        if (v) {
          int8_t tx = (int8_t)x + jx;
          int8_t ty = (int8_t)y + jy;
          if (tx >= 0 && tx < (int8_t)W && ty >= 0 && ty < (int8_t)H)
            matrixSet((uint8_t)tx, (uint8_t)ty, coreCol);
          // soft halo around heart pixels
          if (tx > 0)                 matrixSet((uint8_t)(tx - 1), (uint8_t)ty, glowCol);
          if (tx + 1 < (int8_t)W)     matrixSet((uint8_t)(tx + 1), (uint8_t)ty, glowCol);
          if (ty > 0)                 matrixSet((uint8_t)tx, (uint8_t)(ty - 1), glowCol);
          if (ty + 1 < (int8_t)H)     matrixSet((uint8_t)tx, (uint8_t)(ty + 1), glowCol);
        }
      }
    }

    // Optional click on main beat, stronger at start
    if (amp1 > 200 && (prog < 40000)) {
      playSound((uint16_t)(140 + (random(30))), (uint16_t)10);
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixCocktail
   A tiny cocktail: layered drink colors inside a small glass silhouette with rising bubbles
   and a blinking garnish pixel. Fits 5x4.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCocktail(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  // expected 5x4
  // Glass inner area: x = 1..3, y = 1..3 (bottom at y=3)
  // Layers: bottom (y=3) orange, mid (y=2) red, top (y=1) pink; foam highlight occasionally
  uint8_t bubbleX[3] = {1, 2, 3};
  int8_t  bubbleY[3] = {3, 2, 3};
  uint8_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);
    // Glass rim and stem (simple hints)
    matrixSet(1, 0, pixels.Color(200, 200, 220));
    matrixSet(2, 0, pixels.Color(200, 200, 220));
    matrixSet(3, 0, pixels.Color(200, 200, 220));
    matrixSet(2, 3, pixels.Color(150, 150, 170)); // stem hint at base

    // Drink layers inside (1..3, y=1..3)
    for (uint8_t x = 1; x <= 3; x++) {
      // top layer (y=1): pink
      matrixSet(x, 1, dimColor(pixels.Color(255, 80, 160), 200));
      // mid (y=2): red
      matrixSet(x, 2, dimColor(pixels.Color(255, 40, 40), 220));
      // bottom (y=3): orange
      if (x != 2) matrixSet(x, 3, dimColor(pixels.Color(255, 140, 20), 230));
      else        matrixSet(x, 3, dimColor(pixels.Color(255, 180, 40), 240));
    }

    // Rising bubbles within glass (columns 1..3)
    for (uint8_t i = 0; i < 3; i++) {
      if (bubbleY[i] >= 1 && bubbleY[i] <= 3) {
        uint32_t bub = pixels.Color(255, 255, 255);
        matrixSet((uint8_t)bubbleX[i], (uint8_t)bubbleY[i], bub);
      }
      // Move up slowly; when above the rim, respawn at random depth
      if ((t & 0x01) == 0) {
        bubbleY[i] -= 1;
        if (bubbleY[i] < 1) {
          bubbleX[i] = (uint8_t)(1 + (random(3))); // 1..3
          bubbleY[i] = (int8_t)(2 + (random(2)));  // 2..3 start
        }
      }
    }

    // Garnish (umbrella/cherry) in a corner blinking
    uint8_t blink = (uint8_t)((t >> 2) & 1);
    matrixSet(4, 0, blink ? pixels.Color(255, 100, 100) : pixels.Color(120, 50, 160));

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixAlpineStorm
   Tiny Alps with bad weather: dark sky, drifting storm clouds, diagonal wind-driven snow/rain,
   occasional blue lightning, and white snow caps on the mountain peaks.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixAlpineStorm(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H; // 5x4 expected
  // Mountain profile (height in cells from bottom). Clamp to H.
  uint8_t mH[5] = { (uint8_t)(H>1?2:1), (uint8_t)(H>2?3:2), (uint8_t)(H>3?4:3), (uint8_t)(H>2?3:2), (uint8_t)(H>1?2:1) };
  // Snow line: cap the top cell of each peak in white (blinking a bit)
  uint8_t t = 0;
  struct Drop { int8_t x, y; int8_t vx, vy; uint8_t kind; };
  Drop drops[4];
  for (uint8_t i=0;i<4;i++){ drops[i] = { (int8_t)random(W), (int8_t)random(H), (int8_t)-1, (int8_t)1, (uint8_t)(i&1) }; }
  // Sparse drifting clouds positions (top rows)
  uint8_t cloudMask = 0; // 5-bit mask for row 0..1 clusters
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Base sky
    for (uint8_t y=0;y<H;y++){
      for(uint8_t x=0;x<W;x++){
        uint8_t shade = (uint8_t)(y==0 ? 70 : (y==1 ? 50 : 30));
        uint32_t sky = pixels.Color(10, 20, (uint8_t)(40 + shade));
        matrixSet(x, y, sky);
      }
    }
    // Storm clouds drifting in top 2 rows
    if ((t & 0x03)==0){ cloudMask = (uint8_t)((cloudMask<<1) | (random(2)&1)); }
    for (uint8_t x=0;x<W;x++){
      if (cloudMask & (1<< (x%5))) {
        matrixSet(x, 0, pixels.Color(30, 40, 70));
        if (H>1) matrixSet(x, 1, pixels.Color(20, 30, 60));
      }
    }
    // Mountains (from bottom up), rock gray with subtle variation
    for (uint8_t x=0;x<W;x++){
      uint8_t h = mH[x < 5 ? x : 4]; if (h>H) h=H;
      for (uint8_t k=0;k<h;k++){
        uint8_t y = (uint8_t)(H-1-k);
        uint8_t var = (uint8_t)((x*17 + y*11 + t*3) & 0x1F);
        uint32_t rock = pixels.Color((uint8_t)(60+var), (uint8_t)(60+var), (uint8_t)(70+var));
        matrixSet(x, y, rock);
      }
      // Snow cap on top cell (blink a bit with wind)
      if (h>0){ uint8_t yTop = (uint8_t)(H - h); if ((t + x) & 0x04) matrixSet(x, yTop, pixels.Color(240, 240, 255)); }
    }

    // Alpenröti: slow warm glow on the peaks (fades in and out very gently)
    {
      static uint8_t agPhase = 0;               // persists across calls during runtime
      if ((t & 0x03) == 0) agPhase += 1;        // very slow progression
      // Local triangle wave 0..255 without depending on tri8_local
      uint8_t a = agPhase;
      uint8_t ag = (a & 0x80) ? (uint8_t)(255 - ((a & 0x7F) << 1)) : (uint8_t)((a & 0x7F) << 1);
      uint8_t agLevel = (uint8_t)(ag / 6);      // keep subtle (0..42)
      uint32_t warm = pixels.Color(255, 140, 60);
      for (uint8_t x=0;x<W;x++){
        uint8_t h = mH[x < 5 ? x : 4]; if (h==0 || h>H) continue;
        uint8_t yTop = (uint8_t)(H - h);
        // Tint the snow/top cell warmly
        matrixSet(x, yTop, dimColor(warm, (uint8_t)(180 + agLevel)));
        // Optionally tint the cell below the peak a bit weaker
        if (yTop + 1 < H) matrixSet(x, (uint8_t)(yTop + 1), dimColor(warm, (uint8_t)(80 + (agLevel >> 1))));
      }
    }
    // Wind-driven snow/rain (diagonal streaks)
    for (uint8_t i=0;i<4;i++){
      uint32_t col = (drops[i].kind==0) ? pixels.Color(200, 220, 255) : pixels.Color(120, 170, 240);
      if (drops[i].x>=0 && drops[i].x<(int8_t)W && drops[i].y>=0 && drops[i].y<(int8_t)H){
        matrixSet((uint8_t)drops[i].x, (uint8_t)drops[i].y, col);
        if (drops[i].y>0 && drops[i].x+1 < (int8_t)W) matrixSet((uint8_t)(drops[i].x+1), (uint8_t)(drops[i].y-1), dimColor(col, 120));
      }
      drops[i].x += drops[i].vx; drops[i].y += drops[i].vy;
      if (drops[i].x<0 || drops[i].y>= (int8_t)H) {
        drops[i].x = (int8_t)(W-1); drops[i].y = (int8_t)random(H); drops[i].vx = -1; drops[i].vy = 1; drops[i].kind ^= 1;
      }
    }
    // Occasional blue lightning flash along a column above peaks
    if ((random(32)==0)){
      uint8_t bx = (uint8_t)random(W); uint8_t by = 0;
      uint8_t steps = (uint8_t)(H);
      for (uint8_t s=0;s<steps;s++){
        matrixSet(bx, by, pixels.Color(180, 220, 255));
        if (bx>0) matrixSet((uint8_t)(bx-1), by, pixels.Color(50, 90, 200));
        if (bx+1<W) matrixSet((uint8_t)(bx+1), by, pixels.Color(50, 90, 200));
        pixels.show(); delay(12);
        by++; if (by>=H) break;
      }
      // quick fade
      for (uint8_t k=0;k<2;k++){ matrixFade(160); pixels.show(); delay(14); }
      if (random(2)==0) playSound(90, 20);
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

inline void matrixFlowerBurst(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint16_t t = 0;
  bool exploded = false;
  struct P { int8_t x, y; int8_t vx, vy; uint8_t life; };
  P parts[8]; uint8_t pc = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);
    if (!exploded) {
      uint8_t ph = (uint8_t)((t * 9) & 0xFF);
      uint32_t petal = Wheel((uint8_t)(ph + 200));
      uint32_t core  = Wheel((uint8_t)(ph + 20));
      uint8_t pulse = (uint8_t)(180 + ((t & 0x0F) * 4));
      uint8_t cx = W / 2; uint8_t cy = H / 2;
      matrixSet(cx, cy, dimColor(core, 255));
      if (W >= 3) { if (cx > 0) matrixSet((uint8_t)(cx - 1), cy, dimColor(petal, pulse)); if (cx + 1 < W) matrixSet((uint8_t)(cx + 1), cy, dimColor(petal, pulse)); }
      if (H >= 3) { if (cy > 0) matrixSet(cx, (uint8_t)(cy - 1), dimColor(petal, pulse)); if (cy + 1 < H) matrixSet(cx, (uint8_t)(cy + 1), dimColor(petal, pulse)); }
      if (cx > 0 && cy > 0) matrixSet((uint8_t)(cx - 1), (uint8_t)(cy - 1), dimColor(petal, (uint8_t)(pulse - 40)));
      if (cx + 1 < W && cy > 0) matrixSet((uint8_t)(cx + 1), (uint8_t)(cy - 1), dimColor(petal, (uint8_t)(pulse - 40)));
      if (cx > 0 && cy + 1 < H) matrixSet((uint8_t)(cx - 1), (uint8_t)(cy + 1), dimColor(petal, (uint8_t)(pulse - 40)));
      if (cx + 1 < W && cy + 1 < H) matrixSet((uint8_t)(cx + 1), (uint8_t)(cy + 1), dimColor(petal, (uint8_t)(pulse - 40)));
      if (t > 48) {
        exploded = true;
        pc = 0;
        int8_t vx[8] = { 0, 1, 0, -1, 1, 1, -1, -1 };
        int8_t vy[8] = { -1,0, 1,  0,-1, 1, -1,  1 };
        for (uint8_t i = 0; i < 8 && i < (uint8_t)(W * H); i++) { parts[i].x = (int8_t)cx; parts[i].y = (int8_t)cy; parts[i].vx = vx[i]; parts[i].vy = vy[i]; parts[i].life = (uint8_t)(10 + (i & 3) * 3); pc++; }
        playSound(220, 80);
      }
    } else {
      for (uint8_t i = 0; i < pc; i++) {
        if (parts[i].life == 0) continue;
        uint8_t bri = (uint8_t)(parts[i].life * 20);
        uint32_t col = dimColor(Wheel((uint8_t)((i * 36 + t * 7) & 0xFF)), bri);
        if (parts[i].x >= 0 && parts[i].x < (int8_t)W && parts[i].y >= 0 && parts[i].y < (int8_t)H)
          matrixSet((uint8_t)parts[i].x, (uint8_t)parts[i].y, col);
        parts[i].x += parts[i].vx; parts[i].y += parts[i].vy;
        if (parts[i].x < 0 || parts[i].x >= (int8_t)W) { parts[i].life = 0; }
        if (parts[i].y < 0 || parts[i].y >= (int8_t)H) { parts[i].life = 0; }
        if (parts[i].life > 0) parts[i].life--;
      }
      matrixFade(40);
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBlueLightning
   Colder blue lightning variant: blue-white bolt with soft blue halo and blue room flash.
   Params: runtime_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBlueLightning(uint16_t runtime_ms) {
  uint32_t start = millis(); randomSeed(millis());
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  auto clampi = [](int v, int lo, int hi) -> int { return (v < lo) ? lo : (v > hi ? hi : v); };
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Dark pause before strike
    matrixFill(0); pixels.show(); delay((uint16_t)(100 + random(260)));
    uint8_t flashes = (uint8_t)(1 + random(3));
    for (uint8_t f = 0; f < flashes; f++) {
      int x = (int)random(W); int y = 0;
      uint8_t steps = (uint8_t)(H + random(2));
      matrixFill(0);
      for (uint8_t s = 0; s < steps; s++) {
        // Bolt head (blue-white) and surrounding halo (deep blue)
        uint32_t head = pixels.Color(180, 220, 255);
        uint32_t halo = pixels.Color(40, 90, 200);
        matrixSet((uint8_t)x, (uint8_t)y, head);
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = clampi(x + dx, 0, (int)W - 1);
            int ny = clampi(y + dy, 0, (int)H - 1);
            matrixSet((uint8_t)nx, (uint8_t)ny, dimColor(halo, 200));
          }
        }
        // Occasional wide blue flash
        if ((random(5) == 0)) {
          for (uint8_t yy = 0; yy < H; yy++) {
            for (uint8_t xx = 0; xx < W; xx++) {
              if (!(xx == (uint8_t)x && yy == (uint8_t)y))
                matrixSet(xx, yy, dimColor(halo, 120));
            }
          }
        }
        pixels.show(); delay((uint16_t)(10 + random(18)));
        x += (int8_t)(random(3)) - 1; y += 1; if (y >= (int)H) y = (int)H - 1; x = clampi(x, 0, (int)W - 1);
      }
      // Blue flicker tail
      for (uint8_t k = 0; k < 3; k++) { matrixFade(180); pixels.show(); delay(20 + random(20)); }
      for (uint8_t d = 0; d < 4; d++) { matrixFade(120); pixels.show(); delay(18); }
      if (random(4) == 0) { playSound((uint16_t)(90 + random(50)), (uint16_t)(20 + random(30))); }
    }
  }
}

inline void matrixTricksterPlasma(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  // Brown→Yellow palette endpoints
  const uint32_t COL_DARK = pixels.Color(30, 15, 0);     // darker base for more contrast
  const uint32_t COL_BRIGHT = pixels.Color(255, 220, 60); // brighter highlight
  auto lerpCol = [&](uint8_t a){
    // a: 0..255 -> interpolate COL_DARK..COL_BRIGHT
    uint8_t dr = (uint8_t)((COL_DARK >> 16) & 0xFF), dg = (uint8_t)((COL_DARK >> 8) & 0xFF), db = (uint8_t)(COL_DARK & 0xFF);
    uint8_t br = (uint8_t)((COL_BRIGHT >> 16) & 0xFF), bg = (uint8_t)((COL_BRIGHT >> 8) & 0xFF), bb = (uint8_t)(COL_BRIGHT & 0xFF);
    uint8_t r = (uint8_t)(dr + ((uint16_t)(br - dr) * a >> 8));
    uint8_t g = (uint8_t)(dg + ((uint16_t)(bg - dg) * a >> 8));
    uint8_t b = (uint8_t)(db + ((uint16_t)(bb - db) * a >> 8));
    return pixels.Color(r, g, b);
  };
  // Motion phases
  float p1 = 0.f, p2 = 0.f, p3 = 0.f;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Slightly faster drift for more dynamic motion
    p1 += 0.18f; p2 += 0.11f; p3 += 0.08f;
    for (uint8_t y = 0; y < MATRIX_H; ++y) {
      for (uint8_t x = 0; x < MATRIX_W; ++x) {
        // 3-field interference for plasma-like value 0..1
        float v = 0.0f;
        v += sin((x + p1) * 0.8f);
        v += cos((y - p2) * 1.1f);
        v += sin(((int)x + (int)y + p3) * 0.6f);
        v += 0.6f * sin((x * 0.5f - y * 0.7f) + p2 * 0.9f); // extra cross-term for contrasty motion
        // normalize from [-3,3] -> [0,1]
        v = (v * (1.0f / 7.2f)) + 0.5f; // adjust normalization for added term
        if (v < 0.f) v = 0.f;
        if (v > 1.f) v = 1.f;
        // add slight pulsing bias to mimic flickery trickster look
        float pulse = (sin(p1 * 0.25f) * 0.15f);
        float vv = v + pulse;
        if (vv < 0.f) vv = 0.f;
        if (vv > 1.f) vv = 1.f;
        // logarithmic/gamma-like contrast to bias toward darkness
        vv = vv * 1.05f;
        if (vv > 1.f) vv = 1.f;
        if (vv < 0.f) vv = 0.f;
        vv = pow(vv, 2.4f);               // gamma > 1 darkens mid/highs
        if (vv < 0.06f) vv = 0.f;         // black floor for deeper shadows
        uint8_t a = (uint8_t)(vv * 255.0f + 0.5f);
        uint32_t c = lerpCol(a);
        matrixSet(x, y, c);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixDigitalRain
   Matrix movie-style green code rain. On a tiny 5x4, each column spawns a bright green head
   that falls with variable speed, leaving a trailing fade. Heads respawn pseudo-randomly.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixDigitalRain(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0x1234);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Per-column head position (0..H-1) or 0xFF when inactive
  uint8_t headY[8];
  // Per-column speed divisor (1..3): lower = faster
  uint8_t speed[8];
  // Per-column tick counter
  uint8_t tick[8];
  for (uint8_t x = 0; x < W; x++) {
    headY[x] = 0xFF; // inactive
    speed[x] = (uint8_t)(1 + (nextL() % 3));
    tick[x] = 0;
  }

  // Initial gentle clear
  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Trails fade: higher value = faster fade; keep trails visible a bit
    matrixFade(70);

    for (uint8_t x = 0; x < W; x++) {
      // Possibly spawn a new head at top if inactive
      if (headY[x] == 0xFF) {
        // Spawn chance ~ 1/5 per frame per column
        if ((nextL() & 0x07) == 0) {
          headY[x] = 0; // top
          speed[x] = (uint8_t)(1 + (nextL() % 3));
          tick[x] = 0;
        }
      } else {
        // Advance based on speed divisor
        tick[x]++;
        if ((tick[x] % speed[x]) == 0) {
          if (headY[x] + 1 < H) headY[x]++;
          else {
            // At bottom: either continue off then deactivate, or immediately reset
            headY[x] = 0xFF; // deactivate; next loop may respawn
            continue;
          }
        }
        // Draw head and a dimmer body just above (if exists)
        uint8_t y = headY[x];
        uint32_t headCol = pixels.Color(170, 255, 170); // bright green with hint of white
        matrixSet(x, y, headCol);
        if (y > 0) {
          uint32_t bodyCol = pixels.Color(40, 180, 60);
          matrixSet(x, (uint8_t)(y - 1), dimColor(bodyCol, 200));
        }
      }
    }

    // Rare bright glyph flicker on random pixels to mimic characters
    if ((nextL() & 0x0F) == 0) {
      uint8_t rx = (uint8_t)(nextL() % W);
      uint8_t ry = (uint8_t)(nextL() % H);
      matrixSet(rx, ry, pixels.Color(120, 255, 120));
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixWaterfall
   A misty/spraying waterfall adapted to 5x4:
   - Soft blue drops fall in each column leaving trails.
   - On hitting the bottom, a light white/blue mist spreads sideways.
   - Continuous subtle seep along bottom row for a wet look.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixWaterfall(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0x42AF);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Per-column falling head (0..H-1) or 0xFF when inactive
  uint8_t headY[8];
  uint8_t speed[8];   // 1..3 (lower is faster)
  uint8_t tick[8];
  for (uint8_t x = 0; x < W; x++) {
    headY[x] = 0xFF; // inactive
    speed[x] = (uint8_t)(1 + (nextL() % 3));
    tick[x] = 0;
  }

  // Gentle clear
  matrixFill(0);
  pixels.show();

  // Bottom foam persistence per column (0..255 intensity)
  uint8_t foam[8];
  for (uint8_t i = 0; i < W; i++) foam[i] = 0;

  // Phase for color crossfade (blue <-> pink)
  uint8_t t = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Trails fade; a bit slower to keep more mist visible
    matrixFade(80);

    // Triangle wave 0..255
    uint8_t ph = (uint8_t)(t);
    uint8_t tri = (ph & 0x80) ? (uint8_t)(255 - ((ph & 0x7F) << 1))
                              : (uint8_t)(((ph & 0x7F) << 1));

    // Helper to blend two RGB colors by weight w (0..255)
    auto blend = [&](uint32_t a, uint32_t b, uint8_t w) -> uint32_t {
      uint8_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
      uint8_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
      uint8_t r = (uint16_t)ar * (255 - w) / 255 + (uint16_t)br * w / 255;
      uint8_t g = (uint16_t)ag * (255 - w) / 255 + (uint16_t)bg * w / 255;
      uint8_t bch = (uint16_t)ab * (255 - w) / 255 + (uint16_t)bb * w / 255;
      return pixels.Color(r, g, bch);
    };

    // Define blue and pink palettes, then blend by tri
    const uint32_t HEAD_BLUE  = pixels.Color(120, 180, 255);
    const uint32_t HEAD_PINK  = pixels.Color(255, 120, 200);
    const uint32_t BODY_BLUE  = pixels.Color(40, 100, 220);
    const uint32_t BODY_PINK  = pixels.Color(220, 60, 140);
    const uint32_t MISTA_BLUE = pixels.Color(200, 220, 255);
    const uint32_t MISTA_PINK = pixels.Color(255, 200, 230);
    const uint32_t MISTB_BLUE = pixels.Color(120, 180, 255);
    const uint32_t MISTB_PINK = pixels.Color(255, 140, 200);
    const uint32_t FOAM_BLUE  = pixels.Color(200, 230, 255);
    const uint32_t FOAM_PINK  = pixels.Color(255, 220, 240);
    const uint32_t SPARK_BLUE = pixels.Color(220, 240, 255);
    const uint32_t SPARK_PINK = pixels.Color(255, 230, 245);

    uint32_t COL_HEAD  = blend(HEAD_BLUE,  HEAD_PINK,  tri);
    uint32_t COL_BODY  = blend(BODY_BLUE,  BODY_PINK,  tri);
    uint32_t COL_MISTA = blend(MISTA_BLUE, MISTA_PINK, tri);
    uint32_t COL_MISTB = blend(MISTB_BLUE, MISTB_PINK, tri);
    uint32_t COL_FOAM  = blend(FOAM_BLUE,  FOAM_PINK,  tri);
    uint32_t COL_SPARK = blend(SPARK_BLUE, SPARK_PINK, tri);

    for (uint8_t x = 0; x < W; x++) {
      // Spawn new drop at the top occasionally when inactive
      if (headY[x] == 0xFF) {
        if ((nextL() & 0x07) <= 1) { // ~2/8 chance
          headY[x] = 0; // top
          speed[x] = (uint8_t)(1 + (nextL() % 3));
          tick[x] = 0;
        }
        continue;
      }

      // Progress falling based on per-column speed divider
      tick[x]++;
      if ((tick[x] % speed[x]) == 0) {
        if (headY[x] + 1 < H) headY[x]++;
        else {
          // Splash at bottom: brief mist to sides and above
          uint8_t y = (uint8_t)(H - 1);
          matrixSet(x, y, dimColor(COL_MISTA, 160));
          if (x > 0)      matrixSet((uint8_t)(x - 1), y, dimColor(COL_MISTB, 140));
          if (x + 1 < W)  matrixSet((uint8_t)(x + 1), y, dimColor(COL_MISTB, 140));
          if (y > 0)      matrixSet(x, (uint8_t)(y - 1), dimColor(COL_MISTB, 120));
          // Chance of extra spray pixel popping up one row higher and to the side
          if ((nextL() & 0x03) <= 1) { // slightly more spray
            int8_t sx = (int8_t)x + (int8_t)((nextL() & 1) ? 1 : -1);
            uint8_t sy = (uint8_t)(y > 0 ? y - 1 : y);
            if (sx >= 0 && sx < (int8_t)W) matrixSet((uint8_t)sx, sy, dimColor(COL_MISTA, 200));
          }
          // Add foam persistence at the bottom with lateral push
          if (foam[x] < 220) foam[x] = 220;
          if (x > 0 && foam[x-1] < 140) foam[x-1] = 140;
          if (x + 1 < W && foam[x+1] < 140) foam[x+1] = 140;
          // Deactivate head; new one may spawn later
          headY[x] = 0xFF;
          continue;
        }
      }

      // Draw falling head and faint body above
      uint8_t y = headY[x];
      matrixSet(x, y, dimColor(COL_HEAD, 220));
      if (y > 0) matrixSet(x, (uint8_t)(y - 1), dimColor(COL_BODY, 160));
    }

    // Persistent foam rendering and shimmer along the bottom row
    for (uint8_t x = 0; x < W; x++) {
      // Light diffusion: nudge foam sideways a bit
      if ((nextL() & 0x07) == 0) {
        if ((nextL() & 1) && x + 1 < W && foam[x] > 10) { uint8_t d = (uint8_t)(foam[x] / 6); foam[x] -= d; if (foam[x+1] + d < 255) foam[x+1] += d; }
        else if (x > 0 && foam[x] > 10) { uint8_t d = (uint8_t)(foam[x] / 6); foam[x] -= d; if (foam[x-1] + d < 255) foam[x-1] += d; }
      }
      // Decay foam slowly
      if (foam[x] > 4) foam[x] = (uint8_t)(foam[x] - 4); else foam[x] = 0;
      // Base foam color (white-blue) scaled by foam intensity
      if (foam[x] > 0) matrixSet(x, (uint8_t)(H - 1), dimColor(COL_FOAM, foam[x]));

      // Occasional sparkle and tiny up-splash
      uint8_t rnd = (uint8_t)(nextL() & 0xFF);
      if ((rnd & 0x07) == 0) {
        uint8_t sparkle = (uint8_t)(160 + (rnd & 0x1F));
        matrixSet(x, (uint8_t)(H - 1), dimColor(COL_SPARK, sparkle));
      } else if ((rnd & 0x1F) == 0) {
        if (H >= 2) matrixSet(x, (uint8_t)(H - 2), dimColor(pixels.Color(140, 200, 255), 140));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t += 3; // advance color phase slowly
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixNightStreet2000
   Night street scene on 5x4:
   - Warm streetlights cast a vertical glow with subtle flicker.
   - Passing car headlights sweep across the bottom row (paired white).
   - Neon sign flickers in pink/cyan tones in a small 2x2 area.
   - Distant window twinkles in the top rows.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
static inline void matrixNightStreet2000(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  // LFSR for light randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0xA55A);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Car headlights state: two cars with x position and direction
  int8_t carX0 = -3, carX1 = W + 2; // start off-screen
  int8_t carDir0 = 1;               // left->right
  int8_t carDir1 = -1;              // right->left
  uint8_t tick = 0;

  // Neon sign area (2x2) anchored near top-right if possible
  const uint8_t neonX = (W >= 4) ? (W - 2) : 0;
  const uint8_t neonY = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Fade trails gently
    matrixFade(72);

    // Base night tint (very dark blue), blended sparsely to save cycles
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        if (((x + y + tick) & 0x0F) == 0) {
          uint32_t tint = pixels.Color(5, 8, 20);
          int idx = matrixIndex(x, y);
          uint32_t prev = pixels.getPixelColor(idx);
          uint8_t a = 16; // small blend-in
          uint8_t pr = (prev >> 16) & 0xFF, pg = (prev >> 8) & 0xFF, pb = prev & 0xFF;
          uint8_t tr = (tint >> 16) & 0xFF, tg = (tint >> 8) & 0xFF, tb = tint & 0xFF;
          uint8_t r = (uint16_t)pr * (255 - a) / 255 + (uint16_t)tr * a / 255;
          uint8_t g = (uint16_t)pg * (255 - a) / 255 + (uint16_t)tg * a / 255;
          uint8_t b = (uint16_t)pb * (255 - a) / 255 + (uint16_t)tb * a / 255;
          pixels.setPixelColor(idx, r, g, b);
        }
      }
    }

    // Streetlights: choose a column and cast warm vertical glow with flicker
    if ((tick & 0x03) == 0) {
      uint8_t col = (uint8_t)(nextL() % W);
      uint8_t base = (uint8_t)(180 + (nextL() & 0x1F)); // 180..211
      for (uint8_t y = 0; y < H; y++) {
        uint8_t lvl = (uint8_t)(base - y * 24); // stronger at bottom
        if (lvl < 40) lvl = 40;
        matrixSet(col, y, dimColor(pixels.Color(255, 200, 80), lvl));
      }
    }

    // Car headlights sweeping on bottom row (two cars, opposite directions)
    if ((tick & 0x01) == 0) {
      carX0 += carDir0; carX1 += carDir1;
      if (carX0 >= (int8_t)W + 2) carX0 = -3;   // wrap
      if (carX1 < -3)            carX1 = (int8_t)W + 2; // wrap
    }
    auto drawCar = [&](int8_t cx, uint32_t headCol) {
      for (int8_t k = 0; k < 2; k++) {
        int8_t x = (int8_t)(cx + k);
        if (x >= 0 && x < (int8_t)W) matrixSet((uint8_t)x, (uint8_t)(H - 1), headCol);
      }
    };
    uint32_t headCol0 = pixels.Color(255, 255, 220);
    uint32_t headCol1 = pixels.Color(220, 240, 255);
    drawCar(carX0, dimColor(headCol0, 220));
    drawCar(carX1, dimColor(headCol1, 200));

    // Neon sign flicker in pink/cyan (2x2 area)
    uint8_t neonPhase = (uint8_t)(nextL() & 0xFF);
    uint8_t neonOn = (uint8_t)((neonPhase & 0x1F) > 3); // occasionally off
    if (neonOn) {
      uint32_t neonA = pixels.Color(255, 80, 160); // pink
      uint32_t neonB = pixels.Color(40, 200, 200); // cyan
      uint32_t neonMix = (neonPhase & 0x20) ? neonA : neonB;
      uint8_t pul = (uint8_t)(180 + (neonPhase & 0x3F));
      for (uint8_t dy = 0; dy < 2 && neonY + dy < H; dy++)
        for (uint8_t dx = 0; dx < 2 && neonX + dx < W; dx++)
          matrixSet((uint8_t)(neonX + dx), (uint8_t)(neonY + dy), dimColor(neonMix, pul));
    }

    // Distant windows twinkle in top half
    if ((tick & 0x03) == 0) {
      uint8_t wx = (uint8_t)(nextL() % W);
      uint8_t wy = (uint8_t)(nextL() % (H > 2 ? (H - 2) : H)); // avoid bottom row
      matrixSet(wx, wy, dimColor(pixels.Color(255, 220, 120), (uint8_t)(140 + (nextL() & 0x3F))));
    }

    pixels.show();
    delay(stepDelay_ms);
    tick++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSunsetPickleSun
   Sunset gradient sky with a small "cucumber" sun composed of green rings.
   - Warm sky fades from pink/magenta (top) to orange/yellow (horizon) over time.
   - Sun is a 2x2 disc centered near the horizon with cucumber-green tones and subtle sparkle.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSunsetPickleSun(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint8_t t = 0;

  // Helper to blend two RGB colors (8-bit channels) by weight w (0..255)
  auto blend = [&](uint32_t a, uint32_t b, uint8_t w) -> uint32_t {
    uint8_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
    uint8_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
    uint8_t r = (uint16_t)ar * (255 - w) / 255 + (uint16_t)br * w / 255;
    uint8_t g = (uint16_t)ag * (255 - w) / 255 + (uint16_t)bg * w / 255;
    uint8_t bch = (uint16_t)ab * (255 - w) / 255 + (uint16_t)bb * w / 255;
    return pixels.Color(r, g, bch);
  };

  // Sky palette endpoints
  const uint32_t SKY_TOP_A = pixels.Color(255, 60, 160);  // pink/magenta
  const uint32_t SKY_TOP_B = pixels.Color(255, 120, 60);  // warm orange
  const uint32_t SKY_HOR_A = pixels.Color(255, 180, 40);  // yellow-orange
  const uint32_t SKY_HOR_B = pixels.Color(255, 120, 0);   // burnt orange

  // Cucumber sun greens (inner brighter, outer darker)
  const uint32_t SUN_INNER = pixels.Color(120, 230, 120);
  const uint32_t SUN_RING  = pixels.Color(40, 160, 80);

  // Sun horizontal center stays centered; vertical center will be animated
  const uint8_t cx = MATRIX_W / 2;   // 2 on 5-wide

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Animate sky palette over time via a slow triangle
    uint8_t ph = (uint8_t)(t * 4);           // slow phase
    // inline triangle wave 0..255 without external helper
    uint8_t tri = (ph & 0x80) ? (uint8_t)(255 - ((ph & 0x7F) << 1))
                              : (uint8_t)(((ph & 0x7F) << 1));

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      // row weight: 0 (top) -> 255 (bottom)
      uint8_t roww = (uint8_t)((uint16_t)y * 255 / (MATRIX_H - 1));
      // Interpolate top and horizon palettes separately, then mix by row
      uint32_t topCol = blend(SKY_TOP_A, SKY_TOP_B, tri);
      uint32_t horCol = blend(SKY_HOR_A, SKY_HOR_B, (uint8_t)(255 - tri));
      uint32_t skyCol = blend(topCol, horCol, roww);
      // Top should be a bit darker, bottom full; scale ~200..255 over rows
      uint8_t rowDim = (uint8_t)(200 + ((uint16_t)y * 55 / (MATRIX_H - 1))); // top ~200, bottom 255
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        matrixSet(x, y, dimColor(skyCol, rowDim));
      }
    }

    // Compute animated sun vertical center as a descending sawtooth (sunset), then restart at top
    static uint8_t sunPhase = 0;               // 0..255
    const uint8_t horizonRow = (uint8_t)(MATRIX_H - 1); // treat bottom row as horizon line
    // Map phase to 0..horizonRow+1 (allows one extra step "below" horizon before restart)
    uint8_t cy = (uint8_t)(((uint16_t)sunPhase * (horizonRow + 1)) / 255);
    bool fullyBelow = (cy > horizonRow); // strictly below -> hidden
    // Dwell counters to keep the sun hidden longer: first fade darker, then hold night
    static uint8_t dwell = 0;   // counts total dwell frames after sunset

    // Draw the cucumber sun (2x2 disc) with green ring accent, clipped strictly above horizon
    if (!fullyBelow) {
      for (int8_t dy = -1; dy <= 0; dy++) {
        for (int8_t dx = -1; dx <= 0; dx++) {
          uint8_t x = (uint8_t)(cx + dx);
          uint8_t y = (uint8_t)(cy + dy);
          if (x < MATRIX_W && y < MATRIX_H && y < horizonRow) {
            uint8_t md = (uint8_t)(abs(dx) + abs(dy));
            uint32_t col = (md == 0) ? SUN_INNER : SUN_RING;
            uint8_t pulse = (uint8_t)(200 + ((t * 9 + (dx+1)*7 + (dy+1)*5) & 0x2F));
            // As we approach the horizon (y close to horizonRow-1), dim slightly
            uint8_t edgeDim = (y >= (uint8_t)(horizonRow - 1)) ? (uint8_t)(pulse * 4 / 5) : pulse;
            matrixSet(x, y, dimColor(col, edgeDim));
          }
        }
      }
    }

    // Tiny sparkle seeds around the sun (only when visible)
    if (!fullyBelow && ((t & 0x03) == 0x00)) {
      uint8_t sx = (uint8_t)(cx + ((t >> 2) & 0x01 ? 1 : -1));
      uint8_t sy = (uint8_t)(cy + (((t >> 3) & 0x01) ? 1 : 0));
      if (sx < MATRIX_W && sy < MATRIX_H && sy < horizonRow) matrixSet(sx, sy, pixels.Color(255, 255, 200));
    }

    // During hidden dwell, darken the entire sky gradually, then hold much darker night
    if (fullyBelow) {
      // Phase 1: fade darker over ~40 frames; Phase 2: hold for ~80 frames
      uint8_t dusk;
      if (dwell < 40) {
        dusk = (uint8_t)(220 - (dwell * 3)); // 220 -> ~100
      } else {
        dusk = 96; // hold darkest
      }
      // Night tint: gradually blend the whole sky toward deep blue while dimming
      const uint32_t NIGHT_BLUE = pixels.Color(10, 20, 60);
      // nightW grows with dwell to pull colors toward NIGHT_BLUE
      uint8_t nightW = (uint8_t)(dwell < 40 ? (dwell * 5) : 200); // 0..~200
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        for (uint8_t x = 0; x < MATRIX_W; x++) {
          int idx = matrixIndex(x, y);
          uint32_t c = pixels.getPixelColor(idx);
          uint32_t cTint = blend(c, NIGHT_BLUE, nightW);
          pixels.setPixelColor(idx, dimColor(cTint, dusk));
        }
      }

      // Add more blinking stars across the entire sky region (above horizon), slower and softer
      static uint16_t starLfsr = 0xACE1; // simple LFSR for pseudo-random stars
      static uint8_t starTick = 0;       // slower tick for twinkle
      uint8_t syMax = horizonRow; // stars only above horizon
      // Update stars only every 3rd dwell frame to reduce hectic flicker
      if ((dwell % 3) == 0 && syMax > 0) {
        // advance LFSR sparsely
        starLfsr ^= (uint16_t)((t << 7) ^ (dwell * 13));
        for (uint8_t k = 0; k < 2; k++) { // fewer stars per update
          // LFSR step (x^16 + x^14 + x^13 + x^11)
          uint16_t bit = (uint16_t)(((starLfsr >> 0) ^ (starLfsr >> 2) ^ (starLfsr >> 3) ^ (starLfsr >> 5)) & 1);
          starLfsr = (uint16_t)((starLfsr >> 1) | (bit << 15));
          uint8_t sx = (uint8_t)(starLfsr % MATRIX_W);
          uint8_t sy = (uint8_t)((starLfsr >> 8) % syMax); // 0..horizonRow-1
          // Slow twinkle based on starTick triangle 120..200
          uint8_t ph = (uint8_t)(starTick & 0xFF);
          uint8_t tri = (ph & 0x80) ? (uint8_t)(200 - (((ph & 0x7F) << 1) >> 1))
                                    : (uint8_t)(120 + (((ph & 0x7F) << 1) >> 1));
          int sidx = matrixIndex(sx, sy);
          uint32_t prev = pixels.getPixelColor(sidx);
          uint32_t target = dimColor(pixels.Color(255, 255, 220), tri);
          // Blend in softly so stars fade in/out rather than snap
          uint8_t alpha = 64; // ~25% towards target per update
          uint32_t blended = blend(prev, target, alpha);
          pixels.setPixelColor(sidx, blended);
        }
        starTick += 5; // slow increment
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    // advance sun phase; when hidden below horizon, dwell longer, then restart at the top
    if (fullyBelow) {
      if (dwell < 120) { dwell++; }  // ~40 fade + ~80 hold
      else { dwell = 0; sunPhase = 0; }
    } else {
      sunPhase = (uint8_t)(sunPhase + 3);
    }
    t++;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowWaves
   Flowing rainbow stripes across the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
static inline uint8_t tri8_local(uint8_t v) {
  uint8_t x = v & 0x7F; // 0..127
  x = (x << 1);         // 0..254
  return (v & 0x80) ? (uint8_t)(255 - x) : x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixCoteAzur
   French coastal landscape on 5x4: deep blue sea with shimmering waves at bottom,
   green coastline band above, peppered with rock-gray accents. Gentle motion.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCoteAzur(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint32_t SEA_BASE   = pixels.Color(20, 60, 160);   // deep blue
  const uint32_t SEA_LIGHT  = pixels.Color(60, 120, 220);  // light blue shimmer
  const uint32_t COAST_GRN  = pixels.Color(30, 180, 90);   // lush green

  uint32_t start = millis();
  uint8_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(60);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        if (y <= 1) {
          uint8_t phase = (uint8_t)(x * 40 + t * 10);
          uint8_t w = tri8_local(phase);           // 0..255
          uint8_t rB = (SEA_BASE >> 16) & 0xFF, gB = (SEA_BASE >> 8) & 0xFF, bB = SEA_BASE & 0xFF;
          uint8_t rL = (SEA_LIGHT >> 16) & 0xFF, gL = (SEA_LIGHT >> 8) & 0xFF, bL = SEA_LIGHT & 0xFF;
          uint8_t r = (uint16_t)rB * (255 - w) / 255 + (uint16_t)rL * w / 255;
          uint8_t g = (uint16_t)gB * (255 - w) / 255 + (uint16_t)gL * w / 255;
          uint8_t b = (uint16_t)bB * (255 - w) / 255 + (uint16_t)bL * w / 255;
          matrixSet(x, y, pixels.Color(r, g, b));
        } else if (y == 2) {
          uint8_t rnd = (uint8_t)((x * 61 + t * 17) & 0xFF);
          bool rock = (rnd & 0x1F) == 0; // ~1/32 chance per frame
          if (rock) {
            uint8_t k = (uint8_t)(80 + (rnd & 0x3F));
            matrixSet(x, y, pixels.Color(k, k, k));
          } else {
            uint8_t pulse = (uint8_t)(180 + ((t + x * 9) & 0x2F));
            matrixSet(x, y, dimColor(COAST_GRN, pulse));
          }
        } else { // y == 3
          uint32_t hint = pixels.Color(10, 40, 60);
          uint8_t pulse = (uint8_t)(120 + ((t + x * 7) & 0x1F));
          matrixSet(x, y, dimColor(hint, pulse));
        }
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixShoggoth
   A wobbling pink blob with soft rim and blinking eyes.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixShoggoth(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t t = 0;
  const uint32_t colCore = pixels.Color(255, 60, 140);
  const uint32_t colRim  = pixels.Color(120, 20, 60);
  const uint32_t colEye  = pixels.Color(255, 255, 255);
  const uint8_t W = 5, H = 4;
  uint8_t eyeCount = 2;
  int8_t ex[3] = {1, 3, 2};
  int8_t ey[3] = {1, 2, 1};
  bool eyesOpen = true;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t pulse = (uint8_t)(140 + ((t * 9) & 0x3F));
    uint8_t cx = (uint8_t)(1 + ((t >> 3) & 0x03)); if (cx > (W - 2)) cx = (W - 2);
    uint8_t cy = (uint8_t)(1 + ((t >> 4) & 0x01));
    uint8_t radius = (uint8_t)(1 + ((t >> 2) & 0x01));
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int8_t dx = (int8_t)x - (int8_t)cx;
        int8_t dy = (int8_t)y - (int8_t)cy;
        uint8_t dist = (uint8_t)(abs(dx) + abs(dy));
        if (dist <= radius) {
          matrixSet(x, y, dimColor(colCore, pulse));
        } else if (dist == radius + 1) {
          matrixSet(x, y, dimColor(colRim, (uint8_t)(pulse * 2 / 5)));
        } else if (dist == radius + 2) {
          matrixSet(x, y, dimColor(colRim, (uint8_t)(pulse / 6)));
        }
      }
    }
    if ((t & 0x0F) == 0x00) {
      for (uint8_t i = 0; i < eyeCount; i++) {
        int8_t ox = ((int8_t)random(3)) - 1;
        int8_t oy = ((int8_t)random(3)) - 1;
        int8_t nx = (int8_t)cx + ox;
        int8_t ny = (int8_t)cy + oy;
        if (nx >= 0 && nx < (int8_t)W) ex[i] = (uint8_t)nx;
        if (ny >= 0 && ny < (int8_t)H) ey[i] = (uint8_t)ny;
      }
    }
    if ((t & 0x1F) == 0x1F) { eyesOpen = !eyesOpen; }
    if (eyesOpen) {
      for (uint8_t i = 0; i < eyeCount; i++) matrixSet((uint8_t)ex[i], (uint8_t)ey[i], colEye);
    } else {
      for (uint8_t i = 0; i < eyeCount; i++) matrixSet((uint8_t)ex[i], (uint8_t)ey[i], dimColor(colEye, 100));
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixExplosion
   A nasty explosion: blinding flash, expanding shockwave, flying debris, smoky afterglow.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixExplosion(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for randomness (center + debris)
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBEEF);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Randomize explosion center, avoid direct edges
  uint8_t cx;
  uint8_t cy;
  if (W > 2) cx = (uint8_t)(1 + (nextL() % (W - 2))); else cx = (uint8_t)(nextL() % W);
  if (H > 2) cy = (uint8_t)(1 + (nextL() % (H - 2))); else cy = (uint8_t)(nextL() % H);

  // Debris particles (very few to save RAM)
  const uint8_t N = 6;
  int16_t px[N], py[N];   // position in 1/16 pixels
  int8_t  vx[N], vy[N];   // velocity in 1/16 pixels per frame
  uint8_t life[N];
  for (uint8_t i = 0; i < N; i++) {
    px[i] = (int16_t)(cx * 16);
    py[i] = (int16_t)(cy * 16);
    int8_t ang = (int8_t)(nextL() & 0x1F);     // 0..31
    int8_t spd = (int8_t)(16 + (int8_t)(nextL() & 0x0F)); // 16..31
    // Approx distribute vx,vy on 8 directions without trig
    int8_t dir = (int8_t)(ang & 7);
    const int8_t v8x[8] = { 16, 12,  0, -12, -16, -12,  0,  12 };
    const int8_t v8y[8] = {  0, 12, 16,  12,   0, -12,-16, -12 };
    vx[i] = (int8_t)((int16_t)v8x[dir] * spd / 24);
    vy[i] = (int8_t)((int16_t)v8y[dir] * spd / 24);
    life[i] = (uint8_t)(40 + (nextL() & 0x1F));
  }

  // Phase 0: blinding flash
  matrixFill(pixels.Color(255,255,255));
  pixels.show();
  delay((uint16_t)(stepDelay_ms / 2));
  matrixFade(140);
  pixels.show();
  delay((uint16_t)(stepDelay_ms / 2));

  uint8_t radius = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Gentle decay to create smoke and afterglow
    matrixFade(220);

    // Expanding shockwave ring
    uint8_t r2 = (uint8_t)(radius * radius);
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int8_t dx = (int8_t)x - (int8_t)cx;
        int8_t dy = (int8_t)y - (int8_t)cy;
        // squared distance without lambda to avoid unused warning
        uint16_t d2 = (uint16_t)((int16_t)dx * (int16_t)dx + (int16_t)dy * (int16_t)dy);
        // draw ring for |d^2 - r^2| within band
        int16_t diff = (int16_t)d2 - (int16_t)r2;
        if (diff < 0) diff = (int16_t)-diff;
        if (diff <= 2) {
          // hot color near core to cooler orange as it expands
          uint8_t heat = (uint8_t)(255 - (radius * 20)); // decreases with radius
          if (heat < 40) heat = 40;
          // heat to color: white/yellow -> orange/red
          uint8_t r = 255;
          uint8_t g = (uint8_t)(heat);
          uint8_t b = (uint8_t)(heat / 6);
          matrixSet(x, y, pixels.Color(r, g, b));
        }
      }
    }

    // Debris update and draw
    for (uint8_t i = 0; i < N; i++) {
      if (!life[i]) continue;
      // Integrate
      px[i] += vx[i];
      py[i] += vy[i];
      // gravity pull down
      if ((millis() & 1) == 0) vy[i] += 1; // tiny gravity
      // air drag
      vx[i] = (int8_t)((int16_t)vx[i] * 15 / 16);
      vy[i] = (int8_t)((int16_t)vy[i] * 15 / 16);
      // convert to pixel
      int8_t ix = (int8_t)(px[i] / 16);
      int8_t iy = (int8_t)(py[i] / 16);
      if (ix >= 0 && ix < (int8_t)W && iy >= 0 && iy < (int8_t)H) {
        uint8_t lv = life[i];
        uint8_t rr = (uint8_t)(200 + (lv));
        if (rr > 255) rr = 255;
        uint8_t gg = (uint8_t)(80 + (lv / 2));
        uint8_t bb = (uint8_t)(20 + (lv / 8));
        matrixSet((uint8_t)ix, (uint8_t)iy, pixels.Color(rr, gg, bb));
      }
      if (life[i] > 0) life[i]--;
    }

    // Slight smoky blue-gray drift overlay
    if ((radius & 1) == 0) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t y = (uint8_t)((x + radius) % H);
        uint32_t c = pixels.Color(30, 40, 60);
        matrixSet(x, y, dimColor(c, 80));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    radius++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowZoom
   Concentric rainbow rings that appear to zoom in/out.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowZoom(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t phase = 0;
  const uint8_t W = 5, H = 4;
  const int16_t CX = 2 * 16 + 8; // 2.5
  const int16_t CY = 1 * 16 + 8; // 1.5
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int16_t fx = (int16_t)x * 16 + 8;
        int16_t fy = (int16_t)y * 16 + 8;
        int16_t dx = (fx - CX);
        int16_t dy = (fy - CY);
        uint16_t manh = (uint16_t)(abs(dx) + abs(dy));
        uint8_t hue = (uint8_t)((manh * 4 + phase) & 0xFF);
        uint32_t col = dimColor(Wheel(hue), 200);
        matrixSet(x, y, col);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    phase += 9;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBouncingDot
   A single colored dot bounces around with a fading trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBouncingDot(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  int8_t x = 0, y = 0;
  int8_t vx = 1, vy = 1;
  uint32_t start = millis();
  uint16_t frame = 0;
  matrixFill(0);
  pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t hue = (frame * 6) & 0xFF;
    matrixSet((uint8_t)x, (uint8_t)y, Wheel(hue));
    pixels.show();
    delay(stepDelay_ms);
    x += vx; y += vy;
    if (x < 0) { x = 0; vx = -vx; }
    if (x >= MATRIX_W) { x = MATRIX_W - 1; vx = -vx; }
    if (y < 0) { y = 0; vy = -vy; }
    if (y >= MATRIX_H) { y = MATRIX_H - 1; vy = -vy; }
    frame++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSpinner
   A rainbow dot runs around the outer edge of the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSpinner(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t pathLen = (MATRIX_W * 2) + (MATRIX_H * 2) - 4;
  uint8_t xs[24], ys[24], idx = 0;
  for (uint8_t x = 0; x < MATRIX_W; x++) { xs[idx] = x; ys[idx] = 0; idx++; }
  for (uint8_t y = 1; y < MATRIX_H; y++) { xs[idx] = MATRIX_W - 1; ys[idx] = y; idx++; }
  for (int8_t x = MATRIX_W - 2; x >= 0; x--) { xs[idx] = (uint8_t)x; ys[idx] = MATRIX_H - 1; idx++; }
  for (int8_t y = MATRIX_H - 2; y > 0; y--) { xs[idx] = 0; ys[idx] = (uint8_t)y; idx++; }
  uint32_t start = millis();
  matrixFill(0); pixels.show();
  uint16_t step = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t p = step % pathLen;
    uint32_t c = Wheel((step * 5) & 0xFF);
    matrixSet(xs[p], ys[p], c);
    pixels.show();
    delay(stepDelay_ms);
    step++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixPong
   Mini Pong: AI paddles on the sides, ball bounces with simple spin.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixPong(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t PADDLE_H = 2;
  const uint8_t LEFT_X = 0;
  const uint8_t RIGHT_X = MATRIX_W - 1;
  int8_t ballX = MATRIX_W / 2, ballY = MATRIX_H / 2;
  int8_t vx = 1, vy = 1;
  int8_t lpY = 1, rpY = 1;
  const uint32_t colBall = pixels.Color(255, 200, 220);
  const uint32_t colPaddle = pixels.Color(40, 200, 120);
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);
    for (uint8_t i = 0; i < PADDLE_H; i++) {
      int8_t ly = lpY + i; if (ly >= 0 && ly < MATRIX_H) matrixSet(LEFT_X, (uint8_t)ly, colPaddle);
      int8_t ry = rpY + i; if (ry >= 0 && ry < MATRIX_H) matrixSet(RIGHT_X, (uint8_t)ry, colPaddle);
    }
    if (ballX >= 0 && ballX < MATRIX_W && ballY >= 0 && ballY < MATRIX_H) matrixSet((uint8_t)ballX, (uint8_t)ballY, colBall);
    pixels.show();
    delay(stepDelay_ms);
    ballX += vx; ballY += vy;
    if (ballY < 0) { ballY = 0; vy = -vy; }
    if (ballY >= MATRIX_H) { ballY = MATRIX_H - 1; vy = -vy; }
    if (vx < 0 && ballX == (LEFT_X + 1)) { if (ballY >= lpY && ballY < (lpY + PADDLE_H)) { vx = 1; vy = ((ballY - lpY) == 0) ? -1 : 1; } }
    if (vx > 0 && ballX == (RIGHT_X - 1)) { if (ballY >= rpY && ballY < (rpY + PADDLE_H)) { vx = -1; vy = ((ballY - rpY) == 0) ? -1 : 1; } }
    if (ballX < 0 || ballX >= MATRIX_W) { ballX = MATRIX_W / 2; ballY = MATRIX_H / 2; vx = (random(2) == 0) ? 1 : -1; vy = (random(2) == 0) ? 1 : -1; }
    int8_t lpCenter = lpY + (PADDLE_H / 2); if (ballY > lpCenter) lpY++; else if (ballY < lpCenter) lpY--; if (lpY < 0) lpY = 0; if (lpY > (MATRIX_H - PADDLE_H)) lpY = MATRIX_H - PADDLE_H;
    int8_t rpCenter = rpY + (PADDLE_H / 2); if (ballY > rpCenter) rpY++; else if (ballY < rpCenter) rpY--; if (rpY < 0) rpY = 0; if (rpY > (MATRIX_H - PADDLE_H)) rpY = MATRIX_H - PADDLE_H;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSwitchToggle
   A small switch body whose lever animates between closed/open.
   Params: runtime_ms, frameDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSwitchToggle(uint16_t runtime_ms, uint16_t frameDelay_ms) {
  const uint8_t F = 4;
  const uint8_t frames[F][20] = {
    {0,1,1,1,0, 0,1,2,1,0, 0,1,1,1,0, 0,0,1,0,0},
    {0,1,1,1,0, 0,1,1,2,0, 0,1,1,1,0, 0,0,1,0,0},
    {0,1,1,1,0, 0,2,1,1,0, 0,1,1,1,0, 0,0,1,0,0},
    {0,1,1,1,0, 2,1,1,1,0, 0,1,1,1,0, 0,0,1,0,0}
  };
  uint32_t start = millis();
  uint8_t idx = 0; int8_t dir = 1;
  const uint32_t body = pixels.Color(80, 80, 80);
  const uint32_t leverOn = pixels.Color(255, 20, 120);
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t v = frames[idx][y * MATRIX_W + x];
        matrixSet(x, y, v == 0 ? 0 : (v == 1 ? body : leverOn));
      }
    }
    pixels.show();
    delay(frameDelay_ms);
    idx += dir;
    if (idx == 0 || idx == (F - 1)) dir = -dir;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixCometSweep
   A bright "comet" head sweeps through all pixels with a soft trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCometSweep(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0); pixels.show();
  uint16_t pos = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);
    uint8_t y = (pos / MATRIX_W) % MATRIX_H;
    uint8_t x = pos % MATRIX_W;
    int id = matrixIndex(x, y);
    pixels.setPixelColor(id, Wheel((pos * 7) & 0xFF));
    pixels.show();
    delay(stepDelay_ms);
    pos = (pos + 1) % (MATRIX_W * MATRIX_H);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixPinkSpiral
   A vivid pink dot traces an inward spiral path with a gentle trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixPinkSpiral(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t order[W * H][2];
  uint8_t left = 0, right = W - 1, top = 0, bottom = H - 1;
  uint8_t idx = 0;
  while (left <= right && top <= bottom) {
    for (uint8_t x = left; x <= right; x++) { order[idx][0] = x; order[idx][1] = top; idx++; }
    for (uint8_t y = top + 1; y <= bottom; y++) { order[idx][0] = right; order[idx][1] = y; idx++; }
    if (top < bottom) { for (int8_t x = right - 1; x >= (int8_t)left; x--) { order[idx][0] = (uint8_t)x; order[idx][1] = bottom; idx++; } }
    if (left < right) { for (int8_t y = bottom - 1; y > (int8_t)top; y--) { order[idx][0] = left; order[idx][1] = (uint8_t)y; idx++; } }
    left++; right--; top++; bottom--;
  }
  uint32_t start = millis(); uint16_t step = 0;
  matrixFill(0); pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);
    uint8_t p = step % (W * H);
    matrixSet(order[p][0], order[p][1], pixels.Color(255, 60, 140));
    pixels.show();
    delay(stepDelay_ms);
    step++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixGalagaInvader
   Classic alien sprite flaps as it sweeps across the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixGalagaInvader(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  static const uint8_t PROGMEM F0[20] = {
    0,1,0,1,0,
    1,1,2,1,1,
    1,1,1,1,1,
    0,1,0,1,0
  };
  static const uint8_t PROGMEM F1[20] = {
    0,1,0,1,0,
    1,1,2,1,1,
    1,0,1,0,1,
    1,1,0,1,1
  };
  const uint8_t* frames[2] = { F0, F1 };
  const uint32_t colBody = pixels.Color(60, 220, 100);
  const uint32_t colEye  = pixels.Color(255, 255, 255);
  uint32_t start = millis(); uint8_t fidx = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (int8_t ox = -4; ox <= (int8_t)W; ox++) {
      matrixFill(0);
      const uint8_t* spr = frames[fidx];
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t v = pgm_read_byte(&spr[y * 5 + x]);
          int8_t tx = (int8_t)x + ox; if (tx < 0 || tx >= (int8_t)W) continue;
          if (v == 1) matrixSet((uint8_t)tx, y, colBody); else if (v == 2) matrixSet((uint8_t)tx, y, colEye);
        }
      }
      pixels.show(); delay(stepDelay_ms); fidx ^= 1; if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
    for (int8_t ox = W; ox >= -4; ox--) {
      matrixFill(0);
      const uint8_t* spr = frames[fidx];
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t v = pgm_read_byte(&spr[y * 5 + x]);
          int8_t tx = (int8_t)x + ox; if (tx < 0 || tx >= (int8_t)W) continue;
          if (v == 1) matrixSet((uint8_t)tx, y, colBody); else if (v == 2) matrixSet((uint8_t)tx, y, colEye);
        }
      }
      pixels.show(); delay(stepDelay_ms); fidx ^= 1; if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixLightning
   Random lightning strikes with blue glow, flicker and occasional clicks.
   Params: runtime_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixLightning(uint16_t runtime_ms) {
  uint32_t start = millis(); randomSeed(millis());
  const uint8_t W = 5, H = 4;
  auto clampi = [](int v, int lo, int hi) -> int { return (v < lo) ? lo : (v > hi ? hi : v); };
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint16_t darkMs = 120 + (random(280));
    matrixFill(0); pixels.show(); delay(darkMs);
    uint8_t flashes = 1 + (random(3));
    for (uint8_t f = 0; f < flashes; f++) {
      int x = random(W), y = 0;
      uint8_t steps = H + random(2);
      matrixFill(0);
      for (uint8_t s = 0; s < steps; s++) {
        matrixSet((uint8_t)x, (uint8_t)y, pixels.Color(255, 255, 255));
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = clampi(x + dx, 0, W - 1);
            int ny = clampi(y + dy, 0, H - 1);
            matrixSet((uint8_t)nx, (uint8_t)ny, pixels.Color(80, 120, 255));
          }
        }
        pixels.show(); delay(12 + random(16));
        x += (int8_t)(random(3)) - 1; y += 1; if (y >= H) y = H - 1; x = clampi(x, 0, W - 1);
      }
      for (uint8_t k = 0; k < 3; k++) {
        matrixFade(200);
        for (uint8_t i = 0; i < 3; i++) { int rx = random(W); int ry = random(H); matrixSet((uint8_t)rx, (uint8_t)ry, pixels.Color(255, 255, 255)); }
        pixels.show(); delay(20 + random(20));
      }
      for (uint8_t d = 0; d < 4; d++) { matrixFade(120); pixels.show(); delay(18); }
      if (random(3) == 0) { playSound(80 + random(60), 20 + random(30)); }
    }
  }
}

// Flags engine (5x4)
inline void displayFlag(const uint32_t flag[20]) {
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      uint32_t c = pgm_read_dword(&flag[y * 5 + x]);
      uint8_t r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
      matrixSet(x, y, pixels.Color(r, g, b));
    }
  }
  pixels.show();
}

static const uint32_t COL_BLACK  = 0;
static const uint32_t COL_WHITE  = 0xFFFFFF;
static const uint32_t COL_RED    = 0xFF0000;
static const uint32_t COL_BLUE   = 0x0000FF;
static const uint32_t COL_YELLOW = 0xFFFF00;
static const uint32_t COL_GREEN  = 0x00FF00;
static const uint32_t COL_ORANGE = 0xFF8C00;

static const uint32_t FLAG_DE[20] PROGMEM = {
  COL_BLACK, COL_BLACK, COL_BLACK, COL_BLACK, COL_BLACK,
  COL_RED,   COL_RED,   COL_RED,   COL_RED,   COL_RED,
  COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,
  COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE
};
static const uint32_t FLAG_FR[20] PROGMEM = {
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED
};
static const uint32_t FLAG_IT[20] PROGMEM = {
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED
};
static const uint32_t FLAG_CH[20] PROGMEM = {
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED,
  COL_RED, COL_WHITE, COL_WHITE, COL_WHITE, COL_RED,
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED
};
static const uint32_t FLAG_SE[20] PROGMEM = {
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW,
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE
};
static const uint32_t FLAG_UA[20] PROGMEM = {
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW
};
static const uint32_t FLAG_PL[20] PROGMEM = {
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED
};
static const uint32_t FLAG_NL[20] PROGMEM = {
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE
};

// Simple pirate flag: black field with a small white skull-like symbol
static const uint32_t FLAG_PIRATE[20] PROGMEM = {
  COL_BLACK, COL_WHITE, COL_BLACK, COL_WHITE, COL_BLACK,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_BLACK, COL_WHITE, COL_WHITE, COL_WHITE, COL_BLACK,
  COL_BLACK, COL_WHITE, COL_BLACK, COL_WHITE, COL_BLACK
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFlagsShow
   Cycles through several distinct 5x4 national flags (DE, FR, IT, CH, SE, UA, PL, NL, PIRATE).
   - Each flag is displayed as a solid, static 5x4 image.
   - Instantly jumps between flags after the specified hold duration.
   Params: runtime_ms, hold_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFlagsShow(uint16_t runtime_ms, uint16_t hold_ms) {
  const uint8_t N = 9; const uint32_t* flags[N] = { FLAG_DE, FLAG_FR, FLAG_IT, FLAG_CH, FLAG_SE, FLAG_UA, FLAG_PL, FLAG_NL, FLAG_PIRATE };
  uint32_t start = millis(); uint8_t idx = 0; while ((uint16_t)(millis() - start) < runtime_ms) { displayFlag(flags[idx]); delay(hold_ms); idx = (idx + 1) % N; }
}

inline void fadeFlags(const uint32_t* a, const uint32_t* b, uint16_t fade_ms, uint8_t steps) {
  if (steps == 0 || fade_ms == 0) return;
  uint16_t stepDelay = fade_ms / steps;
  for (uint8_t s = 1; s <= steps; s++) {
    uint8_t w2 = s; uint8_t w1 = steps - s;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t i = y * 5 + x; uint32_t c1 = pgm_read_dword(&a[i]); uint32_t c2 = pgm_read_dword(&b[i]);
        uint8_t r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF; uint8_t r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
        uint8_t r = (uint16_t)r1 * w1 / steps + (uint16_t)r2 * w2 / steps; uint8_t g = (uint16_t)g1 * w1 / steps + (uint16_t)g2 * w2 / steps; uint8_t bch = (uint16_t)b1 * w1 / steps + (uint16_t)b2 * w2 / steps;
        matrixSet(x, y, pixels.Color(r, g, bch));
      }
    }
    pixels.show(); delay(stepDelay);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFlagsShowFade
   Cycles through a series of national flags using smooth cross-fades.
   - Gradually interpolates the RGB values of every pixel to the next flag.
   - Configurable fade duration for smooth or snappy transitions.
   Params: runtime_ms, hold_ms, fade_ms, steps
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFlagsShowFade(uint16_t runtime_ms, uint16_t hold_ms, uint16_t fade_ms, uint8_t steps) {
  const uint8_t N = 9; const uint32_t* flags[N] = { FLAG_DE, FLAG_FR, FLAG_IT, FLAG_CH, FLAG_SE, FLAG_UA, FLAG_PL, FLAG_NL, FLAG_PIRATE };
  uint32_t start = millis(); uint8_t idx = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    const uint32_t* cur = flags[idx]; const uint32_t* nxt = flags[(idx + 1) % N];
    displayFlag(cur); delay(hold_ms); fadeFlags(cur, nxt, fade_ms, steps); idx = (idx + 1) % N;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixDnBDancer
   Two-frame dancer that pulses on bass hits with accent colors.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixDnBDancer(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  static const uint8_t PROGMEM P0[20] = {
    0,1,0,1,0,
    0,1,1,1,0,
    1,0,1,0,1,
    0,1,0,1,0
  };
  static const uint8_t PROGMEM P1[20] = {
    0,1,0,1,0,
    1,1,1,1,1,
    0,1,0,1,0,
    1,0,1,0,1
  };
  const uint8_t* frames[2] = { P0, P1 };
  uint32_t start = millis();
  uint16_t beat = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    bool bass = ((beat & 0x07) == 0);
    if (bass) {
      for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++) matrixSet(x, y, dimColor(pixels.Color(255,60,140), 80));
    } else {
      matrixFade(64);
    }
    uint8_t f = (beat & 0x01);
    const uint8_t* spr = frames[f];
    uint32_t colBody = pixels.Color(255, 80, 160);
    uint32_t colAcc  = bass ? pixels.Color(40, 200, 200) : pixels.Color(200, 120, 220);
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t v = pgm_read_byte(&spr[y * W + x]);
        if (v == 1) matrixSet(x, y, colBody);
        else if (v == 2) matrixSet(x, y, colAcc);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    beat++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixKanjiScroll
   Scrolls a 10x4 kanji-like glyph across the 5x4 display.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixKanjiScroll(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  const uint8_t GW = 10;
  static const uint8_t PROGMEM glyph[40] = {
    0,1,0,0,1, 0,1,0,0,0,
    1,0,1,0,1, 0,1,0,0,0,
    0,1,0,1,0, 1,0,1,0,0,
    0,0,1,0,0, 1,0,1,0,0
  };
  const uint32_t colOn  = pixels.Color(255, 60, 140);
  uint32_t start = millis();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (int8_t ox = W; ox >= -(int8_t)GW; ox--) {
      matrixFill(0);
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t gx = 0; gx < GW; gx++) {
          uint8_t v = pgm_read_byte(&glyph[y * GW + gx]);
          int8_t tx = (int8_t)gx + ox;
          if (v && tx >= 0 && tx < (int8_t)W) {
            matrixSet((uint8_t)tx, y, colOn);
          }
        }
      }
      pixels.show();
      delay(stepDelay_ms);
      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixThaiLogo
   Scrolls a 10x4 Thai-like logo across the 5x4 display in blue and pink.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixThaiLogo(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  const uint8_t GW = 10;
  static const uint8_t PROGMEM glyph[40] = {
    0,1,1,0,0, 0,0,0,1,0,
    1,0,1,0,0, 0,0,1,0,1,
    1,1,1,1,0, 0,0,1,0,1,
    0,0,1,1,0, 0,0,0,1,1
  };
  const uint32_t colPink = pixels.Color(255, 40, 180);
  const uint32_t colBlue = pixels.Color(40, 150, 255);
  uint32_t start = millis();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (int8_t ox = W; ox >= -(int8_t)GW; ox--) {
      matrixFade(100);
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t gx = 0; gx < GW; gx++) {
          uint8_t v = pgm_read_byte(&glyph[y * GW + gx]);
          int8_t tx = (int8_t)gx + ox;
          if (v && tx >= 0 && tx < (int8_t)W) {
            uint32_t c = (gx < 5) ? colPink : colBlue;
            matrixSet((uint8_t)tx, y, c);
          }
        }
      }
      pixels.show();
      delay(stepDelay_ms);
      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRandomStrobe
   A fast, random frequency strobe effect. 
   Flashes the entire matrix brightly at changing, unpredictable intervals.
   Params: runtime_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRandomStrobe(uint16_t runtime_ms) {
  uint32_t start = millis();
  
  // Tiny LFSR for fast pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0xABCD);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(pixels.Color(255, 255, 255));
    pixels.show();
    
    uint16_t onMs = 5 + (nextL() % 10);
    delay(onMs);
    
    matrixFill(0);
    pixels.show();
    
    uint16_t offMs = 10 + (nextL() % 70);
    delay(offMs);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixClappingMusic
   Flashing pattern based on Steve Reich's "Clapping Music".
   Top half plays the static pattern, bottom half plays the shifting pattern.
   Params: runtime_ms, stepDelay_ms (duration of an 8th note rest)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixClappingMusic(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t pattern[12] = {1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0};
  
  uint16_t beatCount = 0;
  uint8_t shift = 0;
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint8_t beatInBar = beatCount % 12;
    uint8_t bars = beatCount / 12;
    
    // Shift the second pattern every 4 bars
    shift = (bars / 4) % 12;
    
    uint8_t p1 = pattern[beatInBar];
    uint8_t p2 = pattern[(beatInBar + shift) % 12];
    
    matrixFill(0);
    
    if (p1) {
      // Top half (Player 1)
      for (uint8_t x=0; x<MATRIX_W; x++) {
        matrixSet(x, 0, pixels.Color(255, 100, 40));
        matrixSet(x, 1, pixels.Color(255, 100, 40));
      }
    }
    
    if (p2) {
      // Bottom half (Player 2)
      for (uint8_t x=0; x<MATRIX_W; x++) {
        matrixSet(x, 2, pixels.Color(40, 100, 255));
        matrixSet(x, 3, pixels.Color(40, 100, 255));
      }
    }
    
    pixels.show();
    
    // The visual "hit" is quick, the rest of the 8th note is dark
    uint16_t flashDuration = stepDelay_ms / 2;
    delay(flashDuration);
    
    matrixFill(0);
    pixels.show();
    
    delay(stepDelay_ms - flashDuration);
    
    beatCount++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixYellowRedBouncing
   Yellow and red balls bouncing back and forth horizontally with fading trails
   and bright wall impact flashes.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixYellowRedBouncing(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  // 8.8 fixed-point position
  int16_t px1 = 0; // starts left
  int16_t py1 = (1) << 8; // row 1
  int16_t px2 = (W - 1) << 8; // starts right
  int16_t py2 = (2) << 8; // row 2
  
  // 8.8 fixed-point velocity (high horizontal velocity, low vertical velocity)
  int16_t vx1 = 150; 
  int16_t vy1 = 30;
  int16_t vx2 = -130;
  int16_t vy2 = -25;
  
  matrixFill(0);
  pixels.show();
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(100); // Nice long smooth trail
    
    px1 += vx1;
    py1 += vy1;
    px2 += vx2;
    py2 += vy2;
    
    bool hitX1 = false, hitY1 = false;
    bool hitX2 = false, hitY2 = false;
    
    // Ball 1 bounds checking
    if (px1 <= 0) {
      px1 = 0; vx1 = -vx1; hitX1 = true;
    } else if (px1 >= ((W - 1) << 8)) {
      px1 = ((W - 1) << 8); vx1 = -vx1; hitX1 = true;
    }
    if (py1 <= 0) {
      py1 = 0; vy1 = -vy1; hitY1 = true;
    } else if (py1 >= ((H - 1) << 8)) {
      py1 = ((H - 1) << 8); vy1 = -vy1; hitY1 = true;
    }
    
    // Ball 2 bounds checking
    if (px2 <= 0) {
      px2 = 0; vx2 = -vx2; hitX2 = true;
    } else if (px2 >= ((W - 1) << 8)) {
      px2 = ((W - 1) << 8); vx2 = -vx2; hitX2 = true;
    }
    if (py2 <= 0) {
      py2 = 0; vy2 = -vy2; hitY2 = true;
    } else if (py2 >= ((H - 1) << 8)) {
      py2 = ((H - 1) << 8); vy2 = -vy2; hitY2 = true;
    }
    
    uint8_t bx1 = (uint8_t)(px1 >> 8);
    uint8_t by1 = (uint8_t)(py1 >> 8);
    uint8_t bx2 = (uint8_t)(px2 >> 8);
    uint8_t by2 = (uint8_t)(py2 >> 8);
    
    // Draw Ball 1 (Yellow)
    if (hitX1 || hitY1) {
      matrixSet(bx1, by1, pixels.Color(255, 255, 180)); // bright white-yellow flash
    } else {
      matrixSet(bx1, by1, pixels.Color(255, 200, 0)); // yellow
    }
    
    // Draw Ball 2 (Red)
    if (hitX2 || hitY2) {
      matrixSet(bx2, by2, pixels.Color(255, 180, 180)); // bright white-red flash
    } else {
      matrixSet(bx2, by2, pixels.Color(255, 0, 0)); // red
    }
    
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixLarsonScanner
   Classic Cylon/Knight Rider scanner with symmetric fading tails.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixLarsonScanner(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  int8_t pos = 0;
  int8_t dir = 1;
  int8_t y = (MATRIX_H > 2) ? 2 : 1;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(40);
    uint32_t head = pixels.Color(255, 0, 0);
    uint32_t tail1 = dimColor(head, 140);
    uint32_t tail2 = dimColor(head, 40);
    int8_t yUp = ((int8_t)y) - 1;
    int8_t yDown = ((int8_t)y) + 1;
    uint32_t headAdj = dimColor(head, 90);
    uint32_t tail1Adj = dimColor(headAdj, 140);
    uint32_t tail2Adj = dimColor(headAdj, 80);
    uint32_t tail3Adj = dimColor(headAdj, 40);
    uint32_t tail4Adj = 0; // fully off at the far tail for above/below lines
    // Boost center head with a hint of white for extra brightness
    uint32_t headCenter = pixels.Color(255, 40, 40);
    matrixSet((uint8_t)pos, y, headCenter);
    if (pos - 1 >= 0) matrixSet((uint8_t)(pos - 1), y, tail1);
    if (pos + 1 < MATRIX_W) matrixSet((uint8_t)(pos + 1), y, tail1);
    if (pos - 2 >= 0) matrixSet((uint8_t)(pos - 2), y, tail2);
    if (pos + 2 < MATRIX_W) matrixSet((uint8_t)(pos + 2), y, tail2);
    if (yUp >= 0) {
      matrixSet((uint8_t)pos, (uint8_t)yUp, headAdj);
      if (pos - 1 >= 0) matrixSet((uint8_t)(pos - 1), (uint8_t)yUp, tail1Adj);
      if (pos + 1 < MATRIX_W) matrixSet((uint8_t)(pos + 1), (uint8_t)yUp, tail1Adj);
      if (pos - 2 >= 0) matrixSet((uint8_t)(pos - 2), (uint8_t)yUp, tail2Adj);
      if (pos + 2 < MATRIX_W) matrixSet((uint8_t)(pos + 2), (uint8_t)yUp, tail2Adj);
      if (pos - 3 >= 0) matrixSet((uint8_t)(pos - 3), (uint8_t)yUp, tail3Adj);
      if (pos + 3 < MATRIX_W) matrixSet((uint8_t)(pos + 3), (uint8_t)yUp, tail3Adj);
      if (pos - 4 >= 0) matrixSet((uint8_t)(pos - 4), (uint8_t)yUp, tail4Adj);
      if (pos + 4 < MATRIX_W) matrixSet((uint8_t)(pos + 4), (uint8_t)yUp, tail4Adj);
    }
    if (yDown < (int8_t)MATRIX_H) {
      matrixSet((uint8_t)pos, (uint8_t)yDown, headAdj);
      if (pos - 1 >= 0) matrixSet((uint8_t)(pos - 1), (uint8_t)yDown, tail1Adj);
      if (pos + 1 < MATRIX_W) matrixSet((uint8_t)(pos + 1), (uint8_t)yDown, tail1Adj);
      if (pos - 2 >= 0) matrixSet((uint8_t)(pos - 2), (uint8_t)yDown, tail2Adj);
      if (pos + 2 < MATRIX_W) matrixSet((uint8_t)(pos + 2), (uint8_t)yDown, tail2Adj);
      if (pos - 3 >= 0) matrixSet((uint8_t)(pos - 3), (uint8_t)yDown, tail3Adj);
      if (pos + 3 < MATRIX_W) matrixSet((uint8_t)(pos + 3), (uint8_t)yDown, tail3Adj);
      if (pos - 4 >= 0) matrixSet((uint8_t)(pos - 4), (uint8_t)yDown, tail4Adj);
      if (pos + 4 < MATRIX_W) matrixSet((uint8_t)(pos + 4), (uint8_t)yDown, tail4Adj);
    }
    pixels.show();
    delay(stepDelay_ms);
    pos += dir;
    if (pos <= 0) { pos = 0; dir = 1; }
    if (pos >= (MATRIX_W - 1)) { pos = MATRIX_W - 1; dir = -1; }
  }
}

// Dual-row Larson scanner: top goes L->R, bottom goes R->L, both with tails
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixLarsonScannerDual
   A dual-row Larson scanner featuring two counter-propagating lines.
   - The top line sweeps Left-to-Right while the bottom sweeps Right-to-Left.
   - Both leave classic Knight Rider / Cylon fading trails.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixLarsonScannerDual(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t yTop = 0;
  const uint8_t yBot = (uint8_t)(MATRIX_H - 1);
  int8_t posTop = 0, dirTop = 1;
  int8_t posBot = (int8_t)(MATRIX_W - 1), dirBot = -1;
  uint8_t hue = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);
    uint32_t headCol = Wheel(hue);
    uint32_t tail1 = dimColor(headCol, 160);
    uint32_t tail2 = dimColor(headCol, 80);
    // Top row
    matrixSet((uint8_t)posTop, yTop, headCol);
    if (posTop - 1 >= 0) matrixSet((uint8_t)(posTop - 1), yTop, tail1);
    if (posTop - 2 >= 0) matrixSet((uint8_t)(posTop - 2), yTop, tail2);
    // Bottom row
    matrixSet((uint8_t)posBot, yBot, headCol);
    if (posBot + 1 < (int8_t)MATRIX_W) matrixSet((uint8_t)(posBot + 1), yBot, tail1);
    if (posBot + 2 < (int8_t)MATRIX_W) matrixSet((uint8_t)(posBot + 2), yBot, tail2);

    pixels.show();
    delay(stepDelay_ms);

    // Advance
    posTop += dirTop; posBot += dirBot; hue += 5;
    // Bounce
    if (posTop <= 0) { posTop = 0; dirTop = 1; }
    if (posTop >= (int8_t)(MATRIX_W - 1)) { posTop = (int8_t)(MATRIX_W - 1); dirTop = -1; }
    if (posBot <= 0) { posBot = 0; dirBot = 1; }
    if (posBot >= (int8_t)(MATRIX_W - 1)) { posBot = (int8_t)(MATRIX_W - 1); dirBot = -1; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBlueScreen
   A classic "Blue Screen of Death" simulation.
   - Entire display floods with maximum-intensity pure blue light.
   - Random subtle flickering to emulate a dying CRT or signal sync failure.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBlueScreen(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint32_t blue = pixels.Color(0, 0, 255);
  matrixFill(blue);
  pixels.show();
  uint16_t d = (stepDelay_ms == 0) ? 50 : stepDelay_ms;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    delay(d);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSnowWhite
   Intense randomized static noise, resembling a detuned analog television.
   - Rapidly flashes black and white pixels at maximum speed.
   - Perfect for jarring "Poltergeist" static transitions.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSnowWhite(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Cycle colors: white -> blue -> red
    uint8_t phase = (t / 16) % 3;
    uint32_t col = pixels.Color(255, 255, 255);
    if (phase == 1) col = pixels.Color(20, 80, 255); // soft blue dress
    if (phase == 2) col = pixels.Color(255, 40, 40); // red ribbon
    matrixFill(col);
    // "Apple" blink bottom-right
    if (((t / 8) & 1) == 0) {
      if (MATRIX_W >= 1 && MATRIX_H >= 1) {
        matrixSet((uint8_t)(MATRIX_W - 1), (uint8_t)(MATRIX_H - 1), pixels.Color(255, 0, 0));
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBigBadWolf
   An aggressive, intense stroboscopic display featuring jagged shapes.
   - High-contrast red and black flashes paired with jagged white teeth.
   - Fast, chaotic motion designed strictly for short burst impacts.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBigBadWolf(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  int8_t eyeX = 0, dir = 1;
  const uint8_t y = (MATRIX_H > 0) ? (MATRIX_H - 1) : 0; // bottom row
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Dark night background
    matrixFill(pixels.Color(0, 0, 10));
    // Moon glow at top-left
    if (MATRIX_W > 0 && MATRIX_H > 0) {
      matrixSet(0, 0, pixels.Color(200, 200, 180));
      if (MATRIX_W > 1) matrixSet(1, 0, pixels.Color(80, 80, 70));
      if (MATRIX_H > 1) matrixSet(0, 1, pixels.Color(80, 80, 70));
    }
    // Wolf eyes (two pixels) with faint red core and yellow edge
    uint32_t eyeCore = pixels.Color(220, 40, 0);
    uint32_t eyeEdge = pixels.Color(180, 140, 0);
    uint8_t x0 = (eyeX < 0) ? 0 : (uint8_t)eyeX;
    if (x0 < MATRIX_W) matrixSet(x0, y, eyeCore);
    if ((int)x0 + 1 < MATRIX_W) matrixSet((uint8_t)(x0 + 1), y, eyeEdge);
    pixels.show();
    delay(stepDelay_ms);
    // Move eyes with bounce
    eyeX += dir;
    if (eyeX <= 0) { eyeX = 0; dir = 1; }
    if (eyeX >= (int8_t)(MATRIX_W - 2)) { eyeX = (int8_t)(MATRIX_W - 2); dir = -1; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRain
   Blue droplets spawn at the top and fall down, leaving fading trails.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRain(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(64);
    if ((millis() & 0x03) == 0) {
      uint8_t x = (uint8_t)random(MATRIX_W);
      matrixSet(x, 0, pixels.Color(20, 60, 150));
    }
    for (int8_t y = MATRIX_H - 1; y >= 0; y--) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        int toIdx = matrixIndex(x, y);
        int fromIdx = (y == 0) ? -1 : matrixIndex(x, y - 1);
        uint32_t c = (fromIdx >= 0) ? pixels.getPixelColor(fromIdx) : 0;
        pixels.setPixelColor(toIdx, c);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTwinkle
   Soft, random multicolor twinkles that fade away gradually.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTwinkle(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(40);
    for (uint8_t i = 0; i < 2; i++) {
      uint8_t x = (uint8_t)random(MATRIX_W);
      uint8_t y = (uint8_t)random(MATRIX_H);
      uint8_t hue = (uint8_t)random(256);
      uint32_t c = dimColor(Wheel(hue), 160);
      matrixSet(x, y, c);
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixWipe
   Wipes a solid color across rows left-to-right and back again.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixWipe(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  uint8_t phase = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint8_t hue = (phase * 8);
    uint32_t c = Wheel(hue);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      if (y % 2 == 0) {
        for (uint8_t x = 0; x < MATRIX_W; x++) { matrixSet(x, y, c); pixels.show(); delay(stepDelay_ms); }
      } else {
        for (int8_t x = MATRIX_W - 1; x >= 0; x--) { matrixSet((uint8_t)x, y, c); pixels.show(); delay(stepDelay_ms); }
      }
    }
    matrixFill(0);
    pixels.show();
    phase++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTetris
   Simple falling blocks stack up; full rows are cleared.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTetris(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  // Use a logical 4x5 Tetris board so a 90° rotation fits on a 5x4 physical matrix without clipping.
  const uint8_t GW = (MATRIX_H < 4 ? MATRIX_H : 4); // logical width  up to 4
  const uint8_t GH = (MATRIX_W < 5 ? MATRIX_W : 5); // logical height up to 5
  uint8_t field[GW * GH];
  for (uint8_t i = 0; i < GW * GH; i++) field[i] = 0;

  // Seed randomness so piece sequence varies per run
  randomSeed((unsigned long)millis());

  auto idx = [&](int8_t x, int8_t y) -> int { return y * GW + x; };
  auto occupied = [&](int8_t x, int8_t y) -> bool {
    // Allow y < 0 (above top) to be empty so pieces can spawn/fall into view
    if (x < 0 || x >= (int8_t)GW || y >= (int8_t)GH) return true; // walls and floor are solid
    if (y < 0) return false; // above top is empty space
    return field[idx(x, y)] != 0;
  };

  // Base piece definitions around an origin (0,0). We'll allow simple orientation variants.
  const int8_t P0[4][2] = {{0,0},{1,0},{0,1},{1,1}};          // O (square)
  const int8_t P1[3][2] = {{0,0},{1,0},{2,0}};                // I-3 (short bar) base horizontal (fits tiny board better)
  const int8_t P2[4][2] = {{0,0},{0,1},{1,1},{2,1}};          // L base
  const int8_t P3[4][2] = {{2,0},{0,1},{1,1},{2,1}};          // J (mirror L) base
  const int8_t P4[4][2] = {{1,0},{0,1},{1,1},{2,1}};          // T base
  const int8_t P5[4][2] = {{1,0},{2,0},{0,1},{1,1}};          // S base
  const int8_t P6[4][2] = {{0,0},{1,0},{1,1},{2,1}};          // Z base

  // Helper to apply an orientation variant to piece coords (0..3): 0=identity, 1=rot90, 2=rot180, 3=rot270
  auto orient = [&](const int8_t base[][2], uint8_t count, uint8_t var, int8_t out[][2]) {
    for (uint8_t i = 0; i < count; i++) {
      int8_t x = base[i][0], y = base[i][1];
      int8_t rx = x, ry = y;
      switch (var & 0x03) {
        case 1: rx = -y; ry = x;  break;  // 90°
        case 2: rx = -x; ry = -y; break;  // 180°
        case 3: rx = y;  ry = -x; break;  // 270°
        default: break; // identity
      }
      out[i][0] = rx; out[i][1] = ry;
    }
    // Normalize so min x,y is 0 (top-left) to simplify spawn/fit calculations
    int8_t minx = 127, miny = 127;
    for (uint8_t i = 0; i < count; i++) { if (out[i][0] < minx) minx = out[i][0]; if (out[i][1] < miny) miny = out[i][1]; }
    for (uint8_t i = 0; i < count; i++) { out[i][0] = (int8_t)(out[i][0] - minx); out[i][1] = (int8_t)(out[i][1] - miny); }
  };

  // Map logical (gx,gy) on a GWxGH board to physical 5x4 rotated 90° COUNTERCLOCKWISE
  // CCW mapping on an MxN grid: (x,y) -> (N-1-y, x)
  auto drawRot = [&](uint8_t gx, uint8_t gy, uint32_t col) {
    uint8_t rx = (uint8_t)(GH - 1 - gy); // width side (maps to MATRIX_W)
    uint8_t ry = gx;                     // height side (maps to MATRIX_H)
    if (rx < MATRIX_W && ry < MATRIX_H) matrixSet(rx, ry, col);
  };

  uint32_t start = millis();
  uint8_t hueBase = 0;
  uint8_t pieceCounter = 0; // ensures variety even if RNG is weak

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Cycle kinds deterministically across 7 types; still vary orientation
    const uint8_t KINDS = 7;
    uint8_t kind = (uint8_t)(pieceCounter % KINDS);
    uint8_t var = (uint8_t)((pieceCounter * 3 + hueBase) & 0x03); // orientation 0..3
    // Build oriented piece coords and determine width/height for spawn bounds
    int8_t cells[4][2]; uint8_t count = 4;
    switch (kind) {
      case 0: orient(P0, 4, 0, cells); count = 4; break;               // O (no rotation changes)
      case 1: orient(P1, 3, var, cells); count = 3; break;              // I-3
      case 2: orient(P2, 4, var, cells); count = 4; break;              // L
      case 3: orient(P3, 4, var, cells); count = 4; break;              // J
      case 4: orient(P4, 4, var, cells); count = 4; break;              // T
      case 5: orient(P5, 4, var, cells); count = 4; break;              // S
      default: orient(P6, 4, var, cells); count = 4; break;             // Z
    }

    int8_t maxx = 0, maxy = 0; for (uint8_t i = 0; i < count; i++) { if (cells[i][0] > maxx) maxx = cells[i][0]; if (cells[i][1] > maxy) maxy = cells[i][1]; }
    // Safe spawn range so x..x+maxx fits in [0..GW-1]
    int8_t px = (int8_t)random(0, (int8_t)(GW - maxx)); if (px < 0) px = 0;
    // Spawn above top: py = -maxy so the highest cell enters first
    int8_t py = (int8_t)(-maxy);
    uint8_t hue = hueBase;

    bool locked = false;
    while (!locked && (uint16_t)(millis() - start) < runtime_ms) {
      // Fall DOWN: increase y until collision
      int8_t ny = (int8_t)(py + 1);
      bool hit = false;
      for (uint8_t i = 0; i < count; i++) { if (occupied((int8_t)(px + cells[i][0]), (int8_t)(ny + cells[i][1]))) { hit = true; break; } }

      if (hit) {
        // Lock piece into field only if at least one block is within bounds.
        // This avoids "stuck at top" when collision occurs while piece is still above the visible area.
        uint8_t wrote = 0;
        for (uint8_t i = 0; i < count; i++) {
          int8_t ax = (int8_t)(px + cells[i][0]);
          int8_t ay = (int8_t)(py + cells[i][1]);
          if (ax >= 0 && ax < (int8_t)GW && ay >= 0 && ay < (int8_t)GH) { field[idx(ax, ay)] = 1; wrote++; }
        }
        if (wrote == 0) { py = ny; continue; }
        locked = true;
        // Clear any full rows
        for (uint8_t y = 0; y < GH; y++) {
          bool full = true;
          for (uint8_t x = 0; x < GW; x++) { if (!field[idx(x, y)]) { full = false; break; } }
          if (full) {
            for (int yy = y; yy < (int)GH - 1; yy++) {
              for (uint8_t x = 0; x < GW; x++) field[idx(x, yy)] = field[idx(x, yy + 1)];
            }
            for (uint8_t x = 0; x < GW; x++) field[idx(x, GH - 1)] = 0;
            y--; // recheck
          }
        }
        // After lock, briefly render the updated field so placement is visible
        matrixFill(0);
        for (uint8_t ry = 0; ry < GH; ry++) {
          for (uint8_t rx = 0; rx < GW; rx++) {
            if (field[idx(rx, ry)]) drawRot(rx, ry, dimColor(pixels.Color(255,160,80), 180));
          }
        }
        pixels.show();
        delay((uint16_t)(stepDelay_ms > 40 ? 40 : stepDelay_ms));
        break;
      } else {
        py = ny;
      }

      // Render field + active piece (display rotated 90° counterclockwise)
      matrixFill(0);
      for (uint8_t y = 0; y < GH; y++) {
        for (uint8_t x = 0; x < GW; x++) {
          if (field[idx(x, y)]) drawRot(x, y, dimColor(pixels.Color(255,160,80), 180));
        }
      }
      uint32_t col = Wheel(hue);
      for (uint8_t i = 0; i < count; i++) {
        int8_t ax = (int8_t)(px + cells[i][0]);
        int8_t ay = (int8_t)(py + cells[i][1]);
        if (ax >= 0 && ax < (int8_t)GW && ay >= 0 && ay < (int8_t)GH) drawRot((uint8_t)ax, (uint8_t)ay, col);
      }
      pixels.show();
      delay(stepDelay_ms);
      hue += 7;
    }
    // Advance to next piece and vary hue baseline for visible variety
    pieceCounter++;
    hueBase = (uint8_t)(hueBase + 23);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTetrisFrantic
   A fast-paced Tetris simulation where the drop speed continually accelerates.
   - Falling blocks stack up and full rows are cleared.
   - The delay between drops becomes 10% shorter with every new piece.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTetrisFrantic(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t GW = (MATRIX_H < 4 ? MATRIX_H : 4);
  const uint8_t GH = (MATRIX_W < 5 ? MATRIX_W : 5);
  uint8_t field[GW * GH];
  for (uint8_t i = 0; i < GW * GH; i++) field[i] = 0;

  uint16_t currentDelay = stepDelay_ms;
  randomSeed((unsigned long)millis());

  auto idx = [&](int8_t x, int8_t y) -> int { return y * GW + x; };
  auto occupied = [&](int8_t x, int8_t y) -> bool {
    if (x < 0 || x >= (int8_t)GW || y >= (int8_t)GH) return true;
    if (y < 0) return false;
    return field[idx(x, y)] != 0;
  };

  const int8_t P0[4][2] = {{0,0},{1,0},{0,1},{1,1}};
  const int8_t P1[3][2] = {{0,0},{1,0},{2,0}};
  const int8_t P2[4][2] = {{0,0},{0,1},{1,1},{2,1}};
  const int8_t P3[4][2] = {{2,0},{0,1},{1,1},{2,1}};
  const int8_t P4[4][2] = {{1,0},{0,1},{1,1},{2,1}};
  const int8_t P5[4][2] = {{1,0},{2,0},{0,1},{1,1}};
  const int8_t P6[4][2] = {{0,0},{1,0},{1,1},{2,1}};

  auto orient = [&](const int8_t base[][2], uint8_t count, uint8_t var, int8_t out[][2]) {
    for (uint8_t i = 0; i < count; i++) {
      int8_t x = base[i][0], y = base[i][1];
      int8_t rx = x, ry = y;
      switch (var & 0x03) {
        case 1: rx = -y; ry = x;  break;
        case 2: rx = -x; ry = -y; break;
        case 3: rx = y;  ry = -x; break;
        default: break;
      }
      out[i][0] = rx; out[i][1] = ry;
    }
    int8_t minx = 127, miny = 127;
    for (uint8_t i = 0; i < count; i++) { if (out[i][0] < minx) minx = out[i][0]; if (out[i][1] < miny) miny = out[i][1]; }
    for (uint8_t i = 0; i < count; i++) { out[i][0] = (int8_t)(out[i][0] - minx); out[i][1] = (int8_t)(out[i][1] - miny); }
  };

  auto drawRot = [&](uint8_t gx, uint8_t gy, uint32_t col) {
    uint8_t rx = (uint8_t)(GH - 1 - gy);
    uint8_t ry = gx;
    if (rx < MATRIX_W && ry < MATRIX_H) matrixSet(rx, ry, col);
  };

  uint32_t start = millis();
  uint8_t hueBase = 0;
  uint8_t pieceCounter = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    const uint8_t KINDS = 7;
    uint8_t kind = (uint8_t)(pieceCounter % KINDS);
    uint8_t var = (uint8_t)((pieceCounter * 3 + hueBase) & 0x03);
    int8_t cells[4][2]; uint8_t count = 4;
    switch (kind) {
      case 0: orient(P0, 4, 0, cells); break;
      case 1: orient(P1, 3, var, cells); count = 3; break;
      case 2: orient(P2, 4, var, cells); break;
      case 3: orient(P3, 4, var, cells); break;
      case 4: orient(P4, 4, var, cells); break;
      case 5: orient(P5, 4, var, cells); break;
      default: orient(P6, 4, var, cells); break;
    }

    int8_t maxx = 0, maxy = 0; for (uint8_t i = 0; i < count; i++) { if (cells[i][0] > maxx) maxx = cells[i][0]; if (cells[i][1] > maxy) maxy = cells[i][1]; }
    int8_t px = (int8_t)random(0, (int8_t)(GW - maxx)); if (px < 0) px = 0;
    int8_t py = (int8_t)(-maxy);
    uint8_t hue = hueBase;

    bool locked = false;
    while (!locked && (uint16_t)(millis() - start) < runtime_ms) {
      int8_t ny = (int8_t)(py + 1);
      bool hit = false;
      for (uint8_t i = 0; i < count; i++) { if (occupied((int8_t)(px + cells[i][0]), (int8_t)(ny + cells[i][1]))) { hit = true; break; } }

      if (hit) {
        uint8_t wrote = 0;
        for (uint8_t i = 0; i < count; i++) {
          int8_t ax = (int8_t)(px + cells[i][0]);
          int8_t ay = (int8_t)(py + cells[i][1]);
          if (ax >= 0 && ax < (int8_t)GW && ay >= 0 && ay < (int8_t)GH) { field[idx(ax, ay)] = 1; wrote++; }
        }
        if (wrote == 0) { py = ny; continue; }
        locked = true;
        for (uint8_t y = 0; y < GH; y++) {
          bool full = true;
          for (uint8_t x = 0; x < GW; x++) { if (!field[idx(x, y)]) { full = false; break; } }
          if (full) {
            for (int yy = y; yy < (int)GH - 1; yy++) {
              for (uint8_t x = 0; x < GW; x++) field[idx(x, yy)] = field[idx(x, yy + 1)];
            }
            for (uint8_t x = 0; x < GW; x++) field[idx(x, GH - 1)] = 0;
            y--;
          }
        }
        matrixFill(0);
        for (uint8_t ry = 0; ry < GH; ry++) {
          for (uint8_t rx = 0; rx < GW; rx++) {
            if (field[idx(rx, ry)]) drawRot(rx, ry, dimColor(pixels.Color(255,160,80), 180));
          }
        }
        pixels.show();
        delay((uint16_t)(currentDelay > 40 ? 40 : currentDelay));
        break;
      } else {
        py = ny;
      }

      matrixFill(0);
      for (uint8_t y = 0; y < GH; y++) {
        for (uint8_t x = 0; x < GW; x++) {
          if (field[idx(x, y)]) drawRot(x, y, dimColor(pixels.Color(255,160,80), 180));
        }
      }
      uint32_t col = Wheel(hue);
      for (uint8_t i = 0; i < count; i++) {
        int8_t ax = (int8_t)(px + cells[i][0]);
        int8_t ay = (int8_t)(py + cells[i][1]);
        if (ax >= 0 && ax < (int8_t)GW && ay >= 0 && ay < (int8_t)GH) drawRot((uint8_t)ax, (uint8_t)ay, col);
      }
      pixels.show();
      delay(currentDelay);
      hue += 7;
    }
    
    pieceCounter++;
    if (currentDelay > 15) {
      currentDelay = (currentDelay * 9) / 10;
    }
    hueBase = (uint8_t)(hueBase + 23);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSchaffhauserBock
   A pixel-art tribute to the heraldic animal of Canton Schaffhausen.
   - Displays a stylized black ram rearing up on a golden yellow background.
   - Alternates between frames to simulate the ram bucking.
   - Features a glowing red eye for extra attitude.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSchaffhauserBock(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  
  uint32_t colBg = dimColor(pixels.Color(255, 180, 0), 100); // Golden yellow
  uint32_t colRam = pixels.Color(0, 0, 0); // Black
  uint32_t colPizzle = pixels.Color(255, 220, 0); // Vibrant Gold

  // 1 = Ram, 0 = Bg. Leftmost bit is x=0.
  const uint8_t frames[2][4] = {
    { 0b01100,
      0b01110,
      0b01110,
      0b10010 },
    { 0b11000,
      0b11100,
      0b01110,
      0b01010 }
  };

  uint8_t f = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        bool isRam = (frames[f][y] & (1 << (4 - x)));
        matrixSet(x, y, isRam ? colRam : colBg);
      }
    }
    
    // Glowing red eye for the Bock
    if (f == 0) {
       matrixSet(1, 0, pixels.Color(255, 0, 0)); 
    } else {
       matrixSet(0, 0, pixels.Color(255, 0, 0)); 
    }
    
    // Middle pixel in the lowest row (x=2, y=3) receives the golden touch
    matrixSet(2, 3, colPizzle);

    pixels.show();
    delay(stepDelay_ms);
    f = 1 - f; // toggle frame
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowWaves
   Flowing rainbow stripes across the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowWaves(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t phase = (uint8_t)(x * 32 + y * 48 + t);
        uint8_t wv = tri8_local(phase); // 0..255 wave
        uint8_t hue = (uint8_t)(phase + (wv >> 2));
        uint32_t col = dimColor(Wheel(hue), 200);
        matrixSet(x, y, col);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    t += 5;
  }
}

// Smooth rainbow wash across the matrix with gentle fade and slight vertical shading
inline void matrixRainbowWash(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint8_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(86);
    uint8_t base = (uint8_t)(t * 3);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t hue = (uint8_t)(base + x * 22 + y * 11);
        uint8_t lvl = (uint8_t)(190 + ((uint8_t)(y * 12) & 0x2F));
        matrixSet(x, y, dimColor(Wheel(hue), lvl));
      }
    }
    // occasional soft accent column drifting across
    if ((t & 0x07) == 0x00) {
      uint8_t sx = (uint8_t)((t >> 1) % MATRIX_W);
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        matrixSet(sx, y, dimColor(Wheel((uint8_t)(base + 128)), 160));
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

// Map a heat value (0..255) to a fire-like color (black -> red -> yellow -> white)
static inline uint32_t colorFromHeat(uint8_t heat) {
    uint8_t t192 = (uint16_t)heat * 191 / 255;   // 0..191
    uint8_t heatramp = (t192 & 63) << 2;         // 0..252 in steps of 4
    if (t192 > 128) {
      return pixels.Color(255, 255, heatramp);   // white-ish
    } else if (t192 > 64) {
      return pixels.Color(255, heatramp, 0);     // yellow->red
    } else {
      return pixels.Color(heatramp, 0, 0);       // dark red
    }
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFire
   Classic Fire2012-inspired effect adapted to 5x4 with hotter center.
   Params: runtime_ms, cooling, sparking, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFire(uint16_t runtime_ms, uint8_t cooling, uint8_t sparking, uint8_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t FW = MATRIX_W; // matrix width
  const uint8_t FH = MATRIX_H; // matrix height
  uint8_t heat[FW * FH];
  for (uint8_t i = 0; i < FW * FH; i++) heat[i] = 0;
  auto idxAt = [&](uint8_t x, uint8_t y) -> int { return y * FW + x; };
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Cool
    for (uint8_t x = 0; x < FW; x++) {
      for (uint8_t y = 0; y < FH; y++) {
        int i = idxAt(x, y);
        uint8_t cool = (uint8_t)random((cooling / FH) + 2);
        heat[i] = (heat[i] > cool) ? (heat[i] - cool) : 0;
      }
    }
    // Diffuse upward (include row 1 so it gets heat from row 0)
    for (int y = FH - 1; y >= 1; y--) {
      for (uint8_t x = 0; x < FW; x++) {
        int i = idxAt(x, y);
        int below = idxAt(x, y - 1);
        int below2 = idxAt(x, (y >= 2) ? (y - 2) : 0); // clamp for y==1
        heat[i] = (uint8_t)(((int)heat[below] + (int)heat[below] + (int)heat[below2]) / 3);
      }
    }
    // New sparks at the bottom
    for (uint8_t x = 0; x < FW; x++) {
      if ((uint8_t)random(255) < sparking) {
        int i = idxAt(x, 0);
        {
          int v = (int)heat[i] + (int)random(160, 255);
          if (v > 255) v = 255;
          heat[i] = (uint8_t)v;
        }
      }
    }
    // Breathing edge scaling
    uint8_t tb = (uint8_t)((millis() >> 6) & 0xFF);
    uint8_t tri = (tb & 0x80) ? (uint8_t)(255 - ((tb & 0x7F) << 1)) : (uint8_t)(((tb & 0x7F) << 1));
    int16_t delta = (int16_t)tri - 128; // -128..127
    int16_t s1i = 200 + (delta * 20) / 128; // near edge 180..220
    int16_t s2i = 110 + (delta * 25) / 128; // far edge 85..135
    uint8_t s1 = (s1i < 0) ? 0 : (s1i > 255 ? 255 : (uint8_t)s1i);
    uint8_t s2 = (s2i < 0) ? 0 : (s2i > 255 ? 255 : (uint8_t)s2i);

    // Draw with hotter center
    for (uint8_t y = 0; y < FH; y++) {
      for (uint8_t x = 0; x < FW; x++) {
        int i = idxAt(x, y);
        uint8_t center = FW / 2; // integer center
        uint8_t d = (x > center) ? (x - center) : (center - x);
        uint8_t scale = (d == 0) ? 255 : (d == 1 ? s1 : s2);
        uint8_t adj = (uint16_t)heat[i] * scale / 255;
        matrixSet(x, y, colorFromHeat(adj));
      }
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixUniverseCreation
   "My theory is, when a particle with super-light-speed zips around and hits another in the universe, then what? 
    Then it hits itself. Then it hits itself and then a new universe is created."
   Phase 1: Super-luminal particle zips around (fast random movement with trail).
   Phase 2: Collision (White flash).
   Phase 3: New Universe (Expanding swirling colors).
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixUniverseCreation(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  // Phase timing
  uint32_t collisionTime = runtime_ms / 3;
  // collision lasts ~500ms
  
  // Particle state for Phase 1
  float px = W / 2.0f;
  float py = H / 2.0f;
  float vx = 0.8f;
  float vy = 0.5f;
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint32_t elapsed = millis() - start;
    
    if (elapsed < collisionTime) {
      // Phase 1: Super-luminal particle
      matrixFade(100); // Fast fade for trail
      
      // Update position
      px += vx;
      py += vy;
      
      // Bounce
      if (px < 0) { px = 0; vx = -vx; }
      else if (px >= W) { px = W - 0.1f; vx = -vx; }
      
      if (py < 0) { py = 0; vy = -vy; }
      else if (py >= H) { py = H - 0.1f; vy = -vy; }
      
      // Randomize velocity occasionally to simulate "super-luminal chaos"
      if (random(5) == 0) {
        vx = (float)(random(100) - 50) / 30.0f;
        vy = (float)(random(100) - 50) / 30.0f;
        // Ensure minimum speed
        if (vx > -0.3f && vx < 0.3f) vx = (vx < 0) ? -0.5f : 0.5f;
        if (vy > -0.3f && vy < 0.3f) vy = (vy < 0) ? -0.5f : 0.5f;
      }
      
      matrixSet((uint8_t)px, (uint8_t)py, pixels.Color(200, 200, 255)); // Blue-white hot particle
      
    } else if (elapsed < collisionTime + 400) {
      // Phase 2: The Collision / Singularity
      // Flash center intensely
      matrixFill(pixels.Color(255, 255, 255));
      // playSound(1000 + random(2000), 10); // (Optional sound if speaker present)
      
    } else {
      // Phase 3: New Universe (Expansion)
      // Swirling galaxy/nebula colors
      matrixFade(15); // Very slow fade for nebulous accumulation
      
      uint32_t universeAge = elapsed - (collisionTime + 400);
      uint8_t baseHue = (uint8_t)((universeAge / 10) & 0xFF);
      
      // Random "matter creation" events
      if (random(2) == 0) {
        uint8_t rx = (uint8_t)random(W);
        uint8_t ry = (uint8_t)random(H);
        
        // Calculate distance from center for color variation
        int8_t cx = W / 2; 
        int8_t cy = H / 2;
        int dist = abs(rx - cx) + abs(ry - cy);
        
        // Hue shifts with distance and time
        uint8_t h = baseHue + (dist * 15);
        
        // Brightness variation
        uint8_t bri = 100 + random(155);
        matrixSet(rx, ry, dimColor(Wheel(h), bri));
      }
    }
    
    pixels.show();
    // Phase 1 is faster than the rest
    uint16_t delayTime = (elapsed < collisionTime) ? (stepDelay_ms / 2) : stepDelay_ms;
    delay(delayTime);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSpear
   "Bring me a spear!"
   A spear is thrown across the matrix.
   - It appears from one side and flies to the other.
   - Has a sharp tip and a shaft.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSpear(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  int16_t spearPos = -4; // Start outside
  uint8_t row = 0;
  int8_t dir = 1; // 1 = right, -1 = left

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(60); // Clear trail

    // Draw Spear
    // Tip
    int16_t tipX = spearPos;
    if (dir == -1) tipX = spearPos; // Tip is leading
    else tipX = spearPos + 2;
    
    // Shaft
    for (int i = 0; i < 3; i++) {
       int16_t px = spearPos + i;
       if (px >= 0 && px < W) {
         if ((dir == 1 && px == tipX) || (dir == -1 && px == spearPos)) {
            // Tip: Silver/White
            matrixSet(px, row, pixels.Color(200, 200, 255));
         } else {
            // Shaft: Brown
            matrixSet(px, row, pixels.Color(139, 69, 19));
         }
       }
    }

    spearPos += dir;

    // Reset if out of bounds
    if ((dir == 1 && spearPos > W + 1) || (dir == -1 && spearPos < -4)) {
      row = random(H);
      dir = (random(2) == 0) ? 1 : -1;
      spearPos = (dir == 1) ? -4 : W + 1;
      // Randomize speed slightly via stepDelay? No, just keep constant for now.
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixCCCRocket
   "Mega Blinker-Rakete!" - A tribute to the Chaos Computer Club.
   Launches a rocket with a blinking engine and a fairy dust trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCCCRocket(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  // Rocket state
  float ry = H + 2;  // Start below screen
  float speed = 0.0; // Vertical speed
  uint8_t rx = W / 2; // Horizontal center
  bool exploded = false;
  int16_t ex = rx, ey = 0;   // explosion center
  uint8_t explosionTicks = 0;
  
  // Trail particles (simple brightness decay)
  uint8_t trail[20]; // 5x4
  memset(trail, 0, sizeof(trail));
  
  uint16_t frame = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Fade trail every frame
    for (uint8_t i = 0; i < W * H; i++) {
      if (trail[i] > 12) trail[i] = (uint8_t)(trail[i] - 12);
      else trail[i] = 0;
    }

    if (!exploded) {
      // Reset if rocket is far above
      if (ry < -4) {
        ry = H + 2;
        rx = (uint8_t)random(W);
        speed = 0.0;
      }

      // Accelerate upwards
      speed += 0.05f;
      if (speed > 0.8f) speed = 0.8f;
      ry -= speed;

      int16_t ty = (int16_t)ry;       // rocket tip
      int16_t fy = (int16_t)(ry + 2); // engine flame

      // When getting near the top, trigger explosion instead of just disappearing
      if (ty <= 0) {
        exploded = true;
        ex = rx;
        ey = (ty < 0) ? 0 : ty;
        explosionTicks = 0;
      } else {
        // Feed trail from engine
        if (fy >= 0 && fy < (int16_t)H) {
          uint8_t idx = (uint8_t)(fy * W + rx);
          trail[idx] = 255;
        }

        matrixFill(0);

        // Draw trail as hot fire
        for (uint8_t y = 0; y < H; y++) {
          for (uint8_t x = 0; x < W; x++) {
            uint8_t heat = trail[y * W + x];
            if (!heat) continue;
            uint32_t c;
            if (heat > 200)      c = pixels.Color(255, 255, 200);          // white-hot
            else if (heat > 100) c = pixels.Color(255, 200, 60);           // yellow
            else                 c = pixels.Color(heat * 2, 0, 0);         // red
            matrixSet(x, y, c);
          }
        }

        // Draw rocket tip (silver)
        if (ty >= 0 && ty < (int16_t)H) {
          matrixSet(rx, ty, pixels.Color(220, 220, 220));
        }

        // Body with fast blinking pink engine light
        int16_t by = (int16_t)(ry + 1);
        if (by >= 0 && by < (int16_t)H) {
          bool blink = ((frame >> 1) & 0x01) == 0; // fast blink
          if (blink) {
            // Pink rocket body
            matrixSet(rx, by, pixels.Color(255, 80, 160));
          } else {
            // Dim grey between blinks
            matrixSet(rx, by, pixels.Color(80, 80, 80));
          }
        }
      }
    } else {
      // Explosion phase: brief colorful burst around (ex, ey)
      matrixFade(80); // let old sparks die quickly

      // Spawn a few new sparks each frame
      for (uint8_t i = 0; i < 4; i++) {
        int8_t dx = (int8_t)((random(3)) - 1); // -1,0,1
        int8_t dy = (int8_t)((random(3)) - 1);
        int16_t sx = ex + dx;
        int16_t sy = ey + dy;
        if (sx < 0 || sx >= (int16_t)W || sy < 0 || sy >= (int16_t)H) continue;

        uint8_t pick = (uint8_t)(random(4));
        uint32_t c;
        switch (pick) {
          default:
          case 0: c = pixels.Color(255, 120, 200); break; // bright pink
          case 1: c = pixels.Color(255, 220, 180); break; // warm white
          case 2: c = pixels.Color(255, 180, 60);  break; // gold
          case 3: c = pixels.Color(120, 200, 255); break; // icy blue
        }
        matrixSet((uint8_t)sx, (uint8_t)sy, c);
      }

      explosionTicks++;
      if (explosionTicks > 14) {
        // Reset to launch next rocket
        exploded = false;
        ry = H + 2;
        rx = (uint8_t)random(W);
        speed = 0.0f;
        memset(trail, 0, sizeof(trail));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    frame++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBeachWave
   Simple Strand-Welle: unten Sand, darüber blaues Wasser mit einer weißen Wellenkante,
   die langsam seitlich hin- und herwandert. Oben ein ruhiger Himmel.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBeachWave(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);

    // y=0: Himmel, y=1-2: Wasser, y=3: Sand
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint32_t c = 0;
        if (y == H - 1) {
          // Sand: warm gelb
          c = pixels.Color(220, 180, 80);
        } else if (y == 0) {
          // Himmel: dezentes Blau
          c = pixels.Color(40, 80, 160);
        } else {
          // Wasser: Grundfarbe türkis / blau
          c = pixels.Color(20, 100, 200);
        }
        matrixSet(x, y, c);
      }
    }

    // Wellenposition (crest) bewegt sich hin und her
    uint8_t phase = t;
    uint8_t tri;
    if (phase < 128) tri = (uint8_t)(phase << 1);           // 0..254
    else             tri = (uint8_t)((255 - phase) << 1);   // 254..0
    uint8_t crestX = (uint8_t)((uint16_t)tri * (W - 1) / 255); // 0..W-1

    // Weiße Schaumkrone im Wasserbereich bei crestX
    for (uint8_t y = 1; y < H - 1; y++) { // nur Wasserzeilen
      for (uint8_t x = 0; x < W; x++) {
        int16_t dx = (int16_t)x - (int16_t)crestX;
        if (dx < 0) dx = -dx;
        uint8_t dist = (uint8_t)dx;

        if (dist == 0) {
          // exakte Kante: helles Weiß
          matrixSet(x, y, pixels.Color(255, 255, 230));
        } else if (dist == 1) {
          // direkt daneben: helleres Wasser / Schaum
          matrixSet(x, y, pixels.Color(120, 200, 255));
        } else if (dist == 2) {
          // leicht aufgehelltes Wasser
          matrixSet(x, y, pixels.Color(40, 130, 220));
        }
      }
    }

    // Optional: kleines Funkeln auf der Wasseroberfläche
    if ((t & 0x03) == 0) {
      uint8_t sx = (uint8_t)random(W);
      uint8_t sy = (uint8_t)(1 + (random(H - 2))); // nur Wasserzeilen
      uint32_t prev = pixels.getPixelColor(matrixIndex(sx, sy));
      uint8_t pr = (prev >> 16) & 0xFF;
      uint8_t pg = (prev >> 8) & 0xFF;
      uint8_t pb = prev & 0xFF;
      // leichtes Aufhellen mit explizitem Clamp (ohne std::min / Arduino-min)
      uint16_t tmp;
      tmp = (uint16_t)pr + 40; if (tmp > 255) tmp = 255; pr = (uint8_t)tmp;
      tmp = (uint16_t)pg + 40; if (tmp > 255) tmp = 255; pg = (uint8_t)tmp;
      tmp = (uint16_t)pb + 40; if (tmp > 255) tmp = 255; pb = (uint8_t)tmp;
      matrixSet(sx, sy, pixels.Color(pr, pg, pb));
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBurningSauna
   Kleine brennende Sauna auf dunklem Eisfeld:
   - Hintergrund: kaltes, dunkles Blau (Eis / Nacht)
   - Sauna: warmes Holzrechteck links unten mit Schornstein
   - Feuer: flackerndes, oranges/rotes Leuchten im Inneren, plus Rauchpixel oben
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBurningSauna(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // Position der Hütte (links unten)
  const uint8_t hutX0 = 0;
  const uint8_t hutWidth = (W >= 4) ? 3 : (W - 1);
  const uint8_t hutRoofY = (H >= 4) ? (H - 4) : 0;
  const uint8_t hutBodyY = (H >= 3) ? (H - 3) : 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Dunkles, kaltes Eisfeld / Nacht-Hintergrund
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        // dezentes Vignette-artiges Blau
        uint8_t dim = (y > 0) ? (uint8_t)(40 + y * 5) : 30;
        uint8_t r = 0;
        uint8_t g = (uint8_t)(dim / 3);
        uint8_t b = (uint8_t)(dim);
        matrixSet(x, y, pixels.Color(r, g, b));
      }
    }

    // Eislinie am unteren Rand (helleres Blau)
    if (H > 0) {
      uint8_t yIce = H - 1;
      for (uint8_t x = 0; x < W; x++) {
        matrixSet(x, yIce, pixels.Color(40, 80, 180));
      }
    }

    // Holzhütte (rechteckiger Körper)
    uint32_t woodCol = pixels.Color(150, 90, 40);
    for (uint8_t y = hutBodyY; y < H - 1; y++) { // bis knapp vor Eislinie
      for (uint8_t x = hutX0; x < (uint8_t)(hutX0 + hutWidth) && x < W; x++) {
        matrixSet(x, y, woodCol);
      }
    }

    // Dach (etwas dunkleres Holz / rot-braun)
    uint32_t roofCol = pixels.Color(110, 60, 30);
    if (hutRoofY < H) {
      for (uint8_t x = hutX0; x < (uint8_t)(hutX0 + hutWidth) && x < W; x++) {
        matrixSet(x, hutRoofY, roofCol);
      }
    }

    // Schornstein auf dem Dach (kleiner dunkler Block rechts auf der Hütte)
    if (hutRoofY > 0 && (uint8_t)(hutX0 + hutWidth - 1) < W) {
      uint8_t cx = (uint8_t)(hutX0 + hutWidth - 1);
      uint8_t cy = (uint8_t)(hutRoofY - 1);
      matrixSet(cx, hutRoofY, pixels.Color(80, 50, 30));
      matrixSet(cx, cy, pixels.Color(70, 40, 25));
    }

    // Feuer im Inneren: flackernd, gelb/rot/orange
    // Wir nehmen ein bis zwei Pixel im unteren Hüttenteil und variieren Helligkeit/Farbe.
    uint8_t flameBaseY = (H >= 2) ? (H - 2) : 0;
    uint8_t fx0 = (uint8_t)(hutX0 + 1);
    if (fx0 >= W) fx0 = hutX0;
    uint8_t fx1 = (uint8_t)(fx0 + 1 < W ? fx0 + 1 : fx0);

    auto drawFlame = [&](uint8_t x, uint8_t y){
      uint8_t flicker = (uint8_t)random(120, 255); // Helligkeit
      uint8_t red   = (uint8_t)flicker;
      uint8_t green = (uint8_t)(flicker * 3 / 4);
      uint8_t blue  = (uint8_t)(flicker / 8);
      matrixSet(x, y, pixels.Color(red, green, blue));
      if (y > 0) {
        // etwas Glut nach oben auslaufen lassen
        uint8_t red2   = (uint8_t)(red * 3 / 4);
        uint8_t green2 = (uint8_t)(green * 2 / 3);
        uint8_t blue2  = (uint8_t)(blue + 5);
        matrixSet(x, (uint8_t)(y - 1), pixels.Color(red2, green2, blue2));
      }
    };

    drawFlame(fx0, flameBaseY);
    if (fx1 != fx0) drawFlame(fx1, flameBaseY);

    // Rauch aus dem Schornstein: seltene, graue Pixel, die langsam nach links/rechts wabern
    if (random(4) == 0) {
      uint8_t sx = (uint8_t)(hutX0 + hutWidth - 1);
      uint8_t sy = (hutRoofY > 0) ? (uint8_t)(hutRoofY - 2) : 0;
      if (sy < H) {
        int8_t dx = (int8_t)(random(3)) - 1; // -1,0,1
        int8_t nx = (int8_t)sx + dx;
        if (nx < 0) nx = 0;
        if (nx >= (int8_t)W) nx = (int8_t)W - 1;
        uint8_t gray = (uint8_t)random(80, 160);
        matrixSet((uint8_t)nx, sy, pixels.Color(gray, gray, gray));
      }
    }

    // Leichte Gesamt-Fade, um Wabern auf dem Eis zu erlauben
    matrixFade(5);

    pixels.show();
    delay(stepDelay_ms);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixMagentaMiniSnake
   Kleine magentafarbene Schlange, die über das dunkle Feld kriecht:
   - Hintergrund: dunkles Blau/Violett
   - Schlange: kurzer Körper (3-5 Segmente) in Magenta, Kopf etwas heller
   - Bewegung: zufälliges Winden mit Richtungswechseln, leichte Spur per matrixFade
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixMagentaMiniSnake(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // Fester Body-Puffer (max 6 Segmente)
  const uint8_t MAX_LEN = 6;
  uint8_t len = 4;
  int8_t sx[MAX_LEN];
  int8_t sy[MAX_LEN];

  // Start mittig
  int8_t hx = (int8_t)(W / 2);
  int8_t hy = (int8_t)(H / 2);
  for (uint8_t i = 0; i < len; i++) {
    sx[i] = hx;
    sy[i] = hy;
  }

  int8_t dx = 1, dy = 0; // Startbewegung nach rechts

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Leichtes Fading für die Spur
    matrixFade(40);

    // Hintergrund minimal abdunkeln, falls komplett schwarz
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint32_t c = pixels.getPixelColor(matrixIndex(x, y));
        if (c == 0) {
          matrixSet(x, y, pixels.Color(2, 0, 5));
        }
      }
    }

    // Gelegentliche Richtungswechsel, aber keine 180° Wendung
    if (random(4) == 0) {
      int8_t ndx = dx;
      int8_t ndy = dy;
      if (dx != 0) { // wir gehen horizontal, Wechsel auf vertikal
        ndx = 0;
        ndy = (random(2) == 0) ? 1 : -1;
      } else {       // wir gehen vertikal, Wechsel auf horizontal
        ndy = 0;
        ndx = (random(2) == 0) ? 1 : -1;
      }
      dx = ndx;
      dy = ndy;
    }

    // Kopfposition aktualisieren
    hx += dx;
    hy += dy;

    // Wrap-around an den Rändern
    if (hx < 0)        hx = (int8_t)(W - 1);
    else if (hx >= (int8_t)W) hx = 0;
    if (hy < 0)        hy = (int8_t)(H - 1);
    else if (hy >= (int8_t)H) hy = 0;

    // Body nachziehen: von hinten nach vorne schieben
    for (int8_t i = (int8_t)(len - 1); i > 0; i--) {
      sx[i] = sx[i - 1];
      sy[i] = sy[i - 1];
    }
    sx[0] = hx;
    sy[0] = hy;

    // Schlange zeichnen: Kopf heller, Körper dunkleres Magenta
    for (uint8_t i = 0; i < len; i++) {
      uint8_t ix = (uint8_t)sx[i];
      uint8_t iy = (uint8_t)sy[i];
      uint8_t br = (uint8_t)(255 - i * 30); // Helligkeit entlang des Körpers
      uint8_t r = br;
      uint8_t g = 0;
      uint8_t b = (uint8_t)(br * 3 / 4);
      matrixSet(ix, iy, pixels.Color(r, g, b));
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixCounter1to9
   Zeigt nacheinander die Ziffern 1 bis 9 auf komplett schwarzem Hintergrund.
   Jede Ziffer wird kurz eingeblendet, dann geht es weiter zur nächsten.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCounter1to9(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // 5x4 Schrift (W x H): wir nutzen das komplette Feld als Ziffern-Canvas.
  // Bits sind zeilenweise (y 0..3), innerhalb jeder Zeile x 0..4 (MSB links).
  static const uint8_t PROGMEM DIGITS[9][4] = {
    // 1
    { 0b00100,
      0b01100,
      0b00100,
      0b01110 },
    // 2
    { 0b01110,
      0b00010,
      0b01110,
      0b01000 },
    // 3
    { 0b01110,
      0b00010,
      0b00110,
      0b01100 },
    // 4
    { 0b01010,
      0b01010,
      0b01110,
      0b00010 },
    // 5
    { 0b01110,
      0b01000,
      0b01110,
      0b00010 },
    // 6
    { 0b00110,
      0b01000,
      0b01110,
      0b01010 },
    // 7
    { 0b01110,
      0b00010,
      0b00100,
      0b00100 },
    // 8
    { 0b01110,
      0b01010,
      0b01110,
      0b01010 },
    // 9
    { 0b01110,
      0b01010,
      0b01110,
      0b00010 },
  };

  uint8_t current = 0; // Index 0..8 -> Ziffern 1..9
  uint32_t lastSwitch = start;
  const uint16_t DIGIT_DURATION = (uint16_t)(stepDelay_ms * 8); // ca. 8 Frames pro Ziffer

  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint32_t now = millis();
    if ((uint16_t)(now - lastSwitch) >= DIGIT_DURATION) {
      lastSwitch = now;
      current = (uint8_t)((current + 1) % 9);
    }

    // Hintergrund komplett schwarz
    matrixFill(0);

    // Ziffer zeichnen und dabei 90° im Uhrzeigersinn drehen
    // Font-Koordinaten (fx, fy) -> Matrix-Koordinaten (mx, my)
    // Rotation 90° CW: mx = H-1 - fy, my = fx
    for (uint8_t fy = 0; fy < 4 && fy < H; fy++) {
      uint8_t row = pgm_read_byte(&DIGITS[current][fy]);
      for (uint8_t fx = 0; fx < 5 && fx < W; fx++) {
        bool on = (row & (1 << (4 - fx))) != 0;
        if (on) {
          uint8_t mx = (uint8_t)(H - 1 - fy);
          uint8_t my = fx;
          if (mx < W && my < H) {
            // Helles Weiß für klare Lesbarkeit
            matrixSet(mx, my, pixels.Color(255, 255, 255));
          }
        }
      }
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTricksterPlasmaLoop
   Endless version of matrixTricksterPlasma: same plasma, but no runtime limit.
   This blocks forever (until reset) and advances motion phases continuously.
   Params: stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTricksterPlasmaLoop(uint16_t stepDelay_ms) {
  // Brown→Yellow palette endpoints (same as matrixTricksterPlasma)
  const uint32_t COL_DARK   = pixels.Color(30, 15, 0);
  const uint32_t COL_BRIGHT = pixels.Color(255, 220, 60);

  auto lerpCol = [&](uint8_t a) {
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
  };

  float p1 = 0.f, p2 = 0.f, p3 = 0.f;

  for (;;) {
    p1 += 0.18f;
    p2 += 0.11f;
    p3 += 0.08f;

    for (uint8_t y = 0; y < MATRIX_H; ++y) {
      for (uint8_t x = 0; x < MATRIX_W; ++x) {
        float v = 0.0f;
        v += sin((x + p1) * 0.8f);
        v += cos((y - p2) * 1.1f);
        v += sin(((int)x + (int)y + p3) * 0.6f);
        v += 0.6f * sin((x * 0.5f - y * 0.7f) + p2 * 0.9f);

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
        uint32_t c = lerpCol(a);
        matrixSet(x, y, c);
      }
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixAntifaFlag
   A stylized, animated pixel-art depiction of the Antifaschistische Aktion flag.
   - A central red/black dual flag icon unfurls and waves gently.
   - Background is rimmed with high-contrast framing pixels.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixAntifaFlag(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint32_t start = millis();

  // Zwei Frames der Flagge (0 = Hintergrund, 1 = Rot, 2 = Schwarz, 3 = Weiß)
  static const uint8_t PROGMEM F0[20] = {
    1,1,1,1,0,
    1,3,3,1,0,
    2,3,3,2,0,
    2,2,2,2,0
  };
  static const uint8_t PROGMEM F1[20] = {
    0,1,1,1,1,
    0,1,3,3,1,
    0,2,3,3,2,
    0,2,2,2,2
  };

  auto drawFrame = [&](const uint8_t* frame) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t v = pgm_read_byte(&frame[y * W + x]);
        uint32_t c = 0;
        switch (v) {
          case 1: c = pixels.Color(220, 30, 30);   break; // Rot
          case 2: c = pixels.Color(0, 0, 0);       break; // Schwarz
          case 3: c = pixels.Color(255, 255, 255); break; // Weißes Emblem
          default: c = 0;                           break;
        }
        matrixSet(x, y, c);
      }
    }
  };

  uint16_t frame = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // leichtes Ausfaden für weichere Kanten
    matrixFade(40);

    const uint8_t* f = (frame & 0x01) ? F1 : F0;
    drawFrame(f);

    // globale Helligkeits-Modulation für „Wehen“
    uint8_t breath = (uint8_t)((frame * 5) & 0xFF);
    uint8_t tri = (breath & 0x80)
                    ? (uint8_t)(255 - ((breath & 0x7F) << 1))
                    : (uint8_t)((breath & 0x7F) << 1);
    uint8_t level = (uint8_t)(200 + tri / 8); // 200..~231

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int idx = matrixIndex(x, y);
        uint32_t c = pixels.getPixelColor(idx);
        pixels.setPixelColor(idx, dimColor(c, level));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    frame++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixChaoticPink
   Emulator version of the audio-reactive chaotic pink visual.
   Simulates audio intensity using a sine wave + noise.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixChaoticPink(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  float t = 0;
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Simulate intensity: 0..255
    // Use a combination of slow wave and fast noise to simulate music
    uint8_t wave = (uint8_t)(128 + 127 * sin(t * 0.1));
    uint8_t noise = (uint8_t)random(60);
    int val = wave + noise - 30;
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    uint8_t intensity = (uint8_t)val;

    // --- Original Logic adapted for matrix coordinates ---
    int num_updates = 2 + (intensity >> 5); // 2 to 9 pixels
  
    for(int i=0; i<num_updates; i++) {
        int pixel = random(20);
        uint8_t x = pixel % MATRIX_W;
        uint8_t y = pixel / MATRIX_W;
        
        uint8_t r = intensity; 
        uint8_t g = (intensity > 220) ? (intensity - 220) * 2 : 0; 
        uint8_t b = 60 + (random(100)); 
        
        matrixSet(x, y, pixels.Color(r, g, b));
    }
    
    int clear_chance = (intensity < 100) ? 2 : 4; 
    for(int i=0; i<5; i++) {
        if ((random(clear_chance)) == 0) {
           int p = random(20);
           uint8_t x = p % MATRIX_W;
           uint8_t y = p / MATRIX_W;
           matrixSet(x, y, 0);
        }
    }
    // --- End Original Logic ---

    pixels.show();
    delay(stepDelay_ms);
    t += 0.5;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSpringStorm
   A beautiful 5x4 spring landscape transitioning into a stormy thunderstorm.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSpringStorm(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBED5);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };
  
  // Raindrop vertical positions per column (0xFF = inactive)
  uint8_t dropY[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  
  // Track the progress threshold for the next lightning flash
  // Initial flash will happen somewhere between 110 and 130 progress
  uint8_t nextFlashTarget = (uint8_t)(110 + (nextL() % 20));

  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint16_t elapsed = (uint16_t)(millis() - start);
    
    // Normalize progress mapping (0 to 255)
    uint32_t prog32 = ((uint32_t)elapsed * 255) / runtime_ms;
    uint8_t progress = (prog32 > 255) ? 255 : (uint8_t)prog32;
    
    // Phase calculation
    // w: Storm darkness factor (0 in spring, 255 at peak storm)
    uint8_t w = (progress < 60) ? 0 : ((progress > 120) ? 255 : (uint8_t)((progress - 60) * 255 / 60));
    
    // Sky gradient blending: Light blue -> Dark grey
    uint8_t skyR = (uint8_t)(((uint16_t)100 * (255 - w) + (uint16_t)40 * w) / 255);
    uint8_t skyG = (uint8_t)(((uint16_t)200 * (255 - w) + (uint16_t)40 * w) / 255);
    uint8_t skyB = (uint8_t)(((uint16_t)255 * (255 - w) + (uint16_t)50 * w) / 255);
    uint32_t skyCol = pixels.Color(skyR, skyG, skyB);
    
    // Field / Forest coloring
    uint32_t fieldCol = pixels.Color(30, 200, 20);   // bright spring green
    uint32_t forestCol = pixels.Color(10, 100, 10);  // darker tree green
    uint32_t trunkCol = pixels.Color(100, 50, 10);   // warm brown for the trunk
    if (w > 0) { // Dim everything as the storm clouds gather
       fieldCol = dimColor(fieldCol, (uint8_t)(255 - (w / 2)));
       forestCol = dimColor(forestCol, (uint8_t)(255 - (w / 2)));
       trunkCol = dimColor(trunkCol, (uint8_t)(255 - (w / 2)));
    }
    
    // Global fade out at the end (dawn / dark)
    // From progress 160 to 220, fade all colors to black. 220 to 255 stays fully dark.
    uint8_t fadeOut = (progress > 160) ? ((progress >= 220) ? 255 : (uint8_t)((progress - 160) * 255 / 60)) : 0;
    if (fadeOut > 0) {
        uint8_t keep = 255 - fadeOut;
        skyCol = dimColor(skyCol, keep);
        fieldCol = dimColor(fieldCol, keep);
        forestCol = dimColor(forestCol, keep);
        trunkCol = dimColor(trunkCol, keep);
    }
    
    // Draw the static landscape elements
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        if (y == 3) {
           // Bottom row is the grassy field
           matrixSet(x, y, fieldCol); 
        } else if (y == 2 && (x == 0 || x == 4)) {
           // Edges are small forest patches
           matrixSet(x, y, forestCol); 
        } else if (y == 2 && x == 2) {
           // Central tree trunk
           matrixSet(x, y, trunkCol);
        } else if (y == 1 && x == 2) {
           // Central tree leaves
           matrixSet(x, y, forestCol);
        } else {
           // Rest is sky
           matrixSet(x, y, skyCol); 
        }
      }
    }
    
    // Rain layer (stops when it gets fully dark/dawn)
    if (progress > 90 && progress <= 190) { 
      uint8_t rainChanceMask = (progress > 130) ? 0x01 : 0x03; // Heavier downpour later
      for (uint8_t x = 0; x < W; x++) {
        if (dropY[x] == 0xFF) {
           if ((nextL() & rainChanceMask) == 0) {
             dropY[x] = 0; // Spawn new drop
           }
        } else {
           // Do not draw rain on the bottom row (y=3)
           if (dropY[x] < 3) {
               uint32_t dropCol = pixels.Color(150, 200, 255); // Raindrop color against sky/ground
               if (fadeOut > 0) dropCol = dimColor(dropCol, 255 - fadeOut); // fade raindrops too
               matrixSet(x, dropY[x], dropCol);
           }
           dropY[x]++;
           if (dropY[x] >= H) dropY[x] = 0xFF; // Splat/reset
        }
      }
    }
    
    // Thunderstorm layer (controlled, longer phase with 3-4 distinct flashes)
    if (progress >= nextFlashTarget && progress <= 200) { 
       matrixFill(pixels.Color(255, 255, 255)); // Bright flash
       // Calculate the next flash to happen 20-35 units of progress from now
       // This spacing ensures roughly 3 or 4 flashes before progress hits 200.
       nextFlashTarget = (uint8_t)(progress + 20 + (nextL() % 15));
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBlueBouncingBall
   A cool ("geils") blue ball bouncing around with a fading trail. 
   Flashes brightly when hitting the walls.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBlueBouncingBall(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  // 8.8 fixed-point position (start in middle)
  int16_t px = (W / 2) << 8;
  int16_t py = (H / 2) << 8;
  
  // 8.8 fixed-point velocity 
  int16_t vx = 114; // ~0.44 pixels per frame
  int16_t vy = 77;  // ~0.30 pixels per frame
  
  matrixFill(0);
  pixels.show();
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Leave a long smooth liquid trail
    matrixFade(100); 
    
    px += vx;
    py += vy;
    
    bool hitX = false, hitY = false;
    
    // Bounds checking & bouncing
    if (px <= 0) {
      px = 0; vx = -vx; hitX = true;
    } else if (px >= ((W - 1) << 8)) {
      px = ((W - 1) << 8); vx = -vx; hitX = true;
    }
    
    if (py <= 0) {
      py = 0; vy = -vy; hitY = true;
    } else if (py >= ((H - 1) << 8)) {
      py = ((H - 1) << 8); vy = -vy; hitY = true;
    }
    
    uint8_t bx = (uint8_t)(px >> 8);
    uint8_t by = (uint8_t)(py >> 8);
    
    if (hitX || hitY) {
       // Bright icy flash on wall impact
       matrixSet(bx, by, pixels.Color(200, 255, 255));
    } else {
       // Pulsing deep blue core
       uint16_t tPhase = (millis() >> 2) & 0xFF; // fast phase
       uint8_t tri = (tPhase & 0x80) ? (uint8_t)(255 - ((tPhase & 0x7F) << 1)) : (uint8_t)((tPhase & 0x7F) << 1);
       uint32_t coreColor = pixels.Color((uint8_t)(tri/4), (uint8_t)(80 + tri/2), 255);
       matrixSet(bx, by, coreColor);
    }
    
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixGreenBouncingBall
   A cool ("geils") green ball bouncing around with a fading trail. 
   Flashes brightly when hitting the walls.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixGreenBouncingBall(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  // 8.8 fixed-point position (start in middle)
  int16_t px = (W / 2) << 8;
  int16_t py = (H / 2) << 8;
  
  // 8.8 fixed-point velocity 
  int16_t vx = 114; // ~0.44 pixels per frame
  int16_t vy = 77;  // ~0.30 pixels per frame
  
  matrixFill(0);
  pixels.show();
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Leave a long smooth liquid trail
    matrixFade(100); 
    
    px += vx;
    py += vy;
    
    bool hitX = false, hitY = false;
    
    // Bounds checking & bouncing
    if (px <= 0) {
      px = 0; vx = -vx; hitX = true;
    } else if (px >= ((W - 1) << 8)) {
      px = ((W - 1) << 8); vx = -vx; hitX = true;
    }
    
    if (py <= 0) {
      py = 0; vy = -vy; hitY = true;
    } else if (py >= ((H - 1) << 8)) {
      py = ((H - 1) << 8); vy = -vy; hitY = true;
    }
    
    uint8_t bx = (uint8_t)(px >> 8);
    uint8_t by = (uint8_t)(py >> 8);
    
    if (hitX || hitY) {
       // Bright green/white flash on wall impact
       matrixSet(bx, by, pixels.Color(150, 255, 150));
    } else {
       // Pulsing deep green core
       uint16_t tPhase = (millis() >> 2) & 0xFF; // fast phase
       uint8_t tri = (tPhase & 0x80) ? (uint8_t)(255 - ((tPhase & 0x7F) << 1)) : (uint8_t)((tPhase & 0x7F) << 1);
       uint32_t coreColor = pixels.Color((uint8_t)(tri/4), 255, (uint8_t)(tri/4));
       matrixSet(bx, by, coreColor);
    }
    
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTechnoOrbit
   A red point and a blue point float around, while a green object rotates in a circle, 
   snapping to new positions "im Takt zur Musik" (in sync with a musical beat).
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTechnoOrbit(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  // 8.8 fixed-point positions for red and blue
  int16_t rx = (1) << 8, ry = (1) << 8;
  int16_t bx = (3) << 8, by = (2) << 8;
  
  // Floating velocities
  int16_t rvx = 110, rvy = 140; 
  int16_t bvx = -125, bvy = 95; 

  const uint16_t beat_ms = 430; // ~140 BPM techno tempo
  
  // 8-step circular path around a 5x4 matrix
  uint8_t ox[8] = {2, 3, 4, 3, 2, 1, 0, 1};
  uint8_t oy[8] = {0, 1, 2, 3, 3, 2, 1, 0};
  
  matrixFill(0);
  pixels.show();
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Beautiful long trails for the dancing dots
    matrixFade(110); 
    
    // Move Red
    rx += rvx; ry += rvy;
    if (rx <= 0) { rx = 0; rvx = -rvx; }
    else if (rx >= ((W - 1) << 8)) { rx = ((W - 1) << 8); rvx = -rvx; }
    if (ry <= 0) { ry = 0; rvy = -rvy; }
    else if (ry >= ((H - 1) << 8)) { ry = ((H - 1) << 8); rvy = -rvy; }

    // Move Blue
    bx += bvx; by += bvy;
    if (bx <= 0) { bx = 0; bvx = -bvx; }
    else if (bx >= ((W - 1) << 8)) { bx = ((W - 1) << 8); bvx = -bvx; }
    if (by <= 0) { by = 0; bvy = -bvy; }
    else if (by >= ((H - 1) << 8)) { by = ((H - 1) << 8); bvy = -bvy; }

    uint8_t brx = (uint8_t)(rx >> 8);
    uint8_t bry = (uint8_t)(ry >> 8);
    uint8_t bbx = (uint8_t)(bx >> 8);
    uint8_t bby = (uint8_t)(by >> 8);

    // If red and blue hit the exact same pixel, make a purple collision flash!
    if (brx == bbx && bry == bby) {
        matrixSet(brx, bry, pixels.Color(255, 0, 255));
    } else {
        matrixSet(brx, bry, pixels.Color(255, 0, 0));
        matrixSet(bbx, bby, pixels.Color(0, 80, 255));
    }
    
    // Green object orbiting "im Takt" (Pulse logic synced to beat_ms)
    uint32_t now = millis();
    uint16_t beatTime = (uint16_t)(now % beat_ms);
    uint8_t beatIndex = ((uint16_t)(now / beat_ms)) % 8;
    
    // Green flashes bright on the beat, then decays
    uint8_t bright = 255 - (uint8_t)((uint32_t)beatTime * 220 / beat_ms);
    matrixSet(ox[beatIndex], oy[beatIndex], pixels.Color(0, bright, 0));
    
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRocketLiftoff
   "Der Flammschweif einer abhebenden Mondrakete."
   Simulation of a moon rocket liftoff, focusing on the turbulent exhaust plume and vertical launch sequence.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRocketLiftoff(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  
  uint16_t lfsr = (uint16_t)(millis() ^ 0x1A4C);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };
  
  matrixFill(0);
  pixels.show();
  
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint16_t elapsed = (uint16_t)(millis() - start);
    uint8_t progress = (uint8_t)((uint32_t)elapsed * 255 / runtime_ms);
    
    // Dynamic fade: heavier fade during static thrust, slower fade after liftoff leaving persistent smoke
    uint8_t fadeAmt = (progress > 200) ? 50 : 130;  
    matrixFade(fadeAmt);
    
    if (progress < 25) {
        // Phase 1: Ignition sparks
        if ((nextL() % 3) == 0) {
            uint8_t sx = 1 + (nextL() % 3);
            matrixSet(sx, 3, pixels.Color(200, 50, 0)); 
        }
    } 
    else if (progress >= 25 && progress < 160) {
        // Phase 2: Main thrust built up, rocket clamped at top (2,0)
        matrixSet(2, 0, pixels.Color(255, 255, 200)); 
        
        uint8_t r1 = nextL();
        matrixSet(2, 1, pixels.Color(255, 150 + (r1 % 105), 0)); // Core
        
        uint8_t r2 = nextL();
        matrixSet(2, 2, pixels.Color(255, 80 + (r2 % 50), 0));   // Mid plume
        if (r2 % 2 == 0) matrixSet(1, 2, pixels.Color(200, 50, 0));
        if (r2 % 3 == 0) matrixSet(3, 2, pixels.Color(200, 50, 0));
        
        uint8_t r3 = nextL();
        matrixSet(2, 3, pixels.Color(220, 40, 0));               // Base plume
        matrixSet(1, 3, pixels.Color(180, 20 + (r3 % 30), 0));
        matrixSet(3, 3, pixels.Color(180, 20 + (r3 % 30), 0));
        
        if (r3 % 2 == 0) matrixSet(0, 3, pixels.Color(80, 0, 0));
        if (r3 % 3 == 0) matrixSet(4, 3, pixels.Color(80, 0, 0));
    }
    else if (progress >= 160 && progress < 210) {
        // Phase 3: Liftoff! Rocket ascends out of frame
        uint8_t offset = (progress - 160) / 5; // 50/5 = offsets up to 10 pixels offscreen
        int8_t y_nozzle = 0 - offset;
        int8_t y_core = 1 - offset;
        
        if (y_nozzle >= 0 && y_nozzle < H) matrixSet(2, y_nozzle, pixels.Color(255, 255, 200));
        if (y_core >= 0 && y_core < H) matrixSet(2, y_core, pixels.Color(255, 150 + (nextL() % 105), 0));
        
        // Stretching tail down to the bottom
        int8_t start_y = y_core + 1;
        if (start_y < 0) start_y = 0;
        
        for (int8_t y = start_y; y < H; y++) {
             uint8_t r = nextL();
             
             int16_t distScale = (y - y_core) * 35; // 35 heat decay per step
             int16_t intensity_calc = 255 - distScale;
             uint8_t intensity = (intensity_calc < 20) ? 20 : (uint8_t)intensity_calc;
             
             matrixSet(2, y, pixels.Color(intensity, intensity / 3 + (r % 20), 0));
             
             if (r % 2 == 0) matrixSet(1, y, pixels.Color(intensity / 2, intensity / 6, 0));
             if (r % 3 == 0) matrixSet(3, y, pixels.Color(intensity / 2, intensity / 6, 0));
             
             // Base spread
             if (y == 3) {
                 if (r % 4 == 0) matrixSet(0, y, pixels.Color(intensity / 3, 0, 0));
                 if (r % 5 == 0) matrixSet(4, y, pixels.Color(intensity / 3, 0, 0));
             }
        }
    }
    else {
        // Phase 4: Rocket is gone, faint stars appear through fading smoke
        if ((nextL() % 15) == 0) {
            uint8_t sx = nextL() % W;
            uint8_t sy = nextL() % H;
            // random bluish stars
            matrixSet(sx, sy, pixels.Color(nextL()%50, nextL()%100, 100 + (nextL()%155)));
        }
    }
    
    pixels.show();
    delay(stepDelay_ms);
  }
}