#pragma once
#include <XPT2046_Touchscreen_TT.h>
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../Feature.h"
#include "../../../CommandInterpreter/CommandInterpreter.h"
#include "../Logging.h"

#include "./screenCustomCommand.h"
#include "./uiPageCustomCommand.h"
#include "./Calibration.h"

// 2. LVGL Buffer and Flush
// definitions moved to .cpp to avoid multiple instances
extern const int screenWidth;
extern const int screenHeight;
extern uint16_t drawBuffer[];

// forward declaration of feature object
extern Feature *UiFeature;

// helper if explicit initialization is needed
void uiFeatureInit();
