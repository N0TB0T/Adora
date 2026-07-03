#include "PairingManager.h"

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#else
    #error "PairingManager requires ESP8266 or ESP32"
#endif

// ===== Internal State =====
namespace {
    PairedDeviceInfo s_paired = {};
    PairingStorage s_storage = {};
    DeviceIdentity s_identity = {};
    bool s_initialized = false;

    void loadFromStorage() {
        if (!s_storage.hasPairing || !s_storage.hasPairing()) {
            memset(&s_paired, 0, sizeof(s_paired));
            s_paired.isPaired = false;
            return;
        }

        uint8_t mac[6];
        s_storage.loadMoteMac(mac);

        // Validate MAC (not all zeros)
        bool valid = false;
        for (int i = 0; i < 6; i++) {
            if (mac[i] != 0) { valid = true; break; }
        }

        if (!valid) {
            memset(&s_paired, 0, sizeof(s_paired));
            s_paired.isPaired = false;
            if (s_storage.clearPairing) s_storage.clearPairing();
            return;
        }

        memcpy(s_paired.deviceMac, mac, 6);
        memcpy(s_paired.moteMac, mac, 6);
        s_paired.isPaired = true;
    }

    void saveToStorage() {
        if (s_paired.isPaired) {
            if (s_storage.saveMoteMac) s_storage.saveMoteMac(s_paired.moteMac);
        } else {
            if (s_storage.clearPairing) s_storage.clearPairing();
        }
    }
}

// ===== PairingManager Implementation =====
namespace PairingManager {

    void init(const PairingStorage& storage, const DeviceIdentity& identity) {
        s_storage = storage;
        s_identity = identity;
        loadFromStorage();
        s_initialized = true;
    }

    bool isPaired() {
        if (!s_initialized) return false;
        return s_paired.isPaired;
    }

    const PairedDeviceInfo* getPairedDevice() {
        if (!isPaired()) return nullptr;
        return &s_paired;
    }

    bool processPairingBroadcast(const PairingData* data, const uint8_t* senderMac) {
        if (!data || !senderMac) return false;

        // Verify checksum
        if (!verify_pairing_checksum(data)) return false;

        // Only accept from AdoraMote devices
        if (data->deviceType != DEVICE_ADORAMOTE) return false;

        // If already paired, respond with status but don't re-pair
        if (s_paired.isPaired) {
            return true; // Caller should send response with current status
        }

        // Device is UNPAIRED — accept new pairing
        memcpy(s_paired.deviceMac, senderMac, 6);
        memcpy(s_paired.moteMac, senderMac, 6);
        strncpy(s_paired.deviceName, data->deviceName, sizeof(s_paired.deviceName) - 1);
        s_paired.deviceName[sizeof(s_paired.deviceName) - 1] = '\0';
        s_paired.deviceType = data->deviceType;
        s_paired.channel = data->channel;
        s_paired.isPaired = true;

        saveToStorage();
        return true;
    }

    bool processUnpairCommand(const UnpairCommand* cmd, const uint8_t* senderMac) {
        if (!cmd || !senderMac) return false;
        if (!verify_unpair_checksum(cmd)) return false;
        if (!s_paired.isPaired) return false;

        unpair();
        return true;
    }

    bool buildPairingResponse(PairingData* out, const uint8_t* requestingMoteMac) {
        if (!out) return false;

        out->msgType = MSG_PAIRING;
        out->deviceType = s_identity.deviceType;
        out->protocolVer = PAIRING_PROTOCOL_VERSION;
        out->firmwareVer = (s_identity.firmwareMajor << 4) | (s_identity.firmwareMinor & 0x0F);

        // Own MAC
        uint8_t mac[6];
        WiFi.macAddress(mac);
        memcpy(out->macAddr, mac, 6);

        out->channel = WiFi.channel();

        // Device name
        const char* name = s_identity.deviceName ? s_identity.deviceName : "Adora";
        strncpy(out->deviceName, name, sizeof(out->deviceName) - 1);
        out->deviceName[sizeof(out->deviceName) - 1] = '\0';

        // Pairing status
        if (!s_paired.isPaired) {
            out->pairing_status = STATUS_UNPAIRED;
            memset(out->paired_to_mac, 0, 6);
        } else {
            bool sameMote = (requestingMoteMac &&
                             memcmp(s_paired.deviceMac, requestingMoteMac, 6) == 0);
            if (sameMote) {
                out->pairing_status = STATUS_PAIRED_TO_THIS;
            } else {
                out->pairing_status = STATUS_PAIRED_TO_OTHER;
            }
            memcpy(out->paired_to_mac, s_paired.moteMac, 6);
        }

        out->capability_flags = s_identity.capabilityFlags;
        out->checksum = calculate_pairing_checksum(out);
        return true;
    }

    void unpair() {
        memset(&s_paired, 0, sizeof(s_paired));
        s_paired.isPaired = false;
        saveToStorage();
    }

    void setDeviceName(const char* name) {
        // Update identity for future responses
        s_identity.deviceName = name;
    }
}
