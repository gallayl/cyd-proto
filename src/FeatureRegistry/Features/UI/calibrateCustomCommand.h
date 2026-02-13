#pragma once

#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../Logging.h"
#include "./Calibration.h"

CustomCommand *calibrateCustomCommand = new CustomCommand("calibrate", [](String command)
                                                          {    
    calibrateScreen();
    LoggerInstance->Info("Calibrated touch screen");
    return String("{\"event\":\"calibrate\", \"status\":\"success\"}"); });