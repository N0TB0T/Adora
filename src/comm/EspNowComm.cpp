#include "EspNowComm.h"

// ===========================
// Platform-specific includes
// ===========================
#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <espnow.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <esp_now.h>
    #include <esp_idf_version.h>
#else
    #error "EspNowComm requires ESP8266 or ESP32"
#endif

// ===== Internal State =====
namespace {
    bool s_initialized = false;

    // Default peer MAC for send()
    uint8_t s_defaultPeer[6] = {0};
    bool s_hasDefaultPeer = false;

    // Keepalive
    bool s_keepaliveEnabled = true;
    uint16_t s_keepaliveIntervalMs = 1000;
    unsigned long s_lastPingTime = 0;

    // Callbacks
    EspNowRecvCallback   s_rawCallback      = nullptr;
    JoystickCallback     s_joystickCb       = nullptr;
    PingCallback         s_pingCb           = nullptr;
    PongCallback         s_pongCb           = nullptr;
    PairingRecvCallback  s_pairingCb        = nullptr;
    UnpairRecvCallback   s_unpairCb         = nullptr;
    UnknownPacketCallback s_unknownCb       = nullptr;

    // --- Internal receive handler ---
    void handleReceive(uint8_t* mac, uint8_t* data, uint8_t len) {
        // Fire raw callback first (if set)
        if (s_rawCallback) {
            s_rawCallback(mac, data, len);
        }

        if (len < 1) return;
        uint8_t pktType = data[0];

        // Unpair command
        if (pktType == MSG_UNPAIR_COMMAND && len >= sizeof(UnpairCommand)) {
            if (s_unpairCb) {
                s_unpairCb(reinterpret_cast<const UnpairCommand*>(data), mac);
            }
            return;
        }

        // Pairing broadcast
        if (pktType == MSG_PAIRING && len >= sizeof(PairingData)) {
            if (s_pairingCb) {
                s_pairingCb(reinterpret_cast<const PairingData*>(data), mac);
            }
            return;
        }

        // Ping
        if (pktType == PKT_PING) {
            bool shouldPong = true;
            if (s_pingCb) shouldPong = s_pingCb();
            if (shouldPong) EspNowComm::sendPong();
            return;
        }

        // Pong
        if (pktType == PKT_PONG) {
            if (s_pongCb) s_pongCb();
            return;
        }

        // Joystick
        if (pktType == PKT_JOYSTICK && len >= sizeof(JoystickData)) {
            if (s_joystickCb) {
                s_joystickCb(reinterpret_cast<const JoystickData*>(data));
            }
            return;
        }

        // Unknown
        if (s_unknownCb) {
            s_unknownCb(pktType, data, len);
        }
    }

// ===========================
// ESP8266 platform callbacks
// ===========================
#if defined(ESP8266)
    void espnow_recv_cb_esp8266(uint8_t* mac, uint8_t* data, uint8_t len) {
        handleReceive(mac, data, len);
    }

    void espnow_send_cb_esp8266(uint8_t* /*mac*/, uint8_t /*status*/) {
        // Could track send success/failure here
    }
#endif

// ===========================
// ESP32 platform callbacks
// ===========================
#if defined(ESP32)
    // ESP-IDF 5.x changed the receive callback signature
    #if ESP_IDF_VERSION_MAJOR >= 5
    void espnow_recv_cb_esp32(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
        if (len <= 0 || len > 250) return;
        // Cast away const for unified internal handler
        handleReceive(const_cast<uint8_t*>(info->src_addr),
                      const_cast<uint8_t*>(data),
                      static_cast<uint8_t>(len));
    }
    #else
    void espnow_recv_cb_esp32(const uint8_t* mac, const uint8_t* data, int len) {
        if (len <= 0 || len > 250) return;
        handleReceive(const_cast<uint8_t*>(mac),
                      const_cast<uint8_t*>(data),
                      static_cast<uint8_t>(len));
    }
    #endif

    void espnow_send_cb_esp32(const uint8_t* /*mac*/, esp_now_send_status_t /*status*/) {
        // Could track send success/failure here
    }
#endif

} // anonymous namespace

// ===== EspNowComm Implementation =====
namespace EspNowComm {

    bool begin() {
        if (s_initialized) return true;

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        #if defined(ESP8266)
            if (esp_now_init() != 0) return false;
            esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
            esp_now_register_recv_cb(espnow_recv_cb_esp8266);
            esp_now_register_send_cb(espnow_send_cb_esp8266);
        #elif defined(ESP32)
            if (esp_now_init() != ESP_OK) return false;
            esp_now_register_recv_cb(espnow_recv_cb_esp32);
            esp_now_register_send_cb(espnow_send_cb_esp32);
        #endif

        s_initialized = true;
        s_lastPingTime = millis();
        return true;
    }

    void end() {
        if (!s_initialized) return;
        #if defined(ESP8266)
            esp_now_unregister_recv_cb();
            esp_now_unregister_send_cb();
        #elif defined(ESP32)
            esp_now_unregister_recv_cb();
            esp_now_unregister_send_cb();
        #endif
        esp_now_deinit();
        s_initialized = false;
    }

    bool isInitialized() { return s_initialized; }

    // --- Peer Management ---
    bool addBroadcastPeer() {
        if (!s_initialized) return false;
        uint8_t bc[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        #if defined(ESP8266)
            if (esp_now_is_peer_exist(bc)) esp_now_del_peer(bc);
            return esp_now_add_peer(bc, ESP_NOW_ROLE_COMBO, 0, NULL, 0) == 0;
        #elif defined(ESP32)
            esp_now_peer_info_t peer = {};
            memcpy(peer.peer_addr, bc, 6);
            peer.channel = 0;
            peer.encrypt = false;
            if (esp_now_is_peer_exist(bc)) esp_now_del_peer(bc);
            return esp_now_add_peer(&peer) == ESP_OK;
        #endif
    }

    void removeBroadcastPeer() {
        uint8_t bc[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if (esp_now_is_peer_exist(bc)) esp_now_del_peer(bc);
    }

    bool addPeer(const uint8_t* mac, uint8_t channel) {
        if (!s_initialized || !mac) return false;
        uint8_t m[6];
        memcpy(m, mac, 6);

        #if defined(ESP8266)
            if (esp_now_is_peer_exist(m)) esp_now_del_peer(m);
            return esp_now_add_peer(m, ESP_NOW_ROLE_COMBO, channel, NULL, 0) == 0;
        #elif defined(ESP32)
            esp_now_peer_info_t peer = {};
            memcpy(peer.peer_addr, mac, 6);
            peer.channel = channel;
            peer.encrypt = false;
            if (esp_now_is_peer_exist(m)) esp_now_del_peer(m);
            return esp_now_add_peer(&peer) == ESP_OK;
        #endif
    }

    void removePeer(const uint8_t* mac) {
        if (!mac) return;
        uint8_t m[6];
        memcpy(m, mac, 6);
        if (esp_now_is_peer_exist(m)) esp_now_del_peer(m);
    }

    bool switchPeer(const uint8_t* oldMac, const uint8_t* newMac, uint8_t channel) {
        if (!s_initialized || !newMac) return false;
        if (oldMac) removePeer(oldMac);
        return addPeer(newMac, channel);
    }

    void setDefaultPeer(const uint8_t* mac) {
        if (mac) {
            memcpy(s_defaultPeer, mac, 6);
            s_hasDefaultPeer = true;
        } else {
            s_hasDefaultPeer = false;
        }
    }

    // --- Sending ---
    bool sendTo(const uint8_t* mac, const uint8_t* data, uint8_t len) {
        if (!s_initialized || !mac) return false;
        uint8_t m[6];
        memcpy(m, mac, 6);
        return esp_now_send(m, const_cast<uint8_t*>(data), len) == 0;
    }

    bool send(const uint8_t* data, uint8_t len) {
        if (!s_hasDefaultPeer) return false;
        return sendTo(s_defaultPeer, data, len);
    }

    bool sendPing() {
        uint8_t pkt = PKT_PING;
        return send(&pkt, 1);
    }

    bool sendPong() {
        uint8_t pkt = PKT_PONG;
        return send(&pkt, 1);
    }

    // --- Callbacks ---
    void onReceive(EspNowRecvCallback cb) { s_rawCallback = cb; }
    void setJoystickCallback(JoystickCallback cb) { s_joystickCb = cb; }
    void setPingCallback(PingCallback cb) { s_pingCb = cb; }
    void setPongCallback(PongCallback cb) { s_pongCb = cb; }
    void setPairingCallback(PairingRecvCallback cb) { s_pairingCb = cb; }
    void setUnpairCallback(UnpairRecvCallback cb) { s_unpairCb = cb; }
    void setUnknownCallback(UnknownPacketCallback cb) { s_unknownCb = cb; }

    // --- Keepalive ---
    void setKeepaliveEnabled(bool enabled) { s_keepaliveEnabled = enabled; }
    void setKeepaliveInterval(uint16_t ms) { s_keepaliveIntervalMs = ms; }

    void updateKeepalive() {
        if (!s_keepaliveEnabled || !s_hasDefaultPeer) return;
        unsigned long now = millis();
        if (now - s_lastPingTime >= s_keepaliveIntervalMs) {
            s_lastPingTime = now;
            sendPing();
        }
    }

    // --- Platform Info ---
    void getOwnMac(uint8_t* mac) {
        if (mac) WiFi.macAddress(mac);
    }

    uint8_t getChannel() {
        return WiFi.channel();
    }
}
