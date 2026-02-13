#pragma once

#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../Logging.h"

extern LGFX tft;

CustomCommand *helloDemoCustomCommand = new CustomCommand("hello", [](String command)
                                                          {    
    // simple hello world demo on the TFT
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.setTextSize(2);
    tft.println("Hello World!");
    LoggerInstance->Info("Displayed hello world demo");
    return String("{\"event\":\"helloDemo\"}"); });