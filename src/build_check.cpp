// Minimal build-check: stellt sicher dass alle Library-Header
// ohne Fehler kompilieren und linken.
#include <Arduino.h>
#include "AdoraDevice.h"
#include "comm/CommManager.h"
#include "comm/EspNowComm.h"
#include "comm/PairingManager.h"
#include "comm/WifiComm.h"

// Compile-time checks
static_assert(sizeof(adora::BoardProfile) > 0, "BoardProfile defined");
static_assert(sizeof(adora::McuCapabilities) > 0, "McuCapabilities defined");
static_assert(sizeof(PairingData) == 38, "PairingData 38 bytes");
static_assert(sizeof(JoystickData) == 9, "JoystickData 9 bytes");

// Link-Check-Sketch nur auf ESP8266 (historisches Verhalten der
// Fahrzeug-Firmware) bzw. bei explizitem Standalone-Check. Auf ESP32-
// Konsumenten (NotBotBot) zieht der Linker dieses Archiv-Member vor dem
// Projekt-main.cpp und setup()/loop() kollidieren.
#if defined(ESP8266) || defined(ADORA_BUILD_CHECK)

adora::AdoraDevice device;

void setup() {
  device.begin();

  // Comm init
  CommManager::init();
  EspNowComm::begin();
}

void loop() {
  device.update();
  CommManager::update();
  EspNowComm::updateKeepalive();
}

#endif  // ESP8266 || ADORA_BUILD_CHECK
