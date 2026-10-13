# Antigravity Project Rules

* **Manual Uploads Only**: NEVER automatically run the PlatformIO upload command (`platformio run --target upload`) yourself. The user must execute this command manually to perfectly time the ATtiny power cycle for the Audio Bootloader. You may run regular compile/build tasks (`platformio run`), but you must omit the `--target upload` flag.
