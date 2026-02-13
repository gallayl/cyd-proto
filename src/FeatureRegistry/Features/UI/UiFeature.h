#pragma once

#include "../../../cydConfig.h" // brings in TOUCH_CS and TFT configuration
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen_TT.h>

#include "../../Feature.h"
#include "../../../CommandInterpreter/CommandInterpreter.h"
#include "../Logging.h"

#include "clearScreenCustomCommand.h"
#include "helloCustomCommand.h"

// global objects used by commands
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS);

Feature *UiFeature = new Feature("UI", []()
                                 {    
    // initialize display
    tft.begin();
    tft.setRotation(1);

    // initialize touch controller
    ts.begin();
    ts.setRotation(1);

    // register our custom commands
    CommandInterpreterInstance->RegisterCommand(*clearScreenCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*helloDemoCustomCommand);

    LoggerInstance->Info("UI feature initialized");

    return FeatureState::RUNNING; }, []() {});
