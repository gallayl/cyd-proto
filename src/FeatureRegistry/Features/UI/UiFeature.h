#pragma once

#include "../../../config.h"

#if ENABLE_UI

#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"
#include "./Calibration.h"

extern Feature *UiFeature;

#endif
