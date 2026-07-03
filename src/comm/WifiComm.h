#pragma once

// ===========================
// WIFI COMM — Lightweight WiFi management wrapper
// ===========================
// Basic WiFi STA/AP mode management.
// Complex provisioning (WiFiManager, captive portal, TCP/JSON) stays in firmware.

#include <Arduino.h>

namespace WifiComm {
    // Connect as station (blocks up to timeoutMs)
    bool beginSTA(const char* ssid, const char* password, uint16_t timeoutMs = 10000);

    // Start access point (for provisioning or direct control)
    bool beginAP(const char* ssid, const char* password = nullptr);

    // Disconnect and power down WiFi
    void end();

    // Check if connected (STA mode)
    bool isConnected();

    // Check if AP mode is active
    bool isAPMode();

    // Get local IP as string
    String getIP();

    // Get own MAC address
    void getMac(uint8_t* mac);

    // Reconnect if STA connection lost (call in loop)
    void update();
}
