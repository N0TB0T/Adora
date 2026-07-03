#pragma once
#include "../AdoraDeviceTypes.h"
#include <stdint.h>

namespace adora {

/// Abstrakte Motor-Schnittstelle: DRV8833 (GPIO) und DRV8830 (I2C)
/// implementieren dieses Interface.
class MotorDriver {
 public:
  virtual ~MotorDriver() = default;

  virtual bool begin() = 0;
  virtual void drive(int16_t leftPower, int16_t rightPower) = 0;
  virtual void stop() = 0;
  virtual void brake() = 0;
  virtual void setEnabled(bool enabled) = 0;
  virtual MotorState state() const = 0;

  /// Fault-Status (nur DRV8830, DRV8833 gibt immer 0).
  virtual uint8_t getFault() { return 0; }
  virtual void clearFault() {}
};

}  // namespace adora
