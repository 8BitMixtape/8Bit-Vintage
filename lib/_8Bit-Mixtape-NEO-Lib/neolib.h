#ifndef _NEOLIB_H_
#define _NEOLIB_H_

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__                 // Whaaadiiissiiiittt? == its only vor AVR microcontrollers ( the ATTINY is one )
#include <avr/power.h>
#endif

#include "neocolors.h"

// hardware description / pin connections
#define NEOPIXELPIN         0
#define SPEAKERPIN          1
#define POTI_RIGHT         A1
#define POTI_LEFT          A2
#define BUTTONS_ADC        A3

#define NUMPIXELS           20

#define BUTTON_NONE         0
#define BUTTON_LEFT         1
#define BUTTON_RIGHT        2


// hardware calibration

#define Vbutton_releaseLevel  490 // Unpressed is ~512 (2.5V)
#define Vbutton_left          470 // Higher voltage button is ~430 (2.1V)
#define Vbutton_right         400 // Lower voltage button is ~368 (1.8V)
#define Vbutton_both          330 // Both is estimated around ~300 (1.5V)
#define Vbutton_pressedLevel  Vbutton_left 

#define Vcc                    37 // 3.7 V for LiPo
#define Vdiv                   26 // measure max Voltage on Analog In

extern Adafruit_NeoPixel pixels;

void     neobegin();
uint8_t  getButton();
uint8_t  wasButtonPressed();

uint16_t analogReadScaled(uint8_t channel);
uint16_t getPoti(uint8_t channel);

void     displayBinaryValue(uint16_t value, uint32_t color);
void     setColorAllPixel(uint32_t color);
void     rainbowCycle(uint8_t wait, uint8_t rounds, uint8_t rainbowPixels);
uint32_t Wheel(byte WheelPos); // depreciated
uint32_t colorWheel(uint8_t colorNumber); 

#endif // _NEOLIB_H_ 
