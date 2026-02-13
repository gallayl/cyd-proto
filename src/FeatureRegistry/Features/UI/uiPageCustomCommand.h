#pragma once

#include <stdlib.h>
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"
#include "./Calibration.h"
#include "./screens/welcome.h"
#include "./screens/rgbled.h"

extern LGFX tft;

extern CustomCommand *pageCustomCommand;
