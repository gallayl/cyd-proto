#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include <string>
#include "../config.h"
#include "../FeatureRegistry/Features/Logging.h"

// ensure logger declaration available (guard against include order)

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
    // translate ESP32 wifi_auth_mode_t to human readable strings
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

inline void startStaMode(const std::string &ssid, const std::string &staPassPharse)
{
    if (WiFiClass::getMode() == WIFI_AP && WiFi.begin() != WL_CONNECTED)
    {
        WiFiClass::mode(WIFI_AP_STA);
        WiFi.softAP(ssid.c_str(), staPassPharse.c_str());
    }
}

inline bool hasStoredCredentials()
{
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    return strlen((char *)conf.sta.ssid) > 0;
}

inline void initWifi()
{
    WiFiClass::mode(WIFI_AP);

    if (!hasStoredCredentials())
    {
        loggerInstance->Info("No WiFi credentials saved, starting in AP mode only");
        startStaMode(STA_SSID, STA_PASSPHRASE);
        return;
    }

    WiFi.begin();
    wl_status_t state = (wl_status_t)WiFi.waitForConnectResult();

    if (state != WL_CONNECTED)
    {
        loggerInstance->Error("Failed to connect to access point");
        startStaMode(STA_SSID, STA_PASSPHRASE);
    }
    else
    {
        loggerInstance->Info("Connected to access point");
    }
}