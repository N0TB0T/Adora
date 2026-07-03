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
  bool i2sPinsConfigurable;  // ESP32*: ja, ESP8266: nein (fixed GPIO3/15/2)
};

// Bekannte MCU-Profile
inline constexpr McuCapabilities mcuESP8266() {
  return {Mcu::Esp8266, false, false, false, 1, 80, 11, false};
}
inline constexpr McuCapabilities mcuESP32() {
  return {Mcu::Esp32, true, false, false, 2, 520, 34, true};
}
inline constexpr McuCapabilities mcuESP32C6() {
  return {Mcu::Esp32C6, true, true, true, 1, 512, 22, true};
}
inline constexpr McuCapabilities mcuESP32S3() {
  return {Mcu::Esp32S3, true, false, false, 2, 512, 45, true};
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
