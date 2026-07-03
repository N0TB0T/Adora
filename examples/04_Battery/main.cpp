// 04_Battery — ADC battery monitoring
//
// Demonstrates:
//   - Battery initialization with type selection
//   - Voltage and percent readout
//   - Low-battery detection with custom threshold
//   - Battery type names
//   - LED color based on charge level
//
// Hardware: Any Adora board with battery voltage divider on A0
// Upload:   pio run -e adoramini -t upload

#include <Arduino.h>
#include <sensor/Battery.h>
#include <led/LedController.h>
#include <profile/BoardProfiles.h>

static const auto board = adora::boardAdora26();
static adora::Battery battery(board);
static adora::LedController led(board);

static unsigned long lastPrint = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 04_Battery ---"));

    // Initialize with specific battery type
    battery.begin(adora::BatteryType::Aaa4xNiMH);

    // Optional: set custom low-battery threshold (default: 3.6V)
    battery.setLowThreshold(4.0);

    led.begin();
    led.setBrightness(40);

    Serial.printf("Battery type: %s\n", battery.typeName());
    Serial.println(F("Reading every 2 seconds..."));
    Serial.println(F("Supported types: UsbPower, Aaa4xNiMH, Aaa4xLithium, LiIon1S, LiFePo4_1S"));
}

void loop() {
    battery.update();

    if (millis() - lastPrint > 2000) {
        lastPrint = millis();

        auto st = battery.state();
        Serial.printf("Battery: %.2fV  %d%%  ADC=%d  %s\n",
                       st.voltage, st.percent, st.rawAdc,
                       st.isLow ? "LOW!" : "OK");

        // LED color: green > 50%, yellow 20-50%, red < 20%
        if (st.isLow) {
            led.setColor(255, 0, 0);          // Red = low
        } else if (st.percent < 50) {
            led.setColor(255, 180, 0);        // Yellow = medium
        } else {
            led.setColor(0, 255, 0);          // Green = good
        }
    }

    led.update();
}
