#include "SystemFeatures.h"
#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../hw/RgbLed.h"
#include "../../hw/lightSensor.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

#if ENABLE_WIFI
#include <WiFi.h>
#include <IPAddress.h>
#include "../../hw/WiFi.h"
#endif

// --- Helper functions ---

JsonDocument getInfo()
{
    JsonDocument response;
    JsonObject esp = response["esp"].to<JsonObject>();

    esp["sdkVersion"] = ESP.getSdkVersion();
    esp["cpuFreqMhz"] = ESP.getCpuFreqMHz();
    esp["freeHeap"] = ESP.getFreeHeap();
    esp["freeSkSpace"] = ESP.getFreeSketchSpace();

    JsonObject flash = response["flash"].to<JsonObject>();

    flash["mode"] = ESP.getFlashChipMode();
    flash["size"] = ESP.getFlashChipSize();
    flash["speed"] = ESP.getFlashChipSpeed();

    JsonObject fs = response["fs"].to<JsonObject>();
    fs["totalBytes"] = LittleFS.totalBytes();
    fs["usedBytes"] = LittleFS.usedBytes();

    return response;
}

// --- WiFi handler ---

#if ENABLE_WIFI
static String wifiHandler(const String &command)
{
    String operation = CommandParser::GetCommandParameter(command, 1);
    if (!operation.compareTo("connect"))
    {
        String ssid = CommandParser::GetCommandParameter(command, 2);
        String password = CommandParser::GetCommandParameter(command, 3);
        if (ssid.length() < 3 || password.length() < 5)
        {
            return String("{\"error\": \"ssid or password too short\"}");
        }
        WiFi.disconnect(true, false);
        WiFi.persistent(true);
        WiFi.mode(WIFI_AP);
        WiFi.begin(ssid.c_str(), password.c_str());
        return String("{\"event\": \"connecting\"}");
    }
    if (!operation.compareTo("list"))
    {
        JsonDocument response;
        JsonArray arr = response.to<JsonArray>();

        int n = WiFi.scanNetworks();

        for (int i = 0; i < n; ++i)
        {
            JsonObject element = arr.add<JsonObject>();
            element["ssid"] = WiFi.SSID(i);
            element["rssi"] = WiFi.RSSI(i);
            element["rssiText"] = getSignalStrength(WiFi.RSSI(i));
            element["encryption"] = getEncryptionType(WiFi.encryptionType(i));
        }
        String output;
        serializeJson(response, output);
        return output;
    }
    if (!operation.compareTo("startSTA"))
    {
        String ssid = CommandParser::GetCommandParameter(command, 2);
        String passphrase = CommandParser::GetCommandParameter(command, 3);
        if (ssid.length() < 3 || passphrase.length() < 5)
        {
            return String("{\"error\": \"ssid or passphrase too short\"}");
        }
        startStaMode(ssid, passphrase);
        return String("{\"event\": \"starting STA\"}");
    }

    if (!operation.compareTo("stopSTA"))
    {
        bool success = WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_AP);
        WiFi.begin();
        return String("{\"event\": \"stopSTA\", \"success\": " + String(success) + "}");
    }

    if (!operation.compareTo("info"))
    {
        JsonDocument response;
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
        {
            JsonObject ap = response["ap"].to<JsonObject>();
            ap["ipAddress"] = WiFi.softAPIP().toString();
            ap["macAddress"] = WiFi.softAPmacAddress();
        }

        if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA)
        {
            JsonObject sta = response["sta"].to<JsonObject>();
            sta["ipAddress"] = WiFi.localIP().toString();
            sta["macAddress"] = WiFi.macAddress();
            sta["ssid"] = WiFi.SSID();
        }

        int32_t rssi = WiFi.RSSI();
        response["wifiStrength"] = getSignalStrength(rssi);
        response["wifiRssiDb"] = rssi;

        String output;
        serializeJson(response, output);
        return output;
    }

    if (!operation.compareTo("restart"))
    {
        WiFi.disconnect(true, false);
        WiFi.begin();
        return String("{\"event\": \"disconnecting\"}");
    }
    return String("{\"event\": \"Unknown WiFi operation command. The available commands are: info, list, connect <ssid> <password>, startSTA <ssid> <passphrase>, stopSTA\"}");
}
#endif

// --- Action definitions ---

FeatureAction restartAction = {
    .name = "restart",
    .type = "POST",
    .handler = [](const String &command)
    {
        delay(100);
        ESP.restart();
        return String("{\"event\": \"restart\"}");
    },
    .transports = {.cli = true, .rest = true, .ws = false, .scripting = true}};

FeatureAction featuresAction = {
    .name = "features",
    .handler = [](const String &command)
    {
        String output;
        serializeJson(registeredFeatures, output);
        return output;
    },
    .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

FeatureAction infoAction = {
    .name = "info",
    .handler = [](const String &command)
    {
        JsonDocument response = getInfo();
        String output;
        serializeJson(response, output);
        return output;
    },
    .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

FeatureAction rgbLedAction = {
    .name = "rgbLed",
    .handler = [](const String &command)
    {
        const String sub = CommandParser::GetCommandParameter(command, 1);
        if (sub == "setColor")
        {
            int r = CommandParser::GetCommandParameter(command, 2).toInt();
            int g = CommandParser::GetCommandParameter(command, 3).toInt();
            int b = CommandParser::GetCommandParameter(command, 4).toInt();
            setRgbLedColor(r, g, b);
            LoggerInstance->Info("Set RGB LED color to " + String(r) + "," + String(g) + "," + String(b));
        }
        else if (sub == "off")
        {
            setRgbLedColor(0, 0, 0);
            LoggerInstance->Info(F("Turned off RGB LED"));
        }
        else
        {
            LoggerInstance->Error("Unknown rgbLed subcommand: " + sub);
        }
        return String("{\"event\": \"rgbLedCommandExecuted\"}");
    },
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

FeatureAction lightSensorAction = {
    .name = "getLightSensorValue",
    .handler = [](const String &command)
    {
        uint16_t value = readLightSensor();
        LoggerInstance->Info("Read light sensor value: " + String(value));
        return String("{\"event\": \"getLightSensorValue\", \"value\": " + String(value) + "}");
    },
    .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

FeatureAction hallSensorAction = {
    .name = "getHallSensorValue",
    .handler = [](const String &command)
    {
        uint16_t value = hallRead();
        LoggerInstance->Info("Read hall sensor value: " + String(value));
        return String("{\"event\": \"getHallSensorValue\", \"value\": " + String(value) + "}");
    },
    .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

#if ENABLE_WIFI
FeatureAction wifiAction = {
    .name = "wifi",
    .handler = wifiHandler,
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};
#endif

// --- Feature ---

Feature *SystemFeatures = new Feature("SystemFeatures", []()
                                      {
#if ENABLE_WIFI
    ActionRegistryInstance->RegisterAction(&wifiAction);
#endif
    ActionRegistryInstance->RegisterAction(&restartAction);
    ActionRegistryInstance->RegisterAction(&featuresAction);
    ActionRegistryInstance->RegisterAction(&infoAction);

    initRgbLed();
    initLightSensor();
    ActionRegistryInstance->RegisterAction(&rgbLedAction);
    ActionRegistryInstance->RegisterAction(&lightSensorAction);
    ActionRegistryInstance->RegisterAction(&hallSensorAction);

    return FeatureState::RUNNING; }, []() {});
