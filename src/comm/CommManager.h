#pragma once

// ===========================
// COMM MANAGER — Connection State Machine
// ===========================
// Protocol-agnostic connection lifecycle management.
// Extracted from firmware connection_state.h + connection_manager.h/.cpp.

#include <Arduino.h>

// === Connection States ===
// IDLE → DISCOVERING → PAIRING → PAIRED ↔ CONNECTED → DISCONNECTED → PAIRED
enum class ConnectionState : uint8_t {
    IDLE,           // No pairing attempt
    DISCOVERING,    // Listening for broadcasts / scanning
    PAIRING,        // Handshake in progress
    PAIRED,         // Device registered, no active data flow
    CONNECTED,      // Active bidirectional communication
    DISCONNECTED    // Connection lost, device still paired
};

// === Connection Protocol ===
enum class ConnectionProtocol : uint8_t {
    NONE,
    ESPNOW,
    WIFI_APP,
    BLUETOOTH_CLASSIC,
    BLE
};

// === State Change Reasons ===
enum class StateChangeReason : uint8_t {
    USER_COMMAND,
    PAIRING_BROADCAST,
    PAIRING_SUCCESS,
    PAIRING_FAILED,
    FIRST_PACKET,
    PACKET_RECEIVED,
    TIMEOUT,
    DISCONNECT_CMD,
    PROTOCOL_ERROR,
    SYSTEM_ERROR
};

// === Connection Info ===
struct ConnectionInfo {
    ConnectionState state          = ConnectionState::IDLE;
    ConnectionProtocol protocol    = ConnectionProtocol::NONE;
    uint8_t activeDeviceId         = 0;
    unsigned long lastPacketTime   = 0;
    unsigned long connectionStart  = 0;
    uint32_t packetsReceived       = 0;
    uint32_t packetsSent           = 0;
    uint32_t connectionCount       = 0;
    uint16_t timeoutMs             = 2000;
};

// === State Change Callback ===
typedef void (*StateChangeCallback)(ConnectionState oldState,
                                    ConnectionState newState,
                                    StateChangeReason reason);

// === Utility ===
inline bool isConnectionActive(ConnectionState s) {
    return s == ConnectionState::CONNECTED;
}

inline bool isDevicePaired(ConnectionState s) {
    return s == ConnectionState::PAIRED ||
           s == ConnectionState::CONNECTED ||
           s == ConnectionState::DISCONNECTED;
}

// === CommManager API ===
namespace CommManager {
    void init(uint16_t timeoutMs = 2000);

    // State queries
    ConnectionState getState();
    ConnectionProtocol getProtocol();
    const ConnectionInfo& getInfo();

    // Callback
    void setStateCallback(StateChangeCallback callback);

    // State transitions
    void startDiscovery(ConnectionProtocol protocol);
    void stopDiscovery();
    void beginPairing();
    void completePairing(uint8_t deviceId = 0);
    void failPairing(StateChangeReason reason = StateChangeReason::PAIRING_FAILED);
    void markConnected();
    void refreshConnection();
    void disconnect(StateChangeReason reason = StateChangeReason::DISCONNECT_CMD);
    void reset(StateChangeReason reason = StateChangeReason::USER_COMMAND);

    // Device switching
    bool switchToDevice(uint8_t deviceId);
    uint8_t getActiveDeviceId();

    // Timeout (call update() in loop)
    bool update();
    void setTimeout(uint16_t timeoutMs);
    unsigned long timeSinceLastPacket();
    bool isTimedOut();

    // Statistics
    void incrementRxCount();
    void incrementTxCount();
    uint32_t getRxCount();
    uint32_t getTxCount();
    uint32_t getConnectionCount();
    unsigned long getConnectionDuration();

    // Utility
    const char* stateToString(ConnectionState state);
    const char* protocolToString(ConnectionProtocol protocol);
}
