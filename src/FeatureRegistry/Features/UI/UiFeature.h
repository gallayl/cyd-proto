#pragma once
#include <XPT2046_Touchscreen_TT.h>
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../Feature.h"
#include "../../../CommandInterpreter/CommandInterpreter.h"
#include "../Logging.h"

#include "./screenCustomCommand.h"
#include "./Calibration.h"

Feature *UiFeature = new Feature("UI", []()
                                 {    
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("UI Feature Initialized");

    readCalibrationData();


    // register our consolidated "screen" namespace command
    CommandInterpreterInstance->RegisterCommand(*screenCustomCommand);


    LoggerInstance->Info("UI feature initialized");

    return FeatureState::RUNNING; }, []() {});
