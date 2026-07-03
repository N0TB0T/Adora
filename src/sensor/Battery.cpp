#include "Battery.h"
#include <Arduino.h>
#include <math.h>

namespace adora {

// Minimum-Intervall zwischen ADC-Reads (ms)
static constexpr unsigned long READ_INTERVAL_MS = 1000;

// Spannungsbereiche pro Batterietyp (min = 0% SoC, max = 100% SoC)
// Reihenfolge MUSS enum BatteryType entsprechen!
struct VoltageRange {
  float minV;
  float maxV;
};

static constexpr VoltageRange VOLTAGE_RANGES[] = {
    {4.8f, 5.2f},   // UsbPower
    {4.0f, 5.4f},   // Aaa4xNiMH     (1.0–1.35V/Zelle)
    {4.0f, 6.4f},   // Aaa4xLithium  (1.0–1.6V/Zelle, Primary)
    {3.0f, 4.2f},   // LiIon1S       (3.0–4.2V)
    {2.5f, 3.65f},  // LiFePo4_1S    (2.5–3.65V)
};

static const char* const TYPE_NAMES[] = {
    "USB 5V",
    "4xAAA NiMH",
    "4xAAA Lithium",
    "Li-Ion 1S",
    "LiFePO4 1S",
};

Battery::Battery(const BoardProfile& board)
    : pin_(board.battery.pinAdc),
      adcFactor_(board.battery.adcFactor),
      state_{0.0f, 0, 0, false, false, board.battery.defaultType},
      lowThreshold_(3.6f),
      lastReadMs_(0) {}

void Battery::begin(BatteryType type) {
  if (type == BatteryType::Aaa4xNiMH) {
    // Default → Auto-Detection
    state_.type = autoDetect();
  } else {
    state_.type = type;
  }
  // Erste Messung sofort
  state_.rawAdc = analogRead(pin_);
  state_.voltage = readVoltage();
  state_.percent = voltageToPercent(state_.voltage);
  state_.isLow = state_.voltage < lowThreshold_ && state_.voltage > 0.5f;
  const auto& r = VOLTAGE_RANGES[static_cast<uint8_t>(state_.type)];
  state_.valid = (state_.voltage >= r.minV - 0.5f) &&
                 (state_.voltage <= r.maxV + 0.5f);
}

void Battery::update() {
  unsigned long now = millis();
  if (now - lastReadMs_ < READ_INTERVAL_MS) return;
  lastReadMs_ = now;

  state_.rawAdc = analogRead(pin_);
  state_.voltage = readVoltage();
  state_.percent = voltageToPercent(state_.voltage);
  state_.isLow = state_.voltage < lowThreshold_ && state_.voltage > 0.5f;
  const auto& r = VOLTAGE_RANGES[static_cast<uint8_t>(state_.type)];
  state_.valid = (state_.voltage >= r.minV - 0.5f) &&
                 (state_.voltage <= r.maxV + 0.5f);
}

BatteryState Battery::state() const { return state_; }

void Battery::setLowThreshold(float voltage) { lowThreshold_ = voltage; }

void Battery::setAdcFactor(float factor) { adcFactor_ = factor; }

void Battery::setType(BatteryType type) { state_.type = type; }

BatteryType Battery::type() const { return state_.type; }

const char* Battery::typeName() const {
  return TYPE_NAMES[static_cast<uint8_t>(state_.type)];
}

float Battery::readVoltage() const {
  if (pin_ == 0 || adcFactor_ <= 0.0f) return 0.0f;
  uint16_t raw = analogRead(pin_);
  return static_cast<float>(raw) * adcFactor_;
}

uint8_t Battery::voltageToPercent(float voltage) const {
  if (state_.type == BatteryType::UsbPower) return 100;

  const auto& r = VOLTAGE_RANGES[static_cast<uint8_t>(state_.type)];
  if (voltage <= r.minV) return 0;
  if (voltage >= r.maxV) return 100;

  float normalized = (voltage - r.minV) / (r.maxV - r.minV);

  switch (state_.type) {
    case BatteryType::Aaa4xNiMH:
      // NiMH: flache Entladekurve — pow(x, 0.8) bildet das besser ab
      return static_cast<uint8_t>(powf(normalized, 0.8f) * 100.0f);
    default:
      // LiIon, LiFePO4, Lithium primary, Alkaline: eher linear
      return static_cast<uint8_t>(normalized * 100.0f);
  }
}

BatteryType Battery::autoDetect() const {
  // 20 Samples mit Varianzanalyse für USB-Erkennung
  constexpr int SAMPLES = 20;
  float voltages[SAMPLES];
  float sum = 0.0f;

  for (int i = 0; i < SAMPLES; i++) {
    uint16_t raw = analogRead(pin_);
    voltages[i] = static_cast<float>(raw) * adcFactor_;
    sum += voltages[i];
    delay(10);
  }

  float avg = sum / SAMPLES;

  float variance = 0.0f;
  for (int i = 0; i < SAMPLES; i++) {
    float diff = voltages[i] - avg;
    variance += diff * diff;
  }
  variance /= SAMPLES;

  // USB: ~5V und sehr stabil (niedrige Varianz)
  if (avg >= 4.75f && avg <= 5.25f) {
    if (variance < 0.01f) return BatteryType::UsbPower;
    return BatteryType::Aaa4xNiMH;  // 5V-Bereich aber instabil → volle NiMH
  }

  // Einzel-Zelle LiFePO4 (2.5–3.65V)
  if (avg >= 2.0f && avg <= 3.7f) return BatteryType::LiFePo4_1S;

  // Einzel-Zelle Li-Ion/LiPo (3.0–4.2V, aber LiFePO4 schon abgefangen)
  if (avg >= 3.7f && avg <= 4.5f) return BatteryType::LiIon1S;

  // 4-Zellen NiMH (4.0–5.5V)
  if (avg >= 4.5f && avg <= 5.5f) return BatteryType::Aaa4xNiMH;

  // 4-Zellen Lithium primary (höhere Spannung, 5.5–6.5V)
  if (avg >= 5.5f && avg <= 6.5f) return BatteryType::Aaa4xLithium;

  // Fallback
  return BatteryType::Aaa4xNiMH;
}

}  // namespace adora
