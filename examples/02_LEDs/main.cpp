// 02_LEDs — LED control patterns and colors
//
// Demonstrates:
//   - LedController from BoardProfile
//   - setColor() for all LEDs at once
//   - setNamedColor() with LedColor enum
//   - setBrightness()
//   - setBlinkMode() for pairing-style blink
//   - Per-pixel access via strip() for individual LED control
//   - off() to turn all LEDs off
//
// Hardware: Any Adora board with WS2812 LEDs
// Upload:   pio run -e adoramini -t upload

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <led/LedController.h>
#include <profile/BoardProfiles.h>

static const auto board = adora::boardAdora26();
static adora::LedController led(board);

// Demo phases — each runs for a few seconds
enum Phase {
    NAMED_COLORS,       // Cycle through named colors
    RGB_GRADIENT,       // Smooth RGB transitions
    BRIGHTNESS_RAMP,    // Fade brightness up/down
    BLINK_MODE,         // Pairing-style blink
    PER_PIXEL,          // Individual pixel colors
    PHASE_COUNT
};

static Phase phase = NAMED_COLORS;
static unsigned long phaseStart = 0;
static constexpr unsigned long PHASE_DURATION = 4000;

void runNamedColors() {
    // Cycle through built-in color names every 500ms
    static const adora::LedColor colors[] = {
        adora::LedColor::Red,
        adora::LedColor::Green,
        adora::LedColor::Blue,
        adora::LedColor::Yellow,
        adora::LedColor::Magenta,
        adora::LedColor::Cyan,
        adora::LedColor::White,
    };
    uint8_t idx = (millis() / 500) % 7;
    led.setNamedColor(colors[idx]);
}

void runRgbGradient() {
    // Smooth hue rotation using sin()
    float t = millis() / 1000.0f;
    uint8_t r = 127 + 127 * sin(t * 2.0f);
    uint8_t g = 127 + 127 * sin(t * 2.0f + 2.094f);
    uint8_t b = 127 + 127 * sin(t * 2.0f + 4.189f);
    led.setColor(r, g, b);
}

void runBrightnessRamp() {
    // Triangle wave brightness 10..200
    uint8_t val = (millis() / 10) % 400;
    if (val > 200) val = 400 - val;
    val = max((uint8_t)10, val);
    led.setColor(0, 200, 255);
    led.setBrightness(val);
}

void runBlinkMode() {
    // Pairing-style blue blink at 300ms interval
    led.setBrightness(80);
    led.setBlinkMode(true, 0, 0, 255, 300);
}

void runPerPixel() {
    // Each pixel a different color (Adora26 has 3 pixels)
    led.setBlinkMode(false);
    led.setBrightness(60);

    Adafruit_NeoPixel* s = led.strip();
    if (!s) return;

    uint16_t n = s->numPixels();
    // Rotate colors across pixels
    uint8_t offset = (millis() / 300) % 3;
    uint32_t palette[] = {
        s->Color(255, 0, 0),   // Red
        s->Color(0, 255, 0),   // Green
        s->Color(0, 0, 255),   // Blue
    };
    for (uint16_t i = 0; i < n; i++) {
        s->setPixelColor(i, palette[(i + offset) % 3]);
    }
    s->show();
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 02_LEDs ---"));

    led.begin();
    led.setBrightness(60);
    phaseStart = millis();

    Serial.println(F("Cycling through LED demos (4s each):"));
    Serial.println(F("  1) Named colors   2) RGB gradient"));
    Serial.println(F("  3) Brightness     4) Blink mode"));
    Serial.println(F("  5) Per-pixel"));
}

void loop() {
    // Advance phase every PHASE_DURATION ms
    if (millis() - phaseStart > PHASE_DURATION) {
        phaseStart = millis();
        phase = static_cast<Phase>((phase + 1) % PHASE_COUNT);
        // Reset state between phases
        led.setBlinkMode(false);
        led.setBrightness(60);
        Serial.printf("Phase %d\n", phase);
    }

    switch (phase) {
        case NAMED_COLORS:    runNamedColors();    break;
        case RGB_GRADIENT:    runRgbGradient();    break;
        case BRIGHTNESS_RAMP: runBrightnessRamp(); break;
        case BLINK_MODE:      runBlinkMode();      break;
        case PER_PIXEL:       runPerPixel();       break;
        default: break;
    }

    led.update();  // Required for blink timer
    delay(20);
}
