#pragma once

// ===========================
// PAIRING PROTOCOL — Shared packet definitions
// ===========================
// Used by both Mote (sender) and Device (receiver).
// References shared/protocol/contracts/v1/ for single-source-of-truth constants.

#include <stdint.h>

// Shared contracts (build must add -I to shared/protocol/contracts/v1/)
#include "message_types.h"
#include "capability_flags.h"
#include "device_types.h"

// === Message Types (mapped from shared contracts) ===
static constexpr uint8_t MSG_PAIRING       = SharedProtocolContractsV1::MESSAGE_TYPE_PAIRING;
static constexpr uint8_t MSG_UNPAIR_COMMAND = SharedProtocolContractsV1::MESSAGE_TYPE_UNPAIR_COMMAND;
static constexpr uint8_t MSG_DATA           = SharedProtocolContractsV1::MESSAGE_TYPE_CONTROL_DATA;

// === Packet Type Constants ===
static constexpr uint8_t PKT_JOYSTICK = SharedProtocolContractsV1::MESSAGE_TYPE_JOYSTICK;
static constexpr uint8_t PKT_SETTINGS = SharedProtocolContractsV1::MESSAGE_TYPE_SETTINGS;
static constexpr uint8_t PKT_BATTERY  = SharedProtocolContractsV1::MESSAGE_TYPE_BATTERY;
static constexpr uint8_t PKT_PING     = SharedProtocolContractsV1::MESSAGE_TYPE_PING;
static constexpr uint8_t PKT_PONG     = SharedProtocolContractsV1::MESSAGE_TYPE_PONG;

// === Device Types (ratified table 2026-07-01, mapped from shared contracts) ===
enum DeviceType : uint8_t {
    DEVICE_ADORAMOTE         = SharedProtocolContractsV1::DEVICE_TYPE_MOTE,
    DEVICE_ADORAMINI25       = SharedProtocolContractsV1::DEVICE_TYPE_MINI25,
    DEVICE_ADORAMAXI         = SharedProtocolContractsV1::DEVICE_TYPE_MAXI,
    DEVICE_ADORAMINIMONSTA25 = SharedProtocolContractsV1::DEVICE_TYPE_MINIMONSTA25,
    DEVICE_ADORAMICRO        = SharedProtocolContractsV1::DEVICE_TYPE_MICRO,
    DEVICE_NOTBOTBOT         = SharedProtocolContractsV1::DEVICE_TYPE_NOTBOTBOT,
    DEVICE_DUMMY             = SharedProtocolContractsV1::DEVICE_TYPE_DUMMY,
    DEVICE_UNKNOWN           = SharedProtocolContractsV1::DEVICE_TYPE_UNKNOWN
};

// === Capability Flags ===
enum CapabilityFlag : uint16_t {
    CAP_SOUND            = SharedProtocolContractsV1::CAPABILITY_SOUND,
    CAP_LED              = SharedProtocolContractsV1::CAPABILITY_LED,
    CAP_HEADLIGHT        = SharedProtocolContractsV1::CAPABILITY_HEADLIGHT,
    CAP_SETTINGS         = SharedProtocolContractsV1::CAPABILITY_SETTINGS,
    CAP_I2C_MOTOR_DRIVER = SharedProtocolContractsV1::CAPABILITY_I2C_MOTOR_DRIVER
};

// === Protocol Version ===
static constexpr uint8_t PAIRING_PROTOCOL_VERSION = SharedProtocolContractsV1::PROTOCOL_VERSION;

// === Pairing Status ===
enum PairingStatus : uint8_t {
    STATUS_UNPAIRED        = 0x00,
    STATUS_PAIRED_TO_THIS  = 0x01,
    STATUS_PAIRED_TO_OTHER = 0x02
};

// === Helpers ===
// Ratified display names — must match the app's SharedDeviceTypeV1.displayNames
// and the SoftAP scheme "Adora-<name>-<suffix>".
inline const char* device_type_to_string(DeviceType type) {
    switch (type) {
        case DEVICE_ADORAMOTE:         return "Mote";
        case DEVICE_ADORAMINI25:       return "Mini25";
        case DEVICE_ADORAMAXI:         return "Maxi";
        case DEVICE_ADORAMINIMONSTA25: return "MiniMonsta25";
        case DEVICE_ADORAMICRO:        return "Micro";
        case DEVICE_NOTBOTBOT:         return "NotBotBot";
        case DEVICE_DUMMY:             return "Dummy";
        default:                       return "Unknown";
    }
}

#ifdef ARDUINO
#include <Arduino.h>
// Firmware version nibbles to display string (e.g. 0x12 -> "1.2").
inline String firmware_version_to_string(uint8_t ver) {
    return String(ver >> 4) + "." + String(ver & 0x0F);
}
#endif

// === Pairing Data (36 bytes packed) ===
struct __attribute__((packed)) PairingData {
    uint8_t msgType;
    uint8_t deviceType;
    uint8_t protocolVer;
    uint8_t firmwareVer;
    uint8_t macAddr[6];
    uint8_t channel;
    char deviceName[17];
    uint8_t pairing_status;
    uint8_t paired_to_mac[6];
    uint16_t capability_flags;
    uint8_t checksum;
};

// === Unpair Command (8 bytes packed) ===
struct __attribute__((packed)) UnpairCommand {
    uint8_t msgType;
    uint8_t admin_mac[6];
    uint8_t checksum;
};

// === Joystick Data (8 bytes packed) ===
struct __attribute__((packed)) JoystickData {
    uint8_t packetType;       // PKT_JOYSTICK (0x01)
    int16_t x, y;
    uint8_t buttons;          // bits: joy_btn(1), red_btn(2), blue_btn(4)
    uint8_t r, g, b;          // RGB LED values from controller
};

// === Checksum Functions ===
inline uint8_t calculate_pairing_checksum(const PairingData* data) {
    uint8_t cs = 0;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < sizeof(PairingData) - 1; i++) cs ^= bytes[i];
    return cs;
}

inline uint8_t calculate_unpair_checksum(const UnpairCommand* cmd) {
    uint8_t cs = 0;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(cmd);
    for (size_t i = 0; i < sizeof(UnpairCommand) - 1; i++) cs ^= bytes[i];
    return cs;
}

inline bool verify_pairing_checksum(const PairingData* data) {
    return calculate_pairing_checksum(data) == data->checksum;
}

inline bool verify_unpair_checksum(const UnpairCommand* cmd) {
    return calculate_unpair_checksum(cmd) == cmd->checksum;
}

// === Contract Static Asserts ===
static_assert(DEVICE_ADORAMOTE         == 0x01, "device table drift (ratified 2026-07-01)");
static_assert(DEVICE_ADORAMINI25       == 0x02, "device table drift (ratified 2026-07-01)");
static_assert(DEVICE_ADORAMAXI         == 0x03, "device table drift (ratified 2026-07-01)");
static_assert(DEVICE_ADORAMINIMONSTA25 == 0x04, "device table drift (ratified 2026-07-01)");
static_assert(DEVICE_ADORAMICRO        == 0x05, "device table drift (ratified 2026-07-01)");
static_assert(DEVICE_NOTBOTBOT         == 0x06, "device table drift (ratified 2026-07-01)");
static_assert(MSG_PAIRING       == 0xA0, "MSG_PAIRING contract drift");
static_assert(MSG_UNPAIR_COMMAND == 0xA1, "MSG_UNPAIR_COMMAND contract drift");
static_assert(PKT_JOYSTICK      == 0x01, "PKT_JOYSTICK contract drift");
static_assert(PKT_PING          == 0xFE, "PKT_PING contract drift");
static_assert(sizeof(PairingData)   == 38, "PairingData size changed");
static_assert(sizeof(UnpairCommand) == 8,  "UnpairCommand size changed");
static_assert(sizeof(JoystickData)  == 9,  "JoystickData size changed");
