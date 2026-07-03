#pragma once
#include "MotorDriver.h"
#include "../profile/BoardProfile.h"

namespace adora {

/// GPIO H-Bridge Motor-Treiber für DRV8833 (AdoraOrig, Adora26).
/// Pins werden aus BoardProfile gelesen — kein Hardcoding.
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

  void driveMotor(uint8_t pin1, uint8_t pin2, int16_t power);
  void stopPins(uint8_t pin1, uint8_t pin2);
};

}  // namespace adora
