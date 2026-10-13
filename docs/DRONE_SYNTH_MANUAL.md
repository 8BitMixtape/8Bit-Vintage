# 8Bit Mixtape Drone Synth Manual

Welcome to the **Drone Synth**! This firmware transforms your 8Bit Mixtape into a rich, dual-oscillator drone synthesizer complete with interactive, generative plasma visuals on the LED matrix.

The audio and visuals are tightly linked, ensuring that what you hear is exactly what you see.

---

## 🎛️ Controls at a Glance

The synthesizer is controlled via a potentiometer (knob), a slider, and the two push buttons.

### The Knob and the Slider

The analog inputs use a smooth "portamento" glide, meaning the sound and visuals will smoothly morph to the new values when you interact with them, rather than jumping instantly.

**Left Potentiometer (Base Pitch & Plasma Speed)**
*   **Audio:** Controls the master base frequency (pitch) of the drone.
*   **Visuals:** Controls the speed of the flowing plasma animation. Turning the pitch up will cause the plasma to flow faster!

**The Slider (Detune)**
*   **Audio:** Controls the "Detune" of the second oscillator. Instead of being an independent pitch, this slider offsets the second oscillator relative to your base pitch. This allows you to easily dial in thick beating drones or musical intervals (like perfect fifths or octaves) that stay in tune as you change the base pitch!

---

### 🚀 The "Shift" Layer (Independent Visuals)
You can decouple the visual controls from the audio controls using the Shift Layer! 

**If you press and hold EITHER button down for half a second**, the Knob and Slider will instantly switch to controlling only the visuals, without affecting your drone pitch:
*   **The Knob (while holding a button):** Controls the "Zoom" or spatial complexity of the plasma. Turning it up creates denser, more intricate interference patterns on the LED matrix.
*   **The Slider (while holding a button):** Controls the Speed of the flowing plasma animation. Sliding it up will cause the plasma to flow faster!

*Note: The synthesizer is smart! It waits ~300ms before activating the Shift Mode. This means if you just do a quick click to switch effects, your visuals won't accidentally jump. But if you hold it down, it treats it as a "Long Press" and safely ignores the mode change when you let go!*

### The Buttons (Mode Selection)

Use the buttons to cycle through the 4 available unified "Modes". Each mode changes both the audio timbre and the visual color theme simultaneously.

*   **Left Button:** Step to the *previous* mode (Down).
*   **Right Button:** Step to the *next* mode (Up).

---

## 🎨 The 4 Linked Modes

There are no harsh sawtooth or square waves here! The synth focuses on warm, thick, and interesting timbres derived entirely from Triangle waves.

### Mode 0: The Classic Drone
*   **Audio:** **Smooth Dual Triangle.** A classic, warm, and humming drone. Great for deep, resonant bass when pitched down.
*   **Visuals:** **Yellow/Brown Plasma.** A warm, earthy visual theme.

### Mode 1: The Fuzz Drone
*   **Audio:** **Fuzzed/Overdriven Triangle.** The two oscillators are driven hard into a clipper, creating a warm, fuzzy, and rich harmonic buzz. Excellent for aggressive, roaring textures.
*   **Visuals:** **Deep Red/Orange Plasma.** A fiery and intense visual theme.

### Mode 2: The Lo-Fi Drone
*   **Audio:** **Bitcrushed Triangle.** The lower resolution bits are intentionally dropped from the signal, resulting in a gritty, lo-fi 8-bit hum. Sounds like a retro video game console struggling to breathe!
*   **Visuals:** **Burnt Orange Plasma.** A darker, moodier visual theme.

### Mode 3: The Sci-Fi Drone
*   **Audio:** **Ring Modulation.** The two triangle waves are mathematically multiplied together. This produces unpredictable, metallic, and bell-like textures. Adjusting the Detune knob in this mode radically changes the harmonic structure!
*   **Visuals:** **Crimson/Warm Red Plasma.** A vibrant, alien-like visual theme.

---

## 💡 Tips for Droning

1.  **Find the Sweet Spot:** In Mode 0 (Classic) or Mode 1 (Fuzz), turn the Pitch knob down low, and slowly move the Detune slider until you hear a rhythmic "wobble" or beating sound. This is the sound of the two oscillators interfering with each other!
2.  **Explore Ring Mod:** In Mode 3, the Detune slider acts as a "Harmonic Ratio" control. Sweep it slowly while holding a steady base pitch to hear the classic sci-fi sound effects.
3.  **Watch the Modulation:** A slow, hidden LFO (Low-Frequency Oscillator) modulates the pitch automatically. The depth of this LFO is tied to the average brightness of the plasma! As the plasma ebbs and flows, the drone will subtly warp and wobble in sync with the lights.
