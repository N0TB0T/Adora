#include "WifiComm.h"

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#else
    #error "WifiComm requires ESP8266 or ESP32"
#endif

namespace {
    bool s_apMode = false;
    char s_ssid[64] = {};
    char s_password[64] = {};
}

namespace WifiComm {

    bool beginSTA(const char* ssid, const char* password, uint16_t timeoutMs) {
        s_apMode = false;
        if (ssid) strncpy(s_ssid, ssid, sizeof(s_ssid) - 1);
        if (password) strncpy(s_password, password, sizeof(s_password) - 1);

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
            delay(100);
        }

        return WiFi.status() == WL_CONNECTED;
    }

    bool beginAP(const char* ssid, const char* password) {
        s_apMode = true;
        WiFi.mode(WIFI_AP);

        if (password && strlen(password) >= 8) {
            return WiFi.softAP(ssid, password);
        }
        return WiFi.softAP(ssid);
    }

    void end() {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        s_apMode = false;
    }

    bool isConnected() {
        return !s_apMode && WiFi.status() == WL_CONNECTED;
    }

    bool isAPMode() {
        return s_apMode;
    }

    String getIP() {
        if (s_apMode) {
            return WiFi.softAPIP().toString();
        }
        return WiFi.localIP().toString();
    }

    void getMac(uint8_t* mac) {
        if (mac) WiFi.macAddress(mac);
    }

    void update() {
        if (s_apMode) return;
        // Auto-reconnect if credentials are stored and connection dropped
        if (WiFi.status() != WL_CONNECTED && s_ssid[0] != '\0') {
            WiFi.begin(s_ssid, s_password);
        }
    }
}
