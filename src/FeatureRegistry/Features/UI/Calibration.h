#pragma once
#include <LovyanGFX.hpp>
#include "../../../hw/LovyanGFX_ILI9341_Settings.h"
#include <LittleFS.h>
#include "../Logging.h"

extern LGFX tft;

#define CALIBRATION_FILE "/CalDatLolyLfs9341"

inline String getCalibrationFileForRotation(uint8_t rotation)
{
    return String("/CalDatLoly") + String(rotation);
}

inline void readCalibrationData()
{
    uint8_t rotation = tft.getRotation();
    String rotationFile = getCalibrationFileForRotation(rotation);
    uint16_t calData[8];

    // Try rotation-specific calibration first
    if (LittleFS.exists(rotationFile))
    {
        File f = LittleFS.open(rotationFile, "r");
        if (f)
        {
            if (f.readBytes((char *)calData, 16) == 16)
            {
                tft.setTouchCalibrate(calData);
                f.close();
                LoggerInstance->Info("Touch calibration loaded for rotation " + String(rotation));
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
            LoggerInstance->Info(F("Touch calibration data loaded from legacy file"));
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

    Serial.printf("Screen Calibration (rotation %d)\n", rotation);
    Serial.printf("uint16_t calData[8] = { %d, %d, %d, %d, %d, %d, %d, %d };\n", calData[0], calData[1], calData[2],
                  calData[3], calData[4], calData[5], calData[6], calData[7]);

    // Save to rotation-specific file
    String rotationFile = getCalibrationFileForRotation(rotation);
    File f = LittleFS.open(rotationFile, "w");
    if (f)
    {
        f.write((const unsigned char *)calData, 16);
        f.close();
        LoggerInstance->Info("Calibration saved for rotation " + String(rotation));
    }
}