#include "LedController.h"
#include <Adafruit_NeoPixel.h>

namespace adora {

LedController::LedController(const BoardProfile& board)
    : pin_(board.led.pinData),
      count_(board.led.onboardCount),
      colorOrder_(board.led.colorOrder),
      strip_(nullptr),
      state_{false, 0, 0, 0, 0},
      blinkEnabled_(false),
      blinkR_(0), blinkG_(0), blinkB_(0),
      blinkIntervalMs_(500),
      lastBlinkToggle_(0),
      blinkState_(false) {}

LedController::~LedController() {
  delete strip_;
}

bool LedController::begin() {
  uint16_t pixelCount = count_ > 0 ? count_ : 1;
  uint16_t neoType = (colorOrder_ == LedColorOrder::RGB)
                         ? (NEO_RGB + NEO_KHZ800)
                         : (NEO_GRB + NEO_KHZ800);
  strip_ = new Adafruit_NeoPixel(pixelCount, pin_, neoType);
  strip_->begin();
  strip_->setBrightness(12);  // 5% default
  strip_->show();
  state_.enabled = true;
  state_.brightness = 12;
  return true;
}

void LedController::setColor(uint8_t r, uint8_t g, uint8_t b) {
  state_.r = r;
  state_.g = g;
  state_.b = b;
  state_.enabled = (r != 0 || g != 0 || b != 0);

  if (!state_.enabled) {
    strip_->setBrightness(0);
  } else {
    strip_->setBrightness(state_.brightness);
  }

  for (uint8_t i = 0; i < count_; i++) {
    setPixelCorrected(i, r, g, b);
  }
  showStrip();
}

void LedController::setColorWithBrightness(uint8_t r, uint8_t g, uint8_t b,
                                            uint8_t brightness) {
  state_.r = r;
  state_.g = g;
  state_.b = b;
  state_.brightness = brightness;
  state_.enabled = true;

  for (uint8_t i = 0; i < count_; i++) {
    setPixelCorrected(i, r, g, b);
  }
  strip_->setBrightness(brightness);
  showStrip();
}

void LedController::setBrightness(uint8_t brightness) {
  state_.brightness = brightness;
  strip_->setBrightness(brightness);
  showStrip();
}

uint8_t LedController::getBrightness() const {
  return state_.brightness;
}

void LedController::off() {
  setColor(0, 0, 0);
}

void LedController::setNamedColor(LedColor color) {
  switch (color) {
    case LedColor::Off:     setColor(0, 0, 0);       break;
    case LedColor::White:   setColor(255, 255, 255);  break;
    case LedColor::Red:     setColor(255, 0, 0);      break;
    case LedColor::Green:   setColor(0, 255, 0);      break;
    case LedColor::Blue:    setColor(0, 0, 255);      break;
    case LedColor::Yellow:  setColor(255, 255, 0);    break;
    case LedColor::Magenta: setColor(255, 0, 255);    break;
    case LedColor::Cyan:    setColor(0, 255, 255);    break;
  }
}

void LedController::setBlinkMode(bool enabled, uint8_t r, uint8_t g,
                                  uint8_t b, uint16_t intervalMs) {
  blinkEnabled_ = enabled;
  blinkR_ = r;
  blinkG_ = g;
  blinkB_ = b;
  blinkIntervalMs_ = intervalMs;
  if (!enabled) {
    // Restore last solid color
    setColor(state_.r, state_.g, state_.b);
  }
}

void LedController::update() {
  if (!blinkEnabled_) return;

  unsigned long now = millis();
  if (now - lastBlinkToggle_ >= blinkIntervalMs_) {
    lastBlinkToggle_ = now;
    blinkState_ = !blinkState_;
    if (blinkState_) {
      for (uint8_t i = 0; i < count_; i++) {
        setPixelCorrected(i, blinkR_, blinkG_, blinkB_);
      }
      strip_->setBrightness(state_.brightness);
    } else {
      strip_->setBrightness(0);
    }
    showStrip();
  }
}

LedState LedController::state() const {
  return state_;
}

Adafruit_NeoPixel* LedController::strip() {
  return strip_;
}

void LedController::setPixelCorrected(uint8_t idx, uint8_t r, uint8_t g,
                                       uint8_t b) {
  // LED 0 (Center): SK6812, normales GRB → kein Swap
  // LED 1+2 (Links/Rechts): WS2812B-2020 Noname, R und G vertauscht
  if (idx >= 1 && idx <= 2) {
    strip_->setPixelColor(idx, strip_->Color(g, r, b));  // R↔G Swap
  } else {
    strip_->setPixelColor(idx, strip_->Color(r, g, b));
  }
}

void LedController::showStrip() {
  noInterrupts();
  strip_->show();
  interrupts();
}

}  // namespace adora
