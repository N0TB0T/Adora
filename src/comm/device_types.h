#pragma once

#include <stdint.h>

// Ratified device-type table (decision N0TB0T, 2026-07-01) — single source of
// truth for ESP-NOW pairing AND WiFi/JSON discovery. Mirrored in device_types.json.
namespace SharedProtocolContractsV1 {
    static constexpr uint8_t DEVICE_TYPE_MOTE          = 0x01;
    static constexpr uint8_t DEVICE_TYPE_MINI25        = 0x02;
    static constexpr uint8_t DEVICE_TYPE_MAXI          = 0x03;
    static constexpr uint8_t DEVICE_TYPE_MINIMONSTA25  = 0x04;
    static constexpr uint8_t DEVICE_TYPE_MICRO         = 0x05;
    static constexpr uint8_t DEVICE_TYPE_NOTBOTBOT     = 0x06;

    // 0xF0–0xFE reserved for test/placeholder devices (never real hardware).
    static constexpr uint8_t DEVICE_TYPE_DUMMY         = 0xF0;
    static constexpr uint8_t DEVICE_TYPE_UNKNOWN       = 0xFF;
}
