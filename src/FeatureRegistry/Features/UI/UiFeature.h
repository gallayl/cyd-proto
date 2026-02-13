#pragma once
#include "./cydConfig.h"
#include <XPT2046_Touchscreen_TT.h>
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../Feature.h"
#include "../../../CommandInterpreter/CommandInterpreter.h"
#include "../Logging.h"

#include "./clearScreenCustomCommand.h"
#include "./helloCustomCommand.h"
#include "./calibrateCustomCommand.h"
#include "./rotateScreenCommand.h"
#include "./Calibration.h"

Feature *UiFeature = new Feature("UI", []()
                                 {    
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("UI Feature Initialized");

    readCalibrationData();


    // register our custom commands
    CommandInterpreterInstance->RegisterCommand(*clearScreenCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*helloDemoCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*calibrateCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*rotateScreenCustomCommand);


    LoggerInstance->Info("UI feature initialized");

    return FeatureState::RUNNING; }, []() {});
