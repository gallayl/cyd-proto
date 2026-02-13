#pragma once

#include "../../CommandInterpreter/CustomCommand.h"
#include "../../hw/WiFi.h"
#include <LittleFS.h>
#include <WiFi.h>

JsonDocument getInfo();

extern CustomCommand *infoCustomCommand;