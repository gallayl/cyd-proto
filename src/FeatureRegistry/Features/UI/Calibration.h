#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"
#include <LittleFS.h>

extern LGFX tft;

#define CALIBRATION_FILE "/CalDatLolyLfs9341"
#define REPEAT_CAL false

void touch_calibrate_LolyanGFX()
{
    // ### lgfx calibration data are longer
    // uint16_t calData[5];
    uint16_t calData[8];
    uint8_t calDataOK = 0;

    // debug to see the new size of calData:
    // Serial.printf("calData size: %d\n", sizeof(calData));
    // calData size: 16

    // check if calibration file exists and size is correct
    if (LittleFS.exists(CALIBRATION_FILE))
    {
        if (REPEAT_CAL)
        {
            // Delete if we want to re-calibrate
            LittleFS.remove(CALIBRATION_FILE);
        }
        else
        {
            File f = LittleFS.open(CALIBRATION_FILE, "r");
            if (f)
            {
                // ### lgfx calibration data are longer
                // if (f.readBytes((char *)calData, 14) == 14)
                if (f.readBytes((char *)calData, 16) == 16)
                    calDataOK = 1;
                f.close();
            }
        }
    }

    if (calDataOK && !REPEAT_CAL)
    {
        // calibration data valid
        // ### lgfx
        // tft.setTouch(calData); // TFT_eSPI
        tft.setTouchCalibrate(calData); // LovyanGFX
    }
    else
    {
        // data not valid so recalibrate
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 0);
        tft.setTextFont(2);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.println("Touch corners as indicated");

        tft.setTextFont(1);
        tft.println();

        if (REPEAT_CAL)
        {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Set REPEAT_CAL to false to stop this running again!");
        }

        tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("Calibration complete!");

        // ### lgfx calibration data are longer
        Serial.println("Your Screen Calibration");
        Serial.printf("uint16_t calData[8] = { %d, %d, %d, %d, %d, %d, %d, %d };\n", calData[0], calData[1], calData[2], calData[3], calData[4], calData[5], calData[6], calData[7]);
        // uint16_t calData[8] = { 3756, 373, 3718, 3827, 314, 370, 248, 3848 };
        // Serial.printf("uint16_t calData[5] = { %d, %d, %d, %d, %d };\n", calData[0], calData[1], calData[2], calData[3], calData[4]);

        // store data
        File f = LittleFS.open(CALIBRATION_FILE, "w");
        if (f)
        {
            // ### lgfx calibration data are longer
            // f.write((const unsigned char *)calData, 14);
            f.write((const unsigned char *)calData, 16);
            f.close();
        }
    }
}