#include "SystemFeatures.h"
#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../hw/RgbLed.h"
#include "../../hw/lightSensor.h"
#include "../registeredFeatures.h"
#include "../../utils/System.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Esp.h>
#include <string>

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
    esp["freeHeap"] = getFreeHeap();
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
static std::string wifiHandler(const std::string &command)
{
    std::string operation = CommandParser::getCommandParameter(command, 1);
    if (operation == "connect")
    {
        std::string ssid = CommandParser::getCommandParameter(command, 2);
        std::string password = CommandParser::getCommandParameter(command, 3);
        if (ssid.length() < 3 || password.length() < 5)
        {
            return "{\"error\": \"ssid or password too short\"}";
        }
        WiFi.disconnect(true, false);
        WiFi.persistent(true);
        WiFiClass::mode(WIFI_AP);
        WiFi.begin(ssid.c_str(), password.c_str());
        return "{\"event\": \"connecting\"}";
    }
    if (operation == "list")
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
        std::string output;
        serializeJson(response, output);
        return output;
    }
    if (operation == "startSTA")
    {
        std::string ssid = CommandParser::getCommandParameter(command, 2);
        std::string passphrase = CommandParser::getCommandParameter(command, 3);
        if (ssid.length() < 3 || passphrase.length() < 5)
        {
            return "{\"error\": \"ssid or passphrase too short\"}";
        }
        startStaMode(ssid.c_str(), passphrase.c_str());
        return "{\"event\": \"starting STA\"}";
    }

    if (operation == "stopSTA")
    {
        bool success = WiFi.softAPdisconnect(true);
        WiFiClass::mode(WIFI_AP);
        WiFi.begin();
        return std::string("{\"event\": \"stopSTA\", \"success\": ") + std::to_string(static_cast<int>(success)) + "}";
    }

    if (operation == "info")
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

        std::string output;
        serializeJson(response, output);
        return output;
    }

    if (operation == "restart")
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
                                          [](const std::string & /*command*/)
                                      {
                                          delay(100);
                                          systemRestart();
                                          return std::string("{\"event\": \"restart\"}");
                                      },
                                      .transports = {.cli = true, .rest = true, .ws = false, .scripting = true}};

static FeatureAction featuresAction = {.name = "features",
                                       .handler =
                                           [](const std::string & /*command*/)
                                       {
                                           std::string output;
                                           withRegisteredFeatures([&output](const JsonDocument &doc)
                                                                  { serializeJson(doc, output); });
                                           return output;
                                       },
                                       .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction infoAction = {.name = "info",
                                   .handler =
                                       [](const std::string & /*command*/)
                                   {
                                       JsonDocument response = getInfo();
                                       std::string output;
                                       serializeJson(response, output);
                                       return output;
                                   },
                                   .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction rgbLedAction = {.name = "rgbLed",
                                     .handler =
                                         [](const std::string &command)
                                     {
                                         const std::string SUB = CommandParser::getCommandParameter(command, 1);
                                         if (SUB == "setColor")
                                         {
                                             int r = atoi(CommandParser::getCommandParameter(command, 2).c_str());
                                             int g = atoi(CommandParser::getCommandParameter(command, 3).c_str());
                                             int b = atoi(CommandParser::getCommandParameter(command, 4).c_str());
                                             setRgbLedColor(r, g, b);
                                             char logBuf[48];
                                             snprintf(logBuf, sizeof(logBuf), "Set RGB LED color to %d,%d,%d", r, g, b);
                                             loggerInstance->Info(logBuf);
                                         }
                                         else if (SUB == "off")
                                         {
                                             setRgbLedColor(0, 0, 0);
                                             loggerInstance->Info("Turned off RGB LED");
                                         }
                                         else
                                         {
                                             loggerInstance->Error(std::string("Unknown rgbLed subcommand: ") + SUB);
                                         }
                                         return std::string("{\"event\": \"rgbLedCommandexecuted\"}");
                                     },
                                     .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

static FeatureAction lightSensorAction = {.name = "getLightSensorValue",
                                          .handler =
                                              [](const std::string & /*command*/)
                                          {
                                              uint16_t value = readLightSensor();
                                              char buf[80];
                                              snprintf(buf, sizeof(buf), "Read light sensor value: %u", value);
                                              loggerInstance->Info(buf);
                                              snprintf(buf, sizeof(buf),
                                                       "{\"event\": \"getLightSensorValue\", \"value\": %u}", value);
                                              return std::string(buf);
                                          },
                                          .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction hallSensorAction = {.name = "getHallSensorValue",
                                         .handler =
                                             [](const std::string & /*command*/)
                                         {
                                             uint16_t value = hallRead();
                                             char buf[80];
                                             snprintf(buf, sizeof(buf), "Read hall sensor value: %u", value);
                                             loggerInstance->Info(buf);
                                             snprintf(buf, sizeof(buf),
                                                      "{\"event\": \"getHallSensorValue\", \"value\": %u}", value);
                                             return std::string(buf);
                                         },
                                         .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

#if ENABLE_WIFI
static FeatureAction wifiAction = {
    .name = "wifi", .handler = wifiHandler, .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};
#endif

// forward declaration to avoid circular include
class FeatureRegistry;
extern FeatureRegistry *featureRegistryInstance;

static std::string memoryHandler(const std::string &command);

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

static std::string memoryHandler(const std::string & /*command*/)
{
    JsonDocument doc;
    doc["freeHeap"] = getFreeHeap();
    doc["minFreeHeap"] = ESP.getMinFreeHeap();
    doc["heapSize"] = ESP.getHeapSize();
    doc["maxAllocHeap"] = ESP.getMaxAllocHeap();

    JsonArray tasks = doc["tasks"].to<JsonArray>();
    for (uint8_t i = 0; i < featureRegistryInstance->getFeatureCount(); i++)
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

    std::string output;
    serializeJson(doc, output);
    return output;
}
