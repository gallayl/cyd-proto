#include "SdCardFeature.h"

#if ENABLE_SD_CARD

#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"
#ifdef USE_ESP_IDF
#include "cJSON.h"
#include "../../../utils/CJsonHelper.h"
#else
#include <ArduinoJson.h>
#endif

#include <cstddef>
#include <string>

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

#ifdef USE_ESP_IDF
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

static sdmmc_card_t *sdCard = nullptr;
static bool sdSpiInitialized = false;
#else
#include <SD.h>
#include <SPI.h>

static SPIClass sdSPI(HSPI);
#endif

static bool sdMounted = false;

bool isSdCardMounted()
{
    return sdMounted;
}

// --- Platform-independent accessors ---

const char *getSdCardTypeName()
{
    if (!sdMounted)
        return "NONE";

#ifdef USE_ESP_IDF
    if (sdCard == nullptr)
        return "UNKNOWN";
    if (sdCard->is_mmc)
        return "MMC";
    if (sdCard->ocr & (1 << 30))
        return "SDHC";
    return "SD";
#else
    sdcard_type_t cardType = SD.cardType();
    switch (cardType)
    {
    case CARD_MMC:
        return "MMC";
    case CARD_SD:
        return "SD";
    case CARD_SDHC:
        return "SDHC";
    default:
        return "UNKNOWN";
    }
#endif
}

uint64_t getSdCardTotalBytes()
{
    if (!sdMounted)
        return 0;

#ifdef USE_ESP_IDF
    uint64_t totalBytes = 0, freeBytes = 0;
    if (esp_vfs_fat_info(SD_MOUNT_POINT, &totalBytes, &freeBytes) == ESP_OK)
    {
        return totalBytes;
    }
    return 0;
#else
    return SD.totalBytes();
#endif
}

uint64_t getSdCardUsedBytes()
{
    if (!sdMounted)
        return 0;

#ifdef USE_ESP_IDF
    uint64_t totalBytes = 0, freeBytes = 0;
    if (esp_vfs_fat_info(SD_MOUNT_POINT, &totalBytes, &freeBytes) == ESP_OK)
    {
        return totalBytes - freeBytes;
    }
    return 0;
#else
    return SD.usedBytes();
#endif
}

uint64_t getSdCardSize()
{
    if (!sdMounted)
        return 0;

#ifdef USE_ESP_IDF
    if (sdCard != nullptr)
    {
        return (uint64_t)sdCard->csd.capacity * sdCard->csd.sector_size;
    }
    return 0;
#else
    return SD.cardSize();
#endif
}

// --- Mount / Unmount / Info ---

static std::string doMount()
{
    if (sdMounted)
    {
        return "{\"error\": \"SD card already mounted\"}";
    }

#ifdef USE_ESP_IDF
    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {};
    mount_cfg.format_if_mount_failed = false;
    mount_cfg.max_files = 5;
    mount_cfg.allocation_unit_size = 16 * 1024;

    if (!sdSpiInitialized)
    {
        spi_bus_config_t bus_cfg = {};
        bus_cfg.mosi_io_num = SD_MOSI;
        bus_cfg.miso_io_num = SD_MISO;
        bus_cfg.sclk_io_num = SD_SCK;
        bus_cfg.quadwp_io_num = -1;
        bus_cfg.quadhd_io_num = -1;
        bus_cfg.max_transfer_sz = 4000;

        esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            loggerInstance->Error("SPI bus init failed");
            return "{\"error\": \"SPI bus init failed\"}";
        }
        sdSpiInitialized = true;
    }

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = (gpio_num_t)SD_CS;
    slot_cfg.host_id = SPI2_HOST;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;

    esp_err_t err = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_cfg, &mount_cfg, &sdCard);
    if (err != ESP_OK)
    {
        loggerInstance->Error("SD card mount failed");
        return "{\"error\": \"SD card mount failed\"}";
    }

    sdMounted = true;

    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "event", "sd_mounted");
    cJSON_AddStringToObject(response, "cardType", getSdCardTypeName());
    cJSON_AddNumberToObject(response, "totalBytes", (double)getSdCardTotalBytes());
    cJSON_AddNumberToObject(response, "usedBytes", (double)getSdCardUsedBytes());

    std::string output = cJsonToString(response);
    cJSON_Delete(response);

    loggerInstance->Info(std::string("SD card mounted (") + getSdCardTypeName() + ", " +
                         std::to_string(getSdCardSize() / (1024 * 1024)) + " MB)");
    return output;
#else
    sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, sdSPI))
    {
        loggerInstance->Error("SD card mount failed");
        return "{\"error\": \"SD card mount failed\"}";
    }

    sdcard_type_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        SD.end();
        loggerInstance->Error("No SD card detected");
        return "{\"error\": \"No SD card detected\"}";
    }

    sdMounted = true;

    JsonDocument response;
    response["event"] = "sd_mounted";
    response["cardType"] = getSdCardTypeName();
    response["totalBytes"] = SD.totalBytes();
    response["usedBytes"] = SD.usedBytes();

    std::string output;
    serializeJson(response, output);

    loggerInstance->Info(std::string("SD card mounted (") + getSdCardTypeName() + ", " +
                         std::to_string(SD.totalBytes() / (1024 * 1024)) + " MB)");
    return output;
#endif
}

static std::string doUnmount()
{
    if (!sdMounted)
    {
        return {"{\"error\": \"SD card not mounted\"}"};
    }

#ifdef USE_ESP_IDF
    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, sdCard);
    sdCard = nullptr;
#else
    SD.end();
#endif

    sdMounted = false;

    loggerInstance->Info("SD card unmounted");
    return {"{\"event\": \"sd_unmounted\"}"};
}

static std::string doInfo()
{
    if (!sdMounted)
    {
        return "{\"error\": \"SD card not mounted\"}";
    }

#ifdef USE_ESP_IDF
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "mounted", true);
    cJSON_AddStringToObject(response, "cardType", getSdCardTypeName());
    cJSON_AddNumberToObject(response, "totalBytes", (double)getSdCardTotalBytes());
    cJSON_AddNumberToObject(response, "usedBytes", (double)getSdCardUsedBytes());
    cJSON_AddNumberToObject(response, "cardSize", (double)getSdCardSize());

    std::string output = cJsonToString(response);
    cJSON_Delete(response);
    return output;
#else
    JsonDocument response;
    response["mounted"] = true;
    response["cardType"] = getSdCardTypeName();
    response["totalBytes"] = getSdCardTotalBytes();
    response["usedBytes"] = getSdCardUsedBytes();
    response["cardSize"] = getSdCardSize();

    std::string output;
    serializeJson(response, output);
    return output;
#endif
}

static std::string sdHandler(const std::string &command)
{
    std::string operation = CommandParser::getCommandParameter(command, 1);

    if (operation == "mount")
    {
        return doMount();
    }

    if (operation == "unmount")
    {
        return doUnmount();
    }

    if (operation == "info")
    {
        return doInfo();
    }

    if (operation == "format")
    {
        return {"{\"error\": \"SD card formatting is not supported\"}"};
    }

    return {"{\"error\": \"Usage: sd mount | sd unmount | sd info\"}"};
}

static FeatureAction sdAction = {
    .name = "sd", .handler = sdHandler, .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

Feature *SdCardFeature = new Feature(
    "SdCard",
    []()
    {
        actionRegistryInstance->registerAction(&sdAction);

#ifdef USE_ESP_IDF
        esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {};
        mount_cfg.format_if_mount_failed = false;
        mount_cfg.max_files = 5;
        mount_cfg.allocation_unit_size = 16 * 1024;

        spi_bus_config_t bus_cfg = {};
        bus_cfg.mosi_io_num = SD_MOSI;
        bus_cfg.miso_io_num = SD_MISO;
        bus_cfg.sclk_io_num = SD_SCK;
        bus_cfg.quadwp_io_num = -1;
        bus_cfg.quadhd_io_num = -1;
        bus_cfg.max_transfer_sz = 4000;

        esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            loggerInstance->Info("SD SPI bus init failed, SD card not available");
            return FeatureState::RUNNING;
        }
        sdSpiInitialized = true;

        sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_cfg.gpio_cs = (gpio_num_t)SD_CS;
        slot_cfg.host_id = SPI2_HOST;

        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        host.slot = SPI2_HOST;

        err = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_cfg, &mount_cfg, &sdCard);
        if (err == ESP_OK)
        {
            sdMounted = true;
            loggerInstance->Info(std::string("SD card detected (") + getSdCardTypeName() + ", " +
                                 std::to_string(getSdCardSize() / (static_cast<uint64_t>(1024 * 1024))) + " MB)");
        }
        else
        {
            loggerInstance->Info("SD card not available");
        }
#else
        // Attempt auto-mount at startup
        sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

        if (SD.begin(SD_CS, sdSPI))
        {
            sdcard_type_t cardType = SD.cardType();
            if (cardType != CARD_NONE)
            {
                sdMounted = true;
                loggerInstance->Info(std::string("SD card detected (") + getSdCardTypeName() + ", " +
                                     std::to_string(SD.totalBytes() / (static_cast<uint64_t>(1024 * 1024))) + " MB)");
            }
            else
            {
                SD.end();
                loggerInstance->Info("No SD card inserted");
            }
        }
        else
        {
            loggerInstance->Info("SD card not available");
        }
#endif

        return FeatureState::RUNNING;
    },
    []() {},
    []()
    {
        if (sdMounted)
        {
#ifdef USE_ESP_IDF
            esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, sdCard);
            sdCard = nullptr;
#else
            SD.end();
#endif
            sdMounted = false;
            loggerInstance->Info("SD card unmounted");
        }
    });

#endif // ENABLE_SD_CARD
