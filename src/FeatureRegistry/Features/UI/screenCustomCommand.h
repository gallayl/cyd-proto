#pragma once

#include <stdlib.h>
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"
#include "./Calibration.h"

// the TFT object is defined in UiFeature.h
extern LGFX tft;

/**
 * Handler for the "screen" namespace command.  Subcommands are taken from the
 * second token of the incoming command string.  Supported subcommands:
 *   - calibrate        : run touch calibration
 *   - demo             : display the hello world demo
 *   - rotate <angle>   : rotate the display (angle may be decimal or 0x.. hex)
 *   - clear [color]    : fill screen with an optional colour (default black)
 */
static String screenCommandHandler(String command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    if (sub == "calibrate")
    {
        calibrateScreen();
        LoggerInstance->Info("Calibrated touch screen");
        return String("{\"event\":\"calibrate\", \"status\":\"success\"}");
    }
    else if (sub == "demo")
    {
        // simple hello world demo on the TFT
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_GREENYELLOW);
        tft.setTextSize(2);
        tft.println("Hello World!");

        tft.drawCircle(120, 160, 50, TFT_MAGENTA);
        tft.drawEllipse(200, 160, 60, 40, TFT_GOLD);

        LoggerInstance->Info("Displayed hello world demo");
        return String("{\"event\":\"helloDemo\"}");
    }
    else if (sub == "rotate")
    {
        String rotateParam = CommandParser::GetCommandParameter(command, 2);
        uint16_t rotate = 0;
        if (rotateParam.length() > 0)
        {
            rotate = (uint16_t)strtoul(rotateParam.c_str(), NULL, 0);
        }
        tft.setRotation(rotate);
        LoggerInstance->Info("Screen rotated to " + String(rotate));
        return String(String("{\"event\":\"rotate\",\"rotation\":") + rotate + String("}"));
    }
    else if (sub == "clear")
    {
        String colorParam = CommandParser::GetCommandParameter(command, 2);
        uint16_t color = TFT_BLACK;
        if (colorParam.length() > 0)
        {
            color = (uint16_t)strtoul(colorParam.c_str(), NULL, 0);
        }
        tft.fillScreen(color);
        LoggerInstance->Info("Screen cleared");
        return String(String("{\"event\":\"clear\",\"color\":\"") + colorParam + String("\"}"));
    }

    // unknown subcommand
    LoggerInstance->Info("Unknown screen subcommand: " + sub);
    return String(String("{\"event\":\"screen\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

// expose the CustomCommand pointer for registration
CustomCommand *screenCustomCommand = new CustomCommand("screen", [](String command) { return screenCommandHandler(command); });
