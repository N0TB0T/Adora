#pragma once

#include <stdint.h>

namespace SharedProtocolContractsV1 {
    struct __attribute__((packed)) EnvelopeV1 {
        uint8_t protocolVersion;
        uint8_t messageType;
        uint16_t messageId;
        uint8_t deviceMac[6];
        uint32_t timestampMs;
    };
}
