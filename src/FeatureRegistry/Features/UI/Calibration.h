#pragma once
#include <string>
#include <LovyanGFX.hpp>
#include "../../../hw/LovyanGFX_ILI9341_Settings.h"
#include "../../../fs/VirtualFS.h"
#include "../Logging.h"
#include "esp_log.h"
#include <cstdio>
#include <sys/stat.h>

extern LGFX tft;

#define CALIBRATION_FILE "/CalDatLolyLfs9341"

inline std::string getCalibrationFileForRotation(uint8_t rotation)
{
    return std::string("/CalDatLoly") + std::to_string(rotation);
}

inline void readCalibrationData()
{
    uint8_t rotation = tft.getRotation();
    std::string rotationFile = getCalibrationFileForRotation(rotation);
    uint16_t calData[8];

    std::string fullPath = resolveToLittleFsPath(rotationFile);

    // Try rotation-specific calibration first
    if (vfsExists(fullPath))
    {
        FILE *f = fopen(fullPath.c_str(), "r");
        if (f)
        {
            if (fread(calData, 1, 16, f) == 16)
            {
                tft.setTouchCalibrate(calData);
                fclose(f);
                loggerInstance->Info("Touch calibration loaded for rotation " + std::to_string(rotation));
                return;
            }
            fclose(f);
        }
    }

    // Fall back to legacy calibration file
    std::string legacyPath = resolveToLittleFsPath(CALIBRATION_FILE);
    if (vfsExists(legacyPath))
    {
        FILE *f = fopen(legacyPath.c_str(), "r");
        if (f)
        {
            if (fread(calData, 1, 16, f) == 16)
                tft.setTouchCalibrate(calData);
            fclose(f);
            loggerInstance->Info("Touch calibration data loaded from legacy file");
        }
    }
}

inline void calibrateScreen()
{
    uint16_t calData[8];
    uint8_t rotation = tft.getRotation();

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");
    tft.setTextFont(1);
    tft.println();
    tft.printf("Calibrating for rotation: %d\n", rotation);

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    tft.waitDisplay(); // Wait for all calibration drawing to complete

    ESP_LOGI("Calibration", "Screen Calibration (rotation %d)", rotation);
    ESP_LOGI("Calibration", "uint16_t calData[8] = { %d, %d, %d, %d, %d, %d, %d, %d }", calData[0], calData[1],
             calData[2], calData[3], calData[4], calData[5], calData[6], calData[7]);

    // Save to rotation-specific file
    std::string rotationFile = getCalibrationFileForRotation(rotation);

    std::string fullPath = resolveToLittleFsPath(rotationFile);
    FILE *f = fopen(fullPath.c_str(), "w");
    if (f)
    {
        fwrite(calData, 1, 16, f);
        fclose(f);
        loggerInstance->Info("Calibration saved for rotation " + std::to_string(rotation));
    }
}
