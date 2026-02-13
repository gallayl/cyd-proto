#pragma once
#include <LovyanGFX.hpp>
#include "../../../hw/LovyanGFX_ILI9341_Settings.h"
#include <LittleFS.h>
#include "../Logging.h"

extern LGFX tft;

#define CALIBRATION_FILE "/CalDatLolyLfs9341"

inline void readCalibrationData()
{
    uint16_t calData[8];

    if (LittleFS.exists(CALIBRATION_FILE))
    {
        File f = LittleFS.open(CALIBRATION_FILE, "r");
        if (f)
        {
            if (f.readBytes((char *)calData, 16) == 16)
                tft.setTouchCalibrate(calData); // LovyanGFX
            f.close();
            LoggerInstance->Info(F("Touch calibration data loaded from LittleFS"));
        }
    }
}

inline void calibrateScreen()
{
    uint16_t calData[8];

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    Serial.println("Your Screen Calibration");
    Serial.printf("uint16_t calData[8] = { %d, %d, %d, %d, %d, %d, %d, %d };\n", calData[0], calData[1], calData[2], calData[3], calData[4], calData[5], calData[6], calData[7]);

    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
        f.write((const unsigned char *)calData, 16);
        f.close();
    }
}