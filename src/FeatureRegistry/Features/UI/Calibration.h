#pragma once
#include <string>
#include <LovyanGFX.hpp>
#include "../../../hw/LovyanGFX_ILI9341_Settings.h"
#include "../../../fs/VirtualFS.h"
#include "../Logging.h"
#ifdef USE_ESP_IDF
#include "esp_log.h"
#include <cstdio>
#include <sys/stat.h>
#else
#include <LittleFS.h>
#endif

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

#ifdef USE_ESP_IDF
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
#else
    // Try rotation-specific calibration first
    if (LittleFS.exists(rotationFile.c_str()))
    {
        File f = LittleFS.open(rotationFile.c_str(), "r");
        if (f)
        {
            if (f.readBytes((char *)calData, 16) == 16)
            {
                tft.setTouchCalibrate(calData);
                f.close();
                loggerInstance->Info("Touch calibration loaded for rotation " + std::to_string(rotation));
                return;
            }
            f.close();
        }
    }

    // Fall back to legacy calibration file
    if (LittleFS.exists(CALIBRATION_FILE))
    {
        File f = LittleFS.open(CALIBRATION_FILE, "r");
        if (f)
        {
            if (f.readBytes((char *)calData, 16) == 16)
                tft.setTouchCalibrate(calData);
            f.close();
            loggerInstance->Info("Touch calibration data loaded from legacy file");
        }
    }
#endif
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

#ifdef USE_ESP_IDF
    ESP_LOGI("Calibration", "Screen Calibration (rotation %d)", rotation);
    ESP_LOGI("Calibration", "uint16_t calData[8] = { %d, %d, %d, %d, %d, %d, %d, %d }", calData[0], calData[1],
             calData[2], calData[3], calData[4], calData[5], calData[6], calData[7]);
#else
    Serial.printf("Screen Calibration (rotation %d)\n", rotation);
    Serial.printf("uint16_t calData[8] = { %d, %d, %d, %d, %d, %d, %d, %d };\n", calData[0], calData[1], calData[2],
                  calData[3], calData[4], calData[5], calData[6], calData[7]);
#endif

    // Save to rotation-specific file
    std::string rotationFile = getCalibrationFileForRotation(rotation);

#ifdef USE_ESP_IDF
    std::string fullPath = resolveToLittleFsPath(rotationFile);
    FILE *f = fopen(fullPath.c_str(), "w");
    if (f)
    {
        fwrite(calData, 1, 16, f);
        fclose(f);
        loggerInstance->Info("Calibration saved for rotation " + std::to_string(rotation));
    }
#else
    File f = LittleFS.open(rotationFile.c_str(), "w");
    if (f)
    {
        f.write((const unsigned char *)calData, 16);
        f.close();
        loggerInstance->Info("Calibration saved for rotation " + std::to_string(rotation));
    }
#endif
}
