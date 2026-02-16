#include "SdCardFeature.h"

#if ENABLE_SD_CARD

#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

#include <cstddef>
#include <string>

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

static SPIClass sdSPI(HSPI);
static bool sdMounted = false;

bool isSdCardMounted()
{
    return sdMounted;
}

static const char *cardTypeName(sdcard_type_t type)
{
    switch (type)
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
}

static std::string doMount()
{
    if (sdMounted)
    {
        return "{\"error\": \"SD card already mounted\"}";
    }

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
    response["cardType"] = cardTypeName(cardType);
    response["totalBytes"] = SD.totalBytes();
    response["usedBytes"] = SD.usedBytes();

    std::string output;
    serializeJson(response, output);

    loggerInstance->Info(std::string("SD card mounted (") + cardTypeName(cardType) + ", " +
                         std::to_string(SD.totalBytes() / (1024 * 1024)) + " MB)");
    return output;
}

static std::string doUnmount()
{
    if (!sdMounted)
    {
        return {"{\"error\": \"SD card not mounted\"}"};
    }

    SD.end();
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

    JsonDocument response;
    response["mounted"] = true;
    response["cardType"] = cardTypeName(SD.cardType());
    response["totalBytes"] = SD.totalBytes();
    response["usedBytes"] = SD.usedBytes();
    response["cardSize"] = SD.cardSize();

    std::string output;
    serializeJson(response, output);
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
        return {"{\"error\": \"SD card formatting is not supported by the Arduino SD library\"}"};
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

        // Attempt auto-mount at startup
        sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

        if (SD.begin(SD_CS, sdSPI))
        {
            sdcard_type_t cardType = SD.cardType();
            if (cardType != CARD_NONE)
            {
                sdMounted = true;
                loggerInstance->Info(std::string("SD card detected (") + cardTypeName(cardType) + ", " +
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

        return FeatureState::RUNNING;
    },
    []() {},
    []()
    {
        if (sdMounted)
        {
            SD.end();
            sdMounted = false;
            loggerInstance->Info("SD card unmounted");
        }
    });

#endif // ENABLE_SD_CARD
