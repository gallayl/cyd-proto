#pragma once
#include "./cydConfig.h"
#include <XPT2046_Touchscreen_TT.h>
#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"

#include "../../Feature.h"
#include "../../../CommandInterpreter/CommandInterpreter.h"
#include "../Logging.h"

#include "./clearScreenCustomCommand.h"
#include "./helloCustomCommand.h"
#include "./calibrateCustomCommand.h"

Feature *UiFeature = new Feature("UI", []()
                                 {    
   // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  // ### lgfx change the rotation from (old) 2 to (new) 0
  tft.setRotation(0);  // In my case '2' is better as the terminals are at the top

  // Clear the screen
  tft.fillScreen(TFT_BLACK);


    // register our custom commands
    CommandInterpreterInstance->RegisterCommand(*clearScreenCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*helloDemoCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*calibrateCustomCommand);

    LoggerInstance->Info("UI feature initialized");

    return FeatureState::RUNNING; }, []() {});
