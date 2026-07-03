#pragma once
#include "McuCapabilities.h"
#include "../AdoraDeviceTypes.h"

namespace adora {

/// Vollständige Hardware-Beschreibung eines Custom Boards.
/// Beschreibt was das Board KANN — nicht was das Device NUTZT.
struct BoardProfile {
  const char* name;  // z.B. "AdoraOrig", "Adora26Pro"
  Mcu mcu;           // impliziert Carrier → MCU Capabilities

  // — Motoren (GPIO) —
  struct {
    uint8_t pinA1, pinA2;  // Motor A (links)   — 0 wenn nicht vorhanden
    uint8_t pinB1, pinB2;  // Motor B (rechts)  — 0 wenn nicht vorhanden
    uint8_t pinSleep;      // DRV8833 nSLEEP    — 0 wenn nicht vorhanden
  } gpioMotor;

  // — Motoren (I2C) —
  struct {
    bool available;   // true → DRV8830 vorhanden
    uint8_t addrA;    // Motor A I2C-Adresse (0 wenn nicht vorhanden)
    uint8_t addrB;    // Motor B I2C-Adresse (0 wenn nicht vorhanden)
  } i2cMotor;

  // — LEDs —
  struct {
    uint8_t pinData;       // WS2812 Data Pin
    uint8_t onboardCount;  // Anzahl onboard Pixel
    LedColorOrder colorOrder;  // GRB (Standard) oder RGB
  } led;

  // — Sound —
  struct {
    bool available;  // true → Audio-Hardware vorhanden
    // ESP8266: I2S NoDac, Pins hardware-fixed (GPIO3=Data, GPIO15=BCK, GPIO2=WS)
    // ESP32*: Pins konfigurierbar (in Zukunft hier ergänzen)
  } sound;

  // — Servo —
  struct {
    uint8_t pin1;  // 0 = nicht vorhanden
    uint8_t pin2;  // 0 = nicht vorhanden
  } servo;

  // — Button —
  struct {
    uint8_t pin;
    bool activeLow;  // true → LOW = gedrückt (mit Pull-up)
  } button;

  // — Batterie —
  struct {
    uint8_t pinAdc;              // A0 typisch
    float adcFactor;             // V pro ADC-Digit (Board-spezifisch, Spannungsteiler)
    BatteryType defaultType;     // Standard-Batterietyp für dieses Board
  } battery;

  // — I2C Bus —
  struct {
    uint8_t pinScl;
    uint8_t pinSda;
  } i2c;

  // — Distanz-Sensor (VL53L0X) —
  struct {
    bool connectorAvailable;  // true → 4-pol I2C-Stecker vorhanden
    uint8_t pinXshut;         // 0 = kein XSHUT-Anschluss
  } distance;

  // — Hilfsfunktionen —
  constexpr bool hasGpioMotor() const { return gpioMotor.pinA1 != 0; }
  constexpr bool hasI2CMotor() const { return i2cMotor.available; }
  constexpr bool hasSound() const { return sound.available; }
  constexpr bool hasServo() const { return servo.pin1 != 0; }
  constexpr bool hasDistance() const { return distance.connectorAvailable; }
  constexpr McuCapabilities mcuCaps() const { return mcuForChip(mcu); }

 private:
  static constexpr McuCapabilities mcuForChip(Mcu m) {
    switch (m) {
      case Mcu::Esp8266:  return mcuESP8266();
      case Mcu::Esp32:    return mcuESP32();
      case Mcu::Esp32C6:  return mcuESP32C6();
      case Mcu::Esp32S3:  return mcuESP32S3();
      default:             return mcuESP8266();
    }
  }
};

}  // namespace adora
