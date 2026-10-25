/* ---------------------------------------------------------------------------------------
    ABK-20 BI-DIRECTIONAL LED TEST (ULTRASONIC PWM)
    Silent, ultrasonic hardware test sketch:
      - Bi-directional KS01 button LED (SW2B) connected between PB1 and PB0
      - Switching frequency bumped to > 40 kHz (inaudible to human ears,
        and completely flattened by R8 330Ω + C3 100nF audio filter).
      - Zero audio hiss or tone through speaker / headphones!

    Hardware Controls:
      1. Rotary Pot (PB4):   HUE / COLOR BLEND
                             Full Left = Green <-> Center = Amber <-> Full Right = Red
      2. Slide Fader (PB2):   STROBE / PULSE SPEED
                             Bottom = Solid steady light (no blink)
                             Push UP = Slow heartbeat pulse -> Fast rave strobe (up to 40 Hz)
      3. Toggle Switch (SW3):
                             UP    = Force Pure GREEN
                             DOWN  = Force Pure RED
                             CENTER= Color tuned freely by Rotary Pot
      4. Push Button (SW2):   COLOR INVERT / FLASH BLAST
                             Pressing the button instantly inverts color / flips strobe phase!
 --------------------------------------------------------------------------------------- */

#include <Arduino.h>

// Pins for the bi-directional LED
#define LED_PIN_A   PB1 // Pin 6 (Audio output net)
#define LED_PIN_B   PB0 // Pin 5 (NeoPixel net)

// Analog inputs
#define BUTTON_PIN  A3  // PB3 (Pin 2): Button & Toggle resistor ladder
#define POT_PIN     A2  // PB4 (Pin 3): Rotary Potentiometer (Filter/Pitch)
#define FADER_PIN   A1  // PB2 (Pin 7): Slide Fader (Timbre/Wave)

// Fast 1-cycle direct port manipulation
static inline void setGreen() {
    PORTB = (PORTB & ~((1 << LED_PIN_A) | (1 << LED_PIN_B))) | (1 << LED_PIN_A);
}

static inline void setRed() {
    PORTB = (PORTB & ~((1 << LED_PIN_A) | (1 << LED_PIN_B))) | (1 << LED_PIN_B);
}

static inline void setOff() {
    PORTB = (PORTB & ~((1 << LED_PIN_A) | (1 << LED_PIN_B)));
}

// Ultrasonic PWM blend (> 45 kHz) to eliminate any audible tone on PB1 (speaker net)
void renderBlend(uint8_t blend_val) {
    if (blend_val <= 6) {
        setGreen();
        delayMicroseconds(250);
        return;
    } 
    if (blend_val >= 249) {
        setRed();
        delayMicroseconds(250);
        return;
    }

    // Map 0..255 to microsecond slice (0..16 us)
    // Total period = ~20 us (50 kHz frequency - well above 20 kHz human hearing limit!)
    uint8_t g_us = (uint16_t)(255 - blend_val) * 18 >> 8;
    uint8_t r_us = 18 - g_us;

    // Run 10 ultrasonic cycles (~200 us total)
    for (uint8_t i = 0; i < 10; i++) {
        if (g_us > 0) {
            setGreen();
            delayMicroseconds(g_us);
        }
        if (r_us > 0) {
            setRed();
            delayMicroseconds(r_us);
        }
    }
}

void setup() {
    // Configure PB1 and PB0 as outputs
    DDRB |= (1 << LED_PIN_A) | (1 << LED_PIN_B);
    setOff();
}

void loop() {
    // 1. Read Button & Toggle Ladder on PB3
    int btn_adc = analogRead(BUTTON_PIN);

    bool btn_pressed = (btn_adc < 235);
    bool toggle_up   = (btn_adc >= 320 && btn_adc <= 420);
    bool toggle_down = (btn_adc >= 240 && btn_adc < 320);

    // 2. Read Rotary Pot on PB4 (A2) -> Color / Hue
    int pot_raw = analogRead(POT_PIN);
    int pot_val = map(pot_raw, 15, 670, 255, 0); // Inverted sweep
    pot_val = constrain(pot_val, 0, 255);

    // 3. Read Slide Fader on PB2 (A1) -> Strobe Speed
    int fader_raw = analogRead(FADER_PIN);
    int fader_val = map(fader_raw, 15, 670, 0, 255); // Direct sweep
    fader_val = constrain(fader_val, 0, 255);

    // Determine target color blend (0 = Green, 255 = Red)
    uint8_t target_blend = pot_val;
    if (toggle_up) {
        target_blend = 0;   // Force Green
    } else if (toggle_down) {
        target_blend = 255; // Force Red
    }

    // Button press inverts the color instantly
    if (btn_pressed) {
        target_blend = 255 - target_blend;
    }

    // Strobe Timing from Slide Fader:
    // - Bottom (fader < 25): Continuous ON (solid glow, zero blinking)
    // - Fader 25..255: Period drops from 600ms (slow pulse) down to 25ms (40Hz laser rave strobe)
    if (fader_val < 25) {
        // Continuous solid light
        renderBlend(target_blend);
    } else {
        // Strobe mode
        uint16_t period = map(fader_val, 25, 255, 600, 25);
        uint32_t now = millis();
        bool is_lit = ((now % period) < (period / 2));

        if (btn_pressed) {
            is_lit = !is_lit;
        }

        if (is_lit) {
            renderBlend(target_blend);
        } else {
            setOff();
            delayMicroseconds(250);
        }
    }
}
