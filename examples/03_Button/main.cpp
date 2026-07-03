// 03_Button — Hardware button with multi-level press actions
//
// Demonstrates:
//   - Simple API: onPress() / onLongPress()
//   - Action system: setActions() with timed levels
//   - Preview callback (LED feedback while holding)
//   - Short press callback
//   - holdDurationMs() readout
//
// Hardware: Any Adora board with push button
// Upload:   pio run -e adoramini -t upload
//
// This example uses the Action system. For the simple API,
// replace the setup() block — see comments below.

#include <Arduino.h>
#include <sensor/Button.h>
#include <led/LedController.h>
#include <profile/BoardProfiles.h>

static const auto board = adora::boardAdora26();
static adora::Button button(board);
static adora::LedController led(board);

// ─── Action definitions ───
// actionType is a user-defined ID (0, 1, 2, ...)
// ButtonTrigger::OnPress  = fires as soon as threshold is reached (while held)
// ButtonTrigger::OnRelease = fires on release after threshold was exceeded

static const adora::ButtonAction kActions[] = {
    {2000, 1, adora::ButtonTrigger::OnRelease},   // 2s hold → action 1 (pairing)
    {5000, 2, adora::ButtonTrigger::OnRelease},   // 5s hold → action 2 (reset)
    {8000, 3, adora::ButtonTrigger::OnPress},      // 8s hold → action 3 (factory reset, immediate)
};

// ─── Callbacks ───

void onShortPress() {
    Serial.println(F("Short press!"));
    // Toggle LED
    static bool on = false;
    on = !on;
    if (on) led.setColor(0, 255, 0);
    else    led.off();
}

void onAction(uint8_t actionType, uint8_t cycleCount) {
    Serial.printf("Action fired: type=%d, cycle=%d\n", actionType, cycleCount);

    switch (actionType) {
        case 1:
            Serial.println(F("  -> Pairing mode"));
            led.setBlinkMode(true, 0, 0, 255, 300);  // Blue blink
            break;
        case 2:
            Serial.println(F("  -> Settings reset"));
            led.setBlinkMode(false);
            led.setColor(255, 128, 0);  // Orange confirmation
            break;
        case 3:
            Serial.println(F("  -> FACTORY RESET"));
            led.setBlinkMode(false);
            led.setColor(255, 0, 0);    // Red = danger
            break;
    }
}

void onPreview(uint8_t actionType, uint8_t cycleCount,
               uint8_t* r, uint8_t* g, uint8_t* b) {
    // Called while button is held — set LED color to show which action is pending.
    // Modify r/g/b pointers to change the preview color.
    switch (actionType) {
        case 1: *r = 0;   *g = 0;   *b = 128; break;  // Dim blue
        case 2: *r = 128; *g = 64;  *b = 0;   break;  // Dim orange
        case 3: *r = 128; *g = 0;   *b = 0;   break;  // Dim red
        default: *r = 40; *g = 40;  *b = 40;  break;   // Dim white (no action yet)
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 03_Button ---"));

    led.begin();
    led.setBrightness(60);
    led.setColor(0, 255, 0);  // Green = ready

    button.begin();
    button.setActions(kActions, 3);
    button.onShortPress(onShortPress);
    button.onAction(onAction);
    button.onPreview(onPreview);

    Serial.println(F("Press the button:"));
    Serial.println(F("  Short    -> toggle LED"));
    Serial.println(F("  2s hold  -> pairing (blue blink)"));
    Serial.println(F("  5s hold  -> reset (orange)"));
    Serial.println(F("  8s hold  -> factory reset (red, immediate)"));

    // ─── Alternative: Simple API ───
    // Instead of the Action system above, you can use:
    //   button.begin();
    //   button.onPress([]() { Serial.println("Pressed!"); });
    //   button.onLongPress([]() { Serial.println("Long press!"); }, 2000);
}

void loop() {
    button.update();
    led.update();
}
