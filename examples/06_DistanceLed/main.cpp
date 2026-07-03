// 06_DistanceLed — VL53L0X distance sensor with LED feedback
//
// Demonstrates:
//   - VL53L0X ToF sensor on I2C (pins from BoardProfile)
//   - Continuous measurement mode (non-blocking)
//   - Distance-to-color mapping via LedController
//   - Proximity alert with blink mode
//
// Hardware: Adora board + VL53L0X on I2C bus (SCL/SDA from board profile)
// Dependencies: Adafruit_VL53L0X (in platformio.ini lib_deps)
// Upload:   pio run -e adoramini -t upload
//
// Note: The distance sensor is NOT part of the adora-device library.
//       This example shows how to combine an external sensor with library LEDs.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <led/LedController.h>
#include <profile/BoardProfiles.h>

static const auto board = adora::boardAdora26();
static adora::LedController led(board);
static Adafruit_VL53L0X tof;

static constexpr uint16_t NEAR_MM = 50;    // Below = blink red
static constexpr uint16_t FAR_MM  = 500;   // Above = LEDs off
static unsigned long lastPrint = 0;

/// Map distance (NEAR..FAR) to color gradient: red → yellow → green
void distanceToColor(uint16_t mm) {
    if (mm < NEAR_MM) {
        // Too close — blink red
        led.setBlinkMode(true, 255, 0, 0, 200);
        return;
    }

    led.setBlinkMode(false);

    if (mm > FAR_MM) {
        led.off();
        return;
    }

    // Linear gradient: near=red, mid=yellow, far=green
    float t = (float)(mm - NEAR_MM) / (FAR_MM - NEAR_MM);  // 0.0 .. 1.0
    uint8_t r = 255 * (1.0f - t);
    uint8_t g = 255 * t;
    led.setColor(r, g, 0);
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 06_DistanceLed ---"));

    // Initialize I2C on board-defined pins
    Wire.begin(board.i2c.sdaPin, board.i2c.sclPin);

    led.begin();
    led.setBrightness(60);

    if (!tof.begin()) {
        Serial.println(F("VL53L0X not found! Check wiring."));
        led.setColor(255, 0, 0);  // Red = error
        while (1) { led.update(); delay(100); }
    }

    // Start continuous ranging — non-blocking, ~30ms per measurement
    tof.startRangeContinuous();

    Serial.println(F("Sensor ready. Distance -> LED color:"));
    Serial.printf("  < %d mm: blink red\n", NEAR_MM);
    Serial.printf("  %d-%d mm: red -> green gradient\n", NEAR_MM, FAR_MM);
    Serial.printf("  > %d mm: off\n", FAR_MM);
}

void loop() {
    if (tof.isRangeComplete()) {
        uint16_t mm = tof.readRange();

        // Ignore out-of-range readings (sensor returns 8190 on timeout)
        if (mm < 8000) {
            distanceToColor(mm);

            if (millis() - lastPrint > 200) {
                lastPrint = millis();
                Serial.printf("%d mm\n", mm);
            }
        }
    }

    led.update();
}
