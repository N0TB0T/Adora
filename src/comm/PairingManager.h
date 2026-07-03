#pragma once

// ===========================
// PAIRING MANAGER — Pairing protocol logic
// ===========================
// Extracted from firmware pairing_manager.h/.cpp.
// Uses PairingStorage abstraction for EEPROM independence.

#include <Arduino.h>
#include "PairingProtocol.h"

// === Paired Device Info ===
struct PairedDeviceInfo {
    uint8_t deviceMac[6];       // MAC of paired mote
    uint8_t moteMac[6];         // MAC of mote (for paired-to-other detection)
    char deviceName[32];
    uint8_t deviceType;
    uint8_t channel;
    bool isPaired;
};

// === Storage Abstraction ===
// Firmware provides these callbacks to map to its EEPROM implementation.
struct PairingStorage {
    bool (*hasPairing)();                        // Check if valid pairing data exists
    void (*loadMoteMac)(uint8_t mac[6]);         // Load paired mote MAC from storage
    void (*saveMoteMac)(const uint8_t mac[6]);   // Save paired mote MAC to storage
    void (*clearPairing)();                      // Clear pairing data from storage
};

// === Device Identity ===
// Set once at init, used to build pairing responses.
struct DeviceIdentity {
    uint8_t deviceType;         // e.g., DEVICE_ADORAMINI25
    uint8_t firmwareMajor;
    uint8_t firmwareMinor;
    const char* deviceName;     // e.g., "Adora26-Mini"
    uint16_t capabilityFlags;   // CAP_SOUND | CAP_LED | ...
};

// === PairingManager API ===
namespace PairingManager {
    // Initialize with storage backend and device identity
    void init(const PairingStorage& storage, const DeviceIdentity& identity);

    // Check if device is currently paired
    bool isPaired();

    // Get paired device info (nullptr if not paired)
    const PairedDeviceInfo* getPairedDevice();

    // Process incoming pairing broadcast from AdoraMote
    // Returns true if response should be sent (always responds with status)
    bool processPairingBroadcast(const PairingData* data, const uint8_t* senderMac);

    // Process incoming unpair command (admin override)
    // Returns true if unpair was accepted
    bool processUnpairCommand(const UnpairCommand* cmd, const uint8_t* senderMac);

    // Build pairing response to send back to AdoraMote
    // Fills responseOut with device info + pairing status
    bool buildPairingResponse(PairingData* responseOut, const uint8_t* requestingMoteMac);

    // Manual unpair (factory reset)
    void unpair();

    // Update device name at runtime
    void setDeviceName(const char* name);
}
