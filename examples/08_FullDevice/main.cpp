// 08_FullDevice — AdoraDevice facade with all features
//
// Demonstrates:
//   - AdoraDevice as single entry point
//   - Board profile selection
//   - Motor control via handleJoystick()
//   - LED feedback for driving direction
//   - Battery monitoring with low-battery warning
//   - Button with multi-level actions
//   - Sound playback (if ADORA_HAS_SOUND is defined)
//   - SteeringConfig tuning
//
// Hardware: Adora26 board (ESP8266 + DRV8833 + 3× WS2812 + button + battery)
// Upload:   pio run -e adoramini -t upload

#define ADORA_HAS_SOUND

#include <Arduino.h>
#include <AdoraDevice.h>

adora::AdoraDevice device(adora::boardAdora26());

// ─── Button actions ───

static const adora::ButtonAction kActions[] = {
    {2000, 1, adora::ButtonTrigger::OnRelease},   // 2s → toggle mute
    {5000, 2, adora::ButtonTrigger::OnRelease},   // 5s → stop motors
};

void onShortPress() {
    Serial.println(F("Short press — honk!"));
    #ifdef ADORA_HAS_SOUND
    device.sound().play(adora::SoundEvent::Horn);
    #endif
}

void onAction(uint8_t actionType, uint8_t cycleCount) {
    switch (actionType) {
        case 1:
            Serial.println(F("Toggle mute"));
            #ifdef ADORA_HAS_SOUND
            if (device.sound().getMuteMode() == adora::MuteMode::Off)
                device.sound().setMuteMode(adora::MuteMode::All);
            else
                device.sound().setMuteMode(adora::MuteMode::Off);
            #endif
            break;
        case 2:
            Serial.println(F("Emergency stop"));
            device.brake();
            device.led().setColor(255, 0, 0);
            break;
    }
}

void onPreview(uint8_t actionType, uint8_t cycleCount,
               uint8_t* r, uint8_t* g, uint8_t* b) {
    switch (actionType) {
        case 1: *r = 0;   *g = 128; *b = 128; break;  // Cyan = mute
        case 2: *r = 128; *g = 0;   *b = 0;   break;  // Red = stop
        default: *r = 40; *g = 40;  *b = 40;  break;
    }
}

// ─── Battery check (every 5s) ───

static unsigned long lastBatteryCheck = 0;

void checkBattery() {
    if (millis() - lastBatteryCheck < 5000) return;
    lastBatteryCheck = millis();

    auto bat = device.battery().state();
    if (bat.isLow) {
        Serial.printf("LOW BATTERY: %.2fV (%d%%)\n", bat.voltage, bat.percent);
        #ifdef ADORA_HAS_SOUND
        device.sound().play(adora::SoundEvent::LowBattery);
        #endif
    }
}

// ─── LED feedback based on motor state ───

void updateDriveLed() {
    auto m = device.motorState();

    if (m.leftPower == 0 && m.rightPower == 0) {
        device.led().setColor(20, 20, 20);        // Dim white = idle
    } else if (m.leftPower > 0 && m.rightPower > 0) {
        device.led().setColor(0, 255, 0);          // Green = forward
    } else if (m.leftPower < 0 && m.rightPower < 0) {
        device.led().setColor(255, 0, 0);          // Red = reverse
    } else {
        device.led().setColor(0, 0, 255);          // Blue = turning
    }
}

// ─── Setup & Loop ───

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 08_FullDevice ---"));

    device.begin();
    device.led().setBrightness(40);
    device.led().setColor(0, 255, 0);

    // Steering tuning
    adora::SteeringConfig cfg;
    cfg.curve = adora::ResponseCurve::Quadratic;
    cfg.turnDamping = 0.6f;
    cfg.maxSpeed = 200;
    device.setSteeringConfig(cfg);

    // Button setup
    device.button().setActions(kActions, 2);
    device.button().onShortPress(onShortPress);
    device.button().onAction(onAction);
    device.button().onPreview(onPreview);

    Serial.printf("Board: %s\n", device.board().name);
    Serial.printf("Battery: %.2fV (%s)\n",
                  device.battery().state().voltage,
                  device.battery().typeName());
    Serial.println(F("Send joystick: x,y  (e.g. '0,512')"));
}

void loop() {
    device.update();

    // Read joystick from serial
    if (Serial.available()) {
        int16_t x = Serial.parseInt();
        if (Serial.read() == ',') {
            int16_t y = Serial.parseInt();
            device.handleJoystick(x, y);
            updateDriveLed();
        }
    }

    checkBattery();
    device.led().update();
}
