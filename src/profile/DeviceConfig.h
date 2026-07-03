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
  uint8_t deviceType;  // Protocol Device-ID (0x02, 0x04, ...)

  // Feature-Aktivierung (Device-Entscheidung, nicht Board-Capability)
  bool useSound;       // Sound-System aktivieren?
  uint8_t useServos;   // 0, 1, oder 2 Servos nutzen
  bool useDistance;     // Distanzsensor nutzen?
  bool useDisplay;     // Display nutzen? (Mote)

  // Validierung: Feature nur wenn Board es auch kann
  constexpr bool canSound() const { return useSound && board.hasSound(); }
  constexpr bool canServo() const { return useServos > 0 && board.hasServo(); }
  constexpr bool canDistance() const { return useDistance && board.hasDistance(); }
};

// ─── Vordefinierte Devices ──────────────────────

// Die Adora-Fahrzeug-Configs referenzieren ESP8266-only Board-Profile
// (BoardProfiles.h) und sind daher plattform-geguarded, damit die Library
// auch auf ESP32 (NotBotBot) kompiliert.
#if defined(ESP8266)

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
    .deviceType = 0x02,  // TBD
    .useSound = true,
    .useServos = 2,
    .useDistance = true,
    .useDisplay = false,
  };
}

#endif  // ESP8266

}  // namespace adora
