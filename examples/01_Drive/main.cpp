// 01_Drive — Motor control via serial joystick
//
// Demonstrates:
//   - DRV8833Driver initialization from BoardProfile
//   - TankSteering joystick-to-motor math
//   - SteeringConfig tuning (deadzone, curves, damping)
//   - Motor state readout
//
// Hardware: Adora board with DRV8833 + serial monitor
// Upload:   pio run -e adoramini -t upload
// Usage:    Send "x,y" via serial (e.g. "0,512" = forward, "-400,0" = turn left)

#include <Arduino.h>
#include <motor/DRV8833Driver.h>
#include <motor/TankSteering.h>
#include <profile/BoardProfiles.h>

static const auto board = adora::boardAdora26();
static adora::DRV8833Driver motor(board);

// Steering config — tweak to taste
static adora::SteeringConfig steering = {
    .joystickDeadzone = 15,
    .motorDeadzone    = 20,
    .turnDamping      = 0.65f,
    .curve            = adora::ResponseCurve::Quadratic,
    .maxSpeed         = 255,
    .minSpeed         = 30,
    .forwardGain      = 1.0f,
    .turnGain         = 0.7f,
};

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 01_Drive ---"));

    motor.begin();
    motor.setEnabled(true);

    Serial.println(F("Send joystick as: x,y"));
    Serial.println(F("  x = -1024..+1024 (left/right)"));
    Serial.println(F("  y = -1024..+1024 (back/forward)"));
    Serial.println(F("  e.g. '0,800' = forward, '0,0' = stop"));
}

void loop() {
    if (Serial.available()) {
        int16_t x = Serial.parseInt();
        if (Serial.read() == ',') {
            int16_t y = Serial.parseInt();

            int16_t left, right;
            adora::TankSteering::joystickToMotors(x, y, steering, left, right);
            motor.drive(left, right);

            auto st = motor.state();
            Serial.printf("Joy(%d,%d) -> Motor L=%d R=%d\n",
                          x, y, st.leftPower, st.rightPower);
        }
    }
}
