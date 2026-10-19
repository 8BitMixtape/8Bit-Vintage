# J.S. BACH: AIR ON THE G STRING & CLASSICAL BAROQUE SUITE

A 3-voice polyphonic classical synthesizer engine for the **8Bit Mixtape** (ATtiny85), bringing baroque counterpoint, walking bass, harpsichord arpeggiation, and expressive violin cantabile to 8-bit hardware.

---

## 🎻 Classical Sound Architecture

Pushing the ATtiny85 to its expressive limits, this sketch implements authentic multi-part classical counterpoint:

1. **3 Independent Polyphonic Voices**:
   - **Voice 1 (Basso Continuo / Cello)**: Deep, resonant walking bass line with octave drops and warm sub-octave acoustic body.
   - **Voice 2 (Baroque Continuo / Inner Harmony)**: Plucked harpsichord plectrum attacks, soft organ flutes, and suspended harmonic chords.
   - **Voice 3 (Solo Cantabile)**: Soaring lyrical soloist featuring **delayed classical vibrato** — notes begin with pristine pitch stability and bloom with expressive vibrato as they sustain, replicating classical violin and vocal artistry.
2. **High-Speed PWM Audio**:
   - Timer1 64 MHz PLL high-speed PWM carrier on PB1 (`OC1A`) running at ~250 kHz, well beyond human hearing, filtered to smooth analog warmth.
3. **Real-time 10 kHz Hardware Sequencer**:
   - Interrupt-driven sequencer on Timer0 keeping rock-solid musical tempo while driving dynamic ADSR envelopes.

---

## 🎼 Repertoire: 4 Classical Movements

Cycle through 4 iconic classical movements using the left button:

1. **Movement 0: J.S. Bach — *Air on the G String* (BWV 1068)**:
   - The immortal Orchestral Suite No. 3 movement with steady walking bass, soaring suspended violin melody, and lush 7-6/4-3 suspensions.
2. **Movement 1: Johann Pachelbel — *Canon in D Major***:
   - The universal 8-chord ground bass with stately hymn-like opening and flowering melodic counterpoint variations.
3. **Movement 2: Antonio Vivaldi — *Winter* (*L'Inverno*, RV 297 - Largo)**:
   - Gentle pizzicato violin raindrops in the accompaniment with a warm, romantic singing fireside melody.
4. **Movement 3: Ludwig van Beethoven — *Moonlight Sonata* (Op. 27 No. 2 - Adagio sostenuto)**:
   - Hypnotic rolling triplet arpeggios, deep bass pedal octaves, and the haunting, melancholic motif.

---

## 🎛️ Hardware Interface Controls

### 🎚️ Potentiometers
* **Left Pot (`POTI_LEFT` / PB4 / A2) — Tempo & Agogic Timing**:
  - Smoothly adjusts step duration from a solemn, majestic *Largo* (220 ms/step) to a lively *Allegro* (70 ms/step).
* **Right Pot (`POTI_RIGHT` / PB2 / A1) — Reverb Tail & Continuo Depth**:
  - Adjusts acoustic room resonance, harpsichord decay tail, and sub-bass depth from intimate chamber dry to grand cathedral hall.

---

### 🔘 Resistor Ladder Buttons (PB3 / A3)
* **Left Button (Tap)**:
  - **Cycles Classical Movements**: Bach Air → Pachelbel Canon → Vivaldi Winter → Beethoven Moonlight.
  - **Random Visual Mode Switch**: Every time the song changes, the visualizer automatically and randomly selects a completely new visual mode and color variation from the 8 modes below!
* **Right Button (Tap)**:
  - **Cycles 4 Instrument Timbres**:
    1. **Baroque Chamber**: Singing violin solo, plucked harpsichord continuo, and warm cello bass.
    2. **Cathedral Pipe Organ**: Principal 8' + 4' dual stops with deep 16' sub-bass pedal coupler.
    3. **Glass Harmonica**: Pure crystalline flute with ring-modulated celestial chime shimmer.
    4. **"Switched-On" Moog**: Tribute to Wendy Carlos' 1968 *Switched-On Bach* with PWM brass and analog saw.
* **Both Buttons (Hold/Tap)**:
  - **Manually Randomizes Visual Mode** across all 8 generative modes:
    1. **Golden Baroque Candelabra**: Warm gold and living amber with dancing candlelight leaping to melody dynamics.
    2. **Cathedral Stained Glass**: Radiant sapphire blue, ruby red, emerald green, and imperial amethyst rose window.
    3. **Polyphonic Counterpoint Harp**: Real-time independent vertical spectrum ribbons for Bass, Arpeggio, and Melody.
    4. **Moonlight on Lake Lucerne**: Deep midnight azure water with full silver moon and rippling water reflections.
    5. **Royal Velvet Tapestry**: Royal crimson/burgundy velvet with ornate golden fleur-de-lis pulsating to continuo.
    6. **Celestial Starlight & Supernova**: Twinkling deep space stars with brilliant supernova bursts igniting on melody notes.
    7. **Baroque Kaleidoscope**: Concentric sacred geometry expanding and contracting in waves of gold, turquoise, and violet.
    8. **Prismatic Aurora Cantabile**: Smooth flowing chromatic rainbow aurora ripples reacting directly to delayed vibrato.

---

## 🚀 Building & Uploading

1. Build firmware and generate AudioBoot WAV:
   ```bash
   pio run
   ```
2. Power-cycle the ATtiny into the AudioBoot bootloader, then manually upload:
   ```bash
   pio run --target upload
   ```
   *(Or play `docs/wav/bach_air_g_string.wav` into the audio jack).*

