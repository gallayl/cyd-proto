#include "SdCardFeature.h"

#if ENABLE_SD_CARD

#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"
#include "cJSON.h"
#include "../../../utils/CJsonHelper.h"

#include <cstddef>
#include <string>

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

static sdmmc_card_t *sdCard = nullptr;
static bool sdSpiInitialized = false;

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

    if (sdCard == nullptr)
        return "UNKNOWN";
    if (sdCard->is_mmc)
        return "MMC";
    if (sdCard->ocr & (1 << 30))
        return "SDHC";
    return "SD";
}

uint64_t getSdCardTotalBytes()
{
    if (!sdMounted)
        return 0;

    uint64_t totalBytes = 0, freeBytes = 0;
    if (esp_vfs_fat_info(SD_MOUNT_POINT, &totalBytes, &freeBytes) == ESP_OK)
    {
        return totalBytes;
    }
    return 0;
}

uint64_t getSdCardUsedBytes()
{
    if (!sdMounted)
        return 0;

    uint64_t totalBytes = 0, freeBytes = 0;
    if (esp_vfs_fat_info(SD_MOUNT_POINT, &totalBytes, &freeBytes) == ESP_OK)
    {
        return totalBytes - freeBytes;
    }
    return 0;
}

uint64_t getSdCardSize()
{
    if (!sdMounted)
        return 0;

    if (sdCard != nullptr)
    {
        return (uint64_t)sdCard->csd.capacity * sdCard->csd.sector_size;
    }
    return 0;
}

// --- Mount / Unmount / Info ---

static std::string doMount()
{
    if (sdMounted)
    {
        return "{\"error\": \"SD card already mounted\"}";
    }

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
}

static std::string doUnmount()
{
    if (!sdMounted)
    {
        return {"{\"error\": \"SD card not mounted\"}"};
    }

    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, sdCard);
    sdCard = nullptr;

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

    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "mounted", true);
    cJSON_AddStringToObject(response, "cardType", getSdCardTypeName());
    cJSON_AddNumberToObject(response, "totalBytes", (double)getSdCardTotalBytes());
    cJSON_AddNumberToObject(response, "usedBytes", (double)getSdCardUsedBytes());
    cJSON_AddNumberToObject(response, "cardSize", (double)getSdCardSize());

    std::string output = cJsonToString(response);
    cJSON_Delete(response);
    return output;
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

        return FeatureState::RUNNING;
    },
    []() {},
    []()
    {
        if (sdMounted)
        {
            esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, sdCard);
            sdCard = nullptr;
            sdMounted = false;
            loggerInstance->Info("SD card unmounted");
        }
    });

#endif // ENABLE_SD_CARD
