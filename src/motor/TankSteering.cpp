#include "TankSteering.h"

// Plattformunabhängig — kein Arduino.h, kein constrain()
namespace {

inline int16_t clamp(int16_t value, int16_t minVal, int16_t maxVal) {
  if (value < minVal) return minVal;
  if (value > maxVal) return maxVal;
  return value;
}

inline int16_t absVal(int16_t v) {
  return v < 0 ? -v : v;
}

}  // namespace

namespace adora {

void TankSteering::joystickToMotors(int16_t joyX, int16_t joyY,
                                     const SteeringConfig& cfg,
                                     int16_t& outLeft, int16_t& outRight) {
  // Joystick Deadzone
  joyX = applyDeadzone(joyX, cfg.joystickDeadzone);
  joyY = applyDeadzone(joyY, cfg.joystickDeadzone);

  // Response Curve
  joyX = applyCurve(joyX, cfg.curve);
  joyY = applyCurve(joyY, cfg.curve);

  // Forward/Turn Berechnung
  float forward = static_cast<float>(joyY) * cfg.forwardGain;
  float turn = static_cast<float>(joyX) * cfg.turnGain * cfg.turnDamping;

  // Tank Steering: links = forward + turn, rechts = forward - turn
  float leftRaw = forward + turn;
  float rightRaw = forward - turn;

  // Skalierung: Joystick-Bereich (±1024) → Motor-Bereich (±maxSpeed)
  float scaleFactor = static_cast<float>(cfg.maxSpeed) / 1024.0f;
  int16_t maxSpd = static_cast<int16_t>(cfg.maxSpeed);
  outLeft = clamp(static_cast<int16_t>(leftRaw * scaleFactor), -maxSpd, maxSpd);
  outRight = clamp(static_cast<int16_t>(rightRaw * scaleFactor), -maxSpd, maxSpd);

  // Motor Deadzone
  if (absVal(outLeft) < static_cast<int16_t>(cfg.motorDeadzone)) outLeft = 0;
  if (absVal(outRight) < static_cast<int16_t>(cfg.motorDeadzone)) outRight = 0;
}

int16_t TankSteering::applyCurve(int16_t input, ResponseCurve curve) {
  if (input == 0) return 0;

  // Normalisierung auf 0..1
  bool negative = input < 0;
  float normalized = static_cast<float>(absVal(input)) / 1024.0f;
  if (normalized > 1.0f) normalized = 1.0f;

  float result;
  switch (curve) {
    case ResponseCurve::Linear:
      result = normalized;
      break;
    case ResponseCurve::Quadratic:
      result = normalized * normalized;
      break;
    case ResponseCurve::Cubic:
      result = normalized * normalized * normalized;
      break;
    case ResponseCurve::Quartic:
      result = normalized * normalized * normalized * normalized;
      break;
    case ResponseCurve::Quintic:
      result = normalized * normalized * normalized * normalized * normalized;
      break;
    default:
      result = normalized;
      break;
  }

  int16_t output = static_cast<int16_t>(result * 1024.0f);
  return negative ? -output : output;
}

int16_t TankSteering::applyDeadzone(int16_t input, uint8_t deadzone) {
  if (absVal(input) < static_cast<int16_t>(deadzone)) return 0;
  return input;
}

}  // namespace adora
