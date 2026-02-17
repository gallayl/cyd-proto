#include "SystemFeatures.h"
#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../hw/RgbLed.h"
#include "../../hw/lightSensor.h"
#include "../registeredFeatures.h"
#include "../../utils/System.h"
#include "../../fs/LittleFsInit.h"
#include "../../utils/CJsonHelper.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"
#include "spi_flash_mmap.h"
#include <string>
#include <vector>

#if ENABLE_SD_CARD
#include "./SdCard/SdCardFeature.h"
#endif

#if ENABLE_WIFI
#include "../../hw/WiFi.h"
#endif

// --- Helper functions ---

cJSON *getInfo()
{
    cJSON *response = cJSON_CreateObject();

    cJSON *espObj = cJSON_AddObjectToObject(response, "esp");
    cJSON_AddStringToObject(espObj, "sdkVersion", esp_get_idf_version());
    cJSON_AddNumberToObject(espObj, "cpuFreqMhz", CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
    cJSON_AddNumberToObject(espObj, "freeHeap", getFreeHeap());
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running)
        cJSON_AddNumberToObject(espObj, "freeSkSpace", running->size);
    else
        cJSON_AddNumberToObject(espObj, "freeSkSpace", 0);

    cJSON *flash = cJSON_AddObjectToObject(response, "flash");
    uint32_t flashSize = 0;
    esp_flash_get_size(NULL, &flashSize);
    cJSON_AddNumberToObject(flash, "size", flashSize);

    cJSON *fs = cJSON_AddObjectToObject(response, "fs");
    cJSON_AddNumberToObject(fs, "totalBytes", getLittleFsTotalBytes());
    cJSON_AddNumberToObject(fs, "usedBytes", getLittleFsUsedBytes());

#if ENABLE_SD_CARD
    cJSON *sd = cJSON_AddObjectToObject(response, "sd");
    cJSON_AddBoolToObject(sd, "mounted", isSdCardMounted());
    if (isSdCardMounted())
    {
        cJSON_AddStringToObject(sd, "cardType", getSdCardTypeName());
        cJSON_AddNumberToObject(sd, "totalBytes", (double)getSdCardTotalBytes());
        cJSON_AddNumberToObject(sd, "usedBytes", (double)getSdCardUsedBytes());
        cJSON_AddNumberToObject(sd, "cardSize", (double)getSdCardSize());
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
        cJSON *arr = cJSON_CreateArray();

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
                cJSON *element = cJSON_CreateObject();
                cJSON_AddStringToObject(element, "ssid", (const char *)ap_records[i].ssid);
                cJSON_AddNumberToObject(element, "rssi", ap_records[i].rssi);
                cJSON_AddStringToObject(element, "rssiText", getSignalStrength(ap_records[i].rssi).c_str());
                cJSON_AddStringToObject(element, "encryption", getEncryptionType(ap_records[i].authmode).c_str());
                cJSON_AddItemToArray(arr, element);
            }
        }

        std::string output = cJsonToString(arr);
        cJSON_Delete(arr);
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
        cJSON *response = cJSON_CreateObject();

        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);

        if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
        {
            cJSON *ap = cJSON_AddObjectToObject(response, "ap");
            esp_netif_ip_info_t ap_ip = {};
            esp_netif_get_ip_info(getWifiApNetif(), &ap_ip);
            cJSON_AddStringToObject(ap, "ipAddress", ipToString(ap_ip.ip).c_str());

            uint8_t ap_mac[6];
            esp_wifi_get_mac(WIFI_IF_AP, ap_mac);
            cJSON_AddStringToObject(ap, "macAddress", macToString(ap_mac).c_str());
        }

        if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
        {
            cJSON *sta = cJSON_AddObjectToObject(response, "sta");
            esp_netif_ip_info_t sta_ip = {};
            esp_netif_get_ip_info(getWifiStaNetif(), &sta_ip);
            cJSON_AddStringToObject(sta, "ipAddress", ipToString(sta_ip.ip).c_str());

            uint8_t sta_mac[6];
            esp_wifi_get_mac(WIFI_IF_STA, sta_mac);
            cJSON_AddStringToObject(sta, "macAddress", macToString(sta_mac).c_str());

            wifi_config_t sta_conf = {};
            esp_wifi_get_config(WIFI_IF_STA, &sta_conf);
            cJSON_AddStringToObject(sta, "ssid", (const char *)sta_conf.sta.ssid);
        }

        wifi_ap_record_t ap_info = {};
        int8_t rssi = 0;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
        {
            rssi = ap_info.rssi;
        }
        cJSON_AddStringToObject(response, "wifiStrength", getSignalStrength(rssi).c_str());
        cJSON_AddNumberToObject(response, "wifiRssiDb", rssi);

        std::string output = cJsonToString(response);
        cJSON_Delete(response);
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
                                           withRegisteredFeatures([&output](cJSON *doc)
                                                                  { output = cJsonToString(doc); });
                                           return output;
                                       },
                                       .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

static FeatureAction infoAction = {.name = "info",
                                   .handler =
                                       [](const std::string & /*command*/)
                                   {
                                       cJSON *response = getInfo();
                                       std::string output = cJsonToString(response);
                                       cJSON_Delete(response);
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

        return FeatureState::RUNNING;
    },
    []() {});

// --- Memory handler (needs FeatureRegistry forward decl above) ---

#include "../FeatureRegistry.h"

static std::string memoryHandler(const std::string & /*command*/)
{
    cJSON *doc = cJSON_CreateObject();
    cJSON_AddNumberToObject(doc, "freeHeap", getFreeHeap());
    cJSON_AddNumberToObject(doc, "minFreeHeap", esp_get_minimum_free_heap_size());
    cJSON_AddNumberToObject(doc, "heapSize", heap_caps_get_total_size(MALLOC_CAP_DEFAULT));
    cJSON_AddNumberToObject(doc, "maxAllocHeap", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

    cJSON *tasks = cJSON_AddArrayToObject(doc, "tasks");
    for (uint8_t i = 0; i < featureRegistryInstance->getFeatureCount(); i++)
    {
        Feature *f = featureRegistryInstance->RegisteredFeatures[i];
        if (f->isTaskBased())
        {
            cJSON *t = cJSON_CreateObject();
            cJSON_AddStringToObject(t, "name", f->GetFeatureName().c_str());
            cJSON_AddBoolToObject(t, "running", f->isTaskRunning());
            cJSON_AddNumberToObject(t, "stackHighWaterMark", f->getTaskStackHighWaterMark());
            cJSON_AddItemToArray(tasks, t);
        }
    }

    std::string output = cJsonToString(doc);
    cJSON_Delete(doc);
    return output;
}
