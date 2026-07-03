#include "DRV8833Driver.h"
#include <Arduino.h>

namespace adora {

DRV8833Driver::DRV8833Driver(const BoardProfile& board)
    : a1_(board.gpioMotor.pinA1),
      a2_(board.gpioMotor.pinA2),
      b1_(board.gpioMotor.pinB1),
      b2_(board.gpioMotor.pinB2),
      sleep_(board.gpioMotor.pinSleep),
      state_{false, 0, 0} {}

bool DRV8833Driver::begin() {
  pinMode(a1_, OUTPUT);
  pinMode(a2_, OUTPUT);
  pinMode(b1_, OUTPUT);
  pinMode(b2_, OUTPUT);
  if (sleep_ != 0) {
    pinMode(sleep_, OUTPUT);
    digitalWrite(sleep_, HIGH);  // nSLEEP active HIGH = enabled
  }
  state_.enabled = true;
  stop();
  return true;
}

void DRV8833Driver::drive(int16_t leftPower, int16_t rightPower) {
  if (!state_.enabled) return;
  driveMotor(a1_, a2_, leftPower);
  driveMotor(b1_, b2_, rightPower);
  state_.leftPower = leftPower;
  state_.rightPower = rightPower;
}

void DRV8833Driver::stop() {
  stopPins(a1_, a2_);
  stopPins(b1_, b2_);
  state_.leftPower = 0;
  state_.rightPower = 0;
}

void DRV8833Driver::brake() {
  // DRV8833: beide Pins HIGH = elektrische Bremse
  analogWrite(a1_, 255);
  analogWrite(a2_, 255);
  analogWrite(b1_, 255);
  analogWrite(b2_, 255);
  state_.leftPower = 0;
  state_.rightPower = 0;
}

void DRV8833Driver::setEnabled(bool enabled) {
  state_.enabled = enabled;
  if (!enabled) {
    stop();
    if (sleep_ != 0) {
      digitalWrite(sleep_, LOW);  // nSLEEP LOW = sleep mode
    }
  } else {
    if (sleep_ != 0) {
      digitalWrite(sleep_, HIGH);
    }
  }
}

MotorState DRV8833Driver::state() const {
  return state_;
}

void DRV8833Driver::driveMotor(uint8_t pin1, uint8_t pin2, int16_t power) {
  if (power == 0) {
    stopPins(pin1, pin2);
  } else if (power > 0) {
    uint8_t speed = (power > 255) ? 255 : static_cast<uint8_t>(power);
    analogWrite(pin1, 0);
    analogWrite(pin2, speed);
  } else {
    uint8_t speed = (power < -255) ? 255 : static_cast<uint8_t>(-power);
    analogWrite(pin1, speed);
    analogWrite(pin2, 0);
  }
}

void DRV8833Driver::stopPins(uint8_t pin1, uint8_t pin2) {
  analogWrite(pin1, 0);
  analogWrite(pin2, 0);
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
}

}  // namespace adora
