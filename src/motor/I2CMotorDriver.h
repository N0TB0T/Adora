#pragma once
#include "MotorDriver.h"
#include "../profile/BoardProfile.h"
#include <Wire.h>

namespace adora {

/// DRV8830 Fault-Register Flags
struct MotorFault {
  static constexpr uint8_t NONE   = 0x00;
  static constexpr uint8_t OCP    = 0x02;  // Overcurrent
  static constexpr uint8_t UVLO   = 0x04;  // Undervoltage lockout
  static constexpr uint8_t OTS    = 0x08;  // Overtemperature
  static constexpr uint8_t ILIMIT = 0x10;  // Current limit exceeded
};

/// DRV8830 I2C Motor-Treiber (Adora26Pro).
/// Dual-Motor über I2C an verschiedenen Adressen.
class I2CMotorDriver : public MotorDriver {
 public:
  explicit I2CMotorDriver(const BoardProfile& board);

  bool begin() override;
  void drive(int16_t leftPower, int16_t rightPower) override;
  void stop() override;
  void brake() override;
  void setEnabled(bool enabled) override;
  MotorState state() const override;
  uint8_t getFault() override;
  void clearFault() override;

 private:
  uint8_t addrA_, addrB_;
  MotorState state_;

  // DRV8830 Register
  static constexpr uint8_t REG_CONTROL = 0x00;
  static constexpr uint8_t REG_FAULT   = 0x01;

  void writeMotor(uint8_t addr, int16_t power);
  uint8_t readFault(uint8_t addr);
  void clearFaultReg(uint8_t addr);
};

}  // namespace adora
