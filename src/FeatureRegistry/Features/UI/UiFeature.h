#pragma once

#include "../../../config.h"

#if ENABLE_UI

#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../Feature.h"
#include "../../../CommandInterpreter/CommandInterpreter.h"
#include "../Logging.h"

#include "./screenCustomCommand.h"
#include "./uiPageCustomCommand.h"
#include "./Calibration.h"

extern const int screenWidth;
extern const int screenHeight;

extern Feature *UiFeature;

void uiFeatureInit();

#endif
