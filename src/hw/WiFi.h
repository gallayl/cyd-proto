#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiClientSecure.h>
#include "../config.h"
#include "../FeatureRegistry/Features/Logging.h"

// ensure logger declaration available (guard against include order)
extern Logger *LoggerInstance;

inline String getSignalStrength(int32_t rssi)
{
    if (rssi > -30)
    {
        return F("Amazing");
    }
    else if (rssi > -67)
    {
        return F("Very good");
    }
    else if (rssi > -70)
    {
        return F("Okay (not good, not terrible)");
    }
    else if (rssi > -80)
    {
        return F("Not good");
    }
    else if (rssi > -90)
    {
        return F("Unusable");
    }
    return F("Unknown");
}

inline String getEncryptionType(wifi_auth_mode_t mode)
{
    // translate ESP32 wifi_auth_mode_t to human readable strings
    switch (mode)
    {
    case WIFI_AUTH_OPEN:
        return F("OPEN");
    case WIFI_AUTH_WEP:
        return F("WEP");
    case WIFI_AUTH_WPA_PSK:
        return F("WPA_PSK");
    case WIFI_AUTH_WPA2_PSK:
        return F("WPA2_PSK");
    case WIFI_AUTH_WPA_WPA2_PSK:
        return F("WPA_WPA2_PSK");
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return F("WPA2_ENTERPRISE");
    case WIFI_AUTH_WPA3_PSK:
        return F("WPA3_PSK");
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return F("WPA2_WPA3_PSK");
    default:
        return F("UNKNOWN");
    }
}

inline void startStaMode(String ssid, String staPassPharse)
{
    if (WiFi.getMode() == WIFI_AP && WiFi.begin() != WL_CONNECTED)
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ssid, staPassPharse);
    }
}

void initWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.begin();
    wl_status_t state = (wl_status_t)WiFi.waitForConnectResult();

    if (state != WL_CONNECTED)
    {
        LoggerInstance->Error(F("Failed to connect to access point"));
        startStaMode(STA_SSID, STA_PASSPHARSE);
    }
    else
    {
        LoggerInstance->Info(F("Connected to access point"));
    }
}