#pragma once

#include <stdint.h>

// Unified WiFi/JSON packet-type space (transport: UDP discovery / TCP control).
// Adora legacy values preserved; NotBotBot migrated onto this space 2026-07-03
// (formerly controlArm=16, controlLed=17, controlSound=18, sensorTelemetry=48,
// settings=173 in the NBB repo). Mirrored in wifi_packet_types.json.
namespace SharedProtocolContractsV1 {
    static constexpr uint8_t WIFI_PKT_DISCOVERY          = 0x00;
    static constexpr uint8_t WIFI_PKT_DISCOVERY_RESPONSE = 0x01;
    static constexpr uint8_t WIFI_PKT_JOYSTICK           = 0x02;
    static constexpr uint8_t WIFI_PKT_STATUS             = 0x03;
    static constexpr uint8_t WIFI_PKT_SETTINGS           = 0x04;
    static constexpr uint8_t WIFI_PKT_LED                = 0x05;
    static constexpr uint8_t WIFI_PKT_SOUND              = 0x06;
    static constexpr uint8_t WIFI_PKT_CONTROL_ARM        = 0x07;
    static constexpr uint8_t WIFI_PKT_SENSOR_TELEMETRY   = 0x08;
    static constexpr uint8_t WIFI_PKT_PING               = 0xFE;
    static constexpr uint8_t WIFI_PKT_PONG               = 0xFD;
}
