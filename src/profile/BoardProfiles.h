#pragma once
#include "BoardProfile.h"

// Platform-spezifische Pin-Definitionen
// ESP8266: Dx-Defines aus Arduino Framework
// ESP32: GPIO-Nummern direkt
#if defined(ESP8266)
  #include <Arduino.h>  // für D0..D8, A0
#endif

namespace adora {

// ─── Adora Boards (ESP8266 / Wemos D1 Mini) ──────────────────────

#if defined(ESP8266)

inline constexpr BoardProfile boardAdoraOrig() {
  return {
    .name = "AdoraOrig",
    .mcu = Mcu::Esp8266,
    .gpioMotor = {D1, D2, D6, D5, D0},    // A1, A2, B1, B2, Sleep
    .i2cMotor  = {false, 0, 0},
    .led       = {D7, 1, LedColorOrder::RGB},  // 1× WS2812 (Center)
    .sound     = {false},
    .servo     = {0, 0},
    .button    = {D8, false},              // D8, Pull-up extern
    .battery   = {A0, 0.010377f, BatteryType::Aaa4xNiMH},
    .i2c       = {D4, D3},                // SCL=D4, SDA=D3
    .distance  = {true, 0},               // 4-pol Stecker, kein XSHUT
  };
}

inline constexpr BoardProfile boardAdora26() {
  return {
    .name = "Adora26",
    .mcu = Mcu::Esp8266,
    .gpioMotor = {D1, D2, D6, D5, D0},
    .i2cMotor  = {false, 0, 0},
    .led       = {D7, 3, LedColorOrder::GRB},  // 3× WS2812 (Center, L, R)
    .sound     = {true},                    // I2S NoDAC auf GPIO3/RX — kein Pin-Konflikt
    .servo     = {0, 0},
    .button    = {D8, false},
    .battery   = {A0, 0.010377f, BatteryType::Aaa4xNiMH},
    .i2c       = {D4, D3},
    .distance  = {true, 0},
  };
}

inline constexpr BoardProfile boardAdora26Pro() {
  return {
    .name = "Adora26Pro",
    .mcu = Mcu::Esp8266,
    .gpioMotor = {0, 0, 0, 0, 0},          // keine GPIO-Motoren
    .i2cMotor  = {true, 0x60, 0x62},        // DRV8830 Dual — Adressen TODO: verify
    .led       = {D7, 3, LedColorOrder::GRB},
    .sound     = {true},                     // I2S NoDac (GPIO3=Data, fixed)
    .servo     = {D5, D6},                   // 2× PWM Servo
    .button    = {D3, true},                 // D3=GPIO0, active LOW
    .battery   = {A0, 0.010377f, BatteryType::Aaa4xNiMH},
    .i2c       = {D1, D2},                   // SCL=D1, SDA=D2
    .distance  = {true, D0},                 // 4-pol Stecker + XSHUT an D0
  };
}

#endif  // ESP8266

// ─── NotBotBot Boards (ESP32-S3) ──────────────────────

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ESP32)

inline constexpr BoardProfile boardNotBotBotS3() {
  return {
    .name = "NotBotBot-S3",
    .mcu = Mcu::Esp32S3,
    .gpioMotor = {0, 0, 0, 0, 0},
    .i2cMotor  = {false, 0, 0},
    .led       = {7, 8, LedColorOrder::GRB},    // GPIO7, 8 Pixels
    .sound     = {false},
    .servo     = {5, 6},                     // GPIO5, GPIO6
    .button    = {8, true},
    .battery   = {0, 0.0f, BatteryType::UsbPower},  // kein Battery ADC
    .i2c       = {0, 0},                     // kein I2C-Bus definiert
    .distance  = {false, 0},
  };
}

#endif  // ESP32

// ─── Compile-time Board-Auswahl ──────────────────────

#define ADORA_BOARD_ORIG      0
#define ADORA_BOARD_26        1
#define ADORA_BOARD_26PRO     2
#define ADORA_BOARD_NBB_S3    10

#ifndef ADORA_BOARD
  #if defined(ESP8266)
    #define ADORA_BOARD ADORA_BOARD_ORIG    // Default: AdoraOrig
  #else
    #define ADORA_BOARD ADORA_BOARD_NBB_S3  // ESP32: einziges ESP32-Profil
  #endif
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
