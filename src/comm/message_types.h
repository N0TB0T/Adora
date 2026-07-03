#pragma once

#include <stdint.h>

namespace SharedProtocolContractsV1 {
    static constexpr uint8_t PROTOCOL_VERSION = 0x01;

    static constexpr uint8_t MESSAGE_TYPE_PAIRING = 0xA0;
    static constexpr uint8_t MESSAGE_TYPE_UNPAIR_COMMAND = 0xA1;
    static constexpr uint8_t MESSAGE_TYPE_CONTROL_DATA = 0xD0;

    static constexpr uint8_t MESSAGE_TYPE_JOYSTICK = 0x01;
    static constexpr uint8_t MESSAGE_TYPE_SETTINGS = 0xAD;
    static constexpr uint8_t MESSAGE_TYPE_BATTERY = 0xBA;
    static constexpr uint8_t MESSAGE_TYPE_PING = 0xFE;
    static constexpr uint8_t MESSAGE_TYPE_PONG = 0xFD;
}
