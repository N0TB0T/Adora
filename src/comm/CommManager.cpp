#include "CommManager.h"

namespace {
    ConnectionInfo g_info;
    StateChangeCallback g_callback = nullptr;

    void transition(ConnectionState newState, StateChangeReason reason) {
        ConnectionState old = g_info.state;
        g_info.state = newState;
        if (g_callback) g_callback(old, newState, reason);
    }
}

namespace CommManager {

    void init(uint16_t timeoutMs) {
        g_info = ConnectionInfo();
        g_info.timeoutMs = timeoutMs;
        g_callback = nullptr;
    }

    ConnectionState getState() { return g_info.state; }
    ConnectionProtocol getProtocol() { return g_info.protocol; }
    const ConnectionInfo& getInfo() { return g_info; }

    void setStateCallback(StateChangeCallback callback) { g_callback = callback; }

    void startDiscovery(ConnectionProtocol protocol) {
        g_info.protocol = protocol;
        transition(ConnectionState::DISCOVERING, StateChangeReason::USER_COMMAND);
    }

    void stopDiscovery() {
        transition(ConnectionState::IDLE, StateChangeReason::USER_COMMAND);
    }

    void beginPairing() {
        transition(ConnectionState::PAIRING, StateChangeReason::PAIRING_BROADCAST);
    }

    void completePairing(uint8_t deviceId) {
        g_info.activeDeviceId = deviceId;
        transition(ConnectionState::PAIRED, StateChangeReason::PAIRING_SUCCESS);
    }

    void failPairing(StateChangeReason reason) {
        transition(ConnectionState::DISCOVERING, reason);
    }

    void markConnected() {
        if (g_info.state != ConnectionState::CONNECTED) {
            g_info.connectionStart = millis();
            g_info.connectionCount++;
            transition(ConnectionState::CONNECTED, StateChangeReason::PACKET_RECEIVED);
        }
    }

    void refreshConnection() {
        g_info.lastPacketTime = millis();
    }

    void disconnect(StateChangeReason reason) {
        transition(ConnectionState::DISCONNECTED, reason);
    }

    void reset(StateChangeReason reason) {
        g_info.activeDeviceId = 0;
        g_info.packetsReceived = 0;
        g_info.packetsSent = 0;
        transition(ConnectionState::IDLE, reason);
    }

    bool switchToDevice(uint8_t deviceId) {
        g_info.activeDeviceId = deviceId;
        return true;
    }

    uint8_t getActiveDeviceId() { return g_info.activeDeviceId; }

    bool update() {
        if (g_info.state == ConnectionState::CONNECTED && isTimedOut()) {
            disconnect(StateChangeReason::TIMEOUT);
            return true;
        }
        return false;
    }

    void setTimeout(uint16_t timeoutMs) { g_info.timeoutMs = timeoutMs; }

    unsigned long timeSinceLastPacket() {
        if (g_info.lastPacketTime == 0) return 0;
        return millis() - g_info.lastPacketTime;
    }

    bool isTimedOut() {
        if (g_info.lastPacketTime == 0) return false;
        return timeSinceLastPacket() > g_info.timeoutMs;
    }

    void incrementRxCount() { g_info.packetsReceived++; }
    void incrementTxCount() { g_info.packetsSent++; }
    uint32_t getRxCount() { return g_info.packetsReceived; }
    uint32_t getTxCount() { return g_info.packetsSent; }
    uint32_t getConnectionCount() { return g_info.connectionCount; }

    unsigned long getConnectionDuration() {
        if (g_info.state != ConnectionState::CONNECTED) return 0;
        return millis() - g_info.connectionStart;
    }

    const char* stateToString(ConnectionState state) {
        switch (state) {
            case ConnectionState::IDLE:         return "IDLE";
            case ConnectionState::DISCOVERING:  return "DISCOVERING";
            case ConnectionState::PAIRING:      return "PAIRING";
            case ConnectionState::PAIRED:       return "PAIRED";
            case ConnectionState::CONNECTED:    return "CONNECTED";
            case ConnectionState::DISCONNECTED: return "DISCONNECTED";
            default: return "UNKNOWN";
        }
    }

    const char* protocolToString(ConnectionProtocol protocol) {
        switch (protocol) {
            case ConnectionProtocol::NONE:              return "NONE";
            case ConnectionProtocol::ESPNOW:            return "ESPNOW";
            case ConnectionProtocol::WIFI_APP:          return "WIFI_APP";
            case ConnectionProtocol::BLUETOOTH_CLASSIC: return "BT_CLASSIC";
            case ConnectionProtocol::BLE:               return "BLE";
            default: return "UNKNOWN";
        }
    }
}
