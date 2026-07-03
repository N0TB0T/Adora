#pragma once
#include "../AdoraDeviceTypes.h"
#include "../profile/BoardProfile.h"
#include <stdint.h>

namespace adora {

/// Batterie-Spannungsmessung über ADC.
/// Unterstützt 5 Batterietypen mit Auto-Detection (Varianzanalyse).
class Battery {
 public:
  explicit Battery(const BoardProfile& board);

  /// Initialisiert ADC, Auto-Detection wenn type == Aaa4xNiMH (Default).
  void begin(BatteryType type = BatteryType::Aaa4xNiMH);

  /// ADC lesen und Zustand aktualisieren. Regelmäßig in loop() aufrufen.
  void update();

  BatteryState state() const;

  /// Schwellwert für Low-Battery Warnung (Default: 3.6V).
  void setLowThreshold(float voltage);

  /// ADC-Faktor manuell ändern (V pro ADC-Digit).
  void setAdcFactor(float factor);

  /// Batterietyp manuell ändern.
  void setType(BatteryType type);
  BatteryType type() const;

  /// Menschenlesbarer Name des aktuellen Typs.
  const char* typeName() const;

 private:
  uint8_t pin_;
  float adcFactor_;
  BatteryState state_;
  float lowThreshold_;
  unsigned long lastReadMs_;

  float readVoltage() const;
  uint8_t voltageToPercent(float voltage) const;
  BatteryType autoDetect() const;
};

}  // namespace adora
