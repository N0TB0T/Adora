# adora-device — Arduino Library Design Document

> **Status:** Design-Phase — Architektur-Entwurf v2 (2026-03-25)  
> **Vorgänger:** adora-vehicle → umbenannt zu `adora-device` (2026-03-26)  
> **Ziel:** Plattformübergreifende Arduino-Library für alle Adora- und NotBotBot-Geräte  
> **Sprache:** Englisch (Code/API) / Deutsch (Erklärungen)

---

## 1. Motivation & Scope

### Was existiert heute

| Komponente | Rolle | Problem |
|---|---|---|
| `adora-basic` | Sender-Library (Mote → Vehicle) | Nur Fernsteuer-Seite; vom Fahrzeug-Firmware gar nicht benutzt |
| Mini-Firmware | Monolithische `.cpp`-Dateien | Alle Pins hardcoded (AdoraOrig only), kein Board-Profil |
| Mote-Firmware | ESP32-C6 Controller | Komplett eigene Config, kein geteiltes Profilsystem |
| NotBotBot-Firmware | `hardware_profile.h` mit PinProfile | Gutes Pattern, aber isoliert in eigenem Repo |

### Was `adora-device` löst

**Eine Library** die alle Adora-Geräte und NotBotBot-Varianten beschreiben und betreiben kann:

```
adora-basic   = "Steuere ein Gerät per Funk"      (Mote/App → Sender)
adora-device  = "Sei ein Gerät"                    (Empfänger/Aktor)
```

### Design-Prinzipien

1. **Vier-Ebenen-Modell:** MCU → Carrier → Custom Board → Device. Jede Ebene impliziert die darunterliegende.
2. **Board-Capability ≠ Device-Feature:** Ein Board *kann* mehr als ein Device *nutzt*.
3. **Ein Binary, Board-Flag:** Compile-time Switch via `-DADORA_BOARD=...`.
4. **"Einiges kann, nichts muss."** Jedes Modul funktioniert unabhängig. Facade optional.
5. **Arduino-Niveau:** So einfach wie `Servo.h` oder `Adafruit_NeoPixel.h`.

### Scope

| ✅ In der Library | ❌ Nicht in der Library |
|---|---|
| MCU/Board/Device-Profile | Main-Loop-Orchestrierung |
| Motor-Treiber (DRV8833 / DRV8830) | Display-GUI (LVGL) |
| NeoPixel-LED-Steuerung | EEPROM-Layout (firmware-spezifisch) |
| Sound-Manager (WAV/PROGMEM) | App-Logik (Screens, Menüs) |
| Joystick→Tank-Steering-Mathematik | |
| Batterie-Monitoring | |
| Button-Handling (Debounce) | |
| Servo-Steuerung | |
| Distanz-Sensor VL53L0X | |
| ESP-NOW (plattformabstrahiert) | |
| WiFi-Management | |
| Pairing-Protokoll | |
| BLE-Abstraktion (#ifdef) | |
| OTA-Update | |

---

## 2. Vier-Ebenen-Modell

```
┌─────────────────────────────────────────────────────────────────────┐
│  MCU (Chip)                                                         │
│  ESP8266, ESP32, ESP32-C6, ESP32-S3                                 │
│  → BLE, 5GHz, Pin-Anzahl, RAM, Cores, I2S-Flexibilität             │
├─────────────────────────────────────────────────────────────────────┤
│  Carrier Board (Fertigmodul)                                        │
│  Wemos D1 Mini, Wemos D1 Mini ESP32, Waveshare ESP32-C6 Touch LCD  │
│  → Welche Pins rausgeführt, USB, Formfaktor                        │
│  → Impliziert MCU                                                   │
├─────────────────────────────────────────────────────────────────────┤
│  Custom Board (eigenes PCB / Verdrahtung)                           │
│  AdoraOrig, Adora26, Adora26Pro, AdoraMote, NotBotBot-S3           │
│  → Peripherie-Verdrahtung: Motor-Treiber, LEDs, Sensoren           │
│  → Impliziert Carrier → MCU                                        │
├─────────────────────────────────────────────────────────────────────┤
│  Device (fertiges Produkt)                                          │
│  Adora Mini, Adora Monsta, Adora Kipper, NotBotBot                 │
│  → Welches Board verbaut, welche Features aktiv                    │
│  → Board kann mehr als Device nutzt                                 │
└─────────────────────────────────────────────────────────────────────┘
```

### Warum vier Ebenen?

**MCU ≠ Carrier Board.** Auf einem Wemos D1 Mini sitzt ein ESP8266 — aber "D1 Mini" beschreibt auch die Pin-Header-Belegung (D0-D8, A0). Ein "D1 Mini ESP32" hat denselben Formfaktor, aber einen ESP32 drin — mehr RAM, BLE, 5GHz. Die MCU-Capabilities bestimmen, ob ein BLE-Stack überhaupt compiliert werden kann.

**Carrier ≠ Custom Board.** Auf einem Wemos D1 Mini sitzt bei AdoraOrig ein DRV8833, bei Adora26Pro ein DRV8830. Selbes Carrier Board, unterschiedliche Verdrahtung.

**Board ≠ Device.** Auf einem AdoraOrig Board kann ein "Adora Mini" oder ein "Adora Monsta" sitzen — mechanisch anders, selbes Board, selbe Pins. Ein Adora26Pro *kann* Sound und Servos, aber nicht jedes Device muss beides nutzen.

### Runtime-Konsequenz

Im Code braucht das Carrier Board **keine eigene Struct** — es ist implizit im Custom Board. Aber die **MCU-Capabilities** müssen abfragbar sein, weil sie bestimmen:
- Ob BLE überhaupt verfügbar ist (ESP32* ja, ESP8266 nein)
- Ob I2S-Pins konfigurierbar sind (ESP32 ja, ESP8266 nein — fixed GPIO3/15/2)
- Welche ESP-NOW API benutzt wird (ESP8266 vs ESP32 APIs sind unterschiedlich)
- Wieviel RAM für Sound-Buffer da ist

---

## 3. Bekannte Instanzen

### MCUs

| MCU | WiFi | BLE | 5GHz | Cores | RAM | I2S Pins | Aktuell verwendet in |
|---|---|---|---|---|---|---|---|
| ESP8266 | 2.4GHz | ❌ | ❌ | 1 | 80KB | Fixed (GPIO3/15/2) | AdoraOrig, Adora26, Adora26Pro |
| ESP32 | 2.4GHz | ✅ BLE 4.2 | ❌ | 2 | 520KB | Konfigurierbar | (Legacy NotBotBot Wemos D1 R32) |
| ESP32-C6 | 2.4GHz | ✅ BLE 5 | ✅ WiFi 6 | 1 RISC-V | 512KB | Konfigurierbar | AdoraMote |
| ESP32-S3 | 2.4GHz | ✅ BLE 5 | ❌ | 2 | 512KB | Konfigurierbar | NotBotBot-S3 |

### Carrier Boards

| Carrier | MCU | Formfaktor | Pins rausgeführt | Verwendet von |
|---|---|---|---|---|
| Wemos D1 Mini | ESP8266 | Mini | D0-D8, A0, RX/TX | AdoraOrig, Adora26, Adora26Pro |
| Wemos D1 Mini ESP32 | ESP32 | Mini | ähnlich D-Nummern | (perspektivisch) |
| Waveshare ESP32-C6 Touch LCD | ESP32-C6 | Display-Modul | GPIO 7-21 + SPI | AdoraMote |
| Wemos D1 R32 | ESP32 | UNO-Formfaktor | Arduino-Header | Legacy NotBotBot |
| ESP32-S3 DevKit | ESP32-S3 | Standard | GPIO 1-21 | NotBotBot-S3 |

### Custom Boards

Detaillierte Pinbelegung: siehe [BOARD_SPEC.md](BOARD_SPEC.md)

| Board | Carrier | Motor-Treiber | LEDs | Sound | Servo | Status |
|---|---|---|---|---|---|---|
| AdoraOrig | D1 Mini (ESP8266) | DRV8833 (4-Pin GPIO) | 1× WS2812 | ❌ | ❌ | ✅ Produktion |
| Adora26 | D1 Mini (ESP8266) | DRV8833 (4-Pin GPIO) | 3× WS2812 | ❌ | ❌ | 🔧 Entwicklung |
| Adora26Pro | D1 Mini (ESP8266) | DRV8830 (Dual I2C) | 3× WS2812 | ✅ I2S NoDac | 2× PWM | ✅ Produktionsbereit |
| AdoraMote | Waveshare ESP32-C6 | — | — | — | — | ✅ Produktion |
| NotBotBot-S3 | ESP32-S3 DevKit | — (Servos) | 8× WS2812 | — | 2× Servo | ✅ Produktion |
| NotBotBot-R32 | Wemos D1 R32 | — (Servos) | 2× WS2812 | — | 2× Servo | 🗄️ Legacy |

### Devices

| Device | Board | Protocol Device-ID | Anmerkung |
|---|---|---|---|
| Adora Mini | AdoraOrig | `0x02` | Standard-Kettenfahrzeug |
| Adora Mini Monsta | AdoraOrig | `0x04` | Monsta-Reifen, selbes Board |
| Adora Mini (2026) | Adora26 | TBD | Nachfolger |
| Adora Mini Pro | Adora26Pro | TBD | Mit Servo-Ausgängen |
| Adora Kipper | TBD | TBD | Platzhalter |
| Adora Bagger | TBD | TBD | Platzhalter |
| NotBotBot | NotBotBot-S3 | — | Stationärer Roboter |

---

## 4. Architektur

```
adora-device/
├── src/
│   ├── AdoraDevice.h           ← Facade (optionaler Einstiegspunkt)
│   ├── AdoraDevice.cpp
│   ├── AdoraDeviceTypes.h      ← Alle Typen, Enums, Structs
│   │
│   ├── profile/
│   │   ├── McuCapabilities.h   ← MCU-Ebene: ESP8266/ESP32/C6/S3 Capabilities
│   │   ├── BoardProfile.h      ← Custom Board: Pin-Mapping, Features
│   │   ├── BoardProfiles.h     ← Vordefinierte Boards (AdoraOrig, Adora26Pro, ...)
│   │   └── DeviceConfig.h      ← Device: Board + aktivierte Features
│   │
│   ├── motor/
│   │   ├── MotorDriver.h       ← Interface (abstract)
│   │   ├── DRV8833Driver.h     ← H-Bridge (AdoraOrig, Adora26)
│   │   ├── DRV8833Driver.cpp
│   │   ├── I2CMotorDriver.h    ← DRV8830 über I2C (Adora26Pro)
│   │   ├── I2CMotorDriver.cpp
│   │   ├── TankSteering.h      ← Joystick-Mix-Mathematik (plattformunabhängig)
│   │   └── TankSteering.cpp
│   │
│   ├── led/
│   │   ├── LedController.h     ← NeoPixel-Abstraktion
│   │   └── LedController.cpp
│   │
│   ├── sound/
│   │   ├── SoundManager.h      ← WAV-Playback (#ifdef ADORA_HAS_SOUND)
│   │   └── SoundManager.cpp
│   │
│   ├── sensor/
│   │   ├── Battery.h           ← ADC-Messung mit Kalibrierung
│   │   ├── Battery.cpp
│   │   ├── Button.h            ← Debounced Button mit Callbacks
│   │   ├── Button.cpp
│   │   ├── DistanceSensor.h    ← VL53L0X ToF (#ifdef ADORA_HAS_DISTANCE)
│   │   └── DistanceSensor.cpp
│   │
│   └── comm/
│       ├── CommManager.h       ← Plattformabstrahierte Kommunikation
│       ├── EspNowComm.h        ← ESP-NOW (ESP8266 + ESP32 APIs)
│       ├── EspNowComm.cpp
│       ├── WifiComm.h          ← WiFi-Management + OTA
│       ├── WifiComm.cpp
│       ├── PairingManager.h    ← Pairing-Protokoll
│       └── PairingManager.cpp
│
├── examples/
│   ├── 01_BasicDrive/
│   ├── 02_LedEffects/
│   ├── 03_FullVehicle/
│   ├── 04_SteeringTuning/
│   ├── 05_SoundPlayback/
│   ├── 06_DistanceLed/
│   └── 07_EspNowReceiver/
│
├── BOARD_SPEC.md
├── I2S_PIN_CONFLICTS.md
├── library.properties
├── keywords.txt
├── README.md
├── CHANGELOG.md
└── LICENSE
```

### Abhängigkeiten

| Modul | Externe Abhängigkeit | Bedingt? |
|---|---|---|
| profile/ | `<Arduino.h>` | Nein (immer) |
| Motor (DRV8833) | `<Arduino.h>` | Nein |
| Motor (DRV8830) | `<Wire.h>` | Nein |
| TankSteering | Keine (pure math) | — |
| LED | `Adafruit_NeoPixel` | Nein |
| Sound | `ESP8266Audio` | `#ifdef ADORA_HAS_SOUND` |
| Distanz | `VL53L0X` (Pololu) | `#ifdef ADORA_HAS_DISTANCE` |
| Battery | `<Arduino.h>` (analogRead) | Nein |
| ESP-NOW | `ESP8266WiFi` / `WiFi` + `esp_now` | Plattform-Switch |
| WiFi/OTA | `ESP8266WiFi`/`WiFi` + `ArduinoOTA` | Plattform-Switch |

---

## 5. API Reference

### 5.1 `McuCapabilities.h` — Was kann der Chip?

```cpp
#pragma once
#include <stdint.h>

namespace adora {

enum class Mcu : uint8_t {
  Esp8266  = 0,  // Xtensa LX106, 80/160MHz, 1 Core
  Esp32    = 1,  // Xtensa LX6, 240MHz, 2 Cores
  Esp32C6  = 2,  // RISC-V, 160MHz, 1 Core, WiFi 6
  Esp32S3  = 3   // Xtensa LX7, 240MHz, 2 Cores, USB OTG
};

struct McuCapabilities {
  Mcu mcu;
  bool hasBLE;
  bool has5GHz;
  bool hasWiFi6;
  uint8_t cores;
  uint32_t ramKB;
  uint8_t gpioCount;
  bool i2sPinsConfigurable;   // ESP32*: ja, ESP8266: nein (fixed GPIO3/15/2)
};

// Bekannte MCU-Profile
inline constexpr McuCapabilities mcuESP8266() {
  return { Mcu::Esp8266, false, false, false, 1, 80, 11, false };
}
inline constexpr McuCapabilities mcuESP32() {
  return { Mcu::Esp32, true, false, false, 2, 520, 34, true };
}
inline constexpr McuCapabilities mcuESP32C6() {
  return { Mcu::Esp32C6, true, true, true, 1, 512, 22, true };
}
inline constexpr McuCapabilities mcuESP32S3() {
  return { Mcu::Esp32S3, true, false, false, 2, 512, 45, true };
}

// Compile-time: aktive MCU automatisch erkennen
inline constexpr McuCapabilities activeMcu() {
#if defined(ESP8266)
  return mcuESP8266();
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  return mcuESP32C6();
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  return mcuESP32S3();
#elif defined(ESP32)
  return mcuESP32();
#else
  #error "Unbekannte MCU-Plattform"
#endif
}

}  // namespace adora
```

### 5.2 `BoardProfile.h` — Custom Board beschreibt seine Verdrahtung

```cpp
#pragma once
#include "McuCapabilities.h"

namespace adora {

/// Vollständige Hardware-Beschreibung eines Custom Boards.
/// Beschreibt was das Board KANN — nicht was das Device NUTZT.
struct BoardProfile {
  const char* name;           // z.B. "AdoraOrig", "Adora26Pro"
  Mcu mcu;                    // impliziert Carrier → MCU Capabilities

  // — Motoren (GPIO) —
  struct {
    uint8_t pinA1, pinA2;     // Motor A (links)   — 0 wenn nicht vorhanden
    uint8_t pinB1, pinB2;     // Motor B (rechts)  — 0 wenn nicht vorhanden
    uint8_t pinSleep;         // DRV8833 nSLEEP    — 0 wenn nicht vorhanden
  } gpioMotor;

  // — Motoren (I2C) —
  struct {
    bool available;           // true → DRV8830 vorhanden
    uint8_t addrA;            // Motor A I2C-Adresse (0 wenn nicht vorhanden)
    uint8_t addrB;            // Motor B I2C-Adresse (0 wenn nicht vorhanden)
  } i2cMotor;

  // — LEDs —
  struct {
    uint8_t pinData;          // WS2812 Data Pin
    uint8_t onboardCount;     // Anzahl onboard Pixel
  } led;

  // — Sound —
  struct {
    bool available;           // true → Audio-Hardware vorhanden
    // ESP8266: I2S NoDac, Pins hardware-fixed (GPIO3=Data, GPIO15=BCK, GPIO2=WS)
    // ESP32*: Pins konfigurierbar (in Zukunft hier ergänzen)
  } sound;

  // — Servo —
  struct {
    uint8_t pin1;             // 0 = nicht vorhanden
    uint8_t pin2;             // 0 = nicht vorhanden
  } servo;

  // — Button —
  struct {
    uint8_t pin;
    bool activeLow;           // true → LOW = gedrückt (mit Pull-up)
  } button;

  // — Batterie —
  struct {
    uint8_t pinAdc;           // A0 typisch
    float voltageDividerRatio;  // 1.0 wenn unbekannt
  } battery;

  // — I2C Bus —
  struct {
    uint8_t pinScl;
    uint8_t pinSda;
  } i2c;

  // — Distanz-Sensor (VL53L0X) —
  struct {
    bool connectorAvailable;  // true → 4-pol I2C-Stecker vorhanden
    uint8_t pinXshut;         // 0 = kein XSHUT-Anschluss
  } distance;

  // — Hilfsfunktionen —
  constexpr bool hasGpioMotor() const { return gpioMotor.pinA1 != 0; }
  constexpr bool hasI2CMotor() const { return i2cMotor.available; }
  constexpr bool hasSound() const { return sound.available; }
  constexpr bool hasServo() const { return servo.pin1 != 0; }
  constexpr bool hasDistance() const { return distance.connectorAvailable; }
  constexpr McuCapabilities mcuCaps() const { return mcuForChip(mcu); }

 private:
  static constexpr McuCapabilities mcuForChip(Mcu m) {
    switch (m) {
      case Mcu::Esp8266: return mcuESP8266();
      case Mcu::Esp32:   return mcuESP32();
      case Mcu::Esp32C6: return mcuESP32C6();
      case Mcu::Esp32S3: return mcuESP32S3();
      default:            return mcuESP8266();
    }
  }
};

}  // namespace adora
```

### 5.3 `BoardProfiles.h` — Vordefinierte Custom Boards

```cpp
#pragma once
#include "BoardProfile.h"

namespace adora {

// ─── Adora Boards (ESP8266 / Wemos D1 Mini) ──────────────────────

inline constexpr BoardProfile boardAdoraOrig() {
  return {
    .name = "AdoraOrig",
    .mcu = Mcu::Esp8266,
    .gpioMotor = { D1, D2, D6, D5, D0 },   // A1, A2, B1, B2, Sleep
    .i2cMotor  = { false, 0, 0 },
    .led       = { D7, 1 },                 // 1× WS2812 (Center)
    .sound     = { false },
    .servo     = { 0, 0 },
    .button    = { D8, false },             // D8, Pull-up extern
    .battery   = { A0, 1.0f },
    .i2c       = { D4, D3 },               // SCL=D4, SDA=D3
    .distance  = { true, 0 },              // 4-pol Stecker, kein XSHUT
  };
}

inline constexpr BoardProfile boardAdora26() {
  return {
    .name = "Adora26",
    .mcu = Mcu::Esp8266,
    .gpioMotor = { D1, D2, D6, D5, D0 },
    .i2cMotor  = { false, 0, 0 },
    .led       = { D7, 3 },                 // 3× WS2812 (Center, L, R)
    .sound     = { false },                  // kein Sound auf Adora26
    .servo     = { 0, 0 },
    .button    = { D8, false },
    .battery   = { A0, 1.0f },
    .i2c       = { D4, D3 },
    .distance  = { true, 0 },
  };
}

inline constexpr BoardProfile boardAdora26Pro() {
  return {
    .name = "Adora26Pro",
    .mcu = Mcu::Esp8266,
    .gpioMotor = { 0, 0, 0, 0, 0 },         // keine GPIO-Motoren
    .i2cMotor  = { true, 0x60, 0x62 },       // DRV8830 Dual — Adressen TODO: verify
    .led       = { D7, 3 },
    .sound     = { true },                    // I2S NoDac (GPIO3=Data, fixed)
    .servo     = { D5, D6 },                  // 2× PWM Servo
    .button    = { D3, true },                // D3=GPIO0, active LOW
    .battery   = { A0, 1.0f },
    .i2c       = { D1, D2 },                  // SCL=D1, SDA=D2
    .distance  = { true, D0 },                // 4-pol Stecker + XSHUT an D0
  };
}

// ─── NotBotBot Boards (ESP32-S3 / ESP32) ──────────────────────

inline constexpr BoardProfile boardNotBotBotS3() {
  return {
    .name = "NotBotBot-S3",
    .mcu = Mcu::Esp32S3,
    .gpioMotor = { 0, 0, 0, 0, 0 },
    .i2cMotor  = { false, 0, 0 },
    .led       = { 7, 8 },                   // GPIO7, 8 Pixels
    .sound     = { false },
    .servo     = { 5, 6 },                    // GPIO5, GPIO6
    .button    = { 8, true },
    .battery   = { 0, 0.0f },                 // kein Battery ADC
    .i2c       = { 0, 0 },                    // kein I2C-Bus definiert
    .distance  = { false, 0 },                // Ultraschall statt ToF (eigenes Modul)
  };
}

// ─── Compile-time Board-Auswahl ──────────────────────

// Gesetzt via platformio.ini: -DADORA_BOARD=ADORA_BOARD_ORIG
#define ADORA_BOARD_ORIG      0
#define ADORA_BOARD_26        1
#define ADORA_BOARD_26PRO     2
#define ADORA_BOARD_NBB_S3    10

#ifndef ADORA_BOARD
  #define ADORA_BOARD ADORA_BOARD_ORIG   // Default: AdoraOrig
#endif

inline constexpr BoardProfile activeBoard() {
#if ADORA_BOARD == ADORA_BOARD_ORIG
  return boardAdoraOrig();
#elif ADORA_BOARD == ADORA_BOARD_26
  return boardAdora26();
#elif ADORA_BOARD == ADORA_BOARD_26PRO
  return boardAdora26Pro();
#elif ADORA_BOARD == ADORA_BOARD_NBB_S3
  return boardNotBotBotS3();
#else
  #error "Unbekanntes ADORA_BOARD"
#endif
}

}  // namespace adora
```

### 5.4 `DeviceConfig.h` — Device aktiviert Subset der Board-Capabilities

```cpp
#pragma once
#include "BoardProfiles.h"

namespace adora {

/// Device-Konfiguration: Welches Board, welche Features aktiv.
/// Board KANN mehr als Device NUTZT.
///
/// Beispiel: Adora26Pro hat Sound + 2 Servos.
/// Ein "Adora Mini Pro" nutzt vielleicht Sound + 1 Servo.
/// Ein "Adora Kipper" nutzt vielleicht 1 Servo, keinen Sound.
struct DeviceConfig {
  const char* name;
  const BoardProfile board;
  uint8_t deviceType;           // Protocol Device-ID (0x02, 0x04, ...)

  // Feature-Aktivierung (Device-Entscheidung, nicht Board-Capability)
  bool useSound;                // Sound-System aktivieren?
  uint8_t useServos;            // 0, 1, oder 2 Servos nutzen
  bool useDistance;              // Distanzsensor nutzen?
  bool useDisplay;               // Display nutzen? (Mote)

  // Validierung: Feature nur wenn Board es auch kann
  constexpr bool canSound() const { return useSound && board.hasSound(); }
  constexpr bool canServo() const { return useServos > 0 && board.hasServo(); }
  constexpr bool canDistance() const { return useDistance && board.hasDistance(); }
};

// ─── Vordefinierte Devices ──────────────────────

inline constexpr DeviceConfig deviceAdoraMini() {
  return {
    .name = "Adora Mini",
    .board = boardAdoraOrig(),
    .deviceType = 0x02,
    .useSound = false,
    .useServos = 0,
    .useDistance = false,
    .useDisplay = false,
  };
}

inline constexpr DeviceConfig deviceAdoraMonsta() {
  return {
    .name = "Adora Monsta",
    .board = boardAdoraOrig(),
    .deviceType = 0x04,
    .useSound = false,
    .useServos = 0,
    .useDistance = false,
    .useDisplay = false,
  };
}

inline constexpr DeviceConfig deviceAdoraMiniPro() {
  return {
    .name = "Adora Mini Pro",
    .board = boardAdora26Pro(),
    .deviceType = 0x02,         // TBD
    .useSound = true,
    .useServos = 2,
    .useDistance = true,
    .useDisplay = false,
  };
}

}  // namespace adora
```

### 5.5 `AdoraDeviceTypes.h` — Typen & Konfiguration

```cpp
#pragma once
#include <stdint.h>

namespace adora {

// === Motor ===
enum class MotorDirection : uint8_t {
  Stop = 0, Forward = 1, Backward = 2, Brake = 3
};

// === Response-Kurven für Joystick ===
enum class ResponseCurve : uint8_t {
  Linear = 0, Quadratic, Cubic, Quartic, Quintic
};

// === Wende-Richtung (Autonommodus) ===
enum class TurnDirection : uint8_t {
  Left = 0, Right, Alternate
};

// === LED ===
enum class LedColor : uint8_t {
  Off = 0, White, Red, Green, Blue, Yellow, Magenta, Cyan
};

enum class LedPattern : uint8_t {
  Solid = 0, BlinkSlow, BlinkFast, Pulse, Rainbow
};

// === Sound ===
enum class SoundEvent : uint8_t {
  None = 0, Horn = 1, Confirm = 2,
  ReverseBeep = 10, Startup = 11, Connected = 12,
  Disconnected = 13, LowBattery = 14
};

enum class MuteMode : uint8_t {
  All = 0, AutoOnly = 1, Off = 2
};

// ─── Konfigurationsstructs ───

struct SteeringConfig {
  uint8_t joystickDeadzone = 15;
  uint8_t motorDeadzone = 20;
  float turnDamping = 0.65f;
  ResponseCurve curve = ResponseCurve::Quadratic;
  uint8_t maxSpeed = 255;
  uint8_t minSpeed = 30;
  float forwardGain = 1.0f;
  float turnGain = 0.7f;
};

struct RangeSenseConfig {
  uint16_t minDistanceMm = 150;
  uint16_t maxDistanceMm = 500;
  TurnDirection turnDirection = TurnDirection::Alternate;
  uint16_t turnDurationMs = 400;
  uint8_t turnPower = 180;
  uint8_t maxTurns = 0;
};

struct BatteryCalibration {
  float voltageDividerRatio = 1.0f;
  float referenceVoltage = 3.3f;
  uint16_t adcResolution = 1024;
  float calibrationOffset = 0.0f;
};

// ─── Zustandsstructs (read-only Snapshots) ───

struct MotorState {
  bool enabled;
  int16_t leftPower;     // -255..+255
  int16_t rightPower;    // -255..+255
};

struct LedState {
  bool enabled;
  uint8_t r, g, b;
  uint8_t brightness;
};

struct BatteryState {
  float voltage;
  uint8_t percent;
  bool isLow;
};

}  // namespace adora
```

### 5.6 `MotorDriver.h` — Abstrakte Motor-Schnittstelle

```cpp
#pragma once
#include "../AdoraDeviceTypes.h"

namespace adora {

class MotorDriver {
 public:
  virtual ~MotorDriver() = default;

  virtual bool begin() = 0;
  virtual void drive(int16_t leftPower, int16_t rightPower) = 0;
  virtual void stop() = 0;
  virtual void brake() = 0;
  virtual void setEnabled(bool enabled) = 0;
  virtual MotorState state() const = 0;

  /// Fault-Status (nur DRV8830, DRV8833 gibt immer 0).
  virtual uint8_t getFault() { return 0; }
  virtual void clearFault() {}
};

}  // namespace adora
```

### 5.7 `DRV8833Driver.h` / `I2CMotorDriver.h`

```cpp
// DRV8833Driver.h — GPIO H-Bridge (AdoraOrig, Adora26)
#pragma once
#include "MotorDriver.h"
#include "../profile/BoardProfile.h"

namespace adora {

class DRV8833Driver : public MotorDriver {
 public:
  explicit DRV8833Driver(const BoardProfile& board);
  bool begin() override;
  void drive(int16_t leftPower, int16_t rightPower) override;
  void stop() override;
  void brake() override;
  void setEnabled(bool enabled) override;
  MotorState state() const override;

 private:
  uint8_t a1_, a2_, b1_, b2_, sleep_;
  MotorState state_;
};

}  // namespace adora
```

```cpp
// I2CMotorDriver.h — DRV8830 Dual I2C (Adora26Pro)
#pragma once
#include "MotorDriver.h"
#include "../profile/BoardProfile.h"
#include <Wire.h>

namespace adora {

struct MotorFault {
  static constexpr uint8_t NONE   = 0x00;
  static constexpr uint8_t OCP    = 0x02;
  static constexpr uint8_t UVLO   = 0x04;
  static constexpr uint8_t OTS    = 0x08;
  static constexpr uint8_t ILIMIT = 0x10;
};

class I2CMotorDriver : public MotorDriver {
 public:
  explicit I2CMotorDriver(const BoardProfile& board);
  bool begin() override;
  void drive(int16_t leftPower, int16_t rightPower) override;
  void stop() override;
  void brake() override;
  void setEnabled(bool enabled) override;
  MotorState state() const override;
  uint8_t getFault() override;
  void clearFault() override;

 private:
  uint8_t addrA_, addrB_;
  MotorState state_;
  void writeMotor(uint8_t addr, int16_t power);
  uint8_t readFault(uint8_t addr);
};

}  // namespace adora
```

### 5.8 `TankSteering.h` — Plattformunabhängige Joystick-Mathematik

```cpp
#pragma once
#include "../AdoraDeviceTypes.h"

namespace adora {

/// Stateless. Kein Arduino nötig — auch in nativen Tests verwendbar.
struct TankSteering {
  static void joystickToMotors(int16_t joyX, int16_t joyY,
                               const SteeringConfig& cfg,
                               int16_t& outLeft, int16_t& outRight);

  static int16_t applyCurve(int16_t input, ResponseCurve curve);
  static int16_t applyDeadzone(int16_t input, uint8_t deadzone);
};

}  // namespace adora
```

### 5.9 `LedController.h`

```cpp
#pragma once
#include "../AdoraDeviceTypes.h"
#include "../profile/BoardProfile.h"

class Adafruit_NeoPixel;

namespace adora {

/// NeoPixel-Controller. Pins und Pixelzahl aus BoardProfile.
/// Handhabt automatisch R/G-Swap für LED 1+2 (WS2812B-2020 Noname-Variante).
class LedController {
 public:
  explicit LedController(const BoardProfile& board);
  ~LedController();

  bool begin();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t brightness);
  void off();
  void setNamedColor(LedColor color);
  void setBlinkMode(bool enabled, uint8_t r, uint8_t g, uint8_t b,
                    uint16_t intervalMs = 500);
  void update();
  LedState state() const;
  Adafruit_NeoPixel* strip();

 private:
  uint8_t pin_;
  uint8_t count_;
  Adafruit_NeoPixel* strip_;
  LedState state_;
};

}  // namespace adora
```

### 5.10 `SoundManager.h`

```cpp
#pragma once
#include "../AdoraDeviceTypes.h"

#ifdef ADORA_HAS_SOUND

namespace adora {

class SoundManager {
 public:
  SoundManager();
  ~SoundManager();

  void begin();
  bool addSound(SoundEvent event, const unsigned char* data, size_t length,
                bool isAuto = false);
  bool play(SoundEvent event);
  void stop();
  bool isPlaying() const;

  void setGain(float gain);
  void setMuteMode(MuteMode mode);
  void update();
};

}  // namespace adora

#endif  // ADORA_HAS_SOUND
```

### 5.11 `Battery.h` / `Button.h`

```cpp
namespace adora {

class Battery {
 public:
  explicit Battery(const BoardProfile& board);
  void begin();
  void update();
  BatteryState state() const;
  void setLowThreshold(float voltage);
  void setCalibration(const BatteryCalibration& cal);
};

class Button {
 public:
  using Callback = void (*)();
  explicit Button(const BoardProfile& board);
  void begin();
  void update();
  bool isPressed() const;
  void onPress(Callback cb);
  void onLongPress(Callback cb);
};

}  // namespace adora
```

### 5.12 `AdoraDevice.h` — Facade

```cpp
#pragma once
#include "AdoraDeviceTypes.h"
#include "profile/BoardProfiles.h"
#include "profile/DeviceConfig.h"
#include "motor/MotorDriver.h"
#include "motor/TankSteering.h"
#include "led/LedController.h"
#include "sensor/Battery.h"
#include "sensor/Button.h"

#ifdef ADORA_HAS_SOUND
#include "sound/SoundManager.h"
#endif

namespace adora {

/// Zentrale Fassade.
/// Erstellt automatisch den richtigen Motor-Treiber basierend auf Board.
/// Kommunikation (ESP-NOW, WiFi) ist über separate Module verfügbar,
/// wird aber nicht von der Facade orchestriert — der Aufrufer entscheidet.
class AdoraDevice {
 public:
  /// Aus Board-Typ (Board-Capabilities = Device-Features).
  explicit AdoraDevice(const BoardProfile& board = activeBoard());

  /// Aus Device-Config (Board kann mehr als Device nutzt).
  explicit AdoraDevice(const DeviceConfig& config);

  bool begin();
  void update();

  // ─── Motor ───
  void handleJoystick(int16_t x, int16_t y);
  void drive(int16_t leftPower, int16_t rightPower);
  void stop();
  void brake();
  void setMotorsEnabled(bool enabled);
  void setSteeringConfig(const SteeringConfig& cfg);

  // ─── Peripherie ───
  LedController& led();
  Battery& battery();
  Button& button();

#ifdef ADORA_HAS_SOUND
  SoundManager& sound();
#endif

  // ─── Status ───
  MotorState motorState() const;
  const BoardProfile& board() const;
  const McuCapabilities mcuCaps() const;

 private:
  BoardProfile profile_;
  MotorDriver* motor_;
  LedController led_;
  Battery battery_;
  Button button_;
  SteeringConfig steering_;
#ifdef ADORA_HAS_SOUND
  SoundManager sound_;
#endif
};

}  // namespace adora
```

---

## 6. Beispiele

### 6.1 Minimal (3 Zeilen Setup)

```cpp
#include <AdoraDevice.h>

// Board wird via -DADORA_BOARD=... in platformio.ini gesetzt.
// Hier: AdoraOrig (Default).
adora::AdoraDevice device;

void setup() {
  device.begin();
}

void loop() {
  device.update();
  device.drive(153, 153);   // geradeaus, 60%
  delay(2000);
  device.stop();
  delay(1000);
}
```

### 6.2 Explizites Board (z.B. für Etude)

```cpp
#include <AdoraDevice.h>

// Board explizit wählen — überschreibt -DADORA_BOARD
adora::AdoraDevice device(adora::boardAdora26Pro());

void setup() {
  device.begin();
  // Automatisch DRV8830 I2C statt DRV8833 GPIO
}
```

### 6.3 Device-Config (Board kann mehr als Device nutzt)

```cpp
#include <AdoraDevice.h>

// Adora26Pro hat Sound + 2 Servos, aber dieses Device nutzt nur 1 Servo
constexpr adora::DeviceConfig myKipper = {
  .name = "Adora Kipper",
  .board = adora::boardAdora26Pro(),
  .deviceType = 0x06,
  .useSound = false,      // Kipper braucht kein Sound
  .useServos = 1,         // nur Kipparm
  .useDistance = true,
  .useDisplay = false,
};

adora::AdoraDevice device(myKipper);

void setup() {
  device.begin();
  // Sound-System wird nicht initialisiert (spart RAM)
}
```

### 6.4 ESP-NOW Empfang (vollständig)

```cpp
#include <AdoraDevice.h>
#include <comm/EspNowComm.h>

adora::AdoraDevice device;

struct __attribute__((packed)) JoystickPacket {
  uint8_t type;
  int16_t x, y;
  uint8_t buttons;
  uint8_t r, g, b;
};

volatile bool newData = false;
JoystickPacket lastPacket;

void onReceive(uint8_t* mac, uint8_t* data, uint8_t len) {
  if (len >= sizeof(JoystickPacket) && data[0] == 0x01) {
    memcpy(&lastPacket, data, sizeof(JoystickPacket));
    newData = true;
  }
}

void setup() {
  device.begin();

  // ESP-NOW-Empfang — plattformabstrahiert
  adora::EspNowComm::begin();
  adora::EspNowComm::onReceive(onReceive);

  device.button().onLongPress([]() {
    device.stop();
    device.led().setBlinkMode(true, 255, 0, 0, 200);
  });
}

void loop() {
  device.update();

  if (newData) {
    newData = false;
    device.handleJoystick(lastPacket.x, lastPacket.y);
    if (lastPacket.r || lastPacket.g || lastPacket.b) {
      device.led().setColor(lastPacket.r, lastPacket.g, lastPacket.b);
    }
  }
}
```

### 6.5 TankSteering standalone (nativ, kein Arduino)

```cpp
#include <motor/TankSteering.h>
#include <stdio.h>

int main() {
  adora::SteeringConfig cfg;
  cfg.curve = adora::ResponseCurve::Quadratic;
  cfg.turnDamping = 0.65f;

  int16_t left, right;

  adora::TankSteering::joystickToMotors(0, 200, cfg, left, right);
  printf("Geradeaus: L=%d R=%d\n", left, right);

  adora::TankSteering::joystickToMotors(-100, 200, cfg, left, right);
  printf("Linkskurve: L=%d R=%d\n", left, right);

  return 0;
}
```

### 6.6 MCU-Capabilities abfragen

```cpp
#include <AdoraDevice.h>

void setup() {
  Serial.begin(115200);

  auto caps = adora::activeMcu();
  Serial.printf("MCU: %d Cores, %dKB RAM\n", caps.cores, caps.ramKB);
  Serial.printf("BLE: %s, 5GHz: %s\n",
    caps.hasBLE ? "ja" : "nein",
    caps.has5GHz ? "ja" : "nein");

  auto board = adora::activeBoard();
  Serial.printf("Board: %s\n", board.name);
  Serial.printf("GPIO Motor: %s, I2C Motor: %s\n",
    board.hasGpioMotor() ? "ja" : "nein",
    board.hasI2CMotor() ? "ja" : "nein");
  Serial.printf("Sound: %s, Servo: %s\n",
    board.hasSound() ? "ja" : "nein",
    board.hasServo() ? "ja" : "nein");
}

void loop() {}
```

---

## 7. platformio.ini — Board-Flag Integration

### Mini-Firmware (Adora-Fahrzeuge)

```ini
[env]
platform = espressif8266
board = d1_mini
framework = arduino

lib_deps =
    adafruit/Adafruit NeoPixel
    adafruit/Adafruit_VL53L0X@^1.2.4
    earlephilhower/ESP8266Audio@^1.9.9
    symlink://../../../shared/library/adora-device

; AdoraOrig — Standard (DRV8833, 1 LED, kein Sound)
[env:adoramini]
build_flags =
    -DADORA_BOARD=ADORA_BOARD_ORIG
    -DDEVICE_TYPE=0x02

; AdoraOrig — Monsta (selbes Board, anderer Device-Type)
[env:adoraminimonsta]
build_flags =
    -DADORA_BOARD=ADORA_BOARD_ORIG
    -DDEVICE_TYPE=0x04

; Adora26 — 3 LEDs, kein Sound
[env:adoramini26]
build_flags =
    -DADORA_BOARD=ADORA_BOARD_26
    -DDEVICE_TYPE=0x02

; Adora26Pro — DRV8830 I2C, Sound, Servos
[env:adoramini26pro]
build_flags =
    -DADORA_BOARD=ADORA_BOARD_26PRO
    -DDEVICE_TYPE=0x02
    -DADORA_HAS_SOUND
    -DADORA_HAS_DISTANCE
```

### NotBotBot-Firmware

```ini
[env]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

lib_deps =
    adafruit/Adafruit NeoPixel
    symlink://../../../shared/library/adora-device

[env:notbotbot]
build_flags =
    -DADORA_BOARD=ADORA_BOARD_NBB_S3
```

---

## 8. Migrations-Plan (Firmware → Library)

### Phase-Übersicht

| # | Paket | Beschreibung | Abhängig von | Parallel möglich? |
|---|---|---|---|---|
| **P1** | Profile | McuCapabilities + BoardProfile + DeviceConfig | — | ✅ |
| **P2** | Motor | DRV8833Driver + I2CMotorDriver + Interface | P1 | ✅ (nach P1) |
| **P3** | TankSteering | Pure math, native unit-tests | — | ✅ |
| **P4** | LED | LedController mit R/G-Swap | P1 | ✅ (nach P1) |
| **P5** | Sound | SoundManager (#ifdef) | P1 | ✅ (nach P1) |
| **P6** | Sensoren | Battery + Button + DistanceSensor | P1 | ✅ (nach P1) |
| **P7** | Comm | ESP-NOW + WiFi + Pairing (plattformabstrahiert) | P1 | ✅ (nach P1) |
| **P8** | Facade + Migration | AdoraDevice.h + Mini-Firmware umstellen | P1-P7 | ❌ |

### P1 → P7 können parallel laufen (nach P1)

```
     P1 (Profile)
     ┌────┤
     │    │
P3 ──┤    ├── P2 (Motor)
     │    ├── P4 (LED)
     │    ├── P5 (Sound)
     │    ├── P6 (Sensoren)
     │    └── P7 (Comm)
     │
     └────────── P8 (Facade + Migration)
```

### Quell-Mapping (Firmware → Library)

| Firmware-Datei | → Library-Modul | Anpassung |
|---|---|---|
| `config.cpp` (Pins) | `BoardProfiles.h` | Konstanten → Profile-Structs |
| `config.h` (Structs) | `AdoraDeviceTypes.h` | `TankSteeringSettings` → `SteeringConfig` |
| `build_config.h` (DEVICE_TYPE) | `DeviceConfig.h` | DEVICE_TYPE + Board-Flag |
| `motors.h/.cpp` | `DRV8833Driver` | Globale → Member, Pins aus BoardProfile |
| *(neu)* | `I2CMotorDriver` | Aus Etude DRV8830 Code |
| `joystick_to_tank_steering()` | `TankSteering` | 1:1, bereits reiner Algorithmus |
| `leds.h/.cpp` | `LedController` | Strip intern, R/G-Swap für LED1+2 |
| `sound_manager.h/.cpp` | `SoundManager` | Fast 1:1 |
| `espnow_handler.h/.cpp` | `EspNowComm` | Plattform-Switch ESP8266/ESP32 |
| `pairing_manager.h/.cpp` | `PairingManager` | Aus Firmware extrahieren |

### Firmware nach Migration

```cpp
// vorher (firmware main.cpp):
#include "config.h"
#include "motors.h"
#include "leds.h"
#include "espnow_handler.h"
// ... 20+ includes

// nachher:
#include <AdoraDevice.h>
#include <comm/EspNowComm.h>

adora::AdoraDevice device;  // Board via -DADORA_BOARD

void setup() {
  device.begin();
  adora::EspNowComm::begin();
}

void loop() {
  device.update();
}
```

### Migrations-Status (2026-03-26)

Firmware/Mini → Library-Delegation, abgeschlossen auf `main`:

| Block | Modul | Ergebnis | Commit | Anmerkung |
|---|---|---|---|---|
| A | Housekeeping | ✅ Done | `6159377` | Rename adora-vehicle → adora-device, LIBRARY_DESIGN.md Enum-Fix |
| B | PIO-Anbindung | ✅ Done | `16432ca` | library.json, lib_deps Symlink, adora_bridge.h |
| C | Motor | ✅ Done | `597b7d9` | `driveMotors()` → `DRV8833Driver`, `joystick_to_tank_steering()` → `TankSteering` |
| D | LED | ✅ Done | `4a9687f` | LedController besitzt Strip via BoardProfile. Macro `#define strip (*g_led.strip())` für Backward-Compat. led_modes.cpp unverändert. |
| E | Button | ✅ Done | `d4a7db4` | `button_handler` → `adora::Button`. Multi-Level Actions, LED-Feedback über Wrapper-Callbacks. -275 Zeilen. |
| F | Sound | ✅ Done | `40b8b19` | `sound_manager` → `adora::SoundManager`. API-Rename, SoundEvent-Enum, MuteMode cast. -418 Zeilen. |

**Migration komplett.** Alle 6 Blöcke delegiert, alle Firmware-Environments grün.

---

## 9. Beziehung zu `adora-basic`

```
                    ┌──────────────────┐
                    │    Flutter App    │
                    └────────┬─────────┘
                             │ WiFi/BT
                    ┌────────▼─────────┐
                    │    AdoraMote      │  ← verwendet adora-basic
                    │  (Fernsteuerung)  │     (Sender-Library)
                    └────────┬─────────┘
                             │ ESP-NOW
                    ┌────────▼─────────┐
                    │   AdoraDevice     │  ← verwendet adora-device
                    │   (Empfänger)     │     (Empfänger/Aktor-Library)
                    └───────────────────┘
```

Perspektivisch: `adora-device` könnte einen `CommandDecoder` enthalten, der `adora-basic` Pakete direkt versteht.
Perspektivisch: `adora-basic` könnte `McuCapabilities` und `BoardProfile` von `adora-device` importieren, damit Mote ebenfalls Board-Profile nutzt.

---

## 10. `library.properties` (Entwurf)

```properties
name=adora-device
version=0.1.0
author=Adora
maintainer=Adora
sentence=Cross-platform device library for Adora and NotBotBot robots.
paragraph=Board profiles (4-layer model: MCU/Carrier/Board/Device), motor control (DRV8833/DRV8830), NeoPixel LEDs, tank steering math, battery monitoring, sound playback, ESP-NOW/WiFi communication. Supports ESP8266, ESP32, ESP32-C6, ESP32-S3.
category=Device Control
architectures=*
depends=Adafruit NeoPixel
includes=AdoraDevice.h
```

---

## 11. Offene Fragen

1. **DRV8830 Adressen:** 0x60 + 0x62 im Design, aber I2C-Scan auf Hardware nötig.
2. **Battery Voltage-Divider-Ratio:** Pro Board identisch oder verschieden? Messen.
3. **Sleep-Pin (AdoraOrig/Adora26):** Wird DRV8833 nSLEEP aktiv genutzt oder nur Pullup?
4. **Comm-Abstraktion Tiefe:** ESP-NOW ESP8266-API vs ESP32-API — Wrapper-Tiefe klären.
5. ~~**Library-Ordner Umbenennung:** `adora-vehicle/` → `adora-device/`~~ ✅ Erledigt (2026-03-26).
6. **NotBotBot-Integration:** Servo-Module (nicht DC-Motor) — eigenes MotorDriver-Interface oder separates ServoController-Modul?
