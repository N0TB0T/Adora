#pragma once

// ===========================
// ESP-NOW COMM — Platform-abstracted ESP-NOW transport
// ===========================
// Wraps ESP8266's <espnow.h> and ESP32's <esp_now.h> behind a unified API.
// Provides raw + typed callbacks for received packets.

#include <Arduino.h>
#include "PairingProtocol.h"

// === Callback Types ===
typedef void (*EspNowRecvCallback)(uint8_t* mac, uint8_t* data, uint8_t len);
typedef void (*JoystickCallback)(const JoystickData* data);
typedef bool (*PingCallback)();
typedef void (*PongCallback)();
typedef void (*PairingRecvCallback)(const PairingData* data, const uint8_t* senderMac);
typedef void (*UnpairRecvCallback)(const UnpairCommand* cmd, const uint8_t* senderMac);
typedef void (*UnknownPacketCallback)(uint8_t packetType, const uint8_t* data, uint8_t len);

// === EspNowComm API ===
namespace EspNowComm {

    // --- Lifecycle ---
    bool begin();
    void end();
    bool isInitialized();

    // --- Peer management ---
    bool addBroadcastPeer();
    void removeBroadcastPeer();
    bool addPeer(const uint8_t* mac, uint8_t channel);
    void removePeer(const uint8_t* mac);
    bool switchPeer(const uint8_t* oldMac, const uint8_t* newMac, uint8_t channel);

    // Set default peer for send()/sendPing()/sendPong()
    void setDefaultPeer(const uint8_t* mac);

    // --- Sending ---
    bool send(const uint8_t* data, uint8_t len);                             // To default peer
    bool sendTo(const uint8_t* mac, const uint8_t* data, uint8_t len);       // To specific peer
    bool sendPing();
    bool sendPong();

    // --- Callbacks ---
    // Raw callback fires for ALL packets (optional, for power users)
    void onReceive(EspNowRecvCallback cb);

    // Typed callbacks (fire after internal routing)
    void setJoystickCallback(JoystickCallback cb);
    void setPingCallback(PingCallback cb);
    void setPongCallback(PongCallback cb);
    void setPairingCallback(PairingRecvCallback cb);
    void setUnpairCallback(UnpairRecvCallback cb);
    void setUnknownCallback(UnknownPacketCallback cb);

    // --- Keepalive ---
    void setKeepaliveEnabled(bool enabled);
    void setKeepaliveInterval(uint16_t ms);
    void updateKeepalive();   // Call in loop

    // --- Platform info ---
    void getOwnMac(uint8_t* mac);
    uint8_t getChannel();
}
