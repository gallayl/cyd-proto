#include "SystemFeatures.h"
#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../hw/RgbLed.h"
#include "../../hw/lightSensor.h"
#include "../registeredFeatures.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Esp.h>

#if ENABLE_SD_CARD
#include <SD.h>
#include "./SdCard/SdCardFeature.h"
#endif

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

#if ENABLE_SD_CARD
    JsonObject sd = response["sd"].to<JsonObject>();
    sd["mounted"] = isSdCardMounted();
    if (isSdCardMounted())
    {
        sdcard_type_t cardType = SD.cardType();
        const char *typeName = "UNKNOWN";
        switch (cardType)
        {
        case CARD_MMC:
            typeName = "MMC";
            break;
        case CARD_SD:
            typeName = "SD";
            break;
        case CARD_SDHC:
            typeName = "SDHC";
            break;
        default:
            break;
        }
        sd["cardType"] = typeName;
        sd["totalBytes"] = SD.totalBytes();
        sd["usedBytes"] = SD.usedBytes();
        sd["cardSize"] = SD.cardSize();
    }
#endif

    return response;
}

// --- WiFi handler ---

#if ENABLE_WIFI
static String wifiHandler(const String &command)
{
    String operation = CommandParser::getCommandParameter(command, 1);
    if (operation.compareTo("connect") == 0)
    {
        String ssid = CommandParser::getCommandParameter(command, 2);
        String password = CommandParser::getCommandParameter(command, 3);
        if (ssid.length() < 3 || password.length() < 5)
        {
            return String("{\"error\": \"ssid or password too short\"}");
        }
        WiFi.disconnect(true, false);
        WiFi.persistent(true);
        WiFiClass::mode(WIFI_AP);
        WiFi.begin(ssid.c_str(), password.c_str());
        return String("{\"event\": \"connecting\"}");
    }
    if (operation.compareTo("list") == 0)
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
    if (operation.compareTo("startSTA") == 0)
    {
        String ssid = CommandParser::getCommandParameter(command, 2);
        String passphrase = CommandParser::getCommandParameter(command, 3);
        if (ssid.length() < 3 || passphrase.length() < 5)
        {
            return String("{\"error\": \"ssid or passphrase too short\"}");
        }
        startStaMode(ssid, passphrase);
        return String("{\"event\": \"starting STA\"}");
    }

    if (operation.compareTo("stopSTA") == 0)
    {
        bool success = WiFi.softAPdisconnect(true);
        WiFiClass::mode(WIFI_AP);
        WiFi.begin();
        return String("{\"event\": \"stopSTA\", \"success\": " + String(static_cast<int>(success)) + "}");
    }

    if (operation.compareTo("info") == 0)
    {
        JsonDocument response;
        if (WiFiClass::getMode() == WIFI_AP || WiFiClass::getMode() == WIFI_AP_STA)
        {
            JsonObject ap = response["ap"].to<JsonObject>();
            ap["ipAddress"] = WiFi.softAPIP().toString();
            ap["macAddress"] = WiFi.softAPmacAddress();
        }

        if (WiFiClass::getMode() == WIFI_STA || WiFiClass::getMode() == WIFI_AP_STA)
        {
            JsonObject sta = response["sta"].to<JsonObject>();
            sta["ipAddress"] = WiFi.localIP().toString();
            sta["macAddress"] = WiFi.macAddress();
            sta["ssid"] = WiFi.SSID();
        }

        int8_t rssi = WiFi.RSSI();
        response["wifiStrength"] = getSignalStrength(rssi);
        response["wifiRssiDb"] = rssi;

        String output;
        serializeJson(response, output);
        return output;
    }

    if (operation.compareTo("restart") == 0)
    {
        WiFi.disconnect(true, false);
        WiFi.begin();
        return {"{\"event\": \"disconnecting\"}"};
    }
    return {"{\"event\": \"Unknown WiFi operation command. The available commands are: info, list, connect "
            "<ssid> <password>, startSTA <ssid> <passphrase>, stopSTA\"}"};
}
#endif

// --- Action definitions ---

static FeatureAction restartAction = {.name = "restart",
                                      .type = "POST",
                                      .handler =
                                          [](const String & /*command*/)
                                      {
                                          delay(100);
                                          ESP.restart();
                                          return String("{\"event\": \"restart\"}");
                                      },
                                      .transports = {.cli = true, .rest = true, .ws = false, .scripting = true}};

static FeatureAction featuresAction = {.name = "features",
                                       .handler =
                                           [](const String & /*command*/)
                                       {
                                           String output;
                                           withRegisteredFeatures([&output](const JsonDocument &doc)
                                                                  { serializeJson(doc, output); });
                                           return output;
                                       },
                                       .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction infoAction = {.name = "info",
                                   .handler =
                                       [](const String & /*command*/)
                                   {
                                       JsonDocument response = getInfo();
                                       String output;
                                       serializeJson(response, output);
                                       return output;
                                   },
                                   .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction rgbLedAction = {.name = "rgbLed",
                                     .handler =
                                         [](const String &command)
                                     {
                                         const String SUB = CommandParser::getCommandParameter(command, 1);
                                         if (SUB == "setColor")
                                         {
                                             int r = CommandParser::getCommandParameter(command, 2).toInt();
                                             int g = CommandParser::getCommandParameter(command, 3).toInt();
                                             int b = CommandParser::getCommandParameter(command, 4).toInt();
                                             setRgbLedColor(r, g, b);
                                             char logBuf[48];
                                             snprintf(logBuf, sizeof(logBuf), "Set RGB LED color to %d,%d,%d", r, g, b);
                                             loggerInstance->Info(logBuf);
                                         }
                                         else if (SUB == "off")
                                         {
                                             setRgbLedColor(0, 0, 0);
                                             loggerInstance->Info(F("Turned off RGB LED"));
                                         }
                                         else
                                         {
                                             loggerInstance->Error("Unknown rgbLed subcommand: " + SUB);
                                         }
                                         return String("{\"event\": \"rgbLedCommandexecuted\"}");
                                     },
                                     .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

static FeatureAction lightSensorAction = {.name = "getLightSensorValue",
                                          .handler =
                                              [](const String & /*command*/)
                                          {
                                              uint16_t value = readLightSensor();
                                              char buf[80];
                                              snprintf(buf, sizeof(buf), "Read light sensor value: %u", value);
                                              loggerInstance->Info(buf);
                                              snprintf(buf, sizeof(buf),
                                                       "{\"event\": \"getLightSensorValue\", \"value\": %u}", value);
                                              return String(buf);
                                          },
                                          .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction hallSensorAction = {.name = "getHallSensorValue",
                                         .handler =
                                             [](const String & /*command*/)
                                         {
                                             uint16_t value = hallRead();
                                             char buf[80];
                                             snprintf(buf, sizeof(buf), "Read hall sensor value: %u", value);
                                             loggerInstance->Info(buf);
                                             snprintf(buf, sizeof(buf),
                                                      "{\"event\": \"getHallSensorValue\", \"value\": %u}", value);
                                             return String(buf);
                                         },
                                         .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

#if ENABLE_WIFI
static FeatureAction wifiAction = {
    .name = "wifi", .handler = wifiHandler, .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};
#endif

// forward declaration to avoid circular include
class FeatureRegistry;
extern FeatureRegistry *featureRegistryInstance;

static String memoryHandler(const String &command);

static FeatureAction memoryAction = {.name = "memory",
                                     .handler = memoryHandler,
                                     .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

// --- Feature ---

Feature *systemFeatures = new Feature(
    "systemFeatures",
    []()
    {
#if ENABLE_WIFI
        actionRegistryInstance->registerAction(&wifiAction);
#endif
        actionRegistryInstance->registerAction(&restartAction);
        actionRegistryInstance->registerAction(&featuresAction);
        actionRegistryInstance->registerAction(&infoAction);
        actionRegistryInstance->registerAction(&memoryAction);

        initRgbLed();
        initLightSensor();
        actionRegistryInstance->registerAction(&rgbLedAction);
        actionRegistryInstance->registerAction(&lightSensorAction);
        actionRegistryInstance->registerAction(&hallSensorAction);

        return FeatureState::RUNNING;
    },
    []() {});

// --- Memory handler (needs FeatureRegistry forward decl above) ---

#include "../FeatureRegistry.h"

static String memoryHandler(const String & /*command*/)
{
    JsonDocument doc;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["minFreeHeap"] = ESP.getMinFreeHeap();
    doc["heapSize"] = ESP.getHeapSize();
    doc["maxAllocHeap"] = ESP.getMaxAllocHeap();

    JsonArray tasks = doc["tasks"].to<JsonArray>();
    for (uint8_t i = 0; i < featureRegistryInstance->GetFeatureCount(); i++)
    {
        Feature *f = featureRegistryInstance->RegisteredFeatures[i];
        if (f->isTaskBased())
        {
            JsonObject t = tasks.add<JsonObject>();
            t["name"] = f->GetFeatureName();
            t["running"] = f->isTaskRunning();
            t["stackHighWaterMark"] = f->getTaskStackHighWaterMark();
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}
