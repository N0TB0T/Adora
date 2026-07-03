#pragma once

#include <stdint.h>

namespace SharedProtocolContractsV1 {
    static constexpr uint16_t CAPABILITY_SOUND = 0x0001;
    static constexpr uint16_t CAPABILITY_LED = 0x0002;
    static constexpr uint16_t CAPABILITY_HEADLIGHT = 0x0004;
    static constexpr uint16_t CAPABILITY_SETTINGS = 0x0008;
    static constexpr uint16_t CAPABILITY_I2C_MOTOR_DRIVER = 0x0010;
    // Unified flag space since 2026-07-03 — NotBotBot capabilities reassigned to
    // free bits (formerly nb::capability with colliding 0x0004..0x0040 values).
    static constexpr uint16_t CAPABILITY_ULTRASONIC = 0x0020;
    static constexpr uint16_t CAPABILITY_BUTTON = 0x0040;
    static constexpr uint16_t CAPABILITY_SERVO_ARM = 0x0080;
    static constexpr uint16_t CAPABILITY_MICROPHONE = 0x0100;
    static constexpr uint16_t CAPABILITY_WIFI_PROVISIONING = 0x0200;
}
