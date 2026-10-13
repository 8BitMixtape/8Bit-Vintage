#pragma once
#include <stdint.h>
#include <Adafruit_NeoPixel.h>

// Matrix geometry and wiring parameters
#ifndef MATRIX_W
#define MATRIX_W 5
#endif
#ifndef MATRIX_H
#define MATRIX_H 4
#endif
#ifndef MATRIX_SERPENTINE
#define MATRIX_SERPENTINE 0   // 1 = serpentine wiring per row, 0 = progressive
#endif
#ifndef MATRIX_ORIGIN_TOPLEFT
#define MATRIX_ORIGIN_TOPLEFT 1 // assume pixel 0 is top-left
#endif

// Externs expected from the sketch or libraries
extern Adafruit_NeoPixel pixels;

// Convert (x,y) -> strip index according to assumed wiring
static inline int matrixIndex(uint8_t x, uint8_t y) {
  if (x >= MATRIX_W || y >= MATRIX_H) return -1;
  uint8_t row = y;
  uint8_t col = x;
  if (!MATRIX_ORIGIN_TOPLEFT) {
    // If origin is bottom-left, flip Y
    row = (MATRIX_H - 1) - y;
  }
  if (MATRIX_SERPENTINE) {
    if (row % 2 == 0) {
      // even row: left -> right
      return row * MATRIX_W + col;
    } else {
      // odd row: right -> left
      return row * MATRIX_W + (MATRIX_W - 1 - col);
    }
  } else {
    // progressive left -> right each row
    return row * MATRIX_W + col;
  }
}

// Set all pixels to the same white level (0-255)
static inline void setWhiteAllPixel(uint8_t level)
{
  uint16_t count = pixels.numPixels();
  for (uint16_t n = 0; n < count; n++) {
    pixels.setPixelColor(n, level, level, level);
  }
}

// Create a dimmed version of a color by a scale factor (0-255)
static inline uint32_t dimColor(uint32_t color, uint8_t scale) {
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  r = (uint16_t)r * scale / 255;
  g = (uint16_t)g * scale / 255;
  b = (uint16_t)b * scale / 255;
  return pixels.Color(r, g, b);
}

// Set a pixel in matrix coordinates
static inline void matrixSet(uint8_t x, uint8_t y, uint32_t color) {
  int idx = matrixIndex(x, y);
  if (idx >= 0 && idx < (int)pixels.numPixels()) {
    pixels.setPixelColor(idx, color);
  }
}

// Fill entire matrix with a color
static inline void matrixFill(uint32_t color) {
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      matrixSet(x, y, color);
    }
  }
}

// Draw a simple binary bitmap (values 0/1) of size w*h onto the matrix
static inline void matrixDrawBitmap(const uint8_t* bmp, uint8_t w, uint8_t h, uint32_t colorOn, uint32_t colorOff) {
  uint8_t mw = (w < MATRIX_W) ? w : MATRIX_W;
  uint8_t mh = (h < MATRIX_H) ? h : MATRIX_H;
  for (uint8_t y = 0; y < mh; y++) {
    for (uint8_t x = 0; x < mw; x++) {
      uint8_t v = bmp[y * w + x];
      matrixSet(x, y, v ? colorOn : colorOff);
    }
  }
}

// Small demo pattern: a simple smiley on 5x4
static inline void matrixDemoSmiley() {
  // 5x4 bitmap (row-major): 1 = on, 0 = off
  const uint8_t bmp[20] = {
    // Row 0: eyes
    0,1,0,1,0,
    // Row 1
    1,0,0,0,1,
    // Row 2
    0,0,1,0,0,
    // Row 3
    1,0,0,0,1
  };
  matrixDrawBitmap(bmp, 5, 4, pixels.Color(255, 200, 0), 0);
  pixels.show();
}

// Dim all pixels by a decay amount (0..255). Higher = faster fade.
static inline void matrixFade(uint8_t decay) {
  uint8_t keep = 255 - decay;
  uint16_t n = pixels.numPixels();
  for (uint16_t i = 0; i < n; i++) {
    uint32_t c = pixels.getPixelColor(i);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    r = (uint16_t)r * keep / 255;
    g = (uint16_t)g * keep / 255;
    b = (uint16_t)b * keep / 255;
    pixels.setPixelColor(i, r, g, b);
  }
}
