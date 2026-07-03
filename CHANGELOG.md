# Changelog

All notable changes to `adora-device` will be documented in this file.

Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)  
Versioning: [Semantic Versioning](https://semver.org/spec/v2.0.0.html)

---

## [0.2.0] - 2026-03-26

### Changed
- Firmware migration complete — all 6 blocks (A–F) delegated from Adora Mini firmware
- Block D (LED): LedController owns NeoPixel strip via BoardProfile
- Block E (Button): Multi-level press actions with preview callbacks
- Block F (Sound): SoundManager with SoundEvent enum, MuteMode, loop support

### Fixed
- Added `<stddef.h>` include in SoundManager.h for `size_t` portability
- Fixed AdoraDevice.cpp SoundManager default constructor initialization

### Added
- README.md with quick start, architecture overview, board support matrix
- LICENSE (MIT)
- CHANGELOG.md
- Library examples: BasicDrive, ButtonLed
- Complete library.json metadata (license, authors, keywords, repository)

---

## [0.1.0] - 2026-03-15

### Added — Initial Library
- **AdoraDevice** facade — single entry point from BoardProfile
- **BoardProfile** system — declarative hardware description for all Adora boards
- **BoardProfiles** — predefined profiles: AdoraOrig, Adora26, Adora26Pro
- **McuCapabilities** — MCU enum with capability queries (BLE, I2S, RAM)
- **DeviceConfig** — runtime feature selection (board can do more than device uses)
- **DRV8833Driver** — GPIO PWM motor driver with nSLEEP control
- **I2CMotorDriver** — DRV8830 I2C motor driver
- **TankSteering** — Joystick-to-tank math (deadzone, response curves, turn compensation)
- **LedController** — NeoPixel wrapper with automatic R/G color order swap
- **Battery** — ADC monitoring, 5 battery types, auto-detection
- **Button** — Hardware debouncing, multi-level actions, preview/short-press callbacks
- **SoundManager** — ESP8266Audio PROGMEM WAV playback, loop mode, mute
- **CommManager** — Connection state machine (Idle→Discovering→Paired→Connected)
- **EspNowComm** — ESP-NOW abstraction
- **WifiComm** — WiFi management
- **PairingManager** — Pairing protocol implementation
- **TankSteering native tests** — 16 unit tests (deadzone, curves, edge cases)
