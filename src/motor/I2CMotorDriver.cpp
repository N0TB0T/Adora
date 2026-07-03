#include "I2CMotorDriver.h"

namespace adora {

I2CMotorDriver::I2CMotorDriver(const BoardProfile& board)
    : addrA_(board.i2cMotor.addrA),
      addrB_(board.i2cMotor.addrB),
      state_{false, 0, 0} {}

bool I2CMotorDriver::begin() {
  // I2C muss extern initialisiert sein (Wire.begin() im Aufrufer)
  // Prüfe ob die Adressen antworten
  Wire.beginTransmission(addrA_);
  if (Wire.endTransmission() != 0) return false;
  Wire.beginTransmission(addrB_);
  if (Wire.endTransmission() != 0) return false;

  state_.enabled = true;
  stop();
  clearFault();
  return true;
}

void I2CMotorDriver::drive(int16_t leftPower, int16_t rightPower) {
  if (!state_.enabled) return;
  writeMotor(addrA_, leftPower);
  writeMotor(addrB_, rightPower);
  state_.leftPower = leftPower;
  state_.rightPower = rightPower;
}

void I2CMotorDriver::stop() {
  writeMotor(addrA_, 0);
  writeMotor(addrB_, 0);
  state_.leftPower = 0;
  state_.rightPower = 0;
}

void I2CMotorDriver::brake() {
  // DRV8830: VSET=0, IN1=1, IN2=1 → Bremse
  // Control register: [VSET(7:2)][IN2][IN1] = 0b00000011 = 0x03
  Wire.beginTransmission(addrA_);
  Wire.write(REG_CONTROL);
  Wire.write(0x03);
  Wire.endTransmission();

  Wire.beginTransmission(addrB_);
  Wire.write(REG_CONTROL);
  Wire.write(0x03);
  Wire.endTransmission();

  state_.leftPower = 0;
  state_.rightPower = 0;
}

void I2CMotorDriver::setEnabled(bool enabled) {
  state_.enabled = enabled;
  if (!enabled) {
    stop();
  }
}

MotorState I2CMotorDriver::state() const {
  return state_;
}

uint8_t I2CMotorDriver::getFault() {
  return readFault(addrA_) | readFault(addrB_);
}

void I2CMotorDriver::clearFault() {
  clearFaultReg(addrA_);
  clearFaultReg(addrB_);
}

void I2CMotorDriver::writeMotor(uint8_t addr, int16_t power) {
  // DRV8830 Control Register: [VSET(7:2)][IN2][IN1]
  // VSET: 6-Bit Spannungswert (0-63, 0x06-0x3F nutzbar, 0-5 = Standby)
  // IN1/IN2: 00=Standby, 01=Reverse, 10=Forward, 11=Brake
  uint8_t ctrl;
  if (power == 0) {
    ctrl = 0x00;  // Standby
  } else {
    // Map -255..+255 → 6..63 VSET (0-5 = Standby, 6 = minimum)
    int16_t absPower = power < 0 ? -power : power;
    if (absPower > 255) absPower = 255;
    uint8_t vset = static_cast<uint8_t>(6 + (absPower * 57UL) / 255);

    if (power > 0) {
      ctrl = static_cast<uint8_t>((vset << 2) | 0x02);  // Forward: IN2=1, IN1=0
    } else {
      ctrl = static_cast<uint8_t>((vset << 2) | 0x01);  // Reverse: IN2=0, IN1=1
    }
  }

  Wire.beginTransmission(addr);
  Wire.write(REG_CONTROL);
  Wire.write(ctrl);
  Wire.endTransmission();
}

uint8_t I2CMotorDriver::readFault(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(REG_FAULT);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, static_cast<uint8_t>(1));
  return Wire.available() ? Wire.read() : 0;
}

void I2CMotorDriver::clearFaultReg(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(REG_FAULT);
  Wire.write(0x80);  // CLEAR bit
  Wire.endTransmission();
}

}  // namespace adora
