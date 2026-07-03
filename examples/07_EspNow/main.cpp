// 07_EspNow — ESP-NOW communication and pairing
//
// Demonstrates:
//   - EspNowComm initialization
//   - Typed callbacks for joystick, pairing, unpair
//   - PairingManager with storage abstraction
//   - CommManager state machine
//   - Keepalive monitoring
//
// Hardware: Any Adora ESP8266/ESP32 board
// Upload:   pio run -e adoramini -t upload
//
// This example sets up the device as an ESP-NOW receiver.
// It waits for pairing broadcasts from an AdoraMote controller,
// then receives joystick data once paired.

#include <Arduino.h>
#include <EEPROM.h>
#include <led/LedController.h>
#include <comm/EspNowComm.h>
#include <comm/PairingManager.h>
#include <comm/CommManager.h>
#include <profile/BoardProfiles.h>

static const auto board = adora::boardAdora26();
static adora::LedController led(board);

// ─── EEPROM-backed pairing storage ───

static constexpr int EEPROM_ADDR = 0;
static constexpr uint8_t MAGIC = 0xAD;

static bool storagHasPairing() {
    return EEPROM.read(EEPROM_ADDR) == MAGIC;
}

static void storageLoadMac(uint8_t mac[6]) {
    for (int i = 0; i < 6; i++)
        mac[i] = EEPROM.read(EEPROM_ADDR + 1 + i);
}

static void storageSaveMac(const uint8_t mac[6]) {
    EEPROM.write(EEPROM_ADDR, MAGIC);
    for (int i = 0; i < 6; i++)
        EEPROM.write(EEPROM_ADDR + 1 + i, mac[i]);
    EEPROM.commit();
}

static void storageClear() {
    EEPROM.write(EEPROM_ADDR, 0);
    EEPROM.commit();
}

static const PairingStorage storage = {
    .hasPairing  = storagHasPairing,
    .loadMoteMac = storageLoadMac,
    .saveMoteMac = storageSaveMac,
    .clearPairing = storageClear,
};

static const DeviceIdentity identity = {
    .deviceType      = 0x02,
    .firmwareMajor   = 2,
    .firmwareMinor   = 0,
    .deviceName      = "Adora26-Example",
    .capabilityFlags = 0x07,  // LED | SOUND | MOTOR
};

// ─── Callbacks ───

void onJoystick(const JoystickData* data) {
    Serial.printf("Joystick: x=%d y=%d btn=0x%02X\n",
                  data->x, data->y, data->buttons);
    // Normally you'd feed this to TankSteering + motor driver
}

void onPairing(const PairingData* data, const uint8_t* mac) {
    Serial.printf("Pairing request from %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    PairingManager::processPairingBroadcast(data, mac);
}

void onUnpair(const UnpairCommand* cmd, const uint8_t* mac) {
    Serial.println(F("Unpair command received"));
    PairingManager::processUnpairCommand(cmd, mac);
}

void onStateChange(ConnectionState oldState, ConnectionState newState,
                   StateChangeReason reason) {
    Serial.printf("Connection: %d -> %d\n", (int)oldState, (int)newState);

    // LED feedback for connection state
    switch (newState) {
        case ConnectionState::IDLE:
            led.off();
            break;
        case ConnectionState::DISCOVERING:
            led.setBlinkMode(true, 0, 0, 255, 500);  // Blue blink
            break;
        case ConnectionState::PAIRING:
            led.setBlinkMode(true, 255, 255, 0, 200); // Fast yellow
            break;
        case ConnectionState::PAIRED:
            led.setBlinkMode(false);
            led.setColor(0, 255, 0);   // Green = paired
            break;
        case ConnectionState::CONNECTED:
            led.setColor(0, 0, 255);   // Blue = active
            break;
        case ConnectionState::DISCONNECTED:
            led.setBlinkMode(true, 255, 0, 0, 1000);  // Slow red
            break;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 07_EspNow ---"));

    EEPROM.begin(64);
    led.begin();
    led.setBrightness(40);

    // Initialize communication
    EspNowComm::begin();
    EspNowComm::addBroadcastPeer();
    EspNowComm::setJoystickCallback(onJoystick);
    EspNowComm::setPairingCallback(onPairing);
    EspNowComm::setUnpairCallback(onUnpair);
    EspNowComm::setKeepaliveEnabled(true);
    EspNowComm::setKeepaliveInterval(1000);

    // Initialize pairing
    PairingManager::init(storage, identity);

    // Initialize connection state machine
    CommManager::init(2000);  // 2s timeout
    CommManager::onStateChange(onStateChange);

    uint8_t mac[6];
    EspNowComm::getOwnMac(mac);
    Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (PairingManager::isPaired()) {
        Serial.println(F("Already paired — waiting for connection"));
        led.setColor(0, 255, 0);
    } else {
        Serial.println(F("Not paired — waiting for pairing broadcast"));
        led.setBlinkMode(true, 0, 0, 255, 500);
    }
}

void loop() {
    EspNowComm::updateKeepalive();
    CommManager::update();
    led.update();
}
