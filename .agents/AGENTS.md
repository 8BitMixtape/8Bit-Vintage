# Antigravity Project Rules

* **Manual Uploads Only**: NEVER automatically run the PlatformIO upload command (`platformio run --target upload`) yourself. The user must execute this command manually to perfectly time the ATtiny power cycle for the Audio Bootloader. You may run regular compile/build tasks (`platformio run`), but you must omit the `--target upload` flag.

* **Two-Phase Sound Development Workflow (ABK-20 / 8Bit Mixtape)**:
  * **Phase 1 (Development & Hardware Testing)**:
    1. Develop and edit C++ code locally in `src/<app_id>.cpp`.
    2. Activate sketch in `platformio.ini` and compile with `pio run` to verify flash budget ($\le 6,300$ bytes) and generate `.pio/build/attiny85/firmware.wav`.
    3. User uploads and tests on physical hardware. Iterate on sound, controls, and visuals based on feedback.
    4. **DO NOT** generate GIFs, audio preview MP3s, manuals, or update `docs/apps.json` / `docs/index.html` during this phase.
  * **Phase 2 (Finalization & Publishing - ONLY ON USER COMMAND)**:
    - Only when the user explicitly commands to finalize/publish the sound:
      1. Generate the animated matrix GIF (`scripts/gen_previews.py`).
      2. Write user manual (`docs/<APP_ID_UPPER>_MANUAL.md`).
      3. Generate audio preview MP3 (`scripts/gen_audio_previews.py`).
      4. Copy production WAV to `docs/wav/<app_id>.wav`.
      5. Register in `docs/apps.json` & sync `docs/apps.js`.
      6. Commit and push to GitHub.
