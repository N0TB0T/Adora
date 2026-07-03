#pragma once
#include "../AdoraDeviceTypes.h"

namespace adora {

/// Plattformunabhängige Joystick→Motor-Mathematik.
/// Kein Arduino, kein Hardware-Zugriff — auch in nativen Tests verwendbar.
struct TankSteering {
  /// Joystick-Werte (±1024 Bereich) in Motor-Power (±maxSpeed) umrechnen.
  static void joystickToMotors(int16_t joyX, int16_t joyY,
                               const SteeringConfig& cfg,
                               int16_t& outLeft, int16_t& outRight);

  /// Response-Kurve auf einen Wert anwenden.
  /// Input und Output im Bereich ±1024.
  static int16_t applyCurve(int16_t input, ResponseCurve curve);

  /// Deadzone anwenden. Werte innerhalb der Deadzone → 0.
  static int16_t applyDeadzone(int16_t input, uint8_t deadzone);
};

}  // namespace adora
