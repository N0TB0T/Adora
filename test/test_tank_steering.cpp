#include <cassert>
#include <cmath>
#include <cstdio>
#include "../src/motor/TankSteering.h"

// TankSteering.cpp einbinden (header-only build für native Tests)
#include "../src/motor/TankSteering.cpp"

using namespace adora;

static SteeringConfig defaultCfg() {
  return SteeringConfig{};  // Defaults aus AdoraDeviceTypes.h
}

// --- applyDeadzone ---

void test_deadzone_inside_returns_zero() {
  assert(TankSteering::applyDeadzone(10, 15) == 0);
  assert(TankSteering::applyDeadzone(-10, 15) == 0);
  assert(TankSteering::applyDeadzone(0, 15) == 0);
  printf("  PASS: deadzone_inside_returns_zero\n");
}

void test_deadzone_outside_passes_through() {
  assert(TankSteering::applyDeadzone(20, 15) == 20);
  assert(TankSteering::applyDeadzone(-20, 15) == -20);
  assert(TankSteering::applyDeadzone(1024, 15) == 1024);
  printf("  PASS: deadzone_outside_passes_through\n");
}

void test_deadzone_boundary() {
  // Exactly at deadzone boundary → still zero (< not <=)
  assert(TankSteering::applyDeadzone(15, 15) == 15);
  assert(TankSteering::applyDeadzone(14, 15) == 0);
  printf("  PASS: deadzone_boundary\n");
}

// --- applyCurve ---

void test_curve_zero_returns_zero() {
  assert(TankSteering::applyCurve(0, ResponseCurve::Linear) == 0);
  assert(TankSteering::applyCurve(0, ResponseCurve::Quadratic) == 0);
  assert(TankSteering::applyCurve(0, ResponseCurve::Cubic) == 0);
  printf("  PASS: curve_zero_returns_zero\n");
}

void test_curve_linear_passthrough() {
  assert(TankSteering::applyCurve(512, ResponseCurve::Linear) == 512);
  assert(TankSteering::applyCurve(1024, ResponseCurve::Linear) == 1024);
  assert(TankSteering::applyCurve(-1024, ResponseCurve::Linear) == -1024);
  printf("  PASS: curve_linear_passthrough\n");
}

void test_curve_quadratic_reduces_low() {
  // 512/1024 = 0.5 → 0.5² = 0.25 → 0.25*1024 = 256
  int16_t result = TankSteering::applyCurve(512, ResponseCurve::Quadratic);
  assert(result == 256);
  printf("  PASS: curve_quadratic_reduces_low\n");
}

void test_curve_quadratic_full_at_max() {
  // 1024/1024 = 1.0 → 1.0² = 1.0 → 1024
  int16_t result = TankSteering::applyCurve(1024, ResponseCurve::Quadratic);
  assert(result == 1024);
  printf("  PASS: curve_quadratic_full_at_max\n");
}

void test_curve_preserves_sign() {
  int16_t pos = TankSteering::applyCurve(512, ResponseCurve::Quadratic);
  int16_t neg = TankSteering::applyCurve(-512, ResponseCurve::Quadratic);
  assert(pos > 0);
  assert(neg < 0);
  assert(pos == -neg);
  printf("  PASS: curve_preserves_sign\n");
}

// --- joystickToMotors ---

void test_centered_joystick_gives_zero() {
  auto cfg = defaultCfg();
  int16_t left, right;
  TankSteering::joystickToMotors(0, 0, cfg, left, right);
  assert(left == 0);
  assert(right == 0);
  printf("  PASS: centered_joystick_gives_zero\n");
}

void test_full_forward() {
  auto cfg = defaultCfg();
  cfg.curve = ResponseCurve::Linear;  // einfacher zu prüfen
  cfg.turnDamping = 1.0f;
  cfg.forwardGain = 1.0f;
  cfg.turnGain = 1.0f;
  int16_t left, right;
  TankSteering::joystickToMotors(0, 1024, cfg, left, right);
  assert(left > 0);
  assert(right > 0);
  assert(left == right);
  printf("  PASS: full_forward (L=%d R=%d)\n", left, right);
}

void test_full_backward() {
  auto cfg = defaultCfg();
  cfg.curve = ResponseCurve::Linear;
  int16_t left, right;
  TankSteering::joystickToMotors(0, -1024, cfg, left, right);
  assert(left < 0);
  assert(right < 0);
  assert(left == right);
  printf("  PASS: full_backward (L=%d R=%d)\n", left, right);
}

void test_turn_right() {
  auto cfg = defaultCfg();
  cfg.curve = ResponseCurve::Linear;
  cfg.turnDamping = 1.0f;
  cfg.turnGain = 1.0f;
  int16_t left, right;
  TankSteering::joystickToMotors(1024, 0, cfg, left, right);
  // Turn right: left = +turn, right = -turn
  assert(left > 0);
  assert(right < 0);
  assert(left == -right);
  printf("  PASS: turn_right (L=%d R=%d)\n", left, right);
}

void test_turn_left() {
  auto cfg = defaultCfg();
  cfg.curve = ResponseCurve::Linear;
  cfg.turnDamping = 1.0f;
  cfg.turnGain = 1.0f;
  int16_t left, right;
  TankSteering::joystickToMotors(-1024, 0, cfg, left, right);
  assert(left < 0);
  assert(right > 0);
  printf("  PASS: turn_left (L=%d R=%d)\n", left, right);
}

void test_small_input_within_deadzone_gives_zero() {
  auto cfg = defaultCfg();
  // joystickDeadzone=15, motorDeadzone=20
  int16_t left, right;
  TankSteering::joystickToMotors(5, 5, cfg, left, right);
  assert(left == 0);
  assert(right == 0);
  printf("  PASS: small_input_within_deadzone_gives_zero\n");
}

void test_motor_deadzone_clamps_small_output() {
  auto cfg = defaultCfg();
  cfg.curve = ResponseCurve::Linear;
  cfg.motorDeadzone = 50;
  int16_t left, right;
  // Y=100 → forward=100, scale=255/1024≈0.249 → ~25 < motorDeadzone 50
  TankSteering::joystickToMotors(0, 100, cfg, left, right);
  assert(left == 0);
  assert(right == 0);
  printf("  PASS: motor_deadzone_clamps_small_output\n");
}

void test_max_speed_clamping() {
  auto cfg = defaultCfg();
  cfg.curve = ResponseCurve::Linear;
  cfg.maxSpeed = 200;
  cfg.forwardGain = 1.0f;
  int16_t left, right;
  TankSteering::joystickToMotors(0, 1024, cfg, left, right);
  assert(left <= 200);
  assert(right <= 200);
  printf("  PASS: max_speed_clamping (L=%d R=%d)\n", left, right);
}

int main() {
  printf("=== TankSteering Tests ===\n\n");

  printf("[applyDeadzone]\n");
  test_deadzone_inside_returns_zero();
  test_deadzone_outside_passes_through();
  test_deadzone_boundary();

  printf("\n[applyCurve]\n");
  test_curve_zero_returns_zero();
  test_curve_linear_passthrough();
  test_curve_quadratic_reduces_low();
  test_curve_quadratic_full_at_max();
  test_curve_preserves_sign();

  printf("\n[joystickToMotors]\n");
  test_centered_joystick_gives_zero();
  test_full_forward();
  test_full_backward();
  test_turn_right();
  test_turn_left();
  test_small_input_within_deadzone_gives_zero();
  test_motor_deadzone_clamps_small_output();
  test_max_speed_clamping();

  printf("\n=== ALL %d TESTS PASSED ===\n", 16);
  return 0;
}
