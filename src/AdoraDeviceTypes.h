#pragma once
#include <stdint.h>

namespace adora {

// === Motor ===
enum class MotorDirection : uint8_t {
  Stop = 0,
  Forward = 1,
  Backward = 2,
  Brake = 3
};

// === Response-Kurven für Joystick ===
enum class ResponseCurve : uint8_t {
  Linear = 0,
  Quadratic,
  Cubic,
  Quartic,
  Quintic
};

// === Wende-Richtung (Autonommodus) ===
enum class TurnDirection : uint8_t {
  Left = 0,
  Right,
  Alternate
};

// === LED ===
enum class LedColor : uint8_t {
  Off = 0,
  White,
  Red,
  Green,
  Blue,
  Yellow,
  Magenta,
  Cyan
};

enum class LedPattern : uint8_t {
  Solid = 0,
  BlinkSlow,
  BlinkFast,
  Pulse,
  Rainbow
};

enum class LedColorOrder : uint8_t {
  GRB = 0,  // Standard WS2812B (NEO_GRB)
  RGB = 1   // Einige Noname-Varianten (NEO_RGB)
};

// === Sound ===
enum class SoundEvent : uint8_t {
  None = 0,
  Horn = 1,
  Confirm = 2,
  ReverseBeep = 10,
  Startup = 11,
  Shutdown = 12,
  LowBattery = 13,
  Error = 14,
  PairSuccess = 15,
  PairFail = 16,
  TurnLeft = 20,
  TurnRight = 21,
  ModeToRemote = 22,
  ModeToRange = 23
};

enum class MuteMode : uint8_t {
  All = 0,
  AutoOnly = 1,
  Off = 2
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

// === Battery ===
// Beliebig erweiterbar — jeder Typ hat eigene Spannungsrange + Entladekurve.
// Der ADC-Faktor (V/Digit) ist Board-spezifisch und steckt im BoardProfile.
enum class BatteryType : uint8_t {
  UsbPower = 0,      // USB 5V (konstant ~5V, kein Akku)
  Aaa4xNiMH,         // 4x AAA NiMH: 4.0V–5.4V (1.0–1.35V/Zelle)
  Aaa4xLithium,      // 4x AAA Lithium primary: 4.0V–6.4V (1.0–1.6V/Zelle)
  LiIon1S,           // 1S Li-Ion/LiPo 3.7V: 3.0V–4.2V
  LiFePo4_1S,        // 1S LiFePO4 3.2V: 2.5V–3.65V
};

// === Button ===
enum class ButtonTrigger : uint8_t {
  OnPress = 0,   // Feuert sofort wenn Schwelle erreicht (während gedrückt)
  OnRelease = 1  // Feuert beim Loslassen (wenn über Schwelle gehalten)
};

struct ButtonAction {
  uint16_t durationMs;      // Zeit-Schwelle in Millisekunden
  uint8_t actionType;       // Action-ID (benutzerdefiniert)
  ButtonTrigger trigger;    // Wann auslösen
};



// ─── Zustandsstructs (read-only Snapshots) ───

struct MotorState {
  bool enabled;
  int16_t leftPower;   // -255..+255
  int16_t rightPower;  // -255..+255
};

struct LedState {
  bool enabled;
  uint8_t r, g, b;
  uint8_t brightness;
};

struct BatteryState {
  float voltage;
  uint8_t percent;
  uint16_t rawAdc;
  bool isLow;
  bool valid;
  BatteryType type;
};

}  // namespace adora
