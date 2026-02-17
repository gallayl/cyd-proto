#include "SystemFeatures.h"
#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../hw/RgbLed.h"
#include "../../hw/lightSensor.h"
#include "../registeredFeatures.h"
#include "../../utils/System.h"
#include "../../fs/LittleFsInit.h"
#include <ArduinoJson.h>
#ifdef USE_ESP_IDF
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"
#include "spi_flash_mmap.h"
#else
#include <Esp.h>
#endif
#include <string>
#include <vector>

#if ENABLE_SD_CARD
#ifndef USE_ESP_IDF
#include <SD.h>
#endif
#include "./SdCard/SdCardFeature.h"
#endif

#if ENABLE_WIFI
#ifndef USE_ESP_IDF
#include <WiFi.h>
#include <IPAddress.h>
#endif
#include "../../hw/WiFi.h"
#endif

// --- Helper functions ---

JsonDocument getInfo()
{
    JsonDocument response;
    JsonObject espObj = response["esp"].to<JsonObject>();

#ifdef USE_ESP_IDF
    espObj["sdkVersion"] = esp_get_idf_version();
    espObj["cpuFreqMhz"] = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
    espObj["freeHeap"] = getFreeHeap();
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running)
        espObj["freeSkSpace"] = running->size;
    else
        espObj["freeSkSpace"] = 0;
#else
    espObj["sdkVersion"] = ESP.getSdkVersion();
    espObj["cpuFreqMhz"] = ESP.getCpuFreqMHz();
    espObj["freeHeap"] = getFreeHeap();
    espObj["freeSkSpace"] = ESP.getFreeSketchSpace();
#endif

    JsonObject flash = response["flash"].to<JsonObject>();

#ifdef USE_ESP_IDF
    uint32_t flashSize = 0;
    esp_flash_get_size(NULL, &flashSize);
    flash["size"] = flashSize;
#else
    flash["mode"] = ESP.getFlashChipMode();
    flash["size"] = ESP.getFlashChipSize();
    flash["speed"] = ESP.getFlashChipSpeed();
#endif

    JsonObject fs = response["fs"].to<JsonObject>();
    fs["totalBytes"] = getLittleFsTotalBytes();
    fs["usedBytes"] = getLittleFsUsedBytes();

#if ENABLE_SD_CARD
    JsonObject sd = response["sd"].to<JsonObject>();
    sd["mounted"] = isSdCardMounted();
    if (isSdCardMounted())
    {
        sd["cardType"] = getSdCardTypeName();
        sd["totalBytes"] = getSdCardTotalBytes();
        sd["usedBytes"] = getSdCardUsedBytes();
        sd["cardSize"] = getSdCardSize();
    }
#endif

    return response;
}

// --- WiFi handler ---

#if ENABLE_WIFI

#ifdef USE_ESP_IDF

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

        esp_wifi_disconnect();

        wifi_config_t sta_cfg = {};
        strncpy((char *)sta_cfg.sta.ssid, ssid.c_str(), sizeof(sta_cfg.sta.ssid) - 1);
        strncpy((char *)sta_cfg.sta.password, password.c_str(), sizeof(sta_cfg.sta.password) - 1);
        esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);

        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);
        if (mode == WIFI_MODE_AP)
        {
            esp_wifi_set_mode(WIFI_MODE_APSTA);
        }

        esp_wifi_connect();
        return "{\"event\": \"connecting\"}";
    }

    if (operation == "list")
    {
        JsonDocument response;
        JsonArray arr = response.to<JsonArray>();

        wifi_scan_config_t scan_cfg = {};
        scan_cfg.show_hidden = true;
        esp_wifi_scan_start(&scan_cfg, true);

        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);

        if (ap_count > 0)
        {
            std::vector<wifi_ap_record_t> ap_records(ap_count);
            esp_wifi_scan_get_ap_records(&ap_count, ap_records.data());

            for (uint16_t i = 0; i < ap_count; i++)
            {
                JsonObject element = arr.add<JsonObject>();
                element["ssid"] = (const char *)ap_records[i].ssid;
                element["rssi"] = ap_records[i].rssi;
                element["rssiText"] = getSignalStrength(ap_records[i].rssi);
                element["encryption"] = getEncryptionType(ap_records[i].authmode);
            }
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
        startStaMode(ssid, passphrase);
        return "{\"event\": \"starting STA\"}";
    }

    if (operation == "stopSTA")
    {
        esp_wifi_deauth_sta(0);
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_connect();
        return "{\"event\": \"stopSTA\", \"success\": 1}";
    }

    if (operation == "info")
    {
        JsonDocument response;

        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);

        if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
        {
            JsonObject ap = response["ap"].to<JsonObject>();
            esp_netif_ip_info_t ap_ip = {};
            esp_netif_get_ip_info(getWifiApNetif(), &ap_ip);
            ap["ipAddress"] = ipToString(ap_ip.ip);

            uint8_t ap_mac[6];
            esp_wifi_get_mac(WIFI_IF_AP, ap_mac);
            ap["macAddress"] = macToString(ap_mac);
        }

        if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
        {
            JsonObject sta = response["sta"].to<JsonObject>();
            esp_netif_ip_info_t sta_ip = {};
            esp_netif_get_ip_info(getWifiStaNetif(), &sta_ip);
            sta["ipAddress"] = ipToString(sta_ip.ip);

            uint8_t sta_mac[6];
            esp_wifi_get_mac(WIFI_IF_STA, sta_mac);
            sta["macAddress"] = macToString(sta_mac);

            wifi_config_t sta_conf = {};
            esp_wifi_get_config(WIFI_IF_STA, &sta_conf);
            sta["ssid"] = (const char *)sta_conf.sta.ssid;
        }

        wifi_ap_record_t ap_info = {};
        int8_t rssi = 0;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
        {
            rssi = ap_info.rssi;
        }
        response["wifiStrength"] = getSignalStrength(rssi);
        response["wifiRssiDb"] = rssi;

        std::string output;
        serializeJson(response, output);
        return output;
    }

    if (operation == "restart")
    {
        esp_wifi_disconnect();
        esp_wifi_connect();
        return {"{\"event\": \"disconnecting\"}"};
    }

    return {"{\"event\": \"Unknown WiFi operation command. The available commands are: info, list, connect "
            "<ssid> <password>, startSTA <ssid> <passphrase>, stopSTA\"}"};
}

#else

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

#endif // USE_ESP_IDF

#endif // ENABLE_WIFI

// --- Action definitions ---

static FeatureAction restartAction = {.name = "restart",
                                      .type = "POST",
                                      .handler =
                                          [](const std::string & /*command*/)
                                      {
                                          vTaskDelay(pdMS_TO_TICKS(100));
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

#ifndef USE_ESP_IDF
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
#endif

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
#ifndef USE_ESP_IDF
        actionRegistryInstance->registerAction(&hallSensorAction);
#endif

        return FeatureState::RUNNING;
    },
    []() {});

// --- Memory handler (needs FeatureRegistry forward decl above) ---

#include "../FeatureRegistry.h"

static std::string memoryHandler(const std::string & /*command*/)
{
    JsonDocument doc;
    doc["freeHeap"] = getFreeHeap();
#ifdef USE_ESP_IDF
    doc["minFreeHeap"] = esp_get_minimum_free_heap_size();
    doc["heapSize"] = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    doc["maxAllocHeap"] = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
#else
    doc["minFreeHeap"] = ESP.getMinFreeHeap();
    doc["heapSize"] = ESP.getHeapSize();
    doc["maxAllocHeap"] = ESP.getMaxAllocHeap();
#endif

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
