#pragma once

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <cstring>
#include <string>
#include "../config.h"
#include "../FeatureRegistry/Features/Logging.h"

// --- Utility functions (both platforms) ---

inline std::string getSignalStrength(int32_t rssi)
{
    if (rssi > -30)
    {
        return "Amazing";
    }
    else if (rssi > -67)
    {
        return "Very good";
    }
    else if (rssi > -70)
    {
        return "Okay (not good, not terrible)";
    }
    else if (rssi > -80)
    {
        return "Not good";
    }
    else if (rssi > -90)
    {
        return "Unusable";
    }
    return "Unknown";
}

inline std::string getEncryptionType(wifi_auth_mode_t mode)
{
    switch (mode)
    {
    case WIFI_AUTH_OPEN:
        return "OPEN";
    case WIFI_AUTH_WEP:
        return "WEP";
    case WIFI_AUTH_WPA_PSK:
        return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
        return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2_WPA3_PSK";
    default:
        return "UNKNOWN";
    }
}

// --- ESP-IDF WiFi API (implemented in WiFi.cpp) ---

void initWifi();
void startStaMode(const std::string &ssid, const std::string &passphrase);
bool hasStoredCredentials();

// State accessors
esp_netif_t *getWifiStaNetif();
esp_netif_t *getWifiApNetif();
bool isWifiStaConnected();

// Helper utilities
inline std::string ipToString(const esp_ip4_addr_t &ip)
{
    char buf[16];
    esp_ip4addr_ntoa(&ip, buf, sizeof(buf));
    return buf;
}

inline std::string macToString(const uint8_t mac[6])
{
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}
