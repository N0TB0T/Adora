#pragma once
#include "../AdoraDeviceTypes.h"
#include "../profile/BoardProfile.h"

class Adafruit_NeoPixel;

namespace adora {

/// NeoPixel-Controller. Pins und Pixelzahl aus BoardProfile.
/// Handhabt automatisch R/G-Swap für LED 1+2 (WS2812B-2020 Noname-Variante).
class LedController {
 public:
  explicit LedController(const BoardProfile& board);
  ~LedController();

  bool begin();

  /// Farbe für alle Onboard-LEDs setzen.
  /// R/G-Swap für LED 1+2 wird automatisch angewendet.
  void setColor(uint8_t r, uint8_t g, uint8_t b);

  /// Farbe mit expliziter Helligkeit (für AdoraMote RGB-Befehle).
  void setColorWithBrightness(uint8_t r, uint8_t g, uint8_t b,
                              uint8_t brightness);

  /// Globale Helligkeit setzen (0-255).
  void setBrightness(uint8_t brightness);
  uint8_t getBrightness() const;

  void off();
  void setNamedColor(LedColor color);

  /// Blink-Modus für Pairing etc.
  void setBlinkMode(bool enabled, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0,
                    uint16_t intervalMs = 500);

  /// Muss in loop() aufgerufen werden (Blink-Timer).
  void update();

  LedState state() const;

  /// Zugriff auf den rohen NeoPixel-Strip (für erweiterte Nutzung).
  Adafruit_NeoPixel* strip();

 private:
  uint8_t pin_;
  uint8_t count_;
  LedColorOrder colorOrder_;
  Adafruit_NeoPixel* strip_;
  LedState state_;

  // Blink
  bool blinkEnabled_;
  uint8_t blinkR_, blinkG_, blinkB_;
  uint16_t blinkIntervalMs_;
  unsigned long lastBlinkToggle_;
  bool blinkState_;

  /// R/G-Swap für LED 1+2 anwenden (WS2812B-2020 Noname).
  void setPixelCorrected(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
  void showStrip();
};

}  // namespace adora
