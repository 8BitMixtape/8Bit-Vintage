#include "neolib.h"
#include "matrix_helpers.h"
#include <Arduino.h>
#include <avr/interrupt.h>

// Game state variables
int16_t pipeX = 5 << 8;
int16_t pipeVel = -20; 
int8_t pipeGapY = 1;   
int8_t gapSize = 2; // 1 or 2 pixels wide

uint16_t score = 0;
bool gameOver = true;
bool gameStarted = false;
uint32_t gameOverTime = 0;

// Hardware audio setup
void setupAudio() {
  pinMode(SPEAKERPIN, OUTPUT);
  TCCR1 = (1 << CTC1) | (1 << COM1A0) | (1 << CS13) | (1 << CS11);
}

void playFreq(uint16_t freq) {
  if (freq == 0) {
    TCCR1 &= ~(1 << COM1A0); 
    digitalWrite(SPEAKERPIN, LOW);
  } else {
    TCCR1 |= (1 << COM1A0);  
    uint32_t top = 15625UL / freq;
    if (top > 0) top--;
    if (top > 255) top = 255;
    OCR1C = top;
  }
}

void playFunnyStartup() {
  // Funny boop-bloop-bleep slide
  for (int i = 0; i < 10; i++) {
    pixels.setPixelColor(i, pixels.Color(200, 0, 200));
    pixels.show();
    playFreq(200 + i * 100);
    delay(40);
  }
  for (int i = 10; i < 20; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 200, 200));
    pixels.show();
    playFreq(1200 - (i - 10) * 90);
    delay(40);
  }
  // Final goofy boop
  matrixFill(pixels.Color(200, 200, 0));
  pixels.show();
  playFreq(150); delay(100);
  playFreq(350); delay(150);
  playFreq(0);
  matrixFill(0);
  pixels.show();
}

void setup() {
  neobegin();
  pixels.setBrightness(100);
  setupAudio();
  playFreq(0); // Stop any default tone immediately!
  randomSeed(analogRead(A1) + analogRead(A2)); 
  
  playFunnyStartup();
}

void resetGame() {
  pipeX = 5 << 8;
  if (gapSize == 1) pipeGapY = random(0, 4);
  else pipeGapY = random(0, 3);
  score = 0;
  gameOver = false;
  matrixFill(0);
}

void loop() {
  // Use Potentiometer A2 to control base game speed, and increase speed with score!
  uint16_t potValue = analogRead(A2); 
  int16_t speedBoost = score; // Speed up by 1 for every wall
  if (speedBoost > 20) speedBoost = 20; // Cap the speed boost
  pipeVel = -((potValue / 30) + 10 + speedBoost); 

  if (gameOver) {
    matrixFill(0);
    
    if (!gameStarted) {
      // Intro Mode Selection Screen
      // Flash Cyan to indicate ready to start
      uint32_t introColor = (millis() % 500 > 250) ? pixels.Color(0, 100, 100) : 0;
      matrixFill(introColor);
      
      // Draw 1 dot on the left, 2 dots on the right to indicate button modes
      matrixSet(1, 1, pixels.Color(150, 150, 150)); // Left button: 1 gap
      
      matrixSet(3, 1, pixels.Color(150, 150, 150)); // Right button: 2 gaps
      matrixSet(3, 2, pixels.Color(150, 150, 150));
    } else {
      // Flash the score using the NeoPixels
      uint8_t level = (score / 20) % 4; // 0=Red, 1=Green, 2=Blue, 3=Yellow
      uint16_t displayScore = score % 20;
      if (score > 0 && displayScore == 0) displayScore = 20; // Show full screen for exact multiples of 20
      
      uint32_t flashColor;
      if (millis() % 500 > 250) {
        if (level == 0) flashColor = pixels.Color(150, 0, 0);
        else if (level == 1) flashColor = pixels.Color(0, 150, 0);
        else if (level == 2) flashColor = pixels.Color(0, 0, 150);
        else flashColor = pixels.Color(150, 150, 0);
      } else {
        if (level == 0) flashColor = pixels.Color(20, 0, 0);
        else if (level == 1) flashColor = pixels.Color(0, 20, 0);
        else if (level == 2) flashColor = pixels.Color(0, 0, 20);
        else flashColor = pixels.Color(20, 20, 0);
      }
      
      for (uint16_t i = 0; i < displayScore; i++) {
        pixels.setPixelColor(i, flashColor);
      }
      
      // Auto-restart to opening screen after 5 seconds
      if (millis() - gameOverTime > 5000) {
        gameStarted = false;
        playFunnyStartup();
        return;
      }
    }
    pixels.show();
    
    uint8_t btn = wasButtonPressed();
    if (btn == BUTTON_LEFT) {
      gapSize = 1;
      gameStarted = true;
      resetGame();
    } else if (btn == BUTTON_RIGHT) {
      gapSize = 2;
      gameStarted = true;
      resetGame();
    }
    delay(30);
    return;
  }

  // Handle Input: Use Potentiometer A1 (Slider) for Bird Y position
  uint16_t sliderVal = analogRead(A1);
  // Map 0-1023 to row 0 to 3. (Matrix Y: 0 is top, 3 is bottom)
  int8_t by = map(sliderVal, 0, 1023, 0, 3); 
  
  // Make sure it doesn't go out of bounds
  if (by < 0) by = 0;
  if (by > 3) by = 3;

  // Pipe movement
  pipeX += pipeVel;
  if (pipeX < -(1 << 8)) {
    pipeX = 5 << 8;
    if (gapSize == 1) pipeGapY = random(0, 4);
    else pipeGapY = random(0, 3);
    score++;
    
    // High Score Tunes!
    if (score > 0 && score % 20 == 0) {
      // Mega fanfare for reaching 20, 40, etc.
      playFreq(523); delay(60); // C
      playFreq(659); delay(60); // E
      playFreq(784); delay(60); // G
      playFreq(1046); delay(150); // High C
      playFreq(0);
    } else if (score > 0 && score % 10 == 0) {
      // Mini fanfare for reaching 10, 30, etc.
      playFreq(440); delay(50); // A
      playFreq(554); delay(50); // C#
      playFreq(659); delay(50); // E
      playFreq(880); delay(100); // High A
      playFreq(0);
    } else {
      // Normal pipe pass sound
      uint16_t pitchOffset = score * 15;
      if (pitchOffset > 2000) pitchOffset = 2000;
      
      playFreq(440 + pitchOffset);
      delay(10);
      playFreq(880 + pitchOffset);
      delay(10);
      playFreq(0);
    }
  }

  // Collision detection
  int8_t bx = 1; 
  int8_t px = pipeX >> 8;

  if (px == bx) {
    bool inGap = false;
    if (gapSize == 1) {
      inGap = (by == pipeGapY);
    } else {
      inGap = (by == pipeGapY || by == (pipeGapY + 1));
    }
    
    if (!inGap) {
      gameOver = true;
    }
  }
  
  // Game Over Sound
  if (gameOver) {
    for (int i = 200; i > 50; i -= 10) {
      playFreq(i);
      matrixFill(pixels.Color(255, 0, 0));
      pixels.show();
      delay(15);
    }
    playFreq(0);
    gameOverTime = millis();
  }

  // Rendering
  matrixFill(pixels.Color(0, 10, 20)); 

  // Draw pipe
  uint32_t wallColor;
  if (potValue < 256) wallColor = pixels.Color(0, 150, 0); // Green
  else if (potValue < 512) wallColor = pixels.Color(0, 100, 150); // Blueish
  else if (potValue < 768) wallColor = pixels.Color(150, 0, 150); // Purple
  else wallColor = pixels.Color(150, 50, 0); // Orange

  if (px >= 0 && px <= 4) {
    for (int y = 0; y < 4; y++) {
      bool inGap = false;
      if (gapSize == 1) {
        inGap = (y == pipeGapY);
      } else {
        inGap = (y == pipeGapY || y == (pipeGapY + 1));
      }
      
      if (!inGap) {
        matrixSet(px, y, wallColor); 
      }
    }
  }

  // Draw bird (Color changes depending on gapSize mode)
  uint32_t birdColor = (gapSize == 1) ? pixels.Color(255, 200, 0) : pixels.Color(255, 0, 200); // Yellow for hard, Pink for easy
  matrixSet(bx, by, birdColor); 

  pixels.show();
  delay(30); 
}
